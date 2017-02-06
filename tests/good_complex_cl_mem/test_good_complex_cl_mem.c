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


// A test to make sure that programs without a buffer overflow but with more
// complex cl_mem access patterns still work.
#include "common_test_functions.h"

const char *kernel_source = "\n"\
"__kernel void test_single(__global uint *cl_mem_buffer, uint len) {\n"\
"    uint i = get_global_id(0);\n"\
"    if (i < len) {\n"\
"        cl_mem_buffer[i] = i;\n"\
"    }\n"\
"}\n"\
"\n"\
"__kernel void test_access_second(__global uint *first_buffer,\n"\
"               __global uint *second_buffer, uint len) {\n"\
"    uint i = get_global_id(0);\n"\
"    if (i < len) {\n"\
"        first_buffer[i] = i;\n"\
"    }\n"\
"    second_buffer[i] = i;\n"\
"}\n"\
"\n"\
"__kernel void test_alias(__global uint *first_buffer,\n"\
"               __global uint *second_buffer, uint len) {\n"\
"    uint i = get_global_id(0);\n"\
"    if (i < len) {\n"\
"        second_buffer[i] = i;\n"\
"    }\n"\
"}\n"\
"__kernel void basic_test(__global uint *buffer, uint len,\n"\
"              uint stride) {\n"\
"    uint i = get_global_id(0);\n"\
"    if (i < len && (i % stride) == 0) {\n"\
"        buffer[i] = i;\n"\
"    }\n"\
"}\n";


// This test will run two kernels, one after the other.
void run_back_to_back(const cl_context context,
        const cl_command_queue cmd_queue, const cl_program program,
        uint64_t buffer_size)
{
    cl_int cl_err;

    cl_mem buffer = clCreateBuffer(context, CL_MEM_READ_WRITE,
        buffer_size,  NULL, &cl_err);
    check_cl_error(__FILE__, __LINE__, cl_err);

    // Each work item will touch sizeof(cl_uint) bytes.
    // This calculates how many work items we can have and still stay within
    // the nominal buffer size.
    uint64_t num_entries_in_buf = buffer_size / sizeof(cl_uint);
    size_t work_items_to_use;
    if (num_entries_in_buf > SIZE_MAX)
    {
        work_items_to_use = SIZE_MAX;
        fprintf(stderr, "\n\nWARNING -- TEST WILL NOT WORK PROPERLY.\n");
        fprintf(stderr, "\tYou are asking for too large a buffer.\n");
        fprintf(stderr, "\tReduce the buffer size to let the test to work.\n");
        fprintf(stderr, "\n\n");
    }
    else
        work_items_to_use = num_entries_in_buf;

    printf("Running back-to-back test.\n");

    cl_kernel test_kernel = setup_kernel(program, "test_single");
    uint64_t bytes_written = (uint64_t)work_items_to_use * sizeof(cl_uint);

    cl_err = clSetKernelArg(test_kernel, 0, sizeof(cl_mem), &buffer);
    check_cl_error(__FILE__, __LINE__, cl_err);

    printf("Launching %lu work items to write %llu entries in the buffer.\n",
            work_items_to_use, (long long unsigned)num_entries_in_buf);
    printf("This will write %llu out of %llu bytes in the buffer.\n",
            (long long unsigned)bytes_written,
            (long long unsigned)buffer_size);
    cl_err = clSetKernelArg(test_kernel, 1, sizeof(cl_uint),
            &work_items_to_use);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clEnqueueNDRangeKernel(cmd_queue, test_kernel, 1, NULL,
        &work_items_to_use, NULL, 0, NULL, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);

    printf("Second kernel.\n");
    bytes_written = (uint64_t)work_items_to_use * sizeof(cl_uint);
    printf("Launching %lu work items to write %llu entries in the buffer.\n",
            work_items_to_use, (long long unsigned)num_entries_in_buf);
    printf("This will write %llu out of %llu bytes in the buffer.\n",
            (long long unsigned)bytes_written,
            (long long unsigned)buffer_size);
    cl_err = clSetKernelArg(test_kernel, 1, sizeof(cl_uint), &work_items_to_use);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clEnqueueNDRangeKernel(cmd_queue, test_kernel, 1, NULL,
        &work_items_to_use, NULL, 0, NULL, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);

    clFinish(cmd_queue);
    printf("Done running back-to-back test.\n\n");

    cl_err = clReleaseKernel(test_kernel);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clReleaseMemObject(buffer);
    check_cl_error(__FILE__, __LINE__, cl_err);
}

// Run a test that does strided accesses.
void run_strided_test(const cl_context context,
        const cl_command_queue cmd_queue, const cl_program program,
        uint64_t buffer_size)
{
    cl_int cl_err;

    cl_mem buffer = clCreateBuffer(context, CL_MEM_READ_WRITE,
        buffer_size,  NULL, &cl_err);
    check_cl_error(__FILE__, __LINE__, cl_err);

    // Each work item will touch sizeof(cl_uint) bytes.
    // This calculates how many work items we can have and still stay within
    // the nominal buffer size.
    uint64_t num_entries_in_buf = buffer_size / sizeof(cl_uint);
    size_t work_items_to_use;
    if (num_entries_in_buf > SIZE_MAX)
    {
        work_items_to_use = SIZE_MAX;
        fprintf(stderr, "\n\nWARNING -- TEST WILL NOT WORK PROPERLY.\n");
        fprintf(stderr, "\tYou are asking for too large a buffer.\n");
        fprintf(stderr, "\tReduce the buffer size to let the test to work.\n");
        fprintf(stderr, "\n\n");
    }
    else
        work_items_to_use = num_entries_in_buf;

    printf("Running strided test.\n");

    cl_kernel test_kernel = setup_kernel(program, "basic_test");
    uint64_t bytes_written = (uint64_t)work_items_to_use * sizeof(cl_uint);

    unsigned int stride = 5;
    cl_err = clSetKernelArg(test_kernel, 0, sizeof(cl_mem), &buffer);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clSetKernelArg(test_kernel, 1, sizeof(cl_uint),
            &work_items_to_use);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clSetKernelArg(test_kernel, 2, sizeof(cl_uint), &stride);
    check_cl_error(__FILE__, __LINE__, cl_err);

    printf("Launching %lu work items to write %llu entries in the buffer.\n",
            work_items_to_use, (long long unsigned)num_entries_in_buf);
    printf("This will write %llu out of %llu bytes in the buffer.\n",
            (long long unsigned)bytes_written,
            (long long unsigned)buffer_size);
    printf("Using a stride of: %u\n", stride);

    cl_err = clEnqueueNDRangeKernel(cmd_queue, test_kernel, 1, NULL,
        &work_items_to_use, NULL, 0, NULL, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);

    clFinish(cmd_queue);
    printf("Done running strided test.\n\n");

    cl_err = clReleaseKernel(test_kernel);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clReleaseMemObject(buffer);
    check_cl_error(__FILE__, __LINE__, cl_err);
}


// This test will create two buffers and try to to access them.
void run_two_buffer_tests(const cl_context context,
        const cl_command_queue cmd_queue, const cl_program program,
        uint64_t buffer_size)
{
    cl_int cl_err;

    // All of the buffers that we create are the correct size.
    cl_mem first_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE,
        buffer_size,  NULL, &cl_err);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_mem second_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE,
        buffer_size,  NULL, &cl_err);
    check_cl_error(__FILE__, __LINE__, cl_err);

    // Each work item will touch sizeof(cl_uint) bytes.
    // This calculates how many work items we can have and still stay within
    // the nominal buffer size.
    uint64_t num_entries_in_buf = buffer_size / sizeof(cl_uint);
    size_t work_items_to_use;
    if (num_entries_in_buf > SIZE_MAX)
    {
        work_items_to_use = SIZE_MAX;
        fprintf(stderr, "\n\nWARNING -- TEST WILL NOT WORK PROPERLY.\n");
        fprintf(stderr, "\tYou are asking for too large a buffer.\n");
        fprintf(stderr, "\tReduce the buffer size to let the test to work.\n");
        fprintf(stderr, "\n\n");
    }
    else
        work_items_to_use = num_entries_in_buf;

    // First test -- pass in two buffers to the kernel.
    printf("Running first two-buffer test.\n");

    cl_kernel test_kernel_1 = setup_kernel(program, "test_access_second");
    uint64_t bytes_written = (uint64_t)work_items_to_use * sizeof(cl_uint);

    cl_err = clSetKernelArg(test_kernel_1, 0, sizeof(cl_mem), &first_buffer);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clSetKernelArg(test_kernel_1, 1, sizeof(cl_mem), &second_buffer);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clSetKernelArg(test_kernel_1, 2, sizeof(cl_uint), &work_items_to_use);
    check_cl_error(__FILE__, __LINE__, cl_err);

    printf("Launching %lu work items to write %llu entries in 2nd buffer.\n",
            work_items_to_use, (long long unsigned)num_entries_in_buf);
    printf("This will write %llu out of %llu bytes in the buffer.\n",
            (long long unsigned)bytes_written,
            (long long unsigned)buffer_size);
    cl_err = clEnqueueNDRangeKernel(cmd_queue, test_kernel_1, 1, NULL,
        &work_items_to_use, NULL, 0, NULL, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);
    clFinish(cmd_queue);
    printf("Done running first two-buffer test.\n");

    // Second test - pass in two buffers.
    printf("\n\n");
    printf("Running the second two-buffer test.\n");
    printf("Launching %lu work items to write %llu entries in both buffers.\n",
            work_items_to_use, (long long unsigned)num_entries_in_buf);
    printf("This will write %llu out of %llu bytes in the buffer.\n",
            (long long unsigned)bytes_written,
            (long long unsigned)buffer_size);
    cl_err = clSetKernelArg(test_kernel_1, 2, sizeof(cl_uint), &work_items_to_use);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clEnqueueNDRangeKernel(cmd_queue, test_kernel_1, 1, NULL,
            &work_items_to_use, NULL, 0, NULL, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);
    clFinish(cmd_queue);
    printf("Done running second two-buffer test.\n");
    cl_err = clReleaseKernel(test_kernel_1);
    check_cl_error(__FILE__, __LINE__, cl_err);

    // Third test - pass the same buffer as both arguments
    printf("\n\n");
    printf("Running the third two-buffer test.\n");
    cl_kernel test_kernel_2 = setup_kernel(program, "test_alias");
    bytes_written = (uint64_t)work_items_to_use * sizeof(cl_uint);

    cl_err = clSetKernelArg(test_kernel_2, 0, sizeof(cl_mem), &first_buffer);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clSetKernelArg(test_kernel_2, 1, sizeof(cl_mem), &first_buffer);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clSetKernelArg(test_kernel_2, 2, sizeof(cl_uint), &work_items_to_use);
    check_cl_error(__FILE__, __LINE__, cl_err);

    printf("Launching %lu work items to write %llu entries in the buffer.\n",
            work_items_to_use, (long long unsigned)num_entries_in_buf);
    printf("This will write %llu out of %llu bytes in the buffer.\n",
            (long long unsigned)bytes_written,
            (long long unsigned)buffer_size);
    cl_err = clEnqueueNDRangeKernel(cmd_queue, test_kernel_2, 1, NULL,
            &work_items_to_use, NULL, 0, NULL, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);
    clFinish(cmd_queue);
    printf("Done running third two-buffer test.\n\n");

    cl_err = clReleaseKernel(test_kernel_1);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clReleaseMemObject(first_buffer);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clReleaseMemObject(second_buffer);
    check_cl_error(__FILE__, __LINE__, cl_err);
}

int main(int argc, char** argv)
{
    uint32_t platform_to_use = 0;
    uint32_t device_to_use = 0;
    cl_device_type dev_type = CL_DEVICE_TYPE_DEFAULT;
    uint64_t buffer_size = DEFAULT_BUFFER_SIZE;

    // Check input options.
    check_opts(argc, argv, "Complex cl_mem without Overflow",
            &platform_to_use, &device_to_use, &dev_type);

    // Set up the OpenCL environment.
    cl_platform_id platform = setup_platform(platform_to_use);
    cl_device_id device = setup_device(device_to_use, platform_to_use,
            platform, dev_type);
    cl_context context = setup_context(platform, device);
    cl_command_queue cmd_queue = setup_cmd_queue(context, device);

    // Build the program
    cl_program program = setup_program(context, 1, &kernel_source, device);

    // Run the actual tests.
    printf("\n\nRunning Good Complex cl_mem Test...\n");
    printf("    Using buffer size: %llu\n", (long long unsigned)buffer_size);

    run_back_to_back(context, cmd_queue, program, buffer_size);

    run_two_buffer_tests(context, cmd_queue, program, buffer_size);

    run_strided_test(context, cmd_queue, program, buffer_size);

    printf("Done Running Good Complex cl_mem Test.\n");
    return 0;
}
