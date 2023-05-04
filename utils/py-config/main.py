#!/bin/python3


import os
import argparse
import serial
from serial.tools.list_ports import comports

def main():
    parser = argparse.ArgumentParser(description='A helper program to configure SPRN ESP32 modules')

    parser.add_argument('-G', '--generate', action='store_true', help='generates a config file')
    parser.add_argument('-C', '--configure', action='store_true', help='configures a board using a config file')
    parser.add_argument('-f', '--file', type=str, help='file to save the config')
    # parser.add_argument('-p', '--port', type=str, help='target COM port to be used')
    parser.add_argument('-k', '--key', type=str, help='128-bit AES key in HEX format')
    parser.add_argument('-i', '--id', type=str, help='network id')
    # parser.add_argument('-s', '--server', type=str, help='server address and port')
    args = parser.parse_args()

    # ARGUEMENT CHECKS AND ASSIGNMENTS
    # TODO
    

    conf_packet = b''

    if (not args.generate and os.path.exists(args.file)):
        # TODO
        # ASSIGN PARAMETERS
        pass
    else:
        raise ValueError(f'Invalid file path')
    
    if (args.generate):
        # GENERATE AN AES KEY, AND A NETWORK ID IF THEY ARE UNSPECIFIED
        # TODO
        pass

        # CREATE THE CONFIG PACKET
        # TODO
        
        # WRTIES THE CONFIGURATION TO THE SPECIFIED FILE
        if (args.file):
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
        
        ser = serial.Serial(ports[p_choice-1].device, 115200, timeout=1)

        # GET DEVICE PASSWORD FROM USER
        passwd = input(f'Enter the module\'s password: ')
        packet = bytes(passwd, 'ASCII') + b'\0' +conf_packet

        # SEND CONFIG PACKET TO DEVICE AND PRINT THE RESPONSE
        ser.write(packet)
        ser.flush()
        
        resp = ser.read(10)

        if (resp == 'DONE'):
            print('Device successfully configured.')
        elif (resp == 'PWERR'):
            print('Wrong password.')
        elif (resp == 'BADPK'):
            print('Packet Corrupted.')
        else:
            print(f'Unknown error: {resp}')

if __name__ == '__main__':
    main()