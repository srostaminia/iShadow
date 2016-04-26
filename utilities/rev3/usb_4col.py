import usb.control, usb.core
import sys
import argparse
import numpy as np
import struct
import pickle
import os
import shutil
import matplotlib.pyplot as plt
import matplotlib


CONTRAST_ADJUST = 0
TX_BITS = 8
PARAM_PACKET_SIZE = 10
plt.ion()
def main():

    # These commands should be written in bash!
    # Create Parser
    parser = argparse.ArgumentParser()
    # Build different args in parser
    # -- this makes the argument optional, if you want to make a required argument you should remove --
    # --debug : writing this in bash means you want to save one image in USB mode.
    parser.add_argument("--debug_folder", help="debug mode output file prefix")
    # with this arg u can use your mask here in stead of in c code!
    parser.add_argument("--mask", help="camera mask")

    group = parser.add_mutually_exclusive_group()
    group.add_argument("--gen_mask", help="generate new camera mask with name")
    group.add_argument("--noflip", help="don't flip image (debug option)", action='store_true')

    parser.add_argument("--columnwise", help="images being sent in column-major order", action="store_true")

    args = parser.parse_args()
    debug_folder = args.debug_folder
    mask_filename = args.mask
    gen_mask_file = args.gen_mask
    columnwise = args.columnwise
    noflip = args.noflip

    # If We want to save an image, these following lines make an folder in our defined directory
    if (debug_folder != None):
        # If a file with the same name exists in the directory, we replace it with newer file
        if os.path.isdir(debug_folder):
            shutil.rmtree(debug_folder)

        os.mkdir(debug_folder)
        # set debug to true. this means that we want to save an frame
        debug = True
    else:
        debug = False

    # Loading mask file
    if (mask_filename == None):
        mask = None
    else:
        mask = load_mask(mask_filename)

    if gen_mask_file == None:
        gen_mask = False
    else:
        gen_mask = True

    # Connecting USB to Mac
    endp = get_usb_endp()
    # This paramete increment at the end of each frame (literally counts the number of frames)
    iters = 0
    next_start = []

    frame = np.zeros((112*4,))
    fig = plt.figure(999)
    image=plt.imshow(np.zeros((112,4)), cmap = matplotlib.cm.Greys_r, interpolation='nearest')
    ax=fig.add_subplot(111)
    ax.set_xlim([0, 4])
    ax.set_ylim([112, 0])

    # packet_size = 184. Number of bytes in each packet
    packet_size = 92 * 16 / TX_BITS


    # Main Loop
    while True:

        frame = np.reshape(frame, (112*4))

        # debug = 1 : Save one frame. In this situation we save two frames (iter = 0 and 1)
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
            # ????????
            packets = 0

        rx_complete = False
        while not rx_complete:

            # Read 10 packets of data
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
                # unpacked is a array of 10 packets of data (10 * 184)
                unpacked = struct.unpack('B' * 1840, data)

            if (debug):
                print_packets(unpacked, packet_size)

            param_idx = check_param_packet(unpacked, packet_size)

            # Parameter packet is the end of an image transmission
            if param_idx != None:
                next_start = unpacked[(param_idx + 1) * packet_size:]
                #param_data = unpacked[param_idx * packet_size:(param_idx * packet_size) + PARAM_PACKET_SIZE]
                unpacked = unpacked[:param_idx * packet_size]
                rx_complete = True
            # valid bytes probably has 10 * 112 values (unpacked has 10 * 184)
            valid_bytes = extract_pixel_data(unpacked, packet_size)
            # Conversion tuple to array
            valid_bytes = np.array(valid_bytes)
            new_pixels = len(valid_bytes)
            try:
                frame[pixels:(pixels+new_pixels)] = valid_bytes
            except:
                frame[pixels:] = valid_bytes[:(448-pixels)]

            pixels += new_pixels

            if (debug):
                print "Pixels in packet:", new_pixels
                print "Total pixels:", pixels, "\n\n"

        frame=np.reshape(frame,(112,4))


        print "Pixels:", pixels
        print "Packets:", packets


        if (not gen_mask and mask != None):
            frame -= mask
        # Adjust the contrast of the Image
        if (CONTRAST_ADJUST == 1):
            frame -= np.mean(frame)
            frame /= float(np.std(frame))

        if columnwise:
            frame = frame.T

        if not gen_mask and not noflip and not debug:
            frame = np.fliplr(frame)

        image.set_data(frame)
        image.autoscale()

        fig.canvas.draw()
        fig.canvas.flush_events()

        iters += 1

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

def Detect_End_of_Data(packet):
    count = 0
    for i in packet:
        if i == 1:
            count += 1

    if count == len(packet):
        return True
    else:
        return False

def check_param_packet(data, packet_size):
    for i in range(len(data) / packet_size):
        this_packet = data[i * packet_size:(i+1) * packet_size]
        # last_data_idx = get_last_idx(this_packet)
        #                     # why last_data_idx < PARAM_PACKET_SIZE?
        # if last_data_idx != None and last_data_idx < PARAM_PACKET_SIZE:
        #     return i
        if (Detect_End_of_Data(this_packet)):
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
            valid.extend(this_packet[:4])

    return valid

#This function is the connection between ishadow code and mac
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

#
# def get_nonzero_pixels(data):
#     nzp = []
#     for item in data:
#         if item != 0:
#             nzp.append(item)
#
#     return nzp

# def get_zero_stop(data):
#     for i, item in enumerate(data):
#         if item != 0:
#             return i
#
#     return 0

# def get_zero_start(data):
#     for i, item in enumerate(data):
#         if item == 0:
#             return i
#
#     return 0

# def parse_param_fields(param_data, noflip):
#     param_fields = ParamFields()
#
#     if TX_BITS == 8:
#         data_type = 'B'
#     elif TX_BITS == 16:
#         data_type = 'H'
#
#     param_fields.model_type = struct.unpack('h', struct.pack('H', param_data[4]))[0]
#
#
#     if param_fields.model_type == 1 or param_fields.model_type == 2:
#         param_fields.cider_row = struct.unpack('h', struct.pack('H', param_data[8]))[0]
#         param_fields.cider_radius = struct.unpack('h', struct.pack('H', param_data[9]))[0]
#
#         if noflip:
#             param_fields.cider_col = struct.unpack('h', struct.pack('H', param_data[7]))[0]
#         else:
#             param_fields.cider_col = 112 - struct.unpack('h', struct.pack('H', param_data[7]))[0]
#     else:
#         param_fields.cider_row = -10
#         param_fields.cider_col = -10
#         param_fields.cider_radius = 0
#
#     return param_fields

# def read_packed_image(image_file):
#     image = []
#     for i in range(112):
#         image.append([])
#
#         for j in range(112):
#             data = image_file.read(2)
#
#             if data == "":
#                 return []
#
#             value = struct.unpack('h', data)[0]
#
#             image[i].append(value)
#
#     return np.array(image)

# def read_all_packed_images(image_file):
#     images = []
#
#     current_image = read_packed_image(image_file)
#     while (len(current_image) != 0):
#         images.append(current_image)
#         current_image = read_packed_image(image_file)
#
#     return images

main()
