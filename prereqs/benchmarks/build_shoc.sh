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
# The apps can be run with clarmor --group=SHOC

# Licensing Information:
# SHOC is distributed under a 3-clause BSD license. See shoc/LICENSE.txt

BASE_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

if [ ! -f ~/benchmarks/shoc/Makefile ]; then
    source ${BASE_DIR}/setup_bench_install.sh
    if [ ! -d ~/benchmarks/shoc ]; then
        echo -e "\n\nAbout to log into GitHub to get SHOC:"
        git clone https://github.com/vetter/shoc.git
    fi
    cd shoc
    git checkout 9432edb93c9a146fef3e2022e0de7733f3ffe725

	# Without this, we can end up with a "negative available memory size" which infinite loops
	sed -i.bak 's#int memSize#unsigned int memSize#' ./src/opencl/level0/DeviceMemory.cpp
	sed -i.bak 's#long availMem#unsigned long availMem#' ./src/opencl/level0/DeviceMemory.cpp
	# Sometimes global and local size don't divide evently, so remove local size.
	sed -i.bak 's#const size_t globalWorkSize#size_t globalWorkSize#' ./src/opencl/level1/spmv/Spmv.cpp
	sed -i.bak '332s#{#{ globalWorkSize += (localWorkSize - (globalWorkSize % localWorkSize));#' ./src/opencl/level1/spmv/Spmv.cpp
	sed -i.bak 's#const size_t scalarGlobalWSize#size_t scalarGlobalWSize#' ./src/opencl/level1/spmv/Spmv.cpp
	sed -i.bak '608s#{#{ scalarGlobalWSize += (localWorkSize - (scalarGlobalWSize % localWorkSize));#' ./src/opencl/level1/spmv/Spmv.cpp
	sed -i.bak 's#const size_t vectorGlobalWSize#size_t vectorGlobalWSize#' ./src/opencl/level1/spmv/Spmv.cpp
	sed -i.bak '686s#{#{ vectorGlobalWSize += (localWorkSize - (vectorGlobalWSize % localWorkSize));#' ./src/opencl/level1/spmv/Spmv.cpp


	if [ $NV_OCL -eq 1 ]; then
		CFLAGS="-g -O3 -I"${OCL_INCLUDE_DIR} LIBS="-lOpenCL" CPPFLAGS="-g -O3 -I"${OCL_INCLUDE_DIR} ./configure --with-opencl --without-cuda --without-mpi
	else
		CFLAGS="-g -O3 -I"${OCL_INCLUDE_DIR} LDFLAGS="-L"${OCL_LIB_DIR} LIBS="-lOpenCL" CPPFLAGS="-g -O3 -I"${OCL_INCLUDE_DIR} ./configure --with-opencl --without-cuda --without-mpi
	fi
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
