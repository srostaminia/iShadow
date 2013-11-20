import argparse
import sys
from os import listdir, mkdir
from os.path import isfile, join
import numpy as np
import Image
import struct
from copy import deepcopy

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("full_file", help="full image file to process")
    parser.add_argument("subsample_file", help="subsampled file to process")
    parser.add_argument("mask_file", help="mask file listing subsamples")
    
    args = parser.parse_args()
        
    try:
        full_packed = open(args.full_file, "rb")
    except IOError:
        print "Input file", args.full_file, "could not be opened."
        sys.exit()

    try:
        sub_packed = open(args.subsample_file, "rb")
    except IOError:
        print "Input file", args.subsample_file, "could not be opened."
        sys.exit()

    coord_list = read_model_data(args.mask_file, False, False)    
    
    full_image = read_packed_image(full_packed)
    full_packed.close()

    manual_samples = []
    for coord in coord_list:
        manual_samples.append(full_image[coord[0] - 1][coord[1] - 1])

    auto_samples = read_packed_pixels(sub_packed)
    sub_packed.close()

    if len(auto_samples) != len(manual_samples):
        print "Length mismatch:", len(auto_samples), len(manual_samples)
        return

    for i in range(len(auto_samples)):
        if abs(auto_samples[i] - manual_samples[i]) > 3:
            print i,auto_samples[i],manual_samples[i]
        
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

def read_packed_pixels(image_file):
    image = []

    data = image_file.read(2)
    while data != "":
        value = struct.unpack('h', data)[0]
        image.append(value)
        data = image_file.read(2)
    
    return np.array(image)

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
