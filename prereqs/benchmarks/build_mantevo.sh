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


# This script will download and buildthe CloverLeaf and TeaLeaf Mantevo apps.
# They can be run with clarmor --group=MANTEVO

# Licensing Information:
#  - CloverLeaf is available under the GPLv3. See CloverLeaf_OpenCL/clover_leaf.f90
#  - CloverLeaf3D is available under the GPLv3. See CloverLeaf3D_OpenCL/clover_leaf.f90
#  - TeaLeaf is available under the GPLv3. See TeaLeaf_OpenCL/tea_leaf.f90
#  - TeaLeaf3D is available under the GPLv3. See TeaLeaf3D_OpenCL/tea_leaf.f90

BASE_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

if [ ! -d ~/benchmarks/mantevo ]; then
    mkdir -p ~/benchmarks/mantevo
fi

cd ~/benchmarks/mantevo

if [ ! -d ~/benchmarks/mantevo/TeaLeaf_OpenCL ]; then
    cd ~/benchmarks/mantevo
    echo -e "\n\nAbout to log into GitHub to get TeaLeaf_OpenCL:"
    git clone https://github.com/UK-MAC/TeaLeaf_OpenCL.git
fi
if [ ! -d ~/benchmarks/mantevo/TeaLeaf3D_OpenCL ]; then
    cd ~/benchmarks/mantevo
    echo -e "\n\nAbout to log into GitHub to get TeaLeaf3D_OpenCL:"
    git clone https://github.com/UK-MAC/TeaLeaf3D_OpenCL.git
fi
if [ ! -d ~/benchmarks/mantevo/CloverLeaf_OpenCL ]; then
    cd ~/benchmarks/mantevo
    echo -e "\n\nAbout to log into GitHub to get CloverLeaf_OpenCL:"
    git clone https://github.com/UK-MAC/CloverLeaf_OpenCL.git
fi
if [ ! -d ~/benchmarks/mantevo/CloverLeaf3D_OpenCL ]; then
    cd ~/benchmarks/mantevo
    echo -e "\n\nAbout to log into GitHub to get CloverLeaf3D_OpenCL:"
    git clone https://github.com/UK-MAC/CloverLeaf3D_OpenCL.git
fi

if [ ! -f ~/benchmarks/mantevo/TeaLeaf_OpenCL/tea_leaf ] ||\
    [ ! -f ~/benchmarks/mantevo/TeaLeaf3D_OpenCL/tea_leaf ] ||\
    [ ! -f ~/benchmarks/mantevo/CloverLeaf_OpenCL/clover_leaf ] ||\
    [ ! -f ~/benchmarks/mantevo/CloverLeaf3D_OpenCL/clover_leaf ];
    then
    source ${BASE_DIR}/setup_bench_install.sh
fi


if [ ! -f ~/benchmarks/mantevo/TeaLeaf_OpenCL/tea_leaf ]; then
    cd ~/benchmarks/mantevo/TeaLeaf_OpenCL/
    git checkout 862e85420d25796760accbd83a257d7e3cd67683
    sed -i.bak s'/-O3 -funroll-loops -cpp/-O3 -funroll-loops -cpp -g/' ./Makefile
    sed -i.bak s'/CFLAGS_          = -O3/CFLAGS_          = -O3 -g -march=native -funroll-loops/' ./Makefile
    sed -i.bak s'/FLAGS_          = -O3/FLAGS_          = -O3 -g -march=native -funroll-loops/' ./Makefile
    sed -i.bak s'/MPICXX_LIB=#-lmpi_cxx/MPICXX_LIB=-lmpi_cxx/' ./Makefile
    sed -i.bak s'#LDLIBS+=-lOpenCL -lstdc++ $(MPICXX_LIB)#LDLIBS+=-lOpenCL -lstdc++ -lmpi $(MPICXX_LIB) -L'${OCL_LIB_DIR}'#' ./Makefile
    sed -i.bak s'#CFLAGS=\$(CFLAGS_\$(COMPILER)) \$(OMP) \$(I3E) \$(C_OPTIONS) -c#CFLAGS=$(CFLAGS_$(COMPILER)) $(OMP) $(I3E) $(C_OPTIONS) -c -I'${OCL_INCLUDE_DIR}'#' ./Makefile
    sed -i.bak s'/tl_max_iters=10000/tl_max_iters=10/' ./tea.in
    if [ $AMD_OCL -eq 1 ]; then
        sed -i.bak s'/opencl_vendor=any/opencl_vendor=advanced/' ./tea.in
    fi
	if [ $NV_OCL -eq 1 ]; then
		sed -i.bak s'/opencl_vendor=any/opencl_vendor=nvidia/' ./tea.in
	fi
	if [ $INT_OCL -eq 1 ]; then
		sed -i.bak s'/opencl_vendor=any/opencl_vendor=intel/' ./tea.in
	fi
    sed -i.bak s'/opencl_type=gpu/opencl_type=gpu\nuse_opencl_kernels/' ./tea.in
    make COMPILER=GNU -j `nproc`
    if [ $? -ne 0 ]; then
        echo -e "Failed to build TeaLeaf."
        exit -1
    fi
else
    echo -e "~/benchmarks/mantevo/tea_leaf exists. Not rebuilding TeaLeaf_OpenCL."
fi

if [ ! -f ~/benchmarks/mantevo/TeaLeaf3D_OpenCL/tea_leaf ]; then
    cd ~/benchmarks/mantevo/TeaLeaf3D_OpenCL/
    git checkout b1d5c644980389201572e0b152e1946fda82b2d9
    sed -i.bak s'/CFLAGS_GNU       = -O3 -march=native -funroll-loops/CFLAGS_GNU       = -O3 -march=native -funroll-loops -g/' ./Makefile
    sed -i.bak s'/CFLAGS_          = -O3/CFLAGS_          = -O3 -g -march=native -funroll-loops/' ./Makefile
    sed -i.bak s'/FLAGS_          = -O3/FLAGS_          = -O3 -g -march=native -funroll-loops/' ./Makefile
    sed -i.bak s'#LDLIBS+=-lOpenCL -lstdc++ $(MPICXX_LIB)#LDLIBS+=-lOpenCL -lstdc++ $(MPICXX_LIB) -L'${OCL_LIB_DIR}'#' ./Makefile
    sed -i.bak s'#CFLAGS=$(CFLAGS_$(COMPILER)) $(OMP) $(I3E) $(C_OPTIONS) -c#CFLAGS=$(CFLAGS_$(COMPILER)) $(OMP) $(I3E) $(C_OPTIONS) -c -I'${OCL_INCLUDE_DIR}'#' ./Makefile
	if [ $AMD_OCL -eq 1 ]; then
		sed -i.bak s'/opencl_vendor=nvidia/opencl_vendor=advanced/' ./tea.in
	fi
	if [ $INT_OCL -eq 1 ]; then
		sed -i.bak s'/opencl_vendor=nvidia/opencl_vendor=intel/' ./tea.in
	fi
    sed -i.bak s'/opencl_device=0/opencl_device=0\n use_opencl_kernels/' ./tea.in
    sed -i.bak s'/end_step=20/end_step=2/' ./tea.in
    # Makefile broken for parallel builds here, just run it serial.
    make COMPILER=GNU
    if [ $? -ne 0 ]; then
        echo -e "Failed to build TeaLeaf3D"
        exit -1
    fi
else
    echo -e "~/benchmarks/mantevo/TeaLeaf3D_OpenCL/tea_leaf exists. Not rebuilding TeaLeaf3D_OpenCL."
fi

if [ ! -f ~/benchmarks/mantevo/CloverLeaf_OpenCL/clover_leaf ]; then
    cd ~/benchmarks/mantevo/CloverLeaf_OpenCL/
    git checkout 85e9aa0baa22b2049d5da12233d4e7a6c53f450a
    sed -i.bak s'#OCL_AMD_INC=-I/opt/opencl/amd-app-2.7/include#OCL_AMD_INC=-I'${OCL_INCLUDE_DIR}' -I/usr/lib/openmpi/include -pthread -I/usr/lib/openmpi/lib#' ./Makefile
    sed -i.bak s'#OCL_AMD_LIB=-L/opt/opencl/amd-app-2.7/lib/x86_64 -lOpenCL -lstdc++#OCL_AMD_LIB=-L'${OCL_LIB_DIR}' -lOpenCL -lstdc++ -lmpi -lmpi_cxx#' ./Makefile
    sed -i.bak s'#FLAGS=$(FLAGS_$(COMPILER)) $(I3E) $(OPTIONS) $(OCL_LIB) -DUSE_EXPLICIT_COMMS_BUFF_PACK -DCLOVER_OUTPUT_FILE=$(CLOVER_OUT_STRING)#FLAGS=$(FLAGS_$(COMPILER)) $(OCL_INC) $(I3E) $(OPTIONS) -DUSE_EXPLICIT_COMMS_BUFF_PACK -DCLOVER_OUTPUT_FILE=$(CLOVER_OUT_STRING)\nLDFLAGS=$(OCL_LIB) -L/usr//lib -L/usr/lib/openmpi/lib -lmpi_f90 -lmpi_f77 -lmpi -ldl -lhwloc#' ./Makefile
    sed -i.bak s'#CFLAGS=$(CFLAGS_$(COMPILER)) $(I3E)#CFLAGS=$(CFLAGS_$(COMPILER)) $(OCL_INC) $(I3E)#' ./Makefile
    sed -i.bak s'#CloverCL.o                      \\#CloverCL.o                      \\\n    $(LDFLAGS)                      \\#' ./Makefile
    sed -i.bak s'#MPI_COMPILER=mpif90#MPI_COMPILER=gfortran-4.8#' ./Makefile
    sed -i.bak s'#-O3#-O3 -g#' ./Makefile
	if [ $AMD_OCL -eq 1 ]; then
		sed -i.bak s'# opencl_vendor=Nvidia# opencl_vendor=Advanced#' ./clover_bm.in
	fi
	if [ $INT_OCL -eq 1 ]; then
		sed -i.bak s'# opencl_vendor=Nvidia# opencl_vendor=Intel#' ./clover_bm.in
	fi
    sed -i.bak s'# end_step=2955# end_step=10#' ./clover_bm.in
    for i in `grep -R ocl_knls.h * | sed s/:/\ /g | awk {'print $1'}`; do
        sed -i.bak s'%#include "ocl_knls.h"%#include "'"$PWD"'/ocl_knls.h"%' $i
    done
    make OCL_VENDOR=AMD COMPILER=GNU -j `nproc`
    if [ $? -ne 0 ]; then
        echo -e "Failed to build CloverLeaf"
        exit -1
    fi
else
    echo -e "~/benchmarks/mantevo/CloverLeaf_OpenCL/clover_leaf exists. Not rebuilding CloverLeaf_OpenCL."
fi

if [ ! -f ~/benchmarks/mantevo/CloverLeaf3D_OpenCL/clover_leaf ]; then
    cd ~/benchmarks/mantevo/CloverLeaf3D_OpenCL/
    git checkout 8772c04f90a9e01ddecf1cecd990b1e3f21e63bf
    sed -i.bak s'#CFLAGS_GNU       = -O3 -march=native -funroll-loops#CFLAGS_GNU       = -O3 -march=native -funroll-loops -I'${OCL_INCLUDE_DIR}' -I/usr/lib/openmpi/include -pthread -I/usr/lib/openmpi/lib#' ./Makefile
    sed -i.bak s'#LDLIBS+=-lOpenCL -lstdc++ $(MPICXX_LIB)#LDLIBS+=-L'${OCL_LIB_DIR}' -lOpenCL -lstdc++ $(MPICXX_LIB)#' ./Makefile
    sed -i.bak s'#FLAGS=$(FLAGS_$(COMPILER)) $(OMP) $(I3E) $(OPTIONS)#FLAGS=$(FLAGS_$(COMPILER)) $(OMP) $(I3E) $(OPTIONS) -I'${OCL_INCLUDE_DIR}' -I/usr/lib/openmpi/include -pthread -I/usr/lib/openmpi/lib#' ./Makefile
    sed -i.bak s'#MPI_COMPILER=mpif90#MPI_COMPILER=gfortran-4.8\nLDFLAGS=-L/usr//lib -L/usr/lib/openmpi/lib -lmpi_f90 -lmpi_f77 -lmpi -ldl -lhwloc#' ./Makefile
    sed -i.bak s'#-O3#-O3 -g#' ./Makefile
	if [ $AMD_OCL -eq 1 ]; then
		sed -i.bak s'#opencl_vendor=nvidia#opencl_vendor=advanced#' ./clover.in
	fi
	if [ $INT_OCL -eq 1 ]; then
		sed -i.bak s'#opencl_vendor=nvidia#opencl_vendor=intel#' ./clover.in
	fi
    sed -i.bak s'#end_step=300#end_step=10#' ./clover.in
    make COMPILER=GNU -j `nproc`
    if [ $? -ne 0 ]; then
        echo -e "Failed to build CloverLeaf3D"
        exit -1
    fi
else
    echo -e "~/benchmarks/mantevo/CloverLeaf3D_OpenCL/clover_leaf exists. Not rebuilding CloverLeaf3D_OpenCL."
fi

