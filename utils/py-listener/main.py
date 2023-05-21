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

children = [{'TIMER' : -1, 'TIMEOUTS' : 0},
            {'TIMER' : -1, 'TIMEOUTS' : 0},
            {'TIMER' : -1, 'TIMEOUTS' : 0}] # timeouts for child nodes

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
        packet = parseHead(databuffer)
        if (packet):
            databuffer = databuffer[len(packet):]
        if (len(packet) > 1):
            print(f'yay')

def parseHead(buffer : bytes) -> bytes:
    blen = len(buffer)
    if (blen < 16):
        return None
    
    packlen = buffer[0]

    if (blen < packlen + 8):
        return None

    if (buffer[1:5] != net_key):
        return b'\00'
    
    seq_number = buffer[5:8]
    salt = buffer[:8]
    enc_payload = buffer[8:packlen+8]
    dec_payload = decrypt(enc_payload, salt)
    typecode = enc_payload[0]
    device_id = dec_payload[1:5] if (typecode != HEADER_SRCH) else dec_payload[1:3]
    timestamp = dec_payload[-4:]

    if (timestamp and not valid_time(timestamp)):
        return b'\00'


    if (not valid_seq(seq_number, device_id)):
        return b'\00'

    children[device_id[0] >> 6] = int(time.time())

    if (typecode == HEADER_MSG):
        payload_len = dec_payload[5]
        payload = dec_payload[6:payload_len+6]
        crc32 = dec_payload[payload_len+6:payload_len+10]
        if (not check_crc(payload, crc32)):
            return b'\00'


    elif (typecode == HEADER_CHECK):
        updatetimeout(device_id)
    
    elif (typecode == HEADER_ACK):
        pass

    elif (typecode == HEADER_CMD):
        payload_len = dec_payload[5]
        payload = dec_payload[6:payload_len+6]
        crc32 = dec_payload[payload_len+6:payload_len+10]
        if (not check_crc(payload, crc32)):
            return b'\00'

    elif (typecode == HEADER_SRCH):
        if (dec_payload[3:7] != net_key):
            return b'\00'
        #TODO

    elif (typecode == HEADER_ADP):
        # No action is necessary, since this is a static node
        pass

    else:
        return b'\00'


def decrypt(payload : bytes, salt : bytes) -> bytes:
    """
    decrypt(payload : bytes, salt : bytes) -> bytes

    Decryptes a payload using the salt and the pre-shared secret. Returns the decrypted value.
    """
    hash = SHA256.new()
    hash.update(secret + salt)
    key = hash.digest()
    aes = AES.new(key, AES.MODE_CBC, (0).to_bytes(16, byteorder='big'))
    IV = b'\00' * 16
    payload = AES.decrypt(key)
    return payload

def check_crc(bytestr : bytes, crc32 : bytes) -> bool:
    """
    check_crc(bytestr : bytes, crc32 : bytes) -> bool

    Checks the CRC32 digest of a byte string. Returns true if the given checksum is valid.
    """
    if (crc32 == (binascii.crc32(bytestr) & 0XFFFFFFFF)):
        return True
    else:
        return False

if __name__ == '__main__':
    main()