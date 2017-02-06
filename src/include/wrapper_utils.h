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

#ifndef __WRAPPER_UTILS__
#define __WRAPPER_UTILS__

#include <stdint.h>
#include <CL/cl.h>

#include "detector_defines.h"

void allowCanaryAccess(void);

void disallowCanaryAccess(void);

uint8_t canaryAccessAllowed(void);

int getCommandQueueForContext(cl_context context, cl_command_queue *cmdQueue);

void allocFlatImageCopy(char **copy_ptr_p, char *host_ptr, cl_memobj *img);

#ifdef CL_VERSION_2_0
#ifdef DEBUG_CHECKER_TIME
cl_queue_properties *add_profiling(const cl_queue_properties *properties);
#endif // DEBUG_CHECKER_TIME

#endif //CL_VERSION_2_0


uint8_t apiBufferOverflowCheck(char * const func, cl_mem buffer, size_t offset, size_t size);

uint8_t apiBufferRectOverflowCheck(char * const func, cl_mem buffer, const size_t * buffer_offset, const size_t * region, size_t row_pitch, size_t slice_pitch);

uint8_t apiImageOverflowCheck(char * const func, cl_mem image, const size_t * origin, const size_t * region);

// Take a description of a clImage and expand its dimensions to make room for
// poison bits. Pass in the cl_image_desc, and this function will
// automatically increase the width, height, depth, and array_size as needed.
void expandLastDimForFill(cl_image_desc *desc);

#endif //__WRAPPER_UTILS__
