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


# This script will download the StreamMR OpenCL Map Reduce framework from
# GitHub and build it into the  ~/benchmarks/StreamMR directory.
# The apps can be run with clarmor --group=STREAMMR

# Licensing Information:
# StreamMR is made available under the LGPL v2.1. See StreamMR/LICENSE

BASE_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

if [ ! -f ~/benchmarks/StreamMR/KMeans ] || \
    [ ! -f ~/benchmarks/StreamMR/MatrixMul ] || \
    [ ! -f ~/benchmarks/StreamMR/StringMatch ] || \
    [ ! -f ~/benchmarks/StreamMR/WordCount ];
then
    source ${BASE_DIR}/setup_bench_install.sh
    if [ ! -d ~/benchmarks/StreamMR ]; then
        echo -e "\n\nAbout to log into GitHub to get StreamMR:"
        git clone https://github.com/jlgreathouse/StreamMR.git
    fi
    cd StreamMR
    sed -i.bak 's#/opt/OCLSDK#'${OCL_DIR}'#g' ./Makefile
    make -j `nproc`
    if [ $? -ne 0 ]; then
        echo -e "Failed to build StreamMR."
        exit -1
    fi
    cd ./test/
    echo "Generating input data..."
    bunzip2 generator.pl.bz2
    ./generator.pl 1000000 > 1mil.txt
else
    echo -e "~/benchmarks/StreamMR/KMeans, MatrixMul, StringMatch, and WordCount exist. Not rebuilding StreamMR."
fi
