import struct
import numpy as np
import pickle
import pylab
import argparse

def main():
	parser = argparse.ArgumentParser()

	parser.add_argument("input_file", help="raw data file")
	parser.add_argument("file_prefix", help="output file prefix")
	parser.add_argument("mask", help="camera mask")
	parser.add_argument("num_images", type=int, help="number of images to read out")

	args = parser.parse_args()

	image_data = open(args.input_file)
	mask = np.array(load_mask(args.mask))
	mask = mask[1:]
	label = args.file_prefix
	
	if args.num_images == 0:
		total = 0
	else:
		total = args.num_images

	first = True
	i = 0
	while True:
	# for i in range(total):
		out_file = open(label + "_" + ("%06d" % i) + ".txt", 'w')

		data = np.array(struct.unpack('H' * (112 * 112), image_data.read(25088))).reshape((112, 112))
		data = data[1:]

		if first:
			print len(data)
			first = False

		data -= mask

		for line in data:
			for item in line:
				out_file.write(str(item) + ' ')
			out_file.write('\n')

		out_file.close()

		img = pylab.figure()

		pylab.figimage(data, cmap = pylab.cm.Greys_r)

		img.set_size_inches(1, 1)

		pylab.savefig(label + "_" + ("%06d" % i) + ".png", dpi=112)

		pylab.close()

		if i % 500 == 0:
			print "Completed", i, "images"

		i += 1

		if (args.num_images != 0) and (i > args.num_images):
			break


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

main()