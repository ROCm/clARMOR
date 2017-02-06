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
#include <CL/cl.h>

#include "cl_err.h"
#include "detector_defines.h"
#include "util_functions.h"

#include "cl_utils.h"

cl_event create_complete_user_event(cl_context kern_ctx)
{
    cl_int cl_err;
    cl_event user_evt = clCreateUserEvent(kern_ctx, &cl_err);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clSetUserEventStatus(user_evt, CL_COMPLETE);
    check_cl_error(__FILE__, __LINE__, cl_err);
    return user_evt;
}

// finds the space for a flattened canary array to be returned in canary_p
// space allocated is returned in read_len
//
// hierarchically copies canaries from the image in order of increasing
// dimensionality entries -> rows -> slices
uint64_t get_image_canary_size(cl_mem_object_type image_type,
        size_t data_size, uint32_t i_lim, uint32_t j_lim, uint32_t j_dat,
        uint32_t k_dat)
{
    uint64_t transfer_len = k_dat * j_dat * IMAGE_POISON_WIDTH;

    //total size required for canary array
    switch(image_type)
    {
        case CL_MEM_OBJECT_IMAGE2D_ARRAY:
        case CL_MEM_OBJECT_IMAGE3D:
            transfer_len += IMAGE_POISON_DEPTH * j_lim * i_lim;
        case CL_MEM_OBJECT_IMAGE1D_ARRAY:
        case CL_MEM_OBJECT_IMAGE2D:
            transfer_len += k_dat * IMAGE_POISON_HEIGHT * i_lim;
        case CL_MEM_OBJECT_IMAGE1D:
        case CL_MEM_OBJECT_IMAGE1D_BUFFER:
            break;
        default:
            det_printf("failed to find image type");
            exit(-1);
    }
    transfer_len *= data_size;
    return transfer_len;
}

void get_image_dimensions(cl_image_desc image_desc, uint32_t *i_lim,
        uint32_t *j_lim, uint32_t *k_lim, uint32_t *i_dat,
        uint32_t *j_dat, uint32_t *k_dat)
{
    uint32_t int_i_lim, int_j_lim, int_k_lim;
    uint32_t int_i_dat, int_j_dat, int_k_dat;
    // dimension sizes of the array
    int_i_lim = image_desc.image_width;
    int_j_lim = image_desc.image_height;
    int_k_lim = image_desc.image_depth;

    if(image_desc.image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY)
        int_j_lim = image_desc.image_array_size;
    else if(image_desc.image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY)
        int_k_lim = image_desc.image_array_size;

    //find the dimensions of the data
    int_i_dat = (int_i_lim == 1) ? 1 : (int_i_lim) - IMAGE_POISON_WIDTH;
    int_j_dat = (int_j_lim == 1) ? 1 : (int_j_lim) - IMAGE_POISON_HEIGHT;
    int_k_dat = (int_k_lim == 1) ? 1 : (int_k_lim) - IMAGE_POISON_DEPTH;

    if (i_lim != NULL)
        *i_lim = int_i_lim;
    if (j_lim != NULL)
        *j_lim = int_j_lim;
    if (k_lim != NULL)
        *k_lim = int_k_lim;
    if (i_dat != NULL)
        *i_dat = int_i_dat;
    if (j_dat != NULL)
        *j_dat = int_j_dat;
    if (k_dat != NULL)
        *k_dat = int_k_dat;
}

launchOclKernelStruct setup_ocl_args(cl_command_queue cmd_queue,
        cl_kernel kernel, cl_uint work_dim, size_t * global_work_offset,
        size_t *global_work, size_t *local_work,
        cl_uint num_events_in_wait_list, cl_event *kern_wait,
        cl_event *out_evt)
{
    launchOclKernelStruct ocl_args;
    ocl_args.command_queue = cmd_queue;
    ocl_args.kernel = kernel;
    ocl_args.work_dim = work_dim;
    ocl_args.global_work_offset = global_work_offset;
    ocl_args.global_work_size = global_work;
    ocl_args.local_work_size = local_work;
    ocl_args.num_events_in_wait_list = num_events_in_wait_list;
    ocl_args.event_wait_list = kern_wait;
    ocl_args.event = out_evt;
    return ocl_args;
}

void cl_set_arg_and_check(cl_kernel kernel, cl_uint arg_index, size_t arg_size,
        const void *arg_value)
{
    cl_int cl_err = clSetKernelArg(kernel, arg_index, arg_size, arg_value);
    check_cl_error(__FILE__, __LINE__, cl_err);
}

#ifdef CL_VERSION_2_0
void cl_set_svm_arg_and_check(cl_kernel kernel, cl_uint arg_index,
        const void *arg_value)
{
    cl_int cl_err = clSetKernelArgSVMPointer(kernel, arg_index, arg_value);
    check_cl_error(__FILE__, __LINE__, cl_err);
}
#endif
