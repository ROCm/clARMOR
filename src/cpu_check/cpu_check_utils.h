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

#ifndef __CPU_CHECK_UTILS_H
#define __CPU_CHECK_UTILS_H

#include <stdint.h>
#include <CL/cl.h>

#include "meta_data_lists/cl_kernel_lists.h"

// Checks a series of canaries on the CPU. If any of them differe, this
// function will print out an error, optionally print to the global log file,
// and will optionally exit the program. The are set in the env. variables
// queried by get_logging_envvar() and get_error_envvar(), respedctively.
// Inputs:
//      cmd_queue: the command queue that can put work onto the device with the
//                  canaries. Will be used to update the canaries if any of
//                  them were corrupted and we want to continue execution.
//      check_len: The number of bytes of canaries values to check.
//      map_ptr: the buffer which contains the canaries to check
//      kern_info:  Structure which holds information about the kernel which
//                  last wrote to the buffer. Used for printing out debug
//                  information if there is a buffer overflow.
//      buffer: The handle for the original cl_mem, SVM, or cl_image that
//              we're checking. Used to print debug information.
//      dupe:   List of arguments in the kernel that are duplicates. This is
//              used when printing out information about buffers overflows.
//              The syntax of each entry in the array is "the first kernel arg
//              that is this memory buffer". So if dupe[i]==i, where i is arg
//              number, this is the first arg that points to that buffer.
void cpu_parse_canary(cl_command_queue cmd_queue, uint32_t check_len,
        uint32_t *map_ptr, kernel_info *kern_info, void *buffer,
        uint32_t *dupe);

#endif // __CPU_CHECK_UTILS_H
