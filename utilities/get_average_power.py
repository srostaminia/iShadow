import argparse
from os import listdir
from os.path import isfile, join

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("data_folder")
    parser.add_argument("num_items", type=int)

    args = parser.parse_args()

    data_folder = args.data_folder

    files = [ f for f in listdir(data_folder) if isfile(join(data_folder,f)) ]

    averages = []
    for filename in files:
        if ".csv" not in filename:
            continue

        print filename
        try:
            data_file = open(data_folder + "\\" + filename)
        except IOError:
            print "Unable to open file", data_folder + "\\" + filename
            return

        averages.append(get_average_power(data_file, args.num_items))
        data_file.close()

    final_average = 0
    for average in averages:
        final_average += average

    final_average /= len(averages)

    print (final_average / 10.2) * 1000, "mA"
    print ((final_average / 10.2) * 3.04) * 1000, "mW"

def get_average_power(data_file, num_items):
    line = data_file.readline()
    while line[0] != "0":
        line = data_file.readline()

    total = float(line.split(",")[1])
    for i in range(num_items - 1):
        total += float(data_file.readline().split(",")[1])

    average = total / num_items
    print (average / 10.2) * 1000, "mA"
    print ((average / 10.2) * 3.04) * 1000, "mW"
    print

    return average

main()
