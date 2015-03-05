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

random_seeds = [136824521, 160358906, 614159124, 153154915, 776882322]

if len(experiment_descriptor) != 0:
	experiment_descriptor = '_' + experiment_descriptor

experiment_title = 'awesomeness' + experiment_descriptor

def main():
	uniquefy_pre = 'uniquefy_'
	data_limit_pre = 'data_limit_'
	hidden_shapes_pre = 'hidden_'
	init_pre = ''
	subset_pre = ''
	maxiter_pre = 'iter_'

	params = [uniquefy, data_limit, hidden_shapes, init, subset, maxiter]
	prefixes = [uniquefy_pre, data_limit_pre, hidden_shapes_pre, init_pre, subset_pre, maxiter_pre]

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

		os.link(work_dir + '/training_sets/' + name + '.mat', path + '/' + 'eye_data.mat')

		for i in range(5):
			this_text = settings_text
			this_text += str(random_seeds[i]) + '\n'
			this_text += str(i + 1) + '\n'

			this_path = path + '/rep' + str(i + 1) + '_run'

			if not os.path.exists(this_path):
				os.mkdir(this_path)
			elif len(params) == 1:
				print "ERROR: Run script folder", this_path, "already exists - halting"
				sys.exit()

			param_file = open(this_path + '/' + 'params.txt','w')
			param_file.write(this_text)
			param_file.close()

			data_title = name + experiment_descriptor + '_rep' + str(i + 1)

			run_script = open(this_path + '/' + data_title + '.sh','w')
			run_script.write('#!/bin/bash\n')
			run_script.write('octave ' + work_dir + '/' + mat_file + '\n')
			run_script.close()

			cwd = os.getcwd()

			os.chdir(this_path)
			call(['qsub', '-cwd', '-o', 'stdout.txt', '-e', 'stderr.txt', data_title + '.sh'])
			os.chdir(cwd)

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