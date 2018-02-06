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


# This script will download hpl-gpu, a version of HPLINPACK that works on AMD
# GPUs. It will put everything in it into the ~/benchmarks/LINPACK directory.
# The apps can be run with clarmor --group=LINPACK

# Licensing Information:
# hpl-gpu is available under a mix of GPLv3 and 4-clause BSD. See:
#  https://github.com/davidrohr/hpl-gpu/blob/master/COPYING
# CALDGEMM is made available under GPLv3 and LGPL. See:
#  https://github.com/davidrohr/caldgemm/blob/master/COPYING
#  and https://github.com/davidrohr/caldgemm/blob/master/COPYING.LESSER
# ACML is available under the AMD ACML License Agreement.

BASE_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
source ${BASE_DIR}/setup_bench_install.sh

if [ $AMD_OCL -eq 1 ] && [ $AMD_HAWAII_GPU -eq 1 ] && [ ! -f ~/benchmarks/LINPACK/hpl-gpu/bin/Generic/xhpl ]; then
    if [ ! -d ~/benchmarks/LINPACK/ ]; then
        mkdir -p ~/benchmarks/LINPACK/
    fi
    cd ~/benchmarks/LINPACK/
    cp $BASE_DIR/../support_files/libgomp.patch ~/benchmarks/LINPACK/

    if [ ! -d ~/benchmarks/LINPACK/acml/gfortran64_mp/ ]; then
        echo -e "Extracting ACML for LINPACK.."
        mkdir -p ~/benchmarks/LINPACK/acml/
        cd ~/benchmarks/LINPACK/acml/
	if [ -f ~/benchmarks/acml-6.1.0.31-gfortran64.tgz ]; then
	    cp ~/benchmarks/acml-6.1.0.31-gfortran64.tgz ~/benchmarks/LINPACK/acml/
	fi
        if [ -f ~/benchmarks/libraries/acml-6.1.0.31-gfortran64.tgz ]; then
            cp ~/benchmarks/libraries/acml-6.1.0.31-gfortran64.tgz ~/benchmarks/LINPACK/acml/
        fi
        if [ ! -f ~/benchmarks/LINPACK/acml/acml-6.1.0.31-gfortran64.tgz ]; then
            ${BASE_DIR}/../support_files/get_acml.sh ~/benchmarks/LINPACK/acml/
        fi
        if [ ! -f ~/benchmarks/LINPACK/acml/acml-6.1.0.31-gfortran64.tgz ]; then
            echo -e "Error. Could not find acml-6.1.0.31-gfortran64.tgz in"
            echo -e "~/benchmarks/, ~/benchmarks/libraries/, or ~/benchmarks/LINPACK/acml/"
            echo -e "Downloading ACML requires agreeing to a license."
            echo -e "As such, we cannot automatically download it for you."
            echo -e "Please go to the following site to download the file:"
            echo -e "    http://developer.amd.com/tools-and-sdks/archive/compute/amd-core-math-library-acml/acml-downloads-resources/"
            echo -e "Then put it into ~/benchmarks/libraries/"
            exit -1
        fi
        tar -xf acml-6.1.0.31-gfortran64.tgz
    fi

    if [ ! -f ~/benchmarks/LINPACK/CBLAS/lib/cblas_LINUX.a ]; then
    echo -e "Downloading and building CBLAS for LINPACK.."
        cd ~/benchmarks/LINPACK/
        if [ ! -d ~/benchmarks/LINPACK/CBLAS ]; then
            if [ ! -f ~/benchmarks/LINPACK/cblas.tgz ]; then
                wget http://www.netlib.org/blas/blast-forum/cblas.tgz
            fi
            tar -xf cblas.tgz
        fi
        cd ~/benchmarks/LINPACK/CBLAS
        make alllib > /dev/null
        if [ $? -ne 0 ]; then
            echo -e "Failed to build CBLAS"
            exit -1
        fi
    fi

    if [ ! -f ~/benchmarks/LINPACK/lib/libgomp.so ]; then
        if [ ! -f ~/benchmarks/LINPACK/gcc-4.9.3/build_dir/x86_64-unknown-linux-gnu/libgomp/.libs/libgomp.so ]; then
            echo -e "Downloading and building libgomp for LINPACK.."
            cd ~/benchmarks/LINPACK/
            if [ ! -d ~/benchmarks/LINPACK/gcc-4.9.3/ ]; then
                if [ ! -f ~/benchmarks/LINPACK/gcc-4.9.3.tar.bz2 ]; then
                    wget https://ftp.gnu.org/gnu/gcc/gcc-4.9.3/gcc-4.9.3.tar.bz2
                fi
                tar -xf gcc-4.9.3.tar.bz2
            fi
            cd gcc-4.9.3
            patch -p 0 < ../libgomp.patch
            ./contrib/download_prerequisites
            mkdir build_dir
            cd build_dir
            ../configure --disable-multilib --disable-bootstrap
            make all-target-libgomp -j `nproc` > /dev/null
            if [ $? -ne 0 ]; then
                echo -e "Failed to build libgomp"
                exit -1
            fi
            mkdir -p ~/benchmarks/LINPACK/lib/
        fi
        cp ~/benchmarks/LINPACK/gcc-4.9.3/build_dir/x86_64-unknown-linux-gnu/libgomp/.libs/libgomp.so* ~/benchmarks/LINPACK/lib
    fi


    if [ ! -f ~/benchmarks/LINPACK/caldgemm/dgemm_bench ]; then
        echo -e "Downloading and building CALDGEMM for LINPACK.."
        cd ~/benchmarks/LINPACK/
        if [ ! -d ~/benchmarks/LINPACK/caldgemm/ ]; then
            git clone https://github.com/davidrohr/caldgemm.git
        fi
        cp ./caldgemm/3rd_party_dgemm_kernels/amd_dgemm_2015_08_05/amddgemm.so ./lib/
        cd caldgemm
        if [ ! -f ~/benchmarks/LINPACK/caldgemm/config_options.mak ]; then
            echo "BLAS_BACKEND = ACML" >> ./config_options.mak
            echo "INCLUDE_OPENCL = 1" >> ./config_options.mak
            echo "INCLUDE_CAL = 0" >> ./config_options.mak
            echo "INCLUDE_CUDA = 0" >> ./config_options.mak
            echo "INCLUDE_CUBLAS = 0" >> ./config_options.mak
            echo "CONFIG_LTO = 1" >> ./config_options.mak
            echo "CONFIGURED = 1" >> ./config_options.mak
        fi
        if [ ! -f ~/benchmarks/LINPACK/caldgemm/caldgemm_config.h ]; then
            cp ./caldgemm_config.sample ./caldgemm_config.h
        fi
        CBLAS_PATH=~/benchmarks/LINPACK/CBLAS/ ACML_PATH=~/benchmarks/LINPACK/acml/gfortran64_mp make > /dev/null
        if [ $? -ne 0 ]; then
            echo -e "Failed to build CALDGEMM"
            exit -1
        fi
    fi

    echo -e "Downloading and building HPL-GPU.."
    cd ~/benchmarks/LINPACK/
    if [ ! -d ~/benchmarks/LINPACK/hpl-gpu/ ]; then
        git clone https://github.com/davidrohr/hpl-gpu.git
    fi
    cd ~/benchmarks/LINPACK/hpl-gpu/
    ln -s ../caldgemm/
    cp ./setup/Make.Generic .
    cp ./setup/Make.Generic.Options .
    AMDAPPSDKROOT=${OCL_DIR} CBLAS_PATH=~/benchmarks/LINPACK/CBLAS/ ACML_PATH=~/benchmarks/LINPACK/acml/gfortran64_mp LD_LIBRARY_PATH=~/benchmarks/LINPACK/lib:$LD_LIBRARY_PATH ./build.sh > /dev/null
    if [ $? -ne 0 ]; then
        echo -e "Failed to build HPL-GPU"
        exit -1
    fi
    sed -i.bak s'/89088/3840/' ./bin/Generic/HPL.dat
elif [ $AMD_OCL -ne 1 ]; then
    echo -e "LINPACK is only supported on the AMD APP SDK. Not building LINPACK."
elif [ $AMD_HAWAII_GPU -ne 1 ]; then
    echo -e "This LINPACK installation requires an AMD Hawaii GPU to work. Not building LINPACK."
else
    echo -e "~/benchmarks/LINPACK/hpl-gpu/bin/Generic/xhpl exists. Not rebuilding LINPACK."
fi
