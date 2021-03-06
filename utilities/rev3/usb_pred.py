#!/usr/bin/env python2

import usb.control, usb.core
import sys
import argparse
import time
import numpy as np
import struct
import pickle
# from PIL import Image, ImageTk, ImageDraw, ImageColor
import os
import matplotlib.pyplot as plt
import matplotlib.image as mpimg
import matplotlib
import Tkinter
import shutil
from utils import keyboard

CONTRAST_ADJUST = 0
TX_BITS = 8
PARAM_PACKET_SIZE = 10

class ParamFields:
    def __init__(self):
        self.us_elapsed = float('NaN');
        self.model_type = float('NaN');
        self.pred_x = float('NaN');
        self.pred_y = float('NaN');
        self.cider_col = float('NaN');
        self.cider_row = float('NaN');
        self.cider_radius = float('NaN');

plt.ion()

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--noflip", help="don't flip image", action='store_true')
    args = parser.parse_args()

    noflip = args.noflip

    endp = get_usb_endp()

    iters = 0
    pred = [None, None]

    frame = np.zeros((112*112,))
    frame = np.reshape(frame, (112*112))
    fig = plt.figure(999) 
    image=plt.imshow(np.zeros((112,112)), cmap = matplotlib.cm.Greys_r, interpolation='nearest')

    ax=fig.add_subplot(111)
    ax.set_xlim([0, 112])
    ax.set_ylim([112, 0])

    packet_size = 92 * 16 / TX_BITS

    vline,=ax.plot([0, 1], [0, 1], 'r-', linewidth=2)
    hline,=ax.plot([0, 1], [0, 1], 'r-', linewidth=2)
    vline_cider,=ax.plot([-10, -9], [-10, -9], 'b-', linewidth=2)
    hline_cider,=ax.plot([-10, -9], [-10, -9], 'b-', linewidth=2)
    circle = plt.Circle((-10, -10), 0, fill=False, linewidth=2, color='g')
    ax.add_artist(circle)

    while True:
        data = endp.read(1840)

        # if (debug):
        #     print_packets(unpacked, packet_size)

        unpacked = struct.unpack('B' * 1840, data)

        # iterate over each packet (each containing 184 pixels)
        for packet in zip(*(iter(unpacked),) * 184):
            if any(v != 0 for v in packet):
                param_fields = parse_param_fields(packet, noflip)

                if param_fields.model_type == 0:
                    vline.set_data([-10, -10], [-10, -10])
                    hline.set_data([-10, -10], [-10, -10])
                    vline_cider.set_data([-10, -10], [-10, -10])
                    hline_cider.set_data([-10, -10], [-10, -10])
                    circle.center = -10, -10
                    circle.set_radius(1)
                else:
                    vline.set_data([param_fields.pred_x, param_fields.pred_x], [max(0, param_fields.pred_y - 10), min(111, param_fields.pred_y + 10)])
                    hline.set_data([max(0, param_fields.pred_x - 10), min(111, param_fields.pred_x + 10)], [param_fields.pred_y, param_fields.pred_y])
                    vline_cider.set_data([param_fields.cider_col, param_fields.cider_col], [0, 112])
                    hline_cider.set_data([0, 112], [param_fields.cider_row, param_fields.cider_row])
                    circle.center = param_fields.pred_x, param_fields.pred_y
                    circle.set_radius(param_fields.cider_radius)

                print "Frame collect time:", param_fields.us_elapsed / 1000, "ms"
                if param_fields.model_type != 0:
                    print "Prediction (X, Y):", param_fields.pred_x, param_fields.pred_y

                if param_fields.model_type == 1 or param_fields.model_type == 2:
                    print "CIDER Sample Point (X, Y):", param_fields.cider_col, param_fields.cider_row
                    print "CIDER Radius:", param_fields.cider_radius
                    
                print

                fig.canvas.draw()
                fig.canvas.flush_events()

        iters += 1


def print_packets(unpacked, packet_size):
    if (len(unpacked) % packet_size != 0):
        print "ERROR: Received length is not a multiple of specified packet size"
        sys.exit()

    for i in range(len(unpacked) / packet_size):
        data = unpacked[(i * packet_size):((i + 1) * packet_size)]
        print data, ", nonzero:", count_valid_bytes(data)

    print "\n"

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

    mask_file.close()

    return mask_data

def get_first(data):
    for item in data:
        if item != 0:
            return item

    return None

def get_last_idx(data):
    for i, item in enumerate(reversed(data)):
        if item != 0:
            return len(data) - i - 1

    return None

def check_param_packet(data, packet_size):
    for i in range(len(data) / packet_size):
        this_packet = data[i * packet_size:(i+1) * packet_size]
        last_data_idx = get_last_idx(this_packet)

        if last_data_idx != None and last_data_idx < PARAM_PACKET_SIZE:
            return i

    return None

def count_valid_bytes(data):
    count = 0
    for item in data:
        if item != 0:
            count += 1

    return count

def extract_pixel_data(data, packet_size):
    valid = []
    for i in range(len(data) / packet_size):
        this_packet = data[i * packet_size:(i+1) * packet_size]
        
        if get_first(this_packet) != None:
            valid.extend(this_packet[:112])

    return valid

def get_nonzero_pixels(data):
    nzp = []
    for item in data:
        if item != 0:
            nzp.append(item)

    return nzp

def get_zero_stop(data):
    for i, item in enumerate(data):
        if item != 0:
            return i

    return 0

def get_zero_start(data):
    for i, item in enumerate(data):
        if item == 0:
            return i

    return 0

def get_usb_endp():
    dev = usb.core.find(idVendor = 0x483)

    for cfg in dev:
        pass

    cfg.set()

    usb.control.set_interface(dev, 1, 1)

    for intf in cfg:
        pass

    for endp in intf:
        pass

    return endp

def parse_param_fields(param_data, noflip):
    param_fields = ParamFields()

    if TX_BITS == 8:
        data_type = 'B'
    elif TX_BITS == 16:
        data_type = 'H'

    timer_data = ''
    for i in range(4):
        timer_data += (struct.pack(data_type,param_data[i]))

    param_fields.us_elapsed = struct.unpack('I', timer_data)[0]

    param_fields.model_type = struct.unpack('h', struct.pack('H', param_data[4]))[0]
    param_fields.pred_y = struct.unpack('h', struct.pack('H', param_data[6]))[0]
    if noflip:
        param_fields.pred_x = struct.unpack('h', struct.pack('H', param_data[5]))[0]
    else:
        param_fields.pred_x = 112 - struct.unpack('h', struct.pack('H', param_data[5]))[0]

    if param_fields.model_type == 1 or param_fields.model_type == 2:
        param_fields.cider_row = struct.unpack('h', struct.pack('H', param_data[8]))[0]
        param_fields.cider_radius = struct.unpack('h', struct.pack('H', param_data[9]))[0]

        if noflip:
            param_fields.cider_col = struct.unpack('h', struct.pack('H', param_data[7]))[0]
        else:
            param_fields.cider_col = 112 - struct.unpack('h', struct.pack('H', param_data[7]))[0]
    else:
        param_fields.cider_row = -10
        param_fields.cider_col = -10
        param_fields.cider_radius = 0

    return param_fields

def read_packed_image(image_file):
    image = []
    for i in range(112):
        image.append([])
        
        for j in range(112):
            data = image_file.read(2)
            
            if data == "":
                return []
            
            value = struct.unpack('h', data)[0]
            
            image[i].append(value)
    
    return np.array(image)

def read_all_packed_images(image_file):
    images = []
    
    current_image = read_packed_image(image_file)
    while (len(current_image) != 0):
        images.append(current_image)
        current_image = read_packed_image(image_file)

    return images

main()
