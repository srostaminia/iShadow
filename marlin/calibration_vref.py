#!/opt/local/bin/python2.7
from serial import *
import numpy as np
import string
import matplotlib.pyplot as plt
import code
import sys
import pickle
from PIL import Image
import scipy.stats.mstats as stats

nr = 20
nc = 20
amp = 1
gain = 7
vref = 0
adc_type=0
aref = 7
vrefs = [28,30,32,34,36,40];
masks = np.zeros((nr,nc,6))
ser = Serial(port="/dev/tty.usbmodem1411",baudrate=115200,bytesize=EIGHTBITS,parity=PARITY_NONE,stopbits=STOPBITS_ONE,timeout=10,xonxoff=0,rtscts=0,interCharTimeout=None)

def main():
  mask_file = "mask_vref.pkl";
  sys.stdout = os.fdopen(sys.stdout.fileno(), 'w', 0)
  plt.ion()  
  img = np.zeros((nr,nc))
  print ser.readline()  

  global amp
  global gain
  global vref
  global adc_type
  global aref 
  global masks
  global count
 
  print (amp,gain,vref,adc_type,aref)
 
  ser.write("a%d"%amp) #set amp
  ser.write("g%d"%gain) #set gain
  ser.write("v%d"%vrefs[vref]) #set vref
  ser.write("d%d"%adc_type) #set adc 
  ser.write("f%d"%aref) #set aref 
 
  try:
    f = open(mask_file,'rb')
    masks = pickle.load(f)
    f.close()
  except IOError as e:
    f = open(mask_file, 'wb')
    setMasks()
    pickle.dump(masks, f)
    f.close()
  
  count = 0;
  while(1):
    try:
      vref=input("VREF (0-5)>>")
      ser.write("v%d"%vrefs[vref]) #set vref
      readImage()
      count = count +1;
    except EOFError as e:
      print "Invalid input"

      
def readImage():
  print("Acquiring Image...")
  raw_img = readRawImage()
    
  print("Applying Mask...")
  img = raw_img - masks[:,:,vref]
  plt.figure(2)
  plt.clf()
  img=img-np.min(np.min(img))
  plt.subplot(122)
  vrange = stats.mquantiles(img.flatten(),prob=[0.01,0.99])
  plt.imshow(img,cmap="gray",interpolation="Nearest",vmin=vrange[0],vmax=vrange[1]);
  plt.colorbar()
  plt.subplot(121)
  plt.imshow(raw_img,cmap="gray",interpolation="Nearest");
  plt.colorbar()
  plt.draw()
  plt.savefig("img_%d_vref%d.pdf"%(count, vrefs[vref]))
  
  print [np.min(np.min(img)), np.max(np.max(img)),np.max(np.max(img))-np.min(np.min(img)) ]


def setMasks():
  global masks
  amp=1;
  print("Acquiring Masks...")
  raw_input("  Place a white sheet over camera lens and press return>>")
  for v in range(6):
    ser.write("v%d"%vrefs[v]);
    reps = 5.0
    for i in range(int(reps)):
      print [i,v]
      masks[:,:,v]=masks[:,:,v]+readRawImage()/reps
    masks[:,:,v]=masks[:,:,v] - np.min(np.min(masks[:,:,v])) 
  #return mask

def readRawImage():
  ser.write("r") #reset image readout counter
  global nr
  global nc

  img = np.zeros((nr,nc))  
  print("Acquiring Raw Image"),
  ser.write("n")
  for i in range(nr):
    sys.stdout.write(".")
    img_str1 =  ser.read(nc) #read low bits
    img[i,:] = map(ord,img_str1)    
    img_str2 =  ser.read(nc) #read hi bits
    img[i,:] = img[i,:] + 256*np.array(map(ord,img_str2))

  img=np.fliplr(img)
  sys.stdout.write("\n"),    
  plt.figure(1)
  plt.clf();
  plt.imshow(img,cmap="gray",interpolation="Nearest");
  plt.colorbar()
  plt.draw()
  print [np.min(np.min(img)), np.max(np.max(img)),np.max(np.max(img))-np.min(np.min(img)) ]
  return img


def keyboard(banner=None):
    ''' Function that mimics the matlab keyboard command '''
    # use exception trick to pick up the current frame
    try:
        raise None
    except:
        frame = sys.exc_info()[2].tb_frame.f_back
    # evaluate commands in current namespace
    namespace = frame.f_globals.copy()
    namespace.update(frame.f_locals)
    try:
        code.interact(banner=banner, local=namespace)
    except SystemExit:
        return 


def histeq(im,nbr_bins=256):

   #get image histogram
   imhist,bins = np.histogram(im.flatten(),nbr_bins,normed=True)
   cdf = imhist.cumsum() #cumulative distribution function
   cdf = 255 * cdf / cdf[-1] #normalize

   #use linear interpolation of cdf to find new pixel values
   im2 = np.interp(im.flatten(),bins[:-1],cdf)

   return im2.reshape(im.shape)

main()