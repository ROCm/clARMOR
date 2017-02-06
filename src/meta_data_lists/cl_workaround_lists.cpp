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
#include <unordered_map>
#include "generic_lists.hpp"
#include "meta_data_lists/cl_workaround_lists.h"

#ifdef CL_VERSION_2_0
/******************************************************************************
 * Fine-grained SVM allocation caches
 *****************************************************************************/
typedef struct {
    void * base;
    cl_svm_mem_flags flags;
    size_t size;
    unsigned int alignment;
} fg_svm_region;

// All of these are initially keyd on the context, because SVM allocations
// come from a particular context.

// Keyed based on the pointer, for allocs and frees
std::map< cl_context, std::map<uint64_t, fg_svm_region> > allocated_fg_svms;
// Keyed based on the available size, holds what "free" SVMs we have
std::map< cl_context, std::multimap<size_t, fg_svm_region> > free_fg_svms;

void cl_svm_fg_alloc(cl_context context, void* base, cl_svm_mem_flags flags,
        size_t size, unsigned int alignment)
{
    fg_svm_region temp;
    temp.base = base;
    temp.flags = flags;
    temp.size = size;
    temp.alignment = alignment;
    std::pair<uint64_t, fg_svm_region> insert_me((uint64_t)(temp.base), temp);
    (allocated_fg_svms[context]).insert(insert_me);
}

int cl_svm_fg_free(cl_context context, void* base)
{
    std::map<cl_context, std::map<uint64_t, fg_svm_region> >::iterator
        cur_ctx_pair = allocated_fg_svms.find(context);
    if (cur_ctx_pair == allocated_fg_svms.end())
        return 1;

    std::map<uint64_t, fg_svm_region> *cur_ctx_fg_svm = &(cur_ctx_pair->second);
    std::map<uint64_t, fg_svm_region>::iterator result;
    result = cur_ctx_fg_svm->find((uint64_t)base);

    if (result != cur_ctx_fg_svm->end())
    {
        fg_svm_region temp = result->second;
        cur_ctx_fg_svm->erase(result);
        (free_fg_svms[context]).insert(
                std::pair<size_t, fg_svm_region>(temp.size, temp));
        return 0;
    }
    return 1;
}

void *find_free_svm(cl_context context, size_t size, unsigned int alignment)
{
    std::map<cl_context, std::multimap<size_t, fg_svm_region> >::iterator
        cur_ctx_pair = free_fg_svms.find(context);
    if (cur_ctx_pair == free_fg_svms.end())
        return NULL;

    std::multimap<uint64_t, fg_svm_region> *cur_ctx_free_svm;
    cur_ctx_free_svm = &(cur_ctx_pair->second);
    std::multimap<uint64_t, fg_svm_region>::iterator result;
    result = cur_ctx_free_svm->lower_bound(size);

    while (result != cur_ctx_free_svm->end())
    {
        fg_svm_region temp = result->second;
        if (temp.alignment == alignment)
        {
            cur_ctx_free_svm->erase(result);
            std::pair<uint64_t, fg_svm_region> insert_me(
                    (uint64_t)(temp.base), temp);
            (allocated_fg_svms[context]).insert(insert_me);
            return (void*)(temp.base);
        }
        else
            ++result;
    }
    return NULL;
}
#endif // CL_VERSION_2_0



std::unordered_map<cl_context, commandQueueCache*> global_cmd_queue_cache;

void* get_cmd_queue_cache( void )
{
    return &global_cmd_queue_cache;
}

int commandQueueCache_insert(void* map_v, commandQueueCache *item)
{
    return unomap_insert<cl_context, commandQueueCache*>(map_v, item);
}

commandQueueCache* commandQueueCache_remove(void* map_v,
        const cl_context handle)
{
    return unomap_remove<cl_context, commandQueueCache*>(map_v, handle);
}

commandQueueCache* commandQueueCache_find(void* map_v, const cl_context handle)
{
    return unomap_find<cl_context, commandQueueCache*>(map_v, handle);
}

int commandQueueCache_delete(commandQueueCache *item)
{
    if(item)
        free(item);
    return L_SUCCESS;
}
