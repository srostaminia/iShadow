import os, shutil

from subprocess import call

names = ['addison2', 'dan', 'james', 'john', 'keith', 'luis', 'niri', 'michelle', 'seth', 'steve']

data = ['results', 'model']


for [i, name] in enumerate(names):
	for datum in data:
		datum_pathname = 'awesomeness/holdout/holdout_' + name + '/' + datum + '/'

		if not os.path.exists(datum_pathname):
			os.makedirs(datum_pathname)

	name_pathname = 'awesomeness/holdout/holdout_' + name + '/'

	dataf = open(name_pathname + 'init.txt','w')

	# if ftype.find('kmed') != -1:
	# 	dataf.write('1\n')
	# elif ftype.find('rand') != -1:
	# 	dataf.write('2\n')
	# else:
	# 	dataf.write('0\n')

	dataf.write('0\n')

	dataf.close()

	dataf = open(name_pathname + 'data_limit.txt','w')

	# if ftype == '5m':
	# 	dataf.write(str(3000) + '\n')
	# elif ftype == '3m':
	# 	dataf.write(str(1800) + '\n')
	# elif ftype == '2m':
	# 	dataf.write(str(1200) + '\n')
	# elif ftype == '1m':
	# 	dataf.write(str(600) + '\n')
	# elif ftype == '30s':
	# 	dataf.write(str(300) + '\n')
	# else:
	# 	dataf.write(str(0) + '\n')

	dataf.write(str(0) + '\n')

	dataf.close()

	dataf = open(name_pathname + 'hold_out.txt','w')

	dataf.write(str(i+1) + '\n')

	dataf.close()

	shutil.copy('run_holdout_template.sh', name_pathname + 'holdout_' + name + '.sh')

	os.chdir(name_pathname)
	call(['qsub', '-cwd', '-o', 'stdout.txt', '-e', 'stderr.txt', 'holdout_' + name + '.sh'])
	os.chdir('/home/amayberr')