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

#ifndef __OVERFLOW_ERROR_H
#define __OVERFLOW_ERROR_H

#include "meta_data_lists/cl_kernel_lists.h"

// Open and initialize the buffer overflow detector's logging file.
// If the environment variable DETECTOR_LOG_LOCATION is set, this function
// attempts to open that file for writing. Throughout the program, any
// buffer overflow detector outputs will be printed to stderr and to this
// file.
// If the DETECTOR_LOG_LOCATION envvar is not set, we only print outputs to
// stderr and do not attempt to open any log files.
void initialize_logging(void);

// Call this at the end of the buffer overflow detector application (ideally
// during a destructor) so that we can print out some final information to
// the log file.
void finalize_detector(void);

void apiImageOverflowError(char * const func, void * const buffer, int x, int y, int z);

void apiOverflowError(char * const func, void * const buffer, const unsigned bad_byte);

/*
 * error message for detected overflow
 */
void overflowError(const kernel_info * const kernInfo,
        void * const buffer,
        const unsigned bad_byte);

// Print out a warning about having duplicated arguments. Use this after
// you print out an error so that the user will know if there the error
// could have potentially happened in one of the duplicates.
void printDupeWarning(const cl_kernel kern, const uint32_t * const dupe);

// This function will optionally exit the program.
// It checks the environment variable that tells us whether to die or not.
// Does nothing if the environment variable tells us to move on.
void optionalKillOnOverflow(const int err_ret_val, pthread_t parent);

#endif //__OVERFLOW_ERROR_H
