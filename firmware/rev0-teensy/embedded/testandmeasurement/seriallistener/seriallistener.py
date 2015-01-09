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

sp = serial.Serial( port='/dev/ttyACM0',
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
sp.close();
sp.open();

while 1:
    # poll keyboard
#    cc = getch()
#    if('x' == cc):
#        break;

    # grab the returned data
    while(1):
        rxData = sp.read();
        if(0 == len(rxData)):
            time.sleep(0.1);
            rxData = sp.read();
        if(0 < len(rxData)):
            sys.stdout.write(rxData);
        else:
            break;

print("Exiting");

sp.close();

