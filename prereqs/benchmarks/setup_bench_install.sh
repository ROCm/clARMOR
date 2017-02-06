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

# This script will set up bash variables for other scripts that generically
# describe where the OpenCL installation is. Other scripts should use the
# OCL_* variables instead of the vendor-specific SDK variables.
# It will also install the AMD APP SDK so that we can get access to the
# AMD 'clinfo' tool to learn about which versions of OpenCL are supported
# on this system.

# Licensing Information:
# The AMD APP SDK is made available under the AMD Software Development
# Kit License Agreement.

AMD_OCL=0
NV_OCL=0
INT_OCL=0

if [ ! -z ${AMDAPPSDKROOT+x} ]; then
    OCL_DIR=${AMDAPPSDKROOT}
    OCL_INCLUDE_DIR=${AMDAPPSDKROOT}/include
    OCL_LIB_DIR=${AMDAPPSDKROOT}/lib/x86_64
	AMD_OCL=1
elif [ ! -z ${ATISTREAMSDKROOT+x} ]; then
    OCL_DIR=${ATISTREAMSDKROOT}
    OCL_INCLUDE_DIR=${ATISTREAMSDKROOT}/include
    OCL_LIB_DIR=${ATISTREAMSDKROOT}/lib/x86_64
	AMD_OCL=1
elif [ ! -z ${INTELOCLSDKROOT+x} ]; then
    OCL_DIR=${INTELOCLSDKROOT}
    OCL_INCLUDE_DIR=${INTELOCLSDKROOT}/include
    OCL_LIB_DIR=${INTELOCLSDKROOT}/lib/x86_64
	INT_OCL=1
elif [ ! -z ${CUDA_INC_PATH+x} ]; then
    OCL_DIR=${CUDA_INC_PATH}/../
    OCL_INCLUDE_DIR=${CUDA_INC_PATH}
    OCL_LIB_DIR=${CUDA_LIB_PATH}
	NV_OCL=1
elif [ ! -z ${CUDA_PATH+x} ]; then
    OCL_DIR=${CUDA_PATH}
    OCL_INCLUDE_DIR=${CUDA_PATH}/include
    OCL_LIB_DIR=${CUDA_PATH}/lib64
	NV_OCL=1
else
    echo -e "OpenCL environment variables are not set, so we cannot build this benchmarks."
    echo -e "Please set one of the following environment variables:"
    echo -e "AMDAPPSDKROOT, ATISTREAMSDKROOT, INTELOCLSDKROOT, CUDA_PATH, or CUDA_INC_PATH/CUDA_LIB_PATH"
    exit -1
fi

if [ ! -d ~/benchmarks ]; then
    mkdir -p ~/benchmarks
fi


cd ~/benchmarks

CL_V2_SUPPORTED=0

if [ ! -d ~/benchmarks/AMDAPP/AMDAPP_install ];
then
    mkdir -p ~/benchmarks/AMDAPP
    cd ~/benchmarks/AMDAPP
    echo -e "Checking if the AMD APP SDK files exist."
    if [ ! -f ~/benchmarks/AMD-APP-SDKInstaller-v3.0.130.136-GA-linux64.tar.bz2 ] && [ ! -f ~/benchmarks/AMDAPP/AMD-APP-SDKInstaller-v3.0.130.136-GA-linux64.tar.bz2 ]; then
        echo -e "Error. Could not find ~/benchmarks/AMD-APP-SDKInstaller-v3.0.130.136-GA-linux64.tar.bz2"
        echo -e "Downloading the AMD APP SDK requires manually agreeing to a license."
        echo -e "As such, we cannot automatically download it for you."
        echo -e "Please download the AMD APP SDK from:"
        echo -e "     http://developer.amd.com/tools-and-sdks/opencl-zone/amd-accelerated-parallel-processing-app-sdk/"
        echo -e "Then put it into ~/benchmarks/"
        echo -e "You should do this even if you are using an OpenCL runtime from someone else."
        echo -e "The AMD APP SDK includes benchmarks that we automatically test against."
        exit -1
    fi

    echo -e "Moving AMD APP SDK installer to ~/benchmarks/AMDAPP/ and untarring it."
    cp ~/benchmarks/AMD-APP-SDKInstaller-v3.0.130.136-GA-linux64.tar.bz2 ~/benchmarks/AMDAPP/
    tar -xf AMD-APP-SDKInstaller-v3.0.130.136-GA-linux64.tar.bz2

    echo -e "Running AMD APP SDK local installer..."
    ./AMD-APP-SDK-v3.0.130.136-GA-linux64.sh --noexec --nox11 --target $(pwd)/AMDAPP_install/
	if [ $? -ne 0 ]; then
		echo -e "Failed to properly install the AMD APP SDK. Quitting."
		exit -1
	fi
fi

# Note that this only finds the OpenCL version of the first available OpenCL device.
echo -e "Checking available OpenCL versions. Please wait..."
CL_AVAIL_VERSION=`~/benchmarks/AMDAPP/AMDAPP_install/bin/x86_64/clinfo | grep "^  Version:" | awk '{print $3}' | head -n 1`
if (( $(echo "$CL_AVAIL_VERSION > 1.2" | bc -l) )); then
    CL_V2_SUPPORTED=1
fi

cd ~/benchmarks/
