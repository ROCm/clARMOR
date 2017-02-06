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


/*! \file bufferOverflowDetect.h
 * Methods to launch an overflow detection scheme.
 */

#ifndef __BUFFER_OVERFLOW_DETECT_H
#define __BUFFER_OVERFLOW_DETECT_H

#include <stdint.h>
// kernel_arg is defined in cl_kernel_lists.h
// In addition, other files that include detector calls need access to
// kernel and kernel argument information.
#include "meta_data_lists/cl_kernel_lists.h"
// Lots of the buffer overflow tools need access to the memory lists
#include "meta_data_lists/cl_memory_lists.h"



/*!
 * find duplicate arguments
 *
 * \param nargs
 *      number of arguments to the kernel
 * \param args
 *      list to information about kernel arguments
 * \param dupe_p
 *      Output
 *      pointer to duplicate list to be created
 *      uint32_t at index i is equal to i if arg i is unique in the set of indexes <= i,
 *      otherwise it is equal to the index of the first occurrence of the argument
 *
 */
void findDuplicates(uint32_t nargs, kernel_arg *args, uint32_t **dupe_p);

/*!
 * uses one of the overflow detection methods to verify the integrity of the canary regions
 * by default chooses between checking on cpu vs gpu, by number of active buffers
 *
 * \param cmdQueue
 *      cl_command_queue on which to perform the verification
 * \param kern
 *      cl_kernel to verify
 * \param dupe
 *      list of duplicate kernel arguments
 * \param evt
 *      leading event, synchronization point for start of check
 * \param retEvt
 *      end event, synchronization point for end of check
 */
void verifyBufferInBounds(cl_command_queue cmdQueue, cl_kernel kern, uint32_t *dupe, const cl_event *evt, cl_event *retEvt);

/*!
 * deletes the kernel and any arguments created by the overflow detector
 *
 * \param del_kern
 *      cl_kernel deletion target
 * \param dupe
 *      list of duplicate kernel arguments
 */
void delPoisonKernel(cl_kernel del_kern, uint32_t *dupe);

/*!
 * create a copy kernel
 * arguments uninitialized
 *
 * \param kernel
 *      kernel to copy
 * \return duplicate kernel
 */
cl_kernel kernelDuplicate(cl_kernel kernel);

/*!
 * If there are buffers that still need to be expanded with canaries,
 *  this function will create a new kernel with the updated arguments
 * If all buffers have canaries, returns the argument kernel
 *
 * \param command_queue
 *      create the new kernel on this queue
 * \param kernel
 *      use this kernel to create the new kernel
 * \param dupe
 *      list of duplicate kernel arguments
 * \return kernel with poisoned buffers
 */
cl_kernel createPoisonedKernel(cl_command_queue command_queue,
        cl_kernel kernel,
        uint32_t *dupe);

/*!
 * copies buffer arguments across kernels
 *
 * \param to
 *      copy buffers to this kernel
 * \param from
 *      copy buffers from this kernel
 * \param dupe
 *      list of duplicate kernel arguments
 * \param command_queue
 *      copy using this cl_command_queue
 */
void copyKernelBuffers(cl_kernel to, cl_kernel from,
        uint32_t * dupe, cl_command_queue command_queue);

#endif //__BUFFER_OVERFLOW_DETECT_H
