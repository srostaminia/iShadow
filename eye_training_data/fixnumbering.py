#!/usr/bin/env python

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
files = sorted(files)

i = 0
adjust = 0
for f in files:
    if ".jpg" not in f:
        continue

    temp = f.split('_')
    name_parts = [temp[0]]
    name_parts.extend(temp[1].split("."))

    index = int(name_parts[1])

    if i < index:
        print i, "is missing"
        adjust += 1
        i += 2
    
    if adjust != 0:
        index -= adjust
        index = str(index).zfill(5)
        new_filename = name_parts[0] + "_" + index + "." + name_parts[2]
        print "Moving", f, "to", new_filename
        shutil.move(f, new_filename)
    
    i += 1
        
print "Removed", adjust, "files"
