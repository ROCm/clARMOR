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


# This script will download a working version of the GPU-STREAM benchmark
# from GitHub and build it into the ~/benchmarks/GPU-STREAM directory.
# The apps can be run with clarmor.py --group=GPUSTREAM

# Licensing Information:
# GPU-STREAM is available under a permissive license that allows its use
# freely, unless you are publishing benchmark results from it.
# If publishing benchmark results that do not follow their official run rules:
#      Results based on modified source code or on runs not in
#      accordance with the GPU-STREAM Run Rules must be clearly
#      labelled whenever they are published.  Examples of
#      proper labelling include:
#      "tuned GPU-STREAM benchmark results"
#      "based on a variant of the GPU-STREAM benchmark code"
# https://github.com/UoB-HPC/GPU-STREAM/blob/master/LICENSE

BASE_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
source ${BASE_DIR}/setup_bench_install.sh

# Test to see if the first guy has been build, not all of them
# testing all benchmarks would be a big pain to write..
if [ ! -f ~/benchmarks/GPU-STREAM/gpu-stream-ocl ]; then
    if [ ! -d ~/benchmarks/GPU-STREAM ]; then
        echo -e "\n\nAbout to log into GitHub to get GPU-STREAM:"
        git clone https://github.com/UoB-HPC/GPU-STREAM.git
    fi
    cd ~/benchmarks/GPU-STREAM
    git checkout bbee43998514a8d618592adb90bcb1b27d4764e0

	if [ ! -z ${CUDA_LIB_PATH+x} ] || [ ! -z ${CUDA_PATH+x} ]; then
		sed -i.bak s"#O3#g -O3 -I "${OCL_INCLUDE_DIR}"#" ~/benchmarks/GPU-STREAM/Makefile
	else
		sed -i.bak s"#O3#g -O3 -I "${OCL_INCLUDE_DIR}" -L "${OCL_LIB_DIR}"#" ~/benchmarks/GPU-STREAM/Makefile
	fi
    make -j `nproc` gpu-stream-ocl
    if [ $? -ne 0 ]; then
        echo -e "Failed to build GPU-STREAM"
        exit -1
    fi
else
    echo -e "~/benchmarks/GPU-STREAM/gpu-strema-ocl exists. Not rebuilding GPU-STREAM."
fi
