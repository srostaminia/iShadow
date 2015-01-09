import argparse
import sys
import os
import struct
import pickle

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("model_folder", help="folder containing model data files")
    parser.add_argument("output_file", help="target file for array output")
    parser.add_argument("eye_fpn_mask", help="file containing FPN mask for eye-facing camera")
    args = parser.parse_args()

    folder = args.model_folder
    out_file = args.output_file
    fpn_file = args.eye_fpn_mask

#    if os.path.exists(out_file):
#        print "ERROR: File", out_file, "already exists"
#        sys.exit()

    bh = read_model_data(folder + "/nn_bh.txt", is_vector=True)
    bo = read_model_data(folder + "/nn_bo.txt", is_vector=True)
    mask = read_model_data(folder + "/nn_mask.txt", is_vector=False)
    who = read_model_data(folder + "/nn_who.txt", is_vector=False)
    wih = read_model_data(folder + "/nn_wih.txt", is_vector=False)

    try:
        fpn_file = open(fpn_file, "r")
    except IOError:
        print "ERROR: Could not open", fpn_file
        sys.exit()

    fpn_data = pickle.load(fpn_file)
    print len(fpn_data)

    # sample_file = open("eye_test.pi")
    # sample_data = pickle.load(sample_file)
    # sample_file.close()

    for i in range(len(mask)):
        mask[i][1] -= 1

    try:
        out_file = open(out_file + "_" + str(len(mask)) + "p" + ".raw", "wb")
    except IOError:
        print "ERROR: Could not open", out_file
        sys.exit()

    size = 32 * (len(bh) + len(bo) + (len(who) * len(who[0])) + (len(wih) * len(wih[0]))) + (16 * 2) * len(mask) * len(mask[0])
    size = float(size) / 8192

    print "Number of pixels:", len(mask)
    print "Model memory footprint:", size, "KiB"

    # Store number of pixels
    out_file.write(struct.pack('H', len(mask)))

    # Store number of hidden units
    out_file.write(struct.pack('H', len(who)))

    write_model_data_binary(out_file, bh, "bh", "float", is_vector=True)
    write_model_data_binary(out_file, bo, "bo", "float", is_vector=True)
    write_model_data_binary(out_file, mask, "mask", "unsigned short", is_vector=False)
    write_model_data_binary(out_file, who, "who", "float", is_vector=False)
    write_model_data_binary(out_file, wih, "wih", "float", is_vector=False)

    # print fpn_data[mask[0][0]][mask[0][1]]
    # print fpn_data[mask[1][0]][mask[1][1]]
    # print fpn_data[mask[2][0]][mask[2][1]]

    # for i in range(len(mask)):
        # out_file.write(struct.pack('H', fpn_data[mask[i][0]][mask[i][1]]))

    for i in range(112):
        for j in range(112):
            out_file.write(struct.pack('H', fpn_data[i][j]))

    # for i in range(112):
    #     for j in range(112):
    #         out_file.write(struct.pack('H', sample_data[i][j]))

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
    return data

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
