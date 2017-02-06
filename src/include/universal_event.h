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


/*! \file universal_event.h
 * Allow cl_event to be used more generically.
 */

#ifndef __UNIVERSAL_EVENT_H
#define __UNIVERSAL_EVENT_H

#include <stdint.h>
#include <CL/cl.h>

/*!
 * convert event list to the specified context
 *
 * \param ctx
 *      changed events to this context
 * \param numEvts
 *      number of events in list
 * \param evtList
 *      list of events to convert
 */
void convertEvents(cl_context ctx, uint32_t numEvts, cl_event *evtList);

/*!
 * release a cl_event list
 *
 * \param numEvts
 *      number of events in list
 * \param evtList
 *      list of events to release
 */
void releaseEvents(uint32_t numEvts, const cl_event *evtList);

#endif //__UNIVERSAL_EVENT_H
