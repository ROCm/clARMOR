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

# This script will get ACML v6.1 from the web and put it into the directory of
# your choice.
BASE_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

print_help_header()
{
    echo -e "This script will download a tarball of v6.1.0.31-gfortran64 of" 1>&2
    echo -e "the AMD Core Math Library (ACML)." 1>&2
    echo -e "By default, it will download this tarball into the current directory." 1>&2
    echo -e ""
}

print_usage()
{
    echo -e "Usage: $0 [-h] [-d TARGET_DIRECTORY]" 1>&2
    echo -e "   -h will print this help message and exit." 1>&2
    echo -e "   -d allows you to set the target directory that" 1>&2
    echo -e "          acml-6.1.0.31-gfortran64.gz will be put into" 1>&2
    echo -e "          By default, this is the directory this script is in." 1>&2
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

target_dir=${BASE_DIR}
temp_dir=/tmp/fake_temp_dir/

while getopts "hd:" option;
do
    case "${option}" in
        h)
            print_help_and_exit
            ;;
        d)
            target_dir=${OPTARG}/
            ;;
        *)
            print_error_and_exit
            ;;
    esac
done

mkdir -p ${target_dir}
if [ $? -ne 0 ]
then
    echo -e "Cannot create download directory ${target_dir}"
    exit -1
fi

# Get the the Boost.org Compute files for getting the AMD APP SDK
# We will patch this to work with ACML.
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
# This worked around commit 94ad91d
git checkout HEAD ./.travis/amd_sdk.sh
patch ./.travis/amd_sdk.sh ${BASE_DIR}/acml_helper.patch
cp ./.travis/amd_sdk.sh ${target_dir}/acml.sh
if [ $? -ne 0 ]
then
    echo -e "Failed to find amd_sdk.sh from Boost.org GitHub"
    rm -rf ${temp_dir}
    exit -1
fi

# Move back to the main directory and delete the temp dir
cd ${target_dir}
rm -rf ${temp_dir}
chmod +x ${target_dir}/acml.sh

# We now have the script to download ACML.
# Download the tarball of the correct version
${target_dir}/acml.sh ${amdapp_version}
if [ $? -ne 0 ]
then
    echo -e "Failed to properly download ACML"
    rm -f ${target_dir}/acml.sh
    exit -1
fi
rm -f ${target_dir}/acml.sh
