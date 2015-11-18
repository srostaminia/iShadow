---------------------------------iShadow---------------------------------------

This branch (master) contains the essentials to run the iShadow glasses with or without the eye tracking algorithm. There aren't any proper makefiles at the moment, continue in this readme for instructions to set up and run everything.

Each directory contains further readmes with more detail on how to use things, this is a general start point and overview on how the whole toolchain works together.

-----------------------------Table of Contents---------------------------------

1. Directory structure
2. Prerequisite hardware / software
3. First time setup
4. Running the glasses (no eye tracking)
5. Using the eye tracking models



--------------------------1. Directory Structure-------------------------------

algorithms 	- code for running various pieces of the eye tracking algorithms 				offline (i.e., on a computer, not the glasses)
doc 		- documentation for various sw and hw components
firmware 	- code to be loaded onto the glasses to run different tasks
utilities	- various scripts for interfacing with and debugging the glasses



--------------------2. Prerequisite Hardware / Software------------------------

For the moment, running the glasses requires simultaneous use of a few different tools and environments. This is mostly due to restricted availability of certain tools to certain platforms, code coming from several different sources, and ease of developing different parts of the platform in one environment or another.

HARDWARE REQUIREMENTS:
-It should go without saying that you need a pair of the iShadow glasses. If you somehow find yourself without a working pair, contact the repo maintainer and we should be able to arrange to get you some.

-ST-LINK/v2 programmer, manufactured by STMicroelectronics. Also some kind of jumper wires or header to connect pins from the programmer to the pins on the glasses.

-MicroUSB cable for power / communication with a desktop machine

-MicroSD card. Believe it or not, there seems to be a spectrum of performance across card models, with some outright failing to work. SanDisk Ultras of any size have worked the best for us, and are fairly ubiquitous. You'll also obviously need some way to read the data off the card onto a computer.


SOFTWARE REQUIREMENTS:
-Operating system is the tricky part. Programming the glasses currently has to happen in a Windows environment, because our code is written for IAR Systems compilers, and all of their software / drivers only run in Windows. However, to actually get data off the glasses, you need OSX - presumably any *nix environment will work, but it is only tested and working in OSX.

WINDOWS:
-IAR EWARM compiler. There is a "Kickstart" edition available that does not expire but has a limit on the code size. All of our firmware is currently small enough to work with this limit.

-STM32 ST-Link Utility (currently available at http://www.st.com/web/en/catalog/tools/PF258168 ). Awesome little tool for helping diagnose and fix connection / programming issues.

-Driver for ST-LINK/v2 programmer, available from STM's website

OSX / *NIX:
-Python 2.x, with the following packages:
  -numpy
  -matplotlib
  -pyusb

-Matlab or Octave of some flavor

-libusb, available via Homebrew on OSX or at http://www.libusb.org/



-----------------------------3. First Time Setup-------------------------------

Well, you made it this far without getting scared off, that in itself is impressive. Everything here should be in good enough shape that you can run the glasses with the instructions below, but if you hit any snags, let us know.

STEP 1. Load the firmware

First, connect the pins of the ST-LINK to the programming pins on the glasses. You can find the pinout of the ST-LINK in its spec sheet, available in doc/hw/mcu. Pin 1 of the glasses is the one closest to the SD card slot. Connect them as follows (we recommend using long cables so you don't have to hold the programmer near the glasses at all times):

ST-LINK | iShadow
   1	->   1
   7    ->   2
   9    ->   3
   20   ->   4
   15   ->   5

Once that's done, navigate to the driver project (firmware/rev3/ishadow_driver) and open up the "ishadow_driver.eww" file with IAR (TODO: change project name). This should load the project with all of the correct settings. See the readme in the project file for details on how to use it.

STEP 2. FPN masks

---Overview---
The cameras we use in iShadow (model name: Stonyman) are generally awesome, but have one this one really annoying issue called "fixed pattern noise" (FPN). Basically, the pixels are very sensitive to manufacturing variations and all have slightly different performance and response to light. Fortunately, assuming consistent lighting conditions, this can be modeled as a simple set of offsets to be subtracted off each pixel (we refer to these sets as "FPN masks"), and all of the code is designed to handle it gracefully.

The FPN masks are just stored as binary files, and you should only ever need to generate them once for each set of glasses. The caveat to that is the fact that significantly different lighting conditions change the pixel response. Thus, if you move the glasses to a significantly lighter / darker environment than that in which the image to create the mask was taken, you'll start to see serious degradation in image quality and you'll need a new set of masks specifically for that environment. In practice, so long as you're operating in normal indoor lighting conditions at all times, you should be fine with just one set of masks that you use all the time.

I say "set of masks" because you need three at any given time, two for the eye-facing camera and one for the out-facing. I'm sure you're wondering why two for the eye camera, but the answer to that is outside the scope of this document. Just trust us - it has to do with the way the Stonyman accesses pixels, and CIDER specifically won't work without both eye masks.

---Creating the Masks----
Making the masks is pretty easy. There are flags in the firmware (TODO: ADD THIS FLAG AND PUT THE NAME HERE) called XXX_EYE and XXX_OUT, which will record the mask data to the SD card. For each camera (eye-facing and out-facing), do the following:

1. 	Set the appropriate flag (XXX_EYE or XXX_OUT)
2. 	Make sure the camera you're calibrating is facing a light source, a
	ceiling light is fine (it should *not* be a window unless you're making a 
	mask for use outdoors)
3. 	Cover the camera with a piece of white paper - place it directly against 
   	the lens
4. 	Load and run the firmware with the paper against the lens
5.	Make sure the program doesn't restart - if you just remove the programmer,
	the program will start again and overwrite the mask data. Best to just shut down the glasses after running the program.
5.	Remove the SD card and plug it into your non-Windows machine
6.	Use the script utilities/rev3/unpack_mask.py to extract the mask(s) from 
	the SD card. unpack_mask.py has XXX_eye and XXX_out options that will automatically pull out and label the mask files for use by other scripts. (TODO: ADD THESE FLAGS TO UNPACK_MASK)

With these two tasks done, you're ready to actually start using the glasses. TODO: add text for generating 8-bit masks for use with usb_image



------------------4. Running the Glasses (No Eye Tracking)---------------------

The basic operating mode of the glasses is just reading from one or both cameras and sending the data somewhere, either to a computer via USB or to an SD card for storage. The USB is only for doing fast debugging / fitting of the glasses, as it's much than the SD card. Also, because the USB is already so slow, it's only configured to send data from one camera at a time, whereas the SD card can store data from both cameras simultaneously without any slowdown. So, for any data collection we strongly recommend using the SD card.

All of these options are controlled via #define flags in the stonyman_conf.h file included in the firmware, see the readme there for more detailed info on how to select different options and run the firmware. There are two other pieces you need in order to succesfully get data from the glasses: loading the FPN masks and running the data extraction scripts.

---Loading the FPN Masks---
There are two ways to use the FPN masks. The simplest (and recommended) way is to load them directly into the firmware on the glasses, where they will be used to automatically clean up the images before they're sent out (TODO: change mask options in unpack_images so they're not always on). Note that if you want to use any of the eye models discussed in the next section, you have to include at least the eye camera masks in the firmware.

Alternatively, all of the data extraction scripts are able to read the mask files and do post-facto cleanup of the raw video. This is not recommended because it means you always have to have a copy of the masks sitting around on whichever machine you're reading data on. The option exists because the mask files are relatively large, which slows down the programming process a bit - however, unless you're doing debugging of the glasses firmware, you likely won't be reprogramming them enough to care (TODO: change process_models to not include mask files if they're unused).

To create the mask files, simply run the build_param_file.py script with the option "--fpn_folder X", where X is the folder that you saved the masks to. The script will create a binary file with a name that you specify. Follow the instructions for the USE_PARAM_FILE flag in the firmware readme to see how to set the firmware to use the masks.

---Extracting the Video---
Once you've configured and loaded the firmware, you can begin collecting video. To read it, use the usb_image.py script in utilities/rev3 for USB mode or the unpack_images.py script in the same location for reading from an SD card. Both scripts come with instructions.



----------------------5. Using the Eye Tracking Models-------------------------

Nowwww for the fun stuff. 