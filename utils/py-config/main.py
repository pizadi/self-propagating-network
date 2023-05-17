#!/bin/python3


import os
import argparse
import serial
from serial.tools.list_ports import comports
import secrets
import getpass

def main():
    parser = argparse.ArgumentParser(description='A helper program to configure SPRN ESP32 modules')

    parser.add_argument('-G', '--generate', action='store_true', help='generates a config file')
    parser.add_argument('-C', '--configure', action='store_true', help='configures a board using a config file')
    parser.add_argument('-f', '--file', type=str, help='file to save the config')
    # parser.add_argument('-p', '--port', type=str, help='target COM port to be used')
    parser.add_argument('-s', '--secret', type=str, help='240-bit SECRET in HEX format')
    parser.add_argument('-i', '--id', type=str, help='network id in hex format')
    # parser.add_argument('-b', '--baud-rate', type=str, help='baud rate')
    # parser.add_argument('-s', '--server', type=str, help='server address and port')
    args = parser.parse_args()

    # ARGUEMENT CHECKS AND ASSIGNMENTS
    conf_packet = b''

    # GETS THE SECRET
    if (args.generate and args.key is None):
        conf_packet += bytes.fromhex(secrets.token_hex(30))
    elif (args.generate):
        try:
            if (len(args.key) > 60):
                raise ValueError
            else:
                conf_packet += bytes.fromhex(args.key)
        except ValueError:
            print(f'Invalid AES key.')
            return
    
    # GETS THE NET ID
    if (args.generate and args.id is None):
        conf_packet += bytes.fromhex(secrets.token_hex(4))
    elif (args.generate):
        try:
            if (len(args.id) > 8):
                raise ValueError
            else:
                conf_packet += bytes.fromhex(args.id)
        except ValueError:
            print(f'Invalid network ID.')
            return

    if (not args.generate):
        if (os.path.exists(args.file)):
            # READS THE CONFIG FILE

            fptr = open(args.file, 'rb')
            cont = fptr.read()
            if (len(cont) != 34):
                print(f'invalid config file')
            else:
                conf_packet = cont
            fptr.close()
        else:
            print(f'Invalid file path')
            return

    if (args.generate and args.file is not None):
        # WRTIES THE CONFIGURATION TO THE SPECIFIED FILE
        
        fptr = open(args.file, 'wb')
        fptr.write(conf_packet)
        fptr.close()


    if (args.configure):

        # GET COM PORT FROM USER
        ports = comports()
        if (len(ports) == 0):
            raise IOError(f'No serial port was found.')
        print('Available Ports:')
        [print(f'{i+1}. {port}') for (i, port) in enumerate(ports)]
        
        p_choice = 0
        while (True):
            p_choice = int(input(f'Choose a port for serial communication({1}-{len(ports)}): '))
            if (p_choice < 1 or p_choice > len(ports)):
                print('Invalid port.')
                continue
            break
        try:
            ser = serial.Serial(ports[p_choice-1].device, 115200, timeout=1)
        except ValueError:
            print(f'Invalid COM port {ports[p_choice-1]}, could not connect.')
            return
        except serial.serialutil.SerialException:
            print(f'Could not connect. Port {ports[p_choice-1]} might be busy or unavailable.')
            return

        # GET DEVICE PASSWORD FROM USER
        passwd = getpass.getpass(f'Enter the module\'s password: ')
        packet = bytes(passwd, 'ASCII') + b'\0' +conf_packet

        # SEND CONFIG PACKET TO DEVICE AND PRINT THE RESPONSE
        ser.write(packet)
        ser.flush()
        
        resp = ser.read(4).decode('ASCII')

        if (resp == 'DONE'):
            print('Device successfully configured.')
        elif (resp == 'WRPW'):
            print('Wrong password.')
        elif (resp == 'WRPK'):
            print('Packet Corrupted.')
        else:
            print(f'Unknown error: {(resp)}')

if __name__ == '__main__':
    main()
