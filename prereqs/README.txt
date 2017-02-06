/********************************************************************************
 * Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ********************************************************************************/


The AMD Research OpenCL Buffer Overflow Detector requires a working GPU OpenCL
implementation. Currently, it has been tested with the AMD APP SDK running on
AMD GPUs. No guarantee is made for being able to use this tool on GPUs from
other companies.

This directory contains a number of files to help install set your system up
to run this tool.

This directory contains download_and_build_benchmarks.sh and the
'benchmarks' subdirectory. The download_and_build script will automatically
download and build a series of 169 benchmarks from 17 different benchmark
suites.
You must manually download Parboil, SNU OpenCL NPB, and ACML yourself, because
of licensing restirctions. However, the script will handle all building and
preparing of the benchmarks for you.



The ubuntu_fresh_install files will set up a fresh install of an Ubuntu
14.04.4 LTS with all the appropriate things needed to build and run all
of the bencmarks that are discussed above.

These files will require a number of reboots, so they are split into
multiple numbered steps that should be run one after another.

Early in this file there is an important note for getting OpenCL to work with
Ubuntu 14.04.4 while connecting over SSH.
It is reproduced here to make sure you read it.
#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# It's highly recommended that when you perform the installation, you set up
# a user (e.g. 'guiuser') that is set to automatically log in w/o a password.
# The AMD Linux OpenCL implementation requires the user running the OpenCL
# kernels to be logged into an X Windows session.
# We will later work around this, but it requires someone to be logged into
# the GUI. This 'guiuser' is a good candidate for that.
#
# The following script assumes you've done this.
#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

If following these directions, you should also make sure that all users are
added to the secondary group 'amdgpu'
