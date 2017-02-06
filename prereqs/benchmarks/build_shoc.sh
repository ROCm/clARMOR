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


# This script will download the SHOC benchmark suite from GitHub and build it
# into the ~/benchmarks/shoc directory.
# The apps can be run with run_overflow_detect.py --group=SHOC

# Licensing Information:
# SHOC is distributed under a 3-clause BSD license. See shoc/LICENSE.txt

if [ ! -d ~/benchmarks ]; then
    mkdir -p ~/benchmarks
fi

cd ~/benchmarks

if [ ! -f ~/benchmarks/shoc/Makefile ]; then
    if [ ! -d ~/benchmarks/shoc ]; then
        echo -e "\n\nAbout to log into GitHub to get SHOC:"
        git clone https://github.com/vetter/shoc.git
    fi
    cd shoc
    git checkout 9432edb93c9a146fef3e2022e0de7733f3ffe725
    CFLAGS="-g -O3 -I/opt/AMDAPP/include/" LDFLAGS="-L/opt/AMDAPP/lib/x86_64/" LIBS="-lOpenCL" CPPFLAGS="-g -O3 -I/opt/AMDAPP/include/" ./configure --with-opencl --without-cuda --without-mpi
    make -j `nproc`
    if [ $? -ne 0 ]; then
        echo -e "Failed to build SHOC."
        exit -1
    fi
    cd ~/benchmarks/shoc/src/opencl/level1/bfs/
    wget http://www.cc.gatech.edu/dimacs10/archive/data/matrix/ldoor.graph.bz2
    bunzip2 ldoor.graph.bz2
    cd ~/benchmarks/shoc/src/opencl/level1/spmv/
    wget http://www.cise.ufl.edu/research/sparse/MM/Williams/pdb1HYS.tar.gz
    tar -xf pdb1HYS.tar.gz
    mv ./pdb1HYS/pdb1HYS.mtx .
else
    echo -e "~/benchmarks/shoc/Makefile exists. Not rebuilding SHOC."
fi
