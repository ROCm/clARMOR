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


# This script will download ViennaCL 1.7.1 from the NA cluster head node
# and build it into the ~/benchmarks/ViennaCL-1.7.1 directory.
# The apps can be run with run_overflow_detect.py --group=VIENNACL

# Licensing Information:
# ViennaCL is distributed under an MIT license. See: ViennaCL-1.7.1/LICENSE

if [ ! -d ~/benchmarks ]; then
    mkdir -p ~/benchmarks
fi

cd ~/benchmarks

if [ ! -f ~/benchmarks/ViennaCL-1.7.1/build/examples/benchmarks/dense_blas-bench-opencl ]; then
    if [ ! -f ~/benchmarks/ViennaCL-1.7.1.tar.gz ];
    then
        cd ~/benchmarks/
        echo -e "About to download ViennaCL from SourceForge"
        wget http://downloads.sourceforge.net/project/viennacl/1.7.x/ViennaCL-1.7.1.tar.gz
        if [ $? -ne 0 ]; then
            echo -e "Failed to download file"
            exit -1
        fi
    fi
    tar -xvf ViennaCL-1.7.1.tar.gz
    cd ViennaCL-1.7.1/
    # Make the dense_blas benchmark deterministic.
    sed -i.bak 's#double time_per_benchmark = 1#size_t times_through = 10#' ./examples/benchmarks/dense_blas.cpp
    sed -i.bak 's#time_spent < time_per_benchmark#Nruns < times_through#' ./examples/benchmarks/dense_blas.cpp
    # Make the amg benchmark deterministic
    sed -i.bak 's#amg_tag_direct);#amg_tag_direct);\n  return EXIT_SUCCESS;#' ./examples/tutorial/amg.cpp
    # Make the input size for bisect larger.
    sed -i.bak 's#mat_size = 30#mat_size = 10000#' ./examples/tutorial/bisect.cpp
    # Make the input size for matrix-free larger and stop its large output.
    sed -i.bak 's#size_t N = 10#size_t N = 10000#' ./examples/tutorial/matrix-free.cpp
    sed -i.bak 's/y " << std::endl;/y " << std::endl;\n#if 0/' ./examples/tutorial/matrix-free.cpp
    sed -i.bak 's/std::cout << "\*----/#endif\n  std::cout << "*----/' ./examples/tutorial/matrix-free.cpp
    # Make the input size for nmf larger and stop its outputs
    sed -i.bak 's#int m = 7#int m = 170#' ./examples/tutorial/nmf.cpp
    sed -i.bak 's#int n = 6#int n = 160#' ./examples/tutorial/nmf.cpp
    sed -i.bak 's#int k = 3#int k = 130#' ./examples/tutorial/nmf.cpp
    sed -i.bak 's#std::cout << "Input matrices:" << std::endl;#/*std::cout << "Input matrices:" << std::endl;#' ./examples/tutorial/nmf.cpp
    sed -i.bak 's#std::cout << "RESULT:" << std::endl;#/*std::cout << "RESULT:" << std::endl;#' ./examples/tutorial/nmf.cpp
    sed -i.bak 's#std::cout << "H" << H << "\\n" << std::endl;#std::cout << "H" << H << "\\n" << std::endl;*/#' ./examples/tutorial/nmf.cpp
    cd ./build
    cmake -DBUILD_TESTING=ON ..
    make -j `nproc`
    if [ $? -ne 0 ]; then
        echo -e "Failed to build ViennaCL."
        exit -1
    fi
else
    echo -e "~/benchmarks/ViennaCL-1.7.1/build/examples/benchmarks/dense_blas-bench-opencl exists. Not rebuilding ViennaCL."
fi
