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
#include "util_functions.h"
#include "universal_copy.h"
#include "cl_err.h"
#include "cl_utils.h"
#include "../gpu_check_utils.h"

#include "copy_canary_cl_buffer.h"

static cl_mem create_clmem_copies(cl_context kern_ctx,
        cl_command_queue cmd_queue, uint32_t num_cl_mem, void **buffer_ptrs,
        const cl_event *evt, cl_event *events, cl_event *mend_events)
{
    cl_int cl_err;
    cl_mem clmem_canary_copies = clCreateBuffer(kern_ctx, 0,
            POISON_FILL_LENGTH*(num_cl_mem), 0, &cl_err);
    check_cl_error(__FILE__, __LINE__, cl_err);
    for(uint32_t i = 0; i < num_cl_mem; i++)
    {
        cl_memobj *m1 = cl_mem_find(get_cl_mem_alloc(), buffer_ptrs[i]);
        if(m1 == NULL)
        {
            det_fprintf(stderr, "failure to find cl_memobj at %s:%d.\n", __FILE__,
                    __LINE__);
            exit(-1);
        }

        cl_buffer_copy(cmd_queue, m1->handle, clmem_canary_copies,
                m1->size - POISON_FILL_LENGTH, i*POISON_FILL_LENGTH,
                POISON_FILL_LENGTH, 1, evt, &events[i]);

        mend_this_canary(kern_ctx, cmd_queue, m1->handle, events[i],
                &mend_events[i]);
    }
    return clmem_canary_copies;
}

static void *create_svm_copies(cl_context kern_ctx, cl_command_queue cmd_queue,
        uint32_t num_svm, void **buffer_ptrs, const cl_event *evt,
        cl_event *events, cl_event *mend_events)
{
    void *svm_canary_copies = NULL;
#ifdef CL_VERSION_2_0
    cl_svm_memobj *m1;
    svm_canary_copies = clSVMAlloc(kern_ctx, CL_MEM_READ_WRITE,
            POISON_FILL_LENGTH*num_svm, 0);
    if (svm_canary_copies == NULL)
    {
        det_fprintf(stderr, "Failed to SVMAlloc at %s:%d\n", __FILE__,
                __LINE__);
        exit(-1);
    }
    m1 = cl_svm_mem_find(get_cl_svm_mem_alloc(), svm_canary_copies);
    if (m1 == NULL)
    {
        det_fprintf(stderr, "failure to find cl_svm_memobj at %s:%d.\n", __FILE__,
                __LINE__);
        exit(-1);
    }
    m1->detector_internal_buffer = 1;

    for(uint32_t i = 0; i < num_svm; i++)
    {
        m1 = cl_svm_mem_find(get_cl_svm_mem_alloc(), buffer_ptrs[i]);
        if(m1 == NULL)
        {
            det_fprintf(stderr, "failure to find cl_svm_memobj at %s:%d.\n", __FILE__,
                    __LINE__);
            exit(-1);
        }

        char *base_ptr = (char*)m1->handle;
        char *canary_ptr = base_ptr + (m1->size - POISON_FILL_LENGTH);
        char *map_ptr = ((char*)svm_canary_copies)+(i*POISON_FILL_LENGTH);

        cl_int cl_err = clEnqueueSVMMemcpy(cmd_queue, CL_NON_BLOCKING, map_ptr,
                canary_ptr, POISON_FILL_LENGTH, 1, evt, &events[i]);
        check_cl_error(__FILE__, __LINE__, cl_err);

        mend_this_canary(kern_ctx, cmd_queue, m1->handle, events[i],
                &mend_events[i]);
    }
#else
    (void)kern_ctx;
    (void)cmd_queue;
    (void)num_svm;
    (void)buffer_ptrs;
    (void)evt;
    (void)events;
    (void)mend_events;
#endif
    return svm_canary_copies;
}

static void ** create_svm_ptr_copies(cl_context kern_ctx,
        cl_command_queue cmd_queue, uint32_t num_svm, void **buffer_ptrs,
        void **ret_clmem, const cl_event *evt, cl_event *events)
{
    void **ret_poison_ptrs = NULL;
    if (num_svm <= 0)
    {
        if (ret_clmem != NULL)
            *ret_clmem = NULL;
        return NULL;
    }

#ifdef CL_VERSION_2_0
    cl_int cl_err;
    if (ret_clmem == NULL)
        return NULL;

    ret_poison_ptrs = calloc(sizeof(void*), num_svm);
    cl_mem temp_ptr = clCreateBuffer(kern_ctx, 0, sizeof(void*)*num_svm, 0,
            &cl_err);
    *ret_clmem = (void*)temp_ptr;
    check_cl_error(__FILE__, __LINE__, cl_err);

    for(uint32_t i = 0; i < num_svm; i++)
    {
        cl_svm_memobj *m1 = cl_svm_mem_find(get_cl_svm_mem_alloc(),
                buffer_ptrs[i]);
        if(m1 == NULL)
        {
            det_fprintf(stderr, "failure to find cl_svm_memobj.\n");
            exit(-1);
        }

        char *ptr_base = (char *)m1->handle;
        ret_poison_ptrs[i] = ptr_base + (m1->size - POISON_FILL_LENGTH);
    }
    cl_err = clEnqueueWriteBuffer(cmd_queue, *ret_clmem, CL_NON_BLOCKING,
            0, sizeof(void*) * num_svm, ret_poison_ptrs, 1, evt,
            &events[0]);
    check_cl_error(__FILE__, __LINE__, cl_err);
    // We only need one event from this one.
    for (uint32_t i = 1; i < num_svm; i++)
        events[i] = create_complete_user_event(kern_ctx);
#else
    (void)cmd_queue;
    (void)buffer_ptrs;
    (void)ret_clmem;
    (void)evt;
    for (uint32_t i = 0; i < num_svm; i++)
        events[i] = create_complete_user_event(kern_ctx);
#endif
    return ret_poison_ptrs;
}

static cl_event perform_cl_buffer_checks(cl_context kern_ctx,
        cl_command_queue cmd_queue, uint32_t num_cl_mem, uint32_t num_svm,
        uint32_t total_buffs, cl_mem clmem_canary_copies,
        void *svm_canary_copies, int copy_svm_ptrs, cl_event *init_evts,
        cl_event *mend_events, cl_mem result)
{
    size_t global_work[3] = {poisonWordLen, 1, 1};
    size_t local_work[3] = {256, 1, 1};
    global_work[0] *= total_buffs;

    uint32_t buff_end = num_cl_mem * poisonWordLen;
    uint32_t svm_end = buff_end + num_svm * poisonWordLen;

    cl_kernel check_kern;
#ifdef CL_VERSION_2_0
    // No way to be in this "if" if CL_VERSION != 2.0
    if (num_svm > 0)
    {
        check_kern = get_canary_check_kernel(kern_ctx);
        if (copy_svm_ptrs)
        {
            cl_set_arg_and_check(check_kern, 5, sizeof(cl_mem),
                    &svm_canary_copies);
        }
        else
            cl_set_svm_arg_and_check(check_kern, 5, svm_canary_copies);
        cl_set_arg_and_check(check_kern, 6, sizeof(void*), (void*) &result);
    }
    else
#else
    (void)svm_canary_copies;
    (void)copy_svm_ptrs;
#endif
    {
        check_kern = get_canary_check_kernel_no_svm(kern_ctx);
        cl_set_arg_and_check(check_kern, 5, sizeof(void*), (void*) &result);
    }
    cl_set_arg_and_check(check_kern, 0, sizeof(unsigned), &poisonWordLen);
    cl_set_arg_and_check(check_kern, 1, sizeof(unsigned), &buff_end);
    cl_set_arg_and_check(check_kern, 2, sizeof(unsigned), &svm_end);
    cl_set_arg_and_check(check_kern, 3, sizeof(unsigned), &poisonFill_32b);
    cl_set_arg_and_check(check_kern, 4, sizeof(cl_mem), &clmem_canary_copies);

    cl_event kern_end;
    launchOclKernelStruct ocl_args = setup_ocl_args(cmd_queue, check_kern,
            1, NULL, global_work, local_work, total_buffs + 1, init_evts,
            &kern_end);
    cl_int cl_err = runNDRangeKernel( &ocl_args );
    check_cl_error(__FILE__, __LINE__, cl_err);

    if(global_tool_stats_flags & STATS_CHECKER_TIME)
    {
        clFinish(cmd_queue);
        uint64_t times[4];
        populateKernelTimes(&kern_end, &times[0], &times[1], &times[2],
                &times[3]);
        add_to_kern_runtime((times[3] - times[2]) / 1000);
    }

    cl_event finish;
    if(!get_error_envvar())
    {
#ifdef CL_VERSION_1_2
        cl_event mend_finish;
        cl_err = clEnqueueMarkerWithWaitList(cmd_queue, total_buffs,
                mend_events, &mend_finish);
        check_cl_error(__FILE__, __LINE__, cl_err);
        cl_event evt_list[] = {mend_finish, kern_end};
        cl_err = clEnqueueMarkerWithWaitList(cmd_queue, 2, evt_list, &finish);
#else
        (void)mend_events;
        cl_err = clEnqueueMarker(cmd_queue, &finish);
#endif
        check_cl_error(__FILE__, __LINE__, cl_err);
    }
    else
        finish = kern_end;
    return finish;
}

/*
 * launch checker kernel for cl_mem and svm buffers
 */
void verify_cl_buffer_copy(cl_context kern_ctx, cl_command_queue cmd_queue,
        uint32_t num_cl_mem, uint32_t num_svm, void **buffer_ptrs,
        int copy_svm_ptrs, kernel_info *kern_info, uint32_t *dupe,
        const cl_event *evt, cl_event *ret_evt)
{
    cl_int cl_err;
    uint32_t total_buffs = num_cl_mem + num_svm;

    if(total_buffs <= 0)
    {
        if (ret_evt != NULL)
            *ret_evt = create_complete_user_event(kern_ctx);
        return;
    }

    cl_mem clmem_canary_copies = NULL;
    void *svm_canary_copies = NULL;
    void **poison_pointers = NULL;

    cl_event *events = calloc(sizeof(cl_event), (total_buffs+1));
    cl_event *mend_events = calloc(sizeof(cl_event), (total_buffs));

    if (num_cl_mem > 0)
    {
        clmem_canary_copies = create_clmem_copies(kern_ctx, cmd_queue,
                num_cl_mem, buffer_ptrs, evt, events, mend_events);
    }
    else
    {
        clmem_canary_copies = clCreateBuffer(kern_ctx, 0, 1, 0, &cl_err);
        check_cl_error(__FILE__, __LINE__, cl_err);
    }
    if (num_svm > 0 && copy_svm_ptrs)
    {
        poison_pointers = create_svm_ptr_copies(kern_ctx, cmd_queue,
                num_svm, &(buffer_ptrs[num_cl_mem]), &svm_canary_copies,
                evt, &(events[num_cl_mem]));
        for (uint32_t i = num_cl_mem; i < total_buffs; i++)
            mend_events[i] = create_complete_user_event(kern_ctx);
    }
    else if (num_svm > 0)
    {
        svm_canary_copies = create_svm_copies(kern_ctx, cmd_queue, num_svm,
                &(buffer_ptrs[num_cl_mem]), evt, &(events[num_cl_mem]),
                &(mend_events[num_cl_mem]));
    }

    cl_mem result = create_result_buffer(kern_ctx, cmd_queue, total_buffs,
            &events[total_buffs]);

    cl_event kern_end = perform_cl_buffer_checks(kern_ctx, cmd_queue,
            num_cl_mem, num_svm, total_buffs, clmem_canary_copies,
            svm_canary_copies, copy_svm_ptrs, events, mend_events, result);

    for (uint32_t i = 0; i < total_buffs; i++)
    {
        clReleaseEvent(events[i]);
        clReleaseEvent(mend_events[i]);
    }
    clReleaseEvent(events[total_buffs]);
    free(events);
    free(mend_events);

    cl_event read_result;
    int * first_change = get_change_buffer(cmd_queue, total_buffs, result, 1,
            &kern_end, &read_result);

    if(ret_evt)
        *ret_evt = read_result;

    analyze_check_results(cmd_queue, read_result, kern_info, total_buffs,
            buffer_ptrs, svm_canary_copies, copy_svm_ptrs, poison_pointers,
            first_change, dupe);

    clReleaseMemObject(result);
    clReleaseMemObject(clmem_canary_copies);
}
