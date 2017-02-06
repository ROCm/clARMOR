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


// This file contains a lot of helper functions to access different types of
// lists that can hold meta-data for our buffer overflow detector.

// Every function can optoinally take a pthread_mutex_t as an argument, which
// it will grab internally and then release before returning. This allows the
// lists to easily be accessed atomically without needing extra code in the
// calling function.

#include <pthread.h>

// Used internally for returns from all the meta-data lists
#define L_SUCCESS 0
#define L_FAIL 1


// Generic custom linked lists.
// Each of these assumes your LL_Type is a struct with an entry "handle", where
// of type "H_Type". This is used to find distrinct entries in the linked list
// using "==".
// For insert and delete, pass a reference to the pointer that contains the
// head of the linked list.
template <typename LL_Type>
int insert(LL_Type **list, LL_Type *item, pthread_mutex_t *lock = NULL);

template <typename LL_Type, typename H_Type>
LL_Type* remove(LL_Type **list, const H_Type handle,
        pthread_mutex_t *lock = NULL);

// For Find, just pass the pointer to the head of the linked list.
template <typename LL_Type, typename H_Type>
LL_Type* find(LL_Type *const list, const H_Type handle,
        pthread_mutex_t *lock = NULL);


// C++ STL unordered map. Pass a pointer to the std::unordered_map you want
// to use into the void* uomap_v.
template <typename KEY, typename ITEM>
int unomap_insert(void* unomap_v, ITEM item, pthread_mutex_t *lock = NULL);

template <typename KEY, typename ITEM>
ITEM unomap_remove(void* unomap_v, KEY handle, pthread_mutex_t *lock = NULL);

template <typename KEY, typename ITEM>
ITEM unomap_find(void* unomap_v, KEY handle, pthread_mutex_t *lock = NULL);


// C++ STL map. Pass a pointer to the std::map you want to use into the void*
// map_v
template <typename KEY, typename ITEM>
int map_insert(void* map_v, ITEM item, pthread_mutex_t *lock = NULL);

template <typename KEY, typename ITEM>
ITEM map_remove(void* map_v, KEY handle, pthread_mutex_t *lock = NULL);

template <typename KEY, typename ITEM>
ITEM map_find(void* map_v, KEY handle, pthread_mutex_t *lock = NULL);

// Will find the next entry in the map *after* the one we find using handle.
template <typename KEY, typename ITEM>
ITEM map_next(void* map_v, KEY handle, pthread_mutex_t *lock = NULL);

#include "generic_lists.ipp"
