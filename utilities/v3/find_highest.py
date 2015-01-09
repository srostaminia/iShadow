import struct

infile = open('addison1_unadjusted/addison1_a.raw','rb')

i = 0
largest = 0
data = struct.unpack('H' * 256, infile.read(512))
while (len(data) != 0):
	for item in data:
		if item > largest and item != 960:
			largest = item
			print largest

	data = struct.unpack('H' * 256, infile.read(512))

	i += 1
	if i % 10000 == 0:
		print i, "blocks out of", 313551
