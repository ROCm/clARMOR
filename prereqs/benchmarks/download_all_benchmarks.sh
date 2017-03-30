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


# This script will make a local copy of all of the benchmarks.

BASE_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

if [ ! -d ~/benchmarks/ ]; then
    mkdir -p ~/benchmarks/
fi
if [ ! -d ~/benchmarks/libraries/ ]; then
    mkdir -p ~/benchmarks/libraries/
fi
if [ ! -d ~/benchmarks/proxyapps ]; then
    mkdir -p ~/benchmarks/proxyapps
fi
if [ ! -d ~/benchmarks/phoronix ]; then
    mkdir -p ~/benchmarks/phoronix
fi
if [ ! -d ~/benchmarks/mantevo ]; then
    mkdir -p ~/benchmarks/mantevo
fi
if [ ! -d ~/benchmarks/LINPACK/ ]; then
    mkdir -p ~/benchmarks/LINPACK
fi
if [ ! -d ~/benchmarks/LINPACK/acml/ ]; then
    mkdir -p ~/benchmarks/LINPACK/acml
fi

#---------------- Stuff that we cannot automatically download -----------------
cd ~/benchmarks/

echo -e "Checking if Parboil files exist."
if [ ! -f ~/benchmarks/pb2.5driver.tgz ]; then
    echo -e "Error. Could not find ~/benchmarks/pb2.5driver.tgz"
    echo -e "Downloading Parboil requires manually agreeing to a license."
    echo -e "As such, we cannot automatically download it for you."
    echo -e "Please download the Parboil driver from:"
    echo -e "     http://impact.crhc.illinois.edu/parboil/parboil.aspx"
    echo -e "Then put it into ~/benchmarks/"
    exit -1
fi
if [ ! -f ~/benchmarks/pb2.5benchmarks.tgz ] && [ ! -f ~/benchmarks/parboil/pb2.5benchmarks.tgz ]; then
    echo -e "Error. Could not find pb2.5benchmarks.tgz in either:"
    echo -e "~/benchmarks/ or ~/benchmarks/parboil/"
    echo -e "Downloading Parboil requires manually agreeing to a license."
    echo -e "As such, we cannot automatically download it for you."
    echo -e "Please download the Parboil benchmarks from:"
    echo -e "     http://impact.crhc.illinois.edu/parboil/parboil.aspx"
    echo -e "Then put it into ~/benchmarks/"
    exit -1
fi
if [ ! -f ~/benchmarks/pb2.5datasets_standard.tgz ] && [ ! -f  ~/benchmarks/parboil/pb2.5datasets_standard.tgz ]; then
    echo -e "Error. Could not find pb2.5datasets_standard.tgz in either:"
    echo -e "~/benchmarks/ or ~/benchmarks/parboil/"
    echo -e "Downloading Parboil requires manually agreeing to a license."
    echo -e "As such, we cannot automatically download it for you."
    echo -e "Please download the Parboil datasets from:"
    echo -e "     http://impact.crhc.illinois.edu/parboil/parboil.aspx"
    echo -e "Then put it into ~/benchmarks/"
    exit -1
fi

echo -e "Checking if SNU NPB OpenCL files exist."
if [ ! -f ~/benchmarks/SNU_NPB-1.0.3.tar.gz ]; then
    echo -e "Error. Could not find ~/benchmarks/SNU_NPB-1.0.3.tar.gz"
    echo -e "Downloading this requires entering information on a page."
    echo -e "As such, we cannot automatically download it for you."
    echo -e "Please go to the following site to download the file:"
    echo -e "    http://aces.snu.ac.kr/software/snu-npb/"
    echo -e "Then put it into ~/benchmarks/"
    exit -1
fi
#------------------------------------------------------------------------------

#-------------------------------- AMD APP SDK ---------------------------------
if [ ! -d ~/benchmarks/AMDAPP/AMDAPP_install ]; then
    ${BASE_DIR}/../support_files/get_amd_app_sdk.sh -d ~/benchmarks/AMDAPP_install
    if [ $? -ne 0 ]; then
        echo -e "Failed to download the AMD APP SDK."
        exit -1
    fi
    mkdir -p ~/benchmarks/AMDAPP/
    mv ~/benchmarks/AMDAPP_install/AMDAPP ~/benchmarks/AMDAPP/AMDAPP_install
    rm -rf ~/benchmarks/AMDAPP_install/
fi
#------------------------------------------------------------------------------

#----------------------------------- ACML -------------------------------------
if [ ! -f ~/benchmarks/libraries/acml-6.1.0.31-gfortran64.tgz ]; then
    mkdir -p ~/benchmarks/libraries/
    ${BASE_DIR}/../support_files/get_acml.sh -d ~/benchmarks/libraries/
    if [ -f ~/benchmarks/libraries/acml-6.1.0.31-gfortran64.tgz ]; then
        cp ~/benchmarks/libraries/acml-6.1.0.31-gfortran64.tgz ~/benchmarks/LINPACK/acml/
    else
        echo -e "Failed to download ACML."
        exit -1
    fi
fi
#------------------------------------------------------------------------------

#---------------------------- Exascale Proxy Apps -----------------------------
if [ ! -d ~/benchmarks/proxyapps/ComputeApps/ ]; then
    cd ~/benchmarks/proxyapps/
    echo -e "\n\nAbout to log into GitHub to get AMD Proxy Apps:"
    git clone https://github.com/AMDComputeLibraries/ComputeApps.git
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
    echo -e "Skipping download of OpenCL proxy apps from GitHub. They already exist."
fi

if [ ! -d ~/benchmarks/proxyapps/SNAP-OpenCL/ ]; then
    cd ~/benchmarks/proxyapps/
    echo -e "\n\nAbout to log into GitHub to get SNAP:"
    git clone https://github.com/UoB-HPC/SNAP-OpenCL.git
else
    echo -e "Skipping download of SNAP-OpenCL. It already exists."
fi

if [ ! -d ~/benchmarks/proxyapps/SNAP_MPI_OpenCL/ ]; then
    echo -e "\n\nAbout to log into GitHub to get SNAP MPI:"
    git clone https://github.com/jlgreathouse/SNAP_MPI_OpenCL.git
else
    echo -e "Skipping download of SNAP-MPI-OpenCL. It already exists."
fi
#------------------------------------------------------------------------------

#------------------------------------ SHOC ------------------------------------
if [ ! -d ~/benchmarks/shoc ]; then
    cd ~/benchmarks/
    echo -e "\n\nAbout to log into GitHub to get SHOC:"
    git clone https://github.com/vetter/shoc.git
else
    echo -e "Skipping download of SHOC from GitHub. It already exists."
fi
#------------------------------------------------------------------------------

#---------------------------------- RODINIA -----------------------------------
if [ ! -f ~/benchmarks/rodinia_3.1.tar.bz2 ]; then
    cd ~/benchmarks/
    echo -e "\n\nAbout to download Rodinia 3.1 from virginia.edu"
    wget http://www.cs.virginia.edu/~kw5na/lava/Rodinia/Packages/Current/rodinia_3.1.tar.bz2
    if [ $? -ne 0 ]; then
        echo -e "Failed to download file"
        exit -1
    fi
else
    echo -e "Skipping download of Rodinia 3.1. It already exists."
fi
#------------------------------------------------------------------------------

#--------------------------------- OpenDwarfs ---------------------------------
if [ ! -d ~/benchmarks/OpenDwarfs ]; then
    cd ~/benchmarks/
    echo -e "\n\nAbout to log into GitHub to get OpenDwarfs:"
    git clone https://github.com/vtsynergy/OpenDwarfs.git
    rm -rf ~/benchmarks/OpenDwarfs/build/
else
    echo -e "Skipping download of OpenDwarfs from GitHub. It already exists."
fi
#------------------------------------------------------------------------------

#---------------------------------- Pannotia ----------------------------------
if [ ! -d ~/benchmarks/pannotia ]; then
    cd ~/benchmarks/
    echo -e "\n\nAbout to log into GitHub to get Pannotia:"
    git clone https://github.com/jlgreathouse/pannotia.git
else
    echo -e "Skipping download of Pannotia from GitHub. It already exists."
fi
#------------------------------------------------------------------------------

#---------------------------------- StreamMR ----------------------------------
if [ ! -d ~/benchmarks/StreamMR ]; then
    cd ~/benchmarks/
    echo -e "\n\nAbout to log into GitHub to get StreamMR:"
    git clone https://github.com/jlgreathouse/StreamMR.git
else
    echo -e "Skipping download of StreamMR from GitHub. It already exists."
fi
#------------------------------------------------------------------------------

#---------------------------------- Phoronix ----------------------------------
cd ~/benchmarks/phoronix/
if [ ! -f ~/benchmarks/phoronix/JuliaGPU-v1.2pts-1.tar.bz2 ] || \
    [ ! -f ~/benchmarks/phoronix/MandelGPU-v1.3pts-1.tar.bz2 ] || \
    [ ! -f ~/benchmarks/phoronix/SmallptGPU-v1.6pts-1.tar.bz2 ] || \
    [ ! -f ~/benchmarks/phoronix/mandelbulbGPU-v1.0pts-1.tar.bz2 ]; then
    echo -e "\n\nAbout to download Phoronix Test Suite."
else
    echo -e "Skipping download of Phoronix benchmarks. They already exist."
fi

if [ ! -f ~/benchmarks/phoronix/JuliaGPU-v1.2pts-1.tar.bz2 ]; then
    echo -e "About to download JuliaGPU"
    wget http://www.phoronix-test-suite.com/benchmark-files/JuliaGPU-v1.2pts-1.tar.bz2
    if [ $? -ne 0 ]; then
        echo -e "Failed to download file"
        exit -1
    fi
fi
if [ ! -f ~/benchmarks/phoronix/MandelGPU-v1.3pts-1.tar.bz2 ]; then
    echo -e "About to download MandelGPU"
    wget http://www.phoronix-test-suite.com/benchmark-files/MandelGPU-v1.3pts-1.tar.bz2
    if [ $? -ne 0 ]; then
        echo -e "Failed to download file"
        exit -1
    fi
fi
if [ ! -f ~/benchmarks/phoronix/SmallptGPU-v1.6pts-1.tar.bz2 ]; then
    echo -e "About to download SmallptGPU"
    wget http://www.phoronix-test-suite.com/benchmark-files/SmallptGPU-v1.6pts-1.tar.bz2
    if [ $? -ne 0 ]; then
        echo -e "Failed to download file"
        exit -1
    fi
fi
if [ ! -f ~/benchmarks/phoronix/mandelbulbGPU-v1.0pts-1.tar.bz2 ]; then
    echo -e "About to download mandelbulbGPU"
    wget http://www.phoronix-test-suite.com/benchmark-files/mandelbulbGPU-v1.0pts-1.tar.bz2
    if [ $? -ne 0 ]; then
        echo -e "Failed to download file"
        exit -1
    fi
fi
#------------------------------------------------------------------------------

#--------------------------------- PolyBench ----------------------------------
if [ ! -d ~/benchmarks/PolyBench-ACC/ ]; then
    echo -e "\n\nAbout to log into GitHub to get PolyBench-ACC"
    git clone https://github.com/cavazos-lab/PolyBench-ACC.git
else
    echo -e "Skipping download of PolyBench-ACC. It already exists."
fi
#------------------------------------------------------------------------------

#-------------------------------- FinanceBench --------------------------------
if [ ! -d ~/benchmarks/FinanceBench/ ]; then
    echo -e "\n\nAbout to log into GitHub to get FinanceBench"
    git clone https://github.com/cavazos-lab/FinanceBench.git
else
    echo -e "Skipping download of FinanceBench. It already exists."
fi
#------------------------------------------------------------------------------

#---------------------------------- GPU-STREAM --------------------------------
if [ ! -d ~/benchmarks/GPU-STREAM ]; then
    echo -e "\n\nAbout to log into GitHub to get GPU-STREAM:"
    git clone https://github.com/UoB-HPC/GPU-STREAM.git
else
    echo -e "Skipping download of GPU-STREAM. It already exists."
fi
#------------------------------------------------------------------------------

#----------------------------------- MANTEVO ----------------------------------
if [ ! -d ~/benchmarks/mantevo/TeaLeaf_OpenCL ]; then
    cd ~/benchmarks/mantevo
    echo -e "\n\nAbout to log into GitHub to get TeaLeaf_OpenCL:"
    git clone https://github.com/UK-MAC/TeaLeaf_OpenCL.git
else
    echo -e "Skipping download of TeaLeaf. It already exists."
fi
if [ ! -d ~/benchmarks/mantevo/TeaLeaf3D_OpenCL ]; then
    cd ~/benchmarks/mantevo
    echo -e "\n\nAbout to log into GitHub to get TeaLeaf3D_OpenCL:"
    git clone https://github.com/UK-MAC/TeaLeaf3D_OpenCL.git
else
    echo -e "Skipping download of TeaLeaf3D. It already exists."
fi
if [ ! -d ~/benchmarks/mantevo/CloverLeaf_OpenCL ]; then
    cd ~/benchmarks/mantevo
    echo -e "\n\nAbout to log into GitHub to get CloverLeaf_OpenCL:"
    git clone https://github.com/UK-MAC/CloverLeaf_OpenCL.git
else
    echo -e "Skipping download of CloverLeaf. It already exists."
fi
if [ ! -d ~/benchmarks/mantevo/CloverLeaf3D_OpenCL ]; then
    cd ~/benchmarks/mantevo
    echo -e "\n\nAbout to log into GitHub to get CloverLeaf3D_OpenCL:"
    git clone https://github.com/UK-MAC/CloverLeaf3D_OpenCL.git
else
    echo -e "Skipping download of CloverLeaf3D. It already exists."
fi
#------------------------------------------------------------------------------

#--------------------------------- HETERO-MARK --------------------------------
if [ ! -d ~/benchmarks/Hetero-Mark ]; then
    cd ~/benchmarks/
    echo -e "\n\nAbout to log into GitHub to get Hetero-Mark:"
    git clone -b amd_fixes https://github.com/jlgreathouse/Hetero-Mark.git
else
    echo -e "Skipping download of Hetero-Mark. It already exists."
fi
#------------------------------------------------------------------------------

#----------------------------------- LINPACK ----------------------------------
if [ ! -f ~/benchmarks/LINPACK/cblas.tgz ]; then
    cd ~/benchmarks/LINPACK/
    echo -e "\n\nAbout to download CBLAS from NetLib:"
    wget http://www.netlib.org/blas/blast-forum/cblas.tgz
    if [ $? -ne 0 ]; then
        echo -e "Failed to download file"
        exit -1
    fi
fi
if [ ! -f ~/benchmarks/LINPACK/gcc-4.9.3.tar.bz2 ]; then
    cd ~/benchmarks/LINPACK/
    echo -e "\n\nAbout to download GCC from GNU:"
    wget https://ftp.gnu.org/gnu/gcc/gcc-4.9.3/gcc-4.9.3.tar.bz2
    if [ $? -ne 0 ]; then
        echo -e "Failed to download file"
        exit -1
    fi
fi
if [ ! -d ~/benchmarks/LINPACK/caldgemm/ ]; then
    cd ~/benchmarks/LINPACK/
    echo -e "\n\nAbout to download CALDGEMM from GitHub:"
    git clone https://github.com/davidrohr/caldgemm.git
fi
if [ ! -d ~/benchmarks/LINPACK/hpl-gpu/ ]; then
    cd ~/benchmarks/LINPACK/
    echo -e "\n\nAbout to download HPL-GPU from GitHub:"
    git clone https://github.com/davidrohr/hpl-gpu.git
else
    echo -e "Skipping download of HPL-GPU. It already exists."
fi
#------------------------------------------------------------------------------

#---------------------------------- ViennaCL ----------------------------------
if [ ! -f ~/benchmarks/ViennaCL-1.7.1.tar.gz ]; then
    cd ~/benchmarks/
    echo -e "About to download ViennaCL from SourceForge"
    wget http://downloads.sourceforge.net/project/viennacl/1.7.x/ViennaCL-1.7.1.tar.gz
    if [ $? -ne 0 ]; then
        echo -e "Failed to download file"
        exit -1
    fi
else
    echo -e "Skipping download of ViennaCL. It already exists."
fi
#------------------------------------------------------------------------------
