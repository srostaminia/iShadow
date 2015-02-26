import argparse
import sys
import os
import struct
import pickle
import numpy as np

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("model_folder", help="folder containing model data files")
    parser.add_argument("output_file", help="target file for array output")
    parser.add_argument("eye_fpn_mask", help="file containing FPN mask for eye-facing camera")
    parser.add_argument("col_eye_fpn_mask", help="columnwise FPN mask for eye-facing camera (needed for CIDER)", nargs="?")
    parser.add_argument("--columnwise", help="store ANN pixels in column-major order", action='store_true')
    args = parser.parse_args()

    folder = args.model_folder
    out_file = args.output_file
    row_fpn_file = args.eye_fpn_mask
    col_fpn_file = args.col_eye_fpn_mask
    columnwise = args.columnwise

#    if os.path.exists(out_file):
#        print "ERROR: File", out_file, "already exists"
#        sys.exit()

    bh = read_model_data(folder + "/nn_bh.txt", is_vector=True)
    bo = read_model_data(folder + "/nn_bo.txt", is_vector=True)
    mask = read_model_data(folder + "/nn_mask.txt", is_vector=False)
    who = read_model_data(folder + "/nn_who.txt", is_vector=False)
    wih = read_model_data(folder + "/nn_wih.txt", is_vector=False)

    try:
        row_fpn_file = open(row_fpn_file, "r")
    except IOError:
        print "ERROR: Could not open", row_fpn_file
        sys.exit()

    if col_fpn_file != None:
        try:
            col_fpn_file = open(col_fpn_file, "r")
        except IOError:
            print "ERROR: Could not open", col_fpn_file
            sys.exit()

    row_fpn_data = pickle.load(row_fpn_file)

    if col_fpn_file != None:
        col_fpn_data = pickle.load(col_fpn_file)
        # col_fpn_data = col_fpn_data.T

    for i in range(len(mask)):
        mask[i][1] -= 1

    if columnwise:
        ind = np.lexsort((mask[:,0], mask[:,1]))

        # IMPORTANT NOTE that we are swapping row / column here b/c indexing on the glasses is screwed up
        mask = np.array([[mask[i,1], mask[i,0]] for i in ind])
        
        wih = np.array([wih[i,:] for i in ind])

    try:
        out_file = open(out_file + "_" + str(len(mask)) + "p" + ".raw", "wb")
    except IOError:
        print "ERROR: Could not open", out_file
        sys.exit()

    model_size = 32 * (len(bh) + len(bo) + (len(who) * len(who[0])) + (len(wih) * len(wih[0]))) + 16 * len(mask) * len(mask[0])
    model_size = float(model_size) / 8       # Convert from bits to bytes

    fpn_size = 112 * 112 * 16
    fpn_size *= 2 if col_fpn_file != None else 1
    fpn_size = float(fpn_size) / 8          # Convert from bits to bytes

    print "Number of pixels:", len(mask)
    print "Model memory footprint:", '{0:.2f}'.format(model_size / 1000), "KB /", '{0:.2f}'.format(model_size / 1024), "KiB"
    print "FPN mask(s) memory footprint:", '{0:.2f}'.format(fpn_size / 1000), "KB /", '{0:.2f}'.format(fpn_size / 1024), "KiB"
    print "Total memory footprint:", '{0:.2f}'.format((model_size + fpn_size) / 1000), "KB /", '{0:.2f}'.format((model_size + fpn_size) / 1024), "KiB"

    # Store number of pixels
    out_file.write(struct.pack('H', len(mask)))

    # Store number of hidden units
    out_file.write(struct.pack('H', len(who)))

    write_model_data_binary(out_file, bh, "bh", "float", is_vector=True)
    write_model_data_binary(out_file, bo, "bo", "float", is_vector=True)
    write_model_data_binary(out_file, mask, "mask", "unsigned short", is_vector=False)
    write_model_data_binary(out_file, who, "who", "float", is_vector=False)
    write_model_data_binary(out_file, wih, "wih", "float", is_vector=False)

    for i in range(112):
        for j in range(112):
            out_file.write(struct.pack('H', row_fpn_data[i][j]))

    if col_fpn_file != None:
        for i in range(112):
            for j in range(112):
                out_file.write(struct.pack('H', col_fpn_data[i][j]))

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
