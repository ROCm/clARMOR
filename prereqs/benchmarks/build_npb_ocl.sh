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


# This script will build v1.0.3 of the SNU OpenCL port of the NAS Parallel
# Benchmarks and put them into the ~/benchmarks/SNU_NPB-1.0.3 directory.
# The apps can be run with run_overflow_detect.py --group=NPB_OCL

# Licensing Information:
# SNU NAS Parallel Benchmarks use a variant of the MIT license, the important
# part of which is copied here for posterity.
#   Permission to use, copy, distribute and modify this software for any
#   purpose with or without fee is hereby granted. This software is
#   provided "as is" without express or implied warranty.
#
# See, for example, SNU_NPB-1.0.3/NPB3.3-OCL/BT/bt.c

if [ ! -d ~/benchmarks ]; then
    mkdir -p ~/benchmarks
fi

cd ~/benchmarks

if [ ! -d ~/benchmarks/SNU_NPB-1.0.3 ]; then
    if [ ! -f ~/benchmarks/SNU_NPB-1.0.3.tar.gz ]; then
        echo -e "Error. Could not find ~/benchmarks/SNU_NPB-1.0.3.tar.gz"
        echo -e "Downloading this requires entering information on a page."
        echo -e "As such, we cannot automatically download it for you."
        echo -e "Please go to the following site to download the file:"
        echo -e "    http://aces.snu.ac.kr/software/snu-npb/"
        echo -e "Then put it into ~/benchmarks/"
        exit -1
    fi
    tar -xvf SNU_NPB-1.0.3.tar.gz
    cd ~/benchmarks/SNU_NPB-1.0.3/NPB3.3-OCL/;
    sed -i.bak 's#C_INC = #C_INC = -I/opt/AMDAPP/include/ #' ./config/make.def
    for bench in BT CG EP FT IS LU MG SP
    do
        for class in S W A B C
        do
            # Don't do this build in parallel. The makefiles for this have a
            # race between creating a sysconfig file and building the actual
            # benchmarks. It's easier to just not do a parallel build than to
            # fix this in each makefile.
            make $bench CLASS=$class
            if [ $? -ne 0 ]; then
                echo -e "Failed to build $bench class $class"
                exit -1
            fi
        done
    done
else
    echo -e "~/benchmarks/SNU_NPB-1.0.3 exists. Not rebuilding the OpenCL version of NPB."
fi
