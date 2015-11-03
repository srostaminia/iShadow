#!/usr/bin/env python2

import usb.control, usb.core
import sys
import argparse
import time
import numpy as np
import pylab
import struct
import pickle
# from PIL import Image, ImageTk, ImageDraw, ImageColor
import os
import matplotlib.pyplot as plt
import matplotlib.image as mpimg
import matplotlib
import Tkinter
import shutil
import utils

CONTRAST_ADJUST = 0
TX_BITS = 8

plt.ion()

def main():
    parser = argparse.ArgumentParser()

    parser.add_argument("--debug_folder", help="debug mode output file prefix")
    parser.add_argument("--mask", help="camera mask")

    group = parser.add_mutually_exclusive_group()
    group.add_argument("--gen_mask", help="generate new camera mask with name")
    group.add_argument("--noflip", help="don't flip image (debug option)", action='store_true')
    
    parser.add_argument("--columnwise", help="images being sent in column-major order", action="store_true")
    parser.add_argument("--norender", help="don't render ANN / CIDER output", action="store_true")

    args = parser.parse_args()
    debug_folder = args.debug_folder
    mask_filename = args.mask
    gen_mask_file = args.gen_mask
    columnwise = args.columnwise
    noflip = args.noflip
    norender = args.norender

    if (debug_folder != None): 
        if os.path.isdir(debug_folder):
            # erase = 'x'
            # while (erase.lower() != 'y' and erase.lower() != 'n'):
            #     erase = raw_input("Data folder " + debug_folder + " already exists. Erase old data and proceed (y/n)?")

            # if erase == 'n':
            #     print "\nHalting execution."
            #     sys.exit()

            shutil.rmtree(debug_folder)

        os.mkdir(debug_folder)

        debug = True
    else:
        debug = False

    if (mask_filename == None):
        mask = None
    else:
        mask = load_mask(mask_filename)

    if gen_mask_file == None:
        gen_mask = False
    else:
        gen_mask = True

    if debug == True or gen_mask == True:
        norender = True

    endp = get_usb_endp()

    #imfig = pylab.figure()

    iters = 0
    save_filename = None
    pred = [None, None]
    next_start = []

    frame = np.zeros((112*112,))
    fig = plt.figure(999) 
    image=plt.imshow(np.zeros((112,112)), cmap = matplotlib.cm.Greys_r, interpolation='nearest')

    ax=fig.add_subplot(111)
    ax.set_xlim([0, 112])
    ax.set_ylim([112, 0])

    packet_size = 92 * 16 / TX_BITS

    if norender:
        vline,=ax.plot([0, 1], [0, 1], 'r-', linewidth=2)
        hline,=ax.plot([0, 1], [0, 1], 'r-', linewidth=2)
        vline_cider,=ax.plot([-10, -9], [-10, -9], 'b-', linewidth=2)
        hline_cider,=ax.plot([-10, -9], [-10, -9], 'b-', linewidth=2)
        circle = plt.Circle((-10, -10), 0, fill=False, linewidth=2, color='g')
        ax.add_artist(circle)

    while True:
        frame = np.reshape(frame, (112*112))

        if (debug and iters == 2):
            sys.exit()

        if len(next_start) != 0:
            start_pixels = extract_pixel_data(next_start, packet_size)

            pixels = len(start_pixels)
            frame[:pixels] = start_pixels

            if (debug):
                if TX_BITS == 8:
                    print_packets(next_start, 184)
                elif TX_BITS == 16:
                    print_packets(next_start, 92)

            data_started = 1
            packets = 0
            next_start = []
        else:
            pixels = 0
            data_started = 0
            packets = 0

        rx_complete = False
        while not rx_complete:
            data = endp.read(1840)

            if get_first(data) == None:
                if data_started == 0:
                    continue
            else:
                data_started = 1

            packets += 1

            if TX_BITS == 16:
                # For 16-bit transmission
                unpacked = struct.unpack('H' * 920, data)
            elif TX_BITS == 8:
                # For 8-bit transmission
                unpacked = struct.unpack('B' * 1840, data)

            if (debug):
                print_packets(unpacked, packet_size)

            param_idx = check_param_packet(unpacked, packet_size)

            # utils.keyboard()
            # Parameter packet is the end of an image transmission
            if param_idx != None:
                next_start = unpacked[(param_idx + 1) * packet_size:]
                param_data = unpacked[param_idx * packet_size:(param_idx * packet_size) + 6]
                unpacked = unpacked[:param_idx * packet_size]

                rx_complete = True

            valid_bytes = extract_pixel_data(unpacked, packet_size)

            valid_bytes = np.array(valid_bytes)
            new_pixels = len(valid_bytes)
            try:
                frame[pixels:(pixels+new_pixels)] = valid_bytes
            except:
                # utils.keyboard()
                # sys.exit()
                frame[pixels:] = valid_bytes[:(12544-pixels)]

            pixels += new_pixels

            # print unpacked, "\n"
            if (debug):
                print "Pixels in packet:", new_pixels
                print "Total pixels:", pixels, "\n\n"

            # print len(valid_bytes), "\n", valid_bytes, "\n"
            #valid_packed = struct.pack('H' * len(valid_bytes), *valid_bytes)
            #output.write(valid_packed)

        # TODO: Fix this so we're not copying the entire image every time...
        frame=np.reshape(frame,(112,112))   

        # utils.keyboard()
        model_type = struct.unpack('h', struct.pack('H', param_data[0]))[0]
        pred[1] = struct.unpack('h', struct.pack('H', param_data[2]))[0]
        if noflip:
            pred[0] = struct.unpack('h', struct.pack('H', param_data[1]))[0]
        else:
            pred[0] = 112 - struct.unpack('h', struct.pack('H', param_data[1]))[0]

        if model_type == 1 or model_type == 2:
            cider_row = struct.unpack('h', struct.pack('H', param_data[4]))[0]
            cider_radius = struct.unpack('h', struct.pack('H', param_data[5]))[0]

            if noflip:
                cider_col = struct.unpack('h', struct.pack('H', param_data[3]))[0]
            else:
                cider_col = 112 - struct.unpack('h', struct.pack('H', param_data[3]))[0]
        else:
            cider_col = -10
            cider_row = -10
            cider_radius = 0

        if norender:
            if model_type == 0:
                vline.set_data([-10, -10], [-10, -10])
                hline.set_data([-10, -10], [-10, -10])
                vline_cider.set_data([-10, -10], [-10, -10])
                hline_cider.set_data([-10, -10], [-10, -10])
                circle.center = -10, -10
                circle.set_radius(1)
            else:
                vline.set_data([pred[0], pred[0]], [max(0, pred[1] - 10), min(111, pred[1] + 10)])
                hline.set_data([max(0, pred[0] - 10), min(111, pred[0] + 10)], [pred[1], pred[1]])
                vline_cider.set_data([cider_col, cider_col], [0, 112])
                hline_cider.set_data([0, 112], [cider_row, cider_row])
                circle.center = pred[0], pred[1]
                circle.set_radius(cider_radius)

        print "Pixels:", pixels
        print "Packets:", packets
        print "Prediction (X, Y):", pred[0], pred[1]

        if model_type == 1 or model_type == 2:
            print "CIDER Point (X, Y):", cider_col, cider_row
            print "CIDER Radius:", cider_radius
            
        print

        # if (debug == 0):
        if (not gen_mask and mask != None):
            frame -= mask #************************UNCOMMENT ME ***************

        if (CONTRAST_ADJUST == 1):
            frame -= np.mean(frame)
            frame /= float(np.std(frame))

        if columnwise:
            frame = frame.T
            # tmp = [pred[1], pred[0]]
            # pred = tmp

        if not gen_mask and not noflip and not debug:
            frame = np.fliplr(frame)

        #plt.imshow(frame2, cmap = pylab.cm.Greys_r)
        image.set_data(frame)
        image.autoscale() 

        plt.draw()

        iters += 1

        # outfile = open("usb_frame.pi",'w')
        # pickle.dump(frame, outfile)
        # outfile.close()

        # endp.read(1840)

        if (debug and iters < 2):        
            out_text = open(debug_folder + "/usb_frame.txt",'w')
            for line in frame:
                for item in line:
                    out_text.write(str(item) + " ")
                out_text.write('\n')
            out_text.close()
            
            frame1 = plt.gca()
            frame1.axes.get_xaxis().set_visible(False)
            frame1.axes.get_yaxis().set_visible(False)
            plt.savefig(debug_folder + "/" + debug_folder + ".png")
            # sys.exit()

        if (gen_mask and iters == 2):
            mask_file = open(gen_mask_file + ".pi","wb")

            if (TX_BITS == 16):
                frame = np.array(frame, dtype='uint16')
            elif (TX_BITS == 8):
                frame = np.array(frame, dtype='uint8')

            frame -= np.amin(frame)
            pickle.dump(frame, mask_file)
            mask_file.close()

            frame1 = plt.gca()
            frame1.axes.get_xaxis().set_visible(False)
            frame1.axes.get_yaxis().set_visible(False)
            plt.savefig("img_" + gen_mask_file + ".png")

            sys.exit()


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
        # utils.keyboard()
        last_data_idx = get_last_idx(this_packet)

        if last_data_idx != None and last_data_idx <= 5:
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

# root = Tkinter.Tk()
# label_image = Tkinter.Label(root)
# label_image.place(x=0,y=0,width=112,height=112)
# root.geometry('+%d+%d' % (112,112))
# root.after(0, main)
# root.mainloop()