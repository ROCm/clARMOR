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


# This script will build version 2.5 of the Parboil benchmark suite and put it
# into the ~/benchmarks/parboil directory.
# The apps can be run with clarmor --group=PARBOIL

# Licensing Information:
# Parboil is available under the Illinois Open Source License (a.k.a. the NCSA
# or LLVM license).
# See: http://impact.crhc.illinois.edu/parboil/parboil_download_page.aspx
# and: parboil/LICENSE

BASE_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

if [ ! -d ~/benchmarks/parboil ]; then
    source ${BASE_DIR}/setup_bench_install.sh
    if [ ! -f ~/benchmarks/pb2.5driver.tgz ]; then
        echo -e "Error. Could not find ~/benchmarks/pb2.5driver.tgz"
        echo -e "Downloading Parboil requires manually agreeing to a license."
        echo -e "As such, we cannot automatically download it for you."
        echo -e "Please download the Parboil driver from:"
        echo -e "     http://impact.crhc.illinois.edu/parboil/parboil.aspx"
        echo -e "Then put it into ~/benchmarks/"
        exit -1
    fi
    if [ ! -f ~/benchmarks/pb2.5benchmarks.tgz ] && [ ! -f ~/benchmarks/parboil/pb2.5benchmarks.tgz ]; then
        echo -e "Error. Could not find pb2.5benchmarks.tgz in either:"
        echo -e "~/benchmarks/ or ~/benchmarks/parboil/"
        echo -e "Downloading Parboil requires manually agreeing to a license."
        echo -e "As such, we cannot automatically download it for you."
        echo -e "Please download the Parboil benchmarks from:"
        echo -e "     http://impact.crhc.illinois.edu/parboil/parboil.aspx"
        echo -e "Then put it into ~/benchmarks/"
        exit -1
    fi
    if [ ! -f ~/benchmarks/pb2.5datasets_standard.tgz ] && [ ! -f  ~/benchmarks/parboil/pb2.5datasets_standard.tgz ]; then
        echo -e "Error. Could not find pb2.5datasets_standard.tgz in either:"
        echo -e "~/benchmarks/ or ~/benchmarks/parboil/"
        echo -e "Downloading Parboil requires manually agreeing to a license."
        echo -e "As such, we cannot automatically download it for you."
        echo -e "Please download the Parboil datasets from:"
        echo -e "     http://impact.crhc.illinois.edu/parboil/parboil.aspx"
        echo -e "Then put it into ~/benchmarks/"
        exit -1
    fi
    tar -xvf pb2.5driver.tgz
    if [ -f ~/benchmarks/pb2.5benchmarks.tgz ];
    then
        cp pb2.5benchmarks.tgz ./parboil
    fi
    if [ -f ~/benchmarks/pb2.5datasets_standard.tgz ];
    then
        cp pb2.5datasets_standard.tgz ./parboil
    fi
    cd ~/benchmarks/parboil;
    tar -xvf pb2.5benchmarks.tgz;
    tar -xvf pb2.5datasets_standard.tgz
    cp ./common/Makefile.conf.example-ati ./common/Makefile.conf
    sed -i.bak 's#/opt/ati/#'${OCL_DIR}'#g' ./common/Makefile.conf
    # Patches for mri-gridding.
    # This one prevents our compiler from complaining about being unable to compare an unsigned int vs. a signed int.
    # Previously, the kernel had incorrect parens around a thing they wanted to cast.
    sed -i.bak 's/(LNB)+(index)/((LNB)+(index))/' ./benchmarks/mri-gridding/src/opencl_base/sort.cl
    # This prevents a buffer overflow in the splitRearrange kernel.
    sed -i.bak 's#clCreateBuffer(clContext, CL_MEM_COPY_HOST_PTR, n\*#clCreateBuffer(clContext, CL_MEM_COPY_HOST_PTR, (((n+3)/4)*4)*#' ./benchmarks/mri-gridding/src/opencl_base/OpenCL_interface.cpp
    sed -i.bak 's#clCreateBuffer(clContext, CL_MEM_READ_WRITE, n\*#clCreateBuffer(clContext, CL_MEM_READ_WRITE, (((n+3)/4)*4)*#' ./benchmarks/mri-gridding/src/opencl_base/OpenCL_interface.cpp
    # Add debug symbols
    sed -i.bak 's/^CFLAGS=/CFLAGS=-g -O3 /' ./common/mk/opencl.mk
    # This moves the destroy timer BEFORE the release of the command queue that it relies on.
    # Otherwise, we are doing a use-after-free of an event from that command queue, which causes a crash.
    sed -i.bak 's#pb_DestroyTimerSet#//pb_DestroyTimerSet#' ./benchmarks/sad/src/opencl_base/main.cpp
    sed -i.bak 's#OCL_ERRCK_RETVAL( clReleaseCommandQueue#pb_DestroyTimerSet(\&timers);\n  OCL_ERRCK_RETVAL( clReleaseCommandQueue#' ./benchmarks/sad/src/opencl_base/main.cpp
	# Patch 'histo' so that it works on NV GPUs without running out of local memory space.
	sed -i.bak 's#lmemKB = 48#lmemKB = 32#' ./benchmarks/histo/src/opencl_base/main.cpp
	sed -i.bak 's#lmemKB = 24#lmemKB = 16#' ./benchmarks/histo/src/opencl_base/main.cpp
    for i in `./parboil list | grep "^ " | awk {'print $1'}`; do ./parboil compile $i opencl_base; done
    ./parboil compile mri-q opencl
else
    echo -e "~/benchmarks/parboil exists. Not rebuilding Parboil."
fi
