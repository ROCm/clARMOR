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
#include "cpu_check_utils.h"
#include "meta_data_lists/cl_memory_lists.h"
#include "wrapper_utils.h"

#include "cpu_check_cl_image.h"

static void copy_end_of_row(cl_context kern_ctx, cl_command_queue cmd_queue,
        cl_memobj *img, char *canary, uint32_t i_dat, uint32_t j_dat,
        uint32_t i_lim, uint32_t j, uint32_t k, size_t data_size,
        const cl_event * in_evt, cl_event * out_evt)
{
    if(i_lim > 1)
    {
        size_t origin[3], region[3];
        //copy end of row
        origin[0] = i_dat;
        origin[1] = j;
        origin[2] = k;
        region[0] = IMAGE_POISON_WIDTH;
        region[1] = 1;
        region[2] = 1;
        //index to end of row
        char *segment = canary +
            (k*(j_dat*IMAGE_POISON_WIDTH + IMAGE_POISON_HEIGHT*i_lim) +
             j*IMAGE_POISON_WIDTH)*data_size;
        cl_int cl_err = clEnqueueReadImage(cmd_queue, img->handle,
                CL_NON_BLOCKING, origin, region, 0, 0, (void*)segment, 1,
                in_evt, out_evt);
        check_cl_error(__FILE__, __LINE__, cl_err);
    }
    else
        *out_evt = create_complete_user_event(kern_ctx);
}

static void copy_end_of_slice(cl_context kern_ctx, cl_command_queue cmd_queue,
        cl_memobj *img, char *canary, uint32_t j_dat, uint32_t i_lim,
        uint32_t j_lim, uint32_t k, size_t data_size, const cl_event * in_evt,
        cl_event * out_evt)
{
    if(j_lim > 1)
    {
        size_t origin[3], region[3];
        //copy end of slice
        origin[0] = 0;
        origin[1] = j_dat;
        origin[2] = k;
        region[0] = i_lim;
        region[1] = IMAGE_POISON_HEIGHT;
        region[2] = 1;
        //index to end of slice
        char *segment = (char*)canary +
            (k*IMAGE_POISON_HEIGHT*i_lim +
             (k + 1)*(j_dat*IMAGE_POISON_WIDTH))*data_size;
        cl_int cl_err = clEnqueueReadImage(cmd_queue, img->handle,
                CL_NON_BLOCKING, origin, region, 0, 0, (void*)segment, 1,
                in_evt, out_evt);
        check_cl_error(__FILE__, __LINE__, cl_err);
    }
    else
        *out_evt = create_complete_user_event(kern_ctx);
}

static void copy_canary_slice(cl_context kern_ctx, cl_command_queue cmd_queue,
        cl_memobj *img, char *canary, uint32_t j_dat, uint32_t k_dat, uint32_t i_lim,
        uint32_t j_lim, uint32_t k_lim, size_t data_size, const cl_event * in_evt,
        cl_event * out_evt)
{
    if(k_lim > 1)
    {
        size_t origin[3], region[3];
        //copy canary slice
        origin[0] = 0;
        origin[1] = 0;
        origin[2] = k_dat;
        region[0] = i_lim;
        region[1] = j_lim;
        region[2] = IMAGE_POISON_DEPTH;
        //index to end of image
        char *segment = (char*)canary +
            (k_dat*IMAGE_POISON_HEIGHT*i_lim +
             k_dat*j_dat*IMAGE_POISON_WIDTH)*data_size;
        cl_int cl_err = clEnqueueReadImage(cmd_queue, img->handle,
                CL_NON_BLOCKING, origin, region, 0, 0, (void*)segment, 1,
                in_evt, out_evt);
        check_cl_error(__FILE__, __LINE__, cl_err);
    }
    else
        *out_evt = create_complete_user_event(kern_ctx);
}

// Reads canaries from an cl_mem that contains an image. Because images are
// multi-dimensional, this will read all of the canaries from the end of each
// dimension and put them into a flatteneded array. This also includes the
// regions at the far corner (in a 2D image, that would be the bottom-right
// region that is the "canary columns for the canary rows").
// Copies them in order of increasing dimensionality.
//      Entries->Rows->Slices
// Inputs:
//      kern_ctx:  The context for the original kernel
//      cmd_queue: The command queue that will be used to enqueue the read cmds
//      img:    a cl_memobj that describes all the important image info
//      canary_p:   This function will allocate a memory region and read all
//                  of this image's canary values into it. This region is
//                  returned through the canary_p pointer. It is the calling
//                  function's responsibility to free this region.
//      read_len:   the length of the amount of data put into this image's
//                  canary_p. This is returned to the calling function.
//      incoming_event: even from the main kernel that tells us whether it has
//                      completed. All danary reads must wait on this.
//      out_events: array that will hold all the completion events from the
//                  canary reading functions. Wait on these before checking
//                  the canaries.
static void read_image_canaries(cl_context kern_ctx,
        cl_command_queue cmd_queue, cl_memobj *img,
        void **canary_p, uint32_t *read_len, const cl_event * incoming_event,
        cl_event * out_events)
{
    uint32_t out_evt_num = 0;

    // The limits of the three dimensions of the image as it sits on the device
    uint32_t i_lim, j_lim, k_lim;

    // The limits of the three dimensions when you remove any poison bits.
    // i.e. the limits of the actual image data.
    uint32_t i_dat, j_dat, k_dat;

    //data size of each entry
    size_t data_size = getImageDataSize(&img->image_format);

    // Total size of the canary region in a linear array
    get_image_dimensions(img->image_desc, &i_lim, &j_lim, &k_lim, &i_dat,
            &j_dat, &k_dat);
    uint64_t transfer_len = get_image_canary_size(img->image_desc.image_type,
            data_size, i_lim, j_lim, j_dat, k_dat);

    // Allocate flatented array that will hold the canaries
    *canary_p = malloc(transfer_len);
    if(read_len)
        *read_len = transfer_len;
    void *canary = *canary_p;

    for(uint32_t k=0; k < k_dat; k++)
    {
        for(uint32_t j=0; j < j_dat; j++)
        {
            // For each row, read the canaries from the last few cols
            copy_end_of_row(kern_ctx, cmd_queue, img, (char*)canary, i_dat,
                    j_dat, i_lim, j, k, data_size, incoming_event,
                    &(out_events[out_evt_num]));
            out_evt_num++;
        }
        // For each slice, read the canaries from its last few rows
        copy_end_of_slice(kern_ctx, cmd_queue, img, canary, j_dat, i_lim,
                j_lim, k, data_size, incoming_event,
                &(out_events[out_evt_num]));
        out_evt_num++;
    }

    // Read the last few slices, which are entirely canaries
    copy_canary_slice(kern_ctx, cmd_queue, img, canary, j_dat, k_dat, i_lim,
            j_lim, k_lim, data_size, incoming_event,
            &(out_events[out_evt_num]));
    out_evt_num++;
}

static uint32_t number_of_image_reads(uint32_t j_dat, uint32_t k_dat)
{
    return k_dat * j_dat + k_dat + 1;
}

// Find any overflows in cl_mem image objects.
// Inputs:
//      kern_info: information about the kernel that had the last opportunity
//                  to write to this buffer.
//      num_images: number of image cl_mem buffers in the array image_ptrs
//      image_ptrs: array of void* that each point to a cl_mem image
//      dupe:   array that describes which, if any, arguments to the kernel
//              are duplicates of one another. See below for full description.
//      evt:    The cl_event that tells us when the real kernel has completed,
//              so that we can start checking its canaries.
void verify_images(kernel_info *kern_info, uint32_t num_images,
        void **image_ptrs, uint32_t *dupe, const cl_event *evt)
{
    if (num_images <= 0)
        return;

    cl_context kern_ctx;
    cl_int cl_err = clGetKernelInfo(kern_info->handle, CL_KERNEL_CONTEXT,
            sizeof(cl_context), &kern_ctx, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);

    cl_command_queue cmd_queue;
    getCommandQueueForContext(kern_ctx, &cmd_queue);

    cl_event wait_evt;
    if (evt != NULL)
        wait_evt = *evt;
    else
        wait_evt = create_complete_user_event(kern_ctx);

    cl_memobj ** buffer_images = malloc(num_images * sizeof(cl_memobj *));
    void ** canaries = calloc(num_images, sizeof(void*));
    uint32_t * canary_len = malloc(num_images * sizeof(uint32_t));

    uint32_t total_to_wait = 0;
    uint32_t * total_evts_per_img = malloc(num_images * sizeof(uint32_t));

    //iterate over cl_mem images to find them and get their size
    for(uint32_t i = 0; i < num_images; i++)
    {
        cl_memobj *m1;
        m1 = cl_mem_find(get_cl_mem_alloc(), image_ptrs[i]);
        if(m1 == NULL)
        {
            det_fprintf(stderr, "failure to find cl_memobj %p.\n", image_ptrs[i]);
            exit(-1);
        }
        buffer_images[i] = m1;

        uint32_t j_dat, k_dat;
        get_image_dimensions(m1->image_desc, NULL, NULL, NULL, NULL,
                &j_dat, &k_dat);

        total_evts_per_img[i] = total_to_wait;
        total_to_wait += number_of_image_reads(j_dat, k_dat);
    }

    cl_event * image_read_events = malloc(total_to_wait * sizeof(cl_event));

    for (uint32_t i = 0; i < num_images; i++)
    {
        //read in image and set data length
        read_image_canaries(kern_ctx, cmd_queue, buffer_images[i],
                &(canaries[i]), &(canary_len[i]), &wait_evt,
                image_read_events + total_evts_per_img[i]);
    }

    cl_err = clWaitForEvents(total_to_wait, image_read_events);
    check_cl_error(__FILE__, __LINE__, cl_err);

    for (uint32_t i = 0; i < num_images; i++)
    {
        //parse through the canary data
        cpu_parse_canary(cmd_queue, canary_len[i], canaries[i], kern_info,
                buffer_images[i]->handle, dupe);
    }

    free(total_evts_per_img);
    for (uint32_t i = 0; i < num_images; i++)
        free(canaries[i]);
    for (uint32_t i = 0; i < total_to_wait; i++)
        clReleaseEvent(image_read_events[i]);
    free(image_read_events);
    free(canary_len);
    free(canaries);
    free(buffer_images);
}
