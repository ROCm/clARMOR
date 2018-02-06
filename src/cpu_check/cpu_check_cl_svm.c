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
#include "cl_utils.h"
#include "cpu_check_utils.h"
#include "meta_data_lists/cl_memory_lists.h"
#include "wrapper_utils.h"
#include "util_functions.h"

#include "cpu_check_cl_svm.h"

#ifdef CL_VERSION_2_0
static void set_up_svm_canary_copy(cl_context kern_ctx, uint32_t num_svm,
        void ** svm_ptrs, void **base_ptrs, void **map_ptrs,
        void **canary_ptrs, uint8_t *right_context)
{
    // Cast to char** so we can do pointer arithmetic
    char **c_base_ptrs = (char **)base_ptrs;
    for (uint32_t i = 0; i < num_svm; i++)
    {
        cl_svm_memobj *m1;
        m1 = cl_svm_mem_find(get_cl_svm_mem_alloc(), svm_ptrs[i]);
        if(m1 == NULL)
        {
            det_fprintf(stderr, "failure to find cl_svm_memobj %p.\n", svm_ptrs[i]);
            exit(-1);
        }
        if (m1->context == kern_ctx)
            right_context[i] = 1;
        else
            continue;

        //index to beginning of canary region
        c_base_ptrs[i] = (char*)m1->main_buff;
        uint32_t index = i*POISON_REGIONS;
        uint32_t offset = m1->size;
#ifdef UNDERFLOW_CHECK
        canary_ptrs[index] = c_base_ptrs[i];

        index++;
        offset += POISON_FILL_LENGTH;
#endif
        canary_ptrs[index] = c_base_ptrs[i] + offset;

        map_ptrs[i] = clSVMAlloc(kern_ctx, CL_MEM_READ_WRITE,
                POISON_REGIONS*POISON_FILL_LENGTH, 0);
    }
}

static void copy_and_map_svm_canaries(cl_context kern_ctx,
        cl_command_queue cmd_queue, uint32_t num_svm, void **canary_ptrs,
        void **map_ptrs, cl_event * copy_events, cl_event * map_events,
        const cl_event *incoming_evt, uint8_t * right_context)
{
    cl_int cl_err;
    for (uint32_t i = 0; i < num_svm; i++)
    {
        if (right_context[i] == 0)
        {
            // Create the copy_event so we can cleanly release it later
            copy_events[i] = clCreateUserEvent(kern_ctx, &cl_err);
            check_cl_error(__FILE__, __LINE__, cl_err);
            map_events[i] = create_complete_user_event(kern_ctx);
            continue;
        }
        //copy canary region for this svm to smaller svm
        cl_err = clEnqueueSVMMemcpy(cmd_queue, CL_NON_BLOCKING, map_ptrs[i],
                canary_ptrs[POISON_REGIONS*i], POISON_FILL_LENGTH, 1, incoming_evt,
                &copy_events[POISON_REGIONS*i]);
        check_cl_error(__FILE__, __LINE__, cl_err);

#ifdef UNDERFLOW_CHECK
        cl_err = clEnqueueSVMMemcpy(cmd_queue, CL_NON_BLOCKING, (char*)map_ptrs[i] + POISON_FILL_LENGTH,
                canary_ptrs[POISON_REGIONS*i + 1], POISON_FILL_LENGTH, 1, incoming_evt,
                &copy_events[POISON_REGIONS*i + 1]);
        check_cl_error(__FILE__, __LINE__, cl_err);
#endif

        //map in smaller svm
        cl_err = clEnqueueSVMMap(cmd_queue, CL_NON_BLOCKING, CL_MAP_READ,
                map_ptrs[i], POISON_REGIONS*POISON_FILL_LENGTH, POISON_REGIONS, &copy_events[POISON_REGIONS*i],
                &(map_events[i]));
        check_cl_error(__FILE__, __LINE__, cl_err);
    }
}

static void check_svm_buffers(cl_command_queue cmd_queue, uint32_t num_svm,
        void **map_ptrs, void **svm_ptrs, uint8_t *right_context,
        kernel_info *kern_info, uint32_t *dupe)
{
    for (uint32_t i = 0; i < num_svm; i++)
    {
        if (right_context[i] == 0)
            continue;
        //parse through the canary data
        cpu_parse_canary(cmd_queue, POISON_REGIONS*POISON_FILL_LENGTH, map_ptrs[i], kern_info,
                svm_ptrs[i], dupe);
    }
}

static void unmap_svm_buffers(cl_context kern_ctx, cl_command_queue cmd_queue,
        uint32_t num_svm, void **map_ptrs, cl_event * unmap_events,
        uint8_t *right_context)
{
    cl_int cl_err;
    for (uint32_t i = 0; i < num_svm; i++)
    {
        if (right_context[i] == 0)
        {
            unmap_events[i] = create_complete_user_event(kern_ctx);
            continue;
        }
        // unmap smaller svm
        cl_err = clEnqueueSVMUnmap(cmd_queue, map_ptrs[i], 0, NULL,
                &(unmap_events[i]));
        check_cl_error(__FILE__, __LINE__, cl_err);
    }
}

static void release_and_free(cl_context kern_ctx, uint32_t num_svm,
        void **map_ptrs, cl_event * copy_events, cl_event * map_events,
        cl_event * unmap_events, uint8_t *right_context)
{
    for (uint32_t i = 0; i < num_svm; i++)
    {
        if (right_context[i] == 0)
            continue;
        clSVMFree(kern_ctx, map_ptrs[i]);
        clReleaseEvent(copy_events[i]);
        clReleaseEvent(map_events[i]);
        clReleaseEvent(unmap_events[i]);
    }
}
#endif

// Find any overflows in SVM objects.
// Inputs:
//      kern_info: information about the kernel that had the last opportunity
//                  to write to this buffer.
//      num_svm: number of SVM buffers in the array svm_ptrs
//      svm_ptrs: array of void* that each point to an SVM region
//      dupe:   array that describes which, if any, arguments to the kernel
//              are duplicates of one another. See below for full description.
//      evt:    The cl_event that tells us when the real kernel has completed,
//              so that we can start checking its canaries.
void verify_svm(kernel_info *kern_info, uint32_t num_svm,
        void **svm_ptrs, uint32_t *dupe, const cl_event *evt)
{
#ifdef CL_VERSION_2_0
    if (num_svm <= 0)
        return;

    cl_context kern_ctx;
    cl_int cl_err = clGetKernelInfo(kern_info->handle, CL_KERNEL_CONTEXT,
            sizeof(cl_context), &kern_ctx, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);

    cl_command_queue cmd_queue;
    getCommandQueueForContext(kern_ctx, &cmd_queue);

    // Array that points to the original SVM buffer bases
    void **base_ptrs = malloc(num_svm * sizeof(void*));
    // Array that points to the original canary regions
    void **canary_ptrs = malloc(POISON_REGIONS*num_svm * sizeof(void*));
    // Array that points to all the mapped canaries
    void **map_ptrs = malloc(num_svm * sizeof(void*));
    cl_event * copy_events = malloc(POISON_REGIONS*num_svm * sizeof(cl_event));
    cl_event * map_events = malloc(num_svm * sizeof(cl_event));
    cl_event * unmap_events = malloc(num_svm * sizeof(cl_event));
    uint8_t * right_context = calloc(num_svm, sizeof(uint8_t));

    set_up_svm_canary_copy(kern_ctx, num_svm, svm_ptrs, base_ptrs, map_ptrs,
            canary_ptrs, right_context);

    copy_and_map_svm_canaries(kern_ctx, cmd_queue, num_svm, canary_ptrs,
            map_ptrs, copy_events, map_events, evt, right_context);

    cl_err = clWaitForEvents(num_svm, map_events);
    check_cl_error(__FILE__, __LINE__, cl_err);
    check_svm_buffers(cmd_queue, num_svm, map_ptrs, svm_ptrs, right_context,
            kern_info, dupe);

    unmap_svm_buffers(kern_ctx, cmd_queue, num_svm, map_ptrs, unmap_events,
            right_context);

    // Wait until unmapping is done and then leave the function.
    cl_err = clWaitForEvents(num_svm, unmap_events);
    check_cl_error(__FILE__, __LINE__, cl_err);
    release_and_free(kern_ctx, num_svm, map_ptrs, copy_events, map_events,
            unmap_events, right_context);

    free(right_context);
    free(unmap_events);
    free(map_events);
    free(copy_events);
    free(canary_ptrs);
    free(map_ptrs);
    free(base_ptrs);
#else
    (void)kern_info;
    (void)num_svm;
    (void)svm_ptrs;
    (void)dupe;
    (void)evt;
#endif
}
