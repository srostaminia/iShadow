import os, errno

def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc: # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else: raise

names = ['addison2', 'dan', 'james', 'john', 'keith', 'luis', 'niri', 'michelle']

types = ['full', '5m', '3m', '2m', '1m', '30s', 'full_kmed', 'full_rand']

data = ['results', 'models']

for name in names:
	for ftype in types:
		for datum in data:
			pathname = 'output/' + name + '/' + ftype + '/' + datum

			if not os.path.exists(pathname):
				os.makedirs(pathname)