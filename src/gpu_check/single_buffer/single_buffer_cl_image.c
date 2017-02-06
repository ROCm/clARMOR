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

#include "cl_err.h"
#include "cl_utils.h"
#include "../gpu_check_utils.h"
#include "check_utils.h"

#include "single_buffer_cl_image.h"

static void perform_cl_image_checks(cl_context kern_ctx,
        cl_command_queue cmd_queue, cl_kernel check_kern, cl_event init_evt,
        cl_event real_kern_evt, cl_mem * result, uint32_t num_images,
        void **image_ptrs, uint32_t* canary_lengths, cl_event *check_events)
{
    // Set up kernel invocation constants
    size_t global_work[3] = {1, 1, 1};
    size_t local_work[3] = {64, 1, 1};
    cl_event kern_wait[3] = {init_evt, real_kern_evt};

    // Set up checker kernel launch API arguments
    launchOclKernelStruct ocl_args = setup_ocl_args(cmd_queue, check_kern,
            1, NULL, global_work, local_work, 2, kern_wait, NULL);

    // Set up constant cl_mem checker kernel arguments
    uint32_t check_num = 1;
    clSetKernelArg(check_kern, 0, sizeof(uint8_t), &poisonFill_8b);
    clSetKernelArg(check_kern, 1, sizeof(unsigned), &check_num);
    clSetKernelArg(check_kern, 4, sizeof(void*), (void*) result);

    // We check each buffer independently
    for(uint32_t i = 0; i < num_images; i++)
    {
        cl_int cl_err;
        cl_mem canaryLength = clCreateBuffer(kern_ctx, CL_MEM_COPY_HOST_PTR,
                sizeof(uint32_t), &(canary_lengths[i]), &cl_err);
        check_cl_error(__FILE__, __LINE__, cl_err);

        cl_mem imgCanaryBuff = clCreateBuffer(kern_ctx, 0, canary_lengths[i],
                NULL, &cl_err);
        check_cl_error(__FILE__, __LINE__, cl_err);

        cl_memobj *img = cl_mem_find(get_cl_mem_alloc(), image_ptrs[i]);
        if(img == NULL)
        {
            det_fprintf(stderr, "failure to find cl_memobj.\n");
            exit(-1);
        }

        uint32_t offset = 0;
        cl_event copy_evt, mend_evt;
        copy_image_canaries(cmd_queue, img, imgCanaryBuff, offset,
                &real_kern_evt, &copy_evt);
        if(!get_error_envvar())
        {
            mendCanaryRegion(cmd_queue, img->handle, CL_FALSE, 1, &copy_evt,
                    &mend_evt);
            kern_wait[1] = mend_evt;
        }
        else
            kern_wait[1] = copy_evt;

        global_work[0] = canary_lengths[i];
        ocl_args.global_work_size = global_work;
        ocl_args.event_wait_list = kern_wait;
        ocl_args.event = &(check_events[i]);

        clSetKernelArg(check_kern, 2, sizeof(void*), (void*) &canaryLength);
        clSetKernelArg(check_kern, 3, sizeof(void*), (void*) &imgCanaryBuff);

        cl_err = runNDRangeKernel( &ocl_args );
        check_cl_error(__FILE__, __LINE__, cl_err);

        clReleaseMemObject(canaryLength);
        clReleaseMemObject(imgCanaryBuff);

#ifdef SEQUENTIAL
        clFinish(cmd_queue);
#endif

        if(global_tool_stats_flags & STATS_CHECKER_TIME)
        {
            clFinish(cmd_queue);
            uint64_t times[4];
            populateKernelTimes(&(check_events[i]), &times[0], &times[1],
                    &times[2], &times[3]);
            add_to_kern_runtime((times[3] - times[2]) / 1000);
        }
    }
}

void verify_cl_images_single(cl_context kern_ctx, cl_command_queue cmd_queue,
        uint32_t num_images, void **image_ptrs, kernel_info *kern_info,
        uint32_t *dupe, const cl_event *evt, cl_event *ret_evt)
{
    if(num_images <= 0)
    {
        if (ret_evt != NULL)
            *ret_evt = create_complete_user_event(kern_ctx);
        return;
    }

    //find lengths
    uint32_t *canary_lengths = malloc(sizeof(uint32_t) * num_images);
    for(uint32_t i = 0; i < num_images; i++)
    {
        cl_memobj *img = cl_mem_find(get_cl_mem_alloc(), image_ptrs[i]);
        if(img == NULL)
        {
            det_fprintf(stderr, "failure to find cl_memobj for image.\n");
            exit(-1);
        }
        uint32_t i_lim, j_lim, k_lim, i_dat, j_dat, k_dat;
        get_image_dimensions(img->image_desc, &i_lim, &j_lim, &k_lim, &i_dat,
                &j_dat, &k_dat);
        size_t data_size = getImageDataSize(&img->image_format);
        canary_lengths[i] = get_image_canary_size(img->image_desc.image_type,
                data_size, i_lim, j_lim, j_dat, k_dat);
    }

    cl_event init_evt;
    cl_mem result = create_result_buffer(kern_ctx, cmd_queue, num_images,
            &init_evt);
    cl_event *check_events = malloc(sizeof(cl_event) * num_images);

    // This will walk through all of the cl_mem buffers and launch a GPU kernel
    // to check whether their canaries have been corrupted.
    cl_kernel check_kern = get_canary_check_kernel_image(kern_ctx);
    perform_cl_image_checks(kern_ctx, cmd_queue, check_kern, init_evt, *evt,
            &result, num_images, image_ptrs, canary_lengths, check_events);

    // Read back the results from all of the checks into 'first_change'.
    cl_event readback_evt;
    int *first_change = get_change_buffer(cmd_queue, num_images, result,
            num_images, check_events, &readback_evt);
    if(ret_evt != NULL)
        *ret_evt = readback_evt;

    // Release the memory objects and events that are used by the read.
    // Because they're queued, this is OK. The release won't destroy them until
    // they are no longer needed.
    clReleaseMemObject(result);
    free(canary_lengths);
    for (uint32_t i = 0; i < num_images; i++)
    {
        cl_int cl_err = clReleaseEvent(check_events[i]);
        check_cl_error(__FILE__, __LINE__, cl_err);
    }
    free(check_events);

    // Finally, check the results of the memory checks above.
    analyze_check_results(cmd_queue, readback_evt, kern_info, num_images,
            image_ptrs, NULL, 0, NULL, first_change, dupe);
}
