#!/usr/bin/env python2

import usb.control, usb.core
import time
import sys
import struct

def get_first(data):
	for item in data:
		if item != 0:
			return item

	return -1

def valid_bytes(data):
	count = 0
	for item in data:
		if item != 0:
			count += 1

	return count

def count_em_up(data):
	current_num = -1

	results = {}
	for item in data:
		if item != current_num:
			results[item] = 1
			current_num = item
		else:
			results[current_num] += 1

	return results	

dev = usb.core.find(idVendor = 0x483)

for cfg in dev:
	pass

cfg.set()

usb.control.set_interface(dev, 1, 1)

for intf in cfg:
	pass

for endp in intf:
	pass

pixels = 0
data_started = 0
packets = 0
while True:
	data = endp.read(1840)

	if (get_first(data) != -1) and (data_started == 0):
		data_started = 1
	elif (get_first(data) == -1) and (data_started == 1):
		break

	if (data_started == 1):
		packets += 1

	unpacked = struct.unpack('H' * 920, data)

	temp = valid_bytes(unpacked)
	print "Packet #" + str(packets) + ", pixels:", temp
	# print unpacked
	print count_em_up(unpacked), "\n"
	pixels += temp
	# count += 1

print "\nPixels:", pixels
print "Packets:", packets

# while True:
# 	data = endp.read(1840)

# 	if get_first(data) == -1:
# 		continue

# 	print len(data)
# 	print struct.unpack('H' * 920, data)
# 	break

# data = endp.read(1840)
# print len(data)
# print data

# last_time = time.time()
# i = 1
# while i < 100:
# 	data = endp.read(1840)

# 	val = get_first(data)
# 	if val != -1:
# 		delay = time.time() - last_time

# 		if delay > 3:
# 			print "ERROR:", delay
# 			print data
# 			sys.exit()

# 		print "#" + str(val) + ":", delay
# 		last_time = time.time()

# 		# print data

# 		i += 1