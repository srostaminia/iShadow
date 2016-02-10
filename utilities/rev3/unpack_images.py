#!/usr/bin/env python2

import sys
import argparse
import time
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import NullLocator
import struct
import pickle
import os
import shutil
import scipy.io as sio
from hdf5storage import savemat
from utils import keyboard
import itertools

class SaveOptions:
    def __init__(self, do_images, do_render, do_text):
        self.do_images  = do_images
        self.do_render  = do_render
        self.do_text    = do_text
        self.save_needed= do_images or do_text
        self.render_name= None

    def setRenderName(self, render_name):
        self.render_name = render_name

class ParamFields:
    def __init__(self):
        self.us_elapsed = float('NaN');
        self.model_type = float('NaN');
        self.pred_x = float('NaN');
        self.pred_y = float('NaN');
        self.cider_col = float('NaN');
        self.cider_row = float('NaN');
        self.cider_radius = float('NaN');

    def toList(self):
        list_rep =  [self.us_elapsed]
        list_rep.append(self.model_type)
        list_rep.append(self.pred_x) 
        list_rep.append(self.pred_y) 
        list_rep.append(self.cider_col) 
        list_rep.append(self.cider_row) 
        list_rep.append(self.cider_radius)

        return list_rep

def main():
    parser = argparse.ArgumentParser()

    parser.add_argument("file_prefix", help="output file prefix")
    parser.add_argument("--num_images", type = int, help="retrieve a set number of frames (ignoring end-of-data flag)")
    parser.add_argument("--out_mask", help="outward-facing camera mask to apply to data")
    parser.add_argument("--eye_mask", help="eye-facing camera mask to apply to data")
    parser.add_argument("--columnwise", action="store_true", help="images recorded columnwise on hardware instead of rowwise")
    parser.add_argument("--disknum", help="/dev/disk[N] to open (default = 2)", type=int)
    parser.add_argument("--overwrite", action="store_true", help="overwrite data folder if it already exists")
    parser.add_argument("--save-txt", action="store_true", help="save all the image data as separate .txt files")
    parser.add_argument("--save-png", action="store_true", help="save all the images as separate .png image files")
    parser.add_argument("--render", action="store_true", help="render eye tracking results in saved image files (ignored if images are not being saved)")

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

    save_options = SaveOptions(args.save_png, args.render, args.save_txt)

    unit_size = 25088

    if interleaved:
        unit_size *= 2
    
    # For parameter data tacked on to each frame
    unit_size += 512

    if (num_images != None and num_images <= 0):
        print "Error: Must specify positive nonzero value for num_images"
        sys.exit()

    if (args.disknum == None):
        input_filename = "/dev/disk2"
    else:
        input_filename = "/dev/disk" + str(args.disknum)

    if eye_mask_filename != None:
        eye_mask = load_mask(args.eye_mask)
    else:
        eye_mask = np.zeros((112,112)).astype('int64')

    if out_mask_filename != None:
        out_mask = load_mask(args.out_mask)
    else:
        out_mask = np.zeros((112,112)).astype('int64')

    if outdoor_masks != None:
        outdoor_out_mask = load_mask(outdoor_masks[0])
        outdoor_eye_mask = load_mask(outdoor_masks[1])
    else:
        outdoor_out_mask = None
        outdoor_eye_mask = None

    if interleaved:
        a_postfix = "_a.raw"
    else:
        a_postfix = ".raw"

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
            output_a = open(file_prefix + a_postfix, "wb")
        except IOError:
            print "Input file", file_prefix + "\\" + file_prefix + a_postfix, "could not be opened."
            sys.exit()

        if (interleaved):
            try:
                output_b = open(file_prefix + "_b.raw", "wb")
            except IOError:
                print "Input file", file_prefix + "\\" + file_prefix + "_b.raw", "could not be opened."
                sys.exit()

        frame_results = []
        print "Reading image data..."
        if (num_images != None):
            for i in range(num_images):
                for j in range(3):
                    data = input_file.read(7168)
                    output_a.write(data)

                    if (interleaved):
                        data = input_file.read(7168)
                        output_b.write(data)

                if (i % 500 == 0 and i != 0):
                    print i, "images so far"

                data = input_file.read(3584)
                output_a.write(data)

                if (interleaved):
                    data = input_file.read(3584)
                    output_b.write(data)

                data = input_file.read(512)

                # TODO: Make these variable names more sensible...
                frame_results.append(parse_write_results(data, results_out))

            i += 1

        else:
            i = 0
            while True:
                data_a = ''
                data_b = ''

                for j in range(3):
                    data_a += input_file.read(7168)

                    if (interleaved):
                        data_b += input_file.read(7168)

                if (i % 500 == 0 and i != 0):
                    print i, "images so far"

                data_a += input_file.read(3584)

                if (interleaved):
                    data_b += input_file.read(3584)

                data_param = input_file.read(512)

                if (is_eof(data_param)):
                    break

                output_a.write(data_a)

                if (interleaved):
                    output_b.write(data_b)

                frame_results.append(parse_write_results(data_param, results_out))

                i += 1

        num_images = i
        print "\nTotal:", num_images, "images\n"

        if i == 0:
            print "ERROR: No valid data found on disk (EOF at first sector)"
            sys.exit()

        frame_results = np.array(frame_results)

        input_file.close()
        output_a.close()

        if (interleaved):
            output_b.close()

    try:
        output_a = open(file_prefix + a_postfix, "rb")
    except IOError:
        print "Input file", file_prefix + "\\" + file_prefix + a_postfix, "could not be opened."
        sys.exit()

    if (interleaved):
        try:
            output_b = open(file_prefix + "_b.raw", "rb")
        except IOError:
            print "Input file", file_prefix + "\\" + file_prefix + "_b.raw", "could not be opened."
            sys.exit()

    save_data = {"frame_results" : np.array([param_fields.toList() for param_fields in frame_results])}
    if (interleaved):
        save_data["images_eye"] = read_images(output_a, eye_mask, file_prefix + "_eye", num_images, columnwise, outdoor_eye_mask)
        save_data["images_out"] = read_images(output_b, out_mask, file_prefix + "_out", num_images, columnwise, outdoor_out_mask)
        save_options.setRenderName("images_eye")
    else:
        save_data["images"] = read_images(output_a, out_mask, file_prefix, num_images, columnwise)
        save_options.setRenderName("images")

    output_a.close()

    if (interleaved):
        output_b.close()
        
    results_out.close()

    save_frames(file_prefix, save_data, save_options, frame_results)

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
    param_fields = ParamFields()

    param_fields.us_elapsed = struct.unpack('I', result_data[0:4])[0]
    params = struct.unpack('b' * 6, result_data[4:10])

    out_file.write(str(param_fields.us_elapsed) + ", ")
    out_file.write(str(params[0]))
    for i in range(1,6):
        out_file.write(", " + str(params[i]))
    out_file.write('\n')

    param_fields.model_type = params[0]
    param_fields.pred_x     = params[1]
    param_fields.pred_y     = params[2]
    param_fields.cider_col = params[3]
    param_fields.cider_row = params[4]
    param_fields.cider_radius = params[5]

    return param_fields

# Look for an all-zero param sector as EOF
def is_eof(result_data):
    values = struct.unpack('b' * 512, result_data)

    for byte in values:
        if byte != 0:
            return False

    return True


def read_images(image_file, mask_data, out_filename, num_images, columnwise, outdoor_mask=None):
    success = True

    images = np.zeros((num_images, 112 * 111))

    i = 0
    while success and ((num_images == 0) or (i < num_images)):
        if i % 500 == 0 and i != 0:
            if num_images > 0:
                print i, '/', num_images
            else:
                print i, '/', 

        if outdoor_mask == None:
            images[i] = read_packed_image(image_file, mask_data, out_filename, columnwise, i)
        # else:
            # success = read_packed_switching_image(image_file, mask_data, outdoor_mask, out_filename, columnwise, i)

        i += 1

    return images

def read_packed_image(image_file, mask_data, out_filename, columnwise, index):
    image = np.zeros((112,112))
    for i, j in itertools.product(range(112), range(112)):
            data = image_file.read(2)
            
            if data == "":
                print "ERROR: Unexpected end of file found"
                sys.exit()
            
            value = struct.unpack('h', data)[0]
            
            image[i,j] = value

    if columnwise:
        image = image.T

    image -= mask_data
    image = image[1:]

    # return flattened array
    return image.ravel('F')

# def read_packed_switching_image(image_file, mask_data, outdoor_mask, out_filename, columnwise, index):
#     image = []
#     for i in range(112):
#         image.append([])
        
#         for j in range(112):
#             data = image_file.read(2)
            
#             if data == "":
#                 # return []
#                 return False
            
#             value = struct.unpack('h', data)[0]
            
#             image[i].append(value)
    
#     figure = plt.figure()

#     image = np.array(image)

#     outdoor_mode = image[0][0]

#     if outdoor_mode == 1:
#         switch_text = 'outdoors'
#         image -= outdoor_mask
#     else:
#         switch_text = 'indoors'
#         image -= mask_data

#     if not columnwise:
#         image = image.T

#     image = image[1:]

#     plt.figimage(image, cmap = plt.cm.Greys_r)

#     figure.set_size_inches(1, 1)

#     plt.savefig(out_filename + "_" + ("%06d" % index) + "_" + switch_text + ".png", dpi=112)

#     plt.close()

#     text_file = open(out_filename + "_" + ("%06d" % index) + "_" + switch_text + ".txt", 'w')
#     for line in image:
#         for item in line:
#             text_file.write(str(item) + ' ')
#         text_file.write('\n')
#     text_file.close()

#     return True

def save_frames(filename, data, save_options, all_param_fields):
    savemat(filename + ".mat", data, format='7.3', store_python_metadata=True)

    if not save_options.save_needed:
        return

    for data_pair in data.items():
        if data_pair[0] == "frame_results":
            continue

        data_name = data_pair[0]

        save_name = filename
        if "_" in data_name:
            save_name += "_" + data_name.split("_")[1]

        print "Saving", save_name

        images = data_pair[1]

        for (i, image_orig), param_fields in itertools.izip(enumerate(images), all_param_fields):
            image = image_orig.reshape((111,112),order='F')

            if save_options.do_images:
                fig = plt.figure() 
                im_display=plt.imshow(image, cmap = plt.cm.Greys_r, interpolation='nearest')

                ax=fig.add_subplot(111)
                ax.set_xlim([0, 112])
                ax.set_ylim([112, 0])
                
                if save_options.do_render and save_options.render_name == data_name:
                    if param_fields.model_type != 0:
                        ax.plot([param_fields.pred_x, param_fields.pred_x], [max(0, param_fields.pred_y - 10), min(111, param_fields.pred_y + 10)], 'r-', linewidth=1)
                        ax.plot([max(0, param_fields.pred_x - 10), min(111, param_fields.pred_x + 10)], [param_fields.pred_y, param_fields.pred_y], 'r-', linewidth=1)

                        if param_fields.model_type == 1 or param_fields.model_type == 2:
                            ax.plot([param_fields.cider_col, param_fields.cider_col], [0, 112], 'b-', linewidth=1)
                            ax.plot([0, 112], [param_fields.cider_row, param_fields.cider_row], 'b-', linewidth=1)
                            circle = plt.Circle((param_fields.pred_x, param_fields.pred_y), param_fields.cider_radius, fill=False, linewidth=1, color='g')
                            ax.add_artist(circle)

                fig.set_size_inches(1, 1)
                frame1 = plt.gca()
                frame1.axes.get_xaxis().set_visible(False)
                frame1.axes.get_yaxis().set_visible(False)
                plt.axis('off')
                plt.savefig(save_name + "_" + ("%06d" % i) + ".png", dpi=112,bbox_inches='tight',pad_inches=0)

                plt.close()

            if save_options.do_text:
                text_file = open(save_name + "_" + ("%06d" % i) + ".txt", 'w')
                for line in image:
                    for item in line:
                        text_file.write(str(int(item)) + ' ')
                    text_file.write('\n')
                text_file.close()

        print

main()
