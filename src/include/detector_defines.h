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

/*! \file detector_defines.h
 * Values for canary lengths and fills.
 * Also has defines for GPU kernel synchronizations.
 */

#ifndef __DETECTOR_DEFINES_H
#define __DETECTOR_DEFINES_H

#include <stdint.h>
#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#include "CL/cl.h"

//GPU check mutators
//#define SEQUENTIAL
#define KERN_CALLBACK

//measured in bytes
#define POISON_FILL_LENGTH 8192

//measured in array indexes
#define IMAGE_POISON_WIDTH 16
#define IMAGE_POISON_HEIGHT 16
#define IMAGE_POISON_DEPTH 16

#define POISON_FILL 0xC2

extern const uint8_t poisonFill_8b; ///< 8 bit poison fill
extern const uint32_t poisonFill_32b; ///< 32 bit poison fill
extern const unsigned poisonWordLen; ///< length of canary region in 32 bit words

#ifndef CL_VERSION_1_2
typedef struct _cl_image_desc {
    cl_mem_object_type      image_type;
    size_t                  image_width;
    size_t                  image_height;
    size_t                  image_depth;
    size_t                  image_array_size;
    size_t                  image_row_pitch;
    size_t                  image_slice_pitch;
    cl_uint                 num_mip_levels;
    cl_uint                 num_samples;
#ifdef __GNUC__
    __extension__   /* Prevents warnings about anonymous union in -pedantic builds */
#endif
    union {
      cl_mem                  buffer;
      cl_mem                  mem_object;
    };
} cl_image_desc;

/* cl_mem_object_types missing in OpenCL 1.1 */
#define CL_MEM_OBJECT_IMAGE2D_ARRAY 0xE000
#define CL_MEM_OBJECT_IMAGE1D 0xE001
#define CL_MEM_OBJECT_IMAGE1D_ARRAY 0xE002
#define CL_MEM_OBJECT_IMAGE1D_BUFFER 0xE003

/* cl_image_info missing in OpenCL 1.1 */
#define CL_IMAGE_ARRAY_SIZE 0xE104

#define CL_BLOCKING CL_TRUE
#define CL_NON_BLOCKING CL_FALSE

#endif

#endif //__DETECTOR_DEFINES_H
