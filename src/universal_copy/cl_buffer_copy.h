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


#ifndef __CL_BUFFER_COPY_H
#define __CL_BUFFER_COPY_H

#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#include <CL/cl.h>

void straight_buffer_copy(cl_command_queue command_queue, cl_mem from,
        cl_mem to, size_t off_from, size_t off_to, size_t size,
        unsigned num_evt, const cl_event *evt_list, cl_event *out);

void new_queue_buffer_copy(cl_context context, cl_mem from, cl_mem to,
        size_t off_from, size_t off_to, size_t size, unsigned num_evt,
        const cl_event *evt_list, cl_event *out);

// Makes a copy of one cl_mem buffer into another cl_mem buffer
// This function will cover all of the complexities of copying these buffers,
// even if they were originally from different contexts.
// To get the best performance, the command_queue should share the same context
// as the 'from' cl_mem and the 'to' cl_mem. If it does not, a new command
// queue will be created internally.
// If the context we must use already has oneo f these internal command queues,
// we will use the cached version of it.
// Inputs:
//      command_queue: command queue onto which the copy commands will
//                     nominally be enqueued. See above for cases where this
//                     will not actually be the queue used.
//      from:   cl_mem that holds the data we will be copying from
//      to: cl_mem where we are trying to copy data to. Should already be
//          created and have enough space to hold all of the copied data
//      off_from: offset into the 'from' buffer we will start copying from 
//      off_to: offset into the 'to' buffer where we will start putting data
//      size:   the number of bytes to copy between the two buffers
//      num_evt:    number of input events that this copy should wait on.
//      evt_list:   list of cl_events that should finish before the copy starts
//      out:    cl_event that will inform the outside world when the
//              asynchronous copies within this command complete
void inner_buffer_copy(cl_command_queue command_queue, cl_mem from, cl_mem to,
        size_t off_from, size_t off_to, size_t size, unsigned num_evt,
        const cl_event *evt_list, cl_event *out);

#endif // __CL_BUFFER_COPY_H

