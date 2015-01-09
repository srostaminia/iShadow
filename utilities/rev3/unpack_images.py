import sys
import argparse
import time
import numpy as np
import pylab
import struct
import pickle
import os

def main():
    parser = argparse.ArgumentParser()

    parser.add_argument("file_prefix", help="output file prefix")
    parser.add_argument("out_mask", help="outward-facing camera mask")
    parser.add_argument("eye_mask", help="eye-facing camera mask")
    parser.add_argument("num_pairs", type = int, help="number of interleaved image pairs stored in the input file")
    parser.add_argument("--reuse-raw", action="store_true", help="Reuse previously stored raw image data files")

    args = parser.parse_args()

    input_filename = "/dev/disk1"
    file_prefix = args.file_prefix
    out_mask_filename = args.out_mask
    eye_mask_filename = args.eye_mask
    num_pairs = args.num_pairs

    eye_mask = load_mask(args.eye_mask)
    out_mask = load_mask(args.out_mask)

    if args.reuse_raw:
        os.chdir(file_prefix)
        num_images = num_pairs
    else:
        try:
            input_file = open(input_filename, "rb")
        except IOError:
            print "Input file", input_filename, "could not be opened."
            sys.exit()

        if not os.path.exists(file_prefix):
            os.makedirs(file_prefix)
        else:
            print "Error: data folder " + file_prefix + " already exists."
            sys.exit()

        os.chdir(file_prefix)

        try:
            output_a = open(file_prefix + "_a.raw", "wb")
        except IOError:
            print "Input file", file_prefix + "\\" + file_prefix + "_a.raw", "could not be opened."
            sys.exit()

        try:
            output_b = open(file_prefix + "_b.raw", "wb")
        except IOError:
            print "Input file", file_prefix + "\\" + file_prefix + "_b.raw", "could not be opened."
            sys.exit()

        print "Reading image data..."
        if (num_pairs > 0):
            for i in range(num_pairs):
                for j in range(2):
                    data = input_file.read(10752)
                    output_a.write(data)

                    data = input_file.read(10752)
                    output_b.write(data)

                if (i % 500 == 0):
                    print i, "images so far"

                data = input_file.read(3584)
                output_a.write(data)

                data = input_file.read(3584)
                output_b.write(data)
        else:
            end = False
            i = 0
            while (True):
                for j in range(2):
                    data = input_file.read(10752)
                    if (ord(data[0]) < 0) and (ord(data[1700]) < 0) and (ord(data[3583]) < 0):
                        end = True
                        break

                    output_a.write(data)

                    data = input_file.read(10752)
                    output_b.write(data)

                if (end):
                    print "Found", i, "images total"
                    num_pairs = i
                    break
                elif (i % 500 == 0):
                    print i, "images so far"

                data = input_file.read(3584)
                output_a.write(data)

                data = input_file.read(3584)
                output_b.write(data)

                i += 1

        num_images = i
        print "Total:", num_images, "images\n"

        output_a.close()
        output_b.close()

    try:
        output_a = open(file_prefix + "_a.raw", "rb")
    except IOError:
        print "Input file", file_prefix + "\\" + file_prefix + "_a.raw", "could not be opened."
        sys.exit()

    try:
        output_b = open(file_prefix + "_b.raw", "rb")
    except IOError:
        print "Input file", file_prefix + "\\" + file_prefix + "_b.raw", "could not be opened."
        sys.exit()

    disp_save_images(output_b, eye_mask, file_prefix + "_eye", num_images)
    disp_save_images(output_a, out_mask, file_prefix + "_out", num_images)

    output_a.close()
    output_b.close()

    os.chdir("..")

def load_mask(mask_filename):
    try:
        mask_file = open(mask_filename, "r")
    except IOError:
        print "Mask file", mask_filename, "could not be opened."
        sys.exit()
        
    try:
        mask_data = pickle.load(mask_file)
    except pickle.PickleError:
        print "Mask file", mask_filename, "is corrupted."
        sys.exit()

    mask_data = mask_data[1:]

    mask_file.close()

    return mask_data


def disp_save_images(image_file, mask_data, out_filename, num_images):
    success = True

    print "Saving", out_filename + ":"

    i = 0
    while success and ((num_images == 0) or (i < num_images)):
        if i % 500 == 0 and i != 0:
            if num_images > 0:
                print i, '/', num_images
            else:
                print i, '/', 

        success = read_packed_image(image_file, mask_data, out_filename, i)

        i += 1

    print

def read_packed_image(image_file, mask_data, out_filename, index):
    image = []
    for i in range(112):
        image.append([])
        
        for j in range(112):
            data = image_file.read(2)
            
            if data == "":
                # return []
                return False
            
            value = struct.unpack('h', data)[0]
            
            image[i].append(value)

    # print image[0]
    image = image[1:]

    figure = pylab.figure()

    image -= mask_data

    pylab.figimage(image, cmap = pylab.cm.Greys_r)

    figure.set_size_inches(1, 1)

    pylab.savefig(out_filename + "_" + ("%06d" % index) + ".png", dpi=112)

    pylab.close()

    text_file = open(out_filename + "_" + ("%06d" % index) + ".txt", 'w')
    for line in image:
        for item in line:
            text_file.write(str(item) + ' ')
        text_file.write('\n')
    text_file.close()

    return True

main()
