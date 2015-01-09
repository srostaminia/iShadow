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
    parser.add_argument("eye_mask", help="camera mask")
    parser.add_argument("out_mask", help="camera mask")

    args = parser.parse_args()

    input_filename = "/dev/disk1"
    file_prefix = args.file_prefix

    try:
        input_file = open(input_filename, "rb")
    except IOError:
        print "Input file", input_filename, "could not be opened."
        sys.exit()

    save_image_raw(input_file, file_prefix + "_eye.raw")
    # input_file.read(512)
    save_image_raw(input_file, file_prefix + "_out.raw")

    disp_save_images(file_prefix + "_eye.raw", load_mask(args.eye_mask), file_prefix + "_eye")
    disp_save_images(file_prefix + "_out.raw", load_mask(args.out_mask), file_prefix + "_out")

def save_image_raw(input_file, filename):
    try:
        output = open(filename, "wb")
    except IOError:
        print "Input file", name, "could not be opened."
        sys.exit()

    output.write(input_file.read(25088))

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


def disp_save_images(image_filename, mask_data, out_filename):
    try:
        image_file = open(image_filename, "rb")
    except IOError:
        print "Intermediate file", image_filename, "could not be opened."
        sys.exit()

    images = read_all_packed_images(image_file)

    img = pylab.figure()
    for i, image in enumerate(images):
        image -= mask_data

        print image

        pylab.figimage(image, cmap = pylab.cm.Greys_r)

        img.set_size_inches(1, 1)

        if (len(images) == 1):
            pylab.savefig(out_filename + ".png", dpi=112)
        else:
            pylab.savefig(out_filename + str(i) + ".png", dpi=112)

        out_text = open(out_filename + ".txt",'w')
        for line in image:
            for item in line:
                out_text.write(str(item) + " ")
            out_text.write('\n')
        out_text.close()

    image_file.close()
    os.remove(image_file.name)  

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

def read_all_packed_images(image_file):
    images = []
    
    current_image = read_packed_image(image_file)
    while (len(current_image) != 0):
        images.append(current_image)
        current_image = read_packed_image(image_file)
        
    return images

main()
