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


/*! \file util_functions.h
 * Fetch environment variables and back-trace.
 */

#ifndef __UTIL_FUNCTIONS__
#define __UTIL_FUNCTIONS__

#include <CL/cl.h>
#include <stdio.h>
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define __CLARMOR_EXIT_ON_OVERFLOW__ "CLARMOR_EXIT_ON_OVERFLOW"
#define __ERROR_EXITCODE__ "CLARMOR_ERROR_EXITCODE"
#define __CLARMOR_LOG_LOCATION__ "CLARMOR_LOG_LOCATION"
#define __CLARMOR_LOG_PREFIX__ "CLARMOR_LOG_PREFIX"

#define __CLARMOR_ALTERNATE_GPU_DETECTION__ "CLARMOR_ALTERNATE_GPU_DETECTION"
#define GPU_MODE_DEFAULT        0
#define GPU_MODE_MULTI_SVMPTR   0
#define GPU_MODE_MULTI_BUFFER   1
#define GPU_MODE_SINGLE_BUFFER  2

#define __CLARMOR_PERFSTAT_MODE__ "CLARMOR_PERFSTAT_MODE"
#define STATS_KERN_ENQ_TIME     1
#define STATS_CHECKER_TIME      2
#define STATS_MEM_OVERHEAD      4

#define __BACKTRACE__ "CLARMOR_PRINT_BACKTRACE"
#define __CLARMOR_DISABLE_API_CHECK__ "CLARMOR_DISABLE_API_CHECK"

extern uint32_t global_tool_stats_flags;

#define __CLARMOR_DEVICE_SELECT__ "CLARMOR_DEVICE_SELECT"

#define DEFAULT_DEVICE_CHECK 0
#define DEVICE_GPU 1
#define DEVICE_CPU 2

#define CHECK_ASPRINTF_RET(num_bytes) \
{ \
    if (num_bytes <= 0) \
    { \
        det_fprintf(stderr, "asprintf in %s:%d failed\n", __FILE__, __LINE__); \
            exit(-1); \
    }\
}

/*!
 * Get the environment variable that tells the buffer overflow detector
 * to crash when it sees a buffer overflow.
 * If that variable is not found, or if it is set to '0', then we only print
 * out a warning on buffer overflow detection and continue operation.
 *
 * \return
 *      0 if the tool should not error on overflow.
 *      1 if the tool *should* error on overflow.
 *      -1 on error.
 */
int get_error_envvar(void);

/*!
 * Get the environment variable that tells the buffer overflow detector
 * which exitcode to send when crashing from a buffer overflow
 *
 * \return
 *      0 default, no environment variable.
 */
int get_exitcode_envvar(void);

/*!
 * Retrieve CLARMOR_ALTERNATE_GPU_DETECTION from environment
 *
 * \return
 *      0 default
 */
int get_gpu_strat_envvar(void);

/*!
 * Get the environment variable that tells the buffer overflow detector
 * to disable api checking
 *
 * \return
 *      0 default, no environment variable.
 */
int get_disable_api_check_envvar(void);

/*!
 * Retrieve CLARMOR_PERFSTAT_MODE from environment
 *
 * \return
 *      0 default
 */
int get_tool_perf_envvar(void);

/*!
 * Get the environment variable that tells the buffer overflow detector
 * where (if anywhere) to log its output.
 * If that variable is not found, then we will not log anything.
 *
 * \return
 *      string holding the file name if the envvar is found.
 *      NULL if the environment variable isn't found.
 */
const char *get_logging_envvar(void);

/*!
 * Get the environment variable that tells the buffer overflow detector
 * the prefix string for overflow error outputs.
 *
 * \return
 *      prefix string
 *      NULL if the environment variable isn't found.
 */
const char *get_log_prefix_envvar(void);

/*!
 * Get the environment variable that tells the buffer overflow detector when
 * to run the checks on the host or the device. This is set with the
 * environment variable "CLARMOR_DEVICE_SELECT".
 * setting this to 1 results in always running on the device.
 * setting this to 2 results in always running on the host.
 * Setting the environment variable to 0 results in the default (clARMOR decides),
 * As does leaving the environment variable unset.
 *
 * You may want to change this because the CPU has the benefit of running
 * faster when there are fewer buffers, since it has better serial performance
 * and has very little launch overhead to amortize. The device (in particular,
 * if it's a GPU) is better when there are more buffers, since the checks would
 * have ample parallel work.
 *
 * \return
 *      default 0
 */
unsigned int get_check_on_device_envvar(void);

/*!
 * Get the environment variable that tells the buffer overflow detector
 * to show a backtrace for each overflow error.
 *
 * \return
 *      0 default, no environment variable.
 */
int get_print_backtrace_envvar(void);

/*!
 * This function is used to get a single level of backtrace in the program.
 * It looks up the file and line number for the backtrace and returns it as a
 * C string.
 *
 * \param level
 *      How many levels to go back in the backtrace. Example: '1'
 *      tells you what function called you. '2' tells you what
 *      function called that.
 *
 * \return
 *      NULL on failure. Otherwise, returns a pointer to the backtrace
 *      string that describes the level requested.
 *      It is up to the caller to free this string.
 */
char* get_backtrace_level(int level);

/*!
 *
 * \param where_to
 *      output to file
 */
void print_backtrace(FILE* where_to);

/*!
 *
 * \param stop
 *      stop time
 * \param start
 *      start time
 *
 * \return
 *      time difference in micro seconds
 */
uint64_t timeval_diff_us(struct timeval *stop, struct timeval *start);

/*!
 *
 * \param format
 *      image format
 *
 * \return
 *      size of image format in bytes
 */
unsigned getImageDataSize(const cl_image_format *format);

/*!
 * printf mutator, adds prefix to each printout
 * det is for detector
 *
 */
int det_printf(const char * format, ...);

/*!
 * fprintf mutator, adds prefix to each printout
 *
 */
int det_fprintf(FILE * stream, const char * format, ...);

/*!
 * vfprintf mutator, adds prefix to each printout
 *
 */
int det_vfprintf(FILE * stream, const char * format, va_list arg );

/*!
 * Tell whether we are using the AMD APP SDK from before 2.9 or not.
 *
 */
int is_app_sdk_2_9(void);

/*!
 * Tell whether we are using the AMD APP SDK 3.0+.
 *
 */
int is_app_sdk_3_0(void);

/*!
 * Returns the version number of clARMOR that is set at build time.
 *
 */
char *get_clarmor_version(void);

#ifdef __cplusplus
}
#endif

#endif //__UTIL_FUNCTIONS__
