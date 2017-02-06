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

#include "cl_err.h"

#include "universal_event.h"

static void universalEvent(cl_event event, cl_int status, void* userEvent)
{
    if(event || status){}

    cl_event *u_event = userEvent;
    clSetUserEventStatus(*u_event, CL_COMPLETE);
    free(u_event);
}

void convertEvents(cl_context ctx, uint32_t numEvts, cl_event *evtList)
{
    uint32_t i;
    for(i = 0; i < numEvts; i++)
    {
        cl_context evtCtx;
        clGetEventInfo(evtList[i], CL_EVENT_CONTEXT, sizeof(cl_context), &evtCtx, NULL);

        if(ctx != evtCtx)
        {
            cl_int cl_err;
            cl_event newEvent, *eventCopy;

            eventCopy = calloc(sizeof(cl_event), 1);

            newEvent = clCreateUserEvent(ctx, NULL);
            *eventCopy = newEvent;

            //eventCopy freed in callback
            cl_err = clSetEventCallback(evtList[i], CL_COMPLETE, universalEvent, eventCopy);
            check_cl_error(__FILE__, __LINE__, cl_err);

            evtList[i] = newEvent;
        }
    }
}

void releaseEvents(uint32_t numEvts, const cl_event *evtList)
{
    uint32_t i;
    for(i = 0; i < numEvts; i++)
    {
        cl_int cl_err;
        cl_err = clReleaseEvent(evtList[i]);
        check_cl_error(__FILE__, __LINE__, cl_err);
    }
}
