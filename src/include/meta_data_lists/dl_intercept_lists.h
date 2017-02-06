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

#ifndef __DL_INTERCEPT_LISTS_H__
#define __DL_INTERCEPT_LISTS_H__

/*! \file dl_intercept_lists.h
 * Set of protected names from dlsym calls.
 */

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * A global list of all of the protected function names.
 * Pass this list into the insert, remove, and find delete functions below.
 *
 * \return pointer to protected name set
 */
void* get_dl_protect(void);

/*!
 *
 * \param set_v
 *      set of string names
 * \param item
 *      name to protect
 * \return
 *      0 success
 *      other fail
 */
int dl_protect_insert(void* set_v, const char *item);

/*!
 * If the name in handle is present in the set,
 * remove it and return it.
 *
 * \param set_v
 *      set of protected names
 * \param handle
 *      name to find
 * \return
 *      char pointer
 *      NULL if not found
 */
const char * dl_protect_remove(void* set_v, const char *handle);

/*!
 * Find name in handle in the set,
 *
 * \param set_v
 *      set of protected names
 * \param handle
 *      name to find
 * \return
 *      char pointer
 *      NULL if not found
 */
const char * dl_protect_find(void* set_v, const char *handle);

/*!
 * delete a protected name
 *
 * \param item
 *      delete this string
 * \return
 *      0 success
 *      other fail
 */
int dl_protect_delete(char *item);


#ifdef __cplusplus
}
#endif

#endif //__DL_INTERCEPT_LISTS_H__
