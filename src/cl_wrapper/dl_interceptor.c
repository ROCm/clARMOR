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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ********************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "meta_data_lists/dl_intercept_lists.h"
#include "util_functions.h"

#include "dl_interceptor_internal.h"

extern void* global_dl_protect;

#define DL_MSG(msg)                                                            \
{                                                                              \
    if (msg != NULL) {                                                         \
        det_fprintf(stderr, "DL_WRAPPER : %s (%d): %s\n", __func__, __LINE__, msg);\
    }                                                                          \
}

/*************** dlopen \ dlsym functions ********************************************************/

/*
 * Protected function set initialization
 * Update this list when new functions are added to cl_interceptor.
 */
void init_protect_list ( void )
{
    protect_name("clGetDeviceInfo");
    protect_name("clCreateCommandQueue");
    protect_name("clCreateCommandQueueWithProperties");
    protect_name("clRetainCommandQueue");
    protect_name("clReleaseCommandQueue");
    protect_name("clGetMemObjectInfo");
    protect_name("clCreateBuffer");
    protect_name("clCreateSubBuffer");
    protect_name("clGetImageInfo");
    protect_name("clCreateImage");
    protect_name("clCreateImage2D");
    protect_name("clCreateImage3D");
    protect_name("clRetainMemObject");
    protect_name("clReleaseMemObject");
    protect_name("clSVMAlloc");
    protect_name("clSVMFree");
    protect_name("clEnqueueSVMFree");
    protect_name("clEnqueueSVMMemcpy");
    protect_name("clEnqueueSVMMemFill");
    protect_name("clRetainKernel");
    protect_name("clReleaseKernel");
    protect_name("clEnqueueNDRangeKernel");
    protect_name("clEnqueueTask");
    protect_name("clEnqueueNativeKernel");
    protect_name("clSetKernelArg");
    protect_name("clSetKernelArgSVMPointer");
    protect_name("clGetEventProfilingInfo");
    protect_name("clRetainEvent");
    protect_name("clReleaseEvent");
    protect_name("clEnqueueReadBuffer");
    protect_name("clEnqueueReadBufferRect");
    protect_name("clEnqueueWriteBuffer");
    protect_name("clEnqueueWriteBufferRect");
    protect_name("clEnqueueFillBuffer");
    protect_name("clEnqueueCopyBuffer");
    protect_name("clEnqueueCopyBufferRect");
    protect_name("clEnqueueReadImage");
    protect_name("clEnqueueWriteImage");
    protect_name("clEnqueueFillImage");
    protect_name("clEnqueueCopyImage");
    protect_name("clEnqueueCopyImageToBuffer");
    protect_name("clEnqueueCopyBufferToImage");

}

// do not call in a constructor, protected name list may not persist
#define LIBC ((void *) -1L)
static void * clInterceptorDllHandle = NULL;
int dll_init ( void )
{
    if(clInterceptorDllHandle == NULL)
    {
        clInterceptorDllHandle = dlopen( NULL, RTLD_LAZY ); //open self
        init_protect_list();
    }
    return 0;
}


DL_API_ENTRY void * DL_API_CALL
dlsym(void *handle, const char *symbol)
{
    dll_init();
    void * ret = NULL;

    void* use = handle;
    if( handle == LIBC )
    {
        use = clInterceptorDllHandle;
    }
    if( dl_protect_find(get_dl_protect(), symbol) != NULL || handle == NULL)
    {
        use = clInterceptorDllHandle;
    }

    char *error;
    dlerror();

    ret = __libc_dlsym(use, symbol);

    error = dlerror();
    if (error != NULL) {
        DL_MSG(error);
    }

    return ret;
}

void protect_name(const char *name)
{
#ifndef __clang_analyzer__
    char *savName;
    unsigned size;

    size = strlen(name);
    savName = calloc(sizeof(char), size+1);
    strncpy(savName, name, size);

    dl_protect_insert(get_dl_protect(), savName);
#else
#endif //__clang_analyzer__
}
