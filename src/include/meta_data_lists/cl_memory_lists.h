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


/*! \file cl_memory_lists.h
 * Structs and list manipulators to keep data created by OCL memory API calls
 */

#ifndef __CL_MEMORY_LISTS_H__
#define __CL_MEMORY_LISTS_H__

#ifdef __cplusplus
extern "C" {
#endif

#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#include <CL/cl.h>
#include "detector_defines.h"

/******************** cl_mem Lists *******************************************/
/*!
 * Structure to describe OpenCL cl_mem buffers. We must keep this around in
 * order to know enough information about them to run the buffer overflow
 * detector. We often need to allocate new buffers with extended memory
 * regions for canaries, or secondary buffers that hold canary values.
 * The values in this struct describe enough information about OpenCL
 * buffers or images that we can recreate that data.
 */
typedef struct cl_memobj_list_
{
    cl_mem handle; /// the unique cl_mem object
    cl_context context;
    cl_mem_flags flags;
    size_t size;
    void * host_ptr;
    uint32_t ref_count;
    /// If this is an OpenCL image, we need some extra informationa bout it.
    uint8_t is_image;
    cl_image_format image_format;
    cl_image_desc image_desc;
    /// If this is a subBuffer, we need its origin
    uint8_t is_sub;
    size_t origin;
    ///buffers with host pointers, images with buffers, and sub buffers do not have canaries
    uint8_t has_canary;
    /// This flag tells whether this was a buffer created by the user,
    /// or whether it is some buffer internal to the detector itself.
    uint8_t detector_internal_buffer;
} cl_memobj;

/*!
 * A global list of pointers to all of the cl_memobj descriptors in the system.
 * Pass this list into the insert, remove, find, and delete functions below.
 *
 * \return void* to map of cl_memobj
 */
void* get_cl_mem_alloc();

// These functions will add or remove a cl_memobj from a list, where the list
// is of the type returned by the above get_cl_mem_alloc() function.
// These lists are indexed by the handle in the cl_memobj function.

/*!
 * Insert a cl_memobj into map_v
 *
 *
 * \param map_v
 *      pointer to map of cl_memobj
 * \param item
 *      cl_mem instantiation information
 * \return
 *      0 success
 *      other fail
 */
int cl_mem_insert(void* map_v, cl_memobj *item);

/*!
 * If a cl_memobj with the requested handle exists, remove it from the list and
 * return a pointer to it.
 *
 * \param map_v
 *      pointer to map of cl_memobj
 * \param handle
 *      cl_mem to find information for
 * \return
 *      pointer to cl_memobj
 *      NULL if not found
 */
cl_memobj* cl_mem_remove(void* map_v, cl_mem handle);

/*!
 * If a cl_memobj with the requested handle exists, return a pointer to it.
 *
 * \param map_v
 *      pointer to map of cl_memobj
 * \param handle
 *      cl_mem to find information for
 * \return
 *      pointer to cl_memobj
 *      NULL if not found
 */
cl_memobj* cl_mem_find(void* map_v, cl_mem handle);

/*!
 * Delete cl_memobj
 *
 * \param item
 *      delete this cl_memobj
 * \return
 *      0 success
 *      else fail
 */
int cl_mem_delete(cl_memobj *item);


/*********************** SVM Lists *******************************************/
/*!
 * Like the cl_mem lists above, this structure can be used to keep track of all
 * of the important information about an SVM object
 */
#ifdef CL_VERSION_2_0
typedef struct cl_svm_memobj_list_
{
    void * handle; /// the pointer to the SVM region
    cl_context context;
    cl_svm_mem_flags flags;
    size_t size;
    unsigned int alignment;
    uint8_t detector_internal_buffer;
} cl_svm_memobj;
#else // !CL_VERSION_2_0
/*!
 * This definition only exists so that we can compile our tool correctly
 * on systems that do not support OpenCL 2.0.
 */
typedef struct cl_svm_memobj_list_
{
    void * handle;
    size_t size;
} cl_svm_memobj;
#endif // CL_VERSION_2_0

/*!
 * A global list of pointers to all of the cl_svm_memobj descriptors in the
 * system. Pass this into the insert, remove, find, etc. functions below.
 *
 * \return pointer to cl_svm_memobj map
 */
void* get_cl_svm_mem_alloc();

#ifdef CL_VERSION_2_0
/*!
 * For a description of what these functions do, see the comments for the
 * cl_mem_* functions above.
 */
int cl_svm_mem_insert(void* map_v, cl_svm_memobj *item);
cl_svm_memobj* cl_svm_mem_remove(void* map_v, void* handle);
cl_svm_memobj* cl_svm_mem_find(void* map_v, const void* handle);
int cl_svm_mem_delete(cl_svm_memobj *item);
/*!
 * Return next item in the map if present, NULL if iterator is at the end
 */
cl_svm_memobj* cl_svm_mem_next(void* map_v, const void* handle);
#endif

#ifdef __cplusplus
}
#endif

#endif //__CL_MEMORY_LISTS_H__
