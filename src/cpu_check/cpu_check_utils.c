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

#include <stdint.h>

#include "detector_defines.h"
#include "cl_err.h"
#include "check_utils.h"
#include "overflow_error.h"
#include "util_functions.h"

#include "cpu_check_utils.h"

static uint32_t divide_ceiling(uint32_t a, uint32_t b)
{
    uint32_t ret = a / b;
    if(a % b)
        ret += 1;
    return ret;
}

void cpu_parse_canary(cl_command_queue cmd_queue, uint32_t check_len,
        uint32_t *map_ptr, kernel_info *kern_info, void *buffer,
        uint32_t *dupe)
{
    uint32_t max_to_check = divide_ceiling(check_len, sizeof(uint32_t));
    for(uint32_t p = 0; p < max_to_check; p++)
    {
        uint32_t word = *(((uint32_t*)map_ptr) + p);

        if (word == poisonFill_32b)
            continue;

        //find the first byte from this word that mismatched
        for(uint32_t q = 0; q < sizeof(uint32_t); q++)
        {
            uint8_t byte = *(((uint8_t*)map_ptr) + sizeof(uint32_t)*p + q);
            if(byte != poisonFill_8b && sizeof(uint32_t)*p + q < check_len)
            {
                char * backtrace_str = NULL;
                if(get_print_backtrace_envvar())
                {
                    //clEnqueueNDRangeKernel->kernelLaunchFunc->verifyBufferInBounds->verify_buffer_on_host->verify_cl_mem->cpu_parse_canary
                    backtrace_str = get_backtrace_level(5);
                }

                overflowError(kern_info, buffer, sizeof(uint32_t)*p + q, backtrace_str);
                printDupeWarning(kern_info->handle, dupe);
                optionalKillOnOverflow(get_exitcode_envvar(), 0);
                mendCanaryRegion(cmd_queue, buffer, CL_TRUE, 0, NULL, NULL);

                if(backtrace_str)
                    free(backtrace_str);

                break;
            }
        }
        break;
    }
}
