import csv, sys
import numpy as np

infile = open('shannon_holdout_results.csv','r')
outfile = open('shannon_holdout_avg.csv','w')

reader = csv.reader(infile)
writer = csv.writer(outfile)

rep_count = 0
for row in reader:
	if rep_count == 0:
		pix_data = []
		err_data = []

	pix_data.append(float(row[2]))
	err_data.append(float(row[3]))

	if rep_count == 4:
		rep_count = 0
		# print err_data
		writer.writerow([row[1], np.mean(pix_data), np.std(pix_data), np.mean(err_data), np.std(err_data)])
	else:
		rep_count += 1

infile.close()
outfile.close()