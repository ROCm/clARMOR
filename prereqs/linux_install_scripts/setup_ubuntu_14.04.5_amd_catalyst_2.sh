#!/bin/bash
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

# The following script will help set up a fresh Ubuntu 14.04.5 LTS installation
# with the AMD Catalyst drivers.

BASE_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
INSTALLER_DIR=${BASE_DIR}/../install_files/

#Don't run this script again
#==============================================================================
sudo rm -f /etc/xdg/autostart/catal_setup.desktop

#Install Catalyst drivers
#==============================================================================
cd ${INSTALLER_DIR}

sudo sed -i.bak 's/199947/262656/' /usr/include/linux/version.h
sudo cp /usr/include/linux/version.h /lib/modules/`uname -r`/build/include/linux/
#!! ON THE UBUNTU 14.04.4 KERNEL, YOU MUST MODIFY THIS TO SAY THE FOLLOWING !!!
# #define LINUX_VERSION_CODE 262656
#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

wget --referer="http://support.amd.com/en-us/download/workstation?os=Linux+x86_64#catalyst-pro" https://www2.ati.com/drivers/firepro/15.302.2301-linux-retail_end_user.zip
unzip 15.302.2301-linux-retail_end_user.zip
cd fglrx-15.302.2301
sudo ./amd-driver-installer-15.302.2301-x86.x86_64.run --install
# This will require some manual work to agree to licenses, etc.
# Generate a distribution-specific package and then install it

sudo aticonfig --initial -f

#Install hacks to make OpenCL and graphics applications work over SSH
#==============================================================================
sudo cp $BASE_DIR/../support_files/enable-amd-compute.sh /etc/enable-amd-compute.sh
sudo chmod +x /etc/enable-amd-compute.sh

sudo sh -c "echo display-setup-script=/etc/enable-amd-compute.sh >> /etc/lightdm/lightdm.conf"
sudo sh -c "echo source /tmp/enable-amd-compute.sh >> /etc/bash.bashrc"
sudo sh -c "echo shell -\\\$SHELL >> /etc/screenrc"

sudo sh -c "echo [keyfile] >> /etc/NetworkManager/NetworkManager.conf"
sudo sh -c "echo hostname=$HOSTNAME >> /etc/NetworkManager/NetworkManager.conf"

#==============================================================================
#After a reboot, clinfo should show a GPU
# dpkg -r fglrx-amdcccle fglrx-dev fglrx
# ^^^ This will uninstall the AMD catalyst/firepro drivers.

#Install general utilities
#===============================================================================
# Install lubuntu for the graphics APIs that some of the benchmarks needs
sudo apt-get -y install lubuntu-desktop
# Other common build things
sudo apt-get -y install gfortran fort77 mesa-common-dev libboost-dev binutils-dev libboost-dev libcpufreq-dev autoconf automake cmake cmake-curses-gui libtool automake1.11 autotools-dev numactl cpufreqd flex bison libxml2-dev aptitude valgrind dos2unix cppcheck libx11-6:i386 libc6:i386 gcc-multilib g++-multilib libncurses5:i386 libstdc++6:i386 lib32z1 lib32ncurses5 lib32bz2-1.0 lib32stdc++6 libelf-dev libboost-dev libboost-all-dev libswitch-perl qt5-default qttools5-dev-tools libvtk5.8 libboost-thread-dev clang-3.8 clang-3.8-doc libclang-common-3.8-dev libclang-3.8-dev libclang1-3.8 libclang1-3.8-dbg libllvm3.8 llvm-3.8 llvm-3.8-dev llvm-3.8-doc llvm-3.8-examples llvm-3.8-runtime clang-format-3.8 python-clang-3.8 lldb-3.8-dev libstdc++-4.8-dev libdwarf-dev libtinfo-dev libc6-dev-i386 llvm llvm-dev llvm-runtime libc++1 libc++-dev libc++abi1 libc++abi-dev libncurses5-dev
sudo ln -s -f /usr/bin/clang-3.8 /usr/bin/clang
sudo ln -s -f /usr/bin/clang++-3.8 /usr/bin/clang++
sudo ln -s -f /usr/share/clang/scan-build-3.8/bin/scan-build /usr/bin/scan-build
# Need GCC 4.9 for Hetero-Mark
sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
sudo apt-get update
sudo apt-get -y install g++-4.9 gfortran-4.9
sudo ln -sf /usr/bin/gcc-4.9 /usr/bin/gcc
sudo ln -sf /usr/bin/g++-4.9 /usr/bin/g++
sudo ln -sf /usr/bin/gcc-ar-4.9 /usr/bin/gcc-ar
sudo ln -sf /usr/bin/gcc-nm-4.9 /usr/bin/gcc-nm
sudo ln -sf /usr/bin/gcc-ranlib-4.9 /usr/bin/gcc-ranlib
sudo ln -sf /usr/bin/gfortran-4.9 /usr/bin/gfortran

#Install general Python
#===============================================================================
sudo apt-get -y install python-setuptools python-dev python-argparse python-scipy pylint

#Really, really, really turn off screen locking.
#===============================================================================
sudo mv /etc/xdg/autostart/light-locker.desktop /etc/xdg/autostart/light-locker.desktop.bak
sudo mv /etc/xdg/autostart/gnome-screensaver.desktop /etc/xdg/autostart/gnome-screensaver.desktop.bak

#Enable large OpenCL memory buffers on the GPU
#===============================================================================
sudo sh -c "echo export GPU_MAX_ALLOC_PERCENT=100 >> /etc/profile.d/04.OCL.sh"
#Use the command below if you want to be able to access all of the dGPU's memory
sudo sh -c "echo export GPU_FORCE_64BIT_PTR=1 >> /etc/profile.d/04.OCL.sh"

#Enable PerfMon access
#===============================================================================
sudo sh -c "echo kernel.perf_event_paranoid = -1 >> /etc/sysctl.conf"
sudo sh -c "echo kernel.pid_max = 4194304 >> /etc/sysctl.conf"
sudo sh -c "echo kernel.nmi_watchdog = 0 >> /etc/sysctl.conf"

#Install OpenMPI -- Used for some benchmarks
#===============================================================================
sudo apt-get -y install openmpi-bin openmpi-doc libopenmpi-dev

#Reboot at this point
#===============================================================================
sudo reboot
#===============================================================================
