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


/*! \file gpu_check_kernels.h
 * functions in this file to fetch source strings for gpu check kernels
 */

#ifndef __GPU_CHECK_KERNELS_H
#define __GPU_CHECK_KERNELS_H

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * Returns the OpenCL source code for kernels that check the canary values
 * from a large number of buffers. This assumes that, previous to calling these
 * kernels, someone has created the larger "canary copy" buffer that holds the
 * canaries for multiple buffers.
 */
const char * get_buffer_copy_canary_src(void);

/*!
 * Similar to the above kernel, this returns the OpenCL source code for kernels
 * that check a series of canary values that are copied into their own "all
 * canary" buffer. The kernels here are for canaries that were originally
 * copied from images, since their layout can be much more complicated.
 */
const char * get_image_copy_canary_src(void);

/*!
 * Returns the OpenCL source code for kernels that check the canary values
 * from a single buffer. This allows the kernel to check and mend the canaries
 * in place, reducing copy costs. It has added overhead from requiring more
 * kernels when there are >1 buffer to check, however.
 */
const char * get_single_buffer_src(void);

/*!
 * Returns the OpenCL source code for kernels that check copies of canary
 * values from regular cl_mem buffers. However, if there are canaries in SVM
 * regions, the canaries are left in the same place and only the *pointers*
 * to those canary regions are passed into the big "all canary" buffer.
 * As such, SVM canaries are checked in place but cl_mem canaries are
 * copied into one contiguous buffer. All of this is done int he same kernel.
 */
const char * get_buffer_and_ptr_copy_src(void);

#ifdef __cplusplus
}
#endif

#endif // __GPU_CHECK_KERNELS_H
