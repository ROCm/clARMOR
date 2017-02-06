#!/bin/bash
# Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.


# This script will download FinanceBench from GitHub and build it into the
# ~/benchmarks/FinanceBench directory.
# The apps can be run with run_overflow_detect.py --group=FINANCEBENCH

# Licensing Information:
# FinanceBench uses a 3-clause BSD license. See FinanceBench/LICENSE

MAKE_CMD="make"

if [ ! -d ~/benchmarks ]; then
    mkdir -p ~/benchmarks
fi

cd ~/benchmarks/

if [ ! -f ~/benchmarks/FinanceBench/done_building ]; then
    if [ ! -d ~/benchmarks/FinanceBench/ ]; then
        echo -e "\n\nAbout to log into GitHub to get FinanceBench" 
        git clone https://github.com/cavazos-lab/FinanceBench.git
    fi

    cd ~/benchmarks/FinanceBench/
    git checkout b56735b

    for bench in Black-Scholes Monte-Carlo
    do
        cd ~/benchmarks/FinanceBench/${bench}/OpenCL/
        sed -i.bak s'/\/usr\/local\/cuda-5.0\/include/\/opt\/AMDAPP\/include/' ./Makefile
        sed -i.bak s'/O3/g -O3/' ./Makefile
        sed -i.bak s'/\${LIBPATH}/-L \/opt\/AMDAPP\/lib\/x86_64\//' ./Makefile
        sed -i.bak s'/\${LIB}//' ./Makefile
        sed -i.bak s'/\-o/\${LIB} -o/' ./Makefile
        if [ $bench=="Black-Scholes" ]; then
            sed -i.bak s'/DBL_MIN/FLT_MIN/' ./blackScholesAnalyticEngineKernels.cl
            sed -i.bak s'/c =/\/\/c =/' ./blackScholesAnalyticEngine.c
            sed -i.bak s'/numVals = 5000000/numVals = 50000000' ./blackScholesAnalyticEngine.c
        fi
        if [ $bench=="Monte-Carlo" ]; then
            sed -i.bak s'/c =/\/\/c =/' ./monteCarloEngine.c
        fi
        $MAKE_CMD
        if [ $? -ne 0 ]; then
            echo -e "Failed to build $bench"
            exit -1
        fi
    done
    touch ~/benchmarks/FinanceBench/done_building
else
    echo -e "~/benchmarks/FinanceBench/done_building exists. Not rebuilding FinanceBench."
fi
