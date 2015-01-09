import argparse
import sys
from os import listdir, mkdir
from os.path import isfile, join
import numpy as np
import Image
import struct
from copy import deepcopy
import numpy as np

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("image_folder", help="folder with images to be packed")
    parser.add_argument("output_file", help="output file for packed images")

    args = parser.parse_args()

    try:
        images_packed = open(args.output_file, "wb")
    except IOError:
        print "Output file", args.output_file, "could not be opened."
        sys.exit()

    files = sorted([ f for f in listdir(args.image_folder) if isfile(join(args.image_folder,f)) ])

    for f in files:
        if ".png" not in f:
            continue

        print f
        image = Image.open(args.image_folder + "/" + f)
        image = np.array(image)
        
        for line in image:
            for pixel in line:
                images_packed.write(struct.pack('h',pixel))

    images_packed.close()

main()
