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

##############################################
# OpenCL Buffer Overflow Detector Make Defines
##############################################

HERE := $(dir $(lastword $(MAKEFILE_LIST)))

ifeq ($(CC),cc)
CC=gcc
endif

OCL_INCLUDE_DIR=/
OCL_LIB_DIR=/

ifdef AMDAPPSDKROOT
OCL_INCLUDE_DIR=$(AMDAPPSDKROOT)/include
OCL_LIB_DIR=$(AMDAPPSDKROOT)/lib/x86_64
else ifdef ATISTREAMSDKROOT
OCL_INCLUDE_DIR=$(ATISTREAMSDKROOT)/include
OCL_LIB_DIR=$(ATISTREAMSDKROOT)/lib/x86_64
else ifdef INTELOCLSDKROOT
OCL_INCLUDE_DIR=$(INTELOCLSDKROOT)/include
OCL_LIB_DIR=$(INTELOCLSDKROOT)/lib/x86_64
else ifdef CUDA_INC_PATH
OCL_INCLUDE_DIR=$(CUDA_INC_PATH)
OCL_LIB_DIR=$(CUDA_LIB_PATH)
else ifdef CUDA_PATH
OCL_INCLUDE_DIR=$(CUDA_PATH)/include
OCL_LIB_DIR=$(CUDA_PATH)/lib64
else
$(error OpenCL environment variables are not set, so this build has failed.$nPlease set one of the environment variables:$nAMDAPPSDKROOT, ATISTREAMSDKROOT, INTELOCLSDKROOT, CUDA_PATH, or CUDA_INC_PATH/CUDA_LIB_PATH)
endif


INCLUDE_DIR=${HERE}../src/include
CL_WRAPPER_DIR=${HERE}../src/cl_wrapper

SOURCE_DIR=${HERE}../src
UTILS_DIR=${SOURCE_DIR}/utils
TEST_DIR=${HERE}../tests
BIN_DIR=${HERE}../bin
LIB_DIR=${HERE}../lib
REPO_DIR=$(HERE)../

DETECT_SCRIPT=$(BIN_DIR)/run_overflow_detect.py

# Name of all the source files we need for the detector
CSOURCES=$(shell find ${SOURCE_DIR} -name "*.c" -type f)
COBJECTS=$(CSOURCES:.c=.o)
CDEPS=$(COBJECTS:.o=.d)
CPPSOURCES=$(shell find ${SOURCE_DIR} -name "*.cpp" -type f)
CPPOBJECTS=$(CPPSOURCES:.cpp=.o)
CPPDEPS=$(CPPOBJECTS:.o=.d)

# Need these around for make test, which uses stuff from the utils directory
UTILS_DIR_CSOURCES=$(shell find ${UTILS_DIR} -name "*.c" -type f)
UTILS_DIR_COBJECTS=$(UTILS_DIR_CSOURCES:.c=.o)
UTILS_DIR_CDEPS=$(UTILS_DIR_COBJECTS:.o=.d)
UTILS_DIR_CPPSOURCES=$(shell find ${UTILS_DIR} -name "*.cpp" -type f)
UTILS_DIR_CPPOBJECTS=$(UTILS_DIR_CPPSOURCES:.cpp=.o)
UTILS_DIR_CPPDEPS=$(UTILS_DIR_CPPOOBJECTS:.o=.d)

# Name of the final library we want and its version number
CL_WRAPPER=clbufferwrapper
CL_WRAPPER_SO_NAME=lib$(CL_WRAPPER)
MAJOR_VERSION=1
MINOR_VERSION=0
VERSION=$(MAJOR_VERSION).$(MINOR_VERSION)

LDLIBS=-Wl,--no-as-needed -ldl

INCLUDE_FLAGS=-I$(OCL_INCLUDE_DIR) -I$(INCLUDE_DIR)
# -isystem will prevent warnings (and errors) from the APP SDK header files
WERROR_FLAG=-Werror

ifeq ($(CC),gcc)
GCC_GTE_48 := $(shell expr `gcc -dumpversion | cut -f1,2 -d.` \>= 4.8)
ifeq "$(GCC_GTE_48)" "1"
PEDANTIC_FLAGS=-Wpedantic -pedantic-errors
endif
endif
WARN_FLAGS=-Wall -Wextra $(PEDANTIC_FLAGS) -isystem $(OCL_INCLUDE_DIR) -Wpacked -Wundef
CONLY_WARN=-Wold-style-definition

# use the following line for normal operation, or use the second line following this one for debugging/profiling
ifndef DEBUG
C_AND_CXX_FLAGS=-g3 -ggdb -pthread -fPIC -DBASE_FILE_NAME=\"$(<F)\" -DLINUX $(WERROR_FLAG) $(WARN_FLAGS) $(INCLUDE_FLAGS) -O3 -march=native -DNDEBUG
else
C_AND_CXX_FLAGS=-g3 -ggdb -pthread -fPIC -fno-omit-frame-pointer -DBASE_FILE_NAME=\"$(<F)\" -DLINUX $(WARN_FLAGS) $(INCLUDE_FLAGS) -O0 -DDEBUG
endif

CFLAGS=$(C_AND_CXX_FLAGS) -std=gnu11 $(CONLY_WARN)
CXXFLAGS=$(C_AND_CXX_FLAGS) -std=c++11 -DBASE_FILE_NAME=\"$(<F)\" -I$(HERE).. -fno-strict-aliasing -Wformat -Werror=format-security -fwrapv
LDFLAGS=-Wl,--as-needed -lstdc++ -L$(OCL_LIB_DIR) -lOpenCL
# Use either of the following two in LDFLAGS in order to build this tool with
# memory corruption protection. Use only one at at a time.
# It's likely that they will die inside libOpenCL.so, so you may need
# to comment out any real OpenCL calls in the OCL wrapper.
# -lmcheck -lefence

# Defalt install directory. This can be overwritten at the command line
prefix=/usr/local
user=$(shell whoami)
DESTDIR=

%.o: %.c
	$(CC) $(CFLAGS) -fPIC -c -MMD -o $@ $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -fPIC -c -MMD -o $@ $<

%.d: ;
.PRECIOUS: %.d
