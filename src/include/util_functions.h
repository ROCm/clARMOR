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

#ifndef __UTIL_FUNCTIONS__
#define __UTIL_FUNCTIONS__

#include <CL/cl.h>
#include <stdio.h>
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define __EXIT_ON_OVERFLOW__ "EXIT_ON_OVERFLOW"
#define __DETECTOR_LOG_LOCATION__ "DETECTOR_LOG_LOCATION"

#define __USE_ALTERNATE_GPU_DETECTION__ "USE_ALTERNATE_GPU_DETECTION"
#define GPU_MODE_MULTI_SVMPTR   0
#define GPU_MODE_MULTI_BUFFER   1
#define GPU_MODE_SINGLE_BUFFER  2

#define __TOOL_PERF_STATISTIC_MODE__ "TOOL_PERF_STATISTIC_MODE"
#define STATS_KERN_ENQ_TIME     1
#define STATS_CHECKER_TIME      2
#define STATS_MEM_OVERHEAD      4

extern uint32_t global_tool_stats_flags;

#define __DETECTOR_DEVICE_SELECT__ "DETECTOR_DEVICE_SELECT"
// We found that 8 buffers was a good cutoff for CPU vs. GPU checks.
// However, you can set the "DEVICE_CHECK_CUTOFF" environment variable
// to override this.
#define DEFAULT_DEVICE_CHECK 8
#define DEVICE_GPU 1
#define DEVICE_CPU 2

#define CHECK_ASPRINTF_RET(num_bytes) \
{ \
    if (num_bytes <= 0) \
    { \
        fprintf(stderr, "asprintf in %s:%d failed\n", __FILE__, __LINE__); \
            exit(-1); \
    }\
}

// Get the environment variable that tells the buffer overflow detector
// to crash when it sees a buffer overflow.
// If that variable is not found, or if it is set to '0', then we only print
// out a warning on buffer overflow detection and continue operation.
// Returns 0 if the tool should not error on overflow.
// Returns 1 if the tool *should* error on overflow.
// Returns -1 on error.
int get_error_envvar(void);

int get_gpu_strat_envvar(void);

int get_tool_perf_envvar(void);

// Get the environment variable that tells the buffer overflow detector
// where (if anywhere) to log its output.
// If that variable is not found, then we will not log anything.
// Returns string holding the file name if the envvar is found.
// Returns NULL if the environment variable isn't found.
const char *get_logging_envvar(void);

// Get the environment variable that tells the buffer overflow detector when
// to run the checks on the host or the device. This is set with the
// environment variable "DEVICE_CHECK_CUTOFF".
// The actual value returns should be used as "If there are ret_val buffers,
// I should run on the device." In other words, setting this to 1 would
// result in *always* running on the device.
// Setting the environment variable to 0 results in the default,
// "DEFAULT_DEVICE_CHECK" being returned. As does leaving the environment
// variable unset.
//
// You may want to change this because the CPU has the benefit of running
// faster when there are fewer buffers, since it has better serial performance
// and has very little launch overhead to amortize. The device (in particular,
// if it's a GPU) is better when there are more buffers, since the checks would
// have ample parallel work.
unsigned int get_check_on_device_envvar(void);

// This function is used to get a single level of backtrace in the program.
// It looks up the file and line number for the backtrace and returns it as a
// C string.
// Inputs:
//     int level -- How many levels to go back in the backtrace. Example: '1'
//                  tells you what function called you. '2' tells you what
//                  function called that.
// Outputs:
//      Returns NULL on failure. Otherwise, returns a pointer to the backtrace
//      string that describes the level requested.
//      It is up to the caller to free this string.
char* get_backtrace_level(int level);

void print_backtrace(FILE* where_to);

uint64_t timeval_diff_us(struct timeval *stop, struct timeval *start);

/*
 * return size of image format in bytes
 */
unsigned getImageDataSize(const cl_image_format *format);

#ifdef __cplusplus
}
#endif

#endif //__UTIL_FUNCTIONS__
