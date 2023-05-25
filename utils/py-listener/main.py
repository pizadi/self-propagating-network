#!/bin/python3

import os
import argparse
import serial
import time
import binascii
from serial.tools.list_ports import comports
from Crypto.Cipher import AES
from Crypto.Hash import SHA256 

HEADER_MSG = 1 << 0
HEADER_CHECK = 1 << 1
HEADER_ACK = 1 << 2
HEADER_CMD = 1 << 3
HEADER_SRCH = 1 << 4
HEADER_ADP = 1 << 5

MAX_DELAY = 30

children = [{'TIMER' : -1, 'TIMEOUTS' : 0, 'TMPID' : -1},
            {'TIMER' : -1, 'TIMEOUTS' : 0, 'TMPID' : -1},
            {'TIMER' : -1, 'TIMEOUTS' : 0, 'TMPID' : -1}] # timeouts for child nodes

TIMEOUT = .1
output_file = None
secret = None
net_key = None
databuffer = b''

def main():
    # parsing the arguements and setting the global parameters
    parser = argparse.ArgumentParser(description='The internet node listener for SPRN networks')

    parser.add_argument('-c', '--config', type=str, help='network configuration file')
    parser.add_argument('-o', '--output', type=str, help='output file')

    args = parser.parse_args()

    if (args.config is None or not os.path.exists(args.config)):
        print(f'No valid configuration was given.')
        return
    else:
        config_file = open(args.config, 'rb')
        config = config_file.read()
        if (len(config) != 34):
            print('Corrupted config file.')
            return
        secret = config[:30]
        net_key = config[30:]        
    
    if (args.output is None):
        output_file = open('/dev/null', 'ab')
    
    else:
        output_file = open(args.output, 'ab')
        if (output_file is None):
            print(f'Invalid output file "{args.output}".')
            return
    
     # listing the available COM ports
    ports = comports()
    if (len(ports) == 0):
        print('No serial port was found. Quitting...')
        return
    print('Available Ports:')
    [print(f'{i+1}. {port}') for (i, port) in enumerate(ports)]

    # choosing a serial port
    p_choice = 0
    while (True):
        p_choice = int(input(f'Choose a port for serial communication({1}-{len(ports)}): '))
        if (p_choice < 1 or p_choice > len(ports)):
            print('Invalid port.')
            continue
        break
    
    # choosing the baud rate
    baud_rate = 0
    while (True):
        baud_rate = int(input(f'Choose a baud rate for serial communication: '))
        # checking for the validity of the baud rate
        if (not baud_rate in {2400, 9600, 115200}):
            print('Invalid baud rate.')
            continue
        break
    
    try:
        ser = serial.Serial(ports[p_choice-1].device, baud_rate)
    except ValueError:
        print(f'Invalid parameters for device. Quitting...')
        return
    except serial.serialutil.SerialException:
        print(f'Could not Connect to device. Quitting...')
        return

    if (args.output):
        print(f'Logging input data to {outputfile}...')

    # Reception loop
    while (True):
        databuffer += ser.read(64)
        parsed = parseHead(databuffer)
        if (isinstance(parsed, int)):
            databuffer = databuffer[parsed:] # Parser discards n bytes
        elif (isinstance(parsed, bytes)):
            databuffer = databuffer[len(parsed)+8:] # Parser discards n bytes
            # TODO
            # process packet
        else:
            print(f'[Warning] Failure in parsing packets. Discarding 1 byte.') # Parser failure due to bug, etc. Discards 1 byte
            databuffer = databuffer[1:]


def parseHead(buffer : bytes) -> Union[int, bytes]:
    """
    parseHead(buffer : bytes) -> Union[int, bytes]

    Parses the head of a RX buffer. Returns the number of bytes that should be discarded
    if the head does not contain a valid packet. If there is a valid packet, a decrypted
    version of the packet will be returned.
    """
    blen = len(buffer)
    if (blen < 16):
        return 0
    
    packlen = buffer[0]

    if (blen < packlen + 8):
        return 0

    if (buffer[1:5] != net_key):
        return 1
    
    seq_number = buffer[5:8]
    salt = buffer[:8]
    enc_payload = buffer[8:packlen+8]
    dec_payload = decrypt(enc_payload, salt)

    validity = validity_check(buffer[:8] + dec_payload)

    if (validity == -1):
        return 1
    elif (validity == 0):
        return packlen+8
    else:
        return dec_payload
    


def decrypt(packet : bytes, salt : bytes) -> bytes:
    """
    decrypt(packet : bytes, salt : bytes) -> bytes

    Decryptes a packet using the salt and the pre-shared secret. Returns the decrypted value.
    """
    hash = SHA256.new()
    hash.update(secret + salt)
    key = hash.digest()
    aes = AES.new(key, AES.MODE_CBC, (0).to_bytes(16, byteorder='big'))
    IV = b'\00' * 16
    dec_packet = AES.decrypt(packet)
    return dec_packet

def check_crc(bytestr : bytes, crc32 : bytes) -> bool:
    """
    check_crc(bytestr : bytes, crc32 : bytes) -> bool

    Checks the CRC32 digest of a byte string. Returns true if the given checksum is valid.
    """
    if (crc32 == (binascii.crc32(bytestr) & 0XFFFFFFFF)):
        return True
    else:
        return False

def validity_check(packet : bytes) -> int:
    """
    validity_check(packet : bytes) -> int
    
    Checks the validity of packet. Returns the proper typecode if valid and relevent. If the
    packet is valid but irrelevent due to timeouts or being directed to another node, the
    funciton will return 0. If the packet is completely invalid, the function will return -1.
    """
    typecode = packet[0]
    device_id = packet[9:13] if (typecode != HEADER_SRCH) else None
    timestamp = packet[-4:]

    if (typecode != HEADER_MSG and typecode != HEADER_CHECK and typecode != HEADER_ACK and typecode != HEADER_CMD and typecode != HEADER_SRCH and typecode != HEADER_ADP):
        return -1 # Invalid Typecode

    if (typecode == HEADER_MSG):
        origin_id = packet[13:17]
        plen = packet[17]

        if (plen > len(packet)-26): 
            return -1 # Packet is invalid if the payload len is longer than the packet

        payload = packet[18:plen+18]
        crc32 = packet[plen+18:plen+22]
        timestamp = packet[-4:]
        if (not valid_id(device_id)):
            return -1
        
        if (not valid_id(origin_id)):
            return -1
        
        if (not check_crc(payload, crc32)):
            return -1

        if (int.from_bytes(packet[plen+22:-4], byteorder='big') != 0):
            return -1 # Invalid zero-padding
    
    elif (typecode == HEADER_CHECK):
        if (not valid_id(device_id)):
            return -1
        
        if (int.from_bytes(packet[13:-4], byteorder='big') != 0):
            return -1 # Invalid zero-padding
    
    elif (typecode == HEADER_ACK):
        seq_number = packet[13:16]

        if (not valid_id(device_id)):
            return -1

        if (int.from_bytes(packet[16:-4], byteorder='big') != 0):
            return -1 # Invalid zero-padding

    elif (typecode == HEADER_CMD):
        target_id = packet[13:17]
        plen = packet[17]

        if (plen > len(packet)-26): 
            return -1 # Packet is invalid if the payload len is longer than the packet

        payload = packet[18:plen+18]
        crc32 = packet[plen+18:plen+22]
        timestamp = packet[-4:]
        if (not valid_id(device_id)):
            return -1
        
        if (not valid_id(origin_id)):
            return -1
        
        if (not check_crc(payload, crc32)):
            return -1

        if (int.from_bytes(packet[plen+22:-4], byteorder='big') != 0):
            return -1 # Invalid zero-padding
        
        return 0 # The main node does not receive CMD packets

    elif (typecode == HEADER_SRCH):
        temp_id = packet[9:11]

        if (int.from_bytes(packet[11:-4], byteorder='big') != 0):
            return -1 # Invalid zero-padding
        

    elif (typecode == HEADER_ADP):
        temp_id = packet[13:15]

        if (int.from_bytes(packet[15:-4], byteorder='big') != 0):
            return -1 # Invalid zero-padding
        return 0 # The main node does not receive ADP packets

    if (not is_child(device_id)):
        return 0 # External packet

    if (not valid_time(timestamp)):
        return 0 # Outdated packet

    if (not valid_seq(seq_number, device_id)):
        return 0 # Outdated packet
    
    return typecode

def valid_time(timestamp : bytes) -> bool:
    """
    valid_time(timestamp : bytes) -> bool

    Checks if the timestamp is valid and returns a corresponding boolean value. The maximum
    delay can be configured using the MAX_DELAY in seconds.
    """
    cur_t = int(time.time())
    diff = cur_t - int.from_bytes(timestamp, byteorder='big')
    if (diff > MAX_DELAY):
        return False
    return True

def valid_id(id : bytes) -> bool:
    """
    valid_id(id : bytes) -> bool

    Checks if a given byte string is a valid device ID and returns a corresponding boolean value.
    """
    flag = False
    for i in range(16):
        t = id[i//4] & (0b11000000 >> (i%4))
        if (not flag and t == 0):
            flag = True
        elif (flag and t > 0):
            return False
    return True


def is_child(id : bytes) -> bool:
    """
    is_child(id : bytes) -> bool

    Checks if a given byte string is a valid device ID and returns a corresponding boolean value.
    """
    t = id[0] & (0b11000000)
    r = id
    r[0] &= 0b00111111
    r = int.from_bytes(r, byteorder='big')
    if (t > 0 and r == 0):
        return True
    return False


if __name__ == '__main__':
    main()