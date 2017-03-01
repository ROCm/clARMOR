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

#include <stdlib.h>
#include "string.h"

#include "generic_lists.hpp"
#include "meta_data_lists/dl_intercept_lists.h"

pthread_mutex_t dl_protect_lock = PTHREAD_MUTEX_INITIALIZER;

struct lessStr
{
    bool operator() ( const char* lhs, const char* rhs ) const
    {
        int ret = strcmp(lhs, rhs);
        ret = (ret <= 0) ? 0 : 1;
        //fprintf(stderr, "%s : %s : %d\n", lhs, rhs, ret);
        return ret;
    }
};

std::set<const char*, lessStr> global_dl_protect;

void* get_dl_protect(void)
{
    return &global_dl_protect;
}

int dl_protect_insert(void* set_v, const char *item)
{
    return set_insert<const char*, lessStr>(set_v, item, &dl_protect_lock);
}

const char * dl_protect_remove(void* set_v, const char *item)
{
    return set_remove<const char*, lessStr>(set_v, item, &dl_protect_lock);
}

const char * dl_protect_find(void* set_v, const char *item)
{
    return set_find<const char*, lessStr>(set_v, item, &dl_protect_lock);
}

int dl_protect_delete(char *item)
{
    if(item)
        free(item);
    return L_SUCCESS;
}
