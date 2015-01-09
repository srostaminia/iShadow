import serial
import sys
import argparse
import time
import numpy as np
import pylab
import pickle
#import Image

def main():    
    parser = argparse.ArgumentParser()

    parser.add_argument("mask_file", help="mask file")
    parser.add_argument("-m", help="generate mask file", action="store_true")
    parser.add_argument("-o", help="output file (if not generating mask)")

    args = parser.parse_args()
    
    mask_filename = args.mask_file
    gen_mask = args.m
    out_filename = args.o
    
#    ser = serial.Serial( port='/dev/ttyACM0', baudrate=115200, timeout=1 );
    ser = serial.Serial( port='/dev/tty.usbmodem12341', baudrate=57600, timeout=1 );
    
    
    if gen_mask:
        if out_filename != None:
            print "ERROR: Cannot specify output file (-o) if generating a mask (-m)"
            ser.close()
            sys.exit()
            
        try:
            mask_file = open(mask_filename, "w")
        except IOError:
            print "Mask file", mask_filename, "could not be opened."
            ser.close()
            sys.exit()
            
        generate_mask(ser, mask_file)
        
        mask_file.close()
    else:
        if out_filename == None:
            print "ERROR: Must specify output file (-o) if not generating a mask"
            ser.close()
            sys.exit()
            
        try:
            mask_file = open(mask_filename, "r")
        except IOError:
            print "Mask file", mask_filename, "could not be opened."
            ser.close()
            sys.exit()
            
        try:
            mask_data = pickle.load(mask_file)
        except pickle.PickleError:
            print "Mask file", mask_filename, "is corrupted."
            ser.close()
            sys.exit()
            
        mask_file.close()
            
        disp_save_image(ser, mask_data, out_filename)

    ser.close();
    
def generate_mask(ser, mask_file):
    pixels = read_data(ser)
    #f = open('mask_'+str(out_voltage)+'_'+str(out_round)+'.txt', 'w')
    #for i in range(0,112):
#	for j in range(0,112):
#		f.write('%d '%pixels[i][j])
#	f.write('\n')
#    f.close()

    pixels = np.array(pixels,dtype='uint16')
    
    pixels -= np.amin(pixels)
    
    pickle.dump(pixels, mask_file)
    
    img = pylab.figure()
    pylab.figimage(pixels, cmap = pylab.cm.Greys_r)
    
    img.set_size_inches(1, 1)
    
    pylab.savefig("mask_test.png", dpi=112)
    
#    img = Image.fromarray(pixels)
#    img.save("mask_test.png")
#    img.show()

def disp_save_image(ser, mask_data, out_filename):
    pixels = read_data(ser)
    
    pixels = np.array(pixels,dtype='uint16')
    
    pixels -= mask_data

    img = pylab.figure()
    pylab.figimage(pixels, cmap = pylab.cm.Greys_r)

    img.set_size_inches(1, 1)

    pylab.savefig(out_filename, dpi=112)
    
#    img = Image.fromarray(pixels)
#    img.save(out_filename)
#    img.show()

def read_data(ser):
    pixels = []
    
    ser.write('Z')
    data = ser.read(12544*2)
    
    if len(data) == 0:
        time.sleep(0.1)
        data = ser.read(12544*2)
        
#    if len(data) == 12544*2:
    data_img = []
    i = 0
    print len(data)
    while i<12544*2:
        data_item = int(ord(data[i+1])<<8)+int(ord(data[i]))
#        print data_item, " ",
        
        data_img.append(int(ord(data[i+1])<<8)+int(ord(data[i])))
        #data_img.append(int(ord(data[i])))
        i = i+2

    for i in range(112):
        pixels.append([])
        for j in range(112):
            item = data_img[(i * 112) + j]
        pixels[i].append(item)
    else:
        print "ERROR: Received", len(data), " bytes instead of expected", 12544*2
        sys.exit()
        
    return pixels

def read_data_string(ser):
    pixels = []
    
    ser.write('Z')
    data = ser.readlines()
    
    if len(data) == 0:
        time.sleep(0.1)
        data = ser.readlines()
    
    print data
    #for item in data:
    #    pixels.append(int(item))
    
    #return np.array(pixels).reshape((112,112))

main()
