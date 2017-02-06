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


/*! \file cl_event_lists.h
 * Structs and list manipulators to keep data created by OpenCL calls.
 */

#ifndef __CL_EVENT_LISTS_H__
#define __CL_EVENT_LISTS_H__

#ifdef __cplusplus
extern "C" {
#endif

#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#include <CL/cl.h>

/*!
 * Sometimes our detector returns fake events to users that are not
 * the events created by the real OpenCL API call. As such, when they
 * perform calls like clReleaseEvent() or get profiling information,
 * we must be able to link these two events together. As such, the
 * original_cl_event value lets us find the that now-hidden event
 * from the values given back to the user.
 */
typedef struct cl_evt_info_
{
    cl_event handle; /// The user-visible cl_event
    uint32_t ref_count;
    cl_event internal_event;
} cl_evt_info;

/*!
 * A global list of pointers to the cl_event information descriptors.
 * Pass this list into the insert, remove, and find, & delete functions below.
 *
 * \return void pointer to unordered_map of cl_evt_info
 */
void* get_cl_evt_list();

// These functions will add or remove a cl_evt_info from a list, where the
// list is of the type returned by the above get_cl_evt_list() function.
// These lists are indexed by the handle in the real cl_event.

/*!
 *
 * \param map_v
 *      void pointer to unordered_map of cl_evt_info
 * \param evt_info
 *      cl_evt_info struct pointer
 *
 * \return
 *      0 on success
 *      non-zero on failure
 */
int cl_event_insert(void* map_v, cl_evt_info* evt_info);

/*!
 * If the cl_event exists, remove it from the list and return a pointer
 * to its cl_evt_info structure.
 *
 * \param map_v
 *      void pointer to unordered_map of cl_evt_info
 * \param evt
 *      cl_event to remove
 *
 * \return removed cl_evt_info object pointer
 */
cl_evt_info* cl_event_remove(void* map_v, cl_event evt);

/*!
 * If the cl_kernel exists, return a pointer to its kinfo structure.
 *
 * \param map_v
 *      void pointer to unordered_map of cl_evt_info
 * \param evt
 *      cl_event to remove
 *
 * \return cl_evt_info object pointer
 */
cl_evt_info* cl_event_find(void* map_v, cl_event evt);

/*
 * Free a particular cl_evt_info structure. If you don't remove it from
 * the list before deleting it, you're likely to see a segfault.
 *
 * \param item
 *      cl_evt_info struct pointer
 *
 * \return
 *      0 on success
 *      non-zero on failure
 */
int cl_event_delete(cl_evt_info* item);

#ifdef __cplusplus
}
#endif

#endif //__CL_EVENT_LISTS_H__
