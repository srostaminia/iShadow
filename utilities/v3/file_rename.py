import os
from os.path import isfile, join
from os import listdir	

def main():
	files = [ f for f in listdir('.') if isfile(join('.',f)) ]

	for f in files:
	    if ".png" not in f:
		   continue

	    # print f
	    old_name, ext = os.path.splitext(f)

	    old_name = old_name.split('_')[1]

	    new_name = "addison1"

	    nametype = old_name[0:3]
	    id_num = int(old_name[3:])

	    new_name += '_' + ('%06d' % (id_num)) + "_" + nametype + ext

	    os.rename(f, new_name)

main()