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

#Install Catalyst drivers
#==============================================================================
mkdir -p $BASE_DIR/drivers
cd $BASE_DIR/drivers/

sudo sed -i.bak 's/199947/262656/' /usr/include/version.h
sudo cp /usr/include/linux/version.h /lib/modules/`uname -r`/build/include/linux/
#!! ON UBUNTU 14.04.4, YOU MUST MODIFY THIS TO SAY THE FOLLOWING !!!!!!!!!!!!!!
# #define LINUX_VERSION_CODE 262656
#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

wget --referer="http://support.amd.com/en-us/download/workstation?os=Linux+x86_64#catalyst-pro" https://www2.ati.com/drivers/firepro/15.302.2301-linux-retail_end_user.zip
unzip 15.302.2301-linux-retail_end_user.zip
cd fglrx-15.302.2301
./amd-driver-installer-15.302.2301-x86.x86_64.run --install
# This will require some manual work to agree to licenses, etc.
# Generate a distribution-specific package and then install it

sudo aticonfig --initial -f

#Install hacks to make OpenCL and graphics applications work over SSH
#==============================================================================
sudo cp $BASE_DIR/support_files/enable-amd-compute.sh /etc/enable-amd-compute.sh
sudo chmod +x /etc/enable-amd-compute.sh

sudo sh -c "echo display-setup-script=/etc/enable-amd-compute.sh >> /etc/lightdm/lightdm.conf"
sudo sh -c "echo source /tmp/enable-amd-compute.sh >> /etc/bash.bashrc"
sudo sh -c "echo shell -\\\$SHELL >> /etc/screenrc"

sudo sh -c "echo [keyfile] >> /etc/NetworkManager/NetworkManager.conf"
sudo sh -c "echo hostname=$HOSTNAME >> /etc/NetworkManager/NetworkManager.conf"

#Reboot at this point
#==============================================================================
sudo reboot
#==============================================================================
#clinfo should show a GPU now.
# dpkg -r fglrx-amdcccle fglrx-dev fglrx
# ^^^ This will uninstall the AMD catalyst/firepro drivers.
