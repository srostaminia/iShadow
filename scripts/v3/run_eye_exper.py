import graphics
import time
import csv
import argparse
import os
from random import shuffle

# Window dimensions
X_DIM=1280
Y_DIM=980

X_ADDITIONAL=0

# Granularity
GRAN=20

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("logfile")
    args = parser.parse_args()

    # Check to see if we are adding on to existing data
    if os.path.exists(args.logfile + ".csv") and not os.path.isdir(args.logfile + ".csv"):
        newData = False
    else:
    	newData = True

    # Build list of points to capture
    points = []
    for i in range(0, X_DIM+1, GRAN):
    	for j in range(0, Y_DIM+1, GRAN):
    	    points.append([i, j])

    if newData:
    	index = 0
    else:	# If any points have already been covered in the data, remove them from the list
    	index = readExisting(args.logfile + ".csv", points)

    indexFile = open(args.logfile + ".csv", 'a')
    writer = csv.writer(indexFile)

    shuffle(points)

    # Setup graphics window
    win = graphics.GraphWin("Eye Experiment", X_DIM + X_ADDITIONAL, Y_DIM)
    win.setBackground('white')
    circle = graphics.Circle(graphics.Point(X_ADDITIONAL / 2, 0), GRAN)
    circle.setFill('black')
    circle.draw(win)

    print index

    raw_input("Press Enter to begin...")

    for point in points:
    	moveCircle(circle, point, win, index)
    	circle.setFill('black')

    	time.sleep(0.5)
    	writer.writerow([index, point[0], point[1], X_DIM, Y_DIM])
    	
    	index += 1
    	circle.setFill('black')

def readExisting(logfile, points):
    lines = 0
    print len(points)
    with open(logfile) as csvFile:
        reader = csv.reader(csvFile)

        for row in reader:
            location = [int(row[1]), int(row[2])]
            points.remove(location)
            lines += 1
    
    print lines
    print len(points)

    return lines

def moveCircle(circle, loc, win, index):
    newX = loc[0] + (X_ADDITIONAL / 2)
    newY = loc[1]

    if index % 20 == 0:
    	text = graphics.Text(graphics.Point(newX,newY), str(index))
        text.draw(win)

    center = circle.getCenter()
    oldX = center.getX()
    oldY = center.getY()

    if oldX > newX:
    	step = -5
    else:
    	step = 5

    print "Moving from", [oldX - (X_ADDITIONAL / 2), oldY], "to", loc

    # Move horizontally
    for i in range(int(abs(oldX - newX)) / 5):
    	circle.move(step, 0)
        # time.sleep(.00005)

    if oldY > newY:
	   step = -5
    else:
	   step = 5

    # Move vertically
    for i in range(int(abs(oldY - newY)) / 5):
    	circle.move(0, step)
        # time.sleep(.00005)

    if index % 20 == 0:
    	text.undraw()

main()
