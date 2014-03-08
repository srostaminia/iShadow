import os, sys, csv
from subprocess import call

names = ['addison2', 'dan', 'james', 'john', 'keith', 'luis', 'niri', 'michelle', 'seth', 'steve']

lambda_folders = ['subset_l1_init_strips_k7_lambda0.000100',  'subset_l1_init_strips_k7_lambda0.000464',  'subset_l1_init_strips_k7_lambda0.002154',  \
'subset_l1_init_strips_k7_lambda0.010000',  'subset_l1_init_strips_k7_lambda0.046416', 'subset_l1_init_strips_k7_lambda0.000215', \
'subset_l1_init_strips_k7_lambda0.001000',  'subset_l1_init_strips_k7_lambda0.004642',  'subset_l1_init_strips_k7_lambda0.021544', \
'subset_l1_init_strips_k7_lambda0.100000']

if os.path.exists('shannon_multi_results.csv'):
	print "ERROR: shannon_multi_results.csv exists"
	sys.exit()

output_file = open('shannon_multi_results.csv','w')
writer = csv.writer(output_file)

k = 1
for name in names:
	npath = '/Users/ammayber/senseye/ann_model/shannon_data/awesomeness/holdout/holdout_' + name + '/'

	for lam_folder in lambda_folders:
		lpath = npath + 'results/' + lam_folder + '/'
		mat_filename = lpath + 'rep1.mat'

		mat_file = open(mat_filename, 'r')

		print str(k) + ': ' + mat_filename

		while True:
			line = mat_file.readline()

			if "ErrTest" in line:
				mat_file.readline()
				pred_err = float(mat_file.readline().strip())
				break
			elif not line:
				print "ERROR: Count not find ErrTest in", mat_filename
				sys.exit()

		while True:
			line = mat_file.readline()

			if "index_value" in line:
				mat_file.readline()
				num_pixels = int(mat_file.readline().strip().split()[2]) - 1
				break
			elif not line:
				num_pixels = 0
				break

		mat_file.close()

		writer.writerow(['holdout_' + name, lam_folder[-8:], num_pixels, pred_err])

		k += 1

output_file.close()