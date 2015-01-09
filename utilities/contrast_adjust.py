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

i = 0
for f in files:
    if ".png" not in f:
	continue

    print f
    name, ext = os.path.splitext(f)

    oldImage = Image.open(f).convert('L')
    imageArray = np.array(oldImage)

    vmin, vmax = mstats.mquantiles(imageArray.flatten(), prob=[0.01,0.99])

    np.clip(imageArray, vmin, vmax, out=imageArray)
    imageArray = (imageArray-vmin)*255/(vmax-vmin)
    img = Image.fromarray(imageArray.astype('uint8'), 'L')
    img.save(name + ".png")
