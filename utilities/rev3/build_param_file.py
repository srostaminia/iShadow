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

    if model_folder != None and not os.path.isdir(model_folder):
        print "ERROR: Model folder", model_folder, "does not exist or is a file"
        sys.exit()

    if fpn_folder != None and not os.path.isdir(fpn_folder):
        print "ERROR: FPN folder", fpn_folder, "does not exist or is a file"
        sys.exit()

    for mask_name in mask_names:
        mask_filename = fpn_folder + "/" + mask_name + ".pi"

        if fpn_folder != None and os.path.exists(mask_filename):
            mask_file = open(mask_filename,'rb')
            fpn_masks[mask_name] = pickle.load(mask_file)
            mask_file.close()
        else:
            fpn_masks[mask_name] = np.zeros((112,112))

    if model_folder != None:
        bh = read_model_data(model_folder + "/nn_bh.txt", is_vector=True)
        bo = read_model_data(model_folder + "/nn_bo.txt", is_vector=True)
        mask = read_model_data(model_folder + "/nn_mask.txt", is_vector=False)
        who = read_model_data(model_folder + "/nn_who.txt", is_vector=False)
        wih = read_model_data(model_folder + "/nn_wih.txt", is_vector=False)

        for i in range(len(mask)):
            mask[i][1] -= 1

        ind = np.lexsort((mask[:,0], mask[:,1]))

        mask = np.array([[mask[i,0], mask[i,1]] for i in ind])

        keyboard();
        
        wih = np.array([wih[i,:] for i in ind])

        model_size = 32 * (len(bh) + len(bo) + (len(who) * len(who[0])) + (len(wih) * len(wih[0]))) + 16 * len(mask) * len(mask[0])
        model_size = float(model_size) / 8       # Convert from bits to bytes

        out_file += "_" + str(len(mask)) + "p"
        print "Pixels in mask:", len(mask)
    else:
        model_size = 0

        out_file += "_0p"
        print "Pixels in mask: 0"

    try:
        out_file = open(out_file + ".raw", "wb")
    except IOError:
        print "ERROR: Could not open", out_file
        sys.exit()

    for mask_name in mask_names:
        mask_data = fpn_masks[mask_name]

        for i in range(112):
            for j in range(112):
                out_file.write(struct.pack('H', mask_data[i][j]))

    if model_folder != None:
        # Store number of pixels
        out_file.write(struct.pack('H', len(mask)))

        # Store number of hidden units
        out_file.write(struct.pack('H', len(who)))

        write_model_data_binary(out_file, bh, "bh", "float", is_vector=True)
        write_model_data_binary(out_file, bo, "bo", "float", is_vector=True)
        write_model_data_binary(out_file, mask, "mask", "unsigned short", is_vector=False)
        write_model_data_binary(out_file, who, "who", "float", is_vector=False)
        write_model_data_binary(out_file, wih, "wih", "float", is_vector=False)
    else:
        # Store zeros to indicate there are no model parameters in this file
        out_file.write(struct.pack('H', 0))
        out_file.write(struct.pack('H', 0))

    # 112 rows x 112 columns, 16 bits per pixel, 4 fpn masks
    fpn_size = 112 * 112 * 16 * 4
    fpn_size = float(fpn_size) / 8          # Convert from bits to bytes

    print "FPN masks memory footprint:", '{0:.2f}'.format(fpn_size / 1000), "KB /", '{0:.2f}'.format(fpn_size / 1024), "KiB"
    print "Model memory footprint:", '{0:.2f}'.format(model_size / 1000), "KB /", '{0:.2f}'.format(model_size / 1024), "KiB"
    print "Total memory footprint:", '{0:.2f}'.format((fpn_size + model_size) / 1000), "KB /", '{0:.2f}'.format((fpn_size + model_size) / 1024), "KiB"

    out_file.close()

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
    return np.array(data)

def write_model_data_binary(out_file, data, name, data_type, is_vector):
    if data_type == "float":
        format_str = "f"
    else:
        format_str = "H"
    
    if is_vector:
        for entry in data:
            out_file.write(struct.pack(format_str, entry))
    else:
        for line in data:
            for entry in line:
                out_file.write(struct.pack(format_str, entry))
    
main()
