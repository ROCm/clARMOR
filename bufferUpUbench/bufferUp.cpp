
//#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <cmath>
//#include <time.h>
#include <sys/time.h>

#include <CL/cl.h>
//#include <CL/cl.hpp>

#include "common_test_functions.h"

const char *cluErrorString(cl_int err)
{
    switch(err)
    {
        case CL_SUCCESS:
            return "CL_SUCCESS";
        case CL_DEVICE_NOT_FOUND:
            return "CL_DEVICE_NOT_FOUND";
        case CL_DEVICE_NOT_AVAILABLE:
            return "CL_DEVICE_NOT_AVAILABLE";
        case CL_COMPILER_NOT_AVAILABLE:
            return
                "CL_COMPILER_NOT_AVAILABLE";
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            return
                "CL_MEM_OBJECT_ALLOCATION_FAILURE";
        case CL_OUT_OF_RESOURCES:
            return "CL_OUT_OF_RESOURCES";
        case CL_OUT_OF_HOST_MEMORY:
            return "CL_OUT_OF_HOST_MEMORY";
        case CL_PROFILING_INFO_NOT_AVAILABLE:
            return
                "CL_PROFILING_INFO_NOT_AVAILABLE";
        case CL_MEM_COPY_OVERLAP:
            return "CL_MEM_COPY_OVERLAP";
        case CL_IMAGE_FORMAT_MISMATCH:
            return "CL_IMAGE_FORMAT_MISMATCH";
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:
            return
                "CL_IMAGE_FORMAT_NOT_SUPPORTED";
        case CL_BUILD_PROGRAM_FAILURE:
            return "CL_BUILD_PROGRAM_FAILURE";
        case CL_MAP_FAILURE:
            return "CL_MAP_FAILURE";
        case CL_INVALID_VALUE:
            return "CL_INVALID_VALUE";
        case CL_INVALID_DEVICE_TYPE:
            return "CL_INVALID_DEVICE_TYPE";
        case CL_INVALID_PLATFORM:
            return "CL_INVALID_PLATFORM";
        case CL_INVALID_DEVICE:
            return "CL_INVALID_DEVICE";
        case CL_INVALID_CONTEXT:
            return "CL_INVALID_CONTEXT";
        case CL_INVALID_QUEUE_PROPERTIES:
            return "CL_INVALID_QUEUE_PROPERTIES";
        case CL_INVALID_COMMAND_QUEUE:
            return "CL_INVALID_COMMAND_QUEUE";
        case CL_INVALID_HOST_PTR:
            return "CL_INVALID_HOST_PTR";
        case CL_INVALID_MEM_OBJECT:
            return "CL_INVALID_MEM_OBJECT";
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
            return
                "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
        case CL_INVALID_IMAGE_SIZE:
            return "CL_INVALID_IMAGE_SIZE";
        case CL_INVALID_SAMPLER:
            return "CL_INVALID_SAMPLER";
        case CL_INVALID_BINARY:
            return "CL_INVALID_BINARY";
        case CL_INVALID_BUILD_OPTIONS:
            return "CL_INVALID_BUILD_OPTIONS";
        case CL_INVALID_PROGRAM:
            return "CL_INVALID_PROGRAM";
        case CL_INVALID_PROGRAM_EXECUTABLE:
            return
                "CL_INVALID_PROGRAM_EXECUTABLE";
        case CL_INVALID_KERNEL_NAME:
            return "CL_INVALID_KERNEL_NAME";
        case CL_INVALID_KERNEL_DEFINITION:
            return "CL_INVALID_KERNEL_DEFINITION";
        case CL_INVALID_KERNEL:
            return "CL_INVALID_KERNEL";
        case CL_INVALID_ARG_INDEX:
            return "CL_INVALID_ARG_INDEX";
        case CL_INVALID_ARG_VALUE:
            return "CL_INVALID_ARG_VALUE";
        case CL_INVALID_ARG_SIZE:
            return "CL_INVALID_ARG_SIZE";
        case CL_INVALID_KERNEL_ARGS:
            return "CL_INVALID_KERNEL_ARGS";
        case CL_INVALID_WORK_DIMENSION:
            return "CL_INVALID_WORK_DIMENSION";
        case CL_INVALID_WORK_GROUP_SIZE:
            return "CL_INVALID_WORK_GROUP_SIZE";
        case CL_INVALID_WORK_ITEM_SIZE:
            return "CL_INVALID_WORK_ITEM_SIZE";
        case CL_INVALID_GLOBAL_OFFSET:
            return "CL_INVALID_GLOBAL_OFFSET";
        case CL_INVALID_EVENT_WAIT_LIST:
            return "CL_INVALID_EVENT_WAIT_LIST";
        case CL_INVALID_EVENT:
            return "CL_INVALID_EVENT";
        case CL_INVALID_OPERATION:
            return "CL_INVALID_OPERATION";
        case CL_INVALID_GL_OBJECT:
            return "CL_INVALID_GL_OBJECT";
        case CL_INVALID_BUFFER_SIZE:
            return "CL_INVALID_BUFFER_SIZE";
        case CL_INVALID_MIP_LEVEL:
            return "CL_INVALID_MIP_LEVEL";
        case CL_INVALID_GLOBAL_WORK_SIZE:
            return "CL_INVALID_GLOBAL_WORK_SIZE";
        default:
            return "UNKNOWN CL ERROR CODE";
    }
}

void populateKernelTimes( cl_event *event, cl_ulong *queuedTime, cl_ulong *submitTime, cl_ulong *startTime, cl_ulong *endTime )
{
    cl_int retval = 0;
    retval = clGetEventProfilingInfo( *event, CL_PROFILING_COMMAND_QUEUED, sizeof(*queuedTime), queuedTime, NULL );
    if (retval != CL_SUCCESS)
    {
        fprintf(stderr, "Error attempting to get profiling info for kernel queued time: %s\n", cluErrorString(retval));
        exit(-1);
    }
    retval = clGetEventProfilingInfo( *event, CL_PROFILING_COMMAND_SUBMIT, sizeof(*submitTime), submitTime, NULL );
    if (retval != CL_SUCCESS)
    {
        fprintf(stderr, "Error attempting to get profiling info for kernel submit time: %s\n", cluErrorString(retval));
        exit(-1);
    }
    retval = clGetEventProfilingInfo( *event, CL_PROFILING_COMMAND_START, sizeof(*startTime), startTime, NULL );
    if (retval != CL_SUCCESS)
    {
        fprintf(stderr, "Error attempting to get profiling info for kernel start time: %s\n", cluErrorString(retval));
        exit(-1);
    }
    retval = clGetEventProfilingInfo( *event, CL_PROFILING_COMMAND_END, sizeof(*endTime), endTime, NULL );
    if (retval != CL_SUCCESS)
    {
        fprintf(stderr, "Error attempting to get profiling info for kernel end time: %s\n", cluErrorString(retval));
        exit(-1);
    }
}

void check_cl_error( const char *file_name, int line_num, cl_int cl_err)
{
    if (cl_err != CL_SUCCESS)
    {
        fprintf( stderr, "%s:%d: error: %s\n", file_name, line_num, cluErrorString( (cl_err) ));
        exit(-1);
    }
    return;
}

void runClMem(cl_command_queue this_queue, cl_context ctx, cl_program prog, unsigned numBuff, unsigned numIter)
{
    cl_int cl_err;

    char kernName[32];
    sprintf(kernName, "sumArrQ%u", numBuff);
    //sprintf(kernName, "sumArrMidQ%u", numBuff);
    //sprintf(kernName, "sumArrHvyQ%u", numBuff);

    cl_kernel buffUp = clCreateKernel(prog, kernName, &cl_err);
    check_cl_error(__FILE__, __LINE__, cl_err);
    clReleaseProgram(prog);

    void** buffers;
    buffers = (void**)malloc(sizeof(cl_mem)*numBuff);
    buffers[numBuff-1] = clCreateBuffer(ctx, CL_MEM_READ_WRITE, sizeof(unsigned), NULL, NULL);

    std::stringstream out_cs;

    unsigned buffLen = 4096;
    cl_event evts[2];
    for(unsigned i = 0; i < numBuff-1; i++)
    {
        ((cl_mem*)buffers)[i] = clCreateBuffer(ctx, CL_MEM_READ_WRITE, buffLen, NULL, NULL);
    }

    //cl_event evt;
    for(unsigned i = 0; i < numIter; i++)
    {
        clSetKernelArg(buffUp, 0, sizeof(unsigned), &buffLen);

        for(unsigned j = 0; j < numBuff; j++)
        {
            clSetKernelArg(buffUp, j+1, sizeof(void*), &buffers[j]);
        }

        size_t global_work[3] = {buffLen, 1, 1};
        size_t local_work[3] = {256, 1, 1};

        clEnqueueNDRangeKernel(this_queue, buffUp, 1, 0, global_work, local_work, (i>0) ? 1 : 0, (i>0) ? &evts[i%2] : NULL, &evts[(i+1)%2]);
    }

    clFinish(this_queue);

    //unsigned long times[4];
    //populateKernelTimes( &evt, &times[0], &times[1], &times[2], &times[3] );
    //printf("kernel time: %lf ms\n", (times[3] - times[2]) / 1000000.0);

    for(unsigned i = 0; i < numBuff; i++)
    {
        clReleaseMemObject(((cl_mem*)buffers)[i]);
    }
    free(buffers);
}

void runSVM(cl_command_queue this_queue, cl_context ctx, cl_program prog, unsigned numBuff, unsigned numIter)
{
    cl_int cl_err;

    char kernName[32] = "sumArr";
    //char kernName[32] = "sumArrMid";
    //char kernName[32] = "sumArrHvy";

    cl_kernel buffUp = clCreateKernel(prog, kernName, &cl_err);
    check_cl_error(__FILE__, __LINE__, cl_err);
    clReleaseProgram(prog);

    void** buffers;
    buffers = (void**)malloc(sizeof(void*) * numBuff);
    buffers[numBuff-1] = clSVMAlloc(ctx, CL_MEM_READ_WRITE, sizeof(unsigned), 0);

    std::stringstream out_cs;

    unsigned buffLen = 4096;
    cl_event evts[2];
    for(unsigned i = 0; i < numBuff-1; i++)
    {
        buffers[i] = clSVMAlloc(ctx, CL_MEM_READ_WRITE, buffLen, 0);
        cl_err = clEnqueueSVMMap(this_queue, CL_TRUE, CL_MAP_WRITE, buffers[i], buffLen, 0, 0, 0);
        check_cl_error(__FILE__, __LINE__, cl_err);

        for(unsigned j = 0; j < buffLen; j++)
        {
            *((char*)(buffers[i]) + j) = i;
        }

        clEnqueueSVMUnmap(this_queue, buffers[i], 0, 0, &evts[0]);
    }

    cl_event evt;
    for(unsigned i = 0; i < numIter; i++)
    {
        clSetKernelArg(buffUp, 0, sizeof(unsigned), &buffLen);

        clSetKernelArgSVMPointer(buffUp, 1, buffers[(i % (numBuff-1))]);
        clSetKernelArgSVMPointer(buffUp, 2, buffers[numBuff-1]);

        size_t global_work[3] = {buffLen, 1, 1};
        size_t local_work[3] = {256, 1, 1};

        clEnqueueNDRangeKernel(this_queue, buffUp, 1, 0, global_work, local_work, 0, NULL, &evt);//1, &evts[j%2], &evts[(j+1)%2]);
    }

    clFinish(this_queue);

    //unsigned long times[4];
    //populateKernelTimes( &evt, &times[0], &times[1], &times[2], &times[3] );
    //printf("kernel time: %lf ms\n", (times[3] - times[2]) / 1000000.0);

    for(unsigned i = 0; i < numBuff; i++)
    {
        clSVMFree(ctx, buffers[i]);
    }
    free(buffers);
}

static void run_1d_test(const cl_device_id device, const cl_context context,
    const cl_command_queue cmd_queue, const cl_program program,
    uint64_t width)
{
    cl_int cl_err;
    size_t num_work_items;
    cl_kernel test_kernel = setup_kernel(program, "test_1d");

    printf("\nRunning 1D Image Test...\n");
    // In this case, we are going to create an image of the desired buffer
    // size. If the maximum image dimensions won't allow it to fit, then we
    // reduce our dimensions so that it does.
    size_t max_width = get_image_width(device, 1);

    // Checking against "* 2" because we want to leave room for the canary
    // values in the buffer overflow detector.
    if (max_width < (width * 2))
    {
        width = max_width/2;
        printf("    Requested image width is too large. ");
        printf("Reducing width to: %llu\n",
                (long long unsigned)width);
    }
    uint64_t buffer_size = width;
    printf("Using an image of size (H = size): %llu = %llu\n",
        (long long unsigned)width,(long long unsigned)buffer_size);

    // In this case, we are going to create a cl_mem image buffer of the right
    // size. The kernel will then correctly copy the right amount of data
    // into that buffer, and then the program will exit.
    // This will not create a buffer overflow.
    cl_image_desc description;
    description.image_type = CL_MEM_OBJECT_IMAGE1D;
    description.image_width = width;
    description.image_array_size = 1;
    description.image_row_pitch = 0;
    description.image_slice_pitch = 0;
    description.num_mip_levels = 0;
    description.num_samples = 0;
    description.buffer = NULL;

    // Each image entry should be a single channel made of 4-byte floats.
    cl_image_format format;
    format.image_channel_order = CL_R;
    format.image_channel_data_type = CL_FLOAT;

    cl_mem_flags flags;
    flags = CL_MEM_WRITE_ONLY;

    cl_mem good_buffer = clCreateImage(context, flags, &format, &description,
            NULL, &cl_err);
    check_cl_error(__FILE__, __LINE__, cl_err);

    cl_err = clSetKernelArg(test_kernel, 0, sizeof(cl_mem), &good_buffer);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clSetKernelArg(test_kernel, 1, sizeof(cl_uint), &width);
    check_cl_error(__FILE__, __LINE__, cl_err);

    // Each work item will touch one pixel of the image, so we want there to
    // be the same number of work items as pixels.
    // If the maximum work items we can launch won't reach the end of the
    // buffer, that is OK. Then we definitely won't have an overflow.
    num_work_items = width;

    printf("Launching %lu work items to write up to %llu pixels.\n",
            num_work_items, (long long unsigned)buffer_size);
    printf("\nImage1D Test...\n");
    cl_err = clEnqueueNDRangeKernel(cmd_queue, test_kernel, 1, NULL,
        &num_work_items, NULL, 0, NULL, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);
    clFinish(cmd_queue);
    printf("Done.\n");
    clReleaseMemObject(good_buffer);
}

static void run_2d_test(const cl_device_id device, const cl_context context,
    const cl_command_queue cmd_queue, const cl_program program,
    uint64_t width, uint64_t height, uint32_t numImg)
{
    cl_int cl_err;
    uint64_t buffer_size = width * height;
    size_t num_work_items[2];
    cl_kernel test_kernel;

    if(numImg == 1)
        test_kernel = setup_kernel(program, "test_2d");
    else
    {
        char kernName[32];
        sprintf(kernName, "test_2d_%ubuff", numImg);
        test_kernel = setup_kernel(program, kernName);
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
    buffer_size = height * width;
    printf("Using an image of size (H x W = size): %llu x %llu = %llu\n",
        (long long unsigned)height, (long long unsigned)width,
        (long long unsigned)buffer_size);

    // In this case, we are going to create a cl_mem image buffer of the right
    // size. The kernel will then correctly copy the right amount of data
    // into that buffer, and then the program will exit.
    // This will not create a buffer overflow.
    cl_image_desc description;
    description.image_type = CL_MEM_OBJECT_IMAGE2D;
    description.image_width = width;
    description.image_height = height;
    description.image_array_size = 1;
    description.image_row_pitch = 0;
    description.image_slice_pitch = 0;
    description.num_mip_levels = 0;
    description.num_samples = 0;
    description.buffer = NULL;

    // Each image entry should be a single channel made of 4-byte floats.
    cl_image_format format;
    format.image_channel_order = CL_R;
    format.image_channel_data_type = CL_FLOAT;

    cl_mem_flags flags;
    flags = CL_MEM_WRITE_ONLY;

    cl_mem *images;
    images = (cl_mem*)calloc(sizeof(cl_mem), numImg);
    for(uint32_t i = 0; i < numImg; i++)
    {
        images[i] = clCreateImage(context, flags, &format, &description,
                NULL, &cl_err);
        check_cl_error(__FILE__, __LINE__, cl_err);
    }

    cl_err = clSetKernelArg(test_kernel, 0, sizeof(cl_mem), &images[0]);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clSetKernelArg(test_kernel, 1, sizeof(cl_uint), &width);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clSetKernelArg(test_kernel, 2, sizeof(cl_uint), &height);
    check_cl_error(__FILE__, __LINE__, cl_err);

    for(uint32_t i = 1; i < numImg; i++)
    {
        cl_err = clSetKernelArg(test_kernel, 2+i, sizeof(cl_mem), &images[i]);
        check_cl_error(__FILE__, __LINE__, cl_err);
    }

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
    cl_err = clEnqueueNDRangeKernel(cmd_queue, test_kernel, 2, NULL,
        num_work_items, NULL, 0, NULL, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);
    clFinish(cmd_queue);
    printf("Done.\n");

    /*
    float *readOut;
    readOut = (float*)calloc(sizeof(float), width*height);
    size_t origin[] = {0,0,0};
    size_t region[] = {width, height, 1};
    clEnqueueReadImage(cmd_queue, images[0], CL_TRUE, origin, region, 4*width, 0, readOut, 0, NULL, NULL);
    for(uint32_t i = 0; i < width*height; i++)
    {
        if(i > 0 && i % width == 0)
            printf("\n");
        printf("%li:%lx ", i%width, (long unsigned)readOut[i]);
    }
    printf("\n");
    */

    for(uint32_t i = 0; i < numImg; i++)
    {
        clReleaseMemObject(images[i]);
    }
    free(images);
}

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

    // In this case, we are going to create a cl_mem image buffer of the right
    // size. The kernel will then correctly copy the right amount of data
    // into that buffer, and then the program will exit.
    // This will not create a buffer overflow.
    cl_image_desc description;
    description.image_type = CL_MEM_OBJECT_IMAGE3D;
    description.image_width = width;
    description.image_height = height;
    description.image_depth = depth;
    description.image_array_size = 1;
    description.image_row_pitch = 0;
    description.image_slice_pitch = 0;
    description.num_mip_levels = 0;
    description.num_samples = 0;
    description.buffer = NULL;

    // Each image entry should be a single channel made of 4-byte floats.
    cl_image_format format;
    format.image_channel_order = CL_R;
    format.image_channel_data_type = CL_FLOAT;

    cl_mem_flags flags;
    flags = CL_MEM_WRITE_ONLY;

    cl_mem good_buffer = clCreateImage(context, flags, &format, &description,
            NULL, &cl_err);
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

int main(int argc, char *argv[])
{
    uint64_t numBuff = 2, numIter = 1;
    char* dir;
    bool useClmem = false, I1D = false, I2D = false, I3D = false;

    if(argc > 1)
        dir = argv[1];
    else {
        return 0;
    }
    if(argc > 2)
        numBuff = atoi(argv[2]);
    if(argc > 3)
        numIter = atoi(argv[3]);
    if(argc > 4)
    {
        if(strcmp(argv[4], "Q") == 0) useClmem = true;
        if(strcmp(argv[4], "I1D") == 0) I1D = true;
        if(strcmp(argv[4], "I2D") == 0) I2D = true;
        if(strcmp(argv[4], "I3D") == 0) I3D = true;
    }

    cl_int cl_err;
    cl_command_queue this_queue;

    cl_platform_id platform;
    cl_uint num_platforms;
    cl_err = clGetPlatformIDs(1, &platform, &num_platforms);
    check_cl_error(__FILE__, __LINE__, cl_err);

    cl_device_id device;
    cl_uint num_dev;
    cl_err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, &num_dev);
    check_cl_error(__FILE__, __LINE__, cl_err);

    cl_context ctx;
    cl_context_properties cps[3] = {CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0};
    ctx = clCreateContext(cps, 1, &device, NULL, NULL, &cl_err);
    check_cl_error(__FILE__, __LINE__, cl_err);

    cl_queue_properties props[3] = {CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0};
    this_queue = clCreateCommandQueueWithProperties(ctx, device, props, &cl_err);
    check_cl_error(__FILE__, __LINE__, cl_err);

    char clfileName[1024];
    strcpy(clfileName, dir);
    strcat(clfileName, "bufferUp.cl");

    FILE *clfile = fopen(clfileName, "r");
    fseek(clfile, 0, SEEK_END);
    long size = ftell(clfile);
    rewind(clfile);
    char* source = (char*)calloc(sizeof(char),size+1);
    size_t sizeRead = fread(source, 1, size, clfile);
    if(sizeRead){}
    fclose(clfile);
    //printf("%s\n", source);


    const char *slist[2] = {source, 0};
    cl_program prog = clCreateProgramWithSource(ctx, 1, slist, NULL, &cl_err);
    check_cl_error(__FILE__, __LINE__, cl_err);

    free(source);

    cl_err = clBuildProgram(prog, 0, NULL, "-cl-std=CL2.0", NULL, NULL);
    if (cl_err == CL_BUILD_PROGRAM_FAILURE)
    {
        cl_device_id devId;
        clGetContextInfo(ctx, CL_CONTEXT_DEVICES, sizeof(cl_device_id), &devId, 0);
        // Determine the size of the log
        size_t log_size;
        clGetProgramBuildInfo(prog, devId, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

        // Allocate memory for the log
        char *log = (char *) malloc(log_size);

        // Get the log
        clGetProgramBuildInfo(prog, devId, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

        // Print the log
        printf("%s\n", log);
        free(log);
    }
    check_cl_error(__FILE__, __LINE__, cl_err);

    uint64_t buffer_size = numBuff;
    if(useClmem)
        runClMem(this_queue, ctx, prog, numBuff, numIter);
    else if(I1D)
    {
        uint64_t width;
        width = buffer_size;
        for(unsigned i=0; i < numIter; i++)
            run_1d_test(device, ctx, this_queue, prog, width);
    }
    else if(I2D)
    {
        uint32_t numImg = numBuff;
        buffer_size = 256*256;
        //buffer_size = 1024*1024;

        uint64_t width, height;
        width = pow(buffer_size, 1.0 / 2.0);
        height = width;
        for(unsigned i=0; i < numIter; i++)
            run_2d_test(device, ctx, this_queue, prog, width, height, numImg);
    }
    else if(I3D)
    {
        uint64_t width, height, depth;
        width = pow(buffer_size, 1.0 / 3.0);
        height = width;
        depth = width;
        for(unsigned i=0; i < numIter; i++)
            run_3d_test(device, ctx, this_queue, prog, width, height, depth);
    }
    else
        runSVM(this_queue, ctx, prog, numBuff, numIter);


    return 0;
}




