#Watchful IP 
#VERSION=0.0.1
#VDATE=20-12-24
#
#Wait for first bootloader autoboot prompt, send "slp" and then send flash commands from supplied text file one by one
#Verify we have prompt as expected on each action

import serial
import time

def read_until(ser, target, timeout=60, send_slp=False):
    start_time = time.time()
    buffer = b''
    slp_sent = False
    while time.time() - start_time < timeout:
        if ser.in_waiting:
            chunk = ser.read(ser.in_waiting)
            print(chunk.decode('ascii', errors='ignore'), end='', flush=True)
            buffer += chunk
            if send_slp and not slp_sent and b"Autobooting in 1 seconds" in buffer:
                ser.write(b'slp\n')
                slp_sent = True
                print("\nSent 'slp' command")
            if target.encode() in buffer:
                return True
        time.sleep(0.01)
    return False

def main():
    ser = serial.Serial('/dev/ttyAMA0', 115200, timeout=0.1)
    print("Serial port opened. Waiting up to 60 seconds for 'Autobooting in 1 seconds'...")

    ser.write(b'reset\n')   #send reset in case we left device in bootloader shell already
    
    if read_until(ser, "SigmaStar # ", timeout=60, send_slp=True):
        print("\nReceived 'SigmaStar # '. Reading commands from /i/read_keys_to_mem_cmds.txt...")
        
        with open('/i/read_keys_to_mem_cmds.txt', 'r') as f:
            commands = f.readlines()
        
        for cmd in commands:
            cmd = cmd.strip()
            print(f"\nSending command: {cmd}")
            ser.write(f"{cmd}\n".encode())
            
            if read_until(ser, "SF: ", timeout=5):
                print(f"Command '{cmd}' executed successfully.")
            else:
                print(f"Command '{cmd}' failed or didn't return 'SF: '. Stopping execution.")
                break
                
        ser.write(b'go 0x21000800\n')        
                
    else:
        print("Did not receive 'SigmaStar # ' within the timeout period. Exiting.")

    ser.close()
    print("Serial port closed.")

if __name__ == "__main__":
    main()
