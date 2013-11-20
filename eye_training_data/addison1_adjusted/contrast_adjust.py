import matplotlib.pyplot as plt
import numpy as np
from os import listdir
from os.path import isfile, join
import shutil
import scipy.stats.mstats as stats
import Image

# Get all files in directory
files = [ f for f in listdir('.') if isfile(join('.',f)) ]

i = 0
for f in files:
    if ".csv" in f:
	continue

    # Open image and convert to array
    oldImage = Image.open(f)
    imageArray = np.asarray(oldImage)

    # Preprocessing
    imageArray=imageArray-np.min(np.min(imageArray))
    vrange = stats.mquantiles(imageArray.flatten(),prob=[0.01,0.99])

    # Plot and save
    plt.figure(figsize=(1,1))
    plt.imshow(imageArray,cmap="gray",interpolation="Nearest",vmin=vrange[0],vmax=vrange[1]);
    plt.axis('off')
    plt.savefig(f + "_adjusted.png", dpi=112)
