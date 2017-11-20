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

# The following is a step-by-step guide for installing the appropriate
# tools and software for using clARMOR on a fresh Ubuntu 16.04.3 install
# using the ROCm software stack.

# Note that there's very likely some overkill on the installs here. This is
# based off a more comprehensive set of installation directions for a larger
# set of tools inside AMD Research.
# A lot of the X11 libraries are needed for benchmarks like Phoronix, however.

#==============================================================================
#Install Ubuntu 16.04.3 LTS
#==============================================================================
#Use USB thumb drive with Ubuntu 16.04.3 LTS installed to boot
#After booted, select install Ubuntu

BASE_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
INSTALLER_DIR=${BASE_DIR}/../install_files/
REAL_USER=`logname 2>/dev/null || echo ${SUDO_USER:-${USER}}`
su -c "mkdir -p ${INSTALLER_DIR}" $REAL_USER
echo "HLSim Ubuntu 16.04.3 Installation Script (ROCm drivers) Step 1/2"

#Do basic post-install stuff
#==============================================================================
sudo apt-get update
sudo apt-get -y upgrade
sudo apt-get -y install git openssh-server


#Install ROCm
#==============================================================================
sudo sh -c "echo 10.255.8.5      repo.radeon.com >> /etc/hosts"
wget -qO - http://repo.radeon.com/rocm/apt/debian/rocm.gpg.key | sudo apt-key add -
sudo sh -c 'echo deb [arch=amd64] http://repo.radeon.com/rocm/apt/debian/ xenial main > /etc/apt/sources.list.d/rocm.list'
sudo apt-get update

sudo apt-get -y install rocm
sudo apt-get -y install rocm-opencl rocm-opencl-dev
sudo sh -c 'echo export AMDAPPSDKROOT=/opt/rocm/opencl > /etc/profile.d/rocm_ocl.sh'
sudo sh -c 'echo export LD_LIBRARY_PATH=/opt/rocm/opencl/lib/x86_64:/opt/rocm/hsa/lib:\$LD_LIBRARY_PATH >> /etc/profile.d/rocm.sh'
sudo sh -c 'echo export PATH=\$PATH:/opt/rocm/bin/:/opt/rocm/profiler/bin/:/opt/rocm/opencl/bin/x86_64/ >> /etc/profile.d/rocm.sh'
sudo sed /etc/default/grub -i -e 's/GRUB_DEFAULT=[^\n]*/GRUB_DEFAULT=\"1>0\"/g'
sudo update-grub
sudo apt-get -y install aptitude
for i in `aptitude --disable-columns search linux | grep "i A" | awk {'print $3'}`; do echo $i hold | sudo dpkg --set-selections; done

sudo cp -a /opt/rocm/opencl/lib/x86_64/*.so* /usr/lib/.
sudo ln -s /opt/rocm/hsa/lib/libhsa-runtime64.so.1 /opt/rocm/hsa/lib/libhsa-runtime64.so


#Set up the next script to run after the upcoming reboot.
#We must reboot before installing the Catalyst drivers because we need to be
#running the correct kernel version before it tries to build and install the
#modules.
#===============================================================================
INIT_FILE="rocm_setup"
NEXT_SCRIPT=setup_ubuntu_16.04.3_rocm_2.sh
sudo sh -c "echo '#!/bin/bash' > /etc/init.d/${INIT_FILE}"
sudo sh -c "echo ${BASE_DIR}/${NEXT_SCRIPT} >> /etc/init.d/${INIT_FILE}"
sudo chmod 755 /etc/init.d/${INIT_FILE}

sudo sh -c "echo [Desktop Entry] > /etc/xdg/autostart/${INIT_FILE}.desktop"
sudo sh -c "echo Name=${INIT_FILE} >> /etc/xdg/autostart/${INIT_FILE}.desktop"
sudo sh -c "echo Terminal=true >> /etc/xdg/autostart/${INIT_FILE}.desktop"
sudo sh -c "echo Exec=/etc/init.d/${INIT_FILE} >> /etc/xdg/autostart/${INIT_FILE}.desktop"
sudo sh -c "echo Type=Application >> /etc/xdg/autostart/${INIT_FILE}.desktop"
sudo sh -c "echo Categories=Utility\; >> /etc/xdg/autostart/${INIT_FILE}.desktop"

#Reboot at this point
#==============================================================================
sudo reboot
#==============================================================================
