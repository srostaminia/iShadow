import argparse
import sys
from os import listdir, mkdir
from os.path import isfile, join
import numpy as np
import Image
import struct
from copy import deepcopy
import numpy as np

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("image_folder", help="folder with images to be packed")
    parser.add_argument("output_file", help="output file for packed images")
    parser.add_argument("subsample_file", help="file containing subsample pixel list")

    args = parser.parse_args()

    try:
        images_packed = open(args.output_file, "wb")
    except IOError:
        print "Output file", args.output_file, "could not be opened."
        sys.exit()

    coord_list = read_model_data(args.subsample_file, False, False)
    print len(coord_list)

    files = sorted([ f for f in listdir(args.image_folder) if isfile(join(args.image_folder,f)) ])

    for f in files:
        if "eye.png" not in f:
            continue

        print f
        image = Image.open(args.image_folder + "/" + f)
        image = np.array(image)
        
        for coord in coord_list:
            images_packed.write(struct.pack('h',image[coord[0] - 1][coord[1] - 1]))

    images_packed.close()

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
