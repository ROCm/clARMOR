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


This folder contains all of the files to run a series of functional tests on
clARMOR. In general, there are two kinds of tests:

 1. Tests that have buffer overflows which the tool should find.
 2. Tests that do not have buffer overflows, and the tool should not find any.

In general, these are called "bad" and "good" tests, respectively.

At this time, there are a few categories that we want to test. Each of these is
put into its own test in order to help break apart different types of problems.

 1. Basic cl_mem tests (cl_mem):
    These tests work on basic cl_mem buffers allocated from clCreateBuffer().
    These basic tests simply access a single buffer from within an OpenCL
    kernel. If there's an overflow, the first byte after the buffer will be
    overwritten, making it relatively easy to detect.
 2. Basic OpenCL 1.1 image tests (image_cl1_1):
    These tests will create cl_mem images using the clCreateImage2D() and
    clCreateImage3D() functions. Kernel accesses to the image use the
    CLK_ADDRESS_NONE addressing mode. If there's an overflow, it's because the
    kernel tried to write outside the boundary of the final row. This does
    not call the number OpenCL 1.2 functions, nor does it use complex
    sampler types.
 3. Basic OpenCL 1.2+ image tests (image_cl1_2):
    These tests will create cl_mem images using the clCreateImage() function
    that was introduced in OpenCL 1.2. Kernel accesses to the image use the
    CLK_ADDRESS_NONE addressing mode. If there's an overflow, it is be because
    the kernel tried to write outside the boundary of the final dimension.
 4. Basic shared virtual memory tests (svm):
    These tests work on basic coarse-grained SVM buffers allocated from
    clSVMAlloc(). These basic tests simply access a single SVM buffer from
    within an OpenCL kernel. If there's an overflow, the first byte after the
    buffer will be overwritten, making it relatively easy to detect.
 5. Indirect SVM tests (svm_indirect):
    SVM buffers have the added complexity that not all buffers accessible by
    the kernel need to be passed as a kernel parameter. An SVM buffer passed
    as a kernel parameter can contain pointers to *other* SVM buffers, and
    the kernel can buffer overflow those buffers.
    This test will create a number of coarse-grained SVM buffers and pass them
    to a kernel only by point to them from another SVM buffer.
 6. Fine-grained SVM (FG SVM) tests (fine_svm):
    Much like the basic SVM tests, these test whether fine-grained SVM buffers
    allocated from clSVMAlloc() can be used. These basic tests imply access a
    single FG SVM buffer from within an OpenCL kernel. If there's an overflow,
    the first byte after the buffer will be overwritten, making it relatively
    easy to detect. These programs do not test fine-grained system SVM (SVM
    allocated with malloc() calls), nor does it test SVM atomics.
 7. SubBuffer tests (sub_buffer):
    OpenCL SubBuffers can be used as arguments to kernels. A SubBuffer exists
    in the middle of some other full buffer. As such, if a kernel overflows
    the SubBuffer, it may only end up writing within the real "parent" buffer.
    This would not corrupt any canary bits normally, so we  must take special
    precautions to find errors in SubBuffers.
 8. Complex cl_mem tests (complex_cl_mem):
    Rather than simply having a single buffer that has an overflow at the first
    byte after the buffer, these tests work on more complicated situations.
    For instance, if a kernel has multiple buffers, do we detect overflows on
    more than one if need be? Do we detect overflows that happen mutliple bytes
    past the end of the buffer? If there is a buffer overflow in one kernel,
    can we correctly tell that there is not an overflow in that buffer in a
    later kernel?
 9. Complex OpenCL 1.1 image tests (complex_image_cl1_1):
    Rather than simply writing past the end of an image by a byte or two, this
    will test other mechanisms for mistakenly writing outside of an image.
    It will try to use other addressing modes (which should never allow
    buffer overflows) and will use different types of image formats (such
    as multi-value entries, like RGB). In addition, it will try to overflow
    beyond the end of some of the inner dimensions (e.g. writing past the
    last column of an inner row in a 2D image, rather than just writing beyond
    the final row).
 10.Complex OpenCL 1.2 image tests (complex_image_cl1_2):
    Similar to complex_image_cl1_1, except that this uses the newer
    clCreateImage() API calls in the host code.
 11.Complex SVM tests (complex_svm):
    Rather than simplying causing a single input buffer to overflow, or having
    a single indirect buffer, these tests check what happens when there are
    multiple SVM buffers and the overflow does not write to the first byte past
    the end of the buffer. In addition, these test the OpenCL 2.0 nested
    parallelism ability, where the buffer overflows are caused by a device-
    enqueued kernel. If there is a buffer overflow, it should be detected when
    the parent kernel finishes.
    These test both coarse-grained and fine-grained SVM types. It does not test
    fine-grained system SVM or SVM atomics.
 12.Overflows caused by OpenCL functions, but not OpenCL kernels (*_enqueue)
    For instance, calling clEnqueueCopyBuffer() with the wrong size can cause
    an overflow to happen. The *_enqueue tests will try to move too much data
    into a buffer using some kind of OpenCL API call. We should properly
    find and catch these errors because we keep track of appropriate meta-data
    about each buffer.


Tests that are not *currently* written due to lack of full support in the
buffer overflow detection software:

 1. Buffer overflows in the negative direction:
    These apply for all buffer types. Right now, the buffer overflow detector
    does not find overflows that write before the beginning of a buffer.
    This could be done by e.g. wrapping cl_mem creation calls and adding extra
    canary space both before and after. When passing the buffer to a kernel,
    pass it a SubBuffer that points to the "real" buffer start.



------------- How the tests are designed and how to write your own ------------
===============================================================================
The basic structure of the tests directory is that each subdirectory is a
self-contained test. When the main Makefile is called, it calls the
run_tests.sh shell script, which walks through all of the subdirectories and
passed the 'make' command to them.

Each of these subdirectories should have a Makefile that allows 'make',
'make test', 'make build_test', and 'make clean'. The 'make' and 'make test'
commands will build the test and run it using the buffer overflow detector in
the main directory.



Because many parts of these tests are fundamentally identical (e.g. "set up
your OpenCL structures, run a few kernels, check that the tool found the
correct number of overflows), the similar parts are modularized.

=============== Writing the Makefile
The sub-directory 'common_include' includes a common.mk file, which includes
almost all of the proper definitions for building a test program. As such, most
test programs will only need to do the following steps:
 1. Define "EXPECTED_ERRORS", the number of expected buffer overflows in this
    test.
 2. Define "BENCH_NAME", the name of the benchmark. This will be used to
    create the executable.
 3. include common_include/common.mk

=============== Writing the test
The sub-directory 'common_include' includes common_test_functions.{c/h}, which
can be helpful in creating your test application. See the .h file for comments.
Basically, this allows you to use a common set of options and set up your
OpenCL environment.

After that, you should write the application like normal.

One note is the "output_fake_errors()" function. This is primarily useful if
your application will not run sometimes, but you need the buffer overflow
detector to see an output (even if that output is "no buffer overflows").

For instance, if you have an application that will use OpenCL 2.0 features
(such as SVM), you should enclose those features in #ifdef checks, in case
someone is using this tool with an OpenCL runtime that does not support
OpenCL 2.0. In that case, your test and the buffer overflow detector will not
run. Normally, this would make the test fail (because the detector found
the wrong number of overflows). Instead, if you add the output_fake_errors()
call into the #else of the ifdef check, you can at least have the test
succeed.

See, for example, the good_svm and bad_svm tests for how this can be used.
