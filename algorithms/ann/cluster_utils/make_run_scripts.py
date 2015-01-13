import os, shutil, sys
import itertools
import numpy as np

from subprocess import call

# -----------------START OF CONFIGURATION SETTINGS---------------------------------------------------------

# Configure these two values for the experiment
experiment_descriptor = "microbench1"
mat_file = "eye_target_ann.m"

# names = ['addison2', 'dan', 'james', 'john', 'keith', 'luis', 'niri', 'michelle', 'seth', 'steve']
names = ['addison_microbench1_pupil']

uniquefy = 0
data_limit = 0
hidden_shapes = (2, 3)
init = 'strips'
subset = 'l1'
maxiter = 500

lambdas = [0.0001, 0.00021544, 0.00046416, 0.001, 0.00215443, 0.00464159, 0.01, 0.02154435, 0.04641589, 0.1]
# lambdas = [0.001000, 0.010000, 0.100000]

# -----------------END OF CONFIGURATION SETTINGS-----------------------------------------------------------

data = ['results', 'models']

work_dir = '/mnt/nfs/work1/marlin/amayberr'

if len(experiment_descriptor) != 0:
	experiment_descriptor = '_' + experiment_descriptor

experiment_title = 'awesomeness' + experiment_descriptor
template_title = 'run_template' + experiment_descriptor + '.sh'

def main():
	uniquefy_pre = 'uniquefy_'
	data_limit_pre = 'data_limit_'
	hidden_shapes_pre = 'hidden_'
	init_pre = ''
	subset_pre = ''
	maxiter_pre = 'iter_'

	params = [uniquefy, data_limit, hidden_shapes, init, subset, maxiter]
	prefixes = [uniquefy_pre, data_limit_pre, hidden_shapes_pre, init_pre, subset_pre, maxiter_pre]

	template_f = open(work_dir + '/' + template_title, 'w')

	template_f.write('#!/bin/bash\n')
	template_f.write('octave ' + work_dir + '/' + mat_file + '\n')

	template_f.close()

	print names

	exp_path = work_dir + '/' + experiment_title

	if not os.path.exists(exp_path):
		os.mkdir(exp_path)

	for name in names:
		path = exp_path + '/' + name

		if not os.path.exists(path):
			os.mkdir(path)

		settings_text = ''
		params_recurse(path, name, params, prefixes, lambdas, settings_text)

def params_recurse(path, name, params, prefixes, lambdas, settings_text):
	if len(params) != 0:
		param_settings = params[0]

		# If there is only one setting of the parameter, we don't make a new folder
		if (not isinstance(param_settings, list)) or len(param_settings) == 1:
			this_text = settings_text + build_setting_text(param_settings, '', ', ') + '\n'
			params_recurse(path, name, params[1:], prefixes[1:], lambdas, this_text)

		# Otherwise, make a folder for each setting
		else:
			for setting in param_settings:
				this_text = settings_text + build_setting_text(setting, '', ', ') + '\n'
				setting_folder = build_setting_text(setting, prefixes[0], '_')

				# # Construct folder name
				# if isinstance(setting, list) or isinstance(setting, tuple):
				# 	setting_folder = prefixes[0] + str(setting[0])
				# 	this_text += str(setting[0])

				# 	for item in setting[1:]:
				# 		setting_folder += '_' + str(item)
				# 		this_text += ', ' + str(item)

				# 	this_text += '\n'
				# else:
				# 	setting_folder = prefixes[0] + str(setting)
				# 	this_text += str(setting) + '\n'

				this_path = path + '/' + setting_folder

				if not os.path.exists(this_path):
					os.mkdir(this_path)
				elif len(params) == 1:
					print "ERROR: Data folder", this_path, "already exists - halting"
					sys.exit()

				params_recurse(this_path, name, params[1:], prefixes[1:], lambdas, this_text)
	
	# If params is empty, time to execute a script
	else:
		settings_text += str(lambdas[0])
		for lam in lambdas[1:]:
			settings_text += ', ' + str(lam)
		settings_text += '\n'

		param_file = open(path + '/' + 'params.txt','w')

		param_file.write(settings_text)

		param_file.close()

		data_title = name + experiment_descriptor
		shutil.copy(work_dir + '/' + template_title, path + '/' + data_title + '.sh')

		os.link(work_dir + '/training_sets/' + name + '.mat', path + '/' + 'eye_data.mat')

		pwd = os.getcwd()

		os.chdir(path)
		call(['qsub', '-cwd', '-o', 'stdout.txt', '-e', 'stderr.txt', data_title + '.sh'])
		os.chdir(pwd)

def build_setting_text(setting, prefix, delim):
	text = prefix

	if isinstance(setting, list) or isinstance(setting, tuple):
		text += str(setting[0])

		for item in setting[1:]:
			text += delim + str(item)
	else:
		text += str(setting)

	return text

main()

	# # TODO: Create multiple folders iff there is more than one setting for a parameter
	# for name in names:
	# 	for ftype in types:	# FIXME: types is now data_limit
	# 		for datum in data:
	# 			datum_pathname = work_dir + '/' + experiment_title + '/' + name + '/' + ftype + '/' + datum + '/'

	# 			if not os.path.exists(datum_pathname):
	# 				os.makedirs(datum_pathname)

	# 		ftype_pathname = work_dir + '/' + experiment_title + '/' + name + '/' + ftype + '/'

	# 		dataf = open(ftype_pathname + 'init.txt','w')

	# 		if ftype.find('kmed') != -1:
	# 			dataf.write('1\n')
	# 		elif ftype.find('rand') != -1:
	# 			dataf.write('2\n')
	# 		else:
	# 			dataf.write('0\n')

	# 		dataf.close()

	# 		dataf = open(ftype_pathname + 'data_limit.txt','w')

	# 		if ftype == '5m':
	# 			dataf.write(str(3000) + '\n')
	# 		elif ftype == '3m':
	# 			dataf.write(str(1800) + '\n')
	# 		elif ftype == '2m':
	# 			dataf.write(str(1200) + '\n')
	# 		elif ftype == '1m':
	# 			dataf.write(str(600) + '\n')
	# 		elif ftype == '30s':
	# 			dataf.write(str(300) + '\n')
	# 		else:
	# 			dataf.write(str(0) + '\n')

	# 		dataf.close()

	# 		shutil.copy(work_dir + '/' + template_title, ftype_pathname + name + '_' + ftype + experiment_descriptor + '.sh')

	# 		os.link(work_dir + '/training_sets/eye_data_' + name + '_auto.mat', ftype_pathname + 'eye_data.mat')

	# 		pwd = os.getcwd()

	# 		os.chdir(ftype_pathname)
	# 		call(['qsub', '-cwd', '-o', 'stdout.txt', '-e', 'stderr.txt', name + '_' + ftype + experiment_descriptor + '.sh'])
	# 		os.chdir(pwd)