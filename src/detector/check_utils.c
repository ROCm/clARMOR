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

#include <string.h>

#include "cl_err.h"
#include "meta_data_lists/cl_memory_lists.h"
#include "wrapper_utils.h"
#include "universal_event.h"
#include "detector_defines.h"
#include "util_functions.h"

#include "check_utils.h"

/*
 * refresh the poison canaries for an image
 *
 *
 */
void poisonFillImageCanaries(cl_command_queue cmdQueue, cl_memobj *img, uint32_t numEvts, const cl_event *evt, cl_event *retEvt)
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
    if (dataSize == 0)
        return;

    uint32_t numWriteEvents;
    cl_event *writeEvents = NULL;

    if(k_lim > 1)
        numWriteEvents = k_dat*j_dat + k_dat + 1;
    else if(j_lim > 1)
        numWriteEvents = k_dat*j_dat + k_dat;
    else
        numWriteEvents = k_dat*j_dat;

    writeEvents = malloc(sizeof(cl_event)*numWriteEvents);

    //largest size for a color
    const uint32_t len = 16;
    void *poison_ptr;
    poison_ptr = malloc(len);
    memset(poison_ptr, POISON_FILL, len);

#ifndef CL_VERSION_1_2
    // Without OpenCL 1.2 support for clEnqueueFillImage(), we fall back to
    // synchronously filling in the image using clEnqueueWriteBufferRect.
    // We do this synchronously because we need to alloc and dealloc the
    // host-side buffer that we will copy out of.
    // This is slower, but it only affects older systems.
    if (numEvts > 0)
    {
        cl_err = clWaitForEvents(numEvts, evt);
        check_cl_error(__FILE__, __LINE__, cl_err);
    }
    char * poison_data;
#endif

    uint32_t evt_i = 0;
    for(k=0; k < k_dat; k++)
    {
        if(i_lim > 1)
        {
            for(j=0; j < j_dat; j++)
            {
                origin[0] = i_dat;
                origin[1] = j;
                origin[2] = k;
                region[0] = IMAGE_POISON_WIDTH;
                region[1] = 1;
                region[2] = 1;
#ifdef CL_VERSION_1_2
                cl_err = clEnqueueFillImage(cmdQueue, img->handle, poison_ptr, origin, region,
                                            numEvts, evt, &writeEvents[evt_i]);
                check_cl_error(__FILE__, __LINE__, cl_err);
#else
                poison_data = malloc(len * IMAGE_POISON_WIDTH);
                memset(poison_data, POISON_FILL, len * IMAGE_POISON_WIDTH);
                cl_err = clEnqueueWriteImage(cmdQueue, img->handle,
                        CL_BLOCKING, origin, region,
                        img->image_desc.image_row_pitch,
                        img->image_desc.image_slice_pitch,
                        poison_data, 0, NULL, &writeEvents[evt_i]);
                check_cl_error(__FILE__, __LINE__, cl_err);
                free(poison_data);
#endif
                evt_i++;
            }
        }

        if(j_lim > 1 && i_lim > 0)
        {
            origin[0] = 0;
            origin[1] = j_dat;
            origin[2] = k;
            region[0] = i_lim;
            region[1] = IMAGE_POISON_HEIGHT;
            region[2] = 1;
#ifdef CL_VERSION_1_2
            cl_err = clEnqueueFillImage(cmdQueue, img->handle, poison_ptr, origin, region,
                                        numEvts, evt, &writeEvents[evt_i]);
            check_cl_error(__FILE__, __LINE__, cl_err);
#else
            poison_data = malloc(len * i_lim * IMAGE_POISON_HEIGHT);
            memset(poison_data, POISON_FILL, len * i_lim * IMAGE_POISON_HEIGHT);
            cl_err = clEnqueueWriteImage(cmdQueue, img->handle,
                    CL_BLOCKING, origin, region,
                    img->image_desc.image_row_pitch,
                    img->image_desc.image_slice_pitch,
                    poison_data, 0, NULL, &writeEvents[evt_i]);
            check_cl_error(__FILE__, __LINE__, cl_err);
            free(poison_data);
#endif
            evt_i++;
        }
    }

    if(k_lim > 1 && j_lim > 0 && i_lim > 0)
    {
        origin[0] = 0;
        origin[1] = 0;
        origin[2] = k_dat;
        region[0] = i_lim;
        region[1] = j_lim;
        region[2] = IMAGE_POISON_DEPTH;
#ifdef CL_VERSION_1_2
        cl_err = clEnqueueFillImage(cmdQueue, img->handle, poison_ptr, origin, region,
                                    numEvts, evt, &writeEvents[evt_i]);
        check_cl_error(__FILE__, __LINE__, cl_err);
#else
        poison_data = malloc(len * i_lim * j_lim * IMAGE_POISON_DEPTH);
        memset(poison_data, POISON_FILL, len * i_lim * j_lim * IMAGE_POISON_DEPTH);
        cl_err = clEnqueueWriteImage(cmdQueue, img->handle,
                CL_BLOCKING, origin, region,
                img->image_desc.image_row_pitch,
                img->image_desc.image_slice_pitch,
                poison_data, 0, NULL, &writeEvents[evt_i]);
        check_cl_error(__FILE__, __LINE__, cl_err);
        free(poison_data);
#endif
        evt_i++;
    }

    cl_event finishWrites;
#ifdef CL_VERSION_1_2
    cl_err = clEnqueueMarkerWithWaitList(cmdQueue, numWriteEvents, writeEvents, &finishWrites);
#else
    (void)numWriteEvents;
    (void)writeEvents;
    cl_err = clEnqueueMarker(cmdQueue, &finishWrites);
#endif
    check_cl_error(__FILE__, __LINE__, cl_err);

    if(retEvt)
        *retEvt = finishWrites;

    if(writeEvents)
        free(writeEvents);

    free(poison_ptr);
}

/*
 * will update canary region with POISON_FILL
 * works for cl_mem (buffers and images) and svm
 *
 * will perform context converions if the buffer belongs to a different context than the command queue / events
 * return event is in the context of the command queue
 *
 */
void mendCanaryRegion(cl_command_queue cmdQueue, void * const buffer, cl_bool blocking, uint32_t numEvts, const cl_event *evt, cl_event *retEvt)
{
    cl_command_queue fillQueue;
    cl_event finish = NULL, *waits = NULL;

    if(numEvts > 0)
        waits = calloc(sizeof(cl_event), numEvts);

    uint32_t i;
    for(i = 0; i < numEvts; i++)
        waits[i] = evt[i];

    cl_memobj *m1;
    m1 = cl_mem_find(get_cl_mem_alloc(), buffer);

    if(m1 != NULL)
    {
        getCommandQueueForContext(m1->context, &fillQueue);
        convertEvents(m1->context, numEvts, waits);
        if(m1->is_image)
            poisonFillImageCanaries(fillQueue, m1, numEvts, waits, &finish);
        else
        {
#ifdef CL_VERSION_1_2
            cl_int cl_err = clEnqueueFillBuffer(fillQueue, m1->handle, &poisonFill_8b, sizeof(uint8_t), m1->size - POISON_FILL_LENGTH, POISON_FILL_LENGTH, numEvts, waits, &finish);
            check_cl_error(__FILE__, __LINE__, cl_err);
#else
            char * poison_data = malloc(POISON_FILL_LENGTH);
            memset(poison_data, poisonFill_8b, POISON_FILL_LENGTH);
            cl_int cl_err = clEnqueueWriteBuffer(fillQueue, m1->handle,
                    CL_BLOCKING, m1->size - POISON_FILL_LENGTH,
                    POISON_FILL_LENGTH, poison_data, numEvts, waits, &finish);
            check_cl_error(__FILE__, __LINE__, cl_err);
            free(poison_data);
#endif
        }
    }
#ifdef CL_VERSION_2_0
    else
    {
        cl_svm_memobj *m2;
        m2 = cl_svm_mem_find(get_cl_svm_mem_alloc(), buffer);
        if(m2 != NULL)
        {
            getCommandQueueForContext(m2->context, &fillQueue);
            convertEvents(m2->context, numEvts, waits);

            cl_int cl_err = clEnqueueSVMMemFill(fillQueue, (char*)m2->handle + m2->size - POISON_FILL_LENGTH, &poisonFill_8b, sizeof(uint8_t), POISON_FILL_LENGTH, numEvts, waits, &finish);
            check_cl_error(__FILE__, __LINE__, cl_err);
        }
    }
#endif

    if(waits)
        free(waits);

    if(blocking && finish)
        clWaitForEvents(1, &finish);
    if(retEvt)
    {
        *retEvt = finish;

        //magic this to the original command queue context
        cl_context ctx;
        clGetCommandQueueInfo(cmdQueue, CL_QUEUE_CONTEXT, sizeof(cl_context), &ctx, 0);

        convertEvents(ctx, 1, retEvt);
    }
}

