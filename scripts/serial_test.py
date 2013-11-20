# This is a simple proof-of-concept for collecting Stonyman images via the
#   max32 board over the serial line
# Has some problems when running on Linux due to serial buffer overflow

import struct
import serial
import Image
import numpy as np
import sys
import time
import argparse

def main():
    parser = argparse.ArgumentParser()

    parser.add_argument("mask_file")
    parser.add_argument("output")
    parser.add_argument("serial_no", type=int)

    args = parser.parse_args()

    try:
        mask_file = open(args.mask_file, "rb")
    except IOError:
        print "Mask file", args.mask_file, "could not be opened."
        sys.exit()
    
    mask = read_packed_image(mask_file)
    mask -= np.amin(mask)
    mask_file.close()
    
    ser = serial.Serial(args.serial_no, 230400, timeout = 0.2)
    time.sleep(0.1)
    ser.readlines()
    
    data = np.array(collect_serial_image(ser),dtype='uint8')
    data -= mask   

    img = Image.fromarray(data)
    img.save(args.output)

    ser.close()

def collect_serial_image(ser):
    tries = 0
    data = []

    while not is_image_valid(data[1:]) and tries < 5:
        data = []
        ser.write('/single_transmit\n')
        temp = ser.readlines()
        while len(temp) != 0:
            data.extend(temp)
            temp = ser.readlines()
        tries += 1

    if (tries == 5):
        print "ERROR: Repeatedly failed to collect valid data"
        ser.close()
        sys.exit()

    data = [line[:-1] for line in data[1:]]
    image = []

    for i in range(112):
        image.append([])
        for j in range(0, 224, 2):
            image[i].append(struct.unpack('h', data[i][j:j+2])[0])
    
    return image

def is_image_valid(data):
    if len(data) != 112:
        return False

    for line in data:
        if len(line) != 225:
            return False

    return True

def read_packed_image(image_file):
    image = []
    for i in range(112):
        image.append([])
        
        for j in range(112):
            data = image_file.read(2)
            
            if data == "":
                return []
            
            value = struct.unpack('h', data)[0]
            
            image[i].append(value)
    
    return np.array(image,dtype='uint8')

main()
