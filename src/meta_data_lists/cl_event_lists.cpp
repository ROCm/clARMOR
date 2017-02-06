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


#include <unordered_map>
#include "generic_lists.hpp"
#include "meta_data_lists/cl_event_lists.h"

std::unordered_map<cl_event*, cl_evt_info> cl_evt_list;
pthread_mutex_t cl_evt_lock = PTHREAD_MUTEX_INITIALIZER;

void* get_cl_evt_list( void )
{
    return &cl_evt_list;
}

int cl_event_insert(void* map_v, cl_evt_info* evt_info)
{
    return unomap_insert<cl_event, cl_evt_info*>(map_v, evt_info,
            &cl_evt_lock);
}

cl_evt_info* cl_event_remove(void* map_v, cl_event evt)
{
    return unomap_remove<cl_event, cl_evt_info*>(map_v, evt, &cl_evt_lock);
}

cl_evt_info* cl_event_find(void* map_v, cl_event evt)
{
    return unomap_find<cl_event, cl_evt_info*>(map_v, evt, &cl_evt_lock);
}

int cl_event_delete(cl_evt_info *item)
{
    if(item)
        free(item);
    return L_SUCCESS;
}
