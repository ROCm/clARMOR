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

#include <pthread.h>
#include <map>
#include <unordered_map>

#include "generic_lists.hpp"
#include "meta_data_lists/cl_memory_lists.h"


std::unordered_map<cl_mem, cl_memobj*> global_cl_mem_alloc;
pthread_mutex_t cl_mem_lock = PTHREAD_MUTEX_INITIALIZER;

void* get_cl_mem_alloc( void )
{
    return &global_cl_mem_alloc;
}

int cl_mem_insert(void* map_v, cl_memobj *item)
{
    return unomap_insert<cl_mem, cl_memobj*>(map_v, item, &cl_mem_lock);
}

cl_memobj* cl_mem_remove(void* map_v, cl_mem handle)
{
    return unomap_remove<cl_mem, cl_memobj*>(map_v, handle, &cl_mem_lock);
}

cl_memobj* cl_mem_find(void* map_v, cl_mem handle)
{
    return unomap_find<cl_mem, cl_memobj*>(map_v, handle, &cl_mem_lock);
}

int cl_mem_delete(cl_memobj *item)
{
    if(item)
        free(item);
    return L_SUCCESS;
}


#ifdef CL_VERSION_2_0
std::map<void*, cl_svm_memobj*> global_cl_svm_mem_alloc;
pthread_mutex_t svm_lock = PTHREAD_MUTEX_INITIALIZER;

void* get_cl_svm_mem_alloc( void )
{
    return &global_cl_svm_mem_alloc;
}

int cl_svm_mem_insert(void* map_v, cl_svm_memobj *item)
{
    return map_insert<void*, cl_svm_memobj*>(map_v, item, &svm_lock);
}

cl_svm_memobj* cl_svm_mem_remove(void* map_v, void* handle)
{
    return map_remove<void*, cl_svm_memobj*>(map_v, handle, &svm_lock);
}

cl_svm_memobj* cl_svm_mem_find(void* map_v, const void* handle)
{
    std::map<void*, cl_svm_memobj*> *map;
    map = (std::map<void*, cl_svm_memobj*>*)map_v;
    cl_svm_memobj *ret = NULL;

    pthread_mutex_lock(&svm_lock);

    std::map<void*, cl_svm_memobj*>::iterator got;
    got = map->upper_bound((void*)handle);
    if(got == map->begin())
    {
        pthread_mutex_unlock(&svm_lock);
        return ret;
    }

    --got;
    ret = got->second;

    // if (ret->handle == handle){}
    // ^ If we are looking for the head of this buffer, we win

    // If the pointer is to some buffer before lp, quit
    if (ret->handle > handle)
        ret = NULL;
    else
    {
        // If we are looking for somewhere inside this buffer, we win
        char *buffer_base = (char*)ret->handle;
        const char *ptr_to_check = (char*)handle;
        ptrdiff_t difference = ptr_to_check - buffer_base;
        if (ret->size < (size_t)difference)
            ret = NULL;
    }

    pthread_mutex_unlock(&svm_lock);
    return ret;
}

cl_svm_memobj* cl_svm_mem_next(void* map_v, const void* handle)
{
    return map_next<void*, cl_svm_memobj*>(map_v, (void*)handle, &svm_lock);
}

int cl_svm_mem_delete(cl_svm_memobj *item)
{
    if(item)
        free(item);
    return L_SUCCESS;
}
#endif
