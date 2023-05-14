#!/bin/python3

import os
import argparse
import operations

output_file = None
aes_key = None
net_key = None

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
        if (len(config) != 20):
            print('Corrupted config file.')
            return
        aes_key = config[:16]
        net_key = config[16:]        
    
    if (args.output is None):
        output_file = open('/dev/null', 'ab')
    elif (output_file = open(args.output, 'ab')):
        pass
    else:
        print(f'Invalid output file "{args.output}".')
        return
    
    # getting the desired port and baud rate
    operations.listen(FILE_NAME)

if __name__ == '__main__':
    main()