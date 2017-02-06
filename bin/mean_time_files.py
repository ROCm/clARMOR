#!/usr/bin/env python
# coding=utf-8

import sys
import math


if ((len(sys.argv)) < 5 or (len(sys.argv) > 5)):
    print 'Please give 4 arguments'
    print '\t1st argument: time file name '
    print '\t2nd argument: start index.'
    print '\t3rd argument: stop index.'
    print '\t4th argument: index increment.'
    quit(-1)

timefile = sys.argv[1]
start = int(sys.argv[2])
stop = int(sys.argv[3])
increment = int(sys.argv[4])

data = {}
benchmarks = []
first = True
for i in range(start, stop+1, increment):
    f_name = "%s%u.csv" % (timefile, i)

    try:
        f = open(f_name, 'r')
    except IOError:
        print 'Cannot open input file', f_name
        quit(-1)

    for line in f:
        if (line == '\n'):
            continue

        linevals = line.strip('\n').rsplit(', ')

        if first:
            benchmarks.append(linevals[0])

        if(linevals[0] in data):
            for field_i in range(1, len(linevals)):
                data[linevals[0]][field_i-1] *= float(linevals[field_i])
        else:
            data[linevals[0]] = []
            for field_i in range(1, len(linevals)):
                data[linevals[0]].append(float(linevals[field_i]))

    first = False
    f.close()

for bench in data:
    for i in range(0, len(data[bench])):
        data[bench][i] = math.pow(data[bench][i], 1.0 / (float(stop+1 - start) / increment))


out_name = "%s%s" % (timefile, '_mean.csv')
try:
    out_f = open(out_name, 'w')
except IOError:
    print 'Cannot open output file', out_f
    quit(-1)

strout = ""
for bench in benchmarks:
    strout += bench
    for item in data[bench]:
        strout += ', %f' % item
    strout += '\n'


out_f.write(strout)
out_f.close()

