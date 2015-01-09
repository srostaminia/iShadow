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
    parser.add_argument("out_mask", help="outward-facing camera mask")
    parser.add_argument("eye_mask", help="eye-facing camera mask")
    parser.add_argument("num_pairs", type = int, help="number of interleaved image pairs stored in the input file")

    args = parser.parse_args()

    input_filename = "/dev/disk1"
    file_prefix = args.file_prefix
    out_mask_filename = args.out_mask
    eye_mask_filename = args.eye_mask
    num_pairs = args.num_pairs

    if not os.path.exists(file_prefix):
        os.makedirs(file_prefix)
    else:
        print "Error: data folder " + file_prefix + " already exists."
        sys.exit()

    try:
        input_file = open(input_filename, "rb")
    except IOError:
        print "Input file", input_filename, "could not be opened."
        sys.exit()

    eye_mask = load_mask(args.eye_mask)
    out_mask = load_mask(args.out_mask)

    os.chdir(file_prefix)

    try:
        output_a = open(file_prefix + "_a.raw", "wb")
    except IOError:
        print "Input file", file_prefix + "\\" + file_prefix + "_a.raw", "could not be opened."
        sys.exit()

    try:
        output_b = open(file_prefix + "_b.raw", "wb")
    except IOError:
        print "Input file", file_prefix + "\\" + file_prefix + "_b.raw", "could not be opened."
        sys.exit()

    if (num_pairs > 0):
        for i in range(num_pairs):
            for j in range(7):
                data = input_file.read(3584)
                output_a.write(data)

                data = input_file.read(3584)
                output_b.write(data)
    else:
        end = False
        i = 0
        while (True):
            for j in range(7):
                data = input_file.read(3584)
                if (ord(data[0]) == 0) and (ord(data[1700]) == 0) and (ord(data[3583]) == 0):
                    end = True
                    break

                output_a.write(data)

                data = input_file.read(3584)
                output_b.write(data)

            if (end):
                print "Found", i, "images"
                break
            else:
                i += 1

    output_a.close()
    output_b.close()

    try:
        output_a = open(file_prefix + "_a.raw", "rb")
    except IOError:
        print "Input file", file_prefix + "\\" + file_prefix + "_a.raw", "could not be opened."
        sys.exit()

    try:
        output_b = open(file_prefix + "_b.raw", "rb")
    except IOError:
        print "Input file", file_prefix + "\\" + file_prefix + "_b.raw", "could not be opened."
        sys.exit()

    disp_save_images(output_a, out_mask, file_prefix + "_out")
    disp_save_images(output_b, eye_mask, file_prefix + "_eye")

    output_a.close()
    output_b.close()

    os.chdir("..")

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

def read_all_packed_images(image_file):
    images = []
    
    current_image = read_packed_image(image_file)
    while (len(current_image) != 0):
        images.append(current_image)
        current_image = read_packed_image(image_file)
        
    return images

main()
