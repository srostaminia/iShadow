import os, shutil, sys
import itertools
import numpy as np

from subprocess import call

# -----------------START OF CONFIGURATION SETTINGS---------------------------------------------------------

# Configure these two values for the experiment
experiment_descriptor = ['irb_radius_target_median']
if len(experiment_descriptor) != 0:
	experiment_descriptor = '_' + experiment_descriptor

experiment_title = 'awesomeness' + experiment_descriptor

mat_file = "runCiderWrapper_new.m"  #"eye_target_ann.m"

# names = ['addison2', 'dan', 'james', 'john', 'keith', 'luis', 'niri', 'michelle', 'seth', 'steve']
#names = ['addison_microbench1_pupil']
subjects=['addison'];#['duncan','kaituo','malai','mike'] #sean
obj='pupil;
lightings=['_calib_']

names = [sub+light+obj for light in lightings for sub in subjects]

#names=['addison_calib_pupil']+names

uniList = ['uniquefy_0', 'uniquefy_1']

#250 for gaze

lambdas =['0.000100','0.000215','0.000464','0.001000','0.002154','0.004642','0.010000','0.021544','0.046416','0.100000']
# lambdas = [0.001000, 0.010000, 0.100000]

# -----------------END OF CONFIGURATION SETTINGS-----------------------------------------------------------
work_dir = os.getcwd()

def main():
	
	print names

	exp_path = work_dir + '/' + experiment_title+'/'

	for name in names:
		path = exp_path + '/' + name+'/'

                for uni in uniList:
                        path+='/'+uni+'/results/'
                        path_sh=path
                        
                        for lam in lambdas[1:]:
                                
                                path += 'subset_l1_init_strips_k7_lambda'+str(lam)+'/'
		
                                for i in range(5):
                                        path+= 'rep' + str(i + 1)

                                        for i in os.listdir(path):
                                            if os.path.isfile(os.path.join(path,i)) and modelName in i:
                                                print "ERROR: "+modelName+" already ran before."
                                                sys.exit()
                                                                       
                                        
                                        data_title = name + experiment_descriptor + '_'+modelName+'_rep' + str(i + 1)

                                        run_script = open(path_sh + '/' + data_title + '.sh','w')
                                        run_script.write('#!/bin/bash\n')
                                        run_script.write('octave ' + work_dir + '/run_model/' + mat_file + '\n')
                                        run_script.close()

                                        cwd = os.getcwd()

                                        os.chdir(path)
                                        call(['qsub', '-cwd', '-o', 'stdout.txt', '-e', 'stderr.txt', data_title + '.sh'])
                                        os.chdir(cwd)



main()
