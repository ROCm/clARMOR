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


# This script will download Rodinia 3.1 and build it into the
# ~/benchmarks/rodinia_3.1/ directory.
# The apps can be run with clarmor --group=RODINIA

# Licensing Information:
# Rodinia is made available under a 3-clause BSD license.
# See rodinia_3.1/LICENSE

BASE_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

if [ ! -d ~/benchmarks/rodinia_3.1 ]; then
    source ${BASE_DIR}/setup_bench_install.sh
    if [ ! -f ~/benchmarks/rodinia_3.1.tar.bz2 ];
    then
        cd ~/benchmarks/
        echo -e "\n\nAbout to download Rodinia 3.1 from virginia.edu"
        wget http://www.cs.virginia.edu/~kw5na/lava/Rodinia/Packages/Current/rodinia_3.1.tar.bz2
        if [ $? -ne 0 ]; then
            echo -e "Failed to download file"
            exit -1
        fi
    fi
    tar -xvf rodinia_3.1.tar.bz2
    cd rodinia_3.1/
    sed -i.bak s'#OPENCL_DIR = /if10/kw5na/Packages/AMD-APP-SDK-v2.8-RC-lnx64#OPENCL_DIR = '${OCL_DIR}'#' ./common/make.config
    sed -i.bak s'#OPENCL_INC = $(OPENCL_DIR)/include/#OPENCL_INC = '${OCL_INCLUDE_DIR}'#' ./common/make.config
    sed -i.bak s'#OPENCL_LIB = $(OPENCL_DIR)/lib/x86_64/#OPENCL_LIB = '${OCL_LIB_DIR}'#' ./common/make.config
    sed -i.bak s'/opencl.h/cl.h/' ./opencl/dwt2d/main.cpp
    sed -i.bak s'/platformIds\[1/platformIds\[0/' ./opencl/dwt2d/main.cpp
    sed -i.bak s"#/usr/local/cuda-5.5/include#"${OCL_INCLUDE_DIR}"#" ./opencl/dwt2d/Makefile
    sed -i.bak s'/^}$/return 0;\n}/' ./opencl/heartwall/main.c
    sed -i.bak s'/^}$/return 0;\n}/' ./opencl/srad/main.c
    sed -i.bak s'/platformID\[1/platformID\[0/' ./opencl/hybridsort/bucketsort.c
    sed -i.bak s'/properties)platformID\[num/properties)platformID[0/' ./opencl/hybridsort/bucketsort.c
    sed -i.bak s'/platformID\[1/platformID\[0/' ./opencl/hybridsort/mergesort.c
    sed -i.bak s'/resStart\[outidx++\] = b;//' ./opencl/hybridsort/mergesort.cl
    sed -i.bak s'/cd opencl\/backprop/cd opencl\/b+tree;           make;   cp b+tree.out \$(OPENCL_BIN_DIR)\n\tcd opencl\/backprop/' ./Makefile
    sed -i.bak s'/cd opencl\/nw/cd opencl\/myocyte;         make;   cp myocyte.out \$(OPENCL_BIN_DIR)\n\tcd opencl\/nw/' ./Makefile
    rm -f ./opencl/kmeans/unistd.h
    #fix bug where .o files aren't building for new gcc
    sed -i.bak s'/\%.o: \%\.\[ch\]/\%.o: \%\.\[c\]/' ./opencl/leukocyte/OpenCL/Makefile
    #fix for missing path to ocl lib dir
    sed -i.bak s'/\$(OCL_LIB)/-L\$(OPENCL_LIB)/' ./opencl/dwt2d/Makefile
    #fix bug where return type was not known at compile time, leading to a
    #segfault in GCC 5.4.0
    sed -i.bak '28i#include "../util/opencl/opencl.h"' ./opencl/b+tree/kernel/kernel_gpu_opencl_wrapper_2.c

    # Add debug flags to all builds that don't have them
    sed -i.bak s'/O3/g -O3/' ./opencl/b+tree/Makefile
    sed -i.bak s'/O3/g -O3/' ./opencl/cfd/Makefile
    sed -i.bak s'/g++/g++ -g -O3/' ./opencl/dwt2d/Makefile
    sed -i.bak s'/O3/g -O3/' ./opencl/heartwall/makefile
    sed -i.bak s'/O3/g -O3/' ./opencl/hotspot3D/Makefile
    sed -i.bak s'/c99/c99 -g -O3/' ./opencl/hybridsort/Makefile
    sed -i.bak s'/O3/g -O3/' ./opencl/lavaMD/makefile
    sed -i.bak s'/O3/g -O3/' ./opencl/myocyte/Makefile
    sed -i.bak s'/O2/g -O2/' ./opencl/particlefilter/Makefile
    sed -i.bak s'/O3/g -O3/' ./opencl/srad/makefile
    sed -i.bak s'/O3/g -O3/' ./opencl/streamcluster/Makefile

    make -j `nproc` OPENCL
    if [ $? -ne 0 ]; then
        echo -e "Failed to build Rodinia."
        exit -1
    fi
else
    echo -e "~/benchmarks/rodinia_3.1 exists. Not rebuilding Rodinia."
fi
