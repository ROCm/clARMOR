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
#include "cl_copy_utils.h"

#include "cl_buffer_copy.h"

// copy between cl_mem buffers using the supplied cl_command_queue
void straight_buffer_copy(cl_command_queue command_queue, cl_mem from,
        cl_mem to, size_t off_from, size_t off_to,size_t size, unsigned num_evt,
        const cl_event *evt_list, cl_event *out)
{
    cl_int cl_err;
    cl_err = clEnqueueCopyBuffer(command_queue, from, to, off_from, off_to,
            size, num_evt, evt_list, out);
    check_cl_error(__FILE__, __LINE__, cl_err);
}

// copy between cl_mem buffers using cl_context
// all events in the event list (evtList) must be from the context specified
// return event (out) will be from the context specified
void new_queue_buffer_copy(cl_context context, cl_mem from, cl_mem to,
        size_t off_from, size_t off_to, size_t size, unsigned num_evt,
        const cl_event *evt_list, cl_event *out)
{
    cl_command_queue copy_queue;
    cl_event copy_event;

    int we_created = !getCommandQueueForContext(context, &copy_queue);

    straight_buffer_copy(copy_queue, from, to, off_from, off_to, size, num_evt,
            evt_list, &copy_event);
    if(out != NULL)
        *out = copy_event;

    //if we created the queue, clean it up
    if (we_created)
    {
        cl_int cl_err = clReleaseCommandQueue(copy_queue);
        check_cl_error(__FILE__, __LINE__, cl_err);
    }
}

static void copy_between_contexts(cl_command_queue command_queue, cl_mem from,
        cl_mem to, size_t off_from, size_t off_to, size_t size,
        cl_context sample_ctx, cl_memobj *from_info, cl_memobj *to_info,
        unsigned num_evt, const cl_event *evt_list, cl_event *out)
{
    cl_int cl_err;

    void *temp_host_ptr = malloc(size);
    cl_mem_flags host_flag = (from_info->flags | CL_MEM_USE_HOST_PTR) &
        ~CL_MEM_ALLOC_HOST_PTR & ~CL_MEM_COPY_HOST_PTR;

    cl_mem temp_buff1 = clCreateBuffer(from_info->context, host_flag, size,
            temp_host_ptr, &cl_err);
    check_cl_error(__FILE__, __LINE__, cl_err);

    cl_event jump1, jump2;
    if(sample_ctx == from_info->context)
    {
        straight_buffer_copy(command_queue, from, temp_buff1, off_from, 0,
                size, num_evt, evt_list, &jump1);
    }
    else
    {
        cl_err = clWaitForEvents(num_evt, evt_list);
        check_cl_error(__FILE__, __LINE__, cl_err);
        new_queue_buffer_copy(from_info->context, from, temp_buff1, off_from, 0,
                size, 0, 0, &jump1);
    }

    cl_mem temp_buff2 = clCreateBuffer(to_info->context, host_flag, size,
            temp_host_ptr, &cl_err);
    check_cl_error(__FILE__, __LINE__, cl_err);

    cl_err = clWaitForEvents(1, &jump1);
    check_cl_error(__FILE__, __LINE__, cl_err);
    if(sample_ctx == to_info->context)
    {
        straight_buffer_copy(command_queue, temp_buff2, to, 0, off_to, size,
                0, 0, &jump2);
        if(out != NULL)
            *out = jump2;
    }
    else
    {
        new_queue_buffer_copy(to_info->context, temp_buff2, to, 0, off_to, size,
                0, 0, &jump2);
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

void inner_buffer_copy(cl_command_queue command_queue, cl_mem from, cl_mem to,
        size_t off_from, size_t off_to, size_t size, unsigned num_evt,
        const cl_event *evt_list, cl_event *out)
{
    cl_context sample_ctx;
    if (out != NULL)
        *out = NULL;

    cl_memobj *to_info = cl_mem_find(get_cl_mem_alloc(), to);
    cl_memobj *from_info = cl_mem_find(get_cl_mem_alloc(), from);
    if (to_info == NULL || from_info == NULL)
    {
        fprintf(stderr, "Failed to find memory regions %p %p at %s:%d\n",
                (void*)to_info, (void*)from_info, __FILE__, __LINE__);
        exit(-1);
    }
    cl_int cl_err = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT,
            sizeof(cl_context), &sample_ctx, 0);
    check_cl_error(__FILE__, __LINE__, cl_err);

    if(sample_ctx == to_info->context && to_info->context == from_info->context)
    {
        // easy case, everything is in the same context.
        straight_buffer_copy(command_queue, from, to, off_from, off_to, size,
                num_evt, evt_list, out);
    }
    else if(to_info->context == from_info->context)
    {
        // to and from are in the same context, but the command queue we sent
        // in is not. The solution here is to make a new command queue and do
        // the copy.
        cl_err = clWaitForEvents(num_evt, evt_list);
        check_cl_error(__FILE__, __LINE__, cl_err);
        cl_event copy_event;
        new_queue_buffer_copy(to_info->context, from, to, off_from, off_to, size,
                0, 0, &copy_event);

        convertEvents(sample_ctx, 1, &copy_event);
        if(out != NULL)
            *out = copy_event;
    }
    else
    {
        // to and from are in different contexts, so we must put a temporary
        // host-side buffer in between. Copy into the host buffer, then copy
        // out of that host buffer. This lets us move between contexts
        copy_between_contexts(command_queue, from, to, off_from, off_to, size,
                sample_ctx, from_info, to_info, num_evt, evt_list, out);
    }
}
