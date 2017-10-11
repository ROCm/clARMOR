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


# This script will copy the application code from the AMD APP SDK into
# the ~/benchmarks/AMDAPP/ folder so that we can make some
# performance-enhancing modifications (mostly putting data in the right
# side of the PCIe bus).
# The apps can be run with clarmor --group=AMDAPP

# Licensing Information:
# The benchmarks in the AMD APP SDK are made available under the AMD Software
# Development Kit License Agreement.

BASE_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
source ${BASE_DIR}/setup_bench_install.sh

if [ ! -d ~/benchmarks/AMDAPP/AMDAPP_install/ ]; then
    cd ~/benchmarks/
    mkdir -p ~/benchmarks/AMDAPP/
    cd ~/benchmarks/AMDAPP/
    ${BASE_DIR}/../support_files/get_amd_app_sdk.sh -d $(pwd)
    mv ./AMDAPP/ ./AMDAPP_install/
fi
cd ~/benchmarks/AMDAPP/

if [ ! -d ~/benchmarks/AMDAPP/BlackScholes/ ]; then
    echo -e "Copying benchmarks out into main AMDAPP directory..."
    cp -R ~/benchmarks/AMDAPP/AMDAPP_install/samples/opencl/benchmark/* .
    if [ -d ~/benchmarks/AMDAPP/AMDAPP_install/samples/opencl/cl/1.x/ ]; then
        cp -R ~/benchmarks/AMDAPP/AMDAPP_install/samples/opencl/cl/1.x/* .
        cp -R ~/benchmarks/AMDAPP/AMDAPP_install/samples/opencl/cpp_cl/1.x/* .
        if [ $CL_V2_SUPPORTED -eq 1 ]; then
            cp -R ~/benchmarks/AMDAPP/AMDAPP_install/samples/opencl/cl/2.0/* .
        fi
    else
        cp -R ~/benchmarks/AMDAPP/AMDAPP_install/samples/opencl/cl/* .
        cp -R ~/benchmarks/AMDAPP/AMDAPP_install/samples/opencl/cpp_cl/* .
    fi
fi

# The changes below with inMemFlags prevents the APP SDK application
# from putting its data into host memory. Actually, you want these to be
# in device memory.
BENCH_CL1="AdvancedConvolution BinomialOption BitonicSort BlackScholes BlackScholesDP DCT DwtHaar1D EigenValue FastWalshTransform FloydWarshall FluidSimulation2D GaussianNoise HDRToneMapping Histogram ImageOverlap Mandelbrot MatrixMultiplication MatrixMulDouble MatrixTranspose MersenneTwister MonteCarloAsian MonteCarloAsianDP NBody PrefixSum QuasiRandomSequence RadixSort RecursiveGaussian Reduction ScanLargeArrays SimpleConvolution SobelFilter StringSearch UnsharpMask URNG"
BENCH_CL2="BinarySearchDeviceSideEnqueue BufferBandwidth BufferImageInterop CalcPie DeviceEnqueueBFS ExtractPrimes ImageBandwidth ImageBinarization KmeansAutoclustering RangeMinimumQuery SVMBinaryTreeSearch"
if [ $CL_V2_SUPPORTED -eq 1 ]; then
    BENCH="$BENCH_CL1 $BENCH_CL2"
else
    BENCH="$BENCH_CL1"
fi

if [ ! -f /usr/lib/libOpenCL.so.1 ]; then
    rm -f ~/benchmarks/AMDAPP/AMDAPP_install/lib/x86_64/libOpenCL.so
    ln -s ${OCL_LIB_DIR}/libOpenCL.so ~/benchmarks/AMDAPP/AMDAPP_install/lib/x86_64/libOpenCL.so
fi

BENCH_TO_BUILD=""
for to_check in $BENCH
do
    if [ ! -f ~/benchmarks/AMDAPP/$to_check/$to_check ]; then
        BENCH_TO_BUILD="$BENCH_TO_BUILD $to_check"
    fi
done

AMDAPPSDKROOT=../AMDAPP_install
if [ BENCH_TO_BUILD != "" ]; then
    echo "Trying to build $BENCH_TO_BUILD"
    for i in $BENCH_TO_BUILD; do
        sed -i.bak s'/^        inMemFlags/        \/\/inMemFlags/' ./$i/$i.cpp
        if [ ! -d ~/benchmarks/AMDAPP/$i ] || [ -f ~/benchmarks/AMDAPP/$i/bin/x86_64/Release/$i ]; then
            echo -e "Skipping $i..."
            continue
        fi
        cd ~/benchmarks/AMDAPP/$i
        sed -i.bak s'#../../../../../include/#../AMDAPP_install/include/#g' ./CMakeLists.txt
        sed -i.bak s'#../../../../../lib/#'${OCL_LIB_DIR}'#g' ./CMakeLists.txt
        sed -i.bak s'#../../../../include/#../AMDAPP_install/include/#g' ./CMakeLists.txt
        sed -i.bak s'#../../../../lib/#'${OCL_LIB_DIR}'#g' ./CMakeLists.txt
        cmake .
        make -j `nproc`
        if [ $? -ne 0 ]; then
            echo -e "Failed to build $i"
            exit -1
        fi
        cd ~/benchmarks/AMDAPP/
    done
	if [ ! -f ~/benchmarks/AMDAPP/BoxFilter/bin/x86_64/Release/BoxFilter ]; then
		sed -i.bak s'/^        inMemFlags/        \/\/inMemFlags/' ./BoxFilter/BoxFilterSeparable.cpp
		sed -i.bak s'/^        inMemFlags/        \/\/inMemFlags/' ./BoxFilter/BoxFilterSAT.cpp
		cd ~/benchmarks/AMDAPP/BoxFilter/
		sed -i.bak s'#../../../../../include/#../AMDAPP_install/include/#g' ./CMakeLists.txt
		sed -i.bak s'#../../../../../lib/#'${OCL_LIB_DIR}'#g' ./CMakeLists.txt
		cmake .
		make -j `nproc`
		if [ $? -ne 0 ]; then
			echo -e "Failed to build BoxFilter"
			exit -1
		fi
	fi
else
    echo -e "~/benchmarks/AMDAPP exists. Not rebuilding AMD APP SDK samples."
fi
