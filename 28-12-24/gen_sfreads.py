#Watchful IP 
#VERSION=0.0.1
#VDATE=20-12-24
#
#Find flash reads from flash dump to reproduce data in supplied baremetal program


import sys
import binascii

start_addr = 0x21000800

def find_program_in_flash(program_data, flash_data):
    commands = []
    program_len = len(program_data)
    flash_len = len(flash_data)
    
    current_addr = start_addr
    
    i = 0
    while i < program_len:
        chunk = program_data[i:]
        pos = flash_data.find(chunk)
        if pos == -1:
            # If no match found, try smaller chunk
            chunk = chunk[:1]
            pos = flash_data.find(chunk)
            if pos == -1:
                print(f"Error: Unable to find program data in flash at offset {i}")
                return None
        
        chunk_len = len(chunk)
        commands.append(f"sf read 0x{current_addr:08x} 0x{pos:x} 0x{chunk_len:x}")
        
        i += chunk_len
        current_addr += chunk_len
    
    return commands

def main():

    global start_addr

    if len(sys.argv) < 3:
        print("Usage: python3 script.py <program_file> <flash_file>")
        sys.exit(1)
    
    program_file = sys.argv[1]
    flash_file = sys.argv[2]
    start_addr = int(sys.argv[3], 0)
    
    with open(program_file, 'rb') as f:
        program_data = f.read()
    
    with open(flash_file, 'rb') as f:
        flash_data = f.read(1024 * 1024)  # Read 1M of flash data
    
    commands = find_program_in_flash(program_data, flash_data)
    
    if commands:
        for cmd in commands:
            print(cmd)
    else:
        print("Error: Unable to generate sf read commands for the program")

if __name__ == "__main__":
    main()
