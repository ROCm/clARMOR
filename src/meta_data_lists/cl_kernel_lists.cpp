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


#include <map>
#include "generic_lists.hpp"
#include "meta_data_lists/cl_kernel_lists.h"

pthread_mutex_t kernel_arg_lock = PTHREAD_MUTEX_INITIALIZER;

int karg_insert(kernel_arg **list, kernel_arg *item)
{
    return insert<kernel_arg>(list, item, &kernel_arg_lock);
}

kernel_arg* karg_remove(kernel_arg **list, const cl_uint handle)
{
    return remove<kernel_arg, const cl_uint>(list, handle, &kernel_arg_lock);
}

kernel_arg* karg_find(kernel_arg *const list, const cl_uint handle)
{
    return find<kernel_arg, const cl_uint>(list, handle, &kernel_arg_lock);
}

int karg_delete(kernel_arg *item)
{
    if(item == NULL)
        return L_SUCCESS;
    if (item->value)
        free(item->value);
    free(item);
    return L_SUCCESS;
}


std::map<cl_kernel, kernel_info*> global_kernels_list;
pthread_mutex_t kernel_info_lock = PTHREAD_MUTEX_INITIALIZER;

void* get_kern_list( void )
{
    return &global_kernels_list;
}

int kinfo_insert(void* map_v, kernel_info *item)
{
    return map_insert<cl_kernel, kernel_info*>(map_v, item, &kernel_info_lock);
}

kernel_info* kinfo_remove(void* map_v, cl_kernel handle)
{
    return map_remove<cl_kernel, kernel_info*>(map_v, handle, &kernel_info_lock);
}

kernel_info* kinfo_find(void* map_v, cl_kernel handle)
{
    return map_find<cl_kernel, kernel_info*>(map_v, handle, &kernel_info_lock);
}

int kinfo_delete(kernel_info *item)
{
    if(item == NULL)
        return L_SUCCESS;
    kernel_arg *lp = item->arg_list;

    while(lp)
    {
        kernel_arg *temp = lp;
        lp = lp->next;
        karg_delete(temp);
        temp = NULL;
    }

    free(item);

    return L_SUCCESS;
}
