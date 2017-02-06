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

// Structs and lists associated with OpenCL kernel and kernel argument meta-data

#ifndef __CL_KERNEL_LISTS_H__
#define __CL_KERNEL_LISTS_H__

#include "cl_memory_lists.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#include <CL/cl.h>

// Information about an argument to an OpenCL kernel. This is created in the
// OpenCL API calls that set kernel arguments (as well as kernel SVM args).
// With this info, we can later learn about the argument, whether it was
// passing a buffer that we want to analyze, and then pass that same arg
// to a new kernel or to a kernel that should check canary values.
typedef struct kernel_arg_list_
{
    cl_uint handle; //arg_index;
    size_t size;
    void * value;
    cl_memobj * buffer;
    cl_svm_memobj * svm_buffer;
    // Need to keep a 'next' pointer so we can make a linked list of arguments.
    // We need to walk through all of them later in the code
    struct kernel_arg_list_ * next;
} kernel_arg;

// There is no function here to get a global list of arguments. Instead, the
// arg_list is associated with the kernel_info struct below. Every kernel has
// its own linked list of kernel_arg arguments.

// These functions will add or remove a kernel arg from its linked list.
int karg_insert(kernel_arg **list, kernel_arg *item);

// If the {handle}th kernel arg exists, remove it from the list and
// return a pointer to it.
kernel_arg* karg_remove(kernel_arg **list, const cl_uint handle);

// If the {handle}th kernel arg exists, return a pointer to it.
kernel_arg* karg_find(kernel_arg *const list, const cl_uint handle);

// Free a particular kernel argument structure. If you don't remove it from
// the linked list before deleting it, you're likely to see a segfault.
int karg_delete(kernel_arg *item);


// Information about an OpenCL kernel. In particular, the reference count
// (so that we can know when the kernel is released in the OpenCL runtime
// and thus remove this structure) and the list of argumnets to the kernel.
typedef struct kernel_info_list_
{
    cl_kernel   handle;
    uint32_t    ref_count;
    kernel_arg  *arg_list;
} kernel_info;

// A global list of pointers to the kernel_info descriptors in the system.
// Pass this list into the insert, remove, and find, & delete functions below.
void* get_kern_list();

// These functions will add or remove kernel info from a list, where the
// list is of the type returned by the above get_kern_list() function.
// These lists are indexed by the handle in the real cl_kernel.

// Functions return 0 on success, non-zero on failure
int kinfo_insert(void* map_v, kernel_info *item);

// If the cl_kernel exists, remove it from the list and return a pointer
// to its kinfo structure.
kernel_info* kinfo_remove(void* map_v, cl_kernel handle);

// If the cl_kernel exists, return a pointer to its kinfo structure.
kernel_info* kinfo_find(void* map_v, cl_kernel handle);

// Free a particular kinfo structure. If you don't remove it from
// the list before deleting it, you're likely to see a segfault.
int kinfo_delete(kernel_info *item);

#ifdef __cplusplus
}
#endif

#endif //__CL_KERNEL_LISTS_H__
