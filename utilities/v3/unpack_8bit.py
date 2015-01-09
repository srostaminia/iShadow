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

    parser.add_argument("file_prefix", help="output file prefix")
    parser.add_argument("mask", help="camera mask")

    args = parser.parse_args()

    input_filename = "/dev/disk1"
    file_prefix = args.file_prefix
    mask_filename = args.mask

    try:
        input_file = open(input_filename, "rb")
    except IOError:
        print "Input file", input_filename, "could not be opened."
        sys.exit()

    mask = load_mask(mask_filename)

    try:
        output = open(file_prefix + ".raw", "wb")
    except IOError:
        print "Input file", file_prefix + ".raw", "could not be opened."
        sys.exit()

    output.write(input_file.read(12544))

    output.close()

    try:
        output = open(file_prefix + ".raw", "rb")
    except IOError:
        print "Input file", file_prefix + ".raw", "could not be opened."
        sys.exit()

    disp_save_images(output, mask, file_prefix)

    os.remove(output.name)

    output.close()

def load_mask(mask_filename):
    try:
        mask_file = open(mask_filename, "r")
    except IOError:
        print "Mask file", mask_filename, "could not be opened."
        sys.exit()
        
    try:
        mask_data = pickle.load(mask_file)
    except pickle.PickleError:
        print "Mask file", mask_filename, "is corrupted."
        sys.exit()

    mask_file.close()

    return mask_data


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
            data = image_file.read(1)
            
            if data == "":
                return []
            
            value = struct.unpack('B', data)[0]
            
            image[i].append(value)
    
    return np.array(image)

def read_all_packed_images(image_file):
    images = []
    
    current_image = read_packed_image(image_file)
    while (len(current_image) != 0):
        images.append(current_image)
        current_image = read_packed_image(image_file)
        
    return images

main()
