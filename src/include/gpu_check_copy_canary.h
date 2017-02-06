/********************************************************************************
 * Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ********************************************************************************/

#ifndef __GPU_CHECK_COPY_CANARY_H
#define __GPU_CHECK_COPY_CANARY_H

#include <stdint.h>
#include <CL/cl.h>

#include "meta_data_lists/cl_kernel_lists.h"

// Verify the canary regions on all of the buffers passed in through the arrays
// buffer_ptrs and image_ptrs.
// All of the canaries for each memory type are copied into a single buffer
// and checked with one kernel call for each type.
// After checking all of the canary regions, if any errors are found, the
// function will output information about the error and (optionally) kill the
// program.
// Inputs:
//      cmd_queue: the command queue used to enqueue the original kernel. This
//                 will also be used to enqueue the checker kernels.
//      num_cl_mem: the number of cl_mem buffers that in the buffer_ptrs list
//      num_svm: the number of SVM regions in the buffer_ptrs list
//      num_images: the number of cl_mem images that are in the image_ptrs list
//      buffer_ptrs: an array that contains all of the cl_mem buffers and SVM
//                   buffers to check, back to back. The first part of the list
//                   must be cl_mem buffers, and the second part must be SVM.
//                   This contains the actual cl_mem or void* that represent
//                   the buffer. Do not pass a pointer to the cl_mem.
//      image_ptrs: an array that contains all of the cl_mem images to check.
//                  This contains the actual cl_mem that represent the buffer.
//                  Do not pass a pointer ot the cl_mem.
//      copy_svm_ptrs: There are two ways to check SVM canaries. One is to
//                     copy all of the canaries into a large buffer. The other
//                     is to make a buffer full of pointers to all of the
//                     canary regions. If this value is non-zero, we will do
//                     the latter.
//      kern_info:  Information about the kernel that just ran before this
//                  check. Information in here, in particular, will be used
//                  when printing out information about buffer overflows.
//      dupe:   List of arguments in the kernel that are duplicates. This is
//              used when printing out information about buffers overflows.
//              The syntax of each entry in the array is "the first kernel arg
//              that is this memory buffer". So if dupe[i]==i, where i is arg
//              number, this is the first arg that points to that buffer.
//      evt:    The output event from the real kernel, so that we don't start
//              checking the buffers until the kernel completes.
//      ret_evt: event that is returned after that tells the rest of the
//               tool when any OpenCL work within this call is complete.
//               This call may enqueue checker kernels asynchronously,
//               as well as reading back the results, etc. If you want to free
//               anything used by this kernel, you should wait on this event.
void verify_on_gpu_copy_canary(cl_command_queue cmd_queue, uint32_t num_cl_mem,
        uint32_t num_svm, uint32_t num_images, void **buffer_ptrs,
        void **image_ptrs, int copy_svm_ptrs, kernel_info *kern_info,
        uint32_t *dupe, const cl_event *evt, cl_event *ret_evt);

#endif // __GPU_CHECK_COPY_CANARY_H
