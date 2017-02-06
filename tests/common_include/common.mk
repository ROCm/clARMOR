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


# All of the test benchmarks use basically the same makefile.
# Just set "BENCH_NAME" and "EXPECTED_ERRORS" before including this to make a
# valid test makefile.

# BENCH_NAME is the name of the executable for this benchmark
# EXPECTED_ERRORS is the number of buffer overflows you expect the tool to find

include ../../make/master.mk

# Can't use CURDIR, that is based on the root directory.
THIS_DIR=$(shell pwd)

TEST_CSRC=$(shell find $(pwd) -name "*.c" -type f)
TEST_CPPSRC=$(shell find $(pwd) -name "*.cpp" -type f)

TEST_CSRC+=$(shell find $(THIS_DIR)/../common_include/ -name "*.c" -type f)
TEST_CPPSRC+=$(shell find $(THIS_DIR)/../common_include/ -name "*.cpp" -type f)

OUTPUT_FILE_NAME=$(THIS_DIR)/buffer_overflow_detector.out

CFLAGS+=-I$(THIS_DIR)/../common_include/ -DOUTPUT_FILE_NAME='"$(OUTPUT_FILE_NAME)"' -DEXPECTED_ERRORS=$(EXPECTED_ERRORS)

TEST_COBJECTS=$(TEST_CSRC:.c=.o)
TEST_CPPOBJECTS=$(TEST_CPPSRC:.cpp=.o)

.PHONY: all
all: test

.PHONY: clean
clean:
	$(RM) -f $(THIS_DIR)/*.exe $(THIS_DIR)/*.o $(THIS_DIR)/*.out $(THIS_DIR)/*.d $(THIS_DIR)/*.csv

.PHONY: test
test: run_test
	@CHECK_ERROR=`grep total ${OUTPUT_FILE_NAME} | awk {'print $$5'}`; \
	if ! [ -n "$${CHECK_ERROR:+1}" ]; then \
		echo "ERROR. Output log of buffer overflow not found at ${THIS_DIR}/buffer_overflow_detector.out";\
		echo "blah $$CHECK_ERROR";\
		exit 1;\
	fi;\
	if [ $$CHECK_ERROR -ne ${EXPECTED_ERRORS} ]; then \
		echo "ERROR. Found $$CHECK_ERROR buffer overflows instead of ${EXPECTED_ERRORS}";\
		exit 2;\
	fi

build_test: $(BENCH_NAME).exe

.PHONY: run_test
run_test: build_test
	$(DETECT_SCRIPT) -l -w $(THIS_DIR) -r "$(THIS_DIR)/$(BENCH_NAME).exe"

$(BENCH_NAME).exe: $(UTILS_DIR_COBJECTS) $(UTILS_DIR_CPPOBJECTS) $(TEST_COBJECTS) $(TEST_CPPOBJECTS)
	$(CC) $^ $(LDFLAGS) -lm -o $@
