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

#include "util_functions.h"
#include "cl_err.h"
#include "cl_utils.h"

#include "../gpu_check_utils.h"
#include "copy_canary_cl_image.h"
#include "copy_canary_cl_buffer.h"

#include "gpu_check_copy_canary.h"

void verify_on_gpu_copy_canary(cl_command_queue cmd_queue, uint32_t num_cl_mem,
        uint32_t num_svm, uint32_t num_images, void **buffer_ptrs,
        void **image_ptrs, int copy_svm_ptrs, kernel_info *kern_info,
        uint32_t *dupe, const cl_event *evt, cl_event *ret_evt)
{
    uint32_t num_events = 0;
    cl_event input_evt;

    // Could have events from cl_mem and SVM, Plus images. So two.
    cl_event evt_list[2];

    set_kern_runtime(0);

    // Setup context, kernel, results buffer, and checker events.
    cl_context kern_ctx;
    clGetCommandQueueInfo(cmd_queue, CL_QUEUE_CONTEXT, sizeof(cl_context),
            &kern_ctx, 0);

    if (evt == NULL)
        input_evt = create_complete_user_event(kern_ctx);
    else
        input_evt = *evt;

    verify_cl_buffer_copy(kern_ctx, cmd_queue, num_cl_mem, num_svm,
            buffer_ptrs, copy_svm_ptrs, kern_info, dupe, &input_evt,
            &(evt_list[num_events]));
    num_events++;

    verify_cl_images_copy(kern_ctx, cmd_queue, num_images, image_ptrs,
            kern_info, dupe, &input_evt, &(evt_list[num_events]));
    num_events++;

    cl_event finish;
    cl_int cl_err = clEnqueueMarkerWithWaitList(cmd_queue, num_events,
            evt_list, &finish);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clReleaseEvent(evt_list[0]);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clReleaseEvent(evt_list[1]);
    check_cl_error(__FILE__, __LINE__, cl_err);

    if(ret_evt)
        *ret_evt = finish;
    else
    {
        cl_err = clReleaseEvent(finish);
        check_cl_error(__FILE__, __LINE__, cl_err);
    }

    output_kern_runtime();
}
