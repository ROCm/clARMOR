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

include make/master.mk

# Default install directory. This can be overwritten at the command line
prefix=/usr/local

.PHONY: all
all: info_check
	$(MAKE) --directory=$(CL_WRAPPER_DIR)

.PHONY: info_check
info_check:
	$(MAKE) --directory=$(INFO_CHECK_DIR)

.PHONY: test
test: all
	$(MAKE) --directory=$(TEST_DIR)

.PHONY: cpu_test
cpu_test: all
	$(MAKE) --directory=$(TEST_DIR) cpu_test

.PHONY: build_test
build_test:
	$(MAKE) --directory=$(TEST_DIR) build_test

.PHONY: clean
clean:
	$(RM) -f *.o *.d *.so* *.a *~ *.pyc
	$(RM) -f $(COBJECTS)
	$(RM) -f $(CDEPS)
	$(RM) -f $(CPPOBJECTS)
	$(RM) -f $(CPPDEPS)
	$(RM) -f $(UTILS_DIR)/*.exe $(UTILS_DIR)/*.o $(UTILS_DIR)/*~ 
	$(RM) -f $(BIN_DIR)/*.exe $(BIN_DIR)/*.o $(BIN_DIR)/*~  $(BIN_DIR)/*.pyc $(BIN_DIR)/*.txt
	$(RM) -f $(LIB_DIR)/*
	$(RM) -f $(BIN_DIR)/*.out
	$(RM) -f $(SOURCE_DIR)/*.pyc
	$(MAKE) --directory=$(CL_WRAPPER_DIR) clean
	$(MAKE) --directory=$(TEST_DIR) clean
	$(MAKE) --directory=$(INFO_CHECK_DIR) clean

check:
	cppcheck --force --enable=warning,style,performance,portability,information,missingInclude --error-exitcode=-1 --std=c11 --std=c++11 --suppress=*:*cl_ext.h $(SOURCE_DIR) $(TEST_DIR) $(INCLUDE_FLAGS) -q -j `nproc`

pylint:
	cd $(BIN_DIR); $(foreach file, $(shell ls $(BIN_DIR)/*.py |xargs -n 1 basename), pylint -E $(file);) \
	cd $(REPO_DIR)
