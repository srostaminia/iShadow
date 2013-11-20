from graphics import *
import time
import csv
import argparse
import os
from random import shuffle

# Window dimensions
X_DIM=800
Y_DIM=600

# Granularity
GRAN=20

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("dataFolder")
    args = parser.parse_args()

    # Check to see if we are adding on to existing data
    if os.path.exists(args.dataFolder):
	if not os.path.isdir(args.dataFolder):
	    print "ERROR: " + args.dataFolder + " exists and is a file"
	    return
	newData = False
    else:
	os.mkdir(args.dataFolder)    
	newData = True

    os.chdir(args.dataFolder)

    # Build list of points to capture
    points = []
    for i in range(0, X_DIM+1, GRAN):
	for j in range(0, Y_DIM+1, GRAN):
	    points.append([i, j])

    if newData:
	index = 0
    else:	# If any points have already been covered in the data, remove them from the list
	index = readExisting(args.dataFolder, points)

    indexFile = open(args.dataFolder + ".csv", 'a')
    writer = csv.writer(indexFile)

    shuffle(points)

    # Setup graphics window
    win = GraphWin("Eye Experiment", X_DIM, Y_DIM)
    circle = Circle(Point(0,0), GRAN)
    circle.setFill('blue')
    circle.draw(win)

    print index

    raw_input("Press Enter to begin...")

    for point in points:
	moveCircle(circle, point, win, index)
	circle.setFill('red')

	time.sleep(0.5)
	print "cmd: " + "glassescapstream | frontsave -o " + args.dataFolder + "_" + ("%06d" % index)
	os.system("glassescapstream | frontsave -o " + args.dataFolder + "_" + ("%06d" % index))
	writer.writerow([index, point[0], point[1], X_DIM, Y_DIM])
	
	index += 1
	circle.setFill('blue')

    os.chdir('..')

def readExisting(dataFolder, points):
    lines = 0
    print len(points)
    with open(dataFolder + ".csv") as csvFile:
	reader = csv.reader(csvFile)

	for row in reader:
	    location = [int(row[1]), int(row[2])]
	    points.remove(location)
	    lines += 1
	
	print lines
	print len(points)

    return lines

def moveCircle(circle, loc, win, index):
    newX = loc[0]
    newY = loc[1]

    target = Circle(Point(newX,newY), GRAN)
    target.setFill('green')
    target.draw(win)

    if index % 20 == 0:
	text = Text(Point(newX,newY), str(index))
        text.draw(win)

    center = circle.getCenter()
    oldX = center.getX()
    oldY = center.getY()

    if oldX > newX:
	step = -1
    else:
	step = 1

    print "Moving from", [oldX, oldY], "to", loc

    # Move horizontally
    for i in range(int(abs(oldX - newX))):
    	circle.move(step, 0)
        time.sleep(.00005)

    if oldY > newY:
	step = -1
    else:
	step = 1

    # Move vertically
    for i in range(int(abs(oldY - newY))):
    	circle.move(0, step)
        time.sleep(.00005)

    if index % 20 == 0:
	text.undraw()
    target.undraw()

main()
