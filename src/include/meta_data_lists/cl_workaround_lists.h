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


/*! \file cl_workaround_lists.h
 * Structs and lists that are used to work around problems in OpenCL runtimes.
 */

#ifndef __CL_WORKAROUND_LISTS_H__
#define __CL_WORKAROUND_LISTS_H__

#ifdef __cplusplus
extern "C" {
#endif

#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#include <CL/cl.h>

#ifdef CL_VERSION_2_0
/*!
 * Fine-grained SVM allocation caches.
 * The following code is used to cache FG SVM allocations, because the AMD
 * OpenCL runtime does not properly free these. As such, we never call the free
 * functions for these fine-grained buffers. If we allocate one of them, we
 * cache it on the request to free it. We then search our caches for furture
 * allocations to see if one of our previous allocations would work.
 *
 * Call this right after you call clSVMAlloc(CL_MEM_SVM_FINE_GRAIN_BUFFER).
 * This lets us keep track of all fine-grained SVM allocations so that we can
 * cache them. No return value.
 *
 * \param context
 *      alloc context
 * \param base
 *      alloc base address pointer
 * \param flags
 *      configuration flags
 * \param size
 *      length of allocation
 * \param alignment
 *      page alignement
*/
void cl_svm_fg_alloc(cl_context context, void * base, cl_svm_mem_flags flags,
                        size_t size, unsigned int alignment);

/*!
 * Call this *before* calling clSMVFree(). Do this *FOR ALL SVM BUFFERS*.
 *
 * This will check if the address was allocated as an SVM buffer. If so, it
 * put it in our FG SVM cache. At that point, you don't need to actually
 * call clSVMFree().
 *
 * \param context
 *      free context
 * \param base
 *      free base pointer
 * \return
 *      0 if this we successfully "freed" a FG SVM region.
 *      1 if we did not succesffully "free" a FG SVM region.
 *          IF THIS RETURNS 1, YOU MUST CALL clSVMFree() to avoid leaks.
 */
int cl_svm_fg_free(cl_context context, void* base);

/*!
 * Call this before trying to call clSVMAlloc(CL_MEM_SVM_FINE_GRAIN_BUFFER).
 * If you have a previously-allocated-and-then-freed FG SVM allocation, this
 * will try to find it for you. If the previous region is big enough and has
 * the right alignment, we will return it.
 *
 * \param context
 * \param size
 * \param alignment
 * \return
 *      NULL if there is no proper FG SVM region. You must clSVMAlloc yourself.
 *          ===or===
 *      void* pointer to the previously allocated region that's big enough to
 *      hold your request and has the right alignment..
 */
void *find_free_svm(cl_context context, size_t size, unsigned int alignment);
#endif // CL_VERSION_2_0



/*!
 * Command queue cache. This way, when copying buffers, we only create one
 * command queue per context. Right now, we leak command queues due to memory
 * corruption in the AMD OpenCL runtime. As such, creating a massive amount
 * of buffers causes us to run out of host memory to support these queues.
 * We must judiciously create these or risk breaking things.
 * As such, this struct takes a context and tries to find a command queue
 * associated with it.
 */
typedef struct commandQueueCache_
{
    cl_context handle;
    cl_command_queue cached_queue;
    uint32_t    ref_count;
} commandQueueCache;

/*!
 * A global list of all of the command queue cache entries in the system.
 * Pass this list into the insert, remove, and find delete functions below.
 *
 * \return pointer to commandQueueCache map
 */
void* get_cmd_queue_cache(void);

// These functions will add or remove a command queue cache entryrom a list,
// where the list is of the type returned by the above get_cmd_queue_cache().
// These lists are indexed by the handle cl_copntext in the cachce struct.

/*!
 *
 * \param map_v
 *      map of commandQueueCache
 * \param item
 *      cached command queue information
 * \return
 *      0 success
 *      other fail
 */
int commandQueueCache_insert(void* map_v, commandQueueCache *item);

/*!
 * If there is a cache entry from this context in the list of cache entries,
 * remove it from the list and return it.
 *
 * \param map_v
 *      map of commandQueueCache
 * \param handle
 *      cl_context of command queue
 * \return
 *      commandQueueCache pointer
 *      NULL if not found
 */
commandQueueCache* commandQueueCache_remove(void* map_v,
        const cl_context handle);

/*!
 * If there is a cache entry from this context in the list of cache entries,
 * return it.
 *
 * \param map_v
 *      map of commandQueueCache
 * \param handle
 *      cl_context of command queue
 * \return
 *      commandQueueCache pointer
 *      NULL if not found
 */
commandQueueCache* commandQueueCache_find(void* map_v, const cl_context handle);

/*!
 * Free a particular commandQueueCache structure. If you don't remove it from
 * the list before deleting it, you're likely to see a segfault.
 *
 * \param item
 *      pointer to command queue information
 * \return
 *      0 success
 *      other fail
 */
int commandQueueCache_delete(commandQueueCache *item);

#ifdef __cplusplus
}
#endif

#endif //__CL_WORKAROUND_LISTS_H__
