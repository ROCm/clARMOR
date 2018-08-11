/********************************************************************************
 * Copyright (c) 2016-2017 Advanced Micro Devices, Inc. All rights reserved.
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
#include <stdarg.h>
#include <unistd.h>
#include <execinfo.h>
#include <sys/file.h>
#include <CL/cl.h>
#include "cl_err.h"

#include "util_functions.h"

uint32_t global_tool_stats_flags = 0;
char * global_tool_stats_outfile = NULL;

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
        det_fprintf(stderr, "%s (%d) error: trying ", __func__, __LINE__);
        det_fprintf(stderr, "to store environment variable in NULL location.\n");
        return -1;
    }
    if (env_var_nm_ == NULL)
    {
        det_fprintf(stderr, "%s (%d) error: ", __func__, __LINE__);
        det_fprintf(stderr, "trying to find environment variable, ");
        det_fprintf(stderr, "but variable is NULL.\n");
        return -1;
    }
    env_root = getenv(env_var_nm_);
    var_len = (env_root != NULL) ? strlen(env_root) : 0;
    if ( var_len == 0 )
        return 0;
    int num_bytes = asprintf(env_, "%s", env_root);
    CHECK_ASPRINTF_RET(num_bytes);
    if (*env_ == NULL)
    {
        det_fprintf(stderr, "%s (%d) error: asprintf failed\n", __func__, __LINE__);
        return -1;
    }
    return 0;
}

int get_error_envvar(void)
{
    char * error_envvar = NULL;
    if (getenv(__CLARMOR_EXIT_ON_OVERFLOW__) == NULL)
    {
        return 0;
    }
    else
    {
        int ret_val;
        if (!get_env_util(&error_envvar, __CLARMOR_EXIT_ON_OVERFLOW__))
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

int get_exitcode_envvar(void)
{
    char * exitcode_envvar = NULL;
    int ret_val = 0;
    if (getenv(__ERROR_EXITCODE__) != NULL)
    {
        if (!get_env_util(&exitcode_envvar, __ERROR_EXITCODE__))
        {
            if (exitcode_envvar != NULL)
            {
                ret_val = strtol(exitcode_envvar, NULL, 0);
                free(exitcode_envvar);
            }
        }
    }
    return ret_val;
}

int get_gpu_strat_envvar(void)
{
    char * gpu_envvar = NULL;
    if (getenv(__CLARMOR_ALTERNATE_GPU_DETECTION__) == NULL)
    {
        return GPU_MODE_DEFAULT;
    }
    else
    {
        int ret_val;
        if (!get_env_util(&gpu_envvar, __CLARMOR_ALTERNATE_GPU_DETECTION__))
        {
            if (gpu_envvar != NULL)
            {
                ret_val = strtol(gpu_envvar, NULL, 0);
                free(gpu_envvar);
            }
            else
                ret_val = GPU_MODE_DEFAULT;
        }
        else
            ret_val = GPU_MODE_DEFAULT;
        return ret_val;
    }
}

int get_disable_api_check_envvar(void)
{
    char * print_disable_api_envvar = NULL;
    if (getenv(__CLARMOR_DISABLE_API_CHECK__) == NULL)
        return 0;
    else
    {
        unsigned int ret_val = 0;
        if (!get_env_util(&print_disable_api_envvar, __CLARMOR_DISABLE_API_CHECK__))
        {
            if (print_disable_api_envvar != NULL)
            {
                ret_val = strtoul(print_disable_api_envvar, NULL, 0);
                free(print_disable_api_envvar);
            }
        }

        return ret_val;
    }
}

int get_tool_perf_envvar(void)
{
    char * perf_envvar = NULL;
    if (getenv(__CLARMOR_PERFSTAT_MODE__) == NULL)
    {
        return 0;
    }
    else
    {
        int ret_val;
        if (!get_env_util(&perf_envvar, __CLARMOR_PERFSTAT_MODE__))
        {
            if (perf_envvar != NULL)
            {
                ret_val = strtol(perf_envvar, NULL, 0);
                free(perf_envvar);
            }
            else
                ret_val = 0;
        }
        else
            ret_val = 0;

        return ret_val;
    }
}

char* get_tool_perf_outfile_envvar(void)
{
    char * perf_envvar = NULL;
    if (getenv(__CLARMOR_PERFSTAT_OUTFILE__) == NULL)
    {
        return 0;
    }
    else
    {
        char *ret_val;
        if (!get_env_util(&perf_envvar, __CLARMOR_PERFSTAT_OUTFILE__))
        {
            if (perf_envvar != NULL)
            {
                ret_val = perf_envvar;
            }
            else
                ret_val = NULL;
        }
        else
            ret_val = NULL;

        return ret_val;
    }
}

const char *get_logging_envvar(void)
{
    char * logging_envvar = NULL;
    if (getenv(__CLARMOR_LOG_LOCATION__) != NULL)
    {
        if (!get_env_util(&logging_envvar, __CLARMOR_LOG_LOCATION__))
        {
            if (logging_envvar != NULL)
                return logging_envvar;
        }
    }
    return NULL;
}

const char *get_log_prefix_envvar(void)
{
    char * log_prefix_envvar = NULL;
    if (getenv(__CLARMOR_LOG_PREFIX__) != NULL)
    {
        if (!get_env_util(&log_prefix_envvar, __CLARMOR_LOG_PREFIX__))
        {
            if (log_prefix_envvar != NULL)
                return log_prefix_envvar;
        }
    }
    return NULL;
}

unsigned int get_check_on_device_envvar(void)
{
    char * dev_check_envvar = NULL;
    if (getenv(__CLARMOR_DEVICE_SELECT__) == NULL)
        return DEFAULT_DEVICE_CHECK;
    else
    {
        unsigned int ret_val = DEFAULT_DEVICE_CHECK;
        if (!get_env_util(&dev_check_envvar, __CLARMOR_DEVICE_SELECT__))
        {
            if (dev_check_envvar != NULL)
            {
                ret_val = strtoul(dev_check_envvar, NULL, 0);
                free(dev_check_envvar);
            }
        }
        return ret_val;
    }
}

int get_print_backtrace_envvar(void)
{
    char * print_backtrace_envvar = NULL;
    if (getenv(__BACKTRACE__) == NULL)
        return 0;
    else
    {
        unsigned int ret_val = 0;
        if (!get_env_util(&print_backtrace_envvar, __BACKTRACE__))
        {
            if (print_backtrace_envvar != NULL)
            {
                ret_val = strtoul(print_backtrace_envvar, NULL, 0);
                free(print_backtrace_envvar);
            }
        }

        return ret_val;
    }
}

/*
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
*/

char* get_backtrace_human_readable(void* bt, char* bt_symbol)
{
    char *syscom; // command line to get the human-readable backtrace
    char *output; // human-readable backtrace

    int p = strcspn(bt_symbol, "( ");

    int num_bytes = asprintf(&syscom,
            "addr2line %p -i -s -p -C -e %.*s | sed 's/ .inlined by. //'",
            bt, p, bt_symbol);
    CHECK_ASPRINTF_RET(num_bytes);
    if (syscom == NULL)
        return NULL;

    // Before we call the command line, we need to remove any LD_PRELOAD
    char *save_environ = NULL;
    char *old_environ = getenv("LD_PRELOAD");
    if (old_environ != NULL)
    {
        num_bytes = asprintf(&save_environ, "%s", old_environ);
        CHECK_ASPRINTF_RET(num_bytes);
    }
    if (unsetenv("LD_PRELOAD"))
    {
        det_fprintf(stderr, "Failed to unset LD_PRELOAD when getting function name.\n");
        if (save_environ != NULL)
            free(save_environ);
        if (syscom != NULL)
            free(syscom);
        return NULL;
    }

    // Run the addr2line program.
    FILE *function_name = popen(syscom, "re");

    // Get program output
    // Only pull in 4096 bytes, because we likely don't need more than that
    // to see useful information from addr2line.
    output = (char *)calloc(4096, sizeof(char));
    if ( (fgets(output, 4096, function_name) != NULL) )
        output[strcspn(output, "\n")] = '\0'; // replace first newline with string end.

    output[4095]='\0'; // Just in case we didn't see a NULL from the fgets.
    pclose(function_name);

    if (save_environ != NULL)
    {
        if (setenv("LD_PRELOAD", save_environ, 1))
            det_fprintf(stderr, "Failed to reset LD_PRELOAD to %s when getting function name.\n", save_environ);
        free(save_environ);
    }
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
        det_fprintf(stderr, "Asking for an illegal backtrace level: %d\n", level);
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
        det_fprintf(stderr, "%s (%d) error: asprintf failed\n", __func__, __LINE__);
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

    det_fprintf(where_to, "BACKTRACE:\n");

    strings = backtrace_symbols(buffer, nptrs);

    for (j = nptrs-1; j > 0; j--)
    {
        char *bt_string = get_backtrace_human_readable(buffer[j], strings[j]);
        det_fprintf(where_to, "%s - %s\n", strings[j], bt_string);
        free(bt_string);
    }
    free(strings);
}


int is_nvidia_platform(cl_context context)
{
    cl_int cl_err;
    size_t size_dev, size_platform;
    cl_device_id *device;
    cl_platform_id *platform;
    size_t platform_name_len = 0;
    char *platform_name;

    cl_err = clGetContextInfo(context, CL_CONTEXT_DEVICES,
        0, NULL, &size_dev);
    check_cl_error(__FILE__, __LINE__, cl_err);
    device = malloc(size_dev);

    cl_err = clGetContextInfo(context, CL_CONTEXT_DEVICES,
        size_dev, device, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);

    cl_err = clGetDeviceInfo(device[0], CL_DEVICE_PLATFORM,
        0, NULL, &size_platform);
    check_cl_error(__FILE__, __LINE__, cl_err);
    platform = malloc(size_platform);

    cl_err = clGetDeviceInfo(device[0], CL_DEVICE_PLATFORM,
        size_platform, platform, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);

    cl_err = clGetPlatformInfo(platform[0], CL_PLATFORM_NAME, 0, NULL,
            &platform_name_len);
    check_cl_error(__FILE__, __LINE__, cl_err);

    platform_name = calloc(platform_name_len, sizeof(char));

    cl_err = clGetPlatformInfo(platform[0], CL_PLATFORM_NAME,
        platform_name_len, platform_name, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);

    char *found;
    found = strstr(platform_name, "NVIDIA");
    int ret = (found) ? 1 : 0;
    free(platform_name);
    free(device);
    free(platform);

    return ret;
}

int opencl_broken_images(void)
{
    char * broken_images_envvar = NULL;
    if (getenv(__CLARMOR_ROCM_HAWAII__) == NULL)
        return 0;
    else
    {
        unsigned int ret_val = 0;
        if (!get_env_util(&broken_images_envvar, __CLARMOR_ROCM_HAWAII__))
        {
            if (broken_images_envvar != NULL)
            {
                ret_val = strtoul(broken_images_envvar, NULL, 0);
                free(broken_images_envvar);
            }
        }
        return ret_val;
    }
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
#ifdef CL_VERSION_1_2
        case CL_UNORM_INT24:
            byteChannel = 3;
            break;
#endif
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
#ifdef CL_VERSION_1_2
        case CL_DEPTH:
#endif
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
#ifdef CL_VERSION_1_2
        case CL_DEPTH_STENCIL:
            numChannels = 1;
            break;
#endif
        default:
            numChannels = 0;
    }

    return byteChannel * numChannels;
}

int det_printf(const char * format, ...)
{
    int err;
    int ret = 0;
    va_list args;

    const char * pref = get_log_prefix_envvar();
    if(pref)
    {
        err = printf("%s", pref);
        if(err < 0) return err;
        ret += err;
    }

    va_start(args, format);
    err = vprintf(format, args);
    if(err < 0)
    {
        va_end(args);
        return err;
    }
    ret += err;
    va_end(args);

    return ret;
}

int det_fprintf(FILE * stream, const char * format, ...)
{
    int err;
    int ret = 0;
    va_list args;

    const char * pref = get_log_prefix_envvar();
    if(pref)
    {
        err = fprintf(stream, "%s", pref);
        if(err < 0) return err;
        ret += err;
    }

    va_start(args, format);
    err = vfprintf(stream, format, args);
    if(err < 0)
    {
        va_end(args);
        return err;
    }
    ret += err;
    va_end(args);

    return ret;
}

int det_vfprintf(FILE * stream, const char * format, va_list arg )
{
    int err;
    int ret = 0;

    const char * pref = get_log_prefix_envvar();
    if(pref)
    {
        err = fprintf(stream, "%s", pref);
        if(err < 0) return err;
        ret += err;
    }

    err = vfprintf(stream, format, arg);
    if(err < 0) return err;
    ret += err;

    return ret;
}

char *get_clarmor_version(void)
{
    char *version = NULL;
    int num_bytes = asprintf(&version, "%s", CLARMOR_VERSION);
    CHECK_ASPRINTF_RET(num_bytes);
    if (version == NULL)
    {
        det_fprintf(stderr, "%s (%d) error: asprintf failed\n",
                __func__, __LINE__);
        return NULL;
    }
    return version;
}
