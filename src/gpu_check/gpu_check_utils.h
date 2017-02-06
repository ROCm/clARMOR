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

#ifndef __GPU_CHECK_UTILS_H
#define __GPU_CHECK_UTILS_H

#include <stdint.h>
#include <CL/cl.h>

#include "detector_defines.h"
#include "meta_data_lists/cl_kernel_lists.h"
#include "meta_data_lists/cl_memory_lists.h"

typedef struct verif_info_ verif_info;
struct verif_info_
{
    kernel_info *kern_info;
    int *first_change;
    void **argMap;
    unsigned groups;
    unsigned num_buffs;
    cl_command_queue cmd_queue;
    void* used_svm;
    int used_svm_is_clmem;
    void **poison_pointers;
    pthread_t parent;
    uint32_t *dupe;
};

#ifdef DEBUG_CHECKER_TIME
void populateKernelTimes( cl_event *event, cl_ulong *queuedTime, cl_ulong *submitTime, cl_ulong *startTime, cl_ulong *endTime );
#endif

/*
 * Copies the canary regions from an image into a flat buffer
 * canaries are copied hierarchically first by row and then slice etc
 * x0->x1->y0->x0->x1->y1->z0->z1
 *
 */
void copy_image_canaries(cl_command_queue cmdQueue, cl_memobj *img, cl_mem dst_buff, uint32_t offset, const cl_event *evt, cl_event *copyFinish);

cl_mem create_result_buffer(cl_context kern_ctx,
        cl_command_queue cmd_queue, uint32_t num_buffers, cl_event *ret_evt);

int * get_change_buffer(cl_command_queue cmd_queue, uint32_t num_buff,
        cl_mem result, uint32_t num_in_evts, cl_event *check_events,
        cl_event *readback_evt);

void analyze_check_results(cl_command_queue cmd_queue, cl_event readback_evt,
        kernel_info *kern_info, uint32_t num_buffers, void **buffer_ptrs,
        void * used_svm, int used_svm_is_clmem, void ** poison_pointers,
        int *first_change, uint32_t *dupe);

void mend_this_canary(cl_context kern_ctx, cl_command_queue cmd_queue,
        void *handle, cl_event copy_event, cl_event *mend_event);

void set_kern_runtime(uint64_t reset_val);
void add_to_kern_runtime(uint64_t add_val);
uint64_t get_kern_runtime(void);
void output_kern_runtime(void);

// Return the canary check kernels for a given context. If the kernel does not
// yet exist for this context, it is created, compiled, etc.
// Which kernel you get is based on environment settings and
// the number of buffers to check.
cl_kernel get_canary_check_kernel(cl_context context);

// Same as above, but call this when you have no SVM buffers to check. This
// will use a kernel that is simpler and has no SVM handling logic. It should
// therefore run faster.
cl_kernel get_canary_check_kernel_no_svm(cl_context context);

// Return the image canary check kernel for a given context. If the kernel does
// not yet exist for this context, it is created, compiled etc.
cl_kernel get_canary_check_kernel_image(cl_context context);

#endif // __GPU_CHECK_UTILS_H
