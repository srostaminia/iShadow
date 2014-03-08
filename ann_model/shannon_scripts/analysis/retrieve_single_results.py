import os, sys, csv
from subprocess import call

names = ['addison2', 'dan', 'james', 'john', 'keith', 'luis', 'michelle', 'niri', 'seth', 'steve']

types = ['full_kmed', 'full_rand', 'full', '5m_exfold', '3m_exfold', '2m_exfold', '1m_exfold', '30s_exfold', '15s_exfold']

lambda_folders = ['subset_l1_init_strips_k7_lambda0.000100',  'subset_l1_init_strips_k7_lambda0.000464',  'subset_l1_init_strips_k7_lambda0.002154',  \
'subset_l1_init_strips_k7_lambda0.010000',  'subset_l1_init_strips_k7_lambda0.046416', 'subset_l1_init_strips_k7_lambda0.000215', \
'subset_l1_init_strips_k7_lambda0.001000',  'subset_l1_init_strips_k7_lambda0.004642',  'subset_l1_init_strips_k7_lambda0.021544', \
'subset_l1_init_strips_k7_lambda0.100000']

if os.path.exists('shannon_single_results.csv'):
	print "ERROR: shannon_single_results.csv exists"
	sys.exit()

output_file = open('shannon_single_results.csv','w')
writer = csv.writer(output_file)

print types

k = 1
for name in names:
	npath = '/Users/ammayber/senseye/ann_model/shannon_data/awesomeness/' + name + '/'

	for ftype in types:
		fpath = npath + ftype + '/'

		for lam_folder in lambda_folders:
			lpath = fpath + 'results/' + lam_folder + '/'

			for i in range(1,6):
				mat_filename = lpath + 'rep' + str(i) + '.mat'
				print mat_filename

				if os.path.exists(mat_filename):
					mat_file = open(mat_filename, 'r')
				else:
					continue

				print str(k) + ': ' + mat_filename

				while True:
					line = mat_file.readline()

					if "ErrTest" in line:
						mat_file.readline()
						pred_err = float(mat_file.readline().strip())
						break
					elif not line:
						print "ERROR: Could not find ErrTest in", mat_filename
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

				writer.writerow([name, ftype, lam_folder[-8:], 'rep' + str(i), num_pixels, pred_err])

				k += 1

output_file.close()