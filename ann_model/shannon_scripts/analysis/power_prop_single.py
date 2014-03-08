import matplotlib.pyplot as plt
import matplotlib as mpl
from matplotlib import rc
import numpy as np
from copy import deepcopy
import os, sys, csv, math

lambda_val = np.array([0.0001, 0.0002154434690031882, 0.0004641588833612777, 0.001, 0.002154434690031882, 0.004641588833612777, \
	0.01, 0.02154434690031882, 0.04641588833612777, 0.1])

avg_pixels = [12432, 12431, 12385, 11822, 9749, 5792, 2780, 876, 213, 185]

latency = np.array([373, 373, 355, 339, 280, 164, 81, 27, 9, 8])

mcu_current_active = 15.3; # mA
mcu_current_sleep = 6.9;

camera_current_active = 0.949;
camera_current_sleep = 0.000;

def main():
	rc('font',**{'family':'sans-serif','sans-serif':['Helvetica'],'size':25})
	rc('text', usetex=True)
	mpl.rcParams['figure.subplot.bottom'] = 0.15
	mpl.rcParams['lines.linewidth'] = 4
	mpl.rcParams['lines.markersize'] = 10

	in_file = open('shannon_single_avg.csv','rU')
	reader = csv.reader(in_file)

	line = 1
	all_pixel_graphs = []
	all_pixel_stderr = []

	all_pred_graphs = []
	all_pred_stderr = []
	for i in range(7):
		num_pixels = []
		stderr_pixels = []

		pred_error = []
		stderr_error = []

		for l in range(10):
			row = reader.next()
			name = row[1]
			print row[0], row[1], row[2]

			check = [row[0], row[1], row[2]]

			pix_values = [float(row[3])]
			err_values = [float(row[5])]

			line += 1

			for j in range(9):
				row = reader.next()

				pix_values.append(float(row[3]))
				err_values.append(float(row[5]))

				for k in range(1,3):
					if check[k] != row[k]:
						print "ERROR:", check[k], row[k], line

				line += 1

			num_pixels.append(np.mean(pix_values))
			stderr_pixels.append(np.std(pix_values) / math.sqrt(10) * 1.64)

			pred_error.append(np.mean(err_values))
			stderr_error.append(np.std(err_values) / math.sqrt(10) * 1.64)

		size_axis = (np.array(num_pixels) / float(12432)) * 100
		size_axis_stderr = np.array(stderr_pixels) * 100 / float(12432)
		pred_axis = np.array(pred_error) * 0.32
		pred_axis_stderr = np.array(stderr_error) * 0.32

		# size_axis = num_pixels
		# size_axis_stderr = stderr_pixels
		# pred_axis = pred_error
		# pred_axis_stderr = stderr_error

		all_pixel_graphs.append(size_axis)
		all_pixel_stderr.append(size_axis_stderr)
		all_pred_graphs.append(pred_axis)
		all_pred_stderr.append(pred_axis_stderr)

		if 'full' in name:
			speed_axis = 1 / (latency / float(1000))

			power_indices = range(5,10)
			print latency[power_indices]
			power_axis = ((200 - latency[power_indices]) / float(200)) * (mcu_current_active + 2 * camera_current_active) + (mcu_current_sleep + camera_current_sleep)
			power_axis *= 3.3;

			print 'accuracy_speed_' + name + '.pdf'
			plt.figure(1)
			# plt.errorbar(pred_axis, speed_axis, marker='o', color='b', ecolor='k', mew=0.8, yerr=pred_axis_stderr)
			plt.semilogy(pred_axis[1:-2], speed_axis[1:-2], marker='o')
			plt.xlabel('Prediction Error (degrees)')
			plt.ylabel('Framerate (Hz)')
			# plt.title('Accuracy vs. Framerate')
			plt.ylim([1, 140])
			# plt.xlim([0, 105])
			plt.savefig('accuracy_speed_' + name + '.pdf')
			plt.close()

			print 'size_accuracy_' + name + '.pdf'
			plt.figure(1)
			plt.errorbar(size_axis, pred_axis, marker='o', color='b', ecolor='k', capsize=2, yerr=pred_axis_stderr)
			plt.xlabel('Percent Active Pixels')
			plt.ylabel('Prediction Error (degrees)')
			# plt.title('Model Size vs. Accuracy')
			plt.ylim([0, 10])
			plt.xlim([0, 105])
			plt.savefig('size_accuracy_' + name + '.pdf')
			plt.close()

			print 'lambda_size_' + name + '.pdf'
			plt.figure(1)
			plt.errorbar(lambda_val, size_axis, marker='o', color='b', ecolor='k', capsize=2, yerr=size_axis_stderr)
			plt.xlabel('Regularization')
			plt.ylabel('Percent Active Pixels')
			# plt.title('Regularization vs. Model Size')
			plt.xlim([0.00009, 0.11])
			ax = plt.gca()
			ax.set_xscale("log")
			ax.invert_xaxis()
			plt.ylim([0, 105])
			plt.savefig('lambda_size_' + name + '.pdf')
			plt.close()

			print 'lambda_power_' + name + '.pdf'
			plt.figure(1)
			plt.plot(pred_axis[power_indices], power_axis, marker='o')
			plt.xlabel('Prediction Error (degrees)')
			plt.ylabel('Average Instantaneous Power (mW)')
			# plt.title('Regularization vs. Power')
			# plt.xlim([0.00009, 0.11])
			# ax = plt.gca()
			# ax.set_xscale("log")
			# ax.invert_xaxis()
			# plt.ylim([0, 105])
			plt.savefig('accuracy_power_' + name + '.pdf')
			plt.close()

			print 'lambda_accuracy_' + name + '.pdf'
			plt.figure(1)
			plt.errorbar(lambda_val, pred_axis, marker='o', color='b', ecolor='k', capsize=2, yerr=pred_axis_stderr)
			plt.xlabel('Regularization')
			plt.ylabel('Prediction Error (degrees)')
			# plt.title('Regularization vs. Accuracy - ' + name[0:2])
			plt.ylim([0, 10])
			plt.xlim([0.00009, 0.11])
			ax = plt.gca()
			ax.set_xscale("log")
			ax.invert_xaxis()
			plt.savefig('lambda_accuracy_full.pdf')
			plt.close()

	print 'lambda_accuracy_all.pdf'
	plt.figure(1)
	colors = ['r', 'b', 'g', 'k', 'c', 'y', 'm']
	for i in range(7):
		plt.semilogx(lambda_val, np.array(all_pred_graphs[i]), marker='o', color=colors[i])
	plt.legend(['15s', '1m', '2m', '30s', '3m', '5m', 'full'], loc='upper right')
	plt.xlabel('Regularization')
	plt.ylabel('Prediction Error (degrees)')
	plt.ylim([0, 10])
	# plt.title('Regularization vs. Accuracy')
	ax = plt.gca()
	ax.invert_xaxis()
	plt.savefig('lambda_accuracy_all.pdf')
	plt.close()

	temp = all_pred_graphs[3]
	del all_pred_graphs[3]
	all_pred_graphs.insert(1,temp)

	temp = all_pred_stderr[3]
	del all_pred_stderr[3]
	all_pred_stderr.insert(1,temp)

	train_time = np.array([0.25, 0.5, 1, 2, 3, 5]) * 60

	print 'traintime_accuracy.pdf'
	plt.figure(1)
	all_pred_graphs = np.array(all_pred_graphs)
	all_pred_stderr = np.array(all_pred_stderr)
	# plt.errorbar(range(1,7), all_pred_graphs[:,0], marker='o', color='b', ecolor='k', mew=0.8, yerr=all_pred_stderr[:,0])
	plt.errorbar(train_time, all_pred_graphs[0:-1,4], marker='o', color='b', ecolor='k', capsize=2, yerr=all_pred_stderr[0:-1,4])
	# plt.errorbar(range(1,7), all_pred_graphs[:,8], marker='o', color='b', ecolor='k', mew=0.8, yerr=all_pred_stderr[:,8])
	plt.xlabel('Training Time (s)')
	plt.ylabel('Prediction Error (degrees)')
	plt.ylim([1.5, 6])
	ax = plt.gca()
	ax.set_xticks([30, 60, 90, 120, 150, 180, 210, 240, 270, 300])
	plt.xlim([0,310])
	# plt.title('Training Time vs. Accuracy')
	plt.savefig('traintime_accuracy.pdf')
	plt.close()

	# plt.semilogx(num_pixels, pred_error * 0.32, marker='o')
	# plt.xlim([20, 13000])

	# plt.show()

main()