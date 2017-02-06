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

#ifndef __COMMON_TEST_FUNCTIONS_H
#define __COMMON_TEST_FUNCTIONS_H

// Functions that are used in numerous tests for the AMD Research Buffer
// Overflow detector. This includes things like setting up OpenCL platforms and
// devices, plus debug prints and things of that nature.A

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <stdint.h>
#include <CL/cl.h>
#include "cl_err.h"

// The default buffer we want to use is 1 MB
#define DEFAULT_BUFFER_SIZE 1048576

// Check the argc and argv options for this benchmark using GNU getopt_long.
// Arguments:
//      argc - the number of command line arguments, from main()
//      argv - the command line arguments, from main()
//      description - a string that describes this benchmark, to be printed if
//                    the user requests the help menu.
//      *platform - the variable into which we will return the requested number
//                  of the OpenCL platform.
//      *device - the variable into which we will return the requested number
//                  of the OpenCL device.
void check_opts(const int argc, char** argv, const char * description,
    uint32_t * const platform, uint32_t * const device,
    cl_device_type * const dev_type);

// Set up the OpenCL platform that the benchmark needs to run.
// Argument:
//      platform_to_use - A numerical request for which platform this benchmark
//                        wants to use. The application will fail if this is
//                        larger than the number of platforms in the system.
// Returns:
//      The requested cl_platform_id from the OpenCL system.
// Exits the application on error.
cl_platform_id setup_platform(const uint32_t platform_to_use);

// Set up the OpenCL device that the benchmark needs to run.
// Arguments:
//      device_to_use - A numerical request for which platform this benchmark
//                      wants to use. The application will fail if this is
//                      larger than the number of platforms in the system.
//      platform_to_use - The numerical value of the platform, same as what is
//                        passed into setup_platform(). Used for debug.
//      platform - the cl_platform_id on which we want to find the device.
// Returns:
//      The requested cl_device_id from the OpenCL system.
// Exits the application on error.
cl_device_id setup_device(const uint32_t device_to_use,
        const uint32_t platform_to_use, const cl_platform_id platform,
        const cl_device_type dev_type);

// Set up the OpenCL context that this benchmark needs to run.
// Arguments:
//      platform - The cl_platform_id on which we want to set up the context
//      device - The cl_device_id on which we want to set up this context
// Returns:
//      The requested cl_context from the OpenCL system.
// Exits the application on error.
cl_context setup_context(const cl_platform_id platform,
        const cl_device_id device);

// Set an OpenCL command queue that this benchmark needs to run.
// Arguments:
//      context - the cl_context associated with the new command queue
//      device - the cl_device_id on which commands will be queued
// Returns:
//      The requested cl_command_queue from this device and context.
// Exits the application on error.
cl_command_queue setup_cmd_queue(const cl_context context,
        const cl_device_id device);

// Creates and builds an OpenCL C kernel program from the supplied source.
// Arguments:
//      context - The OpenCL context used for this program
//      num_source_strings - The number of strings that make up the source code
//      source - A pointer to the strings that make up the source code
//      device - The device for which to compile the source code
// Returns:
//      The program that has been created annd built.
// Exits the application on error.
cl_program setup_program(const cl_context context,
        const cl_uint num_source_strings, const char **source,
        const cl_device_id device);

// Creates a kernel from the program passed in, based on the requested name.
// Arguments:
//      program - the cl_program that contains this kernel
//      kernel_name - a C string containing the name of the kernel
// Returns:
//      Returns the cl_kernel based on the input parameters
// Exits the application on error.
cl_kernel setup_kernel(const cl_program program, const char *kernel_name);

// Returns the maximum width of an OpenCL image on this device and in this
// implementation. This is only for width, not height or depth.
// Arguments:
//      device - The cl_device_id that we are checking.
//      image_dimensions - 1, 2, or 3. Is this a 1D, a 2D, or a 3D image?
// Returns:
//      Returns the maximum width of an n-dimensional image on this system.
// Exits the application on error.
size_t get_image_width(const cl_device_id device, const int image_dimensions);

// Returns the maximum height of an OpenCL image on this device and in this
// implementation. This is only for height, not width or depth.
// Arguments:
//      device - The cl_device_id that we are checking.
//      image_dimensions - 2, or 3. Is this a 2D or a 3D image?
// Returns:
//      Returns the maximum height of an n-dimensional image on this system.
// Exits the application on error.
size_t get_image_height(const cl_device_id device, const int image_dimensions);

// Returns the maximum depth of an OpenCL 3D image on this device and in this
// implementation.
// Arguments:
//      device - The cl_device_id that we are checking.
// Returns:
//      Returns the maximum depth of an 3D image on this system.
// Exits the application on error.
size_t get_image_depth(const cl_device_id device);

// Outputs a fake buffer overflow detector log file with the correct number
// of errors written into it. This log file may contain the minimal amount
// of information needed to past a 'make test' check.
// This is primarily designed to be used when we need to skip a test. For
// instance, if the runtime does not support OpenCL 2.0 features, we want to
// essentially skip tests for that feature without failing. As such, we need
// a lot file that contains the correct information.
// Arguments:
//      filename - The file name of the fake log file to write.
//      num_errors - The number of fake buffer overflows to include in the log
// Exits the application on error.
void output_fake_errors(const char* filename, const unsigned int num_errors);

// Returns size in bytes of the image format. Different formats can have
// various numbers of channels and various bytes per channel. As such,
// this function will calculate the number of bytes in each pixel of an image.
// Arguments:
//      format - The image format we want to analyze
// Returns:
//      Returns the number of bytes for each pixel of this image
unsigned int get_image_data_size(const cl_image_format * const format);

#endif // __COMMON_TEST_FUNCTIONS_H
