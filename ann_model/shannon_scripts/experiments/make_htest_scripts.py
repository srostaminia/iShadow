import os, shutil

from subprocess import call

name = 'addison2'

ftype = '1m'

structure = '2x3'

data = ['results', 'models']

for datum in data:
	datum_pathname = 'htest/' + structure + '/' + datum + '/'

	if not os.path.exists(datum_pathname):
		os.makedirs(datum_pathname)

ftype_pathname = 'htest/' + structure + '/'

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

shutil.copy('run_template.sh', ftype_pathname + 'htest_' + structure + '.sh')

os.link('training_sets/' + 'eye_data_' + name + '_auto.mat', ftype_pathname + 'eye_data.mat')

os.chdir(ftype_pathname)
call(['qsub', '-cwd', '-o', 'stdout.txt', '-e', 'stderr.txt', 'htest_' + structure + '.sh'])
os.chdir('/home/amayberr')