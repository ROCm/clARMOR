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
#include <stdio.h>

#include "cl_err.h"
#include "cl_utils.h"
#include "meta_data_lists/cl_memory_lists.h"
#include "universal_event.h"
#include "wrapper_utils.h"
#include "cl_copy_utils.h"
#include "util_functions.h"

#include "cl_image_copy.h"

// Copy a source image to a destination image using the supplied command queue
static void straight_image_copy(cl_command_queue command_queue,
        cl_mem src_image, cl_mem dst_image, const size_t src_origin[3],
        const size_t dst_origin[3], const size_t region[3], unsigned num_evt,
        const cl_event *evt_list, cl_event *out)
{
    cl_int cl_err;
    cl_err = clEnqueueCopyImage(command_queue, src_image, dst_image, src_origin,
            dst_origin, region, num_evt, evt_list, out);
    check_cl_error(__FILE__, __LINE__, cl_err);
}

// copy between cl_mem buffers using cl_context
// all events in the event list (evtList) must be from the context specified
// return event (out) will be from the context specified
static void new_queue_image_copy(cl_context context, cl_mem src_image,
        cl_mem dst_image, const size_t src_origin[3],
        const size_t dst_origin[3], const size_t region[3], unsigned num_evt,
        const cl_event *evt_list, cl_event *out)
{
    cl_command_queue copy_queue;
    cl_event copy_event;

    int we_created = !getCommandQueueForContext(context, &copy_queue);

    straight_image_copy(copy_queue, src_image, dst_image, src_origin,
            dst_origin, region, num_evt, evt_list, &copy_event);
    if(out != NULL)
        *out = copy_event;

    if(we_created)
    {
        cl_int cl_err = clReleaseCommandQueue(copy_queue);
        check_cl_error(__FILE__, __LINE__, cl_err);
    }
}

static void copy_image_between_contexts(cl_command_queue command_queue,
        cl_mem src_image, cl_mem dst_image, const size_t src_origin[3],
        const size_t dst_origin[3], const size_t region[3],
        cl_context sample_ctx, cl_memobj *from_info, cl_memobj *to_info,
        unsigned num_evt, const cl_event *evt_list, cl_event *out)
{
    cl_int cl_err;
    void *temp_host_ptr = malloc(region[0]*region[1]*region[2]);
    cl_mem_flags host_flag = (from_info->flags | CL_MEM_USE_HOST_PTR) &
        ~CL_MEM_ALLOC_HOST_PTR & ~CL_MEM_COPY_HOST_PTR;

#ifdef CL_VERSION_1_2
    cl_image_desc desc;
    desc.image_type = CL_MEM_OBJECT_IMAGE3D;
    desc.image_width = region[0];
    desc.image_height = region[1];
    desc.image_depth = region[2];
    desc.image_array_size = 1;
    desc.image_row_pitch = 0;
    desc.image_slice_pitch = 0;
    desc.num_mip_levels = 0;
    desc.num_samples = 0;
    desc.buffer = NULL;

    cl_mem temp_buff1 = clCreateImage(from_info->context, host_flag,
            &from_info->image_format, &desc, temp_host_ptr, &cl_err);
#else
    cl_mem temp_buff1 = clCreateImage3D(from_info->context, host_flag,
            &from_info->image_format, region[0], region[1], region[2],
            0, 0, temp_host_ptr, &cl_err);
#endif
    check_cl_error(__FILE__, __LINE__, cl_err);
    size_t tmp_origin[] = {0, 0, 0};

    cl_event jump1, jump2;
    if(sample_ctx == from_info->context)
    {
        straight_image_copy(command_queue, src_image, temp_buff1, src_origin,
                tmp_origin, region, num_evt, evt_list, &jump1);
    }
    else
    {
        cl_err = clWaitForEvents(num_evt, evt_list);
        check_cl_error(__FILE__, __LINE__, cl_err);
        new_queue_image_copy(from_info->context, src_image, temp_buff1,
                src_origin, tmp_origin, region, 0, 0, &jump1);
    }

#ifdef CL_VERSION_1_2
    cl_mem temp_buff2 = clCreateImage(to_info->context, host_flag,
            &from_info->image_format, &desc, temp_host_ptr, &cl_err);
#else
    cl_mem temp_buff2 = clCreateImage3D(to_info->context, host_flag,
            &from_info->image_format, region[0], region[1], region[2],
            0, 0, temp_host_ptr, &cl_err);
#endif
    check_cl_error(__FILE__, __LINE__, cl_err);

    cl_err = clWaitForEvents(1, &jump1);
    check_cl_error(__FILE__, __LINE__, cl_err);
    if(sample_ctx == to_info->context)
    {
        straight_image_copy(command_queue, temp_buff2, dst_image, tmp_origin,
                dst_origin, region, 0, 0, &jump2);
        if(out != NULL)
            *out = jump2;
    }
    else
    {
        new_queue_image_copy(to_info->context, temp_buff2, dst_image,
                tmp_origin, dst_origin, region, 0, 0, &jump2);
        cl_err = clWaitForEvents(1, &jump2);
        check_cl_error(__FILE__, __LINE__, cl_err);
        if(out != NULL)
            *out = create_complete_user_event(sample_ctx);
    }

    copy_asset* data = (copy_asset*)malloc(sizeof(copy_asset));
    data->host_ptr = temp_host_ptr;
    cl_err = clSetEventCallback(jump2, CL_COMPLETE, cleanup_copy, data);
    check_cl_error(__FILE__, __LINE__, cl_err);

    cl_err = clReleaseMemObject(temp_buff1);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clReleaseMemObject(temp_buff2);
    check_cl_error(__FILE__, __LINE__, cl_err);
}

void inner_image_copy(cl_command_queue command_queue, cl_mem src_image,
        cl_mem dst_image, const size_t src_origin[3],
        const size_t dst_origin[3], const size_t region[3], unsigned num_evt,
        const cl_event *evt_list, cl_event *out)
{
    cl_context sample_ctx;
    if (out != NULL)
        *out = NULL;

    cl_memobj *to_info = cl_mem_find(get_cl_mem_alloc(), dst_image);
    cl_memobj *from_info = cl_mem_find(get_cl_mem_alloc(), src_image);
    if (to_info == NULL || from_info == NULL)
    {
        det_fprintf(stderr, "Failed to find memory regions %p %p at %s:%d\n",
                (void*)to_info, (void*)from_info, __FILE__, __LINE__);
        exit(-1);
    }
    cl_int cl_err = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT,
            sizeof(cl_context), &sample_ctx, 0);
    check_cl_error(__FILE__, __LINE__, cl_err);

    if(sample_ctx == to_info->context && to_info->context == from_info->context)
    {
        // easy case, everything is in the same context.
        straight_image_copy(command_queue, src_image, dst_image, src_origin,
                dst_origin, region, num_evt, evt_list, out);
    }
    else if(to_info->context == from_info->context)
    {
        // to and from are in the same context, but the command queue we sent
        // in is not. The solution here is to make a new command queue and do
        // the copy.
        cl_err = clWaitForEvents(num_evt, evt_list);
        check_cl_error(__FILE__, __LINE__, cl_err);
        cl_event copy_event;
        new_queue_image_copy(to_info->context, src_image, dst_image,
                src_origin, dst_origin, region, 0, 0, &copy_event);

        convertEvents(sample_ctx, 1, &copy_event);
        if(out != NULL)
            *out = copy_event;
    }
    else
    {
        // to and from are in different contexts, so we must put a temporary
        // host-side buffer in between. Copy into the host buffer, then copy
        // out of that host buffer. This lets us move between contexts
        copy_image_between_contexts(command_queue, src_image, dst_image,
                src_origin, dst_origin, region, sample_ctx, from_info, to_info,
                num_evt, evt_list, out);
    }
}
