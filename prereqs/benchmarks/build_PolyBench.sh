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


# This script will download PolyBench-ACC from GitHub and build it into the
# ~/benchmarks/PolyBench-ACC/ directory.
# The apps can be run with clarmor --group=POLYBENCH

# Licensing Information:
# PolyBench/ACC uses a 3-clause BSD license. See PolyBench-ACC/LICENSE

BASE_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

MAKE_CMD="make CFLAGS=-DLARGE_DATASET"

#Set the following lines to use double precision instead of floats
USE_DOUBLE=1

if [ ! -f ~/benchmarks/PolyBench-ACC/detector_done_building ]; then
    source ${BASE_DIR}/setup_bench_install.sh
    if [ ! -d ~/benchmarks/PolyBench-ACC/ ]; then
        echo -e "\n\nAbout to log into GitHub to get PolyBench-ACC"
        git clone https://github.com/cavazos-lab/PolyBench-ACC.git
    fi

    cd ~/benchmarks/PolyBench-ACC/
    git checkout 8356309f68fc1a9f26876ec9ca702a77c063b929
    
    cd ~/benchmarks/PolyBench-ACC/OpenCL/
    sed -i.bak s'#/global/homes/s/sgrauerg/NVIDIA_GPU_Computing_SDK#'${OCL_DIR}'#' ~/benchmarks/PolyBench-ACC/OpenCL/utilities/common.mk
    sed -i.bak s'/OpenCL\/common\/inc/include\//' ~/benchmarks/PolyBench-ACC/OpenCL/utilities/common.mk
    sed -i.bak s'/OpenCL\/common\/lib/lib\/x86_64/' ~/benchmarks/PolyBench-ACC/OpenCL/utilities/common.mk
    sed -i.bak s'/O3/g -O3/' ~/benchmarks/PolyBench-ACC/OpenCL/utilities/common.mk
    sed -i.bak s'/\${LIB} \${CFILES}/\${CFLAGS} \${CFILES} \${LIB}/' ~/benchmarks/PolyBench-ACC/OpenCL/utilities/common.mk

    for type in datamining linear-algebra stencils
    do
        if [ $type == "datamining" ]
        then
            for bench in covariance correlation; do
                echo "Doing benchmark $bench"
                sed -i.bak s'/^#define RUN_ON_CPU/\/\/#define RUN_ON_CPU/' ~/benchmarks/PolyBench-ACC/OpenCL/$type/$bench/$bench.c
                if [ $USE_DOUBLE -eq 1 ]; then
                    echo "\n\n\n\n\n USING DOUBLE \n\n\n\n\n"
                    sed -i.bak s'/define DATA_TYPE float/define DATA_TYPE double/' ~/benchmarks/PolyBench-ACC/OpenCL/$type/$bench/$bench.h
                    sed -i.bak s'/typedef float DATA_TYPE/typedef double DATA_TYPE/' ~/benchmarks/PolyBench-ACC/OpenCL/$type/$bench/$bench.cl
                fi
                cd ~/benchmarks/PolyBench-ACC/OpenCL/$type/$bench/
                echo Building in $(pwd)
                $MAKE_CMD
                if [ $? -ne 0 ]; then
                    echo -e "Failed to build datamining application $bench."
                    exit -1
                fi
            done
        fi
        if [ $type == "linear-algebra" ]
        then
            for bench in lu gramschmidt gesummv syr2k 3mm gemm 2mm mvt bicg doitgen atax syrk gemver; do
                if [ $bench == "lu" ] || [ $bench == "gramschmidt" ]; then
                    group="solvers"
                else
                    group="kernels"
                fi
                echo "Doing benchmark $bench in group $group"
                sed -i.bak s'/^#define RUN_ON_CPU/\/\/#define RUN_ON_CPU/' ~/benchmarks/PolyBench-ACC/OpenCL/$type/$group/$bench/$bench.c
                if [ $USE_DOUBLE -eq 1 ]; then
                    echo "\n\n\n\n\n USING DOUBLE \n\n\n\n\n"
                    sed -i.bak s'/define DATA_TYPE float/define DATA_TYPE double/' ~/benchmarks/PolyBench-ACC/OpenCL/$type/$group/$bench/$bench.h
                    sed -i.bak s'/typedef float DATA_TYPE/typedef double DATA_TYPE/' ~/benchmarks/PolyBench-ACC/OpenCL/$type/$group/$bench/$bench.cl
                fi
                cd ~/benchmarks/PolyBench-ACC/OpenCL/$type/$group/$bench/
                echo Building in $(pwd)
                if [ $bench == "gramschmidt" ]; then
                    # Don't do the large dataset for this. It takes forever.
                    make
                else
                    $MAKE_CMD
                fi
                if [ $? -ne 0 ]; then
                    echo -e "Failed to build linear-algebra application $bench."
                    exit -1
                fi
            done
        fi
        if [ $type == "stencils" ]
        then
            for bench in convolution-2d convolution-3d jacobi-2d-imper fdtd-2d adi jacobi-1d-imper
            do
                echo "Doing benchmark $bench"
                if [ $bench == "convolution-2d" ]; then
                    bench_exe=2DConvolution
                elif [ $bench == "convolution-3d" ]; then
                    bench_exe=3DConvolution
                elif [ $bench == "jacobi-2d-imper" ]; then
                    bench_exe=jacobi2D
                elif [ $bench == "fdtd-2d" ]; then
                    bench_exe=fdtd2d
                elif [ $bench == "jacobi-1d-imper" ]; then
                    bench_exe=jacobi1D
                else
                    bench_exe=$bench
                fi
                sed -i.bak s'/^#define RUN_ON_CPU/\/\/#define RUN_ON_CPU/' ~/benchmarks/PolyBench-ACC/OpenCL/$type/$bench/$bench_exe.c
                if [ $USE_DOUBLE -eq 1 ]; then
                    echo "\n\n\n\n\n USING DOUBLE \n\n\n\n\n"
                    sed -i.bak s'/define DATA_TYPE float/define DATA_TYPE double/' ~/benchmarks/PolyBench-ACC/OpenCL/$type/$bench/$bench_exe.h
                    sed -i.bak s'/typedef float DATA_TYPE/typedef double DATA_TYPE/' ~/benchmarks/PolyBench-ACC/OpenCL/$type/$bench/$bench_exe.cl
                fi
                cd ~/benchmarks/PolyBench-ACC/OpenCL/$type/$bench/
                echo Building in $(pwd)
                $MAKE_CMD
                if [ $? -ne 0 ]; then
                    echo -e "Failed to build application $bench."
                    exit -1
                fi
            done
        fi
        cd ~/benchmarks/PolyBench-ACC/OpenCL/
    done
    touch ~/benchmarks/PolyBench-ACC/detector_done_building
else
    echo -e "~/benchmarks/PolyBench-ACC/detector_done_building exists. Not rebuilding PolyBench."
fi
