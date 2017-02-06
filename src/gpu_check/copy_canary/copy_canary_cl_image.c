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
#include "util_functions.h"
#include "../gpu_check_utils.h"

#include "copy_canary_cl_image.h"

static uint32_t find_canary_ends(void **image_ptrs, uint32_t num_images,
        uint32_t *canary_ends)
{
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
        uint32_t len =  get_image_canary_size(img->image_desc.image_type,
                data_size, i_lim, j_lim, j_dat, k_dat);
        if(i==0)
            canary_ends[i] = len;
        else
            canary_ends[i] = canary_ends[i-1] + len;
    }
    return canary_ends[num_images-1];
}

static cl_memobj * copy_this_image_canary(cl_command_queue cmd_queue,
        void *image, cl_mem canary_copies, uint32_t image_num,
        uint32_t *canary_ends, const cl_event *evt, cl_event *ret_evt)
{
    cl_memobj *img = cl_mem_find(get_cl_mem_alloc(), image);
    if(img == NULL)
    {
        det_fprintf(stderr, "failure to find cl_memobj.\n");
        exit(-1);
    }
    uint32_t offset = 0;
    if(image_num > 0)
        offset = canary_ends[image_num-1];

    copy_image_canaries(cmd_queue, img, canary_copies, offset, evt, ret_evt);
    return img;
}

static cl_event perform_cl_image_checks(cl_context kern_ctx,
        cl_command_queue cmd_queue, cl_kernel check_kern, cl_event *init_evts,
        uint32_t num_init_evts, cl_event *mend_events, cl_mem canary_copies,
        cl_mem result, uint32_t num_images, uint32_t* canary_ends,
        uint32_t total_canary_len)
{
    cl_int cl_err;
    size_t global_work[3] = {total_canary_len, 1, 1};

    // Create a buffer that holds the endpoint of each buffer, so that the
    // device kernel can see it.
    cl_mem canary_ends_buf = clCreateBuffer(kern_ctx, CL_MEM_COPY_HOST_PTR,
            sizeof(uint32_t)*num_images, canary_ends, &cl_err);
    check_cl_error(__FILE__, __LINE__, cl_err);

    cl_set_arg_and_check(check_kern, 0, sizeof(uint8_t), &poisonFill_8b);
    cl_set_arg_and_check(check_kern, 1, sizeof(unsigned), &num_images);
    cl_set_arg_and_check(check_kern, 2, sizeof(void*),
            (void*) &canary_ends_buf);
    cl_set_arg_and_check(check_kern, 3, sizeof(void*), (void*) &canary_copies);
    cl_set_arg_and_check(check_kern, 4, sizeof(void*), (void*) &result);

    // Launch the kernel that checks the copies of the canary values.
    cl_event kern_end;
    launchOclKernelStruct ocl_args = setup_ocl_args(cmd_queue, check_kern,
        1, NULL, global_work, NULL, num_init_evts, init_evts, &kern_end);
    cl_err = runNDRangeKernel( &ocl_args );
    check_cl_error(__FILE__, __LINE__, cl_err);

    if(global_tool_stats_flags & STATS_CHECKER_TIME)
    {
        clFinish(cmd_queue);
        uint64_t times[4];
        populateKernelTimes(&kern_end, &times[0], &times[1], &times[2],
                &times[3]);
        add_to_kern_runtime((times[3] - times[2]) / 1000);
    }

    // If we are also fixing any of the corrupted canary bits in the original
    // buffer, then combine those events with waiting for the kernel to check
    // the copied canaries.
    cl_event mend_finish, finish;
    cl_err = clEnqueueMarkerWithWaitList(cmd_queue, num_images, mend_events,
            &mend_finish);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_event evt_list[] = {mend_finish, kern_end};
    cl_err = clEnqueueMarkerWithWaitList(cmd_queue, 2, evt_list, &finish);
    check_cl_error(__FILE__, __LINE__, cl_err);

    // When the kernel finishes checking this, it will release the buf
    clReleaseMemObject(canary_ends_buf);
    clReleaseEvent(mend_finish);
    clReleaseEvent(kern_end);
    return finish;
}

void verify_cl_images_copy(cl_context kern_ctx, cl_command_queue cmd_queue,
        uint32_t num_images, void **image_ptrs, kernel_info *kern_info,
        uint32_t *dupe, const cl_event *evt, cl_event *ret_evt)
{
    if(num_images <= 0)
    {
        if (ret_evt != NULL)
            *ret_evt = create_complete_user_event(kern_ctx);
        return;
    }

    // find how much space the canary values for each image take up.
    // Store them into
    uint32_t *canary_ends = malloc(sizeof(uint32_t) * num_images);
    uint32_t total_canary_len = find_canary_ends(image_ptrs, num_images,
            canary_ends);

    // Create a buffer big enough to hold all of the canaries
    cl_int cl_err;
    cl_mem canary_copies = clCreateBuffer(kern_ctx, 0, total_canary_len, 0,
            &cl_err);
    check_cl_error(__FILE__, __LINE__, cl_err);

    cl_event *events = calloc(sizeof(cl_event), (num_images+1));
    cl_event *mend_events = calloc(sizeof(cl_event), (num_images));


    //copy image canaries into this single buffer
    for(uint32_t i = 0; i < num_images; i++)
    {
        cl_memobj *img = copy_this_image_canary(cmd_queue, image_ptrs[i],
                canary_copies, i, canary_ends, evt, &events[i]);
        mend_this_canary(kern_ctx, cmd_queue, img->handle, events[i],
                &mend_events[i]);
    }

    cl_mem result = create_result_buffer(kern_ctx, cmd_queue, num_images,
            &events[num_images]);

    // At this point, copying the canaries is events[0] through events[n-1]
    // and initializing the results buffer is events[n].
    cl_kernel check_kern = get_canary_check_kernel_image(kern_ctx);
    cl_event check_done = perform_cl_image_checks(kern_ctx, cmd_queue,
            check_kern, events, num_images+1, mend_events, canary_copies,
            result, num_images, canary_ends, total_canary_len);

    // canary_ends was copied to a GPU buffer in the kernel
    // events are no longer needed, nor are the mend_events. They have been
    // consolidated into the 'check_done' event.
    free(canary_ends);
    free(events);
    free(mend_events);

    cl_event read_result;
    int * firstChange = get_change_buffer(cmd_queue, num_images, result, 1,
            &check_done, &read_result);

    // If anyone wants to wait for this to get done, they should wait for
    // both the kernel and its checks.
    if(ret_evt)
        *ret_evt = read_result;

    analyze_check_results(cmd_queue, read_result, kern_info, num_images,
            image_ptrs, NULL, 0, NULL, firstChange, dupe);

    clReleaseMemObject(result);
    clReleaseMemObject(canary_copies);
}
