import serial
import time
from serial.tools.list_ports import comports
import binascii


pkt_type_dict = {
    0 : "NONE",
    1 : "MSG",
    2 : "CHECK",
    3 : "ACK",
    4 : "NACK"
}

timeout = 500
netkey = b'\x01\x02\x03\x04'
aeskey = b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'

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
            print('Invalid port.')
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
                if (state['TYPE'] == 'NONE'):
                    if (databuffer[0] in pkt_type_dict):
                        state['TYPE'] = pkt_type_dict[databuffer[0]]
                        if (state['TYPE'] == 'CHECK'):
                            state['LEN'] = 8
                        elif (state['TYPE'] == 'MSG'):
                            state['LEN'] = 15
                        elif (state['TYPE'] == 'ACK'):
                            state['LEN'] = 15
                        elif (state['TYPE'] == 'NACK'):
                            state['LEN'] = 15
                    packetbuffer = b'' + databuffer[:1]
                elif (state['TYPE'] == 'PAYLOAD'):
                    payloadbuffer += databuffer[:1]
                    state['LEN'] -= 1
                elif (state['LEN'] > 0):
                    packetbuffer += databuffer[:1]
                    state['LEN'] -= 1
                
                if (state['TYPE'] == 'PAYLOAD' and state['LEN'] == 0):
                    parsed = parsepayload(payloadbuffer, packetbuffer, aeskey)
                    if (parsed):
                        print(f'[INFO] Received {parsed["LEN"]} bytes of payload.')
                    else:
                        print(f'[WARNING] Corrupted payload.')
                    payloadbuffer = b''
                    state['TYPE'] = 'NONE'
                    state['LEN'] = 0

                elif (state['TYPE'] != 'NONE' and state['LEN'] == 0):
                    res = verifyIV(packetbuffer, netkey)
                    if (res['VALID'] == 'VALID'):
                        print(f'[INFO] Valid {res["TYPE"]} package received.')
                        if (res['TYPE'] == 'MSG'):
                            state['TYPE'] = 'PAYLOAD'
                            state['LEN'] = res['LEN']
                            print(f'[INFO] Receiving {res["LEN"]} bytes.')
                        elif (res['TYPE'] == 'CHECK'):
                            state['TYPE'] = 'NONE'
                            state['LEN'] = 0
                        elif (res['TYPE'] == 'ACK'):
                            pass
                        elif (res['TYPE'] == 'NACK'):
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

def verifyIV(block: bytes, netkey: bytes) -> dict:
    out = dict()
    out['VALID'] = 'VALID'

    if (len(block) < 9):
        out['VALID'] = 'INCOMPLETE'
        return out
    elif (block[1:5] == netkey):
        if (block[0] in pkt_type_dict):
            out['TYPE'] = pkt_type_dict[block[0]]
            if (out['TYPE'] == 'MSG'):
                out['LEN'] = block[9]
        else:
            out['VALID'] = 'INVALID'
    else:
        out['VALID'] = 'INVALID'
        return out
    
    return out
    
def parsepayload(payload: bytes, iv: bytes, key: bytes) -> bytes:
    parsed = dict()
    len = int(payload[-5])
    parsed['LEN'] = len
    parsed['DATA'] = payload[:len]
    parsed['CRC'] = int.from_bytes(payload[-4:], 'big')
    if (parsed['CRC'] != (binascii.crc32(parsed['DATA']) & 0XFFFFFFFF)):
        return None
    return parsed