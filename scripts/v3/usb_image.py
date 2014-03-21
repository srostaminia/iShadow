import usb.control, usb.core
import sys
import argparse
import time
import numpy as np
import pylab
import struct
import pickle
from PIL import Image, ImageTk, ImageDraw, ImageColor
import os
import matplotlib.pyplot as plt
import matplotlib.image as mpimg
import Tkinter
import shutil
import utils

plt.ion()

def main():
    parser = argparse.ArgumentParser()

    parser.add_argument("file_prefix", help="output file prefix")
    parser.add_argument("mask", help="camera mask")

    args = parser.parse_args()
    file_prefix = args.file_prefix
    mask_filename = args.mask

    # if os.path.isdir(file_prefix):
    #     erase = 'x'
    #     while (erase.lower() != 'y' and erase.lower() != 'n'):
    #         erase = raw_input("Data folder " + file_prefix + " already exists. Erase old data and proceed (y/n)?")

    #     if erase == 'n':
    #         print "\nHalting execution."
    #         sys.exit()

    shutil.rmtree(file_prefix)

    os.mkdir(file_prefix)

    mask = load_mask(mask_filename)

    endp = get_usb_endp()

    #imfig = pylab.figure()

    iter_num = 0
    save_filename = None
    pred = [None, None]

    frame = np.zeros((112*112,))
    plt.figure(999) 
    image=plt.imshow(np.zeros((112,112)), cmap = pylab.cm.Greys_r, interpolation='nearest')

    while True:

        # try:
        #     output = open(file_prefix + ".raw", "wb")
        # except IOError:
        #     print "Output file", file_prefix + ".raw", "could not be opened."
        #     sys.exit()

        pixels = 0
        data_started = 0
        packets = 0
        while packets < 14 or pixels < (112 * 112):
            data = endp.read(1840)

            if (get_first(data) != -1) and (data_started == 0):
                data_started = 1
            elif (get_first(data) == -1) and (data_started == 1):
                break

            if (data_started == 1):
                packets += 1
            else:
                continue

            unpacked = struct.unpack('H' * 920, data)

            # print unpacked, "\n"

            valid_bytes = get_valid_bytes(unpacked)

            if (packets == 1):
                pred[0] = valid_bytes.pop(0)
                pred[1] = valid_bytes.pop(0)

            valid_bytes = np.array(valid_bytes)
            new_pixels = len(valid_bytes)
            try:
                frame[pixels:(pixels+new_pixels)] = valid_bytes
            except:
                utils.keyboard()
                
            pixels += new_pixels
            # print len(valid_bytes), "\n", valid_bytes, "\n"
            #valid_packed = struct.pack('H' * len(valid_bytes), *valid_bytes)
            #output.write(valid_packed)


        print "Pixels:", pixels
        print "Packets:", packets
        print "Prediction (X, Y):", pred[0], pred[1], "\n"

        frame2=np.reshape(frame,(112,112))   
        frame2 -= mask
        frame2 = np.fliplr(frame2)
        #plt.imshow(frame2, cmap = pylab.cm.Greys_r)
        image.set_data(frame2)
        image.autoscale()
        plt.draw()

        #utils.keyboard()

        # output.close()

        # if (iter_num != 0):
        #     # Reopen image
        #     image1 = Image.open(save_filename)
        #     root.geometry('%dx%d' % (image1.size[0],image1.size[1]))
            
        #     draw = ImageDraw.Draw(image1)

        #     draw.line([(pred[0], max(0, pred[1] - 10)), (pred[0], min(111, pred[1] + 10))], fill=ImageColor.getrgb('red'))
        #     draw.line([(max(0, pred[0] - 10), pred[1]), (min(111, pred[0] + 10), pred[1])], fill=ImageColor.getrgb('red'))

        #     del draw

        #     tkpi = ImageTk.PhotoImage(image1)
        #     label_image.configure(image = tkpi)
        #     root.update()
        #     image1.save(save_filename)

        # data = endp.read(1840)
        # while (get_first(data) == -1):
        #     data = endp.read(1840)

        # print data

        # pred = get_valid_bytes(struct.unpack('H' * 920, data))
        # print pred

        # sys.exit()

        # try:
        #     output = open(file_prefix + ".raw", "rb")
        # except IOError:
        #     print "Intermediate file", file_prefix + ".raw", "could not be opened."
        #     sys.exit()

        #save_filename = disp_save_images(output, mask, file_prefix, iter_num, imfig)

        # os.remove(output.name)

        #output.close()

        iter_num += 1


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

def disp_save_images(image_file, mask_data, out_filename, iter_num, figure):
    images = read_all_packed_images(image_file)

    if len(images) == 0:
        print "ERROR: No full images found in", image_file.name
        sys.exit()

    # img = pylab.figure()
    for i, image in enumerate(images):
        image -= mask_data
        image = np.fliplr(image)

        # print image   # Debug

        pylab.figimage(image, cmap = pylab.cm.Greys_r)

        figure.set_size_inches(1, 1)

        save_filename = out_filename + "/" + out_filename + ("%06d" % (iter_num)) + ".png"

        # Save image
        if (len(images) == 1):
            pylab.savefig(save_filename, dpi=112)
        else:
            pylab.savefig(out_filename + str(i) + ".png", dpi=112)

        return save_filename

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