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
    parser.add_argument("--gen_mask", help="generate new camera mask with name")

    args = parser.parse_args()
    debug_folder = args.debug_folder
    mask_filename = args.mask
    gen_mask_file = args.gen_mask

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

    endp = get_usb_endp()

    #imfig = pylab.figure()

    iters = 0
    save_filename = None
    pred = [None, None]

    frame = np.zeros((112*112,))
    fig = plt.figure(999) 
    image=plt.imshow(np.zeros((112,112)), cmap = pylab.cm.Greys_r, interpolation='nearest')

    ax=fig.add_subplot(111)
    ax.set_xlim([0, 112])
    ax.set_ylim([112, 0])

    if (debug == False):
        vline,=ax.plot([0, 1], [0, 1], 'r-', linewidth=2)
        hline,=ax.plot([0, 1], [0, 1], 'r-', linewidth=2)

    while True:

        # try:
        #     output = open(debug_folder + ".raw", "wb")
        # except IOError:
        #     print "Output file", debug_folder + ".raw", "could not be opened."
        #     sys.exit()

        frame = np.reshape(frame, (112*112))

        pixels = 0
        data_started = 0
        packets = 0
        # while packets < 14 or pixels < (112 * 112):
        while pixels < (112 * 112):
            data = endp.read(1840)
            # print data

            if (get_first(data) != -1) and (data_started == 0):
                data_started = 1
            elif (get_first(data) == -1) and (data_started == 1):
                break

            if (data_started == 1):
                packets += 1
            else:
                continue

            if TX_BITS == 16:
                # For 16-bit transmission
                unpacked = struct.unpack('H' * 920, data)
            elif TX_BITS == 8:
                # For 8-bit transmission
                unpacked = struct.unpack('B' * 1840, data)

            if (debug):
                if TX_BITS == 8:
                    print_packets(unpacked, 184)
                elif TX_BITS == 16:
                    print_packets(unpacked, 92)

            valid_bytes = get_valid_bytes(unpacked)

            if (packets == 1):
                pred[0] = 112 - struct.unpack('h', struct.pack('H', valid_bytes.pop(0)))[0]
                pred[1] = struct.unpack('h', struct.pack('H', valid_bytes.pop(0)))[0]

                if (debug and iters == 1):
                    print "Prediction (X, Y):", pred[0], pred[1], "\n"
                    sys.exit()

            if (iters != 0) and (debug == 0):
                vline.set_data([pred[0], pred[0]], [max(0, pred[1] - 10), min(111, pred[1] + 10)])
                hline.set_data([max(0, pred[0] - 10), min(111, pred[0] + 10)], [pred[1], pred[1]])

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
                print "Pixels in packet:", new_pixels, "\n\n"

            # print len(valid_bytes), "\n", valid_bytes, "\n"
            #valid_packed = struct.pack('H' * len(valid_bytes), *valid_bytes)
            #output.write(valid_packed)


        print "Pixels:", pixels
        print "Packets:", packets
        print "Prediction (X, Y):", pred[0], pred[1], "\n"

        # TODO: Fix this so we're not copying the entire image every time...
        frame=np.reshape(frame,(112,112))   
        frame = 255 - frame
        
        # mask = np.right_shift(mask, 2)
        # mask = np.bitwise_and(mask, 0xFF)

        # if (debug == 0):
        if (not gen_mask and mask != None):
            frame -= mask #************************UNCOMMENT ME ***************

        if (not gen_mask):
            frame = np.fliplr(frame)

        if (CONTRAST_ADJUST == 1):
            frame -= np.mean(frame)
            frame /= float(np.std(frame))

        #plt.imshow(frame2, cmap = pylab.cm.Greys_r)
        image.set_data(frame)
        image.autoscale() 

        plt.draw()

        iters += 1

        # outfile = open("usb_frame.pi",'w')
        # pickle.dump(frame, outfile)
        # outfile.close()

        # endp.read(1840)

        if (debug):        
            out_text = open(debug_folder + "/usb_frame.txt",'w')
            for line in frame:
                for item in line:
                    out_text.write(str(item) + " ")
                out_text.write('\n')
            out_text.close()
            
            frame1 = plt.gca()
            frame1.axes.get_xaxis().set_visible(False)
            frame1.axes.get_yaxis().set_visible(False)
            plt.savefig(debug_folder + "/" + debug_folder + ("_%06d" % (iters)) + ".png")
            # sys.exit()

        if (gen_mask):
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
            plt.savefig(gen_mask_file + ".png")

            sys.exit()


def print_packets(unpacked, packet_size):
    if (len(unpacked) % packet_size != 0):
        print "ERROR: Received length is not a multiple of specified packet size"
        sys.exit()

    for i in range(len(unpacked) / packet_size):
        print unpacked[(i * packet_size):((i + 1) * packet_size)]

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

    return -1

def count_valid_bytes(data):
    count = 0
    for item in data:
        if item != 0:
            count += 1

    return count

def get_valid_bytes(data):
    valid = []
    for item in data:
        if item != 0:
            valid.append(item)

    return valid

def count_em_up(data):
    current_num = -1

    results = {}
    for item in data:
        if item != current_num:
            results[item] = 1
            current_num = item
        else:
            results[current_num] += 1

    return results

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