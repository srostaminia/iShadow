#!/usr/bin/env python

#
# Russ Bielawski
# 2012-09-21
#

rowlocs = [ 1 , 2 , 2 , 3 , 4 , 4 , 7 , 9 , 14 , 16 , 16 , 19 , 21 , 23 , 30 , 31 , 32 , 32 , 33 , 34 , 34 , 37 , 38 , 41 , 43 , 44 , 49 , 49 , 49 , 51 , 52 , 56 , 56 , 58 , 65 , 68 , 69 , 69 , 70 , 71 , 74 , 75 , 77 , 77 , 78 , 81 , 83 , 83 , 83 , 85 , 87 , 87 , 88 , 88 , 91 , 92 , 93 , 95 , 97 , 101 , 101 , 102 , 102 , 104 , 104 , 108 , 108 , 109 , 111 ]
collocs = [ 54 , 36 , 68 , 54 , 24 , 74 , 54 , 81 , 101 , 3 , 67 , 54 , 93 , 47 , 1 , 101 , 55 , 69 , 54 , 48 , 68 , 54 , 34 , 15 , 22 , 50 , 39 , 67 , 90 , 68 , 22 , 27 , 42 , 54 , 88 , 90 , 7 , 32 , 54 , 60 , 75 , 57 , 12 , 55 , 54 , 85 , 1 , 28 , 64 , 68 , 7 , 69 , 54 , 73 , 54 , 7 , 94 , 50 , 69 , 50 , 84 , 54 , 90 , 52 , 54 , 54 , 72 , 54 , 56 ]

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

    pixels = []
    # grab the returned data
    #print "Img = ["
    #outFile.write("Img = [\n")
    while(1):
        rxData = ardumon.readline();
        if(0 == len(rxData)):
            time.sleep(0.1);
            rxData = ardumon.readline();
        if(0 < len(rxData)):
            #print len(rxData)
            #sys.stdout.write(rxData);
            for i in range(69):
                item = rxData[i]
                pixels.append(ord(item))
                #print ord(item),
                #outFile.write(str(ord(item)) + " ")
            #print
            #outFile.write("\n")
            #if(0 != flagWriteOutFile):
            #    outFile.write(rxData);
        else:
            break;
    
    #print "];"
    #outFile.write("];")
    
    for val in pixels:
        outFile.write(str(val) + "\n")
        print val

    if(0 != flagWriteOutFile):
        outFile.close();

print("Exiting");

ardumon.close();

