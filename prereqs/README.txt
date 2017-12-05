# Copyright (c) 2016-2017 Advanced Micro Devices, Inc. All rights reserved.
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

clARMOR requires a working GPU OpenCL implementation. Currently, it has been
tested most rigorously with the AMD APP SDK running on AMD GPUs. This tool
has been tested on Nvidia GPUs with the CUDA SDK OpenCL implementation, but
no guarantee is made for being able to use this tool on GPUs from other
companies.

This directory contains a number of files to help install set your system up
to run this tool.

This directory contains download_and_build_benchmarks.sh and the
'benchmarks' subdirectory. The download_and_build script will automatically
download and build a series of 169 benchmarks from 17 different benchmark
suites (for AMD APP SDK versions that support OpenCL 2.0).
You must manually download Parboil and SNU OpenCL NPB yourself, because
of licensing restirctions. However, the script will handle all building and
preparing of the benchmarks for you.


In addition, there are a number of scripts available to help set up OpenCL
on fresh installations of your operating system. For example:
setup_ubuntu_14.04.5_amd_catalyst.sh will set up the AMD FirePro (Catalyst)
drivers on a fresh installation of Ubuntu 14.04.5 LTS, download anc configure
the AMD APP SDK, and allow OpenCL applications and clARMOR to run after the
next reboot.

After running the setup scripts, the benchmarks should be able to cleanly
install and run.

Early in the Ubuntu 14.04.5/Catalyst setup file there is an important note
for getting OpenCL to work with Ubuntu 14.04.5 while connecting over SSH.
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
