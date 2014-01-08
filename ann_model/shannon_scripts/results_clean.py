import csv, sys
import numpy as np

infile = open('shannon_results.csv','r')
outfile = open('results_clean_avg.csv','w')

reader = csv.reader(infile)
writer = csv.writer(outfile)

rep_count = 0
for row in reader:
	if row[1] != 'full':
		for i in range(4):
			reader.next()
		continue

	if rep_count == 0:
		err_data = []

	err_data.append(float(row[5]))

	if rep_count == 4:
		rep_count = 0
		# print err_data
		writer.writerow([row[0], row[1], row[2], np.mean(err_data), np.std(err_data)])
	else:
		rep_count += 1

# for row in reader:
# 	if row[1] != 'full':
# 		for i in range(4):
# 			reader.next()
# 		continue

# 	writer.writerow(row)

infile.close()
outfile.close()