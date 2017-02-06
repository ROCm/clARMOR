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


// A test to make sure that using indirect buffers does not cause us to
// detect an overflow by accident. In other words, if we do not overflow any of
// the SVMs passed in as kernel arguments, but one of them points to a series of
// other SVM regions. If we don't write past the end of those regions,
// our tool should not find a problem.
#include "common_test_functions.h"

const char *kernel_source = "\n"\
"__kernel void test(__global void *base_buf_in, uint base_len,\n"\
"                   __global uint *lengths) {\n"\
"    __global uint** base_buf = (__global uint**)base_buf_in;\n"\
"    uint i = get_global_id(0);\n"\
"    if (i < base_len) {\n"\
"        __global uint* base = base_buf[i];\n"\
"        for (int j = 0; j < lengths[i]; j++) {\n"\
"            base[j] = i;\n"\
"        }\n"\
"    }\n"\
"}\n";

int main(int argc, char** argv)
{
#ifdef CL_VERSION_2_0
    cl_int cl_err;
    uint32_t platform_to_use = 0;
    uint32_t device_to_use = 0;
    cl_device_type dev_type = CL_DEVICE_TYPE_DEFAULT;
    uint64_t buffer_size = DEFAULT_BUFFER_SIZE;
    uint32_t num_sets = 64;

    // Check input options.
    check_opts(argc, argv, "SVM without Indirect Overflow",
            &platform_to_use, &device_to_use, &dev_type);

    // Set up the OpenCL environment.
    cl_platform_id platform = setup_platform(platform_to_use);
    cl_device_id device = setup_device(device_to_use, platform_to_use,
            platform, dev_type);
    cl_context context = setup_context(platform, device);
    cl_command_queue cmd_queue = setup_cmd_queue(context, device);

    // Build the program and kernel
    cl_program program = setup_program(context, 1, &kernel_source, device);
    cl_kernel test_kernel = setup_kernel(program, "test");

    // Run the actual test.
    printf("\n\nRunning Good Indirect SVM Test...\n");
    printf("    Using %u buffers of size: %llu\n", (unsigned int)num_sets,
            (long long unsigned)buffer_size);

    // Allocate upper-level buffers.
    // The base_buffer will hold a collection of pointers to other buffers.
    void *base_buffer = clSVMAlloc(context, CL_MEM_READ_WRITE,
        sizeof(void*) * num_sets, 0);
    if (base_buffer == NULL)
    {
        fprintf(stderr, "clSVMAlloc near %s:%d failed.\n", __FILE__, __LINE__);
        exit(-1);
    }
    // The base_lengths buffer will tell us how long each of those other
    // buffers is.
    void *base_lengths = clSVMAlloc(context, CL_MEM_READ_WRITE,
        sizeof(cl_uint) * num_sets, 0);
    if (base_lengths == NULL)
    {
        fprintf(stderr, "clSVMAlloc near %s:%d failed.\n", __FILE__, __LINE__);
        exit(-1);
    }

    // Map the upper-level buffers so we can fill them in.
    cl_err = clEnqueueSVMMap(cmd_queue, CL_TRUE, CL_MAP_WRITE, base_buffer,
        sizeof(void*)*num_sets, 0, NULL, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clEnqueueSVMMap(cmd_queue, CL_TRUE, CL_MAP_WRITE, base_lengths,
            sizeof(cl_uint)*num_sets, 0, NULL, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);

    // Allocate a sub-buffer into each of these base buffer entries.
    unsigned int i;
    void **temp_buf = (void**)base_buffer;
    cl_uint *temp_len = (cl_uint *)base_lengths;
    for (i = 0; i < num_sets; i++)
    {
        temp_buf[i] = clSVMAlloc(context, CL_MEM_READ_WRITE,
                sizeof(cl_uint) * buffer_size, 0);
        if (temp_buf[i] == NULL)
        {
            fprintf(stderr, "clSVMAlloc %u near %s:%d failed.\n", i, __FILE__,
                    __LINE__);
            exit(-1);
        }
        temp_len[i] = buffer_size;
    }

    // Unmap the buffers
    cl_err = clEnqueueSVMUnmap(cmd_queue, base_buffer, 0, NULL, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clEnqueueSVMUnmap(cmd_queue, base_lengths, 0, NULL, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);
    clFinish(cmd_queue);

    // Set the arguments
    cl_err = clSetKernelArgSVMPointer(test_kernel, 0, base_buffer);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clSetKernelArg(test_kernel, 1, sizeof(cl_uint), &num_sets);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clSetKernelArgSVMPointer(test_kernel, 2, base_lengths);
    check_cl_error(__FILE__, __LINE__, cl_err);

    size_t work_items_to_use = num_sets;

    printf("Launching %lu work items.\n", work_items_to_use);
    cl_err = clEnqueueNDRangeKernel(cmd_queue, test_kernel, 1, NULL,
        &work_items_to_use, NULL, 0, NULL, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);

    clFinish(cmd_queue);
    printf("Done Running Good Indirect SVM Test.\n");
#else // CL_VERSION_2_0
    (void)argc;
    (void)argv;
    output_fake_errors(OUTPUT_FILE_NAME, EXPECTED_ERRORS);
    printf("OpenCL 2.0 not supported. Skipping Bad SVM Test.\n");
#endif // CL_VERSION_2_0
    return 0;
}
