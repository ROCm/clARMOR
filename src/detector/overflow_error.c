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

#define _GNU_SOURCE
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <CL/cl.h>
#include <errno.h>
#include <signal.h>

#include "util_functions.h"
#include "cl_err.h"
#include "detector_defines.h"
#include "meta_data_lists/cl_kernel_lists.h"

#include "overflow_error.h"

#define RED_TEXT    "\x1b[31m"
#define CYAN_TEXT   "\x1b[36m"
#define BLACK_TEXT  "\x1b[30m"
#define RED_BG      "\x1b[41m"
#define CYAN_BG     "\x1b[46m"
#define YELLOW_BG   "\x1b[43m"
#define RESET_TEXT  "\x1b[0m"


static FILE* log_file = NULL;
static uint64_t buffer_overflows_observed = 0;

void initialize_logging(void)
{
    if (log_file != NULL)
        return;
    const char * log_loc = get_logging_envvar();
    if (log_loc == NULL)
        return;
    log_file = fopen(log_loc, "w");
    if (log_file != NULL)
    {
        fprintf(log_file, "Beginning buffer overflow detection run.\n");
    }
    else
    {
        int err_val = errno;
        det_fprintf(stderr, "Could not open detector log file: %s\n", log_loc);
        det_fprintf(stderr, "   %s\n", strerror(err_val));
    }
}

void finalize_detector(void)
{
    if (log_file != NULL)
    {
        fprintf(log_file, "Buffer overflow detection complete.\n");
        fprintf(log_file, "Found a total of %llu errors.\n",
                (long long unsigned int)buffer_overflows_observed);
    }
}

static void print_err_header(void)
{
    det_fprintf(stderr, "\n");
    det_fprintf(stderr, CYAN_TEXT RED_BG "ATTENTION:" RESET_TEXT "\n");
}

static void print_warn_header(void)
{
    det_fprintf(stderr, "\n");
    det_fprintf(stderr, BLACK_TEXT YELLOW_BG "WARNING:" RESET_TEXT "\n");
}

static void print_err_footer(void)
{
    det_fprintf(stderr, "\n");
}

static void print_and_log_err(const char * format, ...)
{
    va_list args;

    va_start(args, format);
    det_vfprintf(stderr, format, args);
    va_end(args);

    if (log_file != NULL)
    {
        va_start(args, format);
        vfprintf(log_file, format, args);
        va_end(args);
    }
}

/*
 * finds the 3 dimensional location for an index in a flattened image canary
 * take image object and bad byte
 * return x,y,z location of overflow
 * x or y = -1 when not in a real data range (eg for x = -1 & y = 2, the index is 2 rows into the column canary region
 * all -1 indicates an error
 */
static void getImageCanaryXYZ(cl_memobj *img, unsigned bad_byte, int *x, int *y, int *z)
{
    size_t dataSize;
    *x = -1;
    *y = -1;
    *z = -1;

    dataSize = getImageDataSize(&img->image_format);
    if (dataSize == 0)
        return;

    uint32_t dataIndex = bad_byte / dataSize;
    uint32_t i_lim, j_lim, k_lim;
    uint32_t j_dat, k_dat;
    i_lim = img->image_desc.image_width;
    j_lim = img->image_desc.image_height;
    k_lim = img->image_desc.image_depth;
    j_dat = (j_lim == 1) ? 1 : j_lim - IMAGE_POISON_HEIGHT;
    k_dat = (k_lim == 1) ? 1 : k_lim - IMAGE_POISON_DEPTH;

    uint32_t j, k;
    uint32_t accume = 0;

    for(k=0; k < k_dat; k++)
    {
        for(j=0; j < j_dat; j++)
        {
            accume += IMAGE_POISON_WIDTH;
            if(dataIndex < accume)
            {
                *x = dataIndex - (accume - IMAGE_POISON_WIDTH);
                *y = j;
                *z = k;
                //row canary
                break;
            }

        }
        if(dataIndex < accume)
            break;

        accume += IMAGE_POISON_HEIGHT * i_lim;
        if(dataIndex < accume)
        {
            *y = (dataIndex - (accume - IMAGE_POISON_HEIGHT * i_lim)) / i_lim;
            *z = k;
            //column canary
            break;
        }
    }
    if(k == k_dat)
    {
        *z = (dataIndex - accume) / (i_lim * j_lim);
        //depth canary
    }
}

void apiImageOverflowError(char * const func, void * const buffer, int x, int y, int z)
{
    print_err_header();
    print_and_log_err("************* Buffer overflow detected ***********\n");
    buffer_overflows_observed++;

    print_and_log_err("%s: Buffer: %p\n", func, buffer);
    if(x >= 0)
        print_and_log_err("   First dimension overflow at row %d, depth %d, %d column(s) past end.\n", y, z, x+1);
    else if(y >= 0)
        print_and_log_err("   Second dimension overflow at depth %d, %d row(s) past end.\n", z, y+1);
    else
        print_and_log_err("   Third dimension overflow %d slice(s) past end.\n", z+1);

    char * backtrace_str = NULL;
    if(get_print_backtrace_envvar())
    {
        //clEnqueue->apiImageOverflowCheck->apiImageOverflowError
        backtrace_str = get_backtrace_level(3);
    }

    if(backtrace_str)
        print_and_log_err("%s\n", backtrace_str);

    if(backtrace_str)
        free(backtrace_str);

    print_err_footer();
}

void apiOverflowError(char * const func, void * const buffer, const unsigned bad_byte)
{
    print_err_header();
    print_and_log_err("************* Buffer overflow detected ***********\n");
    buffer_overflows_observed++;

    cl_memobj *m1 = cl_mem_find(get_cl_mem_alloc(), buffer);
    if(m1)
    {
        print_and_log_err("%s: Buffer: %p\n", func, buffer);
        print_and_log_err("   First observed writing %u byte(s) past the end.\n",
                bad_byte+1);
    }
    else
    {
        // Can't get buffer name for SVM pointer.
        print_and_log_err("%s: SVM pointer: %p\n", func, buffer);
        print_and_log_err("   First observed writing %u byte(s) past the end.\n",
                bad_byte+1);
    }

    char * backtrace_str = NULL;
    if(get_print_backtrace_envvar())
    {
        //clEnqueue->apiBufferOverflowCheck->apiOverflowError
        backtrace_str = get_backtrace_level(3);
    }

    if(backtrace_str)
        print_and_log_err("%s\n", backtrace_str);

    if(backtrace_str)
        free(backtrace_str);

    print_err_footer();
}

/*
 * for a given buffer handle, find it's location in the kernel argument list
 * returns the buffer's index
 *
 */
int getBufferIndex(kernel_arg *argList, void* buffer)
{
    kernel_arg *temp = argList;
    while(temp != NULL)
    {
        if(temp->buffer != NULL)
        {
            if(temp->buffer->handle == buffer)
                return temp->handle;
        }
        else if(temp->svm_buffer != NULL)
        {
            if(temp->svm_buffer->handle == buffer)
                return temp->handle;
        }
        temp = temp->next;
    }
    return -1;
}

/*
 * print the message for a discovered overflow
 *
 */
void overflowError(const kernel_info * const kernInfo,
        void * const buffer,
        const unsigned bad_byte,
        char * const backtrace_str)
{
    cl_int cl_err;
    size_t size_ret = 0;

    print_err_header();
    print_and_log_err("************* Buffer overflow detected ***********\n");
    buffer_overflows_observed++;

    // Get Kernel Name
    char *kernelName = NULL;
    cl_err = clGetKernelInfo(kernInfo->handle, CL_KERNEL_FUNCTION_NAME, 0,
            NULL, &size_ret);
    check_cl_error(__FILE__, __LINE__, cl_err);
    if (size_ret == 0)
    {
        det_fprintf(stderr, "Bad calloc size (%lu) at %s:%d\n",
                size_ret, __FILE__, __LINE__);
        exit(-1);
    }
    kernelName = calloc(size_ret, sizeof(char));
    if (kernelName == NULL)
    {
        det_fprintf(stderr, "Calloc failed at %s:%d (size: %lu)\n",
                __FILE__, __LINE__, size_ret);
        exit(-1);
    }
    cl_err = clGetKernelInfo(kernInfo->handle, CL_KERNEL_FUNCTION_NAME,
            size_ret, kernelName, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);

    int argIndex = getBufferIndex(kernInfo->arg_list, buffer);
    if(argIndex >= 0)
    {
        // Get Buffer Name
        char *bufferName = NULL;
#ifdef CL_VERSION_1_2
        cl_err = clGetKernelArgInfo(kernInfo->handle, argIndex,
                CL_KERNEL_ARG_NAME, 0, NULL, &size_ret);
        if (cl_err != CL_KERNEL_ARG_INFO_NOT_AVAILABLE)
        {
            check_cl_error(__FILE__, __LINE__, cl_err);

            if (size_ret == 0)
            {
                det_fprintf(stderr, "Bad calloc size (%lu) at %s:%d\n",
                        size_ret, __FILE__, __LINE__);
                exit(-1);
            }
            bufferName = calloc(size_ret, sizeof(char));
            if (kernelName == NULL)
            {
                det_fprintf(stderr, "Calloc failed at %s:%d (size: %lu)\n",
                        __FILE__, __LINE__, size_ret);
                exit(-1);
            }

            cl_err = clGetKernelArgInfo(kernInfo->handle, argIndex,
                    CL_KERNEL_ARG_NAME, size_ret, bufferName, NULL);
            check_cl_error(__FILE__, __LINE__, cl_err);
        }
        else
        {
            int num_bytes = asprintf(&bufferName, "Argument %d", argIndex);
            CHECK_ASPRINTF_RET(num_bytes);
        }
#else
        int num_bytes = asprintf(&bufferName, "Argument %d", argIndex);
        CHECK_ASPRINTF_RET(num_bytes);
#endif

        cl_memobj *m1 = cl_mem_find(get_cl_mem_alloc(), buffer);
        if(m1 && m1->is_image)
        {
            int x,y,z;
            getImageCanaryXYZ(m1, bad_byte, &x, &y, &z);

            print_and_log_err("Kernel: %s, Buffer: %s\n", kernelName, bufferName);
            if(x >= 0)
                print_and_log_err("   First dimension overflow at row %d, depth %d, %d column(s) past end.\n", y, z, x+1);
            else if(y >= 0)
                print_and_log_err("   Second dimension overflow at depth %d, %d row(s) past end.\n", z, y+1);
            else
                print_and_log_err("   Third dimension overflow %d slice(s) past end.\n", z+1);
        }
        else
        {
            print_and_log_err("Kernel: %s, Buffer: %s\n", kernelName, bufferName);
            print_and_log_err("   First observed writing %u byte(s) past the end.\n",
                    bad_byte+1);
        }
        // Free before leaving.
        if (bufferName != NULL)
            free(bufferName);

    }
    else
    {
        // Can't get buffer name for SVM pointer.
        print_and_log_err("Kernel: %s, SVM pointer: %p\n", kernelName, buffer);
        print_and_log_err("   First observed writing %u byte(s) past the end.\n",
                bad_byte+1);
    }

    if(backtrace_str)
        print_and_log_err("%s\n", backtrace_str);
    print_err_footer();


    // Free before leaving.
    if (kernelName != NULL)
        free(kernelName);
}

void printDupeWarning(const cl_kernel kern, const uint32_t * const dupe)
{
    cl_int cl_err;
    uint32_t i, nargs;
    cl_err = clGetKernelInfo(kern, CL_KERNEL_NUM_ARGS, sizeof(nargs), &nargs,
            NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);

    int saw_a_duplicate = 0;
    for(i = 0; i < nargs; i++)
    {
        if (dupe[i] != i)
            saw_a_duplicate = 1;
    }

    if (!saw_a_duplicate)
        return;

    print_warn_header();
    print_and_log_err("Some of this kernel's arguments point to identical "
            "buffers.\n");
    print_and_log_err("An overflow in one argument may therefore be "
            "detected in the other argument with identical buffer.\n");

    for(i = 0; i < nargs; i++)
    {
        if (dupe[i] == i)
            continue;
        // else we've hit a duplicate argument.
        char *origArgName = NULL;
        char *cloneArgName = NULL;

        // Get original argument's name
        //det_fprintf(stderr, "Dupe[%d] = %d\n", i, dupe[i]);
#ifdef CL_VERSION_1_2
        size_t size_ret = 0;
        cl_err = clGetKernelArgInfo(kern, dupe[i], CL_KERNEL_ARG_NAME,
                0, NULL, &size_ret);
        if (cl_err != CL_KERNEL_ARG_INFO_NOT_AVAILABLE)
        {
            check_cl_error(__FILE__, __LINE__, cl_err);
            if (size_ret == 0)
            {
                det_fprintf(stderr, "Bad calloc size (%lu) at %s:%d\n",
                        size_ret, __FILE__, __LINE__);
                exit(-1);
            }
            origArgName = calloc(size_ret, sizeof(char));
            if (origArgName == NULL)
            {
                det_fprintf(stderr, "Calloc failed at %s:%d (size: %lu)\n",
                        __FILE__, __LINE__, size_ret);
                exit(-1);
            }
            cl_err = clGetKernelArgInfo(kern, dupe[i], CL_KERNEL_ARG_NAME,
                    size_ret, origArgName, NULL);
            check_cl_error(__FILE__, __LINE__, cl_err);

            // Get second (cloned) argument's name
            cl_err = clGetKernelArgInfo(kern, i, CL_KERNEL_ARG_NAME, 0, NULL,
                    &size_ret);
            check_cl_error(__FILE__, __LINE__, cl_err);
            if (size_ret == 0)
            {
                det_fprintf(stderr, "Bad calloc size (%lu) at %s:%d\n",
                        size_ret, __FILE__, __LINE__);
                exit(-1);
            }
            cloneArgName = calloc(size_ret, sizeof(char));
            if (cloneArgName == NULL)
            {
                det_fprintf(stderr, "Calloc failed at %s:%d (size: %lu)\n",
                        __FILE__, __LINE__, size_ret);
                exit(-1);
            }
            cl_err = clGetKernelArgInfo(kern, i, CL_KERNEL_ARG_NAME, size_ret,
                    cloneArgName, NULL);
            check_cl_error(__FILE__, __LINE__, cl_err);
        }
        else
        {
            int num_bytes = asprintf(&origArgName, "Argument %d", dupe[i]);
            CHECK_ASPRINTF_RET(num_bytes);
            num_bytes = asprintf(&cloneArgName, "Argument %d", i);
            CHECK_ASPRINTF_RET(num_bytes);
        }
#else
        int num_bytes = asprintf(&origArgName, "Argument %d", dupe[i]);
        CHECK_ASPRINTF_RET(num_bytes);
        num_bytes = asprintf(&cloneArgName, "Argument %d", i);
        CHECK_ASPRINTF_RET(num_bytes);
#endif

        print_and_log_err("    %s (argument %lu) is the same buffer as %s "
            "(argument %lu)\n", cloneArgName, i, origArgName, dupe[i]);
        // Free before leaving.
        if (cloneArgName != NULL)
            free(cloneArgName);
        if (origArgName != NULL)
            free(origArgName);
    }
    det_printf("\n");
}

void optionalKillOnOverflow(const int err_ret_val, pthread_t parent)
{
    if (get_error_envvar())
    {
        det_fprintf(stderr, "Exiting application because of buffer overflow.\n");
        if(parent)
            pthread_kill(parent, 1);
        exit(err_ret_val);
    }
}

