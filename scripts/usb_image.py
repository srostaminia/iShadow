import usb.control, usb.core
import sys
import argparse
import time
import numpy as np
import pylab
import struct
import pickle
from PIL import Image, ImageTk
import os
import matplotlib.pyplot as plt
import matplotlib.image as mpimg
import Tkinter

def main():
    parser = argparse.ArgumentParser()

    parser.add_argument("file_prefix", help="output file prefix")
    parser.add_argument("mask", help="camera mask")

    args = parser.parse_args()
    file_prefix = args.file_prefix
    mask_filename = args.mask

    mask = load_mask(mask_filename)

    endp = get_usb_endp()

    while True:
        try:
            output = open(file_prefix + ".raw", "wb")
        except IOError:
            print "Output file", file_prefix + ".raw", "could not be opened."
            sys.exit()

        pixels = 0
        data_started = 0
        packets = 0
        while packets < 14:
            data = endp.read(1840)

            if (get_first(data) != -1) and (data_started == 0):
                data_started = 1
            elif (get_first(data) == -1) and (data_started == 1):
                break

            if (data_started == 1):
                packets += 1

            unpacked = struct.unpack('H' * 920, data)

            valid_bytes = get_valid_bytes(unpacked)
            pixels += len(valid_bytes)
            # print len(valid_bytes), "\n", valid_bytes, "\n"
            valid_packed = struct.pack('H' * len(valid_bytes), *valid_bytes)
            output.write(valid_packed)

        # print "Pixels:", pixels
        # print "Packets:", packets, "\n"

        output.close()

        try:
            output = open(file_prefix + ".raw", "rb")
        except IOError:
            print "Intermediate file", file_prefix + ".raw", "could not be opened."
            sys.exit()

        disp_save_images(output, mask, file_prefix)

        os.remove(output.name)

        output.close()


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

def disp_save_images(image_file, mask_data, out_filename):
    images = read_all_packed_images(image_file)

    if len(images) == 0:
        print "ERROR: No full images found in", image_file.name
        sys.exit()

    img = pylab.figure()
    for i, image in enumerate(images):
        image -= mask_data

        # print image

        pylab.figimage(image, cmap = pylab.cm.Greys_r)

        img.set_size_inches(1, 1)

        if (len(images) == 1):
            pylab.savefig(out_filename + ".png", dpi=112)
        else:
            pylab.savefig(out_filename + str(i) + ".png", dpi=112)

        # image1 = Image.fromarray(image.tolist())
        image1 = Image.open(out_filename + ".png")
        # image1 = Image.open("refresh.png")
        # root.title("refresh")
        root.geometry('%dx%d' % (image1.size[0],image1.size[1]))
        
        tkpi = ImageTk.PhotoImage(image1)
        label_image.configure(image = tkpi)
        root.update()

        # img_in = open(out_filename + ".png", 'rb')
        # disp_img = Image.open(img_in)
        # disp_img.show()
        # img_in.close()
        # print "Finished"

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

# main()

root = Tkinter.Tk()
label_image = Tkinter.Label(root)
label_image.place(x=0,y=0,width=112,height=112)
root.geometry('+%d+%d' % (112,112))
root.after(0, main)
root.mainloop()