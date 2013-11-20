import argparse
import sys
from os import listdir, mkdir
from os.path import isfile, join
import numpy as np
import Image
import struct
from copy import deepcopy
import scipy.stats.mstats as mstats

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("image_file", help="image file to process")
    parser.add_argument("mask_file", help="mask file to process")
    parser.add_argument("output_folder", help="output folder for images")
    
    args = parser.parse_args()
        
    try:
        images_packed = open(args.image_file, "rb")
    except IOError:
        print "Input file", args.image_file, "could not be opened."
        sys.exit()
        
    try:
        mask_file = open(args.mask_file, "rb")
    except IOError:
        print "Mask file", args.mask_file, "could not be opened."
        sys.exit()
    
    mask = read_packed_image(mask_file)
    mask -= np.amin(mask)
    mask_file.close()
    
    images = read_all_packed_images(images_packed)
    images_packed.close()
    
    img = Image.fromarray(mask.astype('uint8'), 'L')
    img.save(args.output_folder + "/" + "mask.png")

    for i, img in enumerate(images):
        img -= mask
        temp = Image.fromarray(img.astype('uint8'), 'L')
        temp.save(args.output_folder + "/" + "img_" + str(i) + ".png")
        
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
