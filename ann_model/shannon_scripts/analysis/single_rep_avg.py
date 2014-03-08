import csv, sys
import numpy as np

if os.path.exists('shannon_single_avg.csv'):
	print "ERROR: shannon_single_avg.csv exists"
	sys.exit()

infile = open('shannon_single_results.csv','rU')
outfile = open('shannon_single_avg.csv','w')

reader = csv.reader(infile)
writer = csv.writer(outfile)

rep_count = 0
for row in reader:
	if rep_count == 0:
		pix_data = []
		err_data = []

	pix_data.append(float(row[4]))
	err_data.append(float(row[5]))

	if (row[0] == 'addison2') and (row[1] == '15s_exfold') and (row[2] == '0.0001'):
		folds = 4
	else:
		folds = 5

	if rep_count == (folds - 1):
		rep_count = 0
		# print err_data
		writer.writerow([row[0], row[1], row[2], np.mean(pix_data), np.std(pix_data), np.mean(err_data), np.std(err_data)])
	else:
		rep_count += 1

infile.close()
outfile.close()