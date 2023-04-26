import serial
from serial.tools.list_ports import comports

pkt_type_dict = {
    0 : "CHECK",
    1 : "MSG",
    2 : "ACK",
    3 : "NACK"
}

netkey = b'\x01\x02\x03\x04'

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
    try:
        # receiving data and logging it into the target file
        while (True):
            data = ser.read()
            if (data):
                databuffer += data
                f = open(outputfile, 'ab')
                f.write(data)
                f.close()
            while (len(databuffer) >= 16):
                res = verifyIV(databuffer, netkey)
                if (res['VALID'] == 'VALID'):
                    databuffer = databuffer[16:]
                    print(f'[INFO] Valid {res["TYPE"]} package received.')
                    if (res['TYPE'] == 'MSG'):
                        print(f'[INFO] Receiving {res["LENGTH"]} bytes of data.')
                        msg_pkt = b''
                        for _ in range(res['LENGTH']):
                            # TODO
                            pass

                    elif (res['TYPE'] == 'CHECK'):
                        # TODO
                        pass
                    
                    elif (res['TYPE'] == 'ACK'):
                        # TODO
                        pass

                    elif (res['TYPE'] == 'NACK'):
                        # TODO
                        pass

                elif (res['VALID'] == 'INVALID'):
                    print('[WARNING] Invalid data discarded.')
                    databuffer = databuffer[1:]
                else:
                    databuffer = databuffer[1:]

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

    if (len(block) != 16):
        out['VALID'] = 'INCOMPLETE'
        return out
    elif (block[2:6] == netkey):
        if (block[0] in pkt_type_dict):
            out['TYPE'] = pkt_type_dict[block[0]]
            if (out['TYPE'] == 'MSG'):
                out['LENGTH'] = block[1]
            elif (block[1] > 0):
                out['VALID'] = 'INVALID'
        else:
            out['VALID'] = 'INVALID'
    else:
        out['VALID'] = 'INVALID'
        return out
    
    return out


    if (len(block) != 16):
        out['VALID'] = 'INVALID'
        return out
    
    pkt_type = block[0]
    if (pkt_type in pkt_type_dict):
        out['TYPE'] = pkt_type_dict[pkt_type]
    else:
        out['VALID'] = 'INVALID'
        return out

    if (block[2:6] != netkey):
        out['VALID'] = 'FOREIGN'
        return out
    
    if (out['TYPE'] == 'MSG'):
        out['LENGTH'] = block[1]
    elif (block[1] != 0):
        out['VALID'] == 'CORRUPT'
    
    return out

    
