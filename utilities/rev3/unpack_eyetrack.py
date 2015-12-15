#!/usr/bin/env python2

import sys
import argparse
import time
import numpy as np
import pylab
import struct
import pickle
import os
import csv

def main():
    parser = argparse.ArgumentParser()

    parser.add_argument("name", help="output file name")
    parser.add_argument("-d", "--disknum", help="/dev/disk[N] to open (default = 2)", type=int)
    parser.add_argument("-n", "--num-entries", type=int, help="number of entries to read")
    parser.add_argument("--overwrite", action="store_true", help="overwrite data file if it already exists")

    args = parser.parse_args()

    output_filename = args.name
    n = args.num_entries
    overwrite = args.overwrite

    if (args.disknum == None):
        input_filename = "/dev/disk2"
    else:
        input_filename = "/dev/disk" + str(args.disknum)

    try:
        input_file = open(input_filename, "rb")
    except IOError:
        print "Input file", input_filename, "could not be opened."
        sys.exit()

    try:
        input_file.read(512)
    except IOError:
        print "Input file", input_filename, "not available for reading."
        input_file.close()
        sys.exit()

    input_file.seek(0)

    if os.path.exists(output_filename) and overwrite == False:
        print "ERROR: output file", output_filename, "already exists"
        sys.exit()

    out_file = open(output_filename,'w')
    writer = csv.writer(out_file)

    data = None

    if n == None:
        while check_eod(data) != True:
            data = input_file.read(512)
            parse_write_results(data, out_file)
    else:
        for i in range(n):
            data = input_file.read(512)
            parse_write_results(data, out_file)

    out_file.close()
    input_file.close()

def parse_write_results(result_data, out_file):
    values1 = struct.unpack('I', result_data[0:4])
    values2 = struct.unpack('b' * 6, result_data[4:10])

    out_file.write(str(values1[0]) + ", ")
    out_file.write(str(values2[0]))
    for i in range(1,6):
        out_file.write(", " + str(values2[i]))
    out_file.write('\n')

main()
