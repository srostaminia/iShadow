#!/usr/bin/env python

import matplotlib.pyplot as plt
import numpy as np
import os
from os import listdir
from os.path import isfile, join
import shutil
import scipy
import scipy.stats.mstats as mstats
import pylab
import sys
import matplotlib.image as mpimg
import Image
import argparse


def main():
    parser = argparse.ArgumentParser()

    parser.add_argument("input_file", help="file to adjust")
    parser.add_argument("mean", type=float)
    parser.add_argument("std", type=float)

    args = parser.parse_args()

    # Get all files in directory
    # files = [ f for f in listdir('.') if isfile(join('.',f)) ]

    mean = args.mean
    std = args.std

    f = args.input_file
    if ".txt" not in f:
        print "ERROR: Must be a .txt file"
        sys.exit()

    print f

    name, ext = os.path.splitext(f)

    # oldImage = mpimg.imread(f)
    # oldImage = oldImage[1:]

    imageArray = np.array(read_pixel_data(f))

    # imageArray = np.dot(oldImage[...,:3], [0.299, 0.587, 0.114])

    print imageArray
    imageArray -= mean
    imageArray /= std
    print imageArray

    out_file = open(f, 'w')
    for line in imageArray:
        for item in line:
            out_file.write(str(item) + ' ')
        out_file.write('\n')
    out_file.close()

    img = pylab.figure()

    pylab.figimage(imageArray, cmap = pylab.cm.Greys_r)

    img.set_size_inches(1, 1)

    pylab.savefig(name + ".png", dpi=112)

    pylab.close()



def read_pixel_data(filename):
    try:
        data_file = open(filename, "r")
    except IOError:
        print "ERROR: Could not open", filename
        sys.exit()

    data = []

    for line in data_file:
        data.append([float(datum) for datum in line.split()])

    data_file.close()
    return data

main()
