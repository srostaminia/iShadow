import matplotlib.pyplot as plt
import numpy as np
import os
from os import listdir
from os.path import isfile, join
import shutil
import scipy.stats.mstats as mstats
import Image


# Get all files in directory
files = [ f for f in listdir('.') if isfile(join('.',f)) ]

os.mkdir("Scene")
os.mkdir("Eye")

i = 0
for f in files:
    if ".png" not in f:
	continue

    name, ext = os.path.splitext(f)
    name_values = name.split("_")
    
    if name_values[2] == 'eye':
	new_name = "Eye/Eye_" + name_values[1][1:] + ".jpg"
    elif name_values[2] == 'out':
	new_name = "Scene/Scene_" + name_values[1][1:] + ".jpg"
    else:
	print "Error:", name_values[2], "in", f, "is not 'eye' or 'out'"
    
    im = Image.open(f)
    im.save(new_name)
