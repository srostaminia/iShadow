import matplotlib.pyplot as plt
import numpy as np
from copy import deepcopy
import os, sys, csv, math

lambda_val = np.array([0.0001, 0.0002154434690031882, 0.0004641588833612777, 0.001, 0.002154434690031882, 0.004641588833612777, \
	0.01, 0.02154434690031882, 0.04641588833612777, 0.1])

def main():
	in_file = open('shannon_holdout_results.csv','rU')
	reader = csv.reader(in_file)

	line = 1
	num_pixels = []
	stderr_pixels = []

	pred_error = []
	stderr_error = []

	for l in range(10):
		row = reader.next()
		name = row[1]
		print row[1]

		pix_values = [float(row[2])]
		err_values = [float(row[3])]

		line += 1

		for j in range(9):
			row = reader.next()

			pix_values.append(float(row[2]))
			err_values.append(float(row[3]))

			if name != row[1]:
				print "ERROR:", name, row[1], line

			line += 1

		num_pixels.append(np.mean(pix_values))
		stderr_pixels.append(np.std(pix_values) / math.sqrt(10) * 1.64)

		pred_error.append(np.mean(err_values))
		stderr_error.append(np.std(err_values) / math.sqrt(10) * 1.64)

		# power_indices = range(4,9)

	size_axis = (np.array(num_pixels) / float(12432)) * 100
	size_axis_stderr = np.array(stderr_pixels) * 100 / float(12432)
	pred_axis = np.array(pred_error) * 0.32
	pred_axis_stderr = np.array(stderr_error) * 0.32

	plt.figure(1)
	plt.errorbar(size_axis, pred_axis, marker='o', color='b', ecolor='k', mew=0.8, yerr=pred_axis_stderr)
	plt.xlabel('Percent Active Pixels')
	plt.ylabel('Prediction Error (degrees)')
	plt.title('Model Size vs. Accuracy - Multi-User Hold-Out')
	# plt.ylim([0, 10])
	plt.xlim([0, 105])
	plt.savefig('size_accuracy_holdout.pdf')
	plt.close()

	plt.figure(1)
	plt.errorbar(lambda_val, size_axis, marker='o', color='b', ecolor='k', mew=0.8, yerr=size_axis_stderr)
	plt.xlabel('Regularization')
	plt.ylabel('Percent Active Pixels')
	plt.title('Regularization vs. Model Size - Multi-User Hold-Out')
	plt.xlim([0.00009, 0.11])
	ax = plt.gca()
	ax.set_xscale("log")
	ax.invert_xaxis()
	# plt.ylim([0, 105])
	plt.savefig('lambda_size_holdout.pdf')
	plt.close()

	plt.figure(1)
	plt.errorbar(lambda_val, pred_axis, marker='o', color='b', ecolor='k', mew=0.8, yerr=pred_axis_stderr)
	plt.xlabel('Regularization')
	plt.ylabel('Prediction Error (degrees)')
	plt.title('Regularization vs. Accuracy - Multi-User Hold-Out')
	# plt.ylim([0, 10])
	plt.xlim([0.00009, 0.11])
	ax = plt.gca()
	ax.set_xscale("log")
	ax.invert_xaxis()
	plt.savefig('lambda_accuracy_holdout.pdf')
	plt.close()

main()