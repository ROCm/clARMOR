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


# The following is a step-by-step guide for installing the appropriate
# tools and software for using clARMOR on a fresh Ubuntu 14.04.4 install.

# Note that there's very likely some overkill on the installs here. This is
# based off a more comprehensive set of installation directions for a larger
# set of tools inside AMD Research.
# A lot of the X11 libraries are needed for benchmarks like Phoronix, however.

#==============================================================================
#Install Ubuntu 14.04.4 LTS
#==============================================================================
#Use USB thumb drive with Ubuntu 14.04.4 LTS installed to boot
#After booted, select install Ubuntu


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

# Check to see if the guiuser was installed, as requested above.
#==============================================================================
getent passwd guiuser &> /dev/null
user_exists=$?
if [ $user_exists -ne 0 ]; then
    echo -e "The user 'guiuser' does not exist."
    echo -e "This script requires that user to exist to get OpenCL working."
    echo -e "Please see the comment at the top of this script for the raesons."
    exit -1
fi

# Check to see if the AMD APP SDK installer is available.
# If not, give an error and quit, because we want the user to download it.
# If it's around, we will install it later, which will require manual
# interaction with the user.
#==============================================================================
BASE_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
if [ ! -f $BASE_DIR/AMD-APP-SDKInstaller-v3.0.130.136-GA-linux64.tar.bz2 ]; then
    echo -e "Unable to find the AMD APP SDK installation files, AMD-APP-SDKInstaller-v3.0.130.136-GA-linux64.tar.bz2"
    echo -e "Downloading the AMD APP SDK requires agreeing to a license."
    echo -e "As such, we cannot automatically download it for you."
    echo -e "Please go to http://developer.amd.com/tools-and-sdks/opencl-zone/amd-accelerated-parallel-processing-app-sdk/"
    echo -e "   Download the file and put it into $BASE_DIR"
    exit -1
fi

#Do basic post-install stuff
#==============================================================================
sudo apt-get update
sudo apt-get -y upgrade
sudo apt-get -y install openssh-server nfs-common sysstat memstat pbzip2 screen

#Add a group that all users will share with the guiuser that you should have
# added during the installation.
# You need this to get OpenCL to work over SSH.
#==============================================================================
sudo addgroup amdgpu
sudo usermod -a -s /bin/bash -G amdgpu guiuser
#Add a user 'guiuser' and set it to automatically login and system startup.

#Optionally disable turning off the screen, to improve reliability of GPU estimates.
sudo -u guiuser gsettings set org.gnome.desktop.screensaver lock-delay "0"
sudo -u guiuser gsettings set org.gnome.desktop.session idle-delay 0
sudo -u guiuser gsettings set org.gnome.desktop.screensaver lock-enabled false
sudo -u guiuser gsettings set org.gnome.desktop.screensaver ubuntu-lock-on-suspend 'false'
sudo sh -c "echo [SeatDefaults] > /etc/lightdm/lightdm.conf"
sudo sh -c "echo xserver-allow-tcp=true >> /etc/lightdm/lightdm.conf"
sudo sh -c "echo autologin-user=guiuser >> /etc/lightdm/lightdm.conf"

#sudo usermod -a -G amdgpu {all users on the system}

#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#!!! All users should be added with -G amdgpu
#!!! All users must be in the group 'amdgpu' in order to use the GPU
#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#Install some pre-requisites
#==============================================================================
sudo apt-get update
sudo apt-get -y upgrade
sudo apt-get -y install dh-modaliases execstack dpkg-dev debhelper dkms lib32gcc1 libqtgui4 linux-headers-generic dh-make cdbs build-essential cscope cscope-el diffstat diffutils elfutils g++ gdb less m4 make makedev mercurial patch patchutils perl protobuf-compiler python python-dev scons tcpdump tcsh vim wget xemacs21 zlib1g zlib1g-dev libprotobuf8 libprotobuf-dev libperl-dev dwarfdump libdw-dev libgtk2.0-dev environment-modules gawk libtext-lorem-perl
sudo apt-get -y install --install-recommends xserver-xorg-core-lts-vivid xserver-xorg-lts-vivid xserver-xorg-video-all-lts-vivid xserver-xorg-input-all-lts-vivid libwayland-egl1-mesa-lts-vivid libgl1-mesa-glx-lts-vivid libglapi-mesa-lts-vivid libgles1-mesa-lts-vivid libegl1-mesa-lts-vivid xserver-xorg-dev-lts-vivid mesa-common-dev-lts-vivid libxatracker-dev-lts-vivid libgles2-mesa-dev-lts-vivid libgles1-mesa-dev-lts-vivid libgl1-mesa-dev-lts-vivid libgbm-dev-lts-vivid libegl1-mesa-dev-lts-vivid freeglut3 freeglut3-dev libglew-dev
sudo ln -s /usr/lib/x86_64-linux-gnu/libglut.so.3 /usr/lib/x86_64-linux-gnu/libglut.so
sudo apt-get -y remove kerneloops bluetooth modemmanager

#Install the AMD APP SDK - This MUST be done before installing the driver
#==============================================================================
cd $BASE_DIR
echo -e "Untarring the AMD APP SDK"
tar -xf AMD-APP-SDKInstaller-v3.0.130.136-GA-linux64.tar.bz2
sudo ./AMD-APP-SDK-v3.0.130.136-GA-linux64.sh --nox11 -- -a -s
sudo ln -s /opt/AMDAPPSDK-3.0 /opt/AMDAPP
sudo sed -i 's/x86_64\//x86_64\/:\$LD_LIBRARY_PATH/' /etc/profile.d/AMDAPPSDK.sh
sudo sed -i 's/export LD_LIBRARY_PATH/#export LD_LIBRARY_PATH/' /etc/profile.d/AMDAPPSDK.sh
sudo sh -c "echo export LD_LIBRARY_PATH=/usr/lib/:\\\$LD_LIBRARY_PATH >> /etc/profile.d/AMDAPPSDK.sh"
sudo ln -s /opt/AMDAPP/lib/x86_64/sdk/libamdocl64.so  /opt/AMDAPP/lib/x86_64/libamdocl64.so
sudo ln -fs /opt/AMDAPP/lib/x86_64/sdk/libOpenCL.so /opt/AMDAPP/lib/x86_64/libOpenCL.so
sudo ln -s /opt/AMDAPP/lib/x86_64/sdk/libOpenCL.so.1 /opt/AMDAPP/lib/x86_64/libOpenCL.so.1

#Reboot at this point
#==============================================================================
sudo reboot
#==============================================================================
