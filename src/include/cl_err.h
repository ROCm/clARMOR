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


/*! \file cl_err.h
 * Functions descriptive cl error reporting.
 */

#ifndef _CL_ERR_H_
#define _CL_ERR_H_

#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#include <CL/cl.h>

/*!
 * This function takes an OpenCL error value, from a cl_int, and translates
 * it into the string that describes the error.
 * Argument: a cl_int that is returned from an OpenCL function
 * Returns: a C string that describes the return value.
 *
 * \param err
 *      error number
 * \return String for error number
 */
const char *cluErrorString(const cl_int err);

/*!
 * This function checks a cl_int return value from an OpenCL API and prints
 * out an error message if it is not equal to CL_SUCCESS. It also exits
 * the program with '-1' if the API did not succeed.
 *
 * \param file_name
 *      name of file where function is called
 * \param line_num
 *      line number where function is called
 * \param cl_err
 *      cl error number
 */
void check_cl_error(const char * const file_name, const int line_num,
        const cl_int cl_err);

/*!
 * If a clBuildProgram fails with CL_BUILD_PROGRAM_FAILURE, this will print
 * out the error log from the compiler to stderr.
 *
 * \param context
 *      program build context
 * \param prog
 *      program being built
 * \param cl_err
 *      cl error returned from build command
 */
void print_program_build_err(cl_context context, cl_program prog,
        cl_int cl_err);

#endif // _CL_ERR_H_
