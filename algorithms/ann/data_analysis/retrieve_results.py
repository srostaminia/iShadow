from subprocess import call
import matplotlib.pyplot as plt
import matplotlib as mpl
from matplotlib import rc
import numpy as np
from copy import deepcopy
import os, sys, csv, math
import utils
import cPickle as pickle

rc('font',**{'family':'sans-serif','sans-serif':['Helvetica'],'size':25})
rc('text', usetex=True)
mpl.rcParams['figure.subplot.bottom'] = 0.15
mpl.rcParams['lines.linewidth'] = 4
mpl.rcParams['lines.markersize'] = 10

lambda_prefix = ['subset_l1_init_strips_k', '_lambda']

lambda_values = ['0.000100', '0.000215', '0.000464', '0.001000', '0.002154', '0.004642', '0.010000', '0.021544', '0.046416', '0.100000']

# lambda_values = ['0.001000', '0.010000', '0.100000']

# names = ['addison2', 'dan', 'james', 'john', 'keith', 'luis', 'michelle', 'niri', 'seth', 'steve' ]
names = ['addison_pupilclean']

# h_structures = [[2, 3], [2, 4], [2, 5], [3, 4], [2, 7], [4, 4], [3, 6], [4, 5], [2, 11], [4, 6]]
h_structures = [[2, 3]]

# if os.path.exists('htest_small_results.csv'):
# 	print "ERROR: htest_small_results.csv exists"
# 	sys.exit()

out_prefix = 'pupilclean_mu5_500x'

start_path = '/Users/ammayber/senseye/ann_model/awesomeness_pupil_modstrip_uniquefy05_500x/'

output_file = open('data/' + out_prefix + '.csv','w')
writer = csv.writer(output_file)

writer.writerow(['name', 'struct_name', 'lambda', 'mean_pixels', 'stderr_pixels', 'mean_error', 'stderr_error', 'var_error'])

all_pixel_means = []
all_pixel_stderrs = []
all_pixel_vars = []
all_error_means = []
all_error_stderrs = []
all_error_vars = []

k = 1
for name in names:
	for structure in h_structures:
		struct_name = str(structure[0]) + '_' + str(structure[1])
		k_val = (structure[0] * structure[1]) + 1

		npath = start_path + name + '/full/'

		fpath = npath + '/'

		for l_val in lambda_values:
			lam_folder = lambda_prefix[0] + str(k_val) + lambda_prefix[1] + l_val
			lpath = fpath + 'results/' + lam_folder + '/'

			avg_error = []
			avg_pixels = []
			for i in range(1,6):
				mat_filename = lpath + 'rep' + str(i) + '.mat'

				try:
					mat_file = open(mat_filename, 'r')
				except:
					continue

				print str(k) + ': ' + mat_filename

				while True:
					line = mat_file.readline()

					if "ErrTest" in line:
						mat_file.readline()
						pred_err = float(mat_file.readline().strip())
						avg_error.append(pred_err)
						print pred_err
						break
					elif not line:
						print "ERROR: Count not find ErrTest in", mat_filename
						sys.exit()

				while True:
					line = mat_file.readline()

					if "index_value" in line:
						mat_file.readline()
						num_pixels = int(mat_file.readline().strip().split()[2]) - 1
						avg_pixels.append(num_pixels)
						break
					elif not line:
						num_pixels = 0
						break

				mat_file.close()

				k += 1

			avg_pixels = np.array(avg_pixels)
			avg_error = np.array(avg_error)

			mean_pixels = np.mean(avg_pixels)
			mean_error = np.mean(avg_error)

			stderr_pixels = np.std(avg_pixels) / math.sqrt(5) * 1.64
			stderr_error = np.std(avg_error) / math.sqrt(5) * 1.64

			var_pixels = np.var(avg_pixels)
			var_error = np.var(avg_error)

			writer.writerow([name, struct_name, lam_folder[-8:], mean_pixels, stderr_pixels, mean_error, stderr_error, var_error])

			all_pixel_means.append(mean_pixels)
			all_error_means.append(mean_error)

			all_pixel_stderrs.append(stderr_pixels)
			all_error_stderrs.append(stderr_error)

			all_pixel_vars.append(var_pixels)
			all_error_vars.append(var_error)

output_file.close()

all_pixel_means = np.array(all_pixel_means)
all_pixel_stderrs = np.array(all_pixel_stderrs)
all_pixel_vars = np.array(all_pixel_vars)
all_error_means = np.array(all_error_means)
all_error_stderrs = np.array(all_error_stderrs)
all_error_vars = np.array(all_error_vars)

save_file = open('data/' + out_prefix + ".pi", 'w')

pickle.dump(all_pixel_means, save_file)
pickle.dump(all_pixel_stderrs, save_file)
pickle.dump(all_pixel_vars, save_file)

pickle.dump(all_error_means, save_file)
pickle.dump(all_error_stderrs, save_file)
pickle.dump(all_error_vars, save_file)

save_file.close()

# print 'htest_variance_lambda.pdf'
# plt.figure(1)
# h_sizes = [1, 2, 4, 6, 12, 16]
# plt.errorbar(h_sizes, all_error_means*0.32, marker='o', color='b', ecolor='k', capsize=2, yerr=all_error_stderrs*0.32)
# # p3, = plt.plot(pred_axis[power_indices], acq_time + pred_time, marker='o', color='b', linestyle='--')	
# # plt.legend([p1, p2, p3], ["Acquisition", "Prediction", "Camera"])
# # plt.legend([p1, p2], ["Acquisition", "Prediction"])
# plt.xlabel('\# Hidden Units')
# plt.ylabel('Prediction Error (Degrees)')
# plt.ylim([0,9])
# plt.xlim([0,18])
# plt.savefig('htest_variance_lambda.pdf')
# plt.close()

# print 'htest_small_zoomed.pdf'
# plt.figure(1)
# h_sizes = [1, 2, 4, 6, 12, 16]
# plt.errorbar(h_sizes[1:], all_error_means[1:]*0.32, marker='o', color='b', ecolor='k', capsize=2, yerr=all_error_stderrs[1:]*0.32)
# # p3, = plt.plot(pred_axis[power_indices], acq_time + pred_time, marker='o', color='b', linestyle='--')	
# # plt.legend([p1, p2, p3], ["Acquisition", "Prediction", "Camera"])
# # plt.legend([p1, p2], ["Acquisition", "Prediction"])
# plt.xlabel('\# Hidden Units')
# plt.ylabel('Prediction Error (Degrees)')
# plt.grid()
# plt.ylim([0,2])
# plt.xlim([0,18])
# plt.savefig('htest_small_zoomed.pdf')
# plt.close()