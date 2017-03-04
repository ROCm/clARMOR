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

#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include "detector_defines.h"
#include "cl_err.h"
#include "cl_utils.h"
#include "gpu_check_kernels.h"
#include "util_functions.h"
#include "check_utils.h"
#include "universal_copy.h"
#include "overflow_error.h"

#include "gpu_check_utils.h"


void populateKernelTimes( cl_event *event, cl_ulong *queuedTime, cl_ulong *submitTime, cl_ulong *startTime, cl_ulong *endTime )
{
    cl_int retval = 0;
    retval = clGetEventProfilingInfo( *event, CL_PROFILING_COMMAND_QUEUED, sizeof(*queuedTime), queuedTime, NULL );
    if (retval != CL_SUCCESS)
    {
        det_fprintf(stderr, "Error attempting to get profiling info for kernel queued time: %s\n", cluErrorString(retval));
        exit(-1);
    }
    retval = clGetEventProfilingInfo( *event, CL_PROFILING_COMMAND_SUBMIT, sizeof(*submitTime), submitTime, NULL );
    if (retval != CL_SUCCESS)
    {
        det_fprintf(stderr, "Error attempting to get profiling info for kernel submit time: %s\n", cluErrorString(retval));
        exit(-1);
    }
    retval = clGetEventProfilingInfo( *event, CL_PROFILING_COMMAND_START, sizeof(*startTime), startTime, NULL );
    if (retval != CL_SUCCESS)
    {
        det_fprintf(stderr, "Error attempting to get profiling info for kernel start time: %s\n", cluErrorString(retval));
        exit(-1);
    }
    retval = clGetEventProfilingInfo( *event, CL_PROFILING_COMMAND_END, sizeof(*endTime), endTime, NULL );
    if (retval != CL_SUCCESS)
    {
        det_fprintf(stderr, "Error attempting to get profiling info for kernel end time: %s\n", cluErrorString(retval));
        exit(-1);
    }
}

/*
 * copy the image canary regions to a flattened format in the destination buffer
 * format is hierarchical entry->row->slice order
 *
 *
 */
void copy_image_canaries(cl_command_queue cmdQueue, cl_memobj *img, cl_mem dst_buff, uint32_t offset, const cl_event *evt, cl_event *copyFinish)
{
    cl_int cl_err;
    size_t origin[3], region[3];
    size_t dataSize;
    uint32_t j, k;
    uint32_t i_lim, j_lim, k_lim;
    uint32_t i_dat, j_dat, k_dat;
    //dimensions of data + canaries
    i_lim = img->image_desc.image_width;
    j_lim = img->image_desc.image_height;
    k_lim = img->image_desc.image_depth;
    if(img->image_desc.image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY)
        j_lim = img->image_desc.image_array_size;
    else if(img->image_desc.image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY)
        k_lim = img->image_desc.image_array_size;

    //find the dimensions of the data
    i_dat = (i_lim == 1) ? 1 : i_lim - IMAGE_POISON_WIDTH;
    j_dat = (j_lim == 1) ? 1 : j_lim - IMAGE_POISON_HEIGHT;
    k_dat = (k_lim == 1) ? 1 : k_lim - IMAGE_POISON_DEPTH;


    dataSize = getImageDataSize(&img->image_format);

    uint32_t segment;
    uint32_t numEvents;
    cl_event *events;

    if(k_lim > 1)
        numEvents = k_dat*j_dat + k_dat + 1;
    else if(j_lim > 1)
        numEvents = k_dat*j_dat + k_dat;
    else
        numEvents = k_dat*j_dat;

    events = malloc(sizeof(cl_event)*numEvents);

    for(k = 0; k < k_dat; k++)
    {
        for(j = 0; j < j_dat; j++)
        {
            if(i_lim > 1)
            {
                origin[0] = i_dat;
                origin[1] = j;
                origin[2] = k;
                region[0] = IMAGE_POISON_WIDTH;
                region[1] = 1;
                region[2] = 1;
                segment = offset + (k*(j_dat*IMAGE_POISON_WIDTH + IMAGE_POISON_HEIGHT*i_lim) + j*IMAGE_POISON_WIDTH)*dataSize;
                cl_image_to_buffer_copy(cmdQueue, img->handle, dst_buff, origin, region, segment, 1, evt, &events[k*(j_dat + 1) + j]);
            }
        }

        if(j_lim > 1)
        {
            origin[0] = 0;
            origin[1] = j_dat;
            origin[2] = k;
            region[0] = i_lim;
            region[1] = IMAGE_POISON_HEIGHT;
            region[2] = 1;
            segment = offset + (k*IMAGE_POISON_HEIGHT*i_lim + (k + 1)*(j_dat*IMAGE_POISON_WIDTH))*dataSize;
            cl_image_to_buffer_copy(cmdQueue, img->handle, dst_buff, origin, region, segment, 1, evt, &events[k*(j_dat + 1) + j_dat]);
        }
    }

    if(k_lim > 1)
    {
        origin[0] = 0;
        origin[1] = 0;
        origin[2] = k_dat;
        region[0] = i_lim;
        region[1] = j_lim;
        region[2] = IMAGE_POISON_DEPTH;
        segment = offset + (k_dat*IMAGE_POISON_HEIGHT*i_lim + k_dat*j_dat*IMAGE_POISON_WIDTH)*dataSize;
        cl_image_to_buffer_copy(cmdQueue, img->handle, dst_buff, origin, region, segment, 1, evt, &events[k_dat*(j_dat + 1)]);
    }

#ifdef CL_VERSION_1_2
    cl_err = clEnqueueMarkerWithWaitList(cmdQueue, numEvents, events, copyFinish);
    check_cl_error(__FILE__, __LINE__, cl_err);
#else
    cl_err = clEnqueueMarker(cmdQueue, copyFinish);
    check_cl_error(__FILE__, __LINE__, cl_err);
#endif

    free(events);
}

static void report_kernel_overflows(uint32_t num_buffs, int *first_change,
        void **argMap, kernel_info *kern_info, verif_info *data)
{
    uint32_t i;
    uint8_t found_an_overflow = 0;
    pthread_t parentThread = data->parent;
    uint32_t *dupe = data->dupe;

    for(i = 0; i < num_buffs; i++)
    {
        if(first_change[i] < INT_MAX)
        {
            overflowError(kern_info, argMap[i], first_change[i], data->backtrace_str);
            found_an_overflow = 1;
        }
    }

    if(found_an_overflow)
    {
        printDupeWarning(kern_info->handle, dupe);
        optionalKillOnOverflow(get_exitcode_envvar(), parentThread);
    }
}

#ifdef KERN_CALLBACK
 //for asynchronously completing checker kernels
// report findings
// clean up data
static void verify_callback(cl_event event, cl_int status, void* verif_data)
{
    if(event || status){}
    verif_info *data = (verif_info*)verif_data;
    kernel_info *kern_info = data->kern_info;
    int *first_change = data->first_change;
    void **argMap = data->argMap;
    unsigned num_buffs = data->num_buffs;
    void **poison_pointers = data->poison_pointers;

    report_kernel_overflows(num_buffs, first_change, argMap, kern_info, data);

    clReleaseKernel(kern_info->handle);

#ifdef CL_VERSION_2_0
    void *used_svm = data->used_svm;
    int used_svm_is_clmem = data->used_svm_is_clmem;
    if(used_svm)
    {
        cl_command_queue cmd_queue = data->cmd_queue;
        cl_context ctx;
        cl_int cl_err = clGetCommandQueueInfo(cmd_queue, CL_QUEUE_CONTEXT,
                sizeof(cl_context), &ctx, 0);
        check_cl_error(__FILE__, __LINE__, cl_err);
        if (used_svm_is_clmem)
            releaseInternalMemObject(used_svm);
        else
            clSVMFree(ctx, used_svm);
    }
#endif

    if (poison_pointers)
        free(poison_pointers);
    free(data->dupe);
    free(first_change);
    free(argMap);
    if (data->backtrace_str)
        free(data->backtrace_str);
    free(data);
}
#endif //KERN_CALLBACK

cl_mem create_result_buffer(cl_context kern_ctx,
        cl_command_queue cmd_queue, uint32_t num_buffers, cl_event *ret_evt)
{
    cl_int cl_err;
    cl_mem result;
    // Create buffer to hold the results of the buffer overflow checks.
    // Fill it with INT_MAX to initialize it.
    result = clCreateBuffer(kern_ctx, 0, sizeof(int)*num_buffers, 0, &cl_err);
    check_cl_error(__FILE__, __LINE__, cl_err);
#ifdef CL_VERSION_1_2
    int fill = INT_MAX;
    cl_err = clEnqueueFillBuffer(cmd_queue, result, &fill, sizeof(int), 0,
            sizeof(int)*num_buffers, 0, 0, ret_evt);
    check_cl_error(__FILE__, __LINE__, cl_err);
#else
    int * fill_buf = malloc(sizeof(int)*num_buffers);
    for (uint32_t i = 0; i < num_buffers && i < UINT_MAX; i++)
        fill_buf[i] = INT_MAX;
    // Must do a blocking write here so that we can appropriately free fill_buf
    // This will be slower, but this is only slow on old OpenCL 1.1 systems.
    cl_err = clEnqueueWriteBuffer(cmd_queue, result, CL_BLOCKING, 0,
            sizeof(int)*num_buffers, fill_buf, 0, NULL, ret_evt);
    check_cl_error(__FILE__, __LINE__, cl_err);
    free(fill_buf);
#endif
    return result;
}

int * get_change_buffer(cl_command_queue cmd_queue, uint32_t num_buff,
        cl_mem result, uint32_t num_in_evts, cl_event *check_events,
        cl_event *readback_evt)
{
#ifdef KERN_CALLBACK
    cl_bool block = CL_FALSE;
#else
    cl_bool block = CL_TRUE;
#endif
    int *first_change = malloc(sizeof(int) * num_buff);
    cl_int cl_err = clEnqueueReadBuffer(cmd_queue, result, block, 0,
            sizeof(int)*num_buff, first_change, num_in_evts, check_events,
            readback_evt);
    check_cl_error(__FILE__, __LINE__, cl_err);
    return first_change;
}

void analyze_check_results(cl_command_queue cmd_queue, cl_event readback_evt,
        kernel_info *kern_info, uint32_t num_buffers, void **buffer_ptrs,
        void* used_svm, int used_svm_is_clmem, void **poison_ptrs,
        int *first_change, uint32_t *dupe)
{
#ifdef KERN_CALLBACK
    // Make sure that the user does not clRelease this kernel and blow us up.
    clRetainKernel(kern_info->handle);

    // Gather the information about all the buffers we're going to check
    void **arg_map;
    arg_map = calloc(sizeof(void*), num_buffers);
    for(uint32_t i = 0; i < num_buffers; i++)
        arg_map[i] = buffer_ptrs[i];

    // This info will eventually be passed to the callback (after the data
    // read completes) to check the canary values.
    verif_info *data = calloc(sizeof(verif_info), 1);
    data->kern_info = kern_info;
    data->first_change = first_change;
    data->argMap = arg_map;
    data->num_buffs = num_buffers;
    data->cmd_queue = cmd_queue;
    data->used_svm = used_svm;
    data->used_svm_is_clmem = used_svm_is_clmem;
    data->poison_pointers = poison_ptrs;
    data->parent = pthread_self();

    if(get_print_backtrace_envvar())
    {
        //clEnqueueNDRangeKernel->kernelLaunchFunc->verifyBufferInBounds->verify_on_gpu_copy_canary->verify_cl_buffer_copy->analyze_check_results
        data->backtrace_str = get_backtrace_level(5);
    }

    // Deep copy dupe, because it can be freed before the callback happens.
    uint32_t nargs;
    cl_int cl_err = clGetKernelInfo(kern_info->handle, CL_KERNEL_NUM_ARGS,
            sizeof(nargs), &nargs, 0);
    check_cl_error(__FILE__, __LINE__, cl_err);
    uint32_t *dupe_copy = malloc(nargs * sizeof(uint32_t));
    for (uint32_t i = 0; i < nargs; i++)
        dupe_copy[i] = dupe[i];
    data->dupe = dupe_copy;

    // The checker kernel will run after we are finished reading back the
    // results of the check kernel.
    cl_err = clSetEventCallback(readback_evt, CL_COMPLETE, verify_callback,
            data);
    check_cl_error(__FILE__, __LINE__, cl_err);
#else //KERN_CALLBACK
    (void)cmd_queue;
    verif_info data;
    data.parent = pthread_self();
    data.dupe = dupe;
    data.backtrace_str = NULL;

    if(get_print_backtrace_envvar())
    {
        data.backtrace_str = get_backtrace_level(5);
    }

    report_kernel_overflows(num_buffers, first_change, buffer_ptrs, kern_info,
            &data);

#ifdef CL_VERSION_2_0
    if(used_svm)
    {
        cl_context ctx;
        cl_int cl_err = clGetCommandQueueInfo(cmd_queue, CL_QUEUE_CONTEXT,
                sizeof(cl_context), &ctx, 0);
        check_cl_error(__FILE__, __LINE__, cl_err);
        if (used_svm_is_clmem)
            releaseInternalMemObject(used_svm);
        else
            clSVMFree(ctx, used_svm);
    }
#endif

    if (data.backtrace_str)
        free(data.backtrace_str);
    if (poison_ptrs)
        free(poison_ptrs);
    free(first_change);
#endif //KERN_CALLBACK
}

uint64_t check_kern_runtime_us;

void set_kern_runtime(uint64_t reset_val)
{
    check_kern_runtime_us = reset_val;
}

void add_to_kern_runtime(uint64_t add_val)
{
    check_kern_runtime_us += add_val;
}

uint64_t get_kern_runtime(void)
{
    return check_kern_runtime_us;
}

void output_kern_runtime(void)
{
    if(get_tool_perf_envvar() & STATS_CHECKER_TIME)
    {
        static FILE *debug_out = NULL;

        if(debug_out)
            debug_out = fopen("debug_check_time.csv", "a");
        else
            debug_out = fopen("debug_check_time.csv", "w");

        fprintf(debug_out, "%lu\n", check_kern_runtime_us);
        fclose(debug_out);
    }
}

void mend_this_canary(cl_context kern_ctx, cl_command_queue cmd_queue,
        void *handle, cl_event copy_event, cl_event *mend_event)
{
    if(!get_error_envvar())
    {
        mendCanaryRegion(cmd_queue, handle, CL_FALSE, 1, &copy_event,
                mend_event);
    }
    else
        *mend_event = create_complete_user_event(kern_ctx);
}

#ifdef DEBUG
    uint32_t numBuilds = 0;
#endif

static void check_if_same_kernel(cl_kernel *kernel_ptr, cl_context context,
    const char *name)
{
    cl_kernel kernel = *kernel_ptr;
    if(kernel != NULL)
    {
        cl_context last_ctx;
        clGetKernelInfo(kernel, CL_KERNEL_CONTEXT, sizeof(cl_context),
                &last_ctx, 0);
        if(last_ctx != context)
        {
            clReleaseKernel(kernel);
            kernel = NULL;
        }
        else
        {
            size_t size_ret;
            char *kernelName = NULL;

            cl_int cl_err = clGetKernelInfo(kernel, CL_KERNEL_FUNCTION_NAME, 0,
                    NULL, &size_ret);
            check_cl_error(__FILE__, __LINE__, cl_err);
            if (size_ret == 0)
            {
                det_fprintf(stderr, "Bad calloc size (%lu) at %s:%d\n",
                        size_ret, __FILE__, __LINE__);
                exit(-1);
            }
            kernelName = calloc(size_ret, sizeof(char));
            if (kernelName == NULL)
            {
                det_fprintf(stderr, "Calloc failed at %s:%d (size: %lu)\n",
                        __FILE__, __LINE__, size_ret);
                exit(-1);
            }
            cl_err = clGetKernelInfo(kernel, CL_KERNEL_FUNCTION_NAME,
                    size_ret, kernelName, NULL);
            check_cl_error(__FILE__, __LINE__, cl_err);

            if(strcmp(kernelName, name) != 0)
            {
                clReleaseKernel(kernel);
                kernel = NULL;
            }
            free(kernelName);
        }
    }
}

// Create a kernel using the provided context, kernel name, and source string.
// If the kernel already exists for the given parameters, it is returned
// without rebuilding. Otherwise, a new kernel is created.
// Currently, only the last kernel built is cached.
// Update the internals to a map if performance suffers from this
static cl_kernel get_kernel_for_context(cl_kernel *kernel_ptr,
        cl_context context, const char* name, const char* src)
{
    // If we aren't asking for the same kernel, this will release the kernel
    // and set the kernel at kernel_ptr to NULL.
    check_if_same_kernel(kernel_ptr, context, name);
    cl_kernel kernel = *kernel_ptr;

    if(kernel == NULL)
    {
#ifdef DEBUG
        det_printf("building kernel %u\n", numBuilds++);
#endif
        const char *slist[2] = {src, 0};

        cl_int cl_err;
        cl_program prog = clCreateProgramWithSource(context, 1, slist, NULL,
                &cl_err);
        check_cl_error(__FILE__, __LINE__, cl_err);
#ifdef CL_VERSION_2_0
        cl_err = clBuildProgram(prog, 0, NULL, "-cl-std=CL2.0", NULL, NULL);
#else
        cl_err = clBuildProgram(prog, 0, NULL, "", NULL, NULL);
#endif

        if (cl_err == CL_BUILD_PROGRAM_FAILURE)
            print_program_build_err(context, prog, cl_err);
        check_cl_error(__FILE__, __LINE__, cl_err);

        kernel = clCreateKernel(prog, name, &cl_err);
        check_cl_error(__FILE__, __LINE__, cl_err);
        clReleaseProgram(prog);
    }
    *kernel_ptr = kernel;
    return kernel;
}

cl_kernel check_canary_kern = NULL;
cl_kernel check_canary_kern_no_svm = NULL;
cl_kernel check_img_canary_kern = NULL;

// retrieve kernel based on pre-compiler directive
cl_kernel get_canary_check_kernel(cl_context context)
{
    const char *kernel_name;
    const char *source;
    switch(get_gpu_strat_envvar())
    {
        case GPU_MODE_SINGLE_BUFFER:
            kernel_name = "locateDiffParts";
            source = get_single_buffer_src();
            break;
        case GPU_MODE_MULTI_SVMPTR:
            kernel_name = "locateDiffSVMPtr";
            source = get_buffer_and_ptr_copy_src();
            break;
        default :
            kernel_name = "findCorruption";
            source = get_buffer_copy_canary_src();
            break;
    }
    return get_kernel_for_context(&check_canary_kern, context,
            kernel_name, source);
}

cl_kernel get_canary_check_kernel_no_svm(cl_context context)
{
    const char *kernel_name;
    const char *source;
    switch(get_gpu_strat_envvar())
    {
        case GPU_MODE_SINGLE_BUFFER:
            kernel_name = "locateDiffParts";
            source = get_single_buffer_src();
            break;
        default :
            kernel_name = "findCorruptionNoSVM";
            source = get_buffer_copy_canary_src();
            break;
    }
    return get_kernel_for_context(&check_canary_kern_no_svm, context,
            kernel_name, source);
}

cl_kernel get_canary_check_kernel_image(cl_context context)
{
    const char *kernel_name = "findCorruption";
    const char *source = get_image_copy_canary_src();
    return get_kernel_for_context(&check_img_canary_kern, context,
                kernel_name, source);
}
