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

# The following script will set up a fresh Ubuntu 14.04.5 LTS installation
# with the AMD Catalyst drivers and the AMD APP SDK 3.0. This will allow
# OpenCL 2.0 applications to be run over SSH connections, etc. and enable
# clARMOR.

# Note that there's very likely some overkill on the installs here. This is
# based off a more comprehensive set of installation directions for a larger
# set of tools inside AMD Research.
# A lot of the X11 libraries are needed for benchmarks like Phoronix, however.

#==============================================================================
#Install Ubuntu 14.04.5 LTS
#==============================================================================
# Use USB thumb drive with Ubuntu 14.04.5 LTS installed to boot
# After booted, select install Ubuntu, put it on your local hard disk, then:

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

BASE_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
INSTALLER_DIR=${BASE_DIR}/../install_files/
mkdir -p ${INSTALLER_DIR}

#Do basic post-install stuff
#==============================================================================
sudo apt-get update
sudo apt-get -y upgrade
sudo apt-get -y install git openssh-server

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
# Need these for the Catalyst driver
sudo apt-get -y install dh-modaliases execstack dpkg-dev debhelper dkms
# General development stuff
sudo apt-get -y install lib32gcc1 build-essential g++ gdb m4 make patch patchutils perl python vim emacs wget libperl-dev libtext-lorem-perl

# Must install the 14.04.4 kernel (Wily -- 4.2) because the version that comes
# with 14.04.5 (4.4) does not support fglrx so we won't get a working graphics
# stack when we install it below.
sudo apt-get -y install linux-generic-lts-wily
sudo update-grub
sudo sed -i.bak 's/GRUB_DEFAULT=0/GRUB_DEFAULT="1>Ubuntu, with Linux 4.2.0-42-generic"/' /etc/default/grub
sudo update-grub
sudo update-grub

# Install an older version of X11, since the version that comes with 14.04.5 does not
# support the fglrx graphics stack
sudo apt-get -y install --install-recommends libqtgui4 xserver-xorg-core-lts-vivid xserver-xorg-lts-vivid xserver-xorg-video-all-lts-vivid xserver-xorg-input-all-lts-vivid libwayland-egl1-mesa-lts-vivid libgl1-mesa-glx-lts-vivid libglapi-mesa-lts-vivid libgles1-mesa-lts-vivid libegl1-mesa-lts-vivid xserver-xorg-dev-lts-vivid mesa-common-dev-lts-vivid libxatracker-dev-lts-vivid libgles2-mesa-dev-lts-vivid libgles1-mesa-dev-lts-vivid libgl1-mesa-dev-lts-vivid libgbm-dev-lts-vivid libegl1-mesa-dev-lts-vivid
# Need libglew etc. for some Phoronix benchmarks which use the GUI
sudo apt-get -y install freeglut3 freeglut3-dev libglew-dev
sudo ln -s /usr/lib/x86_64-linux-gnu/libglut.so.3 /usr/lib/x86_64-linux-gnu/libglut.so

#Install the AMD APP SDK - This MUST be done before installing the driver
#==============================================================================
cd ${INSTALLER_DIR}
sudo ${BASE_DIR}/../support_files/get_amd_app_sdk.sh -i -d $(pwd)

#Set up the second-half script to run after the upcoming reboot.
#We must reboot before installing the Catalyst drivers because we need to be
#running the correct kernel version before it tries to build and install the
#modules.
#===============================================================================
sudo sh -c "echo #!/bin/bash > /etc/init.d/catal_setup"
sudo sh -c "echo echo Preparing to complete system setup. >> /etc/init.d/catal_setup"
sudo sh -c "echo echo This will require you to give your sudoers password. >> /etc/init.d/catal_setup"
sudo sh -c "echo echo Please wait while the install completes. >> /etc/init.d/catal_setup"
sudo sh -c "echo echo You will need to interact with the Catalyst tool to install your graphics drivers. >> /etc/init.d/catal_setup"
sudo sh -c "echo echo Later, this script will reboot the system when everything is done. >> /etc/init.d/catal_setup"
sudo sh -c "echo sudo ${BASE_DIR}/setup_ubuntu_14.04.5_amd_catalyst_2.sh >> /etc/init.d/catal_setup"
sudo sh -c "echo sudo rm -f /etc/init.d/catal_setup >> /etc/init.d/catal_setup"
sudo chmod 755 /etc/init.d/catal_setup

sudo sh -c "echo [Desktop Entry] > /etc/xdg/autostart/catal_setup.desktop"
sudo sh -c "echo Name=catal_setup >> /etc/xdg/autostart/catal_setup.desktop"
sudo sh -c "echo Terminal=true >> /etc/xdg/autostart/catal_setup.desktop"
sudo sh -c "echo Exec=/etc/init.d/catal_setup >> /etc/xdg/autostart/catal_setup.desktop"
sudo sh -c "echo Type=Application >> /etc/xdg/autostart/catal_setup.desktop"
sudo sh -c "echo Categories=Utility\; >> /etc/xdg/autostart/catal_setup.desktop"

#Reboot at this point
#===============================================================================
sudo reboot
#===============================================================================
