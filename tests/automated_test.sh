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

# This script will run a series of automated stress tests across a user-defined
# group of benchmarks.
#
# When using the "-g" parameter to set a benchmark group:
# It will run these benchmarks through the buffer overflow detector using all
# of the default parameters. It does not verify that the correct number of
# overflows are found, but rather verifies that everything works and nothing
# crashes.
# It will then run them all again, but setting the parameter that forces all
# of the buffer overflow checks to run on the CPU.
# It then runs them a third time and forces all of the buffer overflow checks
# to run on the GPU.
#
# When the "-b" parameter is set, this script will:
# Run all of our automated build-time tests. In particular:
# cppcheck, pylint, scan-build, and 'make test'.
# Because these may use different compiler chains, we allow the "-c" flag to
# set either GCC or LLVM. Alternately, setting the CC and CXX command-line
# variables also allows you to control which build system is used.

print_usage()
{
    echo -e "Usage: $0 [-h] [-g GROUP_TO_BENCHMARK] [-b [-c {gcc/llvm}]]" 1>&2
    echo -e "   -h will print this help message and exit." 1>&2
    echo -e "   -g is the required name of the benchmark group to test." 1>&2
    echo -e "   -b will add in a series of build tests irrespective of the requested group." 1>&2
    echo -e "   -c When using the -b option, this flag lets you pick 'gcc' or 'llvm' as your build system." 1>&2
    echo -e "       The default is whatever the default CC and CXX are on your system." 1>&2
    echo -e "       If both this flag and CC/CXX are unset, we default to the apps 'cc' and 'c++'" 1>&2
    echo -e "You should use at least one of these options to test something."
}

print_help_and_exit()
{
    print_usage
    exit 0
}

print_error_and_exit()
{
    echo -e "ERROR on command line parameter."
    print_usage
    exit -1
}

print_success()
{
    FG='\033[0;30m' # Black
    BG='\033[42m'   # Green BG
    NC='\033[0m'    # Reset colors
    echo -e "       ${FG}${BG}SUCCESS${NC}"
}

print_failure()
{
    FG='\033[0;37m' # White
    BG='\033[41m'   # Red BG
    NC='\033[0m'    # Reset colors
    if [ -z "$1" ]
    then
        echo -e "       ${FG}${BG}FAILED${NC}"
    else
        if [ -z "$2" ]
        then
            echo -e "       ${FG}${BG}FAILED${NC} with error code $1"
        else
            echo -e "       ${FG}${BG}FAILED${NC} ${2} with error code $1"
        fi
    fi
}

build_info_check()
{
    make info_check &> /dev/null
    ret_val=$?
    if [ $ret_val -ne 0 ]; then
        print_failure $ret_val
        has_ever_failed=$ret_val
        echo -e "Failed 'make info_check'"
        echo -e "Stopping test since things won't successfully build."
        exit -1
    fi
}

has_ever_failed=0

build_tests=0
group_tests=0
unset group
compiler_to_use="nothing"

while getopts "hg:vbc:" option;
do
    case "${option}" in
        h)
            print_help_and_exit
            ;;
        b)
            build_tests=1
            ;;
        c)
            compiler_to_use=${OPTARG}
            ;;
        g)
            group_tests=1
            group=${OPTARG}
            ;;
        *)
            print_error_and_exit
            ;;
    esac
done

if [ $build_tests -eq 0 ] && [ $group_tests -eq 0 ]
then
    echo -e "\n\nERROR: Must pass -b and/or -g command-line option."
    echo -e "Right now, you are running *no* tests.\n\n"
    print_error_and_exit
fi

BASE_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

# This section runs the build tests if you pass -b
if [ $build_tests -ne 0 ]
then
    if [ "${compiler_to_use}" == "gcc" ] || [ "${compiler_to_use}" == "GCC" ]
    then
        CC=gcc
        CXX=g++
    elif [ "${compiler_to_use}" == "llvm" ] || [ "${compiler_to_use}" == "LLVM" ]
    then
        CC=clang
        CXX=clang++
    fi
    if [ -z ${CC+x} ]
    then
        echo -e "No C compiler set. Using the following default:"
        cc_compiler=`which cc`
        echo -e "   ${cc_compiler}"
        CC=cc
    fi
    if [ -z ${CXX+x} ]
    then
        echo -e "No C++ compiler set. Using the following default:"
        cxx_compiler=`which c++`
        echo -e "   ${cxx_compiler}"
        CXX=c++
    fi
    export CC
    export CXX

    print_date=`date`
    echo ""
    echo -e "Starting basic 'make' at $print_date ..."
    cd ${BASE_DIR}/../
    make &> ${BASE_DIR}/auto_test_make.out
    ret_val=$?
    print_date=`date`
    echo -e "Finished at $print_date:"
    if [ $ret_val -ne 0 ]
    then
        print_failure $ret_val
        has_ever_failed=$ret_val
        echo -e "   Check out ${BASE_DIR}/auto_test_make.out to find out what happened."
        echo -e "Stopping test since things won't successfully build."
        exit -1
    else
        print_success
    fi
    echo ""

    print_date=`date`
    echo ""
    echo -e "Starting cppcheck at $print_date ..."
    cd ${BASE_DIR}/../
    make check &> ${BASE_DIR}/auto_test_check.out
    ret_val=$?
    print_date=`date`
    echo -e "Finished at $print_date:"
    if [ $ret_val -ne 0 ]
    then
        print_failure $ret_val
        has_ever_failed=$ret_val
        echo -e "   Check out ${BASE_DIR}/auto_test_check.out to find out what happened."
    else
        print_success
    fi
    echo ""

    print_date=`date`
    echo ""
    echo -e "Starting pylint at $print_date ..."
    cd ${BASE_DIR}/../
    make pylint &> ${BASE_DIR}/auto_test_pylint.out
    ret_val=$?
    print_date=`date`
    echo -e "Finished at $print_date:"
    if [ $ret_val -ne 0 ]
    then
        print_failure $ret_val
        has_ever_failed=$ret_val
        echo -e "   Check out ${BASE_DIR}/auto_test_pylint.out to find out what happened."
    else
        print_success
    fi
    echo ""

    print_date=`date`
    echo ""
    echo -e "Starting default scan-build at $print_date ..."
    mv -f ${BASE_DIR}/*.out ${BASE_DIR}/../ &> /dev/null
    cd ${BASE_DIR}/../
    make clean &> /dev/null
    mv -f ${BASE_DIR}/../*.out ${BASE_DIR} &> /dev/null
    cd ${BASE_DIR}/../
    scan-build --status-bugs make -j `nproc` &> ${BASE_DIR}/auto_test_scan-build.out
    ret_val=$?
    print_date=`date`
    echo -e "Finished at $print_date:"
    if [ $ret_val -ne 0 ]
    then
        print_failure $ret_val
        has_ever_failed=$ret_val
        echo -e "   Check out ${BASE_DIR}/auto_test_scan-build.out to find out what happened."
    else
        print_success
    fi
    echo ""

    print_date=`date`
    echo ""
    echo -e "Starting advanced scan-build at $print_date ..."
    mv -f ${BASE_DIR}/*.out ${BASE_DIR}/../ &> /dev/null
    cd ${BASE_DIR}/../
    make clean &> /dev/null
    mv -f ${BASE_DIR}/../*.out ${BASE_DIR} &> /dev/null
    SECURITY_CHECKS="security.insecureAPI.strcpy security.insecureAPI.rand alpha.security.ReturnPtrRange alpha.security.ArrayBoundV2"
    MEMORY_CHECKS="alpha.unix.cstring.OutOfBounds alpha.unix.cstring.BufferOverlap alpha.unix.cstring.NotNullTerminated alpha.cplusplus.NewDeleteLeaks alpha.core.FixedAddr"
    GENERAL_CHECKS="alpha.unix.SimpleStream alpha.core.CastToStruct alpha.core.CastSize alpha.core.BoolAssignment alpha.core.TestAfterDivZero alpha.core.SizeofPtr alpha.core.IdenticalExpr alpha.deadcode.UnreachableCode alpha.cplusplus.VirtualCall"
    UNUSED_CHECKS=""
    TOTAL_CHECKS="$SECURITY_CHECKS $MEMORY_CHECKS $GENERAL_CHECKS"
    ENABLE_THESE_CHECKS=""
    for check_to_use in $TOTAL_CHECKS
    do
        ENABLE_THESE_CHECKS="$ENABLE_THESE_CHECKS -enable-checker $check_to_use"
    done
    SCAN_OPTIONS="--use-cc=${CC} --use-c++=${CXX} -maxloop 512 --status-bugs ${ENABLE_THESE_CHECKS}"
    cd ${BASE_DIR}/../
    scan-build ${SCAN_OPTIONS} make -j `nproc` &> ${BASE_DIR}/auto_test_scan-build_advanced.out
    ret_val=$?
    print_date=`date`
    echo -e "Finished at $print_date:"
    if [ $ret_val -ne 0 ]
    then
        print_failure $ret_val
        has_ever_failed=$ret_val
        echo -e "   Check out ${BASE_DIR}/auto_test_scan-build_advanced.out to find out what happened."
    else
        print_success
    fi
    echo ""

    build_info_check
    CAN_TEST=$(${BASE_DIR}/../bin/clarmor-info -g)
    if [ $((CAN_TEST)) -eq 1 ]
    then
        print_date=`date`
        echo ""
        echo -e "Starting 'make test' at $print_date ..."
        cd ${BASE_DIR}/../
        mv -f ${BASE_DIR}/*.out ${BASE_DIR}/../ &> /dev/null
        make clean &> /dev/null
        mv -f ${BASE_DIR}/../*.out ${BASE_DIR} &> /dev/null
        make -j `nproc` &> ${BASE_DIR}/auto_test_make_test.out
        make test >> ${BASE_DIR}/auto_test_make_test.out 2>&1
        ret_val=$?
        print_date=`date`
        echo -e "Finished at $print_date:"
        if [ $ret_val -ne 0 ]
        then
            print_failure $ret_val
            has_ever_failed=$ret_val
            echo -e "   Check out ${BASE_DIR}/auto_test_make_test.out to find out what happened."
        else
            print_success
        fi
        echo ""
    fi

    build_info_check
    CAN_TEST=$(${BASE_DIR}/../bin/clarmor-info -c)
    if [ $((CAN_TEST)) -eq 1 ]
    then
        print_date=`date`
        echo ""
        echo -e "Starting 'make cpu_test' at $print_date ..."
        cd ${BASE_DIR}/../
        mv -f ${BASE_DIR}/*.out ${BASE_DIR}/../ &> /dev/null
        make clean &> /dev/null
        mv -f ${BASE_DIR}/../*.out ${BASE_DIR} &> /dev/null
        make -j `nproc` &> ${BASE_DIR}/auto_test_make_cpu_test.out
        make cpu_test >> ${BASE_DIR}/auto_test_make_cpu_test.out 2>&1
        ret_val=$?
        print_date=`date`
        echo -e "Finished at $print_date:"
        if [ $ret_val -ne 0 ]
        then
            print_failure $ret_val
            has_ever_failed=$ret_val
            echo -e "   Check out ${BASE_DIR}/auto_test_make_cpu_test.out to find out what happened."
        else
            print_success
        fi
        echo ""
    fi

    build_info_check
    CAN_TEST=$(${BASE_DIR}/../bin/clarmor-info -g)
    if [ $((CAN_TEST)) -eq 1 ]
    then
        print_date=`date`
        echo ""
        echo -e "Starting 'make test' with CLARMOR_DEVICE_SELECT=1 at $print_date ..."
        cd ${BASE_DIR}/../
        mv -f ${BASE_DIR}/*.out ${BASE_DIR}/../ &> /dev/null
        make clean &> /dev/null
        mv -f ${BASE_DIR}/../*.out ${BASE_DIR} &> /dev/null
        make -j `nproc` &> ${BASE_DIR}/auto_test_make_test_device-1.out
        CLARMOR_DEVICE_SELECT=1 make test >> ${BASE_DIR}/auto_test_make_test_device-1.out 2>&1
        ret_val=$?
        print_date=`date`
        echo -e "Finished at $print_date:"
        if [ $ret_val -ne 0 ]
        then
            print_failure $ret_val
            has_ever_failed=$ret_val
            echo -e "   Check out ${BASE_DIR}/auto_test_make_test_device-1.out to find out what happened."
        else
            print_success
        fi
        echo ""
    fi

    build_info_check
    CAN_TEST=$(${BASE_DIR}/../bin/clarmor-info -g)
    if [ $((CAN_TEST)) -eq 1 ]
    then
        print_date=`date`
        echo ""
        echo -e "Starting 'make test' with CLARMOR_DEVICE_SELECT=1 CLARMOR_ALTERNATE_GPU_DETECTION=1 at $print_date ..."
        cd ${BASE_DIR}/../
        mv -f ${BASE_DIR}/*.out ${BASE_DIR}/../ &> /dev/null
        make clean &> /dev/null
        mv -f ${BASE_DIR}/../*.out ${BASE_DIR} &> /dev/null
        make -j `nproc` &> ${BASE_DIR}/auto_test_make_test_device-1_gpudetect-1.out
        CLARMOR_DEVICE_SELECT=1 CLARMOR_ALTERNATE_GPU_DETECTION=1 make test >> ${BASE_DIR}/auto_test_make_test_device-1_gpudetect-1.out 2>&1
        ret_val=$?
        print_date=`date`
        echo -e "Finished at $print_date:"
        if [ $ret_val -ne 0 ]
        then
            print_failure $ret_val
            has_ever_failed=$ret_val
            echo -e "   Check out ${BASE_DIR}/auto_test_make_test_device-1_gpudetect-1.out to find out what happened."
        else
            print_success
        fi
        echo ""
    fi

    build_info_check
    CAN_TEST=$(${BASE_DIR}/../bin/clarmor-info -g)
    if [ $((CAN_TEST)) -eq 1 ]
    then
        print_date=`date`
        echo ""
        echo -e "Starting 'make test' with CLARMOR_DEVICE_SELECT=1 CLARMOR_ALTERNATE_GPU_DETECTION=2 at $print_date ..."
        cd ${BASE_DIR}/../
        mv -f ${BASE_DIR}/*.out ${BASE_DIR}/../ &> /dev/null
        make clean &> /dev/null
        mv -f ${BASE_DIR}/../*.out ${BASE_DIR} &> /dev/null
        make -j `nproc` &> ${BASE_DIR}/auto_test_make_test_device-1_gpudetect-2.out
        CLARMOR_DEVICE_SELECT=1 CLARMOR_ALTERNATE_GPU_DETECTION=2 make test >> ${BASE_DIR}/auto_test_make_test_device-1_gpudetect-2.out 2>&1
        ret_val=$?
        print_date=`date`
        echo -e "Finished at $print_date:"
        if [ $ret_val -ne 0 ]
        then
            print_failure $ret_val
            has_ever_failed=$ret_val
            echo -e "   Check out ${BASE_DIR}/auto_test_make_test_device-1_gpudetect-2.out to find out what happened."
        else
            print_success
        fi
        echo ""
    fi

    build_info_check
    CAN_TEST=$(${BASE_DIR}/../bin/clarmor-info -g)
    if [ $((CAN_TEST)) -eq 1 ]
    then
        print_date=`date`
        echo ""
        echo -e "Starting 'make test' with CLARMOR_DEVICE_SELECT=2 at $print_date ..."
        cd ${BASE_DIR}/../
        mv -f ${BASE_DIR}/*.out ${BASE_DIR}/../ &> /dev/null
        make clean &> /dev/null
        mv -f ${BASE_DIR}/../*.out ${BASE_DIR} &> /dev/null
        make -j `nproc` &> ${BASE_DIR}/auto_test_make_test_device-2.out
        CLARMOR_DEVICE_SELECT=2 make test >> ${BASE_DIR}/auto_test_make_test_device-2.out 2>&1
        ret_val=$?
        print_date=`date`
        echo -e "Finished at $print_date:"
        if [ $ret_val -ne 0 ]
        then
            print_failure $ret_val
            has_ever_failed=$ret_val
            echo -e "   Check out ${BASE_DIR}/auto_test_make_test_device-2.out to find out what happened."
        else
            print_success
        fi
        echo ""
    fi
fi

# This section runs the detector against the desired group if the group
# variable has been set.
if [ $group_tests -eq 1 ]
then
    print_date=`date`
    echo ""
    echo -e "Starting benchmark tests at $print_date ..."
    ${BASE_DIR}/../bin/clarmor --group $group &> ${BASE_DIR}/auto_test_benchmarks.out
    ret_val=$?
    print_date=`date`
    echo -e "Finished at $print_date:"
    if [ $ret_val -ne 0 ]
    then
        print_failure $ret_val
        has_ever_failed=$ret_val
        echo -e "   Check out ${BASE_DIR}/auto_test_benchmarks.out to find out what happened."
    else
        print_success
    fi
    echo ""
fi

exit $has_ever_failed
