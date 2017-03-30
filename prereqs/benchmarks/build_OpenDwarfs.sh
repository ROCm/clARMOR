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


# This script will download a version of the OpenDwarfs benchmark suite
# from GitHub and build it into the ~/benchmarks/OpenDwarfs directory.
# The apps can be run with clarmor --group=OPENDWARFS

# License Information:
# OpenDwarfs is made available under the LGPL v2.1.
# See https://github.com/vtsynergy/OpenDwarfs/blob/master/LICENSE

BASE_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

if [ ! -d ~/benchmarks/OpenDwarfs/build ];
then
    source ${BASE_DIR}/setup_bench_install.sh
    if [ ! -d ~/benchmarks/OpenDwarfs ]; then
        echo -e "\n\nAbout to log into GitHub to get OpenDwarfs:"
        git clone https://github.com/vtsynergy/OpenDwarfs.git
    fi
    cd OpenDwarfs/
    git checkout 31c099aff5343e93ba9e8c3cd42bee5ec536aa93
    # Set the default to device (third arg here) to be GPUs instead of CPUs.
    # As of 3/9/2016, the default version of OpenDwarfs does not make this command-line changable
    sed -i.bak s'/0, -1, 0, 0/0, -1, 1, 0/' ./include/common_args.c 
    ./autogen.sh

    sed -i.bak -e '1300,${s/return;/return 0;/}' ./graphical-models/hmm/main_bwa_hmm.c

    mkdir build
    cd build
    sed -i.bak s'/-Werror//g' ../Makefile.in
    ../configure --with-opencl-sdk=${OCL_DIR} --with-apps=bwa_hmm,crc,csr,gem,nqueens,swat,tdm
    sed -i.bak s'/LIBS = -lOpenCL/LIBS = -lm -lOpenCL/' ./Makefile
    sed -i.bak s'/CFLAGS =/CFLAGS = -g -O3/' ./Makefile
    make -j `nproc`
    if [ $? -ne 0 ]; then
        echo -e "Failed to build OpenDwarfs"
        exit -1
    fi
    # Generate large inputs for CRC and CSR
    ./createcrc -n 256 -s 131072 -f ./crcfile_N256_S128K
    ./createcsr -n 131072 -d 1000 -s 0.01 -f ./csr_n131072_d1000_s01
else
    echo -e "~/benchmarks/OpenDwarfs/build exists. Not rebuilding OpenDwarfds"
fi
