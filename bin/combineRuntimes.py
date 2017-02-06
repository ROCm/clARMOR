#!/usr/bin/env python
# coding=utf-8

import sys

if ((len(sys.argv)) < 3 or (len(sys.argv) > 4)):
    print 'Please give 3 arguments'
    print '\t1st argument: debug_enqueue_time.csv '
    print '\t2nd argument: debug_check_time.csv.'
    print '\t3rd argument: out file name.'
    quit(-1)

haskernel=1
if len(sys.argv) == 4:
    enqueue_times = sys.argv[1]
    check_times = sys.argv[2]
    out_times = sys.argv[3]
else:
    haskernel=0
    enqueue_times = sys.argv[1]
    out_times = sys.argv[2]

try:
    f = open(enqueue_times, 'r')
except IOError:
    print 'Cannot open input file', enqueue_times
    quit(-1)

if haskernel:
    try:
        f2 = open(check_times, 'r')
    except IOError:
        print 'Cannot open input file', check_times
        quit(-1)

try:
    out_f = open(out_times, 'w')
except IOError:
    print 'Cannot open output file', out_times
    quit(-1)

values=[]
runvals=[]
for line in f:
    linevals = line.strip('\n').rsplit(', ')

    if linevals[0] == '':
        values.append(runvals)
        runvals=[]
        continue

    run=[]
    for x in linevals:
        run.append(int(x))

    runvals.append(run)


if haskernel:
    values2=[]
    runvals=[]
    for line in f2:
        linevals = line.strip('\n').rsplit(', ')

        if linevals[0] == '':
            values2.append(runvals)
            runvals=[]
            continue

        run=[]
        for x in linevals:
            run.append(int(x))

        runvals.append(run)


results=[]
for i in range(0,len(values)):
    runvals = values[i]
    total_run_time=1
    enqueue_time=1
    for j in range(1,len(runvals)):
        total_run_time *= runvals[j][0]
        enqueue_time *= runvals[j][1]

    total_run_time = int(pow(total_run_time, 1.0 / (len(runvals) - 1)))
    enqueue_time = int(pow(enqueue_time, 1.0 / (len(runvals) - 1)))
    #if haskernel:
    compile_time = runvals[0][0] - runvals[0][1] - (total_run_time - enqueue_time)
    overhead = total_run_time - enqueue_time
    results.append([])
    #if haskernel:
    results[i].append(compile_time)
    results[i].append(total_run_time)
    results[i].append(enqueue_time)
    results[i].append(overhead)


if haskernel:
    for i in range(0, len(values2)):
        runvals = values2[i]
        kernel_time=1
        for j in range(0, len(runvals)):
            kernel_time *= runvals[j][0]

        kernel_time = int(pow(kernel_time, 1.0 / len(runvals)))
        results[i].append(kernel_time)

out_string=""
if haskernel:
    out_string += "compile time, total time, enqueue time, overhead, kernel time\n"
else:
    out_string += "first run cost, total time, enqueue time, overhead\n"

for i in range(0,len(results)):
    if haskernel:
        frag = "%lu, %lu, %lu, %lu, %lu\n" %(results[i][0], results[i][1], results[i][2], results[i][3], results[i][4])
    else:
        frag = "%lu, %lu, %lu, %lu\n" %(results[i][0], results[i][1], results[i][2], results[i][3])
        #frag = "%lu, %lu, %lu\n" %(results[i][0], results[i][1], results[i][2])
    out_string += frag

print out_string

out_f.write(out_string)

