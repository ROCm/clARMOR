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


/*! \file cl_utils.h
 * Methods for interaction with cl objects
 */

#ifndef _CL_UTILS_H_
#define _CL_UTILS_H_

#include "cl_interceptor.h"

/*!
 * Takes in a cl_context and creates a user event within that context.
 * Also sets the user event to CL_COMPLETE. Then returns in.
 *
 * \param kern_ctx
 *      event context
 * \return user event (complete)
 */
cl_event create_complete_user_event(cl_context kern_ctx);

/*!
 * finds the space for a flattened canary array for this image. This is
 * useful for functions that want to read a linearized version of the
 * canaries for checking.
 *
 * \param image_type
 *      a description of what type of image this is, 1D, 2D, etc.
 * \param data_size
 *      each pixel in the image is this many bytes
 * \param i_lim
 *      the full width of the image, including canaries
 * \param j_lim
 *      the full height of the image, including canaries
 * \param j_dat
 *      the height of the original image, without canaries
 * \param k_dat
 *      the depth of the original image, without canaries
 * \return length of the canaries
 */
uint64_t get_image_canary_size(cl_mem_object_type image_type,
        size_t data_size, uint32_t i_lim, uint32_t j_lim, uint32_t j_dat,
        uint32_t k_dat);

/*!
 * This function calculates the many dimensions of a 1D, 2D, or 3D
 * cl_mem image, including how much space is reserved for canaries.
 *
 * \param image_desc
 *      this describes the image, and this function then splits
 *      out the appropriate information into the other vars.
 *
 * \param i_lim
 *      actual width of the image,
 *      including the canaries.
 * \param j_lim
 *      actual height of the image,
 *      including the canaries.
 * \param k_lim
 *      actual depth of the image,
 *      including the canaries.
 * \param i_dat
 *      width of the original
 *      image *without* the included canaries.
 * \param j_dat
 *      height of the original
 *      image *without* the included canaries.
 * \param k_dat
 *      depth of the original
 *      image *without* the included canaries.
 */
void get_image_dimensions(cl_image_desc image_desc, uint32_t *i_lim,
        uint32_t *j_lim, uint32_t *k_lim, uint32_t *i_dat,
        uint32_t *j_dat, uint32_t *k_dat);

/*!
 * Assign all of the parts of a launchOclKernelStruct.
 *
 * \param cmd_queue
 * \param kernel
 * \param work_dim
 * \param global_work_offset
 * \param global_work
 * \param local_work
 * \param num_events_in_wait_list
 * \param kern_wait
 * \param out_evt
 * \return launchOclKernelStruct
 */
launchOclKernelStruct setup_ocl_args(cl_command_queue cmd_queue,
        cl_kernel kernel, cl_uint work_dim, size_t * global_work_offset,
        size_t *global_work, size_t *local_work,
        cl_uint num_events_in_wait_list, cl_event *kern_wait,
        cl_event *out_evt);

/*!
 * Set a particular argument for the kernel that is passed in, and then
 * automatically run check_cl_error() to ensure there's no problem.
 *
 * \param kernel
 * \param arg_index
 * \param arg_size
 * \param arg_value
 */
void cl_set_arg_and_check(cl_kernel kernel, cl_uint arg_index, size_t arg_size,
        const void *arg_value);

#ifdef CL_VERSION_2_0
/*!
 * Set a particular SVM argument and check it, like above.
 *
 * \param kernel
 * \param arg_index
 * \param arg_value
 */
void cl_set_svm_arg_and_check(cl_kernel kernel, cl_uint arg_index,
        const void *arg_value);

#endif // CL_VERSION_2_0
#endif // _CL_UTILS_H_
