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
    parser.add_argument("num_images", type = int, help="number of image pairs (or single images if --no-interleave) stored in the input file, set to 0 to run continuously")
    parser.add_argument("out_mask", help="outward-facing camera mask")
    parser.add_argument("eye_mask", nargs='?', help="eye-facing camera mask (only used for interleaved images)", default=None)
    parser.add_argument("--no-interleave", action="store_true", help="images are stored singly, not interleaved")
    parser.add_argument("--columnwise", action="store_true", help="images recorded columnwise on hardware instead of rowwise")
    
    group = parser.add_mutually_exclusive_group()
    group.add_argument("-o", "--offset", help="Number of images / pairs to skip on SD card", type=int)
    group.add_argument("--reuse-raw", action="store_true", help="Reuse previously stored raw image data files")

    args = parser.parse_args()

    input_filename = "/dev/disk1"
    file_prefix = args.file_prefix
    out_mask_filename = args.out_mask
    eye_mask_filename = args.eye_mask
    num_images = args.num_images
    interleaved = not args.no_interleave
    columnwise = args.columnwise
    num_skip = args.offset if args.offset != None else 0

    if interleaved == True and eye_mask_filename == None:
        print "Invalid Input: Parameter eye_mask must be provided if using interleaved images"
        sys.exit()

    if interleaved:
        eye_mask = load_mask(args.eye_mask)

    out_mask = load_mask(args.out_mask)

    if args.reuse_raw:
        os.chdir(file_prefix)
        num_images = num_images
    else:
        try:
            input_file = open(input_filename, "rb")
        except IOError:
            print "Input file", input_filename, "could not be opened."
            sys.exit()

        try:
            input_file.read(512)
        except IOError:
            print "Input file", input_filename, "not available for reading."
            sys.exit()

        if interleaved:
            input_file.seek(num_skip * 50176)
        else:
            input_file.seek(num_skip * 25088)

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

        if (interleaved):
            try:
                output_b = open(file_prefix + "_b.raw", "wb")
            except IOError:
                print "Input file", file_prefix + "\\" + file_prefix + "_b.raw", "could not be opened."
                sys.exit()

        print "Reading image data..."
        if (num_images > 0):
            for i in range(num_images):
                for j in range(2):
                    data = input_file.read(10752)
                    output_a.write(data)

                    if (interleaved):
                        data = input_file.read(10752)
                        output_b.write(data)

                if (i % 500 == 0):
                    print i, "images so far"

                data = input_file.read(3584)
                output_a.write(data)

                if (interleaved):
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

                    if (interleaved):
                        data = input_file.read(10752)
                        output_b.write(data)

                if (end):
                    print "Found", i, "images total"
                    num_images = i
                    break
                elif (i % 500 == 0):
                    print i, "images so far"

                data = input_file.read(3584)
                output_a.write(data)

                if (interleaved):
                    data = input_file.read(3584)
                    output_b.write(data)

                i += 1

        num_images = i + 1
        print "Total:", num_images, "images\n"

        output_a.close()

        if (interleaved):
            output_b.close()

    try:
        output_a = open(file_prefix + "_a.raw", "rb")
    except IOError:
        print "Input file", file_prefix + "\\" + file_prefix + "_a.raw", "could not be opened."
        sys.exit()

    if (interleaved):
        try:
            output_b = open(file_prefix + "_b.raw", "rb")
        except IOError:
            print "Input file", file_prefix + "\\" + file_prefix + "_b.raw", "could not be opened."
            sys.exit()

    if columnwise:
        # image = image.T
        out_mask = out_mask.T

        if interleaved:
            eye_mask = eye_mask.T

    if (interleaved):
        disp_save_images(output_b, eye_mask, file_prefix + "_eye", num_images)
        disp_save_images(output_a, out_mask, file_prefix + "_out", num_images)
    else:
        disp_save_images(output_a, out_mask, file_prefix, num_images)

    output_a.close()

    if (interleaved):
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

    # mask_data = mask_data[1:]

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
    # image = image[1:]
    
    figure = pylab.figure()

    # image[::2] -= mask_data[55]
    # image[1::2] -= mask_data[60]

    # image[:20] -= mask_data[58:78]

    # image -= mask_data[55]
    image -= mask_data
    image = image[1:]

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
