import argparse
import sys
import os

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("model_folder", help="folder containing model data files")
    parser.add_argument("output_file", help="target file for array output")
    args = parser.parse_args()

    folder = args.model_folder
    out_file = args.output_file

#    if os.path.exists(out_file):
#        print "ERROR: File", out_file, "already exists"
#        sys.exit()

    bh = read_model_data(folder + "\\nn_bh.txt", is_vector=True)
    bo = read_model_data(folder + "\\nn_bo.txt", is_vector=True)
    mask = read_model_data(folder + "\\nn_mask.txt", is_vector=False)
    who = read_model_data(folder + "\\nn_who.txt", is_vector=False)
    wih = read_model_data(folder + "\\nn_wih.txt", is_vector=False)

    for i in range(len(mask)):
        for j in range(len(mask[0])):
            mask[i][j] -= 1

    try:
        out_file = open(out_file, "w")
    except IOError:
        print "ERROR: Could not open", out_file
        sys.exit()

    write_model_data(out_file, bh, "bh", "float", is_vector=True)
    write_model_data(out_file, bo, "bo", "float", is_vector=True)
    write_model_data(out_file, mask, "mask", "unsigned short", is_vector=False)
    write_model_data(out_file, who, "who", "float", is_vector=False)
    write_model_data(out_file, wih, "wih", "float", is_vector=False)

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

def write_model_data(out_file, data, name, data_type, is_vector):
    out_file.write(data_type + " " + name)

    if data_type == "float":
        format_str = "%.25f"
    else:
        format_str = "%d"
    
    if is_vector:
        out_file.write("[" + str(len(data)) + "] = { ")

        out_file.write(format_str % data[0])
        for entry in data[1:]:
            out_file.write(", " + format_str % entry)
        out_file.write(" };\n\n")
    else:
        out_file.write("[" + str(len(data)) + "][" + str(len(data[0])) + "] = {\n")

        for line in data:
            out_file.write("\t{ ")

            out_file.write(format_str % line[0])
            for entry in line[1:]:
                out_file.write(", " + format_str % entry)
            out_file.write(" },\n")

        out_file.write("};\n\n")
    
main()
