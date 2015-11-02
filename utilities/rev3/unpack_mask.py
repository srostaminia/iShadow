#!/usr/bin/env python2

import sys
import argparse
import time
import numpy as np
import pylab
import struct
import pickle
import os

def main():
    parser = argparse.ArgumentParser()

    parser.add_argument("name", help="mask name")

    args = parser.parse_args()

    input_filename = "/dev/disk2"
    mask_filename = args.name + ".pi"

    try:
        input_file = open(input_filename, "rb")
    except IOError:
        print "Input file", input_filename, "could not be opened."
        sys.exit()

    try:
        mask_file = open(mask_filename, "w")
    except IOError:
        print "Mask file", mask_filename, "could not be opened."
        sys.exit()

    generate_mask(input_file, mask_file)


def disp_save_images(image_file, mask_data, out_filename):
    images = read_all_packed_images(image_file)

    img = pylab.figure()
    for i, image in enumerate(images):
        image -= mask_data

        pylab.figimage(image, cmap = pylab.cm.Greys_r)

        img.set_size_inches(1, 1)

        if (len(images) == 1):
            pylab.savefig(out_filename + ".png", dpi=112)
        else:
            pylab.savefig(out_filename + str(i) + ".png", dpi=112)

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
    
    return np.array(image)

def generate_mask(data_file, mask_output):
    pixels = read_packed_image(data_file)

    pixels = np.array(pixels,dtype='uint16')

    # pixels[0] = pixels[1]
    
    pixels -= np.amin(pixels)
    
    pickle.dump(pixels, mask_output)

    img = pylab.figure()
    pylab.figimage(pixels, cmap = pylab.cm.Greys_r)

    img.set_size_inches(1, 1)

    pylab.savefig("mask.png", dpi=112)

main()
