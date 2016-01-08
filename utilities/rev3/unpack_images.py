#!/usr/bin/env python2

import sys
import argparse
import time
import numpy as np
import pylab
import struct
import pickle
import os
import shutil
from utils import keyboard

def main():
    parser = argparse.ArgumentParser()

    parser.add_argument("file_prefix", help="output file prefix")
    parser.add_argument("num_images", type = int, help="number of image pairs (or single images if --no-interleave) stored in the input file, set to 0 to run continuously")
    parser.add_argument("--out_mask", help="outward-facing camera mask to apply to data")
    parser.add_argument("--eye_mask", help="eye-facing camera mask to apply to data")
    parser.add_argument("--columnwise", action="store_true", help="images recorded columnwise on hardware instead of rowwise")
    parser.add_argument("--disknum", help="/dev/disk[N] to open (default = 2)", type=int)
    parser.add_argument("--overwrite", action="store_true", help="overwrite data folder if it already exists")
    
    groupA = parser.add_mutually_exclusive_group()
    groupA.add_argument("-o", "--offset", help="Number of images / pairs to skip on SD card", type=int)
    groupA.add_argument("--reuse-raw", action="store_true", help="Reuse previously stored raw image data files")

    groupB = parser.add_mutually_exclusive_group()
    groupB.add_argument("--outdoor-switch", nargs=2, help="out and eye mask (in that order) for outdoor switching mode")
    groupB.add_argument("--no-interleave", action="store_true", help="images are stored singly, not interleaved")

    args = parser.parse_args()

    file_prefix = args.file_prefix
    out_mask_filename = args.out_mask
    eye_mask_filename = args.eye_mask
    num_images = args.num_images
    interleaved = not args.no_interleave
    columnwise = args.columnwise
    overwrite = args.overwrite
    num_skip = args.offset if args.offset != None else 0
    outdoor_masks = args.outdoor_switch

    unit_size = 25088

    if interleaved:
        unit_size *= 2
    
    # For parameter data tacked on to each frame
    unit_size += 512

    if (args.disknum == None):
        input_filename = "/dev/disk2"
    else:
        input_filename = "/dev/disk" + str(args.disknum)

    if eye_mask_filename != None:
        eye_mask = load_mask(args.eye_mask)
    else:
        eye_mask = np.zeros((112,112))

    if out_mask_filename != None:
        out_mask = load_mask(args.out_mask)
    else:
        out_mask = np.zeros((112,112))

    if outdoor_masks != None:
        outdoor_out_mask = load_mask(outdoor_masks[0])
        outdoor_eye_mask = load_mask(outdoor_masks[1])
    else:
        outdoor_out_mask = None
        outdoor_eye_mask = None


    if args.reuse_raw:
        os.chdir(file_prefix)
        num_images = num_images

        results_out = make_results_file(file_prefix)
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
            input_file.close()
            sys.exit()

        if interleaved:
            input_file.seek(num_skip * unit_size)
        else:
            input_file.seek(num_skip * unit_size)

        if not os.path.exists(file_prefix):
            os.makedirs(file_prefix)
        elif overwrite == True:
            shutil.rmtree(file_prefix)
            os.makedirs(file_prefix)
        else:
            print "Error: data folder " + file_prefix + " already exists."
            sys.exit()

        results_out = make_results_file(file_prefix)

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
                for j in range(3):
                    data = input_file.read(7168)
                    output_a.write(data)

                    if (interleaved):
                        data = input_file.read(7168)
                        output_b.write(data)

                if (i % 500 == 0):
                    print i, "images so far"

                data = input_file.read(3584)
                output_a.write(data)

                if (interleaved):
                    data = input_file.read(3584)
                    output_b.write(data)

                data = input_file.read(512)
                parse_write_results(data, results_out)

        else:
            print "ERROR: Datastream end detection not working for the moment. Please specify a nonzero number of images to collect."
            return
            # end = False
            # i = 0
            # while (True):
            #     for j in range(2):
            #         data = input_file.read(10752)
            #         if (ord(data[0]) < 0) and (ord(data[1700]) < 0) and (ord(data[3583]) < 0):
            #             end = True
            #             break

            #         output_a.write(data)

            #         if (interleaved):
            #             data = input_file.read(10752)
            #             output_b.write(data)

            #     if (end):
            #         print "Found", i, "images total"
            #         num_images = i
            #         break
            #     elif (i % 500 == 0):
            #         print i, "images so far"

            #     data = input_file.read(3584)
            #     output_a.write(data)

            #     if (interleaved):
            #         data = input_file.read(3584)
            #         output_b.write(data)

            #     i += 1

        num_images = i + 1
        print "Total:", num_images, "images\n"

        input_file.close()
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

    if (interleaved):
        disp_save_images(output_b, eye_mask, file_prefix + "_eye", num_images, columnwise, outdoor_eye_mask)
        disp_save_images(output_a, out_mask, file_prefix + "_out", num_images, columnwise, outdoor_out_mask)
    else:
        disp_save_images(output_a, out_mask, file_prefix, num_images, columnwise)

    output_a.close()

    if (interleaved):
        output_b.close()
        
    results_out.close()

    os.chdir("..")

def make_results_file(file_prefix):
    try:
        results_out = open(file_prefix + "/" + file_prefix + "_model_results.csv", "w")
    except IOError:
        print "Output file", file_prefix + "\\" + file_prefix + "_model_results.csv", "could not be opened."
        sys.exit()

    return results_out

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


def parse_write_results(result_data, out_file):
    values1 = struct.unpack('I', result_data[0:4])
    values2 = struct.unpack('b' * 6, result_data[4:10])

    out_file.write(str(values1[0]) + ", ")
    out_file.write(str(values2[0]))
    for i in range(1,6):
        out_file.write(", " + str(values2[i]))
    out_file.write('\n')


def disp_save_images(image_file, mask_data, out_filename, num_images, columnwise, outdoor_mask=None):
    success = True

    print "Saving", out_filename + ":"

    i = 0
    while success and ((num_images == 0) or (i < num_images)):
        if i % 500 == 0 and i != 0:
            if num_images > 0:
                print i, '/', num_images
            else:
                print i, '/', 

        if outdoor_mask == None:
            success = read_packed_image(image_file, mask_data, out_filename, columnwise, i)
        else:
            success = read_packed_switching_image(image_file, mask_data, outdoor_mask, out_filename, columnwise, i)

        i += 1

    print

def read_packed_image(image_file, mask_data, out_filename, columnwise, index):
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

    image = np.array(image)

    # print image[0]
    # image = image[1:]
    
    figure = pylab.figure()

    if (columnwise):
        image = image.T

    # image -= mask_data
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

def read_packed_switching_image(image_file, mask_data, outdoor_mask, out_filename, columnwise, index):
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
    
    figure = pylab.figure()

    image = np.array(image)

    outdoor_mode = image[0][0]

    if outdoor_mode == 1:
        switch_text = 'outdoors'
        image -= outdoor_mask
    else:
        switch_text = 'indoors'
        image -= mask_data

    image = image[1:]

    if (columnwise):
        image = image.T

    pylab.figimage(image, cmap = pylab.cm.Greys_r)

    figure.set_size_inches(1, 1)

    pylab.savefig(out_filename + "_" + ("%06d" % index) + "_" + switch_text + ".png", dpi=112)

    pylab.close()

    text_file = open(out_filename + "_" + ("%06d" % index) + "_" + switch_text + ".txt", 'w')
    for line in image:
        for item in line:
            text_file.write(str(item) + ' ')
        text_file.write('\n')
    text_file.close()

    return True

main()
