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
    print len(mask)

    try:
        output = open(file_prefix + ".raw", "wb")
    except IOError:
        print "Input file", file_prefix + ".raw", "could not be opened."
        sys.exit()

    output.write(input_file.read(25088))

    output.close()

    try:
        output = open(file_prefix + ".raw", "rb")
    except IOError:
        print "Input file", file_prefix + ".raw", "could not be opened."
        sys.exit()

    fpn_file = open("pred_test/fpn_full", 'w')

    mask_subsample = read_model_data("/Users/ammayber/senseye/ann_model/glasses_model/subset_l1_init_strips_k7_lambda0.010000/rep1/nn_mask.txt", is_vector=False)

    for i in range(len(mask_subsample)):
        fpn_file.write(str(mask[mask_subsample[i][0]][mask_subsample[i][1] - 1]) + '\n')

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

def read_model_data(filename, is_vector, is_int=False):
    try:
        data_file = open(filename, "r")
    except IOError:
        print "ERROR: Could not open", filename
        sys.exit()

    if is_vector:
        if is_int:
            data = [int(datum) for datum in data_file.readline().split()]
        else:
            data = [float(datum) for datum in data_file.readline().split()]
    else:
        data = []

        for line in data_file:
            if is_int:
                data.append([int(datum) for datum in line.split()])
            else:
                data.append([float(datum) for datum in line.split()])

    data_file.close()
    return data

main()
