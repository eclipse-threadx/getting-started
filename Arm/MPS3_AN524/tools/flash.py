# Copyright (c) 2021 Linaro Limited.

import pathlib
import serial
import shutil
import sys
import time

# Serial baudrate
SERIAL_BAUDRATE = 115200

# Path to image file
BIN_FILE = pathlib.Path(__file__).parent.absolute().parent / "build" / "app" / "mps3_524.bin"
# Path where image file should be copied
BIN_DEST_PATH = "/Volumes/V2M-MPS3/SOFTWARE"

def initiate_reboot(port):
    try:
        serial_port = serial.Serial(port, SERIAL_BAUDRATE, exclusive=False)
    except serial.SerialException:
        print("ERROR: Unable to update application image onto FPGA board")
        print(f"ERROR: Unable to open serial port {port}")
        sys.exit(1)
    else:
        # Send reboot command to mcc terminal
        # Wring the string `reboot\n` is causing data loss. Therefore send
        # individual characters and introduce 100ms delay between each
        # character.
        serial_port.write(b'r')
        time.sleep(0.1)
        serial_port.write(b'e')
        time.sleep(0.1)
        serial_port.write(b'b')
        time.sleep(0.1)
        serial_port.write(b'o')
        time.sleep(0.1)
        serial_port.write(b'o')
        time.sleep(0.1)
        serial_port.write(b't')
        time.sleep(0.1)
        serial_port.write(b'\n')

def copy_application_image():
    try:
        shutil.copy2(BIN_FILE, BIN_DEST_PATH, follow_symlinks=False)
    except OSError as e:
        print("ERROR: Unable to update application image onto FPGA board")
        print(f"ERROR: {e.strerror} : {e.filename}")
        sys.exit(1)

if __name__ == '__main__':
    # Copy application image onto USB drive
    copy_application_image()
    # Initiate reboot
    initiate_reboot(sys.argv[1])
