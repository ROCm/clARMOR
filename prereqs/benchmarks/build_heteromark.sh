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


# This script will download a version of Hetero-Mark from GitHub
# and build it into the ~/benchmarks/Hetero-Mark/ directory
# The apps can be run with clarmor --group=HETERO-MARK

# Licensing Information:
# Hetero-Mark is available under the MIT license.
# https://github.com/NUCAR-DEV/Hetero-Mark/blob/Develop/LICENSE

BASE_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

source ${BASE_DIR}/setup_bench_install.sh
if [ ! -d ~/benchmarks/Hetero-Mark/build/src/opencl12 ] || ( [ $CL_V2_SUPPORTED -eq 1 ] && [ ! -d ~/benchmarks/Hetero-Mark/build/src/opencl20 ] ); then
	if [ ! -d ~/benchmarks/Hetero-Mark ]; then
		echo -e "\n\nAbout to log into GitHub to get Hetero-Mark:"
		git clone -b amd_fixes https://github.com/jlgreathouse/Hetero-Mark.git
	fi
	cd Hetero-Mark
	mkdir build
	cd build
	cmake ..

	# Only build all of Hetero-Mark if we support OpenCL 2.0 on this machine.
	if [ $CL_V2_SUPPORTED -eq 1 ]; then
        make -j `nproc`
        if [ $? -ne 0 ]; then
            echo -e "Failed to build Hetero-Mark"
            exit -1
        fi
		if [ ! -f ~/benchmarks/Hetero-Mark/build/src/opencl20/sw_cl20/sw_Kernels.cl ]; then
			cd ~/benchmarks/Hetero-Mark/build/src/opencl20/sw_cl20/
			ln -s ./sw_cl20_kernel.cl sw_Kernels.cl
		fi
		if [ ! -f ~/benchmarks/Hetero-Mark/build/src/opencl20/aes_cl20/aes_Kernels.cl ]; then
			cd ~/benchmarks/Hetero-Mark/build/src/opencl20/aes_cl20/
			ln -s ./aes_cl20_kernel.cl ./aes_Kernels.cl
		fi
    else
        echo -e "This system does not support OpenCL 2.0. Skipping Hetero-Mark CL2.0 build."
		for bench in aes_cl12 fir_cl12 hmm_cl12 iir_cl12 kmeans_cl12 pagerank_cl12 sw_cl12; do
			make -j `nproc` $bench
			if [ $? -ne 0 ]; then
				echo -e "Failed to build Hetero-Mark"
				exit -1
			fi
		done
    fi

    if [ ! -f ~/benchmarks/Hetero-Mark/build/src/opencl12/sw_cl12/sw_Kernels.cl ]; then
        cd ~/benchmarks/Hetero-Mark/build/src/opencl12/sw_cl12/
        ln -s ./sw_cl12_kernel.cl sw_Kernels.cl
    fi
    if [ ! -f ~/benchmarks/Hetero-Mark/build/src/opencl12/aes_cl12/aes_cl12_Kernels.cl ]; then
        cd ~/benchmarks/Hetero-Mark/build/src/opencl12/aes_cl12/
        ln -s ./aes_cl12_kernel.cl ./aes_cl12_Kernels.cl
    fi
else
    echo -e "~/benchmarks/Hetero-Mark/build/src/opencl* exists. Not rebuilding Hetero-Mark."
fi
if [ ! -f ~/benchmarks/Hetero-Mark/data/aes/detector_input.txt ]; then
    cd ~/benchmarks/Hetero-Mark/data/aes/
    echo -e "Generating data for Hetero-Mark aes"
    ./datagen 134217728 > detector_input.txt
    ./datagen 8388608 > detector_medium_input.txt
    ./datagen 1048576 > detector_small_input.txt
    echo -e "00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f 10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f" > key.txt
fi
if [ ! -f ~/benchmarks/Hetero-Mark/data/kmeans/1500000_34.txt ]; then
    cd ~/benchmarks/Hetero-Mark/data/kmeans/
    echo -e "Generating data for Hetero-Mark kmeans"
    ./datagen 1500000
fi
if [ ! -f ~/benchmarks/Hetero-Mark/data/page_rank/csr_9216_10.txt ]; then
    cd ~/benchmarks/Hetero-Mark/data/page_rank/
    echo -e "Generating data for Hetero-Mark Page Rank"
    python ./datagen.py
fi
