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

#include <unordered_map>
#include <map>

template <typename LL_Type>
int insert(LL_Type **list, LL_Type *item, pthread_mutex_t *lock)
{
    if(lock)
        pthread_mutex_lock(lock);

    if(*list == NULL)
    {
        *list = item;
        if(item) item->next = NULL;
    }
    else
    {
        LL_Type *prev = NULL, *lp = *list;

        while(lp)
        {
           if(item->handle < lp->handle)
           {
                item->next = lp;
                if(prev)
                    prev->next = item;
                else
                    *list = item;
                break;
           }
           prev = lp;
           lp = lp->next;
        }
        if(!lp)
        {
            prev->next = item;
            item->next = NULL;
        }
    }

    if(lock)
        pthread_mutex_unlock(lock);

    return L_SUCCESS;
}

template <typename LL_Type, typename H_Type>
LL_Type* remove(LL_Type **list, const H_Type handle, pthread_mutex_t *lock)
{
    LL_Type *prev = NULL, *lp = *list;
    LL_Type *ret = NULL;

    if(lock)
        pthread_mutex_lock(lock);

    while(lp)
    {
        if(lp->handle == handle)
        {
            if(prev)
            {
                prev->next = lp->next;
            }
            else{
                *list = lp->next;
            }
            ret = lp;
            break;
        }

        if(lp->handle > handle) break;

        prev = lp;
        lp = lp->next;
    }

    if(lock)
        pthread_mutex_unlock(lock);

    return ret;
}

template <typename LL_Type, typename H_Type>
LL_Type* find(LL_Type *const list, const H_Type handle, pthread_mutex_t *lock)
{
    LL_Type *lp = list;
    LL_Type *ret = NULL;

    if(lock)
        pthread_mutex_lock(lock);

    while(lp)
    {
        if(lp->handle == handle)
        {
            ret = lp;
            break;
        }

        if(lp->handle > handle) break;

        lp = lp->next;
    }

    if(lock)
        pthread_mutex_unlock(lock);

    return ret;
}

template <typename KEY, typename ITEM>
int unomap_insert(void* unomap_v, ITEM item, pthread_mutex_t *lock)
{
    if(unomap_v == NULL)
        return L_FAIL;

    if(lock)
        pthread_mutex_lock(lock);

    std::unordered_map<KEY, ITEM> *map =
        (std::unordered_map<KEY, ITEM>*)unomap_v;
    std::pair<KEY, ITEM> elem (item->handle, item);

    if(map->empty())
        map->clear();

    map->insert(elem);

    if(lock)
        pthread_mutex_unlock(lock);

    return L_SUCCESS;
}

template <typename KEY, typename ITEM>
ITEM unomap_remove(void* unomap_v, KEY handle, pthread_mutex_t *lock)
{
    if(unomap_v == NULL)
        return NULL;

    if(lock)
        pthread_mutex_lock(lock);

    std::unordered_map<KEY, ITEM> *map =
        (std::unordered_map<KEY, ITEM>*)unomap_v;
    ITEM ret = NULL;

    typename std::unordered_map<KEY, ITEM>::iterator got = map->find(handle);
    if(got != map->end())
    {
        ret = got->second;
        map->erase(handle);
    }

    if(lock)
        pthread_mutex_unlock(lock);

    return ret;
}

template <typename KEY, typename ITEM>
ITEM unomap_find(void* unomap_v, KEY handle, pthread_mutex_t *lock)
{
    if(unomap_v == NULL)
        return NULL;

    if(lock)
        pthread_mutex_lock(lock);

    std::unordered_map<KEY, ITEM> *map =
        (std::unordered_map<KEY, ITEM>*)unomap_v;
    ITEM ret = NULL;

    typename std::unordered_map<KEY, ITEM>::iterator got = map->find(handle);
    if(got != map->end())
        ret = got->second;

    if(lock)
        pthread_mutex_unlock(lock);

    return ret;
}

template <typename KEY, typename ITEM>
int map_insert(void* map_v, ITEM item, pthread_mutex_t *lock)
{
    if(map_v == NULL)
        return L_FAIL;

    if(lock)
        pthread_mutex_lock(lock);

    std::map<KEY, ITEM> *map = (std::map<KEY, ITEM>*)map_v;
    std::pair<KEY, ITEM> elem (item->handle, item);

    if(map->empty())
        map->clear();

    map->insert(elem);

    if(lock)
        pthread_mutex_unlock(lock);

    return L_SUCCESS;
}

template <typename KEY, typename ITEM>
ITEM map_remove(void* map_v, KEY handle, pthread_mutex_t *lock)
{
    if(map_v == NULL)
        return NULL;

    if(lock)
        pthread_mutex_lock(lock);

    std::map<KEY, ITEM> *map = (std::map<KEY, ITEM>*)map_v;
    ITEM ret = NULL;

    typename std::map<KEY, ITEM>::iterator got = map->find(handle);
    if(got != map->end())
    {
        ret = got->second;
        map->erase(handle);
    }

    if(lock)
        pthread_mutex_unlock(lock);

    return ret;
}

template <typename KEY, typename ITEM>
ITEM map_find(void* map_v, KEY handle, pthread_mutex_t *lock)
{
    if(map_v == NULL)
        return NULL;

    if(lock)
        pthread_mutex_lock(lock);

    std::map<KEY, ITEM> *map = (std::map<KEY, ITEM>*)map_v;
    ITEM ret = NULL;

    typename std::map<KEY, ITEM>::iterator got = map->find(handle);
    if(got != map->end())
        ret = got->second;

    if(lock)
        pthread_mutex_unlock(lock);

    return ret;
}

template <typename KEY, typename ITEM>
ITEM map_next(void* map_v, KEY handle, pthread_mutex_t *lock)
{
    if(map_v == NULL)
        return NULL;

    if(lock)
        pthread_mutex_lock(lock);

    std::map<KEY, ITEM> *map = (std::map<KEY, ITEM>*)map_v;
    ITEM ret = NULL;

    typename std::map<KEY, ITEM>::iterator got = map->upper_bound(handle);
    if(got != map->end())
        ret = got->second;

    if(lock)
        pthread_mutex_unlock(lock);

    return ret;
}
