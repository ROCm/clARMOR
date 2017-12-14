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

const char *kernel_source = "\n"\
"__kernel void test(__global uint *cl_mem_buffer, uint len) {\n"\
"    uint i = get_global_id(0);\n"\
"    if (i < len) {\n"\
"        cl_mem_buffer[i] = i;\n"\
"    }\n"\
"}\n";
const char *kernel_source1 = "\n"\
"__kernel void test(__write_only image2d_t image, uint width, uint height) {\n"\
"    uint i = get_global_id(0);\n"\
"    uint x = i % width;\n"\
"    uint y = i / width;\n"\
"    int2 loc = (int2)(x,y);\n"\
"    float4 pix = (float4)(i,i,i,i);\n"\
"    if (x < width && y < height) {\n"\
"        write_imagef(image, loc, pix);\n"\
"    }\n"\
"}\n";
const char *kernel_source2 = "\n"\
"__kernel void test(__write_only image2d_t image, uint width, uint height) {\n"\
"    uint i = get_global_id(0);\n"\
"    uint x = i % width;\n"\
"    uint y = i / width;\n"\
"    int2 loc = (int2)(x,y);\n"\
"    int4 pix = (int4)(i,i,i,i);\n"\
"    if (x < width && y < height) {\n"\
"        write_imagei(image, loc, pix);\n"\
"    }\n"\
"}\n";
const char *kernel_source3 = "\n"\
"__kernel void test(__write_only image2d_t image, uint width, uint height) {\n"\
"    uint i = get_global_id(0);\n"\
"    uint x = i % width;\n"\
"    uint y = i / width;\n"\
"    int2 loc = (int2)(x,y);\n"\
"    uint4 pix = (uint4)(i,i,i,i);\n"\
"    if (x < width && y < height) {\n"\
"        write_imageui(image, loc, pix);\n"\
"    }\n"\
"}\n";
const char *kernel_source4 = "\n"\
"__kernel void test(__write_only image3d_t image, uint width, uint height, uint depth) {\n"\
"    uint i = get_global_id(0);\n"\
"    uint x = i % width;\n"\
"    uint y = (i / width) % height;\n"\
"    uint z = i / (width * height);\n"\
"    int4 loc = (int4)(x,y,z,0);\n"\
"    float4 pix = (float4)(i,i,i,i);\n"\
"    if (x < width && y < height && z < depth) {\n"\
"        write_imagef(image, loc, pix);\n"\
"    }\n"\
"}\n";
const char *kernel_source5 = "\n"\
"__kernel void test(__write_only image3d_t image, uint width, uint height, uint depth) {\n"\
"    uint i = get_global_id(0);\n"\
"    uint x = i % width;\n"\
"    uint y = (i / width) % height;\n"\
"    uint z = i / (width * height);\n"\
"    int4 loc = (int4)(x,y,z,0);\n"\
"    int4 pix = (int4)(i,i,i,i);\n"\
"    if (x < width && y < height && z < depth) {\n"\
"        write_imagei(image, loc, pix);\n"\
"    }\n"\
"}\n";
const char *kernel_source6 = "\n"\
"__kernel void test(__write_only image3d_t image, uint width, uint height, uint depth) {\n"\
"    uint i = get_global_id(0);\n"\
"    uint x = i % width;\n"\
"    uint y = (i / width) % height;\n"\
"    uint z = i / (width * height);\n"\
"    int4 loc = (int4)(x,y,z,0);\n"\
"    uint4 pix = (uint4)(i,i,i,i);\n"\
"    if (x < width && y < height && z < depth) {\n"\
"        write_imageui(image, loc, pix);\n"\
"    }\n"\
"}\n";


const char *cu_source1 = "\n"\
"__kernel void test(__write_only image2d_t image, uint width, uint height) {\n"\
"    uint i = get_global_id(0);\n"\
"    uint x = i % width;\n"\
"    uint y = i / width;\n"\
"    int2 loc = (int2)(x,y);\n"\
"    float4 pix = (float4)(i,i,i,i);\n"\
"    if (x < width && y < height) {\n"\
"        write_imagef(image, loc, pix);\n"\
"    }\n"\
"}\n";
const char *cu_source2 = "\n"\
"__kernel void test(__write_only image2d_t image, uint width, uint height) {\n"\
"    uint i = get_global_id(0);\n"\
"    uint x = i % width;\n"\
"    uint y = i / width;\n"\
"    int2 loc = (int2)(x,y);\n"\
"    int4 pix = (int4)(i,i,i,i);\n"\
"    if (x < width && y < height) {\n"\
"        write_imagei(image, loc, pix);\n"\
"    }\n"\
"}\n";
const char *cu_source3 = "\n"\
"__kernel void test(__write_only image2d_t image, uint width, uint height) {\n"\
"    uint i = get_global_id(0);\n"\
"    uint x = i % width;\n"\
"    uint y = i / width;\n"\
"    int2 loc = (int2)(x,y);\n"\
"    uint4 pix = (uint4)(i,i,i,i);\n"\
"    if (x < width && y < height) {\n"\
"        write_imageui(image, loc, pix);\n"\
"    }\n"\
"}\n";
const char *cu_source4 = "\n"\
"__kernel void test(__write_only image3d_t image, uint width, uint height, uint depth) {\n"\
"    uint i = get_global_id(0);\n"\
"    uint x = i % width;\n"\
"    uint y = (i / width) % height;\n"\
"    uint z = i / (width * height);\n"\
"    int4 loc = (int4)(x,y,z,0);\n"\
"    float4 pix = (float4)(i,i,i,i);\n"\
"    if (x < width && y < height && z < depth) {\n"\
"        write_imagef(image, loc, pix);\n"\
"    }\n"\
"}\n";
const char *cu_source5 = "\n"\
"__kernel void test(__write_only image3d_t image, uint width, uint height, uint depth) {\n"\
"    uint i = get_global_id(0);\n"\
"    uint x = i % width;\n"\
"    uint y = (i / width) % height;\n"\
"    uint z = i / (width * height);\n"\
"    int4 loc = (int4)(x,y,z,0);\n"\
"    int4 pix = (int4)(i,i,i,i);\n"\
"    if (x < width && y < height && z < depth) {\n"\
"        write_imagei(image, loc, pix);\n"\
"    }\n"\
"}\n";
const char *cu_source6 = "\n"\
"__kernel void test(__write_only image3d_t image, uint width, uint height, uint depth) {\n"\
"    uint i = get_global_id(0);\n"\
"    uint x = i % width;\n"\
"    uint y = (i / width) % height;\n"\
"    uint z = i / (width * height);\n"\
"    int4 loc = (int4)(x,y,z,0);\n"\
"    uint4 pix = (uint4)(i,i,i,i);\n"\
"    if (x < width && y < height && z < depth) {\n"\
"        write_imageui(image, loc, pix);\n"\
"    }\n"\
"}\n";


uint64_t getNumWorkItems(uint64_t buffer_size)
{
    // Each work item will touch  sizeof(cl_uint) bytes.
    // This calculates how many work items we can have and still stay within
    // the buffer size.
    // If the maximum work items we can launch won't reach the end of the
    // buffer, that is OK. Then we definitely won't have an overflow.
    uint64_t num_entries_in_buf = buffer_size / sizeof(cl_uint);
    size_t work_items_to_use;
    if (num_entries_in_buf > SIZE_MAX)
        work_items_to_use = SIZE_MAX;
    else
        work_items_to_use = num_entries_in_buf;
    uint64_t bytes_written = (uint64_t)work_items_to_use * sizeof(cl_uint);

    printf("Launching %zu work items to write up to %llu entries.\n",
            work_items_to_use, (long long unsigned)num_entries_in_buf);
    printf("This will write %llu out of %llu bytes in the buffer.\n",
            (long long unsigned)bytes_written,
            (long long unsigned)buffer_size);

    return work_items_to_use;
}

uint32_t getKernNum(cl_channel_type type)
{
    uint32_t ret;
    switch(type)
    {
        case CL_SNORM_INT8:
        case CL_UNORM_INT8:
        case CL_SNORM_INT16:
        case CL_UNORM_INT16:
        case CL_HALF_FLOAT:
        case CL_FLOAT:
            ret = 1;
            break;
        case CL_SIGNED_INT8:
        case CL_SIGNED_INT16:
        case CL_SIGNED_INT32:
            ret = 2;
            break;
        case CL_UNSIGNED_INT8:
        case CL_UNSIGNED_INT16:
        case CL_UNSIGNED_INT32:
            ret = 3;
            break;
        default:
            ret = 0;
    }
    return ret;
}

cl_image_format *formats_2d, *formats_3d;

int main(int argc, char** argv)
{
    cl_int cl_err;
    uint32_t platform_to_use = 0;
    uint32_t device_to_use = 0;
    cl_device_type dev_type = CL_DEVICE_TYPE_DEFAULT;
    uint64_t buffer_size = DEFAULT_BUFFER_SIZE;
    size_t work_items_to_use;
    uint64_t width, height, depth;
    uint32_t reduce2d, reduce3dhw;
    //fraction of buffer size used for width for 2d
    reduce2d = 1024;
    //fraction of buffer size used for height and width for 3d
    reduce3dhw = reduce2d * 8;
    depth = 64;


    // Check input options.
    check_opts(argc, argv, "image_cl1_1 without Overflow",
            &platform_to_use, &device_to_use, &dev_type);

    // Set up the OpenCL environment.
    cl_platform_id platform = setup_platform(platform_to_use);
    cl_device_id device = setup_device(device_to_use, platform_to_use,
            platform, dev_type);
    cl_context context = setup_context(platform, device);
    cl_command_queue cmd_queue = setup_cmd_queue(context, device);

    // Build the program and kernel
    cl_program program;
    cl_kernel test_kernel;
    if(dev_type == CL_DEVICE_TYPE_CPU)
        program = setup_program(context, 1, &cu_source1, device);
    else
        program = setup_program(context, 1, &kernel_source1, device);
    cl_kernel test_kernel1 = setup_kernel(program, "test");

    if(dev_type == CL_DEVICE_TYPE_CPU)
        program = setup_program(context, 1, &cu_source2, device);
    else
        program = setup_program(context, 1, &kernel_source2, device);
    cl_kernel test_kernel2 = setup_kernel(program, "test");

    if(dev_type == CL_DEVICE_TYPE_CPU)
        program = setup_program(context, 1, &cu_source3, device);
    else
        program = setup_program(context, 1, &kernel_source3, device);
    cl_kernel test_kernel3 = setup_kernel(program, "test");

    if(dev_type == CL_DEVICE_TYPE_CPU)
        program = setup_program(context, 1, &cu_source4, device);
    else
        program = setup_program(context, 1, &kernel_source4, device);
    cl_kernel test_kernel4 = setup_kernel(program, "test");

    if(dev_type == CL_DEVICE_TYPE_CPU)
        program = setup_program(context, 1, &cu_source5, device);
    else
        program = setup_program(context, 1, &kernel_source5, device);
    cl_kernel test_kernel5 = setup_kernel(program, "test");

    if(dev_type == CL_DEVICE_TYPE_CPU)
        program = setup_program(context, 1, &cu_source6, device);
    else
        program = setup_program(context, 1, &kernel_source6, device);
    cl_kernel test_kernel6 = setup_kernel(program, "test");

    // In this case, we are going to create a cl_mem buffer of the appropriate
    // size. The kernel will then correctly copy the right amount of data
    // into that buffer, and then the program will exit.
    // This will not create a buffer overflow.
    // Run the actual test.

    printf("\n\nRunning Good complex_image_cl1_1 Test...\n");
    printf("    Using buffer size: %llu\n", (long long unsigned)buffer_size);

    //cl_err = clSetKernelArg(test_kernel, 1, sizeof(cl_uint), &buffer_size);
    //check_cl_error(__FILE__, __LINE__, cl_err);

    cl_mem good_buffer;
    cl_mem_flags flags;

    cl_image_format format;
    uint32_t i = 0;
    unsigned dataSize = 0;

    printf("\n\nHost Pointer Image2D Test...\n");

    cl_uint num_entries = 0;
    // BUGFIX: The Nvidia OpenCL implementation, clGetSupportedImageFormats
    // does not support using CL_MEM_USE_HOST_PTR as a flag. However, the
    // image creation API does support this (???). As such, we don't actually
    // query against the host pointer flag..
    flags = CL_MEM_WRITE_ONLY;

    cl_err = clGetSupportedImageFormats(context, flags, CL_MEM_OBJECT_IMAGE2D, 0, NULL, &num_entries);
    check_cl_error(__FILE__, __LINE__, cl_err);
    formats_2d = calloc(num_entries, sizeof(cl_image_format));
    cl_err = clGetSupportedImageFormats(context, flags, CL_MEM_OBJECT_IMAGE2D, num_entries, formats_2d, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);

    flags = CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR;

    format.image_channel_order =  formats_2d[0].image_channel_order;
    format.image_channel_data_type = formats_2d[0].image_channel_data_type;

    dataSize = get_image_data_size(&format);
    if (dataSize != 0)
    {
        width = (buffer_size / dataSize) / reduce2d;
        height = reduce2d;

        void* buffer = calloc(dataSize, width * height);

        good_buffer = clCreateImage2D(context, flags, &format, width, height, 0, buffer, &cl_err);
        check_cl_error(__FILE__, __LINE__, cl_err);

        switch(getKernNum(format.image_channel_data_type))
        {
            case 1:
                test_kernel = test_kernel1;
                break;
            case 2:
                test_kernel = test_kernel2;
                break;
            case 3:
                test_kernel = test_kernel3;
                break;
            default:
                printf("Error: No Kernel..skipping\n");
                exit(-1);
        }

        printf("dataSize %u, %u - %u\n", dataSize,
                format.image_channel_order, format.image_channel_data_type);

        cl_err = clSetKernelArg(test_kernel, 0, sizeof(cl_mem), &good_buffer);
        check_cl_error(__FILE__, __LINE__, cl_err);
        cl_err = clSetKernelArg(test_kernel, 1, sizeof(cl_uint), &width);
        check_cl_error(__FILE__, __LINE__, cl_err);
        cl_err = clSetKernelArg(test_kernel, 2, sizeof(cl_uint), &height);
        check_cl_error(__FILE__, __LINE__, cl_err);


        work_items_to_use = width * height;

        cl_err = clEnqueueNDRangeKernel(cmd_queue, test_kernel, 1, NULL,
            &work_items_to_use, NULL, 0, NULL, NULL);
        check_cl_error(__FILE__, __LINE__, cl_err);

        clFinish(cmd_queue);
        clReleaseMemObject(good_buffer);
        free(buffer);
    }


    printf("\nImage2D Test...\n");
    flags = CL_MEM_WRITE_ONLY;
    for(i = 0; i < num_entries; i++)
    {
        format.image_channel_order = formats_2d[i].image_channel_order;
        format.image_channel_data_type = formats_2d[i].image_channel_data_type;
        dataSize = get_image_data_size(&format);
        if (dataSize == 0)
            continue;

        width = (buffer_size / dataSize) / reduce2d;
        height = reduce2d;

        good_buffer = clCreateImage2D(context, flags, &format, width, height, 0, NULL, &cl_err);
        if(cl_err == CL_IMAGE_FORMAT_NOT_SUPPORTED || cl_err == CL_INVALID_IMAGE_FORMAT_DESCRIPTOR)
        {
            //printf("doesn't support format %u", i);
            continue;
        }
        check_cl_error(__FILE__, __LINE__, cl_err);

        switch(getKernNum(format.image_channel_data_type))
        {
            case 1:
                test_kernel = test_kernel1;
                break;
            case 2:
                test_kernel = test_kernel2;
                break;
            case 3:
                test_kernel = test_kernel3;
                break;
            default:
                printf("Error: No Kernel..skipping\n");
                continue;
        }

        cl_err = clSetKernelArg(test_kernel, 0, sizeof(cl_mem), &good_buffer);
        if(cl_err != CL_SUCCESS)
        {
            //likely image format error, skip
            continue;
        }
        check_cl_error(__FILE__, __LINE__, cl_err);
        cl_err = clSetKernelArg(test_kernel, 1, sizeof(cl_uint), &width);
        check_cl_error(__FILE__, __LINE__, cl_err);
        cl_err = clSetKernelArg(test_kernel, 2, sizeof(cl_uint), &height);
        check_cl_error(__FILE__, __LINE__, cl_err);


        work_items_to_use = buffer_size / dataSize;//getNumWorkItems(buffer_size);

        printf("%u dataSize %u, bufferSize %lu, image_channel_order:%u, image_channel_data_type:%u\n",
            i, dataSize, width*height,
            format.image_channel_order, format.image_channel_data_type);

        cl_err = clEnqueueNDRangeKernel(cmd_queue, test_kernel, 1, NULL,
                &work_items_to_use, NULL, 0, NULL, NULL);
        check_cl_error(__FILE__, __LINE__, cl_err);

        clFinish(cmd_queue);

        // Run this a second time, because we've seen regressions where the
        // action of "refresh the canary region after running the check"
        // will actually corrupt certain pixel types. Running a second test
        // on the same buffer will then find those corrupted canaries
        cl_err = clEnqueueNDRangeKernel(cmd_queue, test_kernel, 1, NULL,
                &work_items_to_use, NULL, 0, NULL, NULL);
        check_cl_error(__FILE__, __LINE__, cl_err);

        clFinish(cmd_queue);
        clReleaseMemObject(good_buffer);
    }


    printf("\nImage3D Test...\n");
    cl_err = clGetSupportedImageFormats(context, flags, CL_MEM_OBJECT_IMAGE3D, 0, NULL, &num_entries);
    check_cl_error(__FILE__, __LINE__, cl_err);
    formats_3d = calloc(num_entries, sizeof(cl_image_format));
    cl_err = clGetSupportedImageFormats(context, flags, CL_MEM_OBJECT_IMAGE3D, num_entries, formats_3d, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);

    for(i = 0; i < num_entries; i++)
    {
        format.image_channel_order = formats_3d[i].image_channel_order;
        format.image_channel_data_type = formats_3d[i].image_channel_data_type;
        dataSize = get_image_data_size(&format);
        if (dataSize == 0)
            continue;

        width = (buffer_size / dataSize) / reduce3dhw;
        height = reduce3dhw / 64;
        //depth = 64;

        good_buffer = clCreateImage3D(context, flags, &format, width, height, depth, 0, 0, NULL, &cl_err);
        if(cl_err == CL_IMAGE_FORMAT_NOT_SUPPORTED || cl_err == CL_INVALID_IMAGE_FORMAT_DESCRIPTOR)
        {
            //printf("doesn't support format %u", i);
            continue;
        }
        check_cl_error(__FILE__, __LINE__, cl_err);

        switch(getKernNum(format.image_channel_data_type))
        {
            case 1:
                test_kernel = test_kernel4;
                break;
            case 2:
                test_kernel = test_kernel5;
                break;
            case 3:
                test_kernel = test_kernel6;
                break;
            default:
                printf("Error: No Kernel..skipping\n");
                continue;
        }

        cl_err = clSetKernelArg(test_kernel, 0, sizeof(cl_mem), &good_buffer);
        if(cl_err != CL_SUCCESS)
        {
            //likely image format error, skip
            continue;
        }
        check_cl_error(__FILE__, __LINE__, cl_err);
        cl_err = clSetKernelArg(test_kernel, 1, sizeof(cl_uint), &width);
        check_cl_error(__FILE__, __LINE__, cl_err);
        cl_err = clSetKernelArg(test_kernel, 2, sizeof(cl_uint), &height);
        check_cl_error(__FILE__, __LINE__, cl_err);
        cl_err = clSetKernelArg(test_kernel, 3, sizeof(cl_uint), &depth);
        check_cl_error(__FILE__, __LINE__, cl_err);

        work_items_to_use = buffer_size / dataSize;//getNumWorkItems(buffer_size);

        printf("%u dataSize %u, bufferSize %lu, image_channel_order:%u, image_channel_data_type:%u\n",
            i, dataSize, width*height*depth,
            format.image_channel_order, format.image_channel_data_type);

        cl_err = clEnqueueNDRangeKernel(cmd_queue, test_kernel, 1, NULL,
                &work_items_to_use, NULL, 0, NULL, NULL);
        check_cl_error(__FILE__, __LINE__, cl_err);
        clFinish(cmd_queue);

        // See above for why we're running this a second time.
        cl_err = clEnqueueNDRangeKernel(cmd_queue, test_kernel, 1, NULL,
                &work_items_to_use, NULL, 0, NULL, NULL);
        check_cl_error(__FILE__, __LINE__, cl_err);

        clFinish(cmd_queue);
        clReleaseMemObject(good_buffer);
    }


    printf("Done Running Good complex_image_cl1_1 Test.\n");
    return 0;
}
