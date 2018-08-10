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

#include "common_test_functions.h"
#include <string.h>
#include <errno.h>

static void print_help(const char * const description)
{
    fprintf(stderr, "Buffer Overflow Test App -- %s\n", description);
    fprintf(stderr, "Command line parameters:\n");
    fprintf(stderr, "   -h, --help: Print this help menu.\n");
    fprintf(stderr, "The following parameters are optional:\n");
    fprintf(stderr, "   -p, --platform: Choose OpenCL platform (default 0)\n");
    fprintf(stderr, "   -d, --device: Choose OpenCL device (default 0)\n");
    fprintf(stderr, "   -t, --type: Choose the OpenCL device type.\n");
    fprintf(stderr, "       cpu/gpu/accelerator/custom - default=gpu\n");
}

int device_supports_cl2plus(cl_device_id device)
{
    int ret = 1;

    cl_int cl_err;
    size_t size_of_str = 0;
    cl_err = clGetDeviceInfo(device, CL_DEVICE_VERSION, 0, NULL, &size_of_str);
    check_cl_error(__FILE__, __LINE__, cl_err);
    char *string = malloc(size_of_str);
    cl_err = clGetDeviceInfo(device, CL_DEVICE_VERSION, size_of_str, string,
            NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);
    const char * ocl1 = "OpenCL 1";
    if (strncmp(ocl1, string, strlen(ocl1)) == 0)
    {
        fprintf(stderr, "\n\nOnly supports OpenCL 1\n");
        ret = 0;
    }
    free(string);

    return ret;
}

int device_supports_svm(cl_device_id device, int check_fine_grain)
{
#ifdef CL_VERSION_2_0
    cl_int cl_err;
    cl_device_svm_capabilities caps;
    cl_err = clGetDeviceInfo(device, CL_DEVICE_SVM_CAPABILITIES,
            sizeof(cl_device_svm_capabilities), &caps, NULL);
    if (cl_err == CL_INVALID_VALUE)
        return 0;
    check_cl_error(__FILE__, __LINE__, cl_err);
    if (check_fine_grain == 0 && (caps & CL_DEVICE_SVM_COARSE_GRAIN_BUFFER))
        return 1;
    if (check_fine_grain == 1 && (caps & CL_DEVICE_SVM_FINE_GRAIN_BUFFER))
        return 1;
    return 0;
#else
    (void)device;
    (void)check_fine_grain;
    return 0;
#endif
}

int device_supports_dev_queue(cl_device_id device)
{
#ifdef CL_VERSION_2_0
    cl_int cl_err;
    cl_uint queues;
    cl_err = clGetDeviceInfo(device, CL_DEVICE_MAX_ON_DEVICE_QUEUES,
            sizeof(cl_uint), &queues, NULL);
    if (cl_err == CL_INVALID_VALUE)
        return 0;
    check_cl_error(__FILE__, __LINE__, cl_err);
    if (queues > 0)
        return 1;
    return 0;
#else
    (void)device;
    return 0;
#endif
}

void check_opts(const int argc, char** argv, const char *description,
        uint32_t * const platform, uint32_t * const device,
        cl_device_type * const dev_type)
{
    const char* const opts = "hs:p:d:t:";
    const struct option long_opts[] = {
            {"help", 0, NULL, 'h'},
            {"size", 1, NULL, 's'},
            {"platform", 1, NULL, 'p'},
            {"device", 1, NULL, 'd'},
            {"type", 1, NULL, 't'},
            {NULL, 0, NULL, 0}
    };

    if (argv == NULL || description == NULL || platform == NULL ||
            device == NULL)
    {
        fprintf(stderr, "Incorrectly passing arguments to check_opts\n");
        fprintf(stderr, "Pointers were: %p %p %p %p\n", (void*)argv,
                (void*)description, (void*)platform, (void*)device);
        fprintf(stderr, "Error at %s:%d\n", __FILE__, __LINE__);
        exit(-1);
    }

    *platform = 0;
    *device = 0;
    *dev_type = CL_DEVICE_TYPE_GPU;

    while (1)
    {
        int retval = getopt_long(argc, argv, opts, long_opts, NULL);
        if (retval == -1)
            return;
        switch (retval)
        {
            case 'p':
                *platform = (uint32_t)atoi(optarg);
                break;
            case 'd':
                *device = (uint32_t)atoi(optarg);
                break;
            case 't':
                if (!strcmp(optarg, "gpu") || !strcmp(optarg, "GPU"))
                    *dev_type = CL_DEVICE_TYPE_GPU;
                else if (!strcmp(optarg, "cpu") || !strcmp(optarg, "CPU"))
                    *dev_type = CL_DEVICE_TYPE_CPU;
                else if (!strcmp(optarg, "accelerator"))
                    *dev_type = CL_DEVICE_TYPE_ACCELERATOR;
#ifdef CL_VERSION_1_2
                else if (!strcmp(optarg, "custom"))
                    *dev_type = CL_DEVICE_TYPE_CUSTOM;
#endif
                else
                {
                    fprintf(stderr, "Unknown device type: %s\n", optarg);
                    print_help(description);
                    exit(-1);
                }
                break;
            case 'h':
            case '?':
            default:
                print_help(description);
                exit(-1);
        }
    }
}

cl_platform_id setup_platform(const uint32_t platform_to_use)
{
    cl_int cl_err;
    cl_uint num_platforms;

    printf("Searching for platforms...\n");

    cl_err = clGetPlatformIDs(0, NULL, &num_platforms);
    check_cl_error(__FILE__, __LINE__, cl_err);

    if (num_platforms <= platform_to_use)
    {
        fprintf(stderr, "Requested to use platform %u\n", platform_to_use);
        fprintf(stderr, "But there are only %u platforms in the system!\n",
                num_platforms);
        fprintf(stderr, "Quitting in error. %s:%d\n", __FILE__, __LINE__);
        exit(-1);
    }

    cl_platform_id *platform_ids;
    platform_ids= calloc(num_platforms, sizeof(cl_platform_id));
    if (platform_ids == NULL)
    {
        fprintf(stderr, "Unable to calloc(%u, %zu) at %s:%d\n", num_platforms,
                sizeof(cl_platform_id), __FILE__, __LINE__);
        exit(-1);
    }
    cl_err = clGetPlatformIDs(num_platforms, platform_ids, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);

    cl_platform_id plat_to_return = platform_ids[platform_to_use];

    size_t platform_name_len = 0;
    cl_err = clGetPlatformInfo(plat_to_return, CL_PLATFORM_NAME, 0, NULL,
            &platform_name_len);
    check_cl_error(__FILE__, __LINE__, cl_err);

    char *platform_name = calloc(platform_name_len, sizeof(char));
    if (platform_name == NULL)
    {
        fprintf(stderr, "Unable to calloc(%zu) at %s:%d\n",
                platform_name_len, __FILE__, __LINE__);
        exit(-1);
    }
    cl_err = clGetPlatformInfo(plat_to_return, CL_PLATFORM_NAME,
            platform_name_len, platform_name, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);
    printf("    Using platform: %s\n", platform_name);

    free(platform_ids);
    free(platform_name);
    return plat_to_return;
}

cl_device_id setup_device(const uint32_t device_to_use,
        const uint32_t platform_to_use, const cl_platform_id platform,
        const cl_device_type dev_type)
{
    cl_int cl_err;
    cl_uint num_devices;

    printf("Searching for devices...\n");

    cl_err = clGetDeviceIDs(platform, dev_type, 0, NULL,
            &num_devices);
    check_cl_error(__FILE__, __LINE__, cl_err);

    if (num_devices <= device_to_use)
    {
        fprintf(stderr, "Requested to use device %u on platform %u\n",
                device_to_use, platform_to_use);
        fprintf(stderr, "But there are only %u GPU devices on this platform.\n",
                num_devices);
        fprintf(stderr, "Quitting in error. %s:%d\n", __FILE__, __LINE__);
        exit(-1);
    }

    cl_device_id *device_ids = calloc(num_devices, sizeof(cl_device_id));
    if (device_ids == NULL)
    {
        fprintf(stderr, "Unable to calloc(%u, %zu) at %s:%d\n", num_devices,
                sizeof(cl_device_id), __FILE__, __LINE__);
        exit(-1);
    }
    cl_err = clGetDeviceIDs(platform, dev_type, num_devices,
            device_ids, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);

    cl_device_id dev_to_return = device_ids[device_to_use];

    size_t device_name_len = 0;
    cl_err = clGetDeviceInfo(dev_to_return, CL_DEVICE_NAME, 0, NULL,
            &device_name_len);
    check_cl_error(__FILE__, __LINE__, cl_err);

    char *device_name = calloc(device_name_len, sizeof(char));
    if (device_name == NULL)
    {
        fprintf(stderr, "Unable to calloc(%zu) at %s:%d\n",
                device_name_len, __FILE__, __LINE__);
        exit(-1);
    }
    cl_err = clGetDeviceInfo(dev_to_return, CL_DEVICE_NAME, device_name_len,
            device_name, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);
    printf("    Using device: %s\n", device_name);

    free(device_ids);
    free(device_name);
    return dev_to_return;
}

cl_context setup_context(const cl_platform_id platform,
        const cl_device_id device)
{
    cl_int cl_err;
    cl_context ctxt_to_return;

    cl_context_properties properties[] = { CL_CONTEXT_PLATFORM,
        (cl_context_properties)platform, 0 };
    ctxt_to_return = clCreateContext(properties, 1, &device, NULL, NULL,
            &cl_err);
    check_cl_error(__FILE__, __LINE__, cl_err);
    return ctxt_to_return;
}

cl_command_queue setup_cmd_queue(const cl_context context,
        const cl_device_id device)
{
    cl_int cl_err;
    cl_command_queue queue_to_return;
#ifdef CL_VERSION_2_0
    queue_to_return = clCreateCommandQueueWithProperties(context, device, NULL,
            &cl_err);
#else
    queue_to_return = clCreateCommandQueue(context, device, 0, &cl_err);
#endif
    check_cl_error(__FILE__, __LINE__, cl_err);
    return queue_to_return;
}

cl_program setup_program(const cl_context context,
        const cl_uint num_source_strings, const char **source,
        const cl_device_id device)
{
    cl_int cl_err;
    cl_program program = clCreateProgramWithSource(context, num_source_strings,
            source, NULL, &cl_err);
    check_cl_error(__FILE__, __LINE__, cl_err);
#ifdef CL_VERSION_2_0
    char build_opt[] = "-cl-std=CL2.0";
#else
    char build_opt[] = "";
#endif
    cl_err = clBuildProgram(program, 1, &device, build_opt, NULL, NULL);
    if (cl_err != CL_SUCCESS)
    {
        size_t log_size;
        char *log;
        cl_int old_err = cl_err;
        cl_err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                0, NULL, &log_size);
        check_cl_error(__FILE__, __LINE__, cl_err);
        log = calloc(log_size, sizeof(char));
        cl_err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                log_size, log, NULL);
        check_cl_error(__FILE__, __LINE__, cl_err);
        fprintf(stderr, "Build Failure!\n");
        fprintf(stderr, "Error returned was: %s\n", cluErrorString(old_err));
        fprintf(stderr, "Kernel:\n%s\n", *source);
        fprintf(stderr, "\nLog (log size: %zu):\n%s\n", log_size, log);
        free(log);
        exit(-1);
    }
    return program;
}

cl_kernel setup_kernel(const cl_program program, const char *kernel_name)
{
    cl_int cl_err;
    if (kernel_name == NULL)
    {
        fprintf(stderr, "ERROR. Passing bad kernel_name into %s in %s:%d\n",
                __func__, __FILE__, __LINE__);
        exit(-1);
    }
    cl_kernel kernel = clCreateKernel(program, kernel_name, &cl_err);
    check_cl_error(__FILE__, __LINE__, cl_err);
    return kernel;
}

size_t get_image_width(const cl_device_id device, const int image_dimensions)
{
    cl_int cl_err;
    size_t ret_val;
    cl_device_info check_this;

    if (image_dimensions == 1 || image_dimensions == 2)
        check_this = CL_DEVICE_IMAGE2D_MAX_WIDTH;
    else if (image_dimensions == 3)
        check_this = CL_DEVICE_IMAGE3D_MAX_WIDTH;
    else
    {
        fprintf(stderr, "Unsupported image dimension (%d) at %s:%d\n",
                image_dimensions, __FILE__, __LINE__);
        exit(-1);
    }
    cl_err = clGetDeviceInfo(device, check_this, sizeof(size_t), &ret_val,
            NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);
    return ret_val;
}

size_t get_image_height(const cl_device_id device, const int image_dimensions)
{
    cl_int cl_err;
    size_t ret_val;
    cl_device_info check_this;

    if (image_dimensions == 2)
        check_this = CL_DEVICE_IMAGE2D_MAX_HEIGHT;
    else if (image_dimensions == 3)
        check_this = CL_DEVICE_IMAGE3D_MAX_HEIGHT;
    else
    {
        fprintf(stderr, "Unsupported image dimension (%d) at %s:%d\n",
                image_dimensions, __FILE__, __LINE__);
        exit(-1);
    }
    cl_err = clGetDeviceInfo(device, check_this, sizeof(size_t), &ret_val,
            NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);
    return ret_val;
}

size_t get_image_depth(const cl_device_id device)
{
    cl_int cl_err;
    size_t ret_val;
    cl_device_info check_this = CL_DEVICE_IMAGE3D_MAX_DEPTH;

    cl_err = clGetDeviceInfo(device, check_this, sizeof(size_t), &ret_val,
            NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);
    return ret_val;
}

static FILE* open_fake_log_file(const char * log_loc)
{
    FILE* log_file = NULL;
    if (log_loc == NULL)
    {
        fprintf(stderr, "Incorrectly passed NULL to %s\n", __func__);
        exit(-1);
    }
    log_file = fopen(log_loc, "w");
    if (log_file != NULL)
    {
        fprintf(log_file, "This is a fake log file\n");
        fprintf(log_file, "It exists because some feature in this benchmark ");
        fprintf(log_file, "is not supported by the current OpenCL runtime.\n");
    }
    else
    {
        int err_val = errno;
        fprintf(stderr, "Could not open fake log file: %s\n", log_loc);
        fprintf(stderr, "   %s\n", strerror(err_val));
    }
    return log_file;
}

void output_fake_errors(const char* filename, const unsigned int num_errors)
{
    FILE * log_file = open_fake_log_file(filename);
    if (log_file != NULL)
    {
        fprintf(log_file, "Found a total of %u errors.\n", num_errors);
        fclose(log_file);
    }
    else
    {
        fprintf(stderr, "Fake log file open returned NULL\n");
        exit(-1);
    }
}

unsigned int get_image_data_size(const cl_image_format * const format)
{
    cl_channel_type type = format->image_channel_data_type;
    cl_channel_order order = format->image_channel_order;

    unsigned byteChannel = 1;
    unsigned numChannels = 1;

    switch(type)
    {
        case CL_SNORM_INT8:
        case CL_UNORM_INT8:
        case CL_SIGNED_INT8:
        case CL_UNSIGNED_INT8:
            byteChannel = 1;
            break;
        case CL_SNORM_INT16:
        case CL_UNORM_INT16:
        case CL_SIGNED_INT16:
        case CL_HALF_FLOAT:
        case CL_UNSIGNED_INT16:
            byteChannel = 2;
            break;
        case CL_UNORM_SHORT_565:
            byteChannel = 2;
            numChannels = 1; //CL_RGB or CL_RGBx, 3 channel 5,6,5
            break;
        case CL_UNORM_SHORT_555:
            byteChannel = 2;
            numChannels = 1; //CL_RGB or CL_RGBx, 4 channel x,5,5,5
            break;
#ifdef CL_VERSION_1_2
        case CL_UNORM_INT24:
            byteChannel = 3;
            break;
#endif
        case CL_SIGNED_INT32:
        case CL_UNSIGNED_INT32:
        case CL_FLOAT:
            byteChannel = 4;
            break;
        case CL_UNORM_INT_101010:
            byteChannel = 4;
            numChannels = 1; //CL_RGB or CL_RGBx, 4 channel x,10,10,10
            break;
        default:
            byteChannel = 0;
    }

    switch(order)
    {
        case CL_R:
        case CL_Rx:
        case CL_A:
            numChannels = 1;
            break;
        case CL_INTENSITY:
        case CL_LUMINANCE:
#ifdef CL_VERSION_1_2
        case CL_DEPTH:
#endif
            numChannels = 1;
            break;
        case CL_RG:
        case CL_RGx:
        case CL_RA:
            numChannels = 2;
            break;
        case CL_RGB:
        case CL_RGBx:
            //channels assigned in data type
            break;
        case CL_RGBA:
            numChannels = 4;
            break;
#ifdef CL_VERSION_2_0
        case CL_sRGB:
        case CL_sRGBx:
        case CL_sRGBA:
        case CL_sBGRA:
            numChannels = 4;
            break;
#endif
        case CL_ARGB:
        case CL_BGRA:
#ifdef CL_VERSION_2_0
        case CL_ABGR:
#endif
            numChannels = 4;
            break;
#ifdef CL_VERSION_1_2
        case CL_DEPTH_STENCIL:
            numChannels = 1;
            break;
#endif
        default:
            numChannels = 0;
    }

    return byteChannel * numChannels;
}

// Get the Unix environment variable that matches the env_var_nm_ input param.
// This is then stored into a newly malloced C-string that will be returned
// in the env_ parameter.
// padding to the memory allocation.
// Return value: 0 on success. Anything else is an error.
static int get_env_util( char **env_, const char *env_var_nm_ )
{
    char  *env_root;
    size_t var_len;
    if (env_ == NULL)
    {
        fprintf(stderr, "%s (%d) error: trying ", __func__, __LINE__);
        fprintf(stderr, "to store environment variable in NULL location.\n");
        return -1;
    }
    if (env_var_nm_ == NULL)
    {
        fprintf(stderr, "%s (%d) error: ", __func__, __LINE__);
        fprintf(stderr, "trying to find environment variable, ");
        fprintf(stderr, "but variable is NULL.\n");
        return -1;
    }
    env_root = getenv(env_var_nm_);
    var_len = (env_root != NULL) ? strlen(env_root) : 0;
    if ( var_len == 0 )
        return 0;
    int num_bytes = asprintf(env_, "%s", env_root);
	if (num_bytes <= 0)
	{
		fprintf(stderr, "asprintf in %s:%d failed\n", __FILE__, __LINE__);
		exit(-1);
	}
    if (*env_ == NULL)
    {
        fprintf(stderr, "%s (%d) error: asprintf failed\n", __func__, __LINE__);
        return -1;
    }
    return 0;
}

int images_are_broken(void)
{
    char * broken_images_envvar = NULL;
    if (getenv("CLARMOR_ROCM_HAWAII") == NULL)
        return 0;
    else
    {
        unsigned int ret_val = 0;
        if (!get_env_util(&broken_images_envvar, "CLARMOR_ROCM_HAWAII"))
        {
            if (broken_images_envvar != NULL)
            {
                ret_val = strtoul(broken_images_envvar, NULL, 0);
                free(broken_images_envvar);
            }
        }
        return ret_val;
    }
}
