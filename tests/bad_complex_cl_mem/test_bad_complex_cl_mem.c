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


// A test to make sure that programs with a buffer overflow complete and
// cause the buffer overflow detector to find the overflow.
#include "common_test_functions.h"

const char *kernel_source = "\n"\
"__kernel void test_single(__global uint *cl_mem_buffer, uint len) {\n"\
"    uint i = get_global_id(0);\n"\
"    if (i < len) {\n"\
"        cl_mem_buffer[i] = i;\n"\
"    }\n"\
"}\n"\
"\n"\
"__kernel void test_overflow_second(__global uint *first_buffer,\n"\
"               __global uint *second_buffer, uint len) {\n"\
"    uint i = get_global_id(0);\n"\
"    if (i < len) {\n"\
"        first_buffer[i] = i;\n"\
"    }\n"\
"    second_buffer[i] = i;\n"\
"}\n"\
"\n"\
"__kernel void test_alias_overflow(__global uint *first_buffer,\n"\
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


// This test will run two kernels, one after the other. The first will have
// a buffer overflow. The second will use the same buffer but not cause a
// buffer overflow. This test is to verify that you only detect a buffer
// overflow on the first kernel.
// We expect to see only one overflow here.
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
    // However, we shrank the real buffer size above, so overflow territory.
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
    printf("Should find only a single overflow, after the first kernel.\n");

    cl_kernel test_kernel = setup_kernel(program, "test_single");
    size_t overflow_work_items = work_items_to_use + 10;
    uint64_t bytes_written = (uint64_t)overflow_work_items * sizeof(cl_uint);

    cl_err = clSetKernelArg(test_kernel, 0, sizeof(cl_mem), &buffer);
    check_cl_error(__FILE__, __LINE__, cl_err);

    printf("Launching %lu work items to write %llu entries in the buffer.\n",
            overflow_work_items, (long long unsigned)num_entries_in_buf);
    printf("This will write %llu out of %llu bytes in the buffer.\n",
            (long long unsigned)bytes_written,
            (long long unsigned)buffer_size);
    cl_err = clSetKernelArg(test_kernel, 1, sizeof(cl_uint),
            &overflow_work_items);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clEnqueueNDRangeKernel(cmd_queue, test_kernel, 1, NULL,
        &overflow_work_items, NULL, 0, NULL, NULL);
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
        &overflow_work_items, NULL, 0, NULL, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);

    clFinish(cmd_queue);
    printf("Done running back-to-back test.\n\n");

    cl_err = clReleaseKernel(test_kernel);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clReleaseMemObject(buffer);
    check_cl_error(__FILE__, __LINE__, cl_err);
}

// Run a test that buffer overflows with a strided access.
// Now the first byte past the end of the buffer is not the one that
// is the overflow.
// We expect one buffer overflow in this test.
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
    // However, we shrank the real buffer size above, so overflow territory.
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
    printf("Should find only a single overflow.\n");

    cl_kernel test_kernel = setup_kernel(program, "basic_test");
    size_t overflow_work_items = work_items_to_use + 10;
    uint64_t bytes_written = (uint64_t)overflow_work_items * sizeof(cl_uint);

    unsigned int stride = 5;
    cl_err = clSetKernelArg(test_kernel, 0, sizeof(cl_mem), &buffer);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clSetKernelArg(test_kernel, 1, sizeof(cl_uint),
            &overflow_work_items);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clSetKernelArg(test_kernel, 2, sizeof(cl_uint), &stride);
    check_cl_error(__FILE__, __LINE__, cl_err);

    printf("Launching %lu work items to write %llu entries in the buffer.\n",
            overflow_work_items, (long long unsigned)num_entries_in_buf);
    printf("This will write %llu out of %llu bytes in the buffer.\n",
            (long long unsigned)bytes_written,
            (long long unsigned)buffer_size);
    printf("Using a stride of: %u\n", stride);

    cl_err = clEnqueueNDRangeKernel(cmd_queue, test_kernel, 1, NULL,
        &overflow_work_items, NULL, 0, NULL, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);

    clFinish(cmd_queue);
    printf("Done running strided test.\n\n");

    cl_err = clReleaseKernel(test_kernel);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clReleaseMemObject(buffer);
    check_cl_error(__FILE__, __LINE__, cl_err);
}


// This test will create two buffers and try to run buffer overflow tests
// on them.
// In the first test, the only buffer overflow is on the second buffer.
// This test is to make sure that we don't incorrectly find errors in the
// first buffer.
// The second test has overflows in both buffers, to ensure that we can
// check multiple buffers in an application and find both errors.
// The third test attempts to cause a buffer overflow when a single buffer
// is passed in as both arguments.
void run_two_buffer_tests(const cl_context context,
        const cl_command_queue cmd_queue, const cl_program program,
        uint64_t buffer_size)
{
    cl_int cl_err;

    // All of the buffers that we create are the correct size.
    // We will cause buffer overflows by launching too many work items and
    // not properly checking the buffer edges.
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

    // First test -- pass in two buffers to the kernel, only overflow
    // the second one. We should only see a single buffer overflow from this.
    // And that overflow should be on "second_buffer"
    printf("Running first two-buffer test.\n");
    printf("Should find an error in the second argument.\n");

    cl_kernel test_kernel_1 = setup_kernel(program, "test_overflow_second");
    size_t overflow_work_items = work_items_to_use + 10;
    uint64_t bytes_written = (uint64_t)overflow_work_items * sizeof(cl_uint);

    cl_err = clSetKernelArg(test_kernel_1, 0, sizeof(cl_mem), &first_buffer);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clSetKernelArg(test_kernel_1, 1, sizeof(cl_mem), &second_buffer);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clSetKernelArg(test_kernel_1, 2, sizeof(cl_uint), &work_items_to_use);
    check_cl_error(__FILE__, __LINE__, cl_err);

    printf("Launching %lu work items to write %llu entries in 2nd buffer.\n",
            overflow_work_items, (long long unsigned)num_entries_in_buf);
    printf("This will write %llu out of %llu bytes in the buffer.\n",
            (long long unsigned)bytes_written,
            (long long unsigned)buffer_size);
    printf("First buffer will only have %lu work items and won't overflow.\n",
            work_items_to_use);
    cl_err = clEnqueueNDRangeKernel(cmd_queue, test_kernel_1, 1, NULL,
        &overflow_work_items, NULL, 0, NULL, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);
    clFinish(cmd_queue);
    printf("Done running first two-buffer test.\n");

    // Second test - pass in two buffers and overflow both of them.
    // Should have tow buffer overflows detected.
    printf("\n\n");
    printf("Running the second two-buffer test.\n");
    printf("Should find errors in both arguments.\n");
    printf("Launching %lu work items to write %llu entries in both buffers.\n",
            overflow_work_items, (long long unsigned)num_entries_in_buf);
    printf("This will write %llu out of %llu bytes in the buffer.\n",
            (long long unsigned)bytes_written,
            (long long unsigned)buffer_size);
    cl_err = clSetKernelArg(test_kernel_1, 2, sizeof(cl_uint), &overflow_work_items);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clEnqueueNDRangeKernel(cmd_queue, test_kernel_1, 1, NULL,
            &overflow_work_items, NULL, 0, NULL, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);
    clFinish(cmd_queue);
    printf("Done running second two-buffer test.\n");
    cl_err = clReleaseKernel(test_kernel_1);
    check_cl_error(__FILE__, __LINE__, cl_err);

    // Third test - pass the same buffer as both arguments
    // We will see the buffer overflow only in the first one, even though
    // the kernel writes only to the second argument. It's the same buffer.
    printf("\n\n");
    printf("Running the third two-buffer test.\n");
    printf("Should find an error in either argument. They are clones.\n");
    cl_kernel test_kernel_2 = setup_kernel(program, "test_alias_overflow");
    overflow_work_items = work_items_to_use + 10;
    bytes_written = (uint64_t)overflow_work_items * sizeof(cl_uint);

    cl_err = clSetKernelArg(test_kernel_2, 0, sizeof(cl_mem), &first_buffer);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clSetKernelArg(test_kernel_2, 1, sizeof(cl_mem), &first_buffer);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clSetKernelArg(test_kernel_2, 2, sizeof(cl_uint), &overflow_work_items);
    check_cl_error(__FILE__, __LINE__, cl_err);

    printf("Launching %lu work items to write %llu entries in the buffer.\n",
            overflow_work_items, (long long unsigned)num_entries_in_buf);
    printf("This will write %llu out of %llu bytes in the buffer.\n",
            (long long unsigned)bytes_written,
            (long long unsigned)buffer_size);
    printf("Both buffers will see the same overflow.\n");
    printf("We will catch it in the first buffer only, however.\n");
    cl_err = clEnqueueNDRangeKernel(cmd_queue, test_kernel_2, 1, NULL,
            &overflow_work_items, NULL, 0, NULL, NULL);
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
    check_opts(argc, argv, "Complex cl_mem with Overflow",
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
    printf("\n\nRunning Bad Complex cl_mem Test...\n");
    printf("    Using buffer size: %llu\n", (long long unsigned)buffer_size);

    // This one should find one overflow.
    run_back_to_back(context, cmd_queue, program, buffer_size);

    // This one should find four overflows.
    run_two_buffer_tests(context, cmd_queue, program, buffer_size);

    // Try a basic buffer overflow but with strided accesses.
    run_strided_test(context, cmd_queue, program, buffer_size);

    printf("Done Running Bad Complex cl_mem Test.\n");
    return 0;
}
