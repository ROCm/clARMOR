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


# This script will download a version of the Pannotia benchmark suite
# from GitHub and build it into the ~/benchmarks/pannotia directory.
# The apps can be run with clarmor.py --group=PANNOTIA

# Licensing Information:
# Pannotia was written by AMD Research and the copyright is owned by AMD.
# In addition, the benchmark is available under an AMD permissive open source
# license. See:
# https://github.com/pannotia/pannotia/blob/master/license

BASE_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
source ${BASE_DIR}/setup_bench_install.sh

# Test to see if the first guy has been build, not all of them
# testing all benchmarks would be a big pain to write..
if [ ! -f ~/benchmarks/pannotia/graph_app/bc/bc ]; then
    if [ ! -d ~/benchmarks/pannotia ]; then
        echo -e "\n\nAbout to log into GitHub to get Pannotia:"
        git clone https://github.com/jlgreathouse/pannotia.git
    fi
    cd ~/benchmarks/pannotia
    git checkout cd63a4874b6cf0b68d79acba0a7e795f7dbbe777

	sed -i.bak 's#OPENCL_DIR = /opt/AMDAPP/#OPENCL_DIR = '${OCL_DIR}'#' ~/benchmarks/pannotia/common/make.config
    make -j `nproc`
    cd ~/benchmarks/pannotia/dataset/sssp
    #wget http://www.dis.uniroma1.it/challenge9/data/USA-road-d/USA-road-d.USA.gr.gz
    #gunzip USA-road-d.USA.gr.gz
    wget http://www.dis.uniroma1.it/challenge9/data/USA-road-d/USA-road-d.NY.gr.gz
    gunzip USA-road-d.NY.gr.gz
else
    echo -e "~/benchmarks/pannotia/graph_app/bc/bc exists. Not rebuilding pannotia."
fi
