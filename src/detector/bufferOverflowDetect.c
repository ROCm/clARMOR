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

/*! \mainpage clARMOR
 *
 * \section intro_sec Introduction
 *
 * This is a tool to find buffer overflows in OpenCL applications.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <CL/cl.h>

#include "util_functions.h"
#include "cl_err.h"
#include "cl_interceptor.h"
#include "detector_defines.h"
#include "universal_copy.h"
#include "meta_data_lists/cl_kernel_lists.h"

#include "gpu_check_single_buffer.h"
#include "gpu_check_copy_canary.h"
#include "cpu_check.h"

#include "bufferOverflowDetect.h"

const unsigned poisonWordLen = POISON_FILL_LENGTH / sizeof(unsigned);
const uint8_t poisonFill_8b = POISON_FILL;
const unsigned poisonFill_32b = POISON_FILL_32B;

cl_command_queue checkQueue = NULL;
cl_context checkCtx = NULL;

/*
 * parse a kernel argument list and find duplicate arguments
 * allocates space necessary for list
 *
 * in the returned list (dupe_p) an argument is represented by its index
 *  kernel argument represented by index i is unique or first if the value at index i is equal to i
 *
 */
void findDuplicates(uint32_t nargs, kernel_arg *args, uint32_t **dupe_p)
{
    uint32_t *dupe;
    if (nargs == 0) {
        return;
    }
    dupe = calloc(nargs, sizeof(uint32_t));
    uint32_t i, j;
    kernel_arg *arg_itr, *arg_itr2;

    arg_itr = args;
    for(i=0; i < nargs; i++)
    {
        dupe[i] = i;
        if(arg_itr->buffer)
        {
            arg_itr2 = args;
            for(j=0; j < i; j++)
            {
                if(arg_itr2->buffer)
                {
                    if(arg_itr->buffer->handle == arg_itr2->buffer->handle)
                    {
                        dupe[i] = j;
                        break;
                    }
                }
                arg_itr2 = arg_itr2->next;
            }
        }
        arg_itr = arg_itr->next;
    }
    *dupe_p = dupe;
}

/*
 * launch canary verification on either the host or gpu
 *
 */
void verifyBufferInBounds(cl_command_queue cmdQueue, cl_kernel kern, uint32_t *dupe, const cl_event *evt, cl_event *retEvt)
{
    cl_int cl_err;
    uint32_t i, nargs;
    uint8_t hasSVM = 0;
    kernel_info *kernInfo;
    kernel_arg *kernArg;
    uint32_t numBuffs, numSVM, numImgs;

    cl_err = clGetKernelInfo(kern, CL_KERNEL_NUM_ARGS, sizeof(nargs), &nargs, 0);
    check_cl_error(__FILE__, __LINE__, cl_err);

    //count out buffers to be checked
    kernInfo = kinfo_find(get_kern_list(), kern);
    numBuffs = 0;
    numImgs = 0;
    for(i = 0; i < nargs; i++)
    {
        if(dupe != NULL && dupe[i] == i)
        {
            cl_memobj *m1 = NULL;
            cl_svm_memobj *m2 = NULL;
            kernArg = karg_find(kernInfo->arg_list, i);
            if(kernArg != NULL)
            {
                m1 = kernArg->buffer; //is it a cl_mem?
                m2 = kernArg->svm_buffer;
            }
            if(m1 != NULL)
            {
                if(m1->is_image)
                    numImgs++;
                else
                    numBuffs++;
            }
            else if(m2 != NULL)
                hasSVM = 1;
        }
    }

    //count svm
    numSVM = 0;
    if(hasSVM)
    {
#ifdef CL_VERSION_2_0
        cl_svm_memobj *svmIter = cl_svm_mem_next(get_cl_svm_mem_alloc(), 0);
        while(svmIter != NULL)
        {
            if(svmIter->detector_internal_buffer != 1)
                numSVM++;
            svmIter = cl_svm_mem_next(get_cl_svm_mem_alloc(), svmIter->handle);
        }
#endif
    }

    //allocate references for buffers being checked
    void **buffer_ptrs = NULL, **image_ptrs = NULL;
    uint32_t totalBuffs = numBuffs;
    uint32_t totalSVM = numSVM;
    uint32_t totalImgs = numImgs;
    if(totalBuffs + totalSVM > 0)
        buffer_ptrs = (void**)calloc(sizeof(void*), totalBuffs + totalSVM);
    if(totalImgs > 0)
        image_ptrs = (void**)calloc(sizeof(void*), totalImgs);

    //populate buffer list & image list
    numBuffs = 0;
    numImgs = 0;
    for (i = 0; i < nargs; i++)
    {
        if (dupe != NULL && dupe[i] == i)
        {
            cl_memobj *m1 = NULL;
            kernArg = karg_find(kernInfo->arg_list, i);
            if (kernArg != NULL)
                m1 = kernArg->buffer; //is it a cl_mem?
            if (m1 != NULL)
            {
                if(m1->is_image && image_ptrs)
                {
                    image_ptrs[numImgs] = m1->handle;
                    numImgs++;
                }
                if(!(m1->is_image) && buffer_ptrs)
                {
                    buffer_ptrs[numBuffs] = m1->handle;
                    numBuffs++;
                }
            }
        }
    }

    //populate buffer list
    if(hasSVM)
    {
#ifdef CL_VERSION_2_0
        numSVM = 0;
        cl_svm_memobj *svmIter = cl_svm_mem_next(get_cl_svm_mem_alloc(), 0);
        while(svmIter != NULL && buffer_ptrs != NULL && numSVM < totalSVM)
        {
            if(svmIter->detector_internal_buffer != 1)
            {
                buffer_ptrs[totalBuffs+numSVM] = svmIter->handle;
                numSVM++;
            }
            svmIter = cl_svm_mem_next(get_cl_svm_mem_alloc(), svmIter->handle);
        }
#endif
    }

    uint32_t checkItems = totalBuffs + totalSVM + totalImgs;

    unsigned int use_device = get_check_on_device_envvar();

    cl_device_id device;
    clGetCommandQueueInfo(cmdQueue, CL_QUEUE_DEVICE, sizeof(cl_device_id), &device, NULL);

    cl_device_type dev_type;
    clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(cl_device_type), &dev_type, NULL);


    //if(checkItems >= gpuCheck)
    if( dev_type != CL_DEVICE_TYPE_CPU &&
            ( ((totalSVM > 0 || totalImgs > 0) && use_device != DEVICE_CPU) ||
              (checkItems > 0 && use_device == DEVICE_GPU) )
            )
    {
        switch(get_gpu_strat_envvar())
        {
            case GPU_MODE_SINGLE_BUFFER:
                verify_on_gpu_single_buffer(cmdQueue, numBuffs, numSVM, numImgs,
                        buffer_ptrs, image_ptrs, kernInfo, dupe, evt, retEvt);
                break;
            case GPU_MODE_MULTI_SVMPTR:
                verify_on_gpu_copy_canary(cmdQueue, numBuffs, numSVM, numImgs,
                        buffer_ptrs, image_ptrs, 1, kernInfo, dupe, evt, retEvt);
                break;
            default :
                verify_on_gpu_copy_canary(cmdQueue, numBuffs, numSVM, numImgs,
                        buffer_ptrs, image_ptrs, 0, kernInfo, dupe, evt, retEvt);
                break;

        }
    }
    else if(checkItems > 0)
    {
        verify_buffer_on_host(numBuffs, numSVM, numImgs, buffer_ptrs, image_ptrs, kernInfo, dupe, evt);

        if(retEvt)
        {
            cl_context ctx;
            clGetCommandQueueInfo(cmdQueue, CL_QUEUE_CONTEXT, sizeof(cl_context), &ctx, 0);
            *retEvt = clCreateUserEvent(ctx, NULL);
            clSetUserEventStatus(*retEvt, CL_COMPLETE);
        }
    }

    if(buffer_ptrs != NULL)
        free(buffer_ptrs);
    if(image_ptrs != NULL)
        free(image_ptrs);
}

/*
 * deletes the kernel, and internally created objects
 *  Arguments:
 *      cl_kernel del_kern
 *          kernel to be deleted
 *      uint32_t *dupe
 *          list of duplicate arguments
 *          kernel argument represented by index i is unique or first if the value at index i is equal to i
 */
void delPoisonKernel(cl_kernel del_kern, uint32_t *dupe)
{
    cl_int cl_err;
    uint32_t i, nargs;
    kernel_info *del_kern_info;
    kernel_arg *del_kern_arg;

    if(dupe == NULL){ exit(-1); }

    cl_err = clGetKernelInfo(del_kern, CL_KERNEL_NUM_ARGS, sizeof(nargs), &nargs, 0);
    check_cl_error(__FILE__, __LINE__, cl_err);

    del_kern_info = kinfo_find(get_kern_list(), del_kern);

    // delete arguments with host pointers
    // temporary extended copies of the original buffers were made for these
    for(i = 0; i < nargs; i++)
    {
        if(dupe[i] == i)
        {
            cl_memobj *m1 = NULL;
            del_kern_arg = karg_remove(&del_kern_info->arg_list, i);
            if(del_kern_arg != NULL)
            {
                m1 = del_kern_arg->buffer; //is it a cl_mem?
            }
            if(m1 != NULL)
            {
                if(m1->detector_internal_buffer)
                {
                    if(m1->host_ptr != NULL)
                    {
                        free(m1->host_ptr);
                        m1->host_ptr = NULL;
                    }

                    cl_err = clReleaseMemObject(m1->handle);
                    check_cl_error(__FILE__, __LINE__, cl_err);
                }
                karg_delete(del_kern_arg);
            }
        }
    }

    clReleaseKernel(del_kern);
}

/*
 * copy kernel and program
 *
 */
cl_kernel kernelDuplicate(cl_kernel kernel)
{
    cl_int cl_err;
    cl_kernel new_kernel;
    cl_program program;
    char *kernel_name;
    size_t name_length;

    cl_err = clGetKernelInfo(kernel, CL_KERNEL_PROGRAM, sizeof(cl_program), &program, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);

    cl_err = clGetKernelInfo(kernel, CL_KERNEL_FUNCTION_NAME, 0, NULL, &name_length);
    check_cl_error(__FILE__, __LINE__, cl_err);
    kernel_name = (char *)malloc(sizeof(char) * name_length);

    cl_err = clGetKernelInfo(kernel, CL_KERNEL_FUNCTION_NAME, name_length, kernel_name, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);

    new_kernel = clCreateKernel(program, kernel_name, &cl_err);
    check_cl_error(__FILE__, __LINE__, cl_err);

    if (kernel_name != NULL)
        free(kernel_name);

    return new_kernel;
}

/*
 * clone kernel replacing arguments using pre-allocated memory with internally allocated buffers
 *
 */
cl_kernel createPoisonedKernel(cl_command_queue command_queue,
        cl_kernel kernel,
        uint32_t *dupe)
{
    uint32_t i, nargs;
    cl_int cl_err;
    kernel_info *old_kern_info;
    kernel_arg *old_arg_info;

    // NOTE: Don't demangle the kernel names here. We need them the same for when we
    // call clCreateKernel below.

    old_kern_info = kinfo_find(get_kern_list(), kernel);

    cl_err = clGetKernelInfo(kernel, CL_KERNEL_NUM_ARGS, sizeof(nargs), &nargs, 0);
    check_cl_error(__FILE__, __LINE__, cl_err);

    //does the original kernel have any host pointer argumrnts
    uint8_t needInternalBuffer = 0;
    for(i = 0; i < nargs; i++)
    {
        old_arg_info = karg_find(old_kern_info->arg_list, i);
        cl_memobj *old_buffer_info = old_arg_info->buffer;
        if(old_buffer_info != NULL)
        {
            if( !old_buffer_info->has_canary )
            {
                needInternalBuffer = 1;
                break;
            }
        }
    }

    //no need to clone kernel if this is true
    if(!needInternalBuffer)
    {
        return kernel;
    }

    cl_kernel new_kernel;
    new_kernel = kernelDuplicate(kernel);

    //replace arguments using host pointers
    for(i = 0; i < nargs && dupe != NULL; i++)
    {

        if(dupe[i] == i)
        {
            old_arg_info = karg_find(old_kern_info->arg_list, i);
            cl_memobj *old_buffer_info = old_arg_info->buffer;
            cl_svm_memobj *svmBuff = old_arg_info->svm_buffer;

            if(old_buffer_info == NULL && svmBuff == NULL)
            { //make a deep copy of the value, it's not a buffer
                cl_err = clSetKernelArg(new_kernel, i, old_arg_info->size, old_arg_info->value);
                check_cl_error(__FILE__, __LINE__, cl_err);
            }
            else if(svmBuff != NULL)
            {
#ifdef CL_VERSION_2_0
                cl_err = clSetKernelArgSVMPointer(new_kernel, i, svmBuff->handle);
                check_cl_error(__FILE__, __LINE__, cl_err);
#endif
            }
            else if(!old_buffer_info->has_canary)
            {
                cl_mem new_buffer;
                cl_mem_flags flags;
                cl_context new_ctx;

                new_ctx = old_buffer_info->context;
                flags = old_buffer_info->flags;

                flags = flags & ~CL_MEM_USE_HOST_PTR & ~CL_MEM_COPY_HOST_PTR;
                flags = flags & ~CL_MEM_ALLOC_HOST_PTR;

                if(old_buffer_info->is_image)
                {
#ifdef CL_VERSION_1_2
                    cl_image_desc desc;
                    desc.image_type = old_buffer_info->image_desc.image_type;
                    desc.image_width = old_buffer_info->image_desc.image_width;
                    desc.image_height = old_buffer_info->image_desc.image_height;
                    desc.image_depth = old_buffer_info->image_desc.image_depth;
                    desc.image_array_size = old_buffer_info->image_desc.image_array_size;
                    desc.image_row_pitch = old_buffer_info->image_desc.image_row_pitch;
                    desc.image_slice_pitch = old_buffer_info->image_desc.image_slice_pitch;
                    desc.num_mip_levels = old_buffer_info->image_desc.num_mip_levels;
                    desc.num_samples = old_buffer_info->image_desc.num_samples;
                    desc.buffer = NULL;
                    new_buffer = clCreateImage(new_ctx, flags, &old_buffer_info->image_format, &desc, NULL, &cl_err);
#else
                    if(old_buffer_info->image_desc.image_type == CL_MEM_OBJECT_IMAGE3D)
                    {
                        new_buffer = clCreateImage3D(new_ctx, flags,
                                &old_buffer_info->image_format,
                                old_buffer_info->image_desc.image_width,
                                old_buffer_info->image_desc.image_height,
                                old_buffer_info->image_desc.image_depth,
                                old_buffer_info->image_desc.image_row_pitch,
                                old_buffer_info->image_desc.image_slice_pitch,
                                old_buffer_info->host_ptr, &cl_err);
                    }
                    else
                    {
                        new_buffer = clCreateImage2D(new_ctx, flags,
                                &old_buffer_info->image_format,
                                old_buffer_info->image_desc.image_width,
                                old_buffer_info->image_desc.image_height,
                                old_buffer_info->image_desc.image_row_pitch,
                                old_buffer_info->host_ptr, &cl_err);
                    }
#endif // CL_VERSION_1_2
                }
                else
                    new_buffer = clCreateBuffer(new_ctx, flags, old_buffer_info->size, NULL, &cl_err);

                check_cl_error(__FILE__, __LINE__, cl_err);

                cl_memobj* newBuffInfo = cl_mem_find(get_cl_mem_alloc(), new_buffer);
                newBuffInfo->detector_internal_buffer = 1;

                if(old_buffer_info->is_image)
                {
                    size_t origin[] = {0,0,0}, region[3];
                    region[0] = old_buffer_info->image_desc.image_width;
                    region[1] = old_buffer_info->image_desc.image_height;
                    region[2] = old_buffer_info->image_desc.image_depth;
                    if(old_buffer_info->image_desc.image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY)
                        region[1] = old_buffer_info->image_desc.image_array_size;
                    else if(old_buffer_info->image_desc.image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY)
                        region[2] = old_buffer_info->image_desc.image_array_size;

                    cl_image_copy(command_queue, old_buffer_info->handle, new_buffer, origin, origin, region, 0, 0, 0);
                }
                else
                    cl_buffer_copy(command_queue, old_buffer_info->handle, new_buffer, 0, 0, old_buffer_info->size, 0, 0, 0);

                cl_err = clSetKernelArg(new_kernel, i, sizeof(cl_mem), &new_buffer);
                check_cl_error(__FILE__, __LINE__, cl_err);
            }
            else
            {
                cl_err = clSetKernelArg(new_kernel, i, sizeof(cl_mem), &old_buffer_info->handle);
                check_cl_error(__FILE__, __LINE__, cl_err);
            }
        }
        else
        {
            kernel_info *new_kern_info;
            kernel_arg *arg_info;
            new_kern_info = kinfo_find(get_kern_list(), new_kernel);
            arg_info = karg_find(new_kern_info->arg_list, dupe[i]);
            cl_err = clSetKernelArg(new_kernel, i, arg_info->size, arg_info->value);
            check_cl_error(__FILE__, __LINE__, cl_err);
        }

    }

    clFinish(command_queue);

    return new_kernel;
}


/*
 * copies contents of buffer objects from one kernel to another
 * does not create buffers, they must already exist
 *
 *      cl_kernel to
 *      cl_kernel from
 *      uint32_t * dupe
 *          list of duplicate arguments
 *      cl_command_queue command queue
 *          isn't necessary, but will speed up the process
 *
 * copies using size of shortest buffer
 */
void copyKernelBuffers(cl_kernel to, cl_kernel from,
        uint32_t * dupe, cl_command_queue command_queue)
{
    cl_int cl_err;
    int i, nargs;
    uint32_t temp_nargs;
    kernel_info *from_kern_info, *to_kern_info;
    cl_err = clGetKernelInfo(from, CL_KERNEL_NUM_ARGS, sizeof(temp_nargs), &temp_nargs, 0);
    check_cl_error(__FILE__, __LINE__, cl_err);

    assert(temp_nargs < INT_MAX);
    nargs = temp_nargs;
    from_kern_info = kinfo_find(get_kern_list(), from);
    to_kern_info = kinfo_find(get_kern_list(), to);
    assert(from_kern_info != NULL);
    assert(to_kern_info != NULL);

    int nevts = 0;
    cl_event *events;
    events = calloc(sizeof(cl_event), nargs);

    cl_err = clFinish(command_queue);
    check_cl_error(__FILE__, __LINE__, cl_err);
    for(i = 0; i < nargs; i++)
    {
        kernel_arg *from_arg_info, *to_arg_info;
        from_arg_info = karg_find(from_kern_info->arg_list, i);
        to_arg_info = karg_find(to_kern_info->arg_list, i);
        assert(from_arg_info != NULL);
        assert(to_arg_info != NULL);

        if(dupe != NULL && (int)dupe[i] == i)
        {
            cl_memobj *m1 = from_arg_info->buffer; //is it a cl_mem?
            cl_memobj *m2 = to_arg_info->buffer; //is it a cl_mem?
            if(m1 && m2 && m1 != m2)
            {
                if(m1->is_image)
                {
                    size_t origin[] = {0, 0, 0}, region[3];
                    uint32_t w1, w2, h1, h2, d1, d2;
                    w1 = m1->image_desc.image_width;
                    w2 = m2->image_desc.image_width;
                    h1 = m1->image_desc.image_height;
                    h2 = m2->image_desc.image_height;
                    d1 = m1->image_desc.image_depth;
                    d2 = m2->image_desc.image_depth;
                    if(m2->image_desc.image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY)
                    {
                        h1 = m1->image_desc.image_array_size;
                        h2 = m2->image_desc.image_array_size;
                    }
                    else if(m2->image_desc.image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY)
                    {
                        d1 = m1->image_desc.image_array_size;
                        d2 = m2->image_desc.image_array_size;
                    }
                    region[0] = (w1 > w2) ? w2 : w1;
                    region[1] = (h1 > h2) ? h2 : h1;
                    region[2] = (d1 > d2) ? d2 : d1;

                    cl_image_copy(command_queue, m1->handle, m2->handle, origin, origin, region, 0, 0, &events[nevts]);
                }
                else
                {
                    size_t size = m1->size;
                    if(size > m2->size)
                        size = m2->size;
                    cl_buffer_copy(command_queue, m1->handle, m2->handle, 0, 0, size, 0, 0, &events[nevts]);
                }
                nevts++;
            }
        }
    }

    cl_err = clWaitForEvents(nevts, events);
    check_cl_error(__FILE__, __LINE__, cl_err);
    free(events);
}

