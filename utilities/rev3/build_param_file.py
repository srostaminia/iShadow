#!/usr/bin/env python2

import argparse
import sys
import os
import struct
import pickle
import numpy as np
from utils import keyboard

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--model-folder", help="model_folder containing model data files")
    parser.add_argument("--fpn-folder", help="model_folder containing model data files")
    parser.add_argument("output_file", help="target file for array output")
    args = parser.parse_args()

    model_folder = args.model_folder
    fpn_folder = args.fpn_folder
    out_file = args.output_file

    if os.path.exists(out_file):
       print "ERROR: File", out_file, "already exists"
       sys.exit()

    mask_names = ["eye_col_fpn", "eye_row_fpn", "out_col_fpn", "out_row_fpn"]
    fpn_masks = {}

    if fpn_folder != None and not os.path.isdir(fpn_folder):
        print "ERROR: FPN folder", fpn_folder, "does not exist or is a file"
        sys.exit()

    for mask_name in mask_names:
        mask_filename = fpn_folder + "/" + mask_name + ".pi"

        if fpn_folder != None and os.path.exists(mask_filename):
            mask_file = open(mask_filename,'rb')
            fpn_masks[mask_name] = pickle.load(mask_file)
            mask_file.close()
            print mask_filename
        else:
            fpn_masks[mask_name] = np.zeros((112,112))

    try:
        out_file = open(out_file + ".raw", "wb")
    except IOError:
        print "ERROR: Could not open", out_file
        sys.exit()

    # 112 rows x 112 columns, 16 bits per pixel, 4 fpn masks
    fpn_size = 112 * 112 * 16 * 4
    fpn_size = float(fpn_size) / 8          # Convert from bits to bytes

    print "FPN masks memory footprint:", '{0:.2f}'.format(fpn_size / 1000), "KB /", '{0:.2f}'.format(fpn_size / 1024), "KiB"
    print "Total memory footprint:", '{0:.2f}'.format(fpn_size / 1000), "KB /", '{0:.2f}'.format(fpn_size / 1024), "KiB"

    for mask_name in mask_names:
        mask_data = fpn_masks[mask_name]

        for i in range(112):
            for j in range(112):
                out_file.write(struct.pack('H', mask_data[i][j]))

    out_file.close()
    
main()
