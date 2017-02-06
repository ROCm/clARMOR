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


# This script calls a series of other scripts in order to download and build
# a large series of benchmarks into the ~/benchmarks/ directory.
BASE_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

if [ -f $BASE_DIR/benchmarks/download_all_benchmarks.sh ]; then
    echo -e "\n\nDOWNLOADING BENCHMARKS\n\n"
    $BASE_DIR/benchmarks/download_all_benchmarks.sh
    if [ $? -ne 0 ]; then
        echo -e "Failed to download benchmarks"
        exit -1
    fi
fi

if [ -f $BASE_DIR/benchmarks/build_rodinia.sh ]; then
    echo -e "\n\nBUILDING RODINIA 3.1\n\n"
    $BASE_DIR/benchmarks/build_rodinia.sh
    if [ $? -ne 0 ]; then
        echo -e "Failed to build Rodinia"
        exit -1
    fi
fi

if [ -f $BASE_DIR/benchmarks/build_shoc.sh ]; then
    echo -e "\n\nBUILDING SHOC\n\n"
    $BASE_DIR/benchmarks/build_shoc.sh
    if [ $? -ne 0 ]; then
        echo -e "Failed to build SHOC"
        exit -1
    fi
fi

if [ -f $BASE_DIR/benchmarks/build_OpenDwarfs.sh ]; then
    echo -e "\n\nBUILDING OPENDWARFS\n\n"
    $BASE_DIR/benchmarks/build_OpenDwarfs.sh
    if [ $? -ne 0 ]; then
        echo -e "Failed to build OpenDwarfs"
        exit -1
    fi
fi

if [ -f $BASE_DIR/benchmarks/build_proxy_apps.sh ]; then
    echo -e "\n\nBUILDING EXASCALE PROXY APPLICATIONS\n\n"
    $BASE_DIR/benchmarks/build_proxy_apps.sh
    if [ $? -ne 0 ]; then
        echo -e "Failed to build Proxy Apps"
        exit -1
    fi
fi

if [ -f $BASE_DIR/benchmarks/build_pannotia.sh ]; then
    echo -e "\n\nBUILDING PANNOTIA\n\n"
    $BASE_DIR/benchmarks/build_pannotia.sh
    if [ $? -ne 0 ]; then
        echo -e "Failed to build Pannotia"
        exit -1
    fi
fi

if [ -f $BASE_DIR/benchmarks/build_parboil.sh ]; then
    echo -e "\n\nBUILDING PARBOIL\n\n"
    $BASE_DIR/benchmarks/build_parboil.sh
    if [ $? -ne 0 ]; then
        echo -e "Failed to build Parboil"
        exit -1
    fi
fi

if [ -f $BASE_DIR/benchmarks/build_StreamMR.sh ]; then
    echo -e "\n\nBUILDING STREAMMR\n\n"
    $BASE_DIR/benchmarks/build_StreamMR.sh
    if [ $? -ne 0 ]; then
        echo -e "Failed to build StreamMR"
        exit -1
    fi
fi

if [ -f $BASE_DIR/benchmarks/build_phoronix.sh ]; then
    echo -e "\n\nBUILDING PHORONIX\n\n"
    $BASE_DIR/benchmarks/build_phoronix.sh
    if [ $? -ne 0 ]; then
        echo -e "Failed to build Phoronix"
        exit -1
    fi
fi

if [ -f $BASE_DIR/benchmarks/build_amdapp.sh ]; then
    echo -e "\n\nBUILDING AMD APP SDK SAMPLES\n\n"
    $BASE_DIR/benchmarks/build_amdapp.sh
    if [ $? -ne 0 ]; then
        echo -e "Failed to build AMD APP SDK Samples"
        exit -1
    fi
fi

if [ -f $BASE_DIR/benchmarks/build_PolyBench.sh ]; then
    echo -e "\n\nBUILDING POLYBENCH\n\n"
    $BASE_DIR/benchmarks/build_PolyBench.sh
    if [ $? -ne 0 ]; then
        echo -e "Failed to build PolyBench"
        exit -1
    fi
fi

if [ -f $BASE_DIR/benchmarks/build_FinanceBench.sh ]; then
    echo -e "\n\nBUILDING FINANCEBENCH\n\n"
    $BASE_DIR/benchmarks/build_FinanceBench.sh
    if [ $? -ne 0 ]; then
        echo -e "Failed to build FinanceBench"
        exit -1
    fi
fi

if [ -f $BASE_DIR/benchmarks/build_gpustream.sh ]; then
    echo -e "\n\nBUILDING GPU-STREAM\n\n"
    $BASE_DIR/benchmarks/build_gpustream.sh
    if [ $? -ne 0 ]; then
        echo -e "Failed to build GPU-STREAM"
        exit -1
    fi
fi

if [ -f $BASE_DIR/benchmarks/build_mantevo.sh ]; then
    echo -e "\n\nBUILDING MANTEVO\n\n"
    $BASE_DIR/benchmarks/build_mantevo.sh
    if [ $? -ne 0 ]; then
        echo -e "Failed to build Mantevo Apps"
        exit -1
    fi
fi

if [ -f $BASE_DIR/benchmarks/build_heteromark.sh ]; then
    echo -e "\n\nBUILDING HETEROMARK\n\n"
    $BASE_DIR/benchmarks/build_heteromark.sh
    if [ $? -ne 0 ]; then
        echo -e "Failed to build HeteroMark"
        exit -1
    fi
fi

if [ -f $BASE_DIR/benchmarks/build_npb_ocl.sh ]; then
    echo -e "\n\nBUILDING OPENCL NPB\n\n"
    $BASE_DIR/benchmarks/build_npb_ocl.sh
    if [ $? -ne 0 ]; then
        echo -e "Failed to build OpenCL NAS"
        exit -1
    fi
fi

if [ -f $BASE_DIR/benchmarks/build_linpack.sh ]; then
    echo -e "\n\nBUILDING LINPACK\n\n"
    $BASE_DIR/benchmarks/build_linpack.sh
    if [ $? -ne 0 ]; then
        echo -e "Failed to build LINPACK"
        exit -1
    fi
fi

if [ -f $BASE_DIR/benchmarks/build_viennacl.sh ]; then
    echo -e "\n\nBUILDING VIENNACL\n\n"
    $BASE_DIR/benchmarks/build_viennacl.sh
    if [ $? -ne 0 ]; then
        echo -e "Failed to build ViennaCL"
        exit -1
    fi
fi
