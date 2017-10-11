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


# This script will download the Phoronix Test Suite benchmarks from the web and
# build them into the ~/benchmarks/phoronix directory.
# The apps can be run with clarmor --group=PHORONIX

# Licensing Information:
#  - JuliaGPU uses an MIT license. See: JuliaGPU-v1.2pts/LICENSE.txt
#  - mandelbulbGPU uses an MIT license. See: mandelbulbGPU-v1.0pts/LICENSE.txt
#  - MandelGPU uses an MIT license. See: MandelGPU-v1.3pts/LICENSE.txt
#  - SmallPT-GPU uses an MIT license. See: SmallptGPU-v1.6pts/LICENSE.txt

BASE_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

if [ ! -d ~/benchmarks/phoronix ]; then
    mkdir -p ~/benchmarks/phoronix
fi

if [ ! -d ~/benchmarks/phoronix/JuliaGPU-v1.2pts ] ||\
    [ ! -d ~/benchmarks/phoronix/MandelGPU-v1.3pts ] ||\
    [ ! -d ~/benchmarks/phoronix/SmallptGPU-v1.6pts ] ||\
    [ ! -d ~/benchmarks/phoronix/mandelbulbGPU-v1.0pts ];
then
    source ${BASE_DIR}/setup_bench_install.sh
fi

cd ~/benchmarks/phoronix

if [ ! -d ~/benchmarks/phoronix/JuliaGPU-v1.2pts ]; then
    if [ ! -f ~/benchmarks/phoronix/JuliaGPU-v1.2pts-1.tar.bz2 ]; then
        wget http://www.phoronix-test-suite.com/benchmark-files/JuliaGPU-v1.2pts-1.tar.bz2
    fi
    tar -xvf JuliaGPU-v1.2pts-1.tar.bz2
    cd JuliaGPU-v1.2pts/
    sed -i.bak 's#/home/david/src/ati-stream-sdk-v2.0-lnx64#'${OCL_DIR}'#' ./Makefile
    sed -i.bak s'/-L.\+CL/\nLDFLAGS=-L\$(ATISTREAMSDKROOT)\/lib\/x86_64 -lglut -lglut -lGL -lm -lOpenCL/' ./Makefile
    sed -i.bak s'/displayfunc.c$/displayfunc.c \$(LDFLAGS)/' ./Makefile
    sed -i.bak s'/O3/g -O3/' ./Makefile
    sed -i 's/exit(0)/glutLeaveMainLoop()/g' juliaGPU.c
    make -j `nproc`
else
    echo -e "~/benchmarks/phoronix/JuliaGPU-v1.2pts exists. Not rebuilding juliaGPU."
fi

cd ~/benchmarks/phoronix

if [ ! -d ~/benchmarks/phoronix/MandelGPU-v1.3pts ]; then
    if [ ! -f ~/benchmarks/phoronix/MandelGPU-v1.3pts-1.tar.bz2 ]; then
        wget http://www.phoronix-test-suite.com/benchmark-files/MandelGPU-v1.3pts-1.tar.bz2
    fi
    tar -xvf MandelGPU-v1.3pts-1.tar.bz2
    cd MandelGPU-v1.3pts/
    sed -i.bak 's#/home/david/src/ati-stream-sdk-v2.0-lnx64#'${OCL_DIR}'#' ./Makefile
    sed -i.bak s'/-L.\+CL/\nLDFLAGS=-L\$(ATISTREAMSDKROOT)\/lib\/x86_64 -lglut -lglut -lGL -lm -lOpenCL/' ./Makefile
    sed -i.bak s'/displayfunc.c$/displayfunc.c \$(LDFLAGS)/' ./Makefile
    sed -i.bak s'/O3/g -O3/' ./Makefile
    sed -i 's/exit(0)/glutLeaveMainLoop()/g' mandelGPU.c
    make -j `nproc`
else
    echo -e "~/benchmarks/phoronix/MandelGPU-v1.3pts exists. Not rebuilding MandelGPU."
fi

cd ~/benchmarks/phoronix

if [ ! -d ~/benchmarks/phoronix/SmallptGPU-v1.6pts ]; then
    if [ ! -f ~/benchmarks/phoronix/SmallptGPU-v1.6pts-1.tar.bz2 ]; then
        wget http://www.phoronix-test-suite.com/benchmark-files/SmallptGPU-v1.6pts-1.tar.bz2
    fi
    tar -xvf SmallptGPU-v1.6pts-1.tar.bz2
    cd SmallptGPU-v1.6pts
    sed -i.bak 's#/home/david/src/ati-stream-sdk-v2.0-lnx64#'${OCL_DIR}'#' ./Makefile
    sed -i.bak s'/-L.\+CL/\nLDFLAGS=-L\$(ATISTREAMSDKROOT)\/lib\/x86_64 -lglut -lglut -lGL -lm -lOpenCL/' ./Makefile
    sed -i.bak s'/displayfunc.c$/displayfunc.c \$(LDFLAGS)/' ./Makefile
    sed -i.bak s'/O3/g -O3/' ./Makefile
    sed -i.bak s'#if (elapsedTime > tresholdTime)#//if (elapsedTime > tresholdTime)#' ./smallptGPU.c
    sed -i 's/exit(0)/glutLeaveMainLoop()/g' smallptGPU.c
    make -j `nproc`
else
    echo -e "~/benchmarks/phoronix/SmallptGPU-v1.6pts exists. Not rebulding SmallptGPU."
fi

cd ~/benchmarks/phoronix

if [ ! -d ~/benchmarks/phoronix/mandelbulbGPU-v1.0pts ]; then
    if [ ! -f ~/benchmarks/phoronix/mandelbulbGPU-v1.0pts-1.tar.bz2 ]; then
        wget http://www.phoronix-test-suite.com/benchmark-files/mandelbulbGPU-v1.0pts-1.tar.bz2
    fi
    tar -xvf mandelbulbGPU-v1.0pts-1.tar.bz2
    cd mandelbulbGPU-v1.0pts
    sed -i.bak 's#/home/david/src/ati-stream-sdk-v2.0-lnx64#'${OCL_DIR}'#' ./Makefile
    sed -i.bak s'/-L.\+CL/\nLDFLAGS=-L\$(ATISTREAMSDKROOT)\/lib\/x86_64 -lglut -lglut -lGL -lm -lOpenCL/' ./Makefile
    sed -i.bak s'/displayfunc.c$/displayfunc.c \$(LDFLAGS)/' ./Makefile
    sed -i.bak s'/O3/g -O3/' ./Makefile
    sed -i 's/exit(0)/glutLeaveMainLoop()/g' mandelbulbGPU.c
    make -j `nproc`
else
    echo -e "~/benchmarks/phoronix/mandelbulbGPU-v1.0pts exists. Not rebuilding mandelbulbGPU."
fi
