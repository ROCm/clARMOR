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

#include <stdio.h>

#include "detector_defines.h"
#include "cl_err.h"
#include "cpu_check_utils.h"
#include "wrapper_utils.h"
#include "util_functions.h"

#include "cpu_check_cl_mem.h"

static void read_cl_mem_canaries(cl_command_queue cmd_queue,
        uint32_t num_cl_mem, void * canaries, void **buffer_ptrs,
        void **buffer_cl_mem, const cl_event *input_event,
        cl_event *read_events)
{
    for (uint32_t i = 0; i < num_cl_mem; i++)
    {
        // casting to char* to do pointer arithmetic.
        char * c_canaries = (char *)canaries;
        void * this_canary = c_canaries + (i * POISON_FILL_LENGTH);

        cl_memobj *m1;
        m1 = cl_mem_find(get_cl_mem_alloc(), buffer_ptrs[i]);
        if(m1 == NULL)
        {
            det_fprintf(stderr, "failure to find cl_memobj %p.\n", buffer_ptrs[i]);
            exit(-1);
        }
        // found a cl_mem
        buffer_cl_mem[i] = m1->handle;

        cl_int cl_err = clEnqueueReadBuffer(cmd_queue, m1->handle,
                CL_NON_BLOCKING, m1->size - POISON_FILL_LENGTH,
                POISON_FILL_LENGTH, this_canary, 1, input_event,
                &(read_events[i]));
        check_cl_error(__FILE__, __LINE__, cl_err);
    }
}

void verify_cl_mem(kernel_info *kern_info, uint32_t num_cl_mem,
        void **buffer_ptrs, uint32_t *dupe, const cl_event *evt)
{
    if (num_cl_mem <= 0)
        return;

    cl_context kern_ctx;
    cl_int cl_err = clGetKernelInfo(kern_info->handle, CL_KERNEL_CONTEXT,
            sizeof(cl_context), &kern_ctx, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);

    cl_command_queue cmd_queue;
    getCommandQueueForContext(kern_ctx, &cmd_queue);

    void * canaries = malloc(POISON_FILL_LENGTH * num_cl_mem);
    cl_event * read_events = malloc(num_cl_mem * sizeof(cl_event));
    void ** buffer_cl_mem = malloc(num_cl_mem * sizeof(void*));

    read_cl_mem_canaries(cmd_queue, num_cl_mem, canaries, buffer_ptrs,
        buffer_cl_mem, evt, read_events);

    cl_err = clWaitForEvents(num_cl_mem, read_events);
    check_cl_error(__FILE__, __LINE__, cl_err);

    // Now check the canaries for each cl_mem region
    for (uint32_t i = 0; i < num_cl_mem; i++)
    {
        // casting to char* to do pointer arithmetic.
        char * c_canaries = (char *)canaries;
        void * this_canary = c_canaries + (i * POISON_FILL_LENGTH);

        //parse through the canary data
        cpu_parse_canary(cmd_queue, POISON_FILL_LENGTH, this_canary, kern_info,
                buffer_cl_mem[i], dupe);
    }

    free(buffer_cl_mem);
    free(read_events);
    free(canaries);
}
