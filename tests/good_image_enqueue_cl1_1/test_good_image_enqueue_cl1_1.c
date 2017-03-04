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


// A test to make sure that programs without a buffer overflow complete without
// causing the buffer overflow detector to find a false overflow.
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS

#include "common_test_functions.h"
#include <math.h>


static void run_2d_test(const cl_device_id device, const cl_context context,
    const cl_command_queue cmd_queue,
    uint64_t width, uint64_t height)
{
    cl_int cl_err;
    size_t num_work_items[2];

    printf("\nRunning 2D Image Test...\n");
    // In this case, we are going to create an image of the desired buffer
    // size. If the maximum image dimensions won't allow it to fit, then we
    // reduce our dimensions so that it does.
    size_t max_width = get_image_width(device, 2);
    size_t max_height = get_image_height(device, 2);

    // Checking against "* 2" because we want to leave room for the canary
    // values in the buffer overflow detector.
    if (max_width < (width * 2))
    {
        width = max_width/2;
        printf("    Requested image width is too large. ");
        printf("Reducing width to: %llu\n",
                (long long unsigned)width);
    }
    if (max_height < (height * 2))
    {
        height = max_height/2;
        printf("    Requested image height is too large. ");
        printf("Reducing height to: %llu\n",
                (long long unsigned)height);
    }
    uint64_t buffer_size = (uint64_t)height * width;
    printf("Using an image of size (H x W = size): %llu x %llu = %llu\n",
        (long long unsigned)height, (long long unsigned)width,
        (long long unsigned)buffer_size);

    // In this case, we are going to create a cl_memmimage  buffer of the right
    // size. The kernel will then correctly copy the right amount of data
    // into that buffer, and then the program will exit.
    // This will not create a buffer overflow.
    cl_image_format format;
    format.image_channel_order = CL_R;
    format.image_channel_data_type = CL_FLOAT;

    cl_mem_flags flags;
    flags = CL_MEM_WRITE_ONLY;

    cl_mem good_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE,
        4*buffer_size,  NULL, &cl_err);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_mem good_image = clCreateImage2D(context, flags, &format, width, height,
            0, NULL, &cl_err);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_mem good_image2 = clCreateImage2D(context, flags, &format, width, height,
            0, NULL, &cl_err);
    check_cl_error(__FILE__, __LINE__, cl_err);


    // Each work item will touch one pixel of the image, so we want there to
    // be the same number of work items as pixels.
    // If the maximum work items we can launch won't reach the end of the
    // buffer, that is OK. Then we definitely won't have an overflow.
    num_work_items[0] = width;
    num_work_items[1] = height;

    printf("Launching %lu x %lu work items to write up to %llu pixels.\n",
            num_work_items[0], num_work_items[1],
            (long long unsigned)buffer_size);
    printf("\nImage2D Test...\n");


    float *host_ptr;
    host_ptr = calloc(sizeof(float), buffer_size);
    size_t origin[3] = {0, 0, 0};
    size_t region[3] = {width, height, 1};

    cl_err = clEnqueueReadImage(cmd_queue, good_image, CL_TRUE, origin, region, 0, 0, host_ptr, 0, NULL, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clEnqueueWriteImage(cmd_queue, good_image, CL_TRUE, origin, region, 0, 0, host_ptr, 0, NULL, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);
#ifdef CL_VERSION_1_2
    float fill = 29;
    cl_err = clEnqueueFillImage(cmd_queue, good_image, &fill, origin, region, 0, NULL, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);
#endif
    cl_err = clEnqueueCopyImage(cmd_queue, good_image, good_image2, origin, origin, region, 0, NULL, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clEnqueueCopyImageToBuffer(cmd_queue, good_image, good_buffer, origin, region, 0, 0, NULL, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clEnqueueCopyBufferToImage(cmd_queue, good_buffer, good_image, 0, origin, region, 0, NULL, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);


    clFinish(cmd_queue);
    free(host_ptr);
    printf("Done.\n");
    clReleaseMemObject(good_buffer);
    clReleaseMemObject(good_image);
    clReleaseMemObject(good_image2);
}

/*
static void run_3d_test(const cl_device_id device, const cl_context context,
    const cl_command_queue cmd_queue, const cl_program program,
    uint64_t width, uint64_t height, uint64_t depth)
{
    cl_int cl_err;
    uint64_t buffer_size = width * height * depth;
    size_t num_work_items[3];
    cl_kernel test_kernel = setup_kernel(program, "test_3d");

    printf("\nRunning 3D Image Test...\n");
    // In this case, we are going to create an image of the desired buffer
    // size. If the maximum image dimensions won't allow it to fit, then we
    // reduce our dimensions so that it does.
    size_t max_width = get_image_width(device, 3);
    size_t max_height = get_image_height(device, 3);
    size_t max_depth = get_image_depth(device);

    // Checking against "* 2" because we want to leave room for the canary
    // values in the buffer overflow detector.
    if (max_width < (width * 2))
    {
        width = max_width/2;
        printf("    Requested image width is too large. ");
        printf("Reducing width to: %llu\n",
                (long long unsigned)width);
    }
    if (max_height < (height * 2))
    {
        height = max_height/2;
        printf("    Requested image height is too large. ");
        printf("Reducing height to: %llu\n",
                (long long unsigned)height);
    }
    if (max_depth < (depth * 2))
    {
        height = max_depth/2;
        printf("    Requested image depth is too large. ");
        printf("Reducing depth to: %llu\n",
                (long long unsigned)depth);
    }
    buffer_size = height * width * depth;
    printf("Using an image of size (H x W x D = size): ");
    printf("%llu x %llu x %llu = %llu\n",
        (long long unsigned)height, (long long unsigned)width,
        (long long unsigned)depth, (long long unsigned)buffer_size);

    // In this case, we are going to create a cl_mem buffer of the appropriate
    // size. The kernel will then correctly copy the right amount of data
    // into that buffer, and then the program will exit.
    // This will not create a buffer overflow.
    cl_image_format format;
    format.image_channel_order = CL_R;
    format.image_channel_data_type = CL_FLOAT;

    cl_mem_flags flags;
    flags = CL_MEM_WRITE_ONLY;

    cl_mem good_buffer = clCreateImage3D(context, flags, &format, width,
            height, depth, 0, 0, NULL, &cl_err);
    check_cl_error(__FILE__, __LINE__, cl_err);

    cl_err = clSetKernelArg(test_kernel, 0, sizeof(cl_mem), &good_buffer);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clSetKernelArg(test_kernel, 1, sizeof(cl_uint), &width);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clSetKernelArg(test_kernel, 2, sizeof(cl_uint), &height);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clSetKernelArg(test_kernel, 3, sizeof(cl_uint), &depth);
    check_cl_error(__FILE__, __LINE__, cl_err);

    // Each work item will touch one pixel of the image, so we want there to
    // be the same number of work items as pixels.
    // If the maximum work items we can launch won't reach the end of the
    // buffer, that is OK. Then we definitely won't have an overflow.
    num_work_items[0] = width;
    num_work_items[1] = height;
    num_work_items[2] = depth;

    printf("Launching %lu x %lu x %lu work items to write to %llu pixels.\n",
            num_work_items[0], num_work_items[1], num_work_items[2],
            (long long unsigned)buffer_size);
    printf("\nImage3D Test...\n");
    cl_err = clEnqueueNDRangeKernel(cmd_queue, test_kernel, 3, NULL,
        num_work_items, NULL, 0, NULL, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);
    clFinish(cmd_queue);
    printf("Done.\n");

    clReleaseMemObject(good_buffer);
}
*/

int main(int argc, char** argv)
{
    // We don't need to check for OpenCL 1.1 compatibility here, because the
    // OpenCL 1.1 image functions also existed in OpenCL 1.0
    uint32_t platform_to_use = 0;
    uint32_t device_to_use = 0;
    cl_device_type dev_type = CL_DEVICE_TYPE_DEFAULT;
    uint64_t buffer_size = DEFAULT_BUFFER_SIZE;
    uint64_t width, height;//, depth;

    // Check input options.
    check_opts(argc, argv, "image_cl1_1 API Without Overflow",
            &platform_to_use, &device_to_use, &dev_type);

    // Set up the OpenCL environment.
    cl_platform_id platform = setup_platform(platform_to_use);
    cl_device_id device = setup_device(device_to_use, platform_to_use,
            platform, dev_type);
    cl_context context = setup_context(platform, device);
    cl_command_queue cmd_queue = setup_cmd_queue(context, device);

    // Build the program and kernel

    // Run the actual test.
    printf("\n\nRunning Good image_cl1_1 API Test...\n");
    printf("    Using buffer size: %llu\n", (long long unsigned)buffer_size);

    // Make the buffer a square for the 2D test, if possible.
    width = sqrt(buffer_size);
    height = width;
    run_2d_test(device, context, cmd_queue, width, height);

    /*
    // Make buffer a cube for the 3D test, if possible.
    width = pow(buffer_size, 1.0 / 3.0);
    height = width;
    depth = width;
    run_3d_test(device, context, cmd_queue, program, width, height, depth);
    */

    printf("Done Running Good image_cl1_1 API Test.\n");
    return 0;
}
