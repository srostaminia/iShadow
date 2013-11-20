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
    parser.add_argument("prediction_file", help="prediction file to process")
    parser.add_argument("output_file", help="output file to store predictions")
    
    args = parser.parse_args()
        
    try:
        pred_file = open(args.prediction_file, "rb")
    except IOError:
        print "Input file", args.prediction_file, "could not be opened."
        sys.exit()
        
    try:
        out_file = open(args.output_file, "w")
    except IOError:
        print "Output file", args.output_file, "could not be opened."
        sys.exit()

    x_data = pred_file.read(2)
    y_data = pred_file.read(2)
    while (x_data != ""):
        out_file.write(str(struct.unpack('h', x_data)[0]) + ",")
        out_file.write(str(struct.unpack('h', y_data)[0]) + "\n")

        x_data = pred_file.read(2)
        y_data = pred_file.read(2)

    pred_file.close()
    out_file.close()

main()
