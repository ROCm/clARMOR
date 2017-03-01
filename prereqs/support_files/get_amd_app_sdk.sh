#!/bin/bash
# Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
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

# This script will get the AMD APP SDK v3.0 and put it into the 
BASE_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

print_help_header()
{
    echo -e "This script will download and perform an installation of the" 1>&2
    echo -e "AMD APP SDK so that OpenCL applications can be built against it" 1>&2
    echo -e "and can run on the local CPU device." 1>&2
    echo -e "By default, it will put the AMD APP SDK installation into a" 1>&2
    echo -e "subdirectory, ./AMDAPP/, within the same directory as the script." 1>&2
    echo -e "It is also possible to perform a full installation to /opt/AMDAPP/" 1>&2
    echo -e ""
}

print_usage()
{
    echo -e "Usage: $0 [-h] [-d TARGET_DIRECTORY] [-v {2/3}]" 1>&2
    echo -e "   -h will print this help message and exit." 1>&2
    echo -e "   -d allows you to set the target directory that ./AMDAPP/ will be put into" 1>&2
    echo -e "          By default, this is the directory this script is in." 1>&2
    echo -e "   -v sets the version of the AMD APP SDK to download, 3 or 2." 1>&2
    echo -e "          By default, this is 3." 1>&2
    echo -e "          v3 supports OpenCL 2.0 on AMD GPUs, v2 only supports OpenCL 1.2." 1>&2
}

print_help_footer()
{
    echo -e ""
    echo -e "This script will return -1 if something fails before it completes."
}

print_help_and_exit()
{
    print_help_header
    print_usage
    print_help_footer
    exit 0
}

print_error_and_exit()
{
    echo -e "ERROR on command line parameter."
    print_usage
    exit -1
}

amdapp_version=300
target_dir=${BASE_DIR}/AMDAPP/
temp_dir=/tmp/fake_temp_dir/
do_installation=0

while getopts "hd:v:i" option;
do
    case "${option}" in
        h)
            print_help_and_exit
            ;;
        v)
            if [ ${OPTARG} -eq 2 ]; then
                amdapp_version=291
            elif [ ${OPTARG} -eq 3 ]; then
                amdapp_version=300
            else
                print_error_and_exit
            fi
            ;;
        d)
            target_dir=${OPTARG}/AMDAPP/
            ;;
        i)
            do_installation=1
            ;;
        *)
            print_error_and_exit
            ;;
    esac
done

mkdir ${target_dir}
if [ $? -ne 0 ]
then
    echo -e "Cannot create installation directory ${target_dir}"
    exit -1
fi

# Get the AMD APP SDK using the Boost.org Compute method
temp_dir=`mktemp -d`
cd ${temp_dir}
git clone -n https://github.com/boostorg/compute.git --depth 1
if [ $? -ne 0 ]
then
    echo -e "Failed to download amd_sdk.sh from Boost.org GitHub"
    rm -rf ${temp_dir}
    exit -1
fi
cd ./compute/
git checkout HEAD ./.travis/amd_sdk.sh
cp ./.travis/amd_sdk.sh ${target_dir}
if [ $? -ne 0 ]
then
    echo -e "Failed to find amd_sdk.sh from Boost.org GitHub"
    rm -rf ${temp_dir}
    exit -1
fi

# Move back to the main directory and delete the temp dir
cd ${target_dir}
rm -rf ${temp_dir}
chmod +x ./amd_sdk.sh

# We now have the script to download the AMD APP SDK.
# Download the tarball of the correct version
${target_dir}/amd_sdk.sh ${amdapp_version}
if [ $? -ne 0 ]
then
    echo -e "Failed to properly download the AMD APP SDK"
    exit -1
fi

tar -xf AMD-SDK.tar.bz2
if [ $? -ne 0 ]
then
    echo -e "Failed to untar the file: ${target_dir}/AMD-SDK.tar.bz2"
    exit -1
fi

# We now have an AMD APP SDK installerscript.
# And complete what we need to do in order to "install" it.
if [ $do_installation -eq 1 ]
then
    bash AMD-APP-SDK*.sh --nox11 -- -a -s
    if [ $? -ne 0 ]
    then
        echo -e "Failed to install APP SDK from shell script."
        exit -1
    fi
    if [ ${amdapp_version} -eq 300 ]; then
        ln -s /opt/AMDAPPSDK-3.0 /opt/AMDAPP
        ln -s /opt/AMDAPP/lib/x86_64/sdk/libamdocl64.so  /opt/AMDAPP/lib/x86_64/libamdocl64.so
        ln -fs /opt/AMDAPP/lib/x86_64/sdk/libOpenCL.so /opt/AMDAPP/lib/x86_64/libOpenCL.so
        ln -s /opt/AMDAPP/lib/x86_64/sdk/libOpenCL.so.1 /opt/AMDAPP/lib/x86_64/libOpenCL.so.1
    fi
    sed -i 's/x86_64\//x86_64\/:\$LD_LIBRARY_PATH/' /etc/profile.d/AMDAPPSDK.sh
    sed -i 's/export LD_LIBRARY_PATH/#export LD_LIBRARY_PATH/' /etc/profile.d/AMDAPPSDK.sh
    sh -c "echo export LD_LIBRARY_PATH=/usr/lib/:\\\$LD_LIBRARY_PATH >> /etc/profile.d/AMDAPPSDK.sh"
fi
old_AMDAPPSDKROOT=${AMDAPPSDKROOT}
AMDAPPSDKROOT=${target_dir}
export OPENCL_VENDOR_PATH=${AMDAPPSDKROOT}/etc/OpenCL/vendors
mkdir -p ${OPENCL_VENDOR_PATH}
bash AMD-APP-SDK*.sh --nox11 --tar -xf -C ${AMDAPPSDKROOT}
if [ $? -ne 0 ]
then
    echo -e "Failed to install APP SDK from shell script."
    exit -1
fi

echo libamdocl64.so > ${OPENCL_VENDOR_PATH}/amdocl64.icd
if [ $? -ne 0 ]
then
    echo -e "Failed to set up amdocl64.icd"
    exit -1
fi

if [ ${amdapp_version} -eq 300 ]
then
    rm -f ${AMDAPPSDKROOT}/lib/x86_64/libOpenCL.so
    ln -s ${AMDAPPSDKROOT}/lib/x86_64/sdk/libOpenCL.so.1 ${AMDAPPSDKROOT}/lib/x86_64/libOpenCL.so
    if [ $? -ne 0 ]
    then
        echo -e "Failed to soft-link libOpenCL.so"
        exit -1
    fi
    ln -s ${AMDAPPSDKROOT}/lib/x86_64/sdk/libOpenCL.so.1 ${AMDAPPSDKROOT}/lib/x86_64/libOpenCL.so.1
    if [ $? -ne 0 ]
    then
        echo -e "Failed to soft-link libOpenCL.so.1"
        exit -1
    fi

    cp ${AMDAPPSDKROOT}/lib/x86_64/libamdocl12cl64.so ${AMDAPPSDKROOT}/lib/x86_64/sdk/
    if [ $? -ne 0 ]
    then
        echo -e "Failed to copy libamdocl12cl64.so."
        exit -1
    fi
fi

# When using this local installation,
# make sure to set up AMDAPPSDKROOT and OPENCL_VENDOR_PATH environment variables.
