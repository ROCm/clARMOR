/********************************************************************************
 * Copyright (c) 2016-2019 Advanced Micro Devices, Inc. All rights reserved.
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


// A test to see if we can find errors in image API functions.
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS

#include "common_test_functions.h"
#include <math.h>


static void run_2d_test(const cl_device_id device, const cl_context context,
    const cl_command_queue cmd_queue,
    uint64_t width, uint64_t height)
{
    cl_int cl_err;
    size_t num_work_items[2];

    if (images_are_broken(device))
    {
        output_fake_errors(OUTPUT_FILE_NAME, EXPECTED_ERRORS);
        printf("This device does not properly support an implementation of ");
        printf("OpenCL images. As such, we cannot test them.\n");
        printf("Skipping Bad image_enqueue_cl1_1 Test.\n");
		return;
    }

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
    cl_mem bad_image = clCreateImage2D(context, flags, &format, width, height-10,
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

    printf("Launching %zu x %zu work items to write up to %llu pixels.\n",
            num_work_items[0], num_work_items[1],
            (long long unsigned)buffer_size);
    printf("\nImage2D Test...\n");


    float *host_ptr;
    host_ptr = calloc(sizeof(float), buffer_size);
    size_t origin[3] = {0, 0, 0};
    size_t region[3] = {width, height, 1};

    cl_err = clEnqueueReadImage(cmd_queue, bad_image, CL_TRUE, origin, region, 0, 0, host_ptr, 0, NULL, NULL);
    // The runtime really should return CL_INVALID_VALUE here because of the
    // buffer overflow error. If so, we skip killing the program.
    if (cl_err != CL_INVALID_VALUE)
        check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clEnqueueWriteImage(cmd_queue, bad_image, CL_TRUE, origin, region, 0, 0, host_ptr, 0, NULL, NULL);
    if (cl_err != CL_INVALID_VALUE)
        check_cl_error(__FILE__, __LINE__, cl_err);
#ifdef CL_VERSION_1_2
    float fill = 29;
    cl_err = clEnqueueFillImage(cmd_queue, bad_image, &fill, origin, region, 0, NULL, NULL);
    if (cl_err != CL_INVALID_VALUE)
        check_cl_error(__FILE__, __LINE__, cl_err);
#else
    // Redo one of the previous tests so we get the correct number of errors.
    cl_err = clEnqueueWriteImage(cmd_queue, bad_image, CL_TRUE, origin, region, 0, 0, host_ptr, 0, NULL, NULL);
    if (cl_err != CL_INVALID_VALUE)
        check_cl_error(__FILE__, __LINE__, cl_err);
#endif
    cl_err = clEnqueueCopyImage(cmd_queue, bad_image, good_image2, origin, origin, region, 0, NULL, NULL);
    if (cl_err != CL_INVALID_VALUE)
        check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clEnqueueCopyImageToBuffer(cmd_queue, bad_image, good_buffer, origin, region, 0, 0, NULL, NULL);
    if (cl_err != CL_INVALID_VALUE)
        check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clEnqueueCopyBufferToImage(cmd_queue, good_buffer, bad_image, 0, origin, region, 0, NULL, NULL);
    if (cl_err != CL_INVALID_VALUE)
        check_cl_error(__FILE__, __LINE__, cl_err);

    clFinish(cmd_queue);
    free(host_ptr);
    printf("Done.\n");
    clReleaseMemObject(good_buffer);
    clReleaseMemObject(bad_image);
    clReleaseMemObject(good_image2);
}

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
    printf("\n\nRunning Bad image_cl1_1 API Test...\n");
    printf("    Using buffer size: %llu\n", (long long unsigned)buffer_size);

    // Make the buffer a square for the 2D test, if possible.
    width = sqrt(buffer_size);
    height = width;
    run_2d_test(device, context, cmd_queue, width, height);

    printf("Done Running Bad image_cl1_1 API Test.\n");
    return 0;
}
