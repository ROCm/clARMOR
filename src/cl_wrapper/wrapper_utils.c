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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ********************************************************************************/

#include <string.h>
#include <pthread.h>

#include "detector_defines.h"
#include "cl_err.h"
#include "meta_data_lists/cl_memory_lists.h"
#include "meta_data_lists/cl_workaround_lists.h"
#include "util_functions.h"
#include "overflow_error.h"

#include "wrapper_utils.h"

extern pthread_mutex_t command_queue_cache_lock;

static __thread uint8_t canaryAccess = 0;

int is_nvidia_platform(cl_context context)
{
    cl_int cl_err;
    cl_device_id device;
    cl_platform_id platform;
    size_t platform_name_len = 0;
    char *platform_name;

    cl_err = clGetContextInfo(context, CL_CONTEXT_DEVICES,
        sizeof(cl_device_id), &device, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clGetDeviceInfo(device, CL_DEVICE_PLATFORM,
        sizeof(cl_platform_id), &platform, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);

    cl_err = clGetPlatformInfo(platform, CL_PLATFORM_NAME, 0, NULL,
            &platform_name_len);
    check_cl_error(__FILE__, __LINE__, cl_err);

    platform_name = calloc(platform_name_len, sizeof(char));

    cl_err = clGetPlatformInfo(platform, CL_PLATFORM_NAME,
        platform_name_len, platform_name, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);

    char *found;
    found = strstr(platform_name, "NVIDIA");
    int ret = (found) ? 1 : 0;
    free(platform_name);

    return ret;
}

void allowCanaryAccess(void)
{
    canaryAccess = 1;
}

void disallowCanaryAccess(void)
{
    canaryAccess = 0;
}

uint8_t canaryAccessAllowed(void)
{
    return canaryAccess;
}


int getCommandQueueForContext(cl_context context, cl_command_queue *cmdQueue)
{
    cl_int cl_err;
    int ret = 1;

    pthread_mutex_lock(&command_queue_cache_lock);
    commandQueueCache *findme = commandQueueCache_find(get_cmd_queue_cache(), context);
    pthread_mutex_unlock(&command_queue_cache_lock);

    if (findme == NULL)
    {
        uint32_t num_dev;
        cl_err = clGetContextInfo(context, CL_CONTEXT_NUM_DEVICES, sizeof(uint32_t), &num_dev, NULL);
        check_cl_error(__FILE__, __LINE__, cl_err);

        cl_device_id *devices;
        devices = malloc(sizeof(cl_device_id) * num_dev);

        cl_err = clGetContextInfo(context, CL_CONTEXT_DEVICES, sizeof(cl_device_id) * num_dev, devices, NULL);
        check_cl_error(__FILE__, __LINE__, cl_err);

        cl_device_id device = devices[0];

        *cmdQueue = clCreateCommandQueue(context, device, 0, &cl_err);
        check_cl_error(__FILE__, __LINE__, cl_err);
        ret = 0;

        free(devices);
    }
    else
        *cmdQueue = findme->cached_queue;

    return ret;
}

void allocFlatImageCopy(char **copy_ptr_p, char *host_ptr, cl_memobj *img)
{
    *copy_ptr_p = (char*)calloc(img->size, 1);
    char *copy_ptr = *copy_ptr_p;

    uint32_t j, k;
    uint32_t i_lim, j_lim, k_lim;
    uint32_t i_host_len, j_host_len, k_host_len;
    i_lim = img->image_desc.image_width;
    j_lim = img->image_desc.image_height;
    k_lim = img->image_desc.image_depth;
    if(img->image_desc.image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY)
        j_lim = img->image_desc.image_array_size;
    else if(img->image_desc.image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY)
        k_lim = img->image_desc.image_array_size;

    i_host_len = (i_lim == 1) ? 1 : i_lim - IMAGE_POISON_WIDTH;
    j_host_len = (j_lim == 1) ? 1 : j_lim - IMAGE_POISON_HEIGHT;
    k_host_len = (k_lim == 1) ? 1 : k_lim - IMAGE_POISON_DEPTH;

    uint32_t i_host_pitch = i_host_len;
    if (img->image_desc.image_row_pitch != 0)
        i_host_pitch = img->image_desc.image_row_pitch;
    uint32_t j_host_pitch = j_host_len;
    if (img->image_desc.image_slice_pitch != 0)
        j_host_pitch = img->image_desc.image_slice_pitch;

    size_t dataSize;
    dataSize = getImageDataSize(&img->image_format);
    i_lim *= dataSize;
    i_host_len *= dataSize;
    // pitch does not get increased by dataSize. Pitch is calculated
    // in bytes in the OpenCL API.

    for(k=0; k < k_host_len; k++)
    {
        for(j=0; j < j_host_len; j++)
        {
            if(img->flags & CL_MEM_COPY_HOST_PTR)
            {
                char *data, *from_host;
                data = copy_ptr + k*j_lim*i_lim + j*i_lim;
                from_host = host_ptr + k*j_host_pitch*i_host_pitch + j*i_host_pitch;
                memcpy(data, from_host, i_host_len);
            }

            char* canary_x = copy_ptr + k*j_lim*i_lim + j*i_lim + i_host_len;
            memset(canary_x, POISON_FILL, IMAGE_POISON_WIDTH * dataSize);
        }

        if(j_lim > 1)
        {
            char* canary_y = copy_ptr + k*j_lim*i_lim + j_host_len*i_lim;
            memset(canary_y, POISON_FILL, IMAGE_POISON_HEIGHT * i_lim);
        }
    }

    if(k_lim > 1)
    {
        char* canary_z = copy_ptr + k_host_len*j_lim*i_lim;
        memset(canary_z, POISON_FILL, IMAGE_POISON_DEPTH * j_lim * i_lim);
    }
}

#ifdef CL_VERSION_2_0
cl_queue_properties *add_profiling(const cl_queue_properties *properties)
{
    // We want to add profiling to the properties of the command queue.
    // Search through the existing properties (if there are any) and see
    // if we need to either alloacate a CL_QUEUE_PROPERTIES to hold the
    // profiling request, or whether we can just or it in.
    cl_queue_properties *fake_prop = NULL;

    if (properties != NULL)
    {
        // If CL_QUEUE_PROPERTIES exists, we will check if it has
        // CL_QUEUE_PROFILING_ENABLE. If not, this variable will tell us
        // where to add it.
        int add_profiling_here = -1;
        int found_queue_properties = 0;

        // i will contain the current size of the array when we're done.
        int i = 0;
        while (properties[i] != 0)
        {
            if (properties[i] == CL_QUEUE_PROPERTIES)
            {
                found_queue_properties = 1;
                if ( !(properties[i+1] & CL_QUEUE_PROFILING_ENABLE) )
                    add_profiling_here = i+1;
            }
            i += 2;
        }

        if (!found_queue_properties)
            fake_prop = calloc((i+3), sizeof(cl_queue_properties));
        else
            fake_prop = calloc((i+1), sizeof(cl_queue_properties));
        i = 0;
        while (properties[i] != 0)
        {
            // Doing this copy two-at-a-time because, while the queue property
            // (first entry) may be non-zero, the value may be zero, yet there
            // may be another legal property after it. e.g.,
            // {CL_QUEUE_PROPERTIES, 0, CL_QUEUE_SIZE, ...} is legal. Don't
            // want to break out of the while loop when we hit the middle 0.
            fake_prop[i] = properties[i];
            fake_prop[i+1] = properties[i+1];
            i += 2;
        }
        if (!found_queue_properties)
        {
            // Add CL_QUEUE_PROPERTIES and profiling
            fake_prop[i] = CL_QUEUE_PROPERTIES;
            fake_prop[i+1] = CL_QUEUE_PROFILING_ENABLE;
        }
        else if (add_profiling_here >= 0)
            fake_prop[add_profiling_here] =
                properties[add_profiling_here] | CL_QUEUE_PROFILING_ENABLE;
    }
    else
    {
        fake_prop = malloc(3 * sizeof(cl_queue_properties));
        fake_prop[0] = CL_QUEUE_PROPERTIES;
        fake_prop[1] = CL_QUEUE_PROFILING_ENABLE;
        fake_prop[2] = 0;
    }
    return fake_prop;
}

#endif //CL_VERSION_2_0

uint8_t apiBufferOverflowCheck(char * const func, cl_mem buffer, size_t offset, size_t size)
{
    if(get_disable_api_check_envvar())
        return 0;

    uint8_t ret = 0;
    cl_memobj *m1 = cl_mem_find(get_cl_mem_alloc(), buffer);
    if(m1)
    {
        uint64_t data_size;

        data_size = m1->size;

        if(offset + size > data_size)
        {
            int64_t bad_byte;
            if(offset > data_size)
                bad_byte = offset - data_size;
            else
                bad_byte = 0;

            apiOverflowError(func, buffer, bad_byte);
            optionalKillOnOverflow(get_exitcode_envvar(), 0);
            ret = 1;
        }
    }
    return ret;
}

uint8_t apiBufferRectOverflowCheck(char * const func, cl_mem buffer, const size_t * buffer_offset, const size_t * region, size_t row_pitch, size_t slice_pitch)
{
    if(get_disable_api_check_envvar())
        return 0;

    uint8_t ret = 0;
    cl_memobj *m1 = cl_mem_find(get_cl_mem_alloc(), buffer);
    if(m1)
    {
        uint64_t b_off, b_end;

        b_off = buffer_offset[0];
        b_end = region[0];

        if(row_pitch == 0)
            row_pitch = region[0];
        b_off += buffer_offset[1] * row_pitch;
        b_end *= region[1];

        if(slice_pitch == 0)
            slice_pitch = row_pitch * region[1];
        b_off += buffer_offset[2] * slice_pitch;
        b_end *= region[2];

        b_end += b_off;

        if(b_end > m1->size)
        {
            int64_t bad_byte;
            if(b_off > m1->size)
                bad_byte = b_off - m1->size;
            else
                bad_byte = 0;

            apiOverflowError(func, buffer, bad_byte);
            optionalKillOnOverflow(get_exitcode_envvar(), 0);
            ret = 1;
        }
    }
    return ret;
}

uint8_t apiImageOverflowCheck(char * const func, cl_mem image, const size_t * origin, const size_t * region)
{
    if(get_disable_api_check_envvar())
        return 0;

    uint8_t ret = 0;
    cl_memobj *m1 = cl_mem_find(get_cl_mem_alloc(), image);
    if(m1 && m1->is_image)
    {
        uint64_t w, h, d, a;
        w = m1->image_desc.image_width;
        h = m1->image_desc.image_height;
        d = m1->image_desc.image_depth;
        a = m1->image_desc.image_array_size;

        if(m1->image_desc.image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY)
            h = a;
        else if(m1->image_desc.image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY)
            d = a;

        uint64_t tLen[3];
        if( !m1->has_canary || canaryAccessAllowed() )
        {
            tLen[0] = 0;
            tLen[1] = 0;
            tLen[2] = 0;
        }
        else
        {
            tLen[0] = IMAGE_POISON_WIDTH;
            tLen[1] = IMAGE_POISON_HEIGHT;
            tLen[2] = IMAGE_POISON_DEPTH;
        }

        if(origin[2] + region[2] > d - tLen[2])
        {
            uint64_t bad_index;
            if(origin[2] > d - tLen[2])
                bad_index = origin[2] - (d - tLen[2]);
            else
                bad_index = 0;

            apiImageOverflowError(func, image, -1, -1, bad_index);
            optionalKillOnOverflow(get_exitcode_envvar(), 0);
            ret = 1;
        }
        else if(origin[1] + region[1] > h - tLen[1])
        {
            uint64_t bad_index;
            if(origin[1] > h - tLen[1])
                bad_index = origin[1] - (h - tLen[1]);
            else
                bad_index = 0;

            apiImageOverflowError(func, image, -1, bad_index, origin[2]);
            optionalKillOnOverflow(get_exitcode_envvar(), 0);
            ret = 1;
        }
        else if(origin[0] + region[0] > w - tLen[0])
        {
            uint64_t bad_index;
            if(origin[0] > w - tLen[0])
                bad_index = origin[0] - (w - tLen[0]);
            else
                bad_index = 0;

            apiImageOverflowError(func, image, bad_index, origin[1], origin[2]);
            optionalKillOnOverflow(get_exitcode_envvar(), 0);
            ret = 1;
        }
    }
    return ret;
}

void expandLastDimForFill(cl_image_desc *desc)
{
    cl_mem_object_type imageType = desc->image_type;
    switch(imageType)
    {
        case CL_MEM_OBJECT_IMAGE1D:
            desc->image_width += IMAGE_POISON_WIDTH;
            break;
        case CL_MEM_OBJECT_IMAGE1D_BUFFER:
            desc->image_width += IMAGE_POISON_WIDTH;
            break;
        case CL_MEM_OBJECT_IMAGE1D_ARRAY:
            desc->image_width += IMAGE_POISON_WIDTH;
            desc->image_array_size += IMAGE_POISON_HEIGHT;
            break;
        case CL_MEM_OBJECT_IMAGE2D:
            desc->image_width += IMAGE_POISON_WIDTH;
            desc->image_height += IMAGE_POISON_HEIGHT;
            break;
        case CL_MEM_OBJECT_IMAGE2D_ARRAY:
            desc->image_width += IMAGE_POISON_WIDTH;
            desc->image_height += IMAGE_POISON_HEIGHT;
            desc->image_array_size += IMAGE_POISON_DEPTH;
            break;
        case CL_MEM_OBJECT_IMAGE3D:
            desc->image_width += IMAGE_POISON_WIDTH;
            desc->image_height += IMAGE_POISON_HEIGHT;
            desc->image_depth += IMAGE_POISON_DEPTH;
            break;
        default:
            det_fprintf(stderr, "failed to find image type");
            exit(-1);
    }
}
