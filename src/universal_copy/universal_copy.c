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

#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#include <CL/cl.h>

#include "universal_copy.h"
#include "cl_buffer_copy.h"
#include "cl_image_copy.h"
#include "cl_image_to_buffer.h"

void cl_buffer_copy(cl_command_queue command_queue, cl_mem from, cl_mem to,
        size_t off_from, size_t off_to, size_t size, unsigned num_evt,
        const cl_event *evt_list, cl_event *out)
{
    inner_buffer_copy(command_queue, from, to, off_from, off_to, size, num_evt,
            evt_list, out);
}

void cl_image_copy(cl_command_queue command_queue, cl_mem src_image,
        cl_mem dst_image, const size_t src_origin[3],
        const size_t dst_origin[3], const size_t region[3], unsigned num_evt,
        const cl_event *evt_list, cl_event *out)
{
    inner_image_copy(command_queue, src_image, dst_image, src_origin,
            dst_origin, region, num_evt, evt_list, out);
}

void cl_image_to_buffer_copy(cl_command_queue command_queue, cl_mem src_image,
        cl_mem dst_buffer, const size_t src_origin[3], const size_t region[3],
        size_t dst_offset, unsigned num_evt, const cl_event *evt_list,
        cl_event *out)
{
    inner_image_to_buffer_copy(command_queue, src_image, dst_buffer,
            src_origin, region, dst_offset, num_evt, evt_list, out);
}
