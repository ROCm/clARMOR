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

/*! \file dl_interceptor_internal.h
 *  This file is for intercepting dlsym, this prevents wrapped libraries
 *  from explicitly loading functions that clARMOR wraps
 */

#ifndef __DL_INTERCEPT_INTERNAL_H
#define __DL_INTERCEPT_INTERNAL_H

#ifndef DL_API_ENTRY
    #define DL_API_ENTRY
#endif
#ifndef DL_API_CALL
    #define DL_API_CALL
#endif

#include <stdlib.h>
#include <dlfcn.h>

/*!
 * This function is an alias for dlsym.
 * define here so clARMOR knows to look for it when wrapping dlsym
 *
 */
extern void *__libc_dlsym (void *, const char *);

int dll_init ( void );

/*!
 * add character string to the list of protected funtion names.
 * dl_interceptor will forward these names to the functions defined by the wrapper.
 *
 * \param name
 *      character string for function name to protect
 *
 */
void protect_name(const char *name);

typedef DL_API_ENTRY void*
    (DL_API_CALL * interceptor_dlsym)(
            void *handle,
            const char *symbol);

#endif //__DL_INTERCEPT_INTERNAL_H
