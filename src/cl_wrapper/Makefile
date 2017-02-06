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

.PHONY: all
all: $(LIB_DIR)/$(CL_WRAPPER_SO_NAME).so.$(VERSION)

.PHONY: clean
clean:
	$(RM) -f $(CL_WRAPPER_DIR)/*.o $(CL_WRAPPER_DIR)/*.d

$(LIB_DIR)/$(CL_WRAPPER_SO_NAME).so.$(VERSION): $(COBJECTS) $(CPPOBJECTS)
	mkdir -p $(LIB_DIR)
	$(CXX) -shared -g -Wl,-soname,$(CL_WRAPPER_SO_NAME).so.$(MAJOR_VERSION) -o $@ $^ $(LDFLAGS) -L$(LIB_DIR) -ldl -lOpenCL
	ln -f -s $(CL_WRAPPER_SO_NAME).so.$(VERSION) $(LIB_DIR)/$(CL_WRAPPER_SO_NAME).so
	ln -f -s $(CL_WRAPPER_SO_NAME).so.$(VERSION) $(LIB_DIR)/$(CL_WRAPPER_SO_NAME).so.$(MAJOR_VERSION)

-include $(CDEPS)
-include $(CPPDEPS)
