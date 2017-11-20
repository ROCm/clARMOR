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


// When cleaning up an image's canaries, we need to temporarily allocate the
// data that we will copy onto its old canaries. Because our image fill
// commands can run asynchronously, we need to have a callback function
// that will free these temporary allocations only after the canaries have
// been filled. Thus the struct and callback function you see below.
typedef struct clean_these_ptrs {
    void *first;
    void *second;
    void *third;
} clean_these_ptrs_t;

static void free_temp_canaries(cl_event event, cl_int status, void* to_free)
{
    (void)event;
    (void)status;
    clean_these_ptrs_t *clean_me = (clean_these_ptrs_t*)to_free;
    if (clean_me != NULL)
    {
        if (clean_me->first != NULL)
            free(clean_me->first);
        if (clean_me->second != NULL)
            free(clean_me->second);
        if (clean_me->third != NULL)
            free(clean_me->third);
        free(clean_me);
    }
}

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

    //largest size for a color
    const uint32_t len = 16;

    void *first_poison = NULL;
    void *second_poison = NULL;
    void *third_poison = NULL;

    // End of each row
    first_poison = malloc(len * IMAGE_POISON_WIDTH);
    memset(first_poison, POISON_FILL, len * IMAGE_POISON_WIDTH);

    // Each full-row canary in a 2d plane
    if(j_lim > 1 && i_lim > 0)
    {
        second_poison = malloc(len * i_lim * IMAGE_POISON_HEIGHT);
        memset(second_poison, POISON_FILL, len * i_lim * IMAGE_POISON_HEIGHT);
    }

    // Each full-plane canary for a 3d image
    if(k_lim > 1 && j_lim > 0 && i_lim > 0)
    {
        third_poison = malloc(len * i_lim * j_lim * IMAGE_POISON_DEPTH);
        memset(third_poison, POISON_FILL, len * i_lim * j_lim * IMAGE_POISON_DEPTH);
    }

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
                cl_err = clEnqueueWriteImage(cmdQueue, img->handle,
                        CL_NON_BLOCKING, origin, region,
                        0, 0, first_poison,
                        numEvts, evt, NULL);
                check_cl_error(__FILE__, __LINE__, cl_err);
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
            cl_err = clEnqueueWriteImage(cmdQueue, img->handle,
                    CL_NON_BLOCKING, origin, region,
                    0, 0, second_poison,
                    numEvts, evt, NULL);
            check_cl_error(__FILE__, __LINE__, cl_err);
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
        cl_err = clEnqueueWriteImage(cmdQueue, img->handle,
                CL_NON_BLOCKING, origin, region,
                0, 0, third_poison,
                numEvts, evt, NULL);
        check_cl_error(__FILE__, __LINE__, cl_err);
    }

    cl_event finishWrites;
    //wait for command queue to empty
#ifdef CL_VERSION_1_2
    cl_err = clEnqueueMarkerWithWaitList(cmdQueue, 0, NULL, &finishWrites);
#else
    cl_err = clEnqueueMarker(cmdQueue, &finishWrites);
#endif
    check_cl_error(__FILE__, __LINE__, cl_err);

    clean_these_ptrs_t *clean_me = malloc(sizeof(clean_these_ptrs_t));
    clean_me->first = first_poison;
    clean_me->second = second_poison;
    clean_me->third = third_poison;

    cl_err = clSetEventCallback(finishWrites, CL_COMPLETE, free_temp_canaries, clean_me);
    check_cl_error(__FILE__, __LINE__, cl_err);

    if(retEvt)
        *retEvt = finishWrites;
}

/*
 * will update canary region with POISON_FILL
 * works for cl_mem (buffers and images) and svm
 *
 * will perform context converions if the buffer belongs to a different context than the command queue / events
 * return event is in the context of the command queue
 *
 */
void mendCanaryRegion(cl_command_queue cmdQueue, void * const buffer, cl_bool blocking,
                        uint32_t numEvts, const cl_event *evt, cl_event *retEvt)
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
            cl_int cl_err;
            cl_event region_event[2];
            uint32_t offset = m1->size;
#ifdef UNDERFLOW_CHECK
            offset += POISON_FILL_LENGTH;
#endif

#ifdef CL_VERSION_1_2
            cl_err = clEnqueueFillBuffer(fillQueue, m1->main_buff, &poisonFill_8b,
                    sizeof(uint8_t), offset, POISON_FILL_LENGTH, numEvts, waits, &region_event[0]);
            check_cl_error(__FILE__, __LINE__, cl_err);
#ifdef UNDERFLOW_CHECK
            cl_err = clEnqueueFillBuffer(fillQueue, m1->main_buff, &poisonFill_8b,
                    sizeof(uint8_t), 0, POISON_FILL_LENGTH, numEvts, waits, &region_event[1]);
            check_cl_error(__FILE__, __LINE__, cl_err);
#endif

            clEnqueueMarkerWithWaitList(fillQueue, POISON_REGIONS, region_event, &finish);
#else
            char *poison_data = malloc(POISON_FILL_LENGTH);
            memset(poison_data, poisonFill_8b, POISON_FILL_LENGTH);

            cl_err = clEnqueueWriteBuffer(fillQueue, m1->main_buff,
                    CL_NON_BLOCKING, offset, POISON_FILL_LENGTH, poison_data,
                    numEvts, waits, &region_event[0]);
            check_cl_error(__FILE__, __LINE__, cl_err);
#ifdef UNDERFLOW_CHECK
            cl_err = clEnqueueWriteBuffer(fillQueue, m1->main_buff,
                    CL_NON_BLOCKING, 0, POISON_FILL_LENGTH, poison_data,
                    numEvts, waits, &region_event[1]);
            check_cl_error(__FILE__, __LINE__, cl_err);
#endif

            clWaitForEvents(POISON_REGIONS, region_event);
            free(poison_data);

            clEnqueueMarker(fillQueue, &finish);
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

            cl_int cl_err;
            cl_event region_event[2];
            uint32_t offset = m2->size;
#ifdef UNDERFLOW_CHECK
            offset += POISON_FILL_LENGTH;
#endif

            cl_err = clEnqueueSVMMemFill(fillQueue, (char*)m2->main_buff + offset,
                    &poisonFill_8b, sizeof(uint8_t), POISON_FILL_LENGTH, numEvts, waits, &region_event[0]);
            check_cl_error(__FILE__, __LINE__, cl_err);
#ifdef UNDERFLOW_CHECK
            cl_err = clEnqueueSVMMemFill(fillQueue, (char*)m2->main_buff,
                    &poisonFill_8b, sizeof(uint8_t), POISON_FILL_LENGTH, numEvts, waits, &region_event[1]);
            check_cl_error(__FILE__, __LINE__, cl_err);
#endif

            clEnqueueMarkerWithWaitList(fillQueue, POISON_REGIONS, region_event, &finish);
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

