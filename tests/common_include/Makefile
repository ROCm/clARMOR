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

include ../../make/master.mk

# Can't use CURDIR, that is based on the root directory.
THIS_DIR=$(shell pwd)

TEST_CSRC=$(shell find $(pwd) -name "*.c" -type f)
TEST_CPPSRC=$(shell find $(pwd) -name "*.cpp" -type f)
TEST_COBJECTS=$(TEST_CSRC:.c=.o)
TEST_CPPOBJECTS=$(TEST_CPPSRC:.cpp=.o)
TEST_CDEPS=$(TEST_COBJECTS:.o=.d)
TEST_CPPDEPS=$(TEST_CPPOOBJECTS:.o=.d)

.PHONY: all
all:

.PHONY: test
test:

.PHONY: cpu_test
cpu_test:

.PHONY: build_test
build_test:

.PHONY: clean
clean:
	$(RM) $(CURDIR)/*.o $(CURDIR)/*.d

-include $(TEST_CDEPS)
-include $(TEST_CPPDEPS)
-include $(UTILS_DIR_CDEPS)
-include $(UTILS_DIR_CPPDEPS)
