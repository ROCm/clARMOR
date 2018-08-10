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

#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "detector_defines.h"
#include "util_functions.h"
#include "cpu_check_cl_mem.h"
#include "cpu_check_cl_svm.h"
#include "cpu_check_cl_image.h"

#include "cpu_check.h"

static void start_profile(struct timeval* start)
{
    (void)start;
    if(global_tool_stats_flags & STATS_CHECKER_TIME)
    {
        gettimeofday(start, NULL);
    }
}
static void stop_profile_and_print(struct timeval* start, struct timeval* stop)
{
    if(global_tool_stats_flags & STATS_CHECKER_TIME)
    {
        FILE *perf_out_f;

        gettimeofday(stop, NULL);
        uint64_t durr_us = timeval_diff_us(stop, start);

        perf_out_f = fopen(global_tool_stats_outfile, "a");

        fprintf(perf_out_f, "%lu\n", durr_us);
        fclose(perf_out_f);
    }
}

void verify_buffer_on_host(uint32_t num_cl_mem, uint32_t num_svm,
        uint32_t num_images, void **buffer_ptrs, void **image_ptrs,
        kernel_info *kern_info, uint32_t *dupe, const cl_event *evt)
{
    struct timeval stop, start;

    start_profile(&start);

    verify_cl_mem(kern_info, num_cl_mem, buffer_ptrs, dupe, evt);

    verify_images(kern_info, num_images, image_ptrs, dupe, evt);

    verify_svm(kern_info, num_svm, &buffer_ptrs[num_cl_mem], dupe, evt);

    stop_profile_and_print(&start, &stop);
}
