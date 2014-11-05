import os, shutil
import itertools

from subprocess import call

# Configure these two values for the experiment
experiment_descriptor = "sub_resize"
mat_file = "run_me_cluster_sub_resize.m"

# Leave the list alone, just modify the mask
name_list = ['addison2', 'dan', 'james', 'john', 'keith', 'luis', 'niri', 'michelle', 'seth', 'steve']
name_mask = [    1,		   1,	   1,		1,		1,		 1,		 1,		   1,		 1,		  1  ]

# Leave the list alone, just modify the mask
type_list = ['full', '5m', '3m', '2m', '1m', '30s', 'full_kmed', 'full_rand']
type_mask = [  1,	   0,	0,	   0,	 0,	   0,		 0,		      0		]

data = ['results', 'models']

work_dir = '/mnt/nfs/work1/marlin/amayberr'

if len(experiment_descriptor) != 0:
	experiment_descriptor = '_' + experiment_descriptor

experiment_title = 'awesomeness' + experiment_descriptor
template_title = 'run_template' + experiment_descriptor + '.sh'

template_f = open(work_dir + '/' + template_title, 'w')

template_f.write('#!/bin/bash\n')
template_f.write('octave ' + work_dir + '/' + mat_file + '\n')

template_f.close()

names = [ item for item, flag in zip(name_list, name_mask) if flag == 1 ]
types = [ item for item, flag in zip(type_list, type_mask) if flag == 1 ]

print names

for name in names:
	for ftype in types:
		for datum in data:
			datum_pathname = work_dir + '/' + experiment_title + '/' + name + '/' + ftype + '/' + datum + '/'

			if not os.path.exists(datum_pathname):
				os.makedirs(datum_pathname)

		ftype_pathname = work_dir + '/' + experiment_title + '/' + name + '/' + ftype + '/'

		dataf = open(ftype_pathname + 'init.txt','w')

		if ftype.find('kmed') != -1:
			dataf.write('1\n')
		elif ftype.find('rand') != -1:
			dataf.write('2\n')
		else:
			dataf.write('0\n')

		dataf.close()

		dataf = open(ftype_pathname + 'data_limit.txt','w')

		if ftype == '5m':
			dataf.write(str(3000) + '\n')
		elif ftype == '3m':
			dataf.write(str(1800) + '\n')
		elif ftype == '2m':
			dataf.write(str(1200) + '\n')
		elif ftype == '1m':
			dataf.write(str(600) + '\n')
		elif ftype == '30s':
			dataf.write(str(300) + '\n')
		else:
			dataf.write(str(0) + '\n')

		dataf.close()

		shutil.copy(work_dir + '/' + template_title, ftype_pathname + name + '_' + ftype + experiment_descriptor + '.sh')

		os.link(work_dir + '/training_sets/eye_data_' + name + '_auto.mat', ftype_pathname + 'eye_data.mat')

		pwd = os.getcwd()

		os.chdir(ftype_pathname)
		call(['qsub', '-cwd', '-o', 'stdout.txt', '-e', 'stderr.txt', name + '_' + ftype + experiment_descriptor + '.sh'])
		os.chdir(pwd)