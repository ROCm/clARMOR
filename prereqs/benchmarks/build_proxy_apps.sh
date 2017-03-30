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


# This script will make a local copy (git clone) of a series of exascale
# proxy applications, from AMD and from the Univ. of Bristol.
# They apps can be run with clarmor --group=PROXYAPPS

# Licensing Information:
#  - CoMD uses a 3-clause BSD license. See CoMD/LICENSE.md
#  - LULESH uses a 3-clause BSD license. See LULESH2.0/lulesh.cc
#  - SNAP-MPI uses a 3-clause BSD license. See SNAP_MPI_OpenCL/LICENSE
#  - SNAP uses a 3-clause BSD license. See SNAP-OpenCL/README.md
#  - XSBench uses an MIT license. See XSBench/LICENSE

BASE_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

if [ ! -d ~/benchmarks/proxyapps ]; then
    mkdir -p ~/benchmarks/proxyapps
fi

cd ~/benchmarks/proxyapps

if [ ! -d ~/benchmarks/proxyapps/ComputeApps/ ] ||\
    [ ! -f ~/benchmarks/proxyapps/CoMD/CoMD-ocl ] ||\
    [ ! -f ~/benchmarks/proxyapps/LULESH/lulesh ] ||\
    [ ! -f ~/benchmarks/proxyapps/XSBench/src/XSBench ] ||\
    [ ! -f ~/benchmarks/proxyapps/SNAP-OpenCL/src/snap ] ||\
    [ ! -f ~/benchmarks/proxyapps/SNAP_MPI_OpenCL/src/snap ];
    then
    source ${BASE_DIR}/setup_bench_install.sh
fi

if [ ! -d ~/benchmarks/proxyapps/ComputeApps/ ]; then
    cd ~/benchmarks/proxyapps/
    echo -e "\n\nAbout to log into GitHub to get AMD Proxy Apps:"
    git clone https://github.com/AMDComputeLibraries/ComputeApps.git
	git checkout 4e5d9ca628b6b14b7b1094b9d37324e4856ce4c7
    cd ~/benchmarks/proxyapps/ComputeApps/
    git checkout 4e5d9ca628b6b14b7b1094b9d37324e4856ce4c7
    if [ ! -d ~/benchmarks/proxyapps/CoMD ]; then
        cp -R ./ComputeApps/comd-cl ./CoMD
    fi
    if [ ! -d ~/benchmarks/proxyapps/LULESH ]; then
        cp -R ./ComputeApps/lulesh-cl ./LULESH
    fi
    if [ ! -d ~/benchmarks/proxyapps/XSBench ]; then
        cp -R ./ComputeApps/xsbench-cl ./XSBench
    fi
else
    cd ~/benchmarks/proxyapps/ComputeApps/
    git checkout 4e5d9ca628b6b14b7b1094b9d37324e4856ce4c7
fi

if [ ! -f ~/benchmarks/proxyapps/CoMD/CoMD-ocl ]; then
    cd ~/benchmarks/proxyapps
    if [ ! -d ~/benchmarks/proxyapps/CoMD ]; then
        mkdir temp
        cd temp
        echo -e "\n\nAbout to log into GitHub to get AMD Proxy Apps (CoMD):"
        git clone https://github.com/AMDComputeLibraries/ComputeApps.git
		git checkout 4e5d9ca628b6b14b7b1094b9d37324e4856ce4c7
        mv ./ComputeApps/comd-cl ../CoMD
        cd ..
        rm -rf ./temp
    fi
    cd CoMD/src-cl
	sed -i.bak 's#/opt/AMDAPP/include#'${OCL_INCLUDE_DIR}'#' ./Makefile
    make -j `nproc`
    if [ $? -ne 0 ]; then
        echo -e "Failed to build CoMD."
        exit -1
    fi
else
    echo -e "~/benchmarks/proxyapps/CoMD/CoMD-ocl exists. Not rebuilding CoMD."
fi

if [ ! -f ~/benchmarks/proxyapps/LULESH/lulesh ]; then
    cd ~/benchmarks/proxyapps
    if [ ! -d ~/benchmarks/proxyapps/LULESH ]; then
        mkdir temp
        cd temp
        echo -e "\n\nAbout to log into GitHub to get AMD Proxy Apps (LULESH):"
        git clone https://github.com/AMDComputeLibraries/ComputeApps.git
		git checkout 4e5d9ca628b6b14b7b1094b9d37324e4856ce4c7
        mv ./ComputeApps/lulesh-cl ../LULESH
        cd ..
        rm -rf ./temp
    fi
    cd LULESH
    sed -i.bak 's/O3/g -O3/' ./Makefile
	sed -i.bak 's#/opt/AMDAPP/include/#'${OCL_INCLUDE_DIR}'#' ./Makefile
	sed -i.bak 's#/opt/AMDAPP/lib/x86_64#'${OCL_LIB_DIR}'#' ./Makefile
	sed -i.bak 's#-L$(LIB_PATH)##' ./Makefile
	# The following changes need to be made so that this program will compile
	# in Nvidia's OpenCL implementation.
	sed -i.bak 's#(Real_t pfx\[8\],#(__private Real_t pfx[8],#' ./kernels.cl
	sed -i.bak 's#Real_t pfy\[8\],#__private Real_t pfy[8],#' ./kernels.cl
	sed -i.bak 's#Real_t pfz\[8\],#__private Real_t pfz[8],#' ./kernels.cl
	sed -i.bak 's#const Real_t x\[8\],#__private const Real_t x[8],#' ./kernels.cl
	sed -i.bak 's#const Real_t y\[8\],#__private const Real_t y[8],#' ./kernels.cl
	sed -i.bak 's#const Real_t z\[8\],#__private const Real_t z[8],#' ./kernels.cl
	sed -i.bak 's#Real_t b\[\]\[8\],#__private Real_t b[][8],#' ./kernels.cl
	sed -i.bak 's#Real_t dvdx\[8\],#__private Real_t dvdx[8],#' ./kernels.cl
	sed -i.bak 's#Real_t dvdy\[8\],#__private Real_t dvdy[8],#' ./kernels.cl
	sed -i.bak 's#Real_t dvdz\[8\],#__private Real_t dvdz[8],#' ./kernels.cl

    make -j `nproc`
    if [ $? -ne 0 ]; then
        echo -e "Failed to build LULESH."
        exit -1
    fi
else
    echo -e "~/benchmarks/proxyapps/LULESH/lulesh exists. Not rebuilding LULESH."
fi

if [ ! -f ~/benchmarks/proxyapps/XSBench/src/XSBench ]; then
    cd ~/benchmarks/proxyapps
    if [ ! -d ~/benchmarks/proxyapps/XSBench ]; then
        mkdir temp
        cd temp
        echo -e "\n\nAbout to log into GitHub to get AMD Proxy Apps (XSBench):"
        git clone https://github.com/AMDComputeLibraries/ComputeApps.git
		git checkout 4e5d9ca628b6b14b7b1094b9d37324e4856ce4c7
        mv ./ComputeApps/xsbench-cl ../XSBench
        cd ..
        rm -rf ./temp
    fi
	cd XSBench/src
    sed -i.bak 's/O3/g -O3/' ./makefile
	sed -i.bak 's#/opt/AMDAPP/include#'${OCL_INCLUDE_DIR}'#' ./makefile
	sed -i.bak '1055s#local_work_size#NULL#' ./XSBench_OCL.c
	# The following changes *will* affect what the kernel does. But there is a
	# weird read overflow somewhere in this kernel that I don't care to fix.
	# This workaround gets things moving, and doesn't greatly affect
	# performance. This read overflow only shows up on NV GPUs, so take out
	# these changes if you want to test on an AMD GPU.
	sed -i.bak 's#mat_off >> 2#0#' ./xsbench_kernels.cl
	sed -i.bak 's#pnuc_ids + n_nucs#pnuc_ids#' ./xsbench_kernels.cl
    make -j `nproc`
    if [ $? -ne 0 ]; then
        echo -e "Failed to build XSBench."
        exit -1
    fi
else
    echo -e "~/benchmarks/proxyapps/XSBench/src/XSBench exists. Not rebuilding XSBench."
fi

if [ ! -f ~/benchmarks/proxyapps/SNAP-OpenCL/src/snap ]; then
    cd ~/benchmarks/proxyapps/
    if [ ! -d ~/benchmarks/proxyapps/SNAP-OpenCL/ ]; then
        echo -e "\n\nAbout to log into GitHub to get SNAP:"
        git clone https://github.com/UoB-HPC/SNAP-OpenCL.git
    fi
    cd SNAP-OpenCL/src
    sed -i.bak 's#FFLAGS = -O3#FFLAGS = -O3 -g -fopenmp -I'${OCL_INCLUDE_DIR}'#' ./Makefile
    sed -i.bak 's#CFLAGS = -std=c99 -O3#CFLAGS = -std=c99 -O3 -g -I'${OCL_INCLUDE_DIR}'#' ./Makefile
    sed -i.bak 's#OPENCL = -lOpenCL#OPENCL = -lOpenCL -L'${OCL_LIB_DIR}'#' ./Makefile
    rm Makefile.bak
    printf "\n! Input from namelist\n&invar\n  nthreads=1\n  nnested=0\n  npey=1\n  npez=1\n  ndimen=3\n  nx=16\n  lx=4.8\n  ny=16\n  ly=4.8\n  nz=16\n  lz=4.8\n  ichunk=16\n  nmom=2\n  nang=136\n  ng=50\n  mat_opt=0\n  src_opt=0\n  timedep=1\n  it_det=0\n  tf=1.0\n  nsteps=1\n  iitm=1\n  oitm=1\n  epsi=1.E-4\n  fluxp=0\n  scatp=0\n  fixup=1\n/\n\n" > ./snap-16.in
    printf "\n! Input from namelist\n&invar\n  nthreads=1\n  nnested=0\n  npey=1\n  npez=1\n  ndimen=3\n  nx=24\n  lx=4.8\n  ny=24\n  ly=4.8\n  nz=24\n  lz=4.8\n  ichunk=24\n  nmom=2\n  nang=136\n  ng=50\n  mat_opt=0\n  src_opt=0\n  timedep=1\n  it_det=0\n  tf=1.0\n  nsteps=1\n  iitm=1\n  oitm=1\n  epsi=1.E-4\n  fluxp=0\n  scatp=0\n  fixup=1\n/\n\n" > ./snap-24.in
    printf "\n! Input from namelist\n&invar\n  nthreads=1\n  nnested=0\n  npey=1\n  npez=1\n  ndimen=3\n  nx=32\n  lx=4.8\n  ny=32\n  ly=4.8\n  nz=32\n  lz=4.8\n  ichunk=32\n  nmom=2\n  nang=136\n  ng=50\n  mat_opt=0\n  src_opt=0\n  timedep=1\n  it_det=0\n  tf=1.0\n  nsteps=1\n  iitm=1\n  oitm=1\n  epsi=1.E-4\n  fluxp=0\n  scatp=0\n  fixup=1\n/\n\n" > ./snap-32.in
    make -j `nproc`
    if [ $? -ne 0 ]; then
        echo -e "Failed to build SNAP-OpenCL."
        exit -1
    fi
else
    echo -e "~/benchmarks/proxyapps/SNAP-OpenCL/src/snap exists. Not rebuilding SNAP."
fi

if [ ! -f ~/benchmarks/proxyapps/SNAP_MPI_OpenCL/src/snap ]; then
    cd ~/benchmarks/proxyapps/
    if [ ! -d ~/benchmarks/proxyapps/SNAP_MPI_OpenCL/ ]; then
        echo -e "\n\nAbout to log into GitHub to get SNAP MPI:"
        git clone https://github.com/jlgreathouse/SNAP_MPI_OpenCL.git
    fi
    cd SNAP_MPI_OpenCL/src
    sed -i.bak 's#-O3#-I'${OCL_INCLUDE_DIR}' -g -O3#' ./Makefile
    sed -i.bak 's#-lOpenCL#-L'${OCL_LIB_DIR}' -lOpenCL#' ./Makefile
    printf "\n! Input from namelist\n&invar\n  nthreads=1\n  nnested=0\n  npey=1\n  npez=1\n  ndimen=3\n  nx=16\n  lx=4.8\n  ny=16\n  ly=4.8\n  nz=16\n  lz=4.8\n  ichunk=16\n  nmom=2\n  nang=136\n  ng=50\n  mat_opt=0\n  src_opt=0\n  timedep=1\n  it_det=0\n  tf=1.0\n  nsteps=1\n  iitm=1\n  oitm=1\n  epsi=1.E-4\n  fluxp=0\n  scatp=0\n  fixup=1\n/\n\n" > ./snap-16.in
    printf "\n! Input from namelist\n&invar\n  nthreads=1\n  nnested=0\n  npey=1\n  npez=1\n  ndimen=3\n  nx=24\n  lx=4.8\n  ny=24\n  ly=4.8\n  nz=24\n  lz=4.8\n  ichunk=24\n  nmom=2\n  nang=136\n  ng=50\n  mat_opt=0\n  src_opt=0\n  timedep=1\n  it_det=0\n  tf=1.0\n  nsteps=1\n  iitm=1\n  oitm=1\n  epsi=1.E-4\n  fluxp=0\n  scatp=0\n  fixup=1\n/\n\n" > ./snap-24.in
    printf "\n! Input from namelist\n&invar\n  nthreads=1\n  nnested=0\n  npey=1\n  npez=1\n  ndimen=3\n  nx=32\n  lx=4.8\n  ny=32\n  ly=4.8\n  nz=32\n  lz=4.8\n  ichunk=32\n  nmom=2\n  nang=136\n  ng=50\n  mat_opt=0\n  src_opt=0\n  timedep=1\n  it_det=0\n  tf=1.0\n  nsteps=1\n  iitm=1\n  oitm=1\n  epsi=1.E-4\n  fluxp=0\n  scatp=0\n  fixup=1\n/\n\n" > ./snap-32.in
    make -j `nproc` COMPILER=GNU
    if [ $? -ne 0 ]; then
        echo -e "Failed to build SNAP-MPI-OpenCL."
        exit -1
    fi
else
    echo -e "~/benchmarks/proxyapps/SNAP_MPI_OpenCL/src/snap exists. Not rebuilding SNAP MPI."
fi
