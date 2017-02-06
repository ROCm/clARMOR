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
# The apps can be run with run_overflow_detect.py --group=AMDAPP

# Licensing Information:
# The benchmarks in the AMD APP SDK are made available under the AMD Software
# Development Kit License Agreement.

if [ ! -d ~/benchmarks ]; then
    mkdir -p ~/benchmarks
fi

cd ~/benchmarks

if [ ! -d ~/benchmarks/AMDAPP ];
then
    mkdir -p ~/benchmarks/AMDAPP
    cd ~/benchmarks/AMDAPP
    cp -R /opt/AMDAPP/samples/opencl/benchmark/* .
    if [ -d /opt/AMDAPP/samples/opencl/cl/1.x/ ]; then
        cp -R /opt/AMDAPP/samples/opencl/cl/1.x/* .
        cp -R /opt/AMDAPP/samples/opencl/cpp_cl/1.x/* .
        cp -R /opt/AMDAPP/samples/opencl/cl/2.0/* .
    else
        cp -R /opt/AMDAPP/samples/opencl/cl/* .
        cp -R /opt/AMDAPP/samples/opencl/cpp_cl/* .
    fi

    # The changes below with inMemFlags prevents the APP SDK application
    # from putting its data into host memory. Actually, you want these to be
    # in device memory.
    for i in AdvancedConvolution BinarySearchDeviceSideEnqueue BinomialOption BitonicSort BlackScholes BlackScholesDP BufferBandwidth BufferImageInterop CalcPie DCT DeviceEnqueueBFS DwtHaar1D EigenValue ExtractPrimes FastWalshTransform FloydWarshall FluidSimulation2D GaussianNoise HDRToneMapping Histogram ImageBandwidth ImageBinarization ImageOverlap KmeansAutoclustering Mandelbrot MatrixMultiplication MatrixMulDouble MatrixTranspose MersenneTwister MonteCarloAsian MonteCarloAsianDP NBody PrefixSum QuasiRandomSequence RadixSort RangeMinimumQuery RecursiveGaussian Reduction ScanLargeArrays SimpleConvolution SobelFilter StringSearch SVMBinaryTreeSearch UnsharpMask URNG; do
        sed -i.bak s'/^        inMemFlags/        \/\/inMemFlags/' ./$i/$i.cpp
        cd ~/benchmarks/AMDAPP/$i
        cmake .
        make -j `nproc`
        cd ~/benchmarks/AMDAPP/
    done
    sed -i.bak s'/^        inMemFlags/        \/\/inMemFlags/' ./BoxFilter/BoxFilterSeparable.cpp
    sed -i.bak s'/^        inMemFlags/        \/\/inMemFlags/' ./BoxFilter/BoxFilterSAT.cpp
    cd ~/benchmarks/AMDAPP/BoxFilter/
    cmake .
    make -j `nproc`
    if [ $? -ne 0 ]; then
        echo -e "Failed to build AMD APP SDK Samples"
        exit -1
    fi
else
    echo -e "~/benchmarks/AMDAPP exists. Not rebuilding AMD APP SDK samples."
fi
