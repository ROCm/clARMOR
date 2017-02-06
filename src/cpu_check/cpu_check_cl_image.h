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

#ifndef __CPU_CHECK_CL_IMAGE_H
#define __CPU_CHECK_CL_IMAGE_H

#include <stdint.h>
#include <CL/cl.h>

#include "meta_data_lists/cl_kernel_lists.h"

// Find any overflows in cl_mem image objects.
// Inputs:
//      kern_info: information about the kernel that had the last opportunity
//                  to write to this buffer.
//      num_images: number of image cl_mem buffers in the array image_ptrs
//      image_ptrs: array of void* that each point to a cl_mem image
//      dupe:   array that describes which, if any, arguments to the kernel
//              are duplicates of one another. See cpu_check_utils.h for a
//              full description.
//      evt:    The cl_event that tells us when the real kernel has completed,
//              so that we can start checking its canaries.
void verify_images(kernel_info *kern_info, uint32_t num_images,
        void **image_ptrs, uint32_t *dupe, const cl_event *evt);

#endif // __CPU_CHECK_CL_IMAGE_H
