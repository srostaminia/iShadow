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
    parser.add_argument("num_pairs", type = int, help="number of interleaved image pairs stored in the input file")

    args = parser.parse_args()

    input_filename = "/dev/disk1"
    file_prefix = args.file_prefix
    mask_filename = args.mask
    num_pairs = args.num_pairs

    try:
        input_file = open(input_filename, "rb")
    except IOError:
        print "Input file", input_filename, "could not be opened."
        sys.exit()

    mask = load_mask(mask_filename)

    try:
        img_data = open(file_prefix + "_images.raw", "wb")
    except IOError:
        print "Output file", file_prefix + "\\" + file_prefix + "_images.raw", "could not be opened."
        sys.exit()

    try:
        alt_img_data = open(file_prefix + "_altimages.raw", "wb")
    except IOError:
        print "Output file", file_prefix + "\\" + file_prefix + "_altimages.raw", "could not be opened."
        sys.exit()

    try:
        pred_text = open(file_prefix + "_pred.txt", "w")
    except IOError:
        print "Output file", file_prefix + "\\" + file_prefix + "_pred.txt", "could not be opened."
        sys.exit()

    try:
        minmax_text = open(file_prefix + "_minmax.txt", "w")
    except IOError:
        print "Output file", file_prefix + "\\" + file_prefix + "_minmax.txt", "could not be opened."
        sys.exit()

    try:
        dump_file = open(file_prefix + "_dump.raw", "wb")
    except IOError:
        print "Output file", file_prefix + "\\" + file_prefix + "_dump.raw", "could not be opened."
        sys.exit()

    if (num_pairs > 0):
        for i in range(num_pairs):            
            data = input_file.read(25088)
            alt_img_data.write(data)
            dump_file.write(data)

            data = input_file.read(2)
            predX = struct.unpack('h', data)[0]
            dump_file.write(data)

            data = input_file.read(2)
            predY = struct.unpack('h', data)[0]
            dump_file.write(data)

            pred_text.write(str(predX) + " " + str(predY) + '\n')

            data = input_file.read(2)
            min_val = struct.unpack('h', data)[0]
            dump_file.write(data)

            data = input_file.read(2)
            max_val = struct.unpack('h', data)[0]
            dump_file.write(data)

            minmax_text.write(str(min_val) + "\n" + str(max_val) + '\n')

            data = input_file.read(504)
            dump_file.write(data)

            data = input_file.read(25088)
            img_data.write(data)
            dump_file.write(data)

    else:
        end = False
        i = 0
        while (True):
            data = input_file.read(25088)
            if (ord(data[0]) == 0) and (ord(data[10000]) == 0) and (ord(data[15000]) == 0):
                end = True
                break
            
            i += 1
            img_data.write(data)
            dump_file.write(data)

            data = input_file.read(2)
            predX = struct.unpack('h', data)[0]
            dump_file.write(data)

            data = input_file.read(2)
            predY = struct.unpack('h', data)[0]
            dump_file.write(data)

            pred_text.write(str(predX) + " " + str(predY) + '\n')

            input_file.read(508)
            dump_file.write(data)

        if (end):
            print "Found", i, "images"

    img_data.close()
    pred_text.close()

    try:
        img_data = open(file_prefix + "_images.raw", "rb")
    except IOError:
        print "Input file", file_prefix + "_images.raw", "could not be opened."
        sys.exit()

    disp_save_images(img_data, mask, file_prefix + "_img")

    img_data.close()

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

    mask_data = mask_data[1:]

    return mask_data


def disp_save_images(image_file, mask_data, out_filename):
    # images = read_all_packed_images(image_file)

    # img = pylab.figure()
    # for i, image in enumerate(images):
    #     image -= mask_data

    #     pylab.figimage(image, cmap = pylab.cm.Greys_r)

    #     img.set_size_inches(1, 1)

    #     pylab.savefig(out_filename + "_" + ("%06d" % i) + ".png", dpi=112)

    success = True

    i = 0
    while success:
        if i % 500 == 0 and i != 0:
            print "Saved", i, "of", out_filename

        success = read_packed_image(image_file, mask_data, out_filename, i)

        i += 1

def read_packed_image(image_file, mask_data, out_filename, index):
    image = []
    for i in range(112):
        image.append([])
        
        for j in range(112):
            data = image_file.read(2)
            
            if data == "":
                # return []
                return False
            
            value = struct.unpack('h', data)[0]
            
            image[i].append(value)

    image = image[1:]

    img = pylab.figure()

    image -= mask_data

    pylab.figimage(image, cmap = pylab.cm.Greys_r)

    img.set_size_inches(1, 1)

    pylab.savefig(out_filename + "_" + ("%06d" % index) + ".png", dpi=112)

    pylab.close()
    
    # return np.array(image)

    return True

# def read_all_packed_images(image_file):
#     images = []
    
#     current_image = read_packed_image(image_file)
#     while (len(current_image) != 0):
#         images.append(current_image)
#         current_image = read_packed_image(image_file)
        
#     return images

main()
