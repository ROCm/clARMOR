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


/*! \file wrapper_utils.h
 * utilities for the cl_wrapper
 */

#ifndef __WRAPPER_UTILS__
#define __WRAPPER_UTILS__

#include <stdint.h>
#include <CL/cl.h>

#include "detector_defines.h"

/*!
 * Allows access to canary region of a buffer without triggering an api overflow error
 *
 */
void allowCanaryAccess(void);

/*!
 * trigger api overflow error on canary region accesses
 *
 */
void disallowCanaryAccess(void);

/*!
 * Get if checking for canary region accesses
 *
 * \return
 *      0 not checking
 *      1 checking
 */
uint8_t canaryAccessAllowed(void);


/*!
 * Given an OpenCL context, find a command queue associated with it. We
 * handle this in our OpenCL wrapper becuase we cache command queues.
 * (For reasons why we cache the command queues, see the comment in the
 * wrapper for clCreateCommandQueue). If we do not have a cached command
 * queue for this context, this call will create one. As such, you should
 * always be able to get a legal command queue out of this call.
 * Returns 0 on success, non-zero on failure.
 *
 * \param context
 *      get command queue for this context
 * \param cmdQueue
 *      return command queue to this pointer
 * \return
 *      1 - found existing command queue
 *      0 - created new command queue
 */
int getCommandQueueForContext(cl_context context, cl_command_queue *cmdQueue);

/*!
 * Create a flat image copy with initialized canararies from provided flat image
 *
 * \param copy_ptr_p
 *      return flat image
 * \param host_ptr
 *      copy data from this array pointer
 * \param img
 *      cl_memobj with new image dimensions
 *
 */
void allocFlatImageCopy(char **copy_ptr_p, char *host_ptr, cl_memobj *img);

#ifdef CL_VERSION_2_0
/*!
 * adds profiling to command queue properties
 *
 * \properties
 *      update these cl_queue_properties
 *
 * \return new properties
 *
 */
cl_queue_properties *add_profiling(const cl_queue_properties *properties);

#endif //CL_VERSION_2_0

/*!
 * check enqueue buffer function for overflow
 *
 * \param func
 *      string function name
 * \param buffer
 *      cl_mem buffer
 * \param offset
 *      operation offset
 * \param size
 *      operation size
 *
 * \return
 *      0 no issue
 *      1 overflow detected
 */
uint8_t apiBufferOverflowCheck(char * const func, cl_mem buffer, size_t offset, size_t size);

/*!
 * check enqueue buffer rect function for overflow
 *
 * \param func
 *      string function name
 * \param buffer
 *      cl_mem buffer
 * \param buffer_offset
 *      operation offset
 * \param region
 *      operation region
 * \param row_pitch
 * \param slice_pitch
 *
 * \return
 *      0 no issue
 *      1 overflow detected
 */
uint8_t apiBufferRectOverflowCheck(char * const func, cl_mem buffer, const size_t * buffer_offset, const size_t * region, size_t row_pitch, size_t slice_pitch);

/*!
 * check enqueue image function for overflow
 *
 * \param func
 *      string function name
 * \param image
 *      cl_mem image
 * \param origin
 *      operation start point
 * \param region
 *      operation region dimensions
 *
 * \return
 *      0 no issue
 *      1 overflow detected
 */
uint8_t apiImageOverflowCheck(char * const func, cl_mem image, const size_t * origin, const size_t * region);

/*!
 * Take a description of a clImage and expand its dimensions to make room for
 * poison bits. Pass in the cl_image_desc, and this function will
 * automatically increase the width, height, depth, and array_size as needed.
 *
 * \param desc
 *      update this image description
 */
void expandLastDimForFill(cl_image_desc *desc);

#endif //__WRAPPER_UTILS__
