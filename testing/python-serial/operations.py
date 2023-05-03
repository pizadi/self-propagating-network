import serial
import time
import binascii
from serial.tools.list_ports import comports
from Crypto.Cipher import AES

pkt_type_dict = {
    0 : 'NONE',
    1 : 'MSG',
    2 : 'CHECK',
    3 : 'ACK',
    4 : 'NACK',
    5 : 'CMD',
    6 : 'ADP'
}

timeout = .1
netkey = b'\x01\x02\x03\x04'
aeskey = b'\x01\x02\x03\x04\x01\x02\x03\x04\x01\x02\x03\x04\x01\x02\x03\x04'
deviceid = 1

children = dict()

def listen(outputfile: str) -> None:
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

    print(f'Logging input data to {outputfile}...')

    databuffer = b''
    packetbuffer = b''
    payloadbuffer = b''
    lst_time = 0
    try:
        # receiving data and logging it into the target file
        state = {'TYPE' : 'NONE', 'LEN' : 0}
        while (True):
            data = ser.read()
            if (data):
                lst_time = time.time()
                databuffer += data
                f = open(outputfile, 'ab')
                f.write(data)
                f.close()

            if (len(databuffer) > 0):
            # processing data
                if (state['TYPE'] == 'NONE'):
                    if (databuffer[0] in pkt_type_dict):
                        state['TYPE'] = pkt_type_dict[databuffer[0]]
                        # setting the remaining number of bytes
                        if (state['TYPE'] == 'CHECK'):
                            state['LEN'] = 8
                        elif (state['TYPE'] == 'MSG'):
                            state['LEN'] = 15
                        elif (state['TYPE'] == 'ACK'):
                            state['LEN'] = 13
                        elif (state['TYPE'] == 'NACK'):
                            state['LEN'] = 8
                        elif (state['TYPE'] == 'CMD'):
                            state['LEN'] = 15
                        elif (state['TYPE'] == 'ADP'):
                            state['LEN'] = 12
                        
                    packetbuffer = b'' + databuffer[:1]
                elif (state['TYPE'] == 'PAYLOAD'):
                    payloadbuffer += databuffer[:1]
                    state['LEN'] -= 1
                elif (state['LEN'] > 0):
                    packetbuffer += databuffer[:1]
                    state['LEN'] -= 1
                
                if (state['TYPE'] == 'PAYLOAD' and state['LEN'] == 0):
                # parsing a completely transferred payload
                    parsed = parsepayload(payloadbuffer, packetbuffer, aeskey)
                    # print(''.join(f'{c:02X}' for c in payloadbuffer)) # prints the raw data
                    if (parsed):
                        print(f'[INFO] Received {parsed["LEN"]} bytes of payload.')
                        print(f'[INFO] Content: {parsed["DATA"]}')
                    else:
                        print(f'[WARNING] Corrupted payload.')
                    payloadbuffer = b''
                    state['TYPE'] = 'NONE'
                    state['LEN'] = 0

                elif (state['TYPE'] != 'NONE' and state['LEN'] == 0):
                # parsing a non-payload packet
                    res = verifyheader(packetbuffer, netkey)
                    if (res['VALID'] == 'VALID'):
                        print(f'[INFO] Valid {res["TYPE"]} package received from {res["DEVICE"]:08X}.')

                        if (res['TYPE'] == 'MSG' and res['DEVICE'] in children):
                            state['TYPE'] = 'PAYLOAD'
                            state['LEN'] = res['LEN']
                            print(f'[INFO] Receiving {res["LEN"]} bytes.')

                        elif (res['TYPE'] == 'CHECK'):
                            state['TYPE'] = 'NONE'
                            state['LEN'] = 0
                            if (res['DEVICE'] == 0):
                                new_id = newdeviceid()
                                if (new_id):
                                    info = dict()
                                    info['TYPE'] = 'ADP'
                                    info['NETWORK'] = netkey
                                    info['DEVICE'] = deviceid
                                    info['ADPDEVICE'] = new_id
                                    sendpacket(info)
                                    children[new_id] = 0

                        elif (res['TYPE'] == 'ACK'):
                            pass

                        elif (res['TYPE'] == 'NACK'):
                            pass
                        
                        elif (res['TYPE'] == 'CMD'):
                            pass

                        elif (res['TYPE'] == 'ADP'):
                            pass

                    else:
                        print('[WARNING] Invalid data discarded.')
                        state['TYPE'] = 'NONE'
                
                databuffer = databuffer[1:]
        
            if (state['TYPE'] != 'NONE' and time.time() - lst_time > 1.):
                state['TYPE'] = 'NONE'
                state['LEN'] = 0
                print('[WARNING] Incoming packet timeout occured.')

    # conditions for breaking the communication
    except serial.serialutil.SerialException:
        print(f'Reading error. Check for device connectivity. Quitting...')
        return
    except KeyboardInterrupt:
        print(f'Interrupt signal received. Quitting...')
        return

def verifyheader(block: bytes, netkey: bytes) -> dict:
    # verifies packet header
    out = dict()
    out['VALID'] = 'VALID'

    if (len(block) < 9):
        out['VALID'] = 'INCOMPLETE'
        return out
    elif (block[1:5] == netkey):
        out['DEVICE'] = int.from_bytes(block[5:9], 'big')
        if (block[0] in pkt_type_dict):
            out['TYPE'] = pkt_type_dict[block[0]]
            if (out['TYPE'] == 'MSG'):
                out['LEN'] = block[9]
            elif (out['TYPE'] == 'CHECK'):
                pass
        else:
            out['VALID'] = 'INVALID'
        
    else:
        out['VALID'] = 'INVALID'
        return out
    
    return out
    
def parsepayload(payload: bytes, iv: bytes, key: bytes) -> bytes:
    # decrypts and verifies a payload package
    parsed = dict()
    
    cipher = AES.new(aeskey, AES.MODE_CFB, iv=iv, segment_size=128)
    deciphered = cipher.decrypt(payload)
    len = int(deciphered[-9])
    parsed['TIME'] = int.from_bytes(deciphered[-8:-4], 'big')
    parsed['LEN'] = len
    parsed['DATA'] = deciphered[:len]
    parsed['CRC'] = int.from_bytes(deciphered[-4:], 'big')
    crc = (binascii.crc32(parsed['DATA']) & 0XFFFFFFFF)
    if (parsed['CRC'] != crc):
        print(f'[WARNING] CRC-32 mismatch: 0X{parsed["CRC"]:08X} vs 0X{crc:08X}')
        return None
    return parsed

def newdeviceid():
    # returns a new device id
    return 2

def sendpacket(info: dict) -> None:
    # broadcasts a packet
    pass