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

#ifndef _GNU_SOURCE
      #define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <execinfo.h>
#include <sys/file.h>
#include <CL/cl.h>

#include "util_functions.h"

uint32_t global_tool_stats_flags = 0;

// Get the Unix environment variable that matches the env_var_nm_ input param.
// This is then stored into a newly malloced C-string that will be returned
// in the env_ parameter.
// padding to the memory allocation.
// Return value: 0 on success. Anything else is an error.
static int get_env_util( char **env_, const char *env_var_nm_ )
{
    char  *env_root;
    size_t var_len;
    if (env_ == NULL)
    {
        fprintf(stderr, "%s (%d) error: trying ", __func__, __LINE__);
        fprintf(stderr, "to store environment variable in NULL location.\n");
        return -1;
    }
    if (env_var_nm_ == NULL)
    {
        fprintf(stderr, "%s (%d) error: ", __func__, __LINE__);
        fprintf(stderr, "trying to find environment variable, ");
        fprintf(stderr, "but variable is NULL.\n");
    }
    env_root = getenv(env_var_nm_);
    var_len = (env_root != NULL) ? strlen(env_root) : 0;
    if ( var_len == 0 )
        return 0;
    // Need the +1 to hold the terminating null byte.
    int num_bytes = asprintf(env_, "%s", env_root);
    CHECK_ASPRINTF_RET(num_bytes);
    if (*env_ == NULL)
    {
        fprintf(stderr, "%s (%d) error: asprintf failed\n", __func__, __LINE__);
        return -1;
    }
    return 0;
}

int get_error_envvar(void)
{
    char * error_envvar = NULL;
    if (getenv(__EXIT_ON_OVERFLOW__) == NULL)
    {
        return 0;
    }
    else
    {
        int ret_val;
        if (!get_env_util(&error_envvar, __EXIT_ON_OVERFLOW__))
        {
            if (error_envvar != NULL)
            {
                ret_val = strtol(error_envvar, NULL, 0);
                free(error_envvar);
            }
            else
                ret_val = -1;
        }
        else
            ret_val = -1;
        if (ret_val > 1)
            ret_val = 1;
        return ret_val;
    }
}

int get_gpu_strat_envvar(void)
{
    char * error_envvar = NULL;
    if (getenv(__USE_ALTERNATE_GPU_DETECTION__) == NULL)
    {
        return 0;
    }
    else
    {
        int ret_val;
        if (!get_env_util(&error_envvar, __USE_ALTERNATE_GPU_DETECTION__))
        {
            if (error_envvar != NULL)
            {
                ret_val = strtol(error_envvar, NULL, 0);
                free(error_envvar);
            }
            else
                ret_val = 0;
        }
        else
            ret_val = 0;
        return ret_val;
    }
}

int get_tool_perf_envvar(void)
{
    char * error_envvar = NULL;
    if (getenv(__TOOL_PERF_STATISTIC_MODE__) == NULL)
    {
        return 0;
    }
    else
    {
        int ret_val;
        if (!get_env_util(&error_envvar, __TOOL_PERF_STATISTIC_MODE__))
        {
            if (error_envvar != NULL)
            {
                ret_val = strtol(error_envvar, NULL, 0);
                free(error_envvar);
            }
            else
                ret_val = 0;
        }
        else
            ret_val = 0;

        return ret_val;
    }
}

const char *get_logging_envvar(void)
{
    char * logging_envvar = NULL;
    if (getenv(__DETECTOR_LOG_LOCATION__) != NULL)
    {
        if (!get_env_util(&logging_envvar, __DETECTOR_LOG_LOCATION__))
        {
            if (logging_envvar != NULL)
                return logging_envvar;
        }
    }
    return NULL;
}

unsigned int get_check_on_device_envvar(void)
{
    char * dev_check_envvar = NULL;
    if (getenv(__DETECTOR_DEVICE_SELECT__) == NULL)
    {
        return DEFAULT_DEVICE_CHECK;
    }
    else
    {
        unsigned int ret_val;
        if (!get_env_util(&dev_check_envvar, __DETECTOR_DEVICE_SELECT__))
        {
            if (dev_check_envvar != NULL)
            {
                ret_val = strtoul(dev_check_envvar, NULL, 0);
                free(dev_check_envvar);
            }
            else
                ret_val = 0;
        }
        else
            ret_val = 0;
        if (ret_val == 0)
            return DEFAULT_DEVICE_CHECK;
        else
            return ret_val;
    }

}

static char* get_backtrace_human_readable(void* bt, char* bt_symbol)
{
    char *syscom; // command line to get the human-readable backtrace
    char *output; // human-readable backtrace

    int p = strcspn(bt_symbol, "( ");

    int num_bytes = asprintf(&syscom,
            "addr2line %p -i -s -p -C -e %.*s | sed 's/ .inlined by. //'",
            bt, p, bt_symbol);
    CHECK_ASPRINTF_RET(num_bytes);

    // Run the addr2line program.
    FILE *function_name = popen(syscom, "re");

    // Get program output
    // Only pull in 4096 bytes, because we likely don't need more than that
    // to see useful information from addr2line.
    output = (char *)calloc(4096, sizeof(char));
    if ( (fgets(output, 4096, function_name) != NULL) )
    {
        //Replace the first newline with string end.
        output[strcspn(output, "\n")] = '\0';
    }
    output[4095]='\0'; // Just in case we didn't see a NULL from the fgets.
    pclose(function_name);

    if (syscom != NULL)
        free(syscom);

    return output;
}

char* get_backtrace_level(int level)
{
    void **buffer;
    char **messages;
    char *output; // data needed to get human-readable backtrace

    if (level < 1)
    {
        fprintf(stderr, "Asking for an illegal backtrace level: %d\n", level);
        return NULL;
    }
    // Calling this function adds another level to the backtrace.
    // We don't want the user to have to deal with that,
    // so silently hide that extra level.
    int actual_level = level+1;
    // Allocate actual_level+1 because C arrays start at '0'
    buffer = (void **)calloc(actual_level+1, sizeof(void *));

    if (backtrace(buffer, actual_level+1) != (actual_level+1))
    {
        // We wanted a level that does not actually exist.
        free(buffer);
        return NULL;
    }

    messages = backtrace_symbols(&(buffer[actual_level]), 1);

    char * bt_string = get_backtrace_human_readable(buffer[actual_level],
            messages[0]);
    int num_bytes = asprintf(&output, "%s - %s", messages[0], bt_string);
    CHECK_ASPRINTF_RET(num_bytes);
    if (output == NULL)
    {
        fprintf(stderr, "%s (%d) error: asprintf failed\n", __func__, __LINE__);
        exit(-1);
    }
    free(bt_string);
    free(messages);
    free(buffer);
    return output;
}

void print_backtrace( FILE* where_to )
{
    char **strings;
    void *buffer[100];
    int j, nptrs = backtrace(buffer, 100);

    fprintf(where_to, "BACKTRACE:\n");

    strings = backtrace_symbols(buffer, nptrs);

    for (j = nptrs-1; j > 0; j--)
    {
        char *bt_string = get_backtrace_human_readable(buffer[j], strings[j]);
        fprintf(where_to, "%s - %s\n", strings[j], bt_string);
        free(bt_string);
    }
    free(strings);
}


uint64_t timeval_diff_us(struct timeval *stop, struct timeval *start)
{
    uint64_t durr_us = 0;
    if(stop && start)
    {
        durr_us = (stop->tv_sec - start->tv_sec)*1000000 +
            stop->tv_usec - start->tv_usec;
    }

    return durr_us;
}

unsigned getImageDataSize(const cl_image_format *format)
{
    cl_channel_type type = format->image_channel_data_type;
    cl_channel_order order = format->image_channel_order;

    unsigned byteChannel = 1;
    unsigned numChannels = 1;

    switch(type)
    {
        case CL_SNORM_INT8:
        case CL_UNORM_INT8:
        case CL_SIGNED_INT8:
        case CL_UNSIGNED_INT8:
            byteChannel = 1;
            break;
        case CL_SNORM_INT16:
        case CL_UNORM_INT16:
        case CL_SIGNED_INT16:
        case CL_HALF_FLOAT:
        case CL_UNSIGNED_INT16:
            byteChannel = 2;
            break;
        case CL_UNORM_SHORT_565:
            byteChannel = 2;
            break;
        case CL_UNORM_SHORT_555:
            byteChannel = 2;
            break;
        case CL_UNORM_INT24:
            byteChannel = 3;
            break;
        case CL_SIGNED_INT32:
        case CL_UNSIGNED_INT32:
        case CL_FLOAT:
            byteChannel = 4;
            break;
        case CL_UNORM_INT_101010:
            byteChannel = 4;
            break;
        default:
            byteChannel = 0;
    }

    switch(order)
    {
        case CL_R:
        case CL_Rx:
        case CL_A:
            numChannels = 1;
            break;
        case CL_INTENSITY:
        case CL_LUMINANCE:
        case CL_DEPTH:
            numChannels = 1;
            break;
        case CL_RG:
        case CL_RGx:
        case CL_RA:
            numChannels = 2;
            break;
        case CL_RGB:
        case CL_RGBx:
            numChannels = 1;
            break;
        case CL_RGBA:
            numChannels = 4;
            break;
#ifdef CL_VERSION_2_0
        case CL_sRGB:
        case CL_sRGBx:
        case CL_sRGBA:
        case CL_sBGRA:
            numChannels = 4;
            break;
#endif
        case CL_ARGB:
        case CL_BGRA:
#ifdef CL_VERSION_2_0
        case CL_ABGR:
#endif
            numChannels = 4;
            break;
        case CL_DEPTH_STENCIL:
            numChannels = 1;
            break;
        default:
            numChannels = 0;
    }

    return byteChannel * numChannels;
}


