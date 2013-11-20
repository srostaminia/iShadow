#!/usr/bin/env python

#
# Russ Bielawski
# 2012-09-21
#

import serial
import sys
import time
import os
import tty
import termios

ardumon = serial.Serial( port='/dev/ttyACM0',
                         baudrate=115200,
                         bytesize=serial.EIGHTBITS,
                         parity=serial.PARITY_NONE,
                         stopbits=serial.STOPBITS_ONE,
                         timeout=0.2 );

if 'nt' == os.name:
    print("reading from keyboard not implemented for windows; adapt the code!");
    exit(1);

# borrowed code from: http://code.activestate.com/recipes/134892/
def getch():
    fd=sys.stdin.fileno();
    old_settings=termios.tcgetattr(fd)
    try:
        tty.setraw(sys.stdin.fileno());
        ch = sys.stdin.read(1);
    finally:
        termios.tcsetattr(fd, termios.TCSADRAIN, old_settings);
    return ch;


# init the connection
ardumon.close();
ardumon.open();

while 1:
    # poll keyboard
    flagWriteOutFile = 0;
    cc = getch();
    txData = cc;
    if('q' == cc):
        break;
    elif('Z' == cc):
        outFilename = sys.stdin.readline();
        outFilename = outFilename.strip();
        if("" != outFilename):
            flagWriteOutFile = 1;
    else:
        # accept generic command
        txData = cc;

    print("tx: "+txData);
    ardumon.write(txData);
#    rxData = ardumon.readline();
#    ardumon.flushInput();
#    print("rx: "+rxData);

    print("rx:");

    if(0 != flagWriteOutFile):
        try:
            outFile = open(outFilename, 'w');
        except IOError as (errno, strerror):
            print "I/O error({0}): {1} opening out files".format(errno, strerror);
            flagWriteOutFile = 0;
        except:
            print "unexpected error opening out files:", sys.exc_info()[0]
            flagWriteOutFile = 0;

    # grab the returned data
    print "Img = ["
    outFile.write("Img = [\n")
    while(1):
        rxData = ardumon.readline();
        if(0 == len(rxData)):
            time.sleep(0.1);
            rxData = ardumon.readline();
        if(0 < len(rxData)):
	    print len(rxData)
            #sys.stdout.write(rxData);
	    for i in range(112):
	        for j in range(112):
		    item = rxData[(i * 112) + j]
		    print ord(item),
		    outFile.write(str(ord(item)) + " ")
		print
                outFile.write("\n")
            #if(0 != flagWriteOutFile):
            #    outFile.write(rxData);
        else:
            break;
    
    print "];"
    outFile.write("];")

    if(0 != flagWriteOutFile):
        outFile.close();

print("Exiting");

ardumon.close();

