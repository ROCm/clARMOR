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
#include <limits.h>

#include "detector_defines.h"
#include "cl_err.h"
#include "cl_utils.h"
#include "util_functions.h"
#include "../gpu_check_utils.h"
#include "single_buffer_cl_buffer.h"


static void perform_cl_buffer_checks(cl_command_queue cmd_queue,
        cl_kernel check_kern, cl_event init_evt, cl_event real_kern_evt,
        cl_mem * result, uint32_t num_buffers, void **buffer_ptrs,
        int is_svm, cl_event *check_events)
{
    // Set up kernel invocation constants
    cl_int cl_err;
    size_t global_work[3] = {poisonWordLen, 1, 1};
    size_t local_work[3] = {256, 1, 1};
    size_t max_work_items[3] = {1, 1, 1};
    cl_event kern_wait[2] = {init_evt, real_kern_evt};

    cl_context kern_ctx;
    cl_device_id dev_id;
    clGetKernelInfo(check_kern, CL_KERNEL_CONTEXT, sizeof(cl_context), &kern_ctx, NULL);
    clGetContextInfo(kern_ctx, CL_CONTEXT_DEVICES, sizeof(cl_device_id), &dev_id, NULL);
    clGetKernelWorkGroupInfo(check_kern, dev_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), max_work_items, NULL);

    local_work[0] = max_work_items[0];

    // Set up checker kernel launch API arguments
    launchOclKernelStruct ocl_args = setup_ocl_args(cmd_queue, check_kern,
            1, NULL, global_work, local_work, 2, kern_wait, NULL);

    // Set up constant cl_mem checker kernel arguments
    cl_set_arg_and_check(check_kern, 0, sizeof(unsigned), &poisonWordLen);
    cl_set_arg_and_check(check_kern, 2, sizeof(unsigned), &poisonFill_32b);
    cl_set_arg_and_check(check_kern, 5, sizeof(void*), result);

    // We check each buffer independently
    for(uint32_t i = 0; i < num_buffers; i++)
    {
        void *mem_handle = NULL;
        uint32_t mem_size = 0;
        if (is_svm)
        {
#ifdef CL_VERSION_2_0
            cl_svm_memobj *m1 = cl_svm_mem_find(get_cl_svm_mem_alloc(),
                    buffer_ptrs[i]);
            if(m1 == NULL)
            {
                det_fprintf(stderr, "failure to find cl_svm_memobj.\n");
                exit(-1);
            }
            mem_handle = m1->main_buff;
            mem_size = m1->size;
#endif
        }
        else
        {
            cl_memobj *m1 = cl_mem_find(get_cl_mem_alloc(), buffer_ptrs[i]);
            if(m1 == NULL)
            {
                det_fprintf(stderr, "failure to find cl_memobj.\n");
                exit(-1);
            }
            mem_handle = (void*)(m1->main_buff);
            mem_size = m1->size;
        }

        for(uint32_t n = 0; n < POISON_REGIONS; n++)
        {
            uint32_t offset;
            uint32_t index = POISON_REGIONS*i + n;
#ifdef UNDERFLOW_CHECK
            if(n == 0)
                offset = 0;
            else
            {
                offset = mem_size + POISON_FILL_LENGTH;
            }
#else
            offset = mem_size;
#endif

            cl_set_arg_and_check(check_kern, 1, sizeof(unsigned), &index);
            cl_set_arg_and_check(check_kern, 3, sizeof(unsigned), &offset);
            if (is_svm)
            {
#ifdef CL_VERSION_2_0
                cl_set_svm_arg_and_check(check_kern, 4, mem_handle);
#endif
            }
            else
            {
                cl_set_arg_and_check(check_kern, 4, sizeof(void*),
                        &(mem_handle));
            }

            // Each of these checks is enqueued asynchronously, and we will
            // eventually wait on all these events before reading the results.
            ocl_args.event = &(check_events[index]);

            cl_err = runNDRangeKernel(&ocl_args);
            check_cl_error(__FILE__, __LINE__, cl_err);

#ifdef SEQUENTIAL
            cl_err = clFinish(cmd_queue);
            check_cl_error(__FILE__, __LINE__, cl_err);
#endif

            if(global_tool_stats_flags & STATS_CHECKER_TIME)
            {
                clFinish(cmd_queue);
                uint64_t times[4];
                populateKernelTimes(&(check_events[index]), &times[0], &times[1],
                        &times[2], &times[3]);
                add_to_kern_runtime((times[3] - times[2]) / 1000);
            }
        }
    }
}

typedef struct clbk_cmpct_data_
{
    int *first_change;
    uint32_t num_buff;
    cl_event complete;
}clbk_cmpct_data;

static void format_result_buff(cl_event event, cl_int status, void* verif_data)
{
    cl_int cl_err;
    if(event || status){}
    clbk_cmpct_data *data = (clbk_cmpct_data*)verif_data;
    int *first_change = data->first_change;
    uint32_t num_buff = data->num_buff;

    for(uint32_t i=0; i < num_buff; i++)
    {
        int half1, half2;
        half1 = first_change[POISON_REGIONS*i];
        half2 = first_change[POISON_REGIONS*i + 1];

        if(half2 < INT_MAX)
            half2 += POISON_FILL_LENGTH;

        first_change[i] = (half1 < half2) ? half1 : half2;
    }

    cl_err = clSetUserEventStatus(data->complete, CL_COMPLETE);
    check_cl_error(__FILE__, __LINE__, cl_err);

    free(data);
}

void verify_cl_buffer_single(cl_context kern_ctx, cl_command_queue cmd_queue,
        uint32_t num_buff, void **buffer_ptrs, kernel_info *kern_info,
        uint32_t *dupe, const cl_event *evt, int is_svm, cl_event *ret_evt)
{
    if (num_buff <= 0)
    {
        if (ret_evt != NULL)
            *ret_evt = create_complete_user_event(kern_ctx);
        return;
    }

    cl_int cl_err;
    cl_event init_evt;

    cl_kernel check_kern = get_canary_check_kernel(kern_ctx);
    cl_mem result = create_result_buffer(kern_ctx, cmd_queue, POISON_REGIONS*num_buff,
            &init_evt);
    cl_event *check_events = calloc(sizeof(cl_event), POISON_REGIONS*num_buff);

    // This will walk through all of the cl_mem buffers and launch a GPU kernel
    // to check whether their canaries have been corrupted.
    perform_cl_buffer_checks(cmd_queue, check_kern, init_evt, *evt, &result,
            num_buff, buffer_ptrs, is_svm, check_events);

    // Read back the results from all of the checks into 'first_change'.
    cl_event readback_evt;
    int *first_change = get_change_buffer(cmd_queue, POISON_REGIONS*num_buff, result,
            POISON_REGIONS*num_buff, check_events, &readback_evt);
    if(ret_evt != NULL)
        *ret_evt = readback_evt;

    cl_event user_evt;
#ifdef UNDERFLOW_CHECK
    //consolidate change list by buffer
    // index by canary -> index by buffer
    user_evt = clCreateUserEvent(kern_ctx, &cl_err);
    check_cl_error(__FILE__, __LINE__, cl_err);

    clbk_cmpct_data *verif_data = malloc(sizeof(clbk_cmpct_data));
    verif_data->first_change = first_change;
    verif_data->num_buff = num_buff;
    verif_data->complete = user_evt;

    if(!is_nvidia_platform(kern_ctx))
    {
        cl_err = clSetEventCallback(readback_evt, CL_COMPLETE,
                format_result_buff, (void*)verif_data);
        check_cl_error(__FILE__, __LINE__, cl_err);
    }
    else
    {
        cl_err = clWaitForEvents(1, &readback_evt);
        check_cl_error(__FILE__, __LINE__, cl_err);
        format_result_buff(0, 0, verif_data);
        user_evt = readback_evt;
    }

#else
    (void)format_result_buff;
    user_evt = readback_evt;
#endif

    //nvidia implementation may segfault
    //when feeding a user event to clSetEventCallback
    // Finally, check the results of the memory checks above.
    analyze_check_results(cmd_queue, user_evt, kern_info, num_buff,
            buffer_ptrs, NULL, 0, NULL, first_change, dupe);

    // Release the memory objects and events that are used by the read.
    // Because they're queued, this is OK. The release won't destroy them until
    // they are no longer needed.
    cl_err = clReleaseMemObject(result);
    check_cl_error(__FILE__, __LINE__, cl_err);
    for (uint32_t i = 0; i < num_buff; i++)
    {
        cl_err = clReleaseEvent(check_events[i]);
        check_cl_error(__FILE__, __LINE__, cl_err);
    }
    free(check_events);
}
