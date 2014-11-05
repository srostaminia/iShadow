import os, shutil

from subprocess import call

# names = ['addison2', 'dan', 'james', 'john', 'keith', 'luis', 'niri', 'michelle', 'steve', 'seth']

names = ['addison2']

# types = ['5m_exfold', '3m_exfold', '2m_exfold', '1m_exfold', '30s_exfold']

types = ['full']

data = ['results', 'model']

model_sizes = [[6, 6], [8, 8], [10, 10]]

for name in names:
	for ftype in types:
		for size in model_sizes:
			size_text = str(size[0]) + "_" + str(size[1])

			for datum in data:
				datum_pathname = 'fullimage/' + name + '/' + size_text + '/' + ftype + '/' + datum + '/'

				if not os.path.exists(datum_pathname):
					os.makedirs(datum_pathname)

			size_pathname = 'fullimage/' + name + '/' + size_text + '/' + ftype + '/'

			dataf = open(size_pathname + 'hidden_shape.txt', 'w')

			dataf.write(str(size[0]) + ' ' + str(size[1]))

			dataf.close()

			dataf = open(size_pathname + 'init.txt','w')

			if ftype.find('kmed') != -1:
				dataf.write('1\n')
			elif ftype.find('rand') != -1:
				dataf.write('2\n')
			else:
				dataf.write('0\n')

			dataf.close()

			dataf = open(size_pathname + 'data_limit.txt','w')

			if ftype[0:2] == '5m':
				dataf.write(str(3000) + '\n')
			elif ftype[0:2] == '3m':
				dataf.write(str(1800) + '\n')
			elif ftype[0:2] == '2m':
				dataf.write(str(1200) + '\n')
			elif ftype[0:2] == '1m':
				dataf.write(str(600) + '\n')
			elif ftype[0:3] == '30s':
				dataf.write(str(300) + '\n')
			else:
				dataf.write(str(0) + '\n')

			dataf.close()

			shutil.copy('run_fullimage_template.sh', size_pathname + name + '_' + size_text + '_' + ftype + '.sh')

			os.link('training_sets/' + 'eye_data_' + name + '_auto.mat', size_pathname + 'eye_data.mat')

			os.chdir(size_pathname)
			call(['qsub', '-cwd', '-o', 'stdout.txt', '-e', 'stderr.txt', name + '_' + size_text + '_' + ftype + '.sh'])
			os.chdir('/home/amayberr/work')