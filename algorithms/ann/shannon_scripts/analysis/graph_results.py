from subprocess import call
import matplotlib.pyplot as plt
import matplotlib as mpl
from matplotlib import rc
import numpy as np
from copy import deepcopy
import os, sys, csv, math
from utils import keyboard
import cPickle as pickle

rc('font',**{'family':'sans-serif','sans-serif':['Helvetica'],'size':25})
rc('text', usetex=True)
mpl.rcParams['figure.subplot.bottom'] = 0.15
mpl.rcParams['lines.linewidth'] = 4
mpl.rcParams['lines.markersize'] = 10

def main():
	pdf_name = "pupil_comp_zoom.pdf"
	unit_pixels = True

	data_names = ["Clean MU5 500x", "Clean MU5 1000x"]
	file_names = ["pupilclean_mu5_500x.pi", "pupilclean_mu5_1000x.pi"]
	# graph_styles = [['g','--'],['r','--'],['g','-'],['r','-']]
	graph_styles = []

	all_pixels = np.array([]);
	all_errors = np.array([]);
	all_stddev = np.array([]);

	for name in file_names:
		res = process_data(name, unit_pixels)

		all_pixels = combine_ndarrays(all_pixels, res[0])
		all_errors = combine_ndarrays(all_errors, res[1])
		all_stddev = combine_ndarrays(all_stddev, res[2])

	print pdf_name
	plt.figure(1)

	p = []
	for i in range(len(all_pixels)):
		if len(graph_styles) > 0:
			(p_temp, temp1, temp2) = plt.errorbar(all_pixels[i], all_errors[i], marker='o', color=graph_styles[i][0], ls=graph_styles[i][1], mew=0.8, yerr=(all_stddev[i]))
		else:
			(p_temp, temp1, temp2) = plt.errorbar(all_pixels[i], all_errors[i], marker='o', ls='--', mew=0.8, yerr=(all_stddev[i]))

		p.append(p_temp)

	plt.legend(p, data_names, loc=1)
	plt.xlabel('Percent Active Pixels')

	if (unit_pixels):
		plt.ylabel('Prediction Error (Pixels)')
	else:
		plt.ylabel('Prediction Error (Degrees)')
	plt.xlim([-5,65])
	plt.ylim([0.5,4])
	plt.savefig(pdf_name)
	plt.close()

def combine_ndarrays(array_base, array_new):
	if len(array_base) == 0:
		return deepcopy(array_new)
	else:
		return np.vstack((array_base, array_new))

def process_data(fname, unit_pixels = False):
	data_file = open(fname, 'r')

	pixel_means = pickle.load(data_file)
	pixel_stderrs = pickle.load(data_file)
	pixel_vars = pickle.load(data_file)

	error_means = pickle.load(data_file)
	error_stderrs = pickle.load(data_file)
	error_vars = pickle.load(data_file)

	pixels = []
	errors = []
	stddev = []

	if (unit_pixels):
		factor = 1
	else:
		factor = 0.32

	for i in range(0, 10):
		pixels.append( (np.mean(pixel_means[i::10]) / float(12432)) * 100)
		errors.append(np.mean(error_means[i::10]) * factor)

		# print error_means[i::10], '\n'

		if len(error_means[i::10]) > 1:
			stddev.append(np.std(error_means[i::10]) * factor)
		else:
			stddev.append(np.sqrt(error_vars[i]))

	print error_means[i::10], '\n'

	return [pixels, errors, stddev]

main()