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


# This is the second step of installing the the software on a fresh
# Ubuntu 14.04.4 LTS. This script installs the Catalyst drivers.

# Note that there's very likely some overkill on the installs here. This is
# based off a more comprehensive set of installation directions for a larger
# set of tools inside AMD Research.
# A lot of the X11 libraries are needed for benchmarks like Phoronix, however.

BASE_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

#Install general utilities
#===============================================================================
sudo apt-get -y install gfortran fort77 vim-gnome openssh-server nano emacs nfs-common  unzip linux-headers-generic lubuntu-desktop libpam-cracklib pssh mpich2 cpufrequtils tmux finger mesa-common-dev libboost-dev binutils-dev libboost-dev cmake libcpufreq-dev libtool r-base r-base-dev autoconf automake automake1.11 autotools-dev exuberant-ctags vim-gui-common mercurial libunwind8-dev libaudit-dev libslang2-dev libnuma-dev numactl vnc4server cpufreqd eclipse zsh screen emacs ipython libev-dev flex bison gufw libxml2-dev keyutils cifs-utils execstack libtimedate-perl libdpkg-perl dpkg-dev libalgorithm-diff-perl libalgorithm-diff-xs-perl libalgorithm-merge-perl liberror-perl dkms gnuplot libreoffice aptitude valgrind google-perftools libgoogle-perftools-dev xfonts-75dpi linux-tools-generic-lts-wily dos2unix ddd pkg-config gettext libssl-dev libpcre3-dev libreadline-dev libnlopt-dev libgraphviz-dev mesa-utils cppcheck unattended-upgrades cmake-curses-gui libleveldb-dev libsnappy-dev libopencv-dev libhdf5-serial-dev libatlas-base-dev libgflags-dev libgoogle-glog-dev liblmdb-dev electric-fence parallel libx11-6:i386 libc6:i386 gcc-multilib g++-multilib libncurses5:i386 libstdc++6:i386 lib32z1 lib32ncurses5 lib32bz2-1.0 lib32stdc++6 msr-tools git subversion lm-sensors libelf-dev libboost-dev libboost-all-dev astyle tmux ruby-full libswitch-perl qt5-default qttools5-dev-tools libvtk5.8 libboost-thread-dev clang-3.5 clang-3.5-doc libclang-common-3.5-dev libclang-3.5-dev libclang1-3.5 libclang1-3.5-dbg libllvm-3.5-ocaml-dev libllvm3.5 libllvm3.5-dbg lldb-3.5 llvm-3.5 llvm-3.5-dev llvm-3.5-doc llvm-3.5-examples llvm-3.5-runtime clang-modernize-3.5 clang-format-3.5 python-clang-3.5 lldb-3.5-dev libstdc++-4.8-dev libdwarf-dev libtinfo-dev libc6-dev-i386 llvm llvm-dev llvm-runtime libc++1 libc++-dev libc++abi1 libc++abi-dev re2c libncurses5-dev
sudo ln -s -f /usr/bin/clang-3.5 /usr/bin/clang
sudo ln -s -f /usr/bin/clang++-3.5 /usr/bin/clang++
sudo ln -s -f /usr/share/clang/scan-build-3.5/scan-build /usr/bin/scan-build
sudo sh -c "echo msr >> /etc/modules"

#Install general Python
#===============================================================================
sudo apt-get -y install python-numpy python-scipy python-matplotlib ipython ipython-notebook python-pandas python-sympy python-nose python-setuptools python-dev python-argparse pylint
sudo easy_install pip
sudo pip install -U pyyaml Cython vulture

#Really, really, really turn off screen locking.
#===============================================================================
sudo mv /etc/xdg/autostart/light-locker.desktop /etc/xdg/autostart/light-locker.desktop.bak

#Enable large OpenCL memory buffers on the GPU
#===============================================================================
sudo sh -c "echo export GPU_MAX_ALLOC_PERCENT=100 >> /etc/profile.d/04.OCL.sh"
#Use the command below if you want to be able to access all of the dGPU's memory
sudo sh -c "echo export GPU_FORCE_64BIT_PTR=1 >> /etc/profile.d/04.OCL.sh"

#Disable ASLR and Enable PerfMon access
#===============================================================================
sudo sh -c "echo kernel.randomize_va_space = 0 >> /etc/sysctl.conf"
sudo sh -c "echo kernel.perf_event_paranoid = -1 >> /etc/sysctl.conf"
sudo sh -c "echo kernel.kptr_restrict = 0 >> /etc/sysctl.conf"
sudo sh -c "echo kernel.panic = 10 >> /etc/sysctl.conf"
sudo sh -c "echo kernel.panic_on_io_nmi = 1 >> /etc/sysctl.conf"
sudo sh -c "echo kernel.panic_on_oops = 1 >> /etc/sysctl.conf"
sudo sh -c "echo kernel.panic_on_unrecovered_nmi = 1 >> /etc/sysctl.conf"
sudo sh -c "echo kernel.pid_max = 4194304 >> /etc/sysctl.conf"
sudo sh -c "echo kernel.nmi_watchdog = 0 >> /etc/sysctl.conf"

#Install OpenMPI
#===============================================================================
sudo apt-get -y install openmpi-bin openmpi-doc libopenmpi-dev

#Fix cpufreq to work with SETUID
#===============================================================================
sudo apt-get -y install super
mkdir -p ~/Downloads/software/cpufrequtils/
cd ~/Downloads/software/cpufrequtils/
wget https://launchpad.net/ubuntu/+archive/primary/+files/cpufrequtils_008.orig.tar.bz2
tar -xf cpufrequtils_008.orig.tar.bz2
cd cpufrequtils-008/
patch -i $BASE_DIR/support_files/cpufrequtils.patch ./utils/aperf.c
make -j `nproc`
sudo mv cpufreq-aperf /usr/bin/
sudo chown root:root /usr/bin/cpufreq-aperf
sudo chmod u+s /usr/bin/cpufreq-aperf
sudo chmod u+s /usr/bin/cpufreq-set

cd ~/Downloads/software/
#Download the "boost_apps" directory into ~/Downloads/software/
sudo chown -R root:root ./boost_apps/*
sudo chmod a+x ./boost_apps/*
sudo mv ./boost_apps/* /usr/bin/
sudo sh -c "echo disable_boost_helper /usr/bin/disable_boost_helper :amdgpu uid=0 >> /etc/super.tab"
sudo sh -c "echo enable_boost_helper /usr/bin/enable_boost_helper :amdgpu uid=0 >> /etc/super.tab"

#Reboot at this point
#===============================================================================
sudo reboot
#===============================================================================
