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
# The apps can be run with clarmor --group=NPB_OCL

# Licensing Information:
# SNU NAS Parallel Benchmarks use a variant of the MIT license, the important
# part of which is copied here for posterity.
#   Permission to use, copy, distribute and modify this software for any
#   purpose with or without fee is hereby granted. This software is
#   provided "as is" without express or implied warranty.
#
# See, for example, SNU_NPB-1.0.3/NPB3.3-OCL/BT/bt.c

BASE_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

if [ ! -d ~/benchmarks/SNU_NPB-1.0.3 ]; then
    source ${BASE_DIR}/setup_bench_install.sh
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
    sed -i.bak 's#C_INC = #C_INC = -I'${OCL_INCLUDE_DIR}' #' ./config/make.def

    # Patch CG to remove a broken OpenCL set of kernels that cause
    # non-deterministic behavior and random buffer underflows / crashes
    pushd ~/benchmarks/SNU_NPB-1.0.3/NPB3.3-OCL/CG/
    patch cg.c ${BASE_DIR}/../support_files/npb_ocl_cg.patch
    popd

    for bench in BT CG EP FT IS LU MG SP
    do
        if [ $bench == "LU" ]; then
            class=S
        else
            class=A
        fi
        # Because Nvidia's OpenCL runtime does not support including .h
        # files with #ifdef (the -D defines are parsed after the include
        # happens), this code manually "opens up" the header values that
        # we want to use in each benchmark.
        sed -i.bak "s/#define __"${bench}"_H__/#define __"${bench}"_H__\n#ifdef CLASS\n#undef CLASS\n#endif\n#define CLASS '"${class}"'\n/" ~/benchmarks/SNU_NPB-1.0.3/NPB3.3-OCL/${bench}/`echo "${bench,,}"`.h

        # Don't do this build in parallel. The makefiles for this have a
        # race between creating a sysconfig file and building the actual
        # benchmarks. It's easier to just not do a parallel build than to
        # fix this in each makefile.
        LIBRARY_PATH=${OCL_LIB_DIR} make $bench CLASS=$class
        if [ $? -ne 0 ]; then
            echo -e "Failed to build $bench class $class"
            exit -1
        fi
    done
else
    echo -e "~/benchmarks/SNU_NPB-1.0.3 exists. Not rebuilding the OpenCL version of NPB."
fi
