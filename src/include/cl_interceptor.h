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

#ifndef __CL_INTERCEPTOR_H
#define __CL_INTERCEPTOR_H

#include <dlfcn.h>

#ifdef __cplusplus
extern "C" {
#endif

// Structure used to hold all of the arguments to a call to
// clEnqueueNDRangeKernel. Because the OpenCL API wrapper will catch any
// calls to this, the detector-internal kernel calls would be analyzed,
// leading to an infinite recursion of checks. Instead, we directly call
// the real interceptor function using runNDRangeKernel().
// That call takes this struct, which includes all of the arguments that
// it will eventually pass to the real OpenCL kernel enqueue function.
typedef struct {
    cl_command_queue command_queue;
    cl_kernel kernel;
    cl_uint work_dim;
    const size_t * global_work_offset;
    const size_t * global_work_size;
    const size_t * local_work_size;
    cl_uint num_events_in_wait_list;
    const cl_event * event_wait_list;
    cl_event * event;
} launchOclKernelStruct;

// Function that will run clEnqueueNDRangeKernel without going through the
// buffer overflow detector wrapper. The API is the same as the OpenCL
// API for clEnqueueNDRangeKernel, except that the arguments are passed
// through a launchOclKernelStruct.
cl_int runNDRangeKernel(launchOclKernelStruct *ocl_args);

// Function to call clSVMAlloc() when we are inside of our buffer overflow
// detector. If this function is called instead of clSVMAlloc(), the newly
// created SVM region will not have a canary appended at the end.
void *internalSVMAlloc(cl_context context, cl_svm_mem_flags flags,
        size_t size, unsigned int alignment);

// Call this when releasing a cl_mem region from an internal allocation.
// This is primarily only used for proper memory allocation size tracking.
void releaseInternalMemObject(cl_mem memobj);

// Given an OpenCL context, find a command queue associated with it. We
// handle this in our OpenCL wrapper becuase we cache command queues.
// (For reasons why we cache the command queues, see the comment in the
// wrapper for clCreateCommandQueue). If we do not have a cached command
// queue for this context, this call will create one. As such, you should
// always be able to get a legal command queue out of this call.
// Returns 0 on success, non-zero on failure.
int getCommandQueueForContext(cl_context context, cl_command_queue *cmdQueue);

#ifdef __cplusplus
}
#endif

#endif  /* __CL_INTERCEPTOR_H */
