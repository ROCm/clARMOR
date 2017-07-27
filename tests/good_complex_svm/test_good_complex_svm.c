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


// A test to make sure that programs that use OpenCL 2.0 device enqueue
// work properly and that we can properly detect buffer overflows that are
// caused by the child kernel. This is the "good" version of the benchmark,
// so expect no overflows.
#include "common_test_functions.h"

const char *dev_q_kernel_source = "\n"\
"__kernel void child_kern(__global uint *svm_buffer, uint len) {\n"\
"    uint i = get_global_id(0);\n"\
"    if (i < len) {\n"\
"        svm_buffer[i] = i;\n"\
"    }\n"\
"}\n"\
"\n"\
"__kernel void test_dev_enq(__global uint *svm_buffer, uint len,\n"\
"              uint work_items, queue_t queue) {\n"\
"    uint i = get_global_id(0);\n"\
"    if (i == 0) {\n"\
"        ndrange_t child_ndr = ndrange_1D(work_items);\n"\
"        enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_NO_WAIT, child_ndr,\n"\
"           ^{child_kern(svm_buffer, len);});\n"\
"    }\n"\
"}\n";

const char *kernel_source = "\n"\
"__kernel void child_kern(__global uint *svm_buffer, uint len) {\n"\
"    uint i = get_global_id(0);\n"\
"    if (i < len) {\n"\
"        svm_buffer[i] = i;\n"\
"    }\n"\
"}\n"\
"\n"\
"__kernel void basic_test(__global uint *svm_buffer1,\n"\
"              __global uint *svm_buffer2, uint len, uint stride) {\n"\
"    uint i = get_global_id(0);\n"\
"    if (i < len && (i % stride) == 0) {\n"\
"        svm_buffer2[i] = i;\n"\
"    }\n"\
"}\n";

#ifdef CL_VERSION_2_0
// This test will cause use device enqueue. It launches a single work item
// from the host. that work item will launch a child kernel with many
// work items.
void run_device_enqueue_test(const cl_device_id device,
        const cl_context context, const cl_command_queue cmd_queue,
        const cl_program program, cl_svm_mem_flags flags,
        uint64_t buffer_size)
{
    cl_int cl_err;

    // Create the device queue.
    cl_queue_properties dev_properties[] = {CL_QUEUE_PROPERTIES,
        CL_QUEUE_ON_DEVICE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE |
            CL_QUEUE_ON_DEVICE_DEFAULT, 0};
    cl_command_queue device_queue = clCreateCommandQueueWithProperties(
            context, device, dev_properties, &cl_err);
    check_cl_error(__FILE__, __LINE__, cl_err);

    cl_kernel test_kernel = setup_kernel(program, "test_dev_enq");

    // We are going to launch a single work item to the device, and then
    // later, that work item will launch a much larger kernel using a
    // device enqueue.
    // Each of those work item will touch sizeof(cl_uint) bytes.
    // This calculates how many work items we can have and still stay within
    // the nominal buffer size.
    uint64_t num_entries_in_buf = buffer_size / sizeof(cl_uint);
    if (num_entries_in_buf > SIZE_MAX)
    {
        num_entries_in_buf = SIZE_MAX / (sizeof(cl_uint) * 2);
        buffer_size = num_entries_in_buf * sizeof(cl_uint);
    }
    size_t work_items_to_use = num_entries_in_buf;

    printf("Running device enqueue SVM buffer test.\n");

    void *good_buffer = clSVMAlloc(context, flags, buffer_size, 0);
    if (good_buffer == NULL)
    {
        fprintf(stderr, "clSVMAlloc near %s:%d failed.\n", __FILE__, __LINE__);
        exit(-1);
    }

    cl_err = clSetKernelArgSVMPointer(test_kernel, 0, good_buffer);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clSetKernelArg(test_kernel, 1, sizeof(cl_uint), &buffer_size);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clSetKernelArg(test_kernel, 2, sizeof(cl_uint), &work_items_to_use);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clSetKernelArg(test_kernel, 3, sizeof(cl_command_queue),
            &device_queue);
    check_cl_error(__FILE__, __LINE__, cl_err);

    uint64_t bytes_written = (uint64_t)work_items_to_use * sizeof(cl_uint);
    printf("Launching one and then %zu work items to write %llu entries.\n",
            work_items_to_use, (long long unsigned)num_entries_in_buf);
    printf("This will write %llu out of %llu bytes in the buffer.\n",
            (long long unsigned)bytes_written,
            (long long unsigned)buffer_size);
    size_t parent_work_items = 1;
    cl_err = clEnqueueNDRangeKernel(cmd_queue, test_kernel, 1, NULL,
        &parent_work_items, NULL, 0, NULL, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);
    clFinish(cmd_queue);
    printf("Done running device enqueue test.\n\n\n");

    // Clean up
    cl_err = clReleaseKernel(test_kernel);
    check_cl_error(__FILE__, __LINE__, cl_err);
    clSVMFree(context, good_buffer);
    cl_err = clReleaseCommandQueue(device_queue);
    check_cl_error(__FILE__, __LINE__, cl_err);
}

// This will run two SVM-using kernels back to back.
void run_svm_back_to_back(const cl_context context,
        const cl_command_queue cmd_queue, const cl_program program,
        cl_svm_mem_flags flags, uint64_t buffer_size)
{
    cl_int cl_err;

    cl_kernel test_kernel = setup_kernel(program, "child_kern");

    // Each of the work items will touch sizeof(cl_uint) bytes.
    // This calculates how many work items we can have and still stay within
    // the nominal buffer size.
    uint64_t num_entries_in_buf = buffer_size / sizeof(cl_uint);
    if (num_entries_in_buf > SIZE_MAX)
    {
        num_entries_in_buf = SIZE_MAX / (sizeof(cl_uint) * 2);
        buffer_size = num_entries_in_buf * sizeof(cl_uint);
    }
    size_t work_items_to_use = num_entries_in_buf;

    printf("Running back-to-back SVM buffer test.\n");

    void *good_buffer = clSVMAlloc(context, flags, buffer_size, 0);
    if (good_buffer == NULL)
    {
        fprintf(stderr, "clSVMAlloc near %s:%d failed.\n", __FILE__, __LINE__);
        exit(-1);
    }

    cl_err = clSetKernelArgSVMPointer(test_kernel, 0, good_buffer);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clSetKernelArg(test_kernel, 1, sizeof(cl_uint), &work_items_to_use);
    check_cl_error(__FILE__, __LINE__, cl_err);

    printf("Launching first kernel.\n");
    uint64_t bytes_written = (uint64_t)work_items_to_use * sizeof(cl_uint);
    printf("Launching %zu work items to write %llu entries.\n",
            work_items_to_use, (long long unsigned)num_entries_in_buf);
    printf("This will write %llu out of %llu bytes in the buffer.\n",
            (long long unsigned)bytes_written,
            (long long unsigned)buffer_size);
    cl_err = clEnqueueNDRangeKernel(cmd_queue, test_kernel, 1, NULL,
        &work_items_to_use, NULL, 0, NULL, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);
    clFinish(cmd_queue);

    printf("Launching second kernel.\n");
    work_items_to_use = 1;
    cl_err = clSetKernelArg(test_kernel, 1, sizeof(cl_uint), &work_items_to_use);
    check_cl_error(__FILE__, __LINE__, cl_err);

    bytes_written = (uint64_t)work_items_to_use * sizeof(cl_uint);
    printf("Launching %zu work items to write %llu entries.\n",
            work_items_to_use, (long long unsigned)num_entries_in_buf);
    printf("This will write %llu out of %llu bytes in the buffer.\n",
            (long long unsigned)bytes_written,
            (long long unsigned)buffer_size);
    cl_err = clEnqueueNDRangeKernel(cmd_queue, test_kernel, 1, NULL,
        &work_items_to_use, NULL, 0, NULL, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);

    clFinish(cmd_queue);
    printf("Done running back-to-back SVM buffer test.\n\n\n");

    // Clean up
    cl_err = clReleaseKernel(test_kernel);
    check_cl_error(__FILE__, __LINE__, cl_err);
    clSVMFree(context, good_buffer);
}

// This test will create two SVM buffers and run without problem.
// The stride variable means that every
void run_two_svm_test(const cl_context context,
        const cl_command_queue cmd_queue, const cl_program program,
        cl_svm_mem_flags flags, uint64_t buffer_size,
        const unsigned int stride)
{
    cl_int cl_err;

    cl_kernel test_kernel = setup_kernel(program, "basic_test");

    // Each of the work items will touch sizeof(cl_uint) bytes.
    // This calculates how many work items we can have and still stay within
    // the nominal buffer size.
    uint64_t num_entries_in_buf = buffer_size / sizeof(cl_uint);
    if (num_entries_in_buf > SIZE_MAX)
    {
        num_entries_in_buf = SIZE_MAX / (sizeof(cl_uint) * 2);
        buffer_size = num_entries_in_buf * sizeof(cl_uint);
    }
    size_t work_items_to_use = num_entries_in_buf;

    printf("Running two SVM buffer test.\n");

    void *good_buffer = clSVMAlloc(context, flags, buffer_size, 0);
    if (good_buffer == NULL)
    {
        fprintf(stderr, "clSVMAlloc near %s:%d failed.\n", __FILE__, __LINE__);
        exit(-1);
    }
    void *good_buffer_2 = clSVMAlloc(context, flags, buffer_size, 0);
    if (good_buffer_2 == NULL)
    {
        fprintf(stderr, "clSVMAlloc near %s:%d failed.\n", __FILE__, __LINE__);
        exit(-1);
    }

    cl_err = clSetKernelArgSVMPointer(test_kernel, 0, good_buffer);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clSetKernelArgSVMPointer(test_kernel, 1, good_buffer_2);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clSetKernelArg(test_kernel, 2, sizeof(cl_uint), &work_items_to_use);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clSetKernelArg(test_kernel, 3, sizeof(cl_uint), &stride);
    check_cl_error(__FILE__, __LINE__, cl_err);

    uint64_t bytes_written = (uint64_t)work_items_to_use * sizeof(cl_uint);
    printf("Launching %zu work items to write %llu entries.\n",
            work_items_to_use, (long long unsigned)num_entries_in_buf);
    printf("This will write %llu out of %llu bytes in the buffer.\n",
            (long long unsigned)bytes_written,
            (long long unsigned)buffer_size);
    cl_err = clEnqueueNDRangeKernel(cmd_queue, test_kernel, 1, NULL,
        &work_items_to_use, NULL, 0, NULL, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);
    clFinish(cmd_queue);
    printf("Done running two SVM buffer test.\n\n");

    // Clean up
    cl_err = clReleaseKernel(test_kernel);
    check_cl_error(__FILE__, __LINE__, cl_err);
    clSVMFree(context, good_buffer);
    clSVMFree(context, good_buffer_2);
}
#endif // CL_VERSION_2_0

int main(int argc, char** argv)
{
#ifdef CL_VERSION_2_0
    uint32_t platform_to_use = 0;
    uint32_t device_to_use = 0;
    cl_device_type dev_type = CL_DEVICE_TYPE_DEFAULT;
    uint64_t buffer_size = DEFAULT_BUFFER_SIZE;

    // Check input options.
    check_opts(argc, argv, "Complex SVM Without Overflow",
            &platform_to_use, &device_to_use, &dev_type);

    // Set up the OpenCL environment.
    cl_platform_id platform = setup_platform(platform_to_use);
    cl_device_id device = setup_device(device_to_use, platform_to_use,
            platform, dev_type);

    if (!device_supports_svm(device, 0) && !device_supports_svm(device, 1))
    {
        output_fake_errors(OUTPUT_FILE_NAME, EXPECTED_ERRORS);
        printf("Proper SVM not supported. Skipping Good Complex SVM Test.\n");
        return 0;
    }

    cl_context context = setup_context(platform, device);
    cl_command_queue cmd_queue = setup_cmd_queue(context, device);

    // Build the program and kernel
    cl_program program = setup_program(context, 1, &kernel_source, device);

    // Run the actual test.
    printf("\n\nRunning Good Complex SVM Test...\n");
    printf("    Using buffer size: %llu\n", (long long unsigned)buffer_size);

    cl_svm_mem_flags flags;
    flags = CL_MEM_READ_WRITE;

    // Run a test where we have an overflow in an SVM buffer but then run
    // a second kernel and it does not cause an overflow.
    run_svm_back_to_back(context, cmd_queue, program, flags, buffer_size);

    // Run a test that uses two SVM buffers passed to the kernel.
    run_two_svm_test(context, cmd_queue, program, flags, buffer_size, 1);

    // Run a test that uses two SVM buffers passed to the kernel but with a
    // different access stride. Still no overflow.
    run_two_svm_test(context, cmd_queue, program, flags, buffer_size, 4);

    cl_program dev_q_program = 0;
    if (device_supports_dev_queue(device))
    {
        // Run a test to see if a device-enqueued kernel that creates a buffer
        // overflow can be caught by the tool.
        dev_q_program = setup_program(context, 1, &dev_q_kernel_source,
                device);
        run_device_enqueue_test(device, context, cmd_queue, dev_q_program,
                flags, buffer_size);
    }
    else
    {
        // We don't support device queues, so run one of the tests again
        // so that the total number of errors is correct.
        run_two_svm_test(context, cmd_queue, program, flags, buffer_size, 1);
    }

    flags = CL_MEM_READ_WRITE | CL_MEM_SVM_FINE_GRAIN_BUFFER;
    run_svm_back_to_back(context, cmd_queue, program, flags, buffer_size);
    run_two_svm_test(context, cmd_queue, program, flags, buffer_size, 1);
    run_two_svm_test(context, cmd_queue, program, flags, buffer_size, 4);

    if (device_supports_dev_queue(device))
    {
        run_device_enqueue_test(device, context, cmd_queue, dev_q_program,
                flags, buffer_size);
    }
    else
        run_two_svm_test(context, cmd_queue, program, flags, buffer_size, 1);

    printf("Done Running Good Complex SVM Test.\n");
#else // CL_VERSION_2_0
    (void)argc;
    (void)argv;
    output_fake_errors(OUTPUT_FILE_NAME, EXPECTED_ERRORS);
    printf("OpenCL 2.0 not supported. Skipping Good Complex SVM Test.\n");
#endif // CL_VERSION_2_0
    return 0;
}
