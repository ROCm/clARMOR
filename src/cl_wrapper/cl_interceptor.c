/********************************************************************************
 * Copyright (c) 2016-2017 Advanced Micro Devices, Inc. All rights reserved.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ********************************************************************************/

#include <sys/time.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#include <CL/cl.h>

#include "util_functions.h"
#include "detector_defines.h"
#include "bufferOverflowDetect.h"
#include "cl_err.h"
#include "meta_data_lists/cl_event_lists.h"
#include "meta_data_lists/cl_kernel_lists.h"
#include "meta_data_lists/cl_memory_lists.h"
#include "meta_data_lists/cl_workaround_lists.h"
#include "meta_data_lists/dl_intercept_lists.h"
#include "wrapper_utils.h"
#include "overflow_error.h"

#include "dl_interceptor_internal.h"
#include "cl_interceptor_internal.h"
#include "cl_interceptor.h"

const char * OPENCL_NAME_SO = "libOpenCL.so.1";

pthread_mutex_t command_queue_cache_lock = PTHREAD_MUTEX_INITIALIZER;

__thread uint8_t internal_create = 0;
pthread_mutex_t memory_overhead_lock = PTHREAD_MUTEX_INITIALIZER;
uint64_t total_user_mem = 0;
uint64_t total_overhead_mem = 0;
uint64_t high_user_mem = 0;
uint64_t high_overhead_mem = 0;
uint64_t current_user_mem = 0;
uint64_t current_overhead_mem = 0;


#define CL_MSG(msg)                                                            \
{                                                                              \
    if (msg != NULL) {                                                         \
        det_fprintf(stderr, "CL_WRAPPER : %s (%d): %s\n", __func__, __LINE__, msg);\
    }                                                                          \
}

#define CL_INTERCEPTOR_FUNCTION( _FUNCNAME_ ) \
    static interceptor_cl##_FUNCNAME_ _FUNCNAME_ = NULL

#define CL_INTERCEPTOR_FUNCTION_ADDRESS(_FUNCNAME_) \
{ \
    char *error; \
    char func_name[128];\
    sprintf(func_name,"cl%s",#_FUNCNAME_); \
    _FUNCNAME_ = (interceptor_cl##_FUNCNAME_)__libc_dlsym(oclDllHandle, func_name); \
    error = dlerror(); \
    if (error != NULL) { \
        CL_MSG(error); \
    } \
}

#define GET_INTERCEPTOR_FUNCTION( _FUNCNAME_ ) \
    interceptor_cl##_FUNCNAME_ get##_FUNCNAME_( void) \
{ \
    return (_FUNCNAME_); \
}

/*************** intercepted functions ********************************************************/

CL_INTERCEPTOR_FUNCTION(GetDeviceInfo);

/* Command Queue APIs */
CL_INTERCEPTOR_FUNCTION(CreateCommandQueue);
#ifdef CL_VERSION_2_0
CL_INTERCEPTOR_FUNCTION(CreateCommandQueueWithProperties);
#endif
CL_INTERCEPTOR_FUNCTION(RetainCommandQueue);
CL_INTERCEPTOR_FUNCTION(ReleaseCommandQueue);

/* Memory Object APIs */
CL_INTERCEPTOR_FUNCTION(GetMemObjectInfo);
CL_INTERCEPTOR_FUNCTION(CreateBuffer);
CL_INTERCEPTOR_FUNCTION(CreateSubBuffer);
CL_INTERCEPTOR_FUNCTION(GetImageInfo);
#ifdef CL_VERSION_1_2
CL_INTERCEPTOR_FUNCTION(CreateImage);
#endif
CL_INTERCEPTOR_FUNCTION(CreateImage2D);
CL_INTERCEPTOR_FUNCTION(CreateImage3D);
CL_INTERCEPTOR_FUNCTION(RetainMemObject);
CL_INTERCEPTOR_FUNCTION(ReleaseMemObject);
CL_INTERCEPTOR_FUNCTION(EnqueueMapBuffer);
CL_INTERCEPTOR_FUNCTION(EnqueueUnmapMemObject);

/* SVM APIs */
#ifdef CL_VERSION_2_0
CL_INTERCEPTOR_FUNCTION(SVMAlloc);
CL_INTERCEPTOR_FUNCTION(SVMFree);
CL_INTERCEPTOR_FUNCTION(EnqueueSVMFree);
CL_INTERCEPTOR_FUNCTION(EnqueueSVMMap);
CL_INTERCEPTOR_FUNCTION(EnqueueSVMMemcpy);
CL_INTERCEPTOR_FUNCTION(EnqueueSVMMemFill);
CL_INTERCEPTOR_FUNCTION(EnqueueSVMUnmap);
#endif

/* Kernel APIs */
CL_INTERCEPTOR_FUNCTION(RetainKernel);
CL_INTERCEPTOR_FUNCTION(ReleaseKernel);
CL_INTERCEPTOR_FUNCTION(EnqueueNDRangeKernel);
CL_INTERCEPTOR_FUNCTION(EnqueueTask);
CL_INTERCEPTOR_FUNCTION(EnqueueNativeKernel);

/* Kernel argument APIs */
CL_INTERCEPTOR_FUNCTION(SetKernelArg);
#ifdef CL_VERSION_2_0
CL_INTERCEPTOR_FUNCTION(SetKernelArgSVMPointer);
#endif

/* Event APIs */
CL_INTERCEPTOR_FUNCTION(GetEventProfilingInfo);
CL_INTERCEPTOR_FUNCTION(RetainEvent);
CL_INTERCEPTOR_FUNCTION(ReleaseEvent);

/* Enqueued Commands APIs */
CL_INTERCEPTOR_FUNCTION(EnqueueReadBuffer);
CL_INTERCEPTOR_FUNCTION(EnqueueReadBufferRect);
CL_INTERCEPTOR_FUNCTION(EnqueueWriteBuffer);
CL_INTERCEPTOR_FUNCTION(EnqueueWriteBufferRect);
CL_INTERCEPTOR_FUNCTION(EnqueueFillBuffer);
CL_INTERCEPTOR_FUNCTION(EnqueueCopyBuffer);
CL_INTERCEPTOR_FUNCTION(EnqueueCopyBufferRect);

CL_INTERCEPTOR_FUNCTION(EnqueueReadImage);
CL_INTERCEPTOR_FUNCTION(EnqueueWriteImage);
CL_INTERCEPTOR_FUNCTION(EnqueueFillImage);
CL_INTERCEPTOR_FUNCTION(EnqueueCopyImage);
CL_INTERCEPTOR_FUNCTION(EnqueueCopyImageToBuffer);
CL_INTERCEPTOR_FUNCTION(EnqueueCopyBufferToImage);



/*******************************************************************************************************/
/************ internals *****************************************************/
//CL_INTERCEPTOR_FUNCTION_ADDRESS

#define GCC_VER (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)

static int cl_function_addresses( void* oclDllHandle )
{
    int ret = 0;
// GCC would normally complain about function pointers being assigned
// from object pointers. This is OK in POSIX, however, so ignore it.
#pragma GCC diagnostic push
#if !defined(__GNUC__) || GCC_VER >= 40800
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
    CL_INTERCEPTOR_FUNCTION_ADDRESS( GetDeviceInfo );
    CL_INTERCEPTOR_FUNCTION_ADDRESS( CreateCommandQueue );
#ifdef CL_VERSION_2_0
    CL_INTERCEPTOR_FUNCTION_ADDRESS( CreateCommandQueueWithProperties );
#endif
    CL_INTERCEPTOR_FUNCTION_ADDRESS( RetainCommandQueue );
    CL_INTERCEPTOR_FUNCTION_ADDRESS( ReleaseCommandQueue );


    CL_INTERCEPTOR_FUNCTION_ADDRESS( GetMemObjectInfo );
    CL_INTERCEPTOR_FUNCTION_ADDRESS( CreateBuffer );
    CL_INTERCEPTOR_FUNCTION_ADDRESS( CreateSubBuffer );
    CL_INTERCEPTOR_FUNCTION_ADDRESS( GetImageInfo );
#ifdef CL_VERSION_1_2
    CL_INTERCEPTOR_FUNCTION_ADDRESS( CreateImage );
#endif
    CL_INTERCEPTOR_FUNCTION_ADDRESS( CreateImage2D );
    CL_INTERCEPTOR_FUNCTION_ADDRESS( CreateImage3D );
    CL_INTERCEPTOR_FUNCTION_ADDRESS( RetainMemObject );
    CL_INTERCEPTOR_FUNCTION_ADDRESS( ReleaseMemObject );
    CL_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueMapBuffer );
    CL_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueUnmapMemObject );

#ifdef CL_VERSION_2_0
    CL_INTERCEPTOR_FUNCTION_ADDRESS( SVMAlloc );
    CL_INTERCEPTOR_FUNCTION_ADDRESS( SVMFree );
    CL_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueSVMFree );
    CL_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueSVMMap );
    CL_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueSVMMemcpy );
    CL_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueSVMMemFill );
    CL_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueSVMUnmap );
#endif

    /* Kernel Object APIs */
    CL_INTERCEPTOR_FUNCTION_ADDRESS( RetainKernel );
    CL_INTERCEPTOR_FUNCTION_ADDRESS( ReleaseKernel );
    CL_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueNDRangeKernel );
    CL_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueTask );
    CL_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueNativeKernel );

    CL_INTERCEPTOR_FUNCTION_ADDRESS( SetKernelArg);
#ifdef CL_VERSION_2_0
    CL_INTERCEPTOR_FUNCTION_ADDRESS( SetKernelArgSVMPointer );
#endif
    /* Event APIs */
    CL_INTERCEPTOR_FUNCTION_ADDRESS( GetEventProfilingInfo );
    CL_INTERCEPTOR_FUNCTION_ADDRESS( RetainEvent );
    CL_INTERCEPTOR_FUNCTION_ADDRESS( ReleaseEvent );

    /* Enqueued Commands APIs */
    CL_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueReadBuffer );
    CL_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueReadBufferRect );
    CL_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueWriteBuffer );
    CL_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueWriteBufferRect );
    CL_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueFillBuffer );
    CL_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueCopyBuffer );
    CL_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueCopyBufferRect );

    CL_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueReadImage );
    CL_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueWriteImage );
    CL_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueFillImage );
    CL_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueCopyImage );
    CL_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueCopyImageToBuffer );
    CL_INTERCEPTOR_FUNCTION_ADDRESS( EnqueueCopyBufferToImage );

#pragma GCC diagnostic pop
    return(ret);
}

void init_perf_outfile(void)
{
    FILE *perf_out_f = NULL;
    perf_out_f = fopen(global_tool_stats_outfile, "w");

    if(global_tool_stats_flags & STATS_KERN_ENQ_TIME)
    {
        fprintf(perf_out_f, "total_durr_us, enq_durr_us, checker_enqueue_overhead_us\n");
    }
    else if(global_tool_stats_flags & STATS_CHECKER_TIME)
    {
        fprintf(perf_out_f, "checker_time_us\n");
    }
    else if(global_tool_stats_flags & STATS_MEM_OVERHEAD)
    {
        fprintf(perf_out_f, "total_user_mem_B, total_overhead_mem_B, high_user_mem_B, high_overhead_mem_B\n");
    }
    fclose(perf_out_f);
}

void write_out_mem_perf_stats(void)
{
    FILE *perf_out_f = NULL;
    pthread_mutex_lock(&memory_overhead_lock);
    perf_out_f = fopen(global_tool_stats_outfile, "a");
    fprintf(perf_out_f, "%lu, %lu, %lu, %lu\n", total_user_mem, total_overhead_mem, high_user_mem, high_overhead_mem);
    fclose(perf_out_f);
    pthread_mutex_unlock(&memory_overhead_lock);
}

static void * oclDllHandle = NULL;
static int cl_wrapper_init( void )
{
    int ret = INT_MIN;
    if ( NULL == oclDllHandle )
    {
        // Load the dll and keep the handle to it
        oclDllHandle = dlopen( OPENCL_NAME_SO, RTLD_LAZY );
        // If the handle is valid, try to get the function address.
        if (NULL != oclDllHandle)
        {
            det_printf( "Loaded CL_WRAPPER\n" );
            // Get pointers to OCL functions with
            // CL_INTERCEPTOR_FUNCTION_ADDRESS macro
            ret = cl_function_addresses(oclDllHandle);
        }
        else
        {
            det_fprintf(stderr, "Unable to load CL_WRAPPER lib\n");
        }

        global_tool_stats_flags = get_tool_perf_envvar();
        global_tool_stats_outfile = get_tool_perf_outfile_envvar();
        if(global_tool_stats_outfile)
            init_perf_outfile();
    }
    return(ret);
}


CL_API_ENTRY cl_int CL_API_CALL
clGetDeviceInfo(cl_device_id device,
            cl_device_info param_name, size_t param_value_size,
            void *param_value, size_t *param_value_size_ret)
{
    cl_int ret = 0;
    if ( GetDeviceInfo )
    {
        ret = GetDeviceInfo(device, param_name,
                param_value_size,
                param_value,
                param_value_size_ret);

        cl_int cl_err;
        size_t *p_val = param_value;
        uint64_t width, height, depth;
        switch(param_name)
        {
            case CL_DEVICE_IMAGE2D_MAX_WIDTH:
                *p_val -= IMAGE_POISON_WIDTH;
                break;
            case CL_DEVICE_IMAGE2D_MAX_HEIGHT:
                *p_val -= IMAGE_POISON_HEIGHT;
                break;
            case CL_DEVICE_IMAGE3D_MAX_WIDTH:
                *p_val -= IMAGE_POISON_WIDTH;
                break;
            case CL_DEVICE_IMAGE3D_MAX_HEIGHT:
                *p_val -= IMAGE_POISON_HEIGHT;
                break;
            case CL_DEVICE_IMAGE3D_MAX_DEPTH:
                *p_val -= IMAGE_POISON_DEPTH;
                break;
#ifdef CL_VERSION_1_2
            case CL_DEVICE_IMAGE_MAX_BUFFER_SIZE:
                *p_val -= IMAGE_POISON_WIDTH;
                break;
#endif
            case CL_DEVICE_MAX_MEM_ALLOC_SIZE:
                //largest canary space is from slice duplication for depth
                cl_err = GetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_WIDTH,
                        sizeof(uint64_t), &width, NULL);
                check_cl_error(__FILE__, __LINE__, cl_err);
                cl_err = GetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_HEIGHT,
                        sizeof(uint64_t), &height, NULL);
                check_cl_error(__FILE__, __LINE__, cl_err);
                depth = *p_val / (width * height);

                depth -= IMAGE_POISON_DEPTH;
                height -= IMAGE_POISON_HEIGHT;
                width -= IMAGE_POISON_WIDTH;
                *p_val = width * height * depth;
                break;
        }
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }
    return(ret);
}

__attribute__((constructor)) static void wrapper_constructor ( void )
{
    cl_wrapper_init();
}

__attribute__((destructor)) static void cl__destructor ( void )
{
    if(global_tool_stats_flags & STATS_MEM_OVERHEAD)
        write_out_mem_perf_stats();

    finalize_detector();
}


/* Command Queue APIs */
    CL_API_ENTRY cl_command_queue CL_API_CALL
clCreateCommandQueue(cl_context                     context ,
        cl_device_id                   device ,
        cl_command_queue_properties    properties ,
        cl_int *                       errcode_ret )
{
    cl_command_queue ret = 0;
    if ( CreateCommandQueue )
    {
        cl_command_queue_properties local_prop = properties;
        if(global_tool_stats_flags & STATS_CHECKER_TIME)
        {
            local_prop |= CL_QUEUE_PROFILING_ENABLE;
        }
        pthread_mutex_lock(&command_queue_cache_lock);

        ret =
            CreateCommandQueue(context ,
                    device ,
                    local_prop,
                    errcode_ret);


        // We cache these command queues so that we only create one per context.
        // We must do this because we leak command queues in
        // clReleaseCommandQueue due to memory corruption issues in the
        // OpenCL library. We can run out of resources if we aren't judicious
        // in our command queue creation.
        commandQueueCache *insertme = calloc(1, sizeof(commandQueueCache));
        insertme->handle = context;
        insertme->cached_queue = ret;
        insertme->ref_count = 1;
        int err = commandQueueCache_insert(get_cmd_queue_cache(), insertme);
        if (err != 0)
            det_fprintf(stderr, "WARNING: Failed to insert command queue into cache at %s:%d\n", __FILE__, __LINE__);

        pthread_mutex_unlock(&command_queue_cache_lock);
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }
    return(ret);
}

#ifdef CL_VERSION_2_0
    CL_API_ENTRY cl_command_queue CL_API_CALL
clCreateCommandQueueWithProperties(
        cl_context                     context ,
        cl_device_id                   device ,
        const cl_queue_properties *    properties ,
        cl_int *                       errcode_ret )
{
    cl_command_queue ret = 0;
    if ( CreateCommandQueueWithProperties )
    {
        const cl_queue_properties* updated_prop = properties;
        cl_queue_properties* temp_prop = 0;
        if(global_tool_stats_flags & STATS_CHECKER_TIME)
        {
            temp_prop = add_profiling(properties);
            if (temp_prop != NULL)
                updated_prop = temp_prop;
        }

        ret = CreateCommandQueueWithProperties(context, device, updated_prop, errcode_ret);

        /*
         * Work around for bug
         * clCreateCommandQueueWithProperties using the CL_QUEUE_ON_DEVICE property
         *  does not set the error code return value
         */
        uint32_t flags = 0;
        if(updated_prop)
        {
            if(updated_prop[0] == CL_QUEUE_PROPERTIES)
                flags = updated_prop[1];
            else if(updated_prop[2] == CL_QUEUE_PROPERTIES)
                flags = updated_prop[3];
        }

        if(errcode_ret != NULL && flags & CL_QUEUE_ON_DEVICE)
        {
            cl_int cl_err, size;
            cl_err = clGetCommandQueueInfo(ret, CL_QUEUE_SIZE, sizeof(cl_int), &size, NULL);
            if(cl_err != CL_INVALID_COMMAND_QUEUE)
                *errcode_ret = CL_SUCCESS;
            else
                *errcode_ret = CL_INVALID_COMMAND_QUEUE;
        }

        if(global_tool_stats_flags & STATS_CHECKER_TIME)
        {
            free(temp_prop);
        }

        commandQueueCache *insertme = calloc(1, sizeof(commandQueueCache));
        insertme->handle = context;
        insertme->cached_queue = ret;
        insertme->ref_count = 1;

        pthread_mutex_lock(&command_queue_cache_lock);

        int err = commandQueueCache_insert(get_cmd_queue_cache(), insertme);
        if (err != 0)
            det_fprintf(stderr, "WARNING: Failed to insert command queue into cache at %s:%d\n", __FILE__, __LINE__);

        pthread_mutex_unlock(&command_queue_cache_lock);
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }
    return(ret);
}
#endif

    CL_API_ENTRY cl_int CL_API_CALL
clRetainCommandQueue(cl_command_queue command_queue )
{
    cl_int err = -100;
    if ( RetainCommandQueue )
    {
        cl_int cl_err;
        pthread_mutex_lock(&command_queue_cache_lock);

        err = RetainCommandQueue( command_queue );

        cl_context context;
        size_t context_size;
        cl_err = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, 0, NULL, &context_size);
        check_cl_error(__FILE__, __LINE__, cl_err);
        cl_err = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, context_size, &context, NULL);
        check_cl_error(__FILE__, __LINE__, cl_err);
        commandQueueCache *findme = commandQueueCache_find(get_cmd_queue_cache(), context);
        if(findme != NULL)
            findme->ref_count++;

        pthread_mutex_unlock(&command_queue_cache_lock);
    }
    else
    {
        CL_MSG("NOT INTERCEPTED YET!");
    }
    return(err);
}

    CL_API_ENTRY cl_int CL_API_CALL
clReleaseCommandQueue(cl_command_queue command_queue )
{
    cl_int err = -100;
    if ( ReleaseCommandQueue )
    {
        if (command_queue)
        {
            // Purposefully leak command queues in detector. We currently do this
            // because the AMD APP SDK OpenCL runtime has a memory corruption
            // error that happens somewhere inside clReleaseCommandQueue(). We
            // trigger this somewhat frequently.
            // Skipping the real ReleaseCommandQueue avoids this issue.
#if 0
            if(0)
            {
                cl_int cl_err;
                pthread_mutex_lock(&command_queue_cache_lock);

                cl_context context;
                size_t context_size;
                cl_err = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, 0, NULL, &context_size);
                check_cl_error(__FILE__, __LINE__, cl_err);
                cl_err = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, context_size, &context, NULL);
                check_cl_error(__FILE__, __LINE__, cl_err);
                commandQueueCache *findme = commandQueueCache_find(get_cmd_queue_cache(), context);
                if(findme != NULL)
                {
                    findme->ref_count--;
                    if(findme->ref_count <= 0)
                    {
                        commandQueueCache *del = commandQueueCache_remove(get_cmd_queue_cache(), context);
                        commandQueueCache_delete(del);
                    }
                }

                pthread_mutex_unlock(&command_queue_cache_lock);
            }
#endif
            err = CL_SUCCESS;
        }
        else
            err = CL_INVALID_COMMAND_QUEUE;
        //err = ReleaseCommandQueue(command_queue);
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }
    return(err);
}



/*******************************************************************************************************/
/* Memory Object APIs */
CL_API_ENTRY cl_int CL_API_CALL
clGetMemObjectInfo(cl_mem memobj,
            cl_mem_info param_name, size_t param_value_size,
            void *param_value, size_t *param_value_size_ret)
{
    cl_int ret = 0;
    if ( GetMemObjectInfo )
    {
        ret = GetMemObjectInfo(memobj, param_name,
                param_value_size,
                param_value,
                param_value_size_ret);

        if(param_name == CL_MEM_SIZE)
        {
            cl_memobj *m1 = cl_mem_find(get_cl_mem_alloc(), memobj);
            if( m1 && m1->has_canary )
            {
                size_t *p_val = param_value;
                if(m1->is_image)
                {
                    uint64_t w, h, d, a;
                    size_t dataSize;
                    uint64_t img_size;
                    w = m1->image_desc.image_width;
                    h = m1->image_desc.image_height;
                    d = m1->image_desc.image_depth;
                    a = m1->image_desc.image_array_size;

                    if(a > 1)
                    {
                        if(h > 1)
                            a -= IMAGE_POISON_DEPTH;
                        else
                            a -= IMAGE_POISON_HEIGHT;
                    }
                    w = (w > 1) ? w - IMAGE_POISON_WIDTH : w;
                    h = (h > 1) ? h - IMAGE_POISON_HEIGHT : h;
                    d = (d > 1) ? d - IMAGE_POISON_DEPTH : d;

                    dataSize = getImageDataSize(&m1->image_format);
                    img_size = w * h * d * a * dataSize;

                    *p_val = img_size;
                }
                else
                {
                    *p_val = m1->size;
                }
            }
        }

    }
    else
    {
        CL_MSG("NOT FOUND!");
    }
    return(ret);
}

CL_API_ENTRY cl_mem CL_API_CALL
clCreateBuffer(cl_context   context,
        cl_mem_flags        flags,
        size_t              size,
        void *              host_ptr,
        cl_int *            errcode_ret)
{
    cl_mem ret = 0;
    if (CreateBuffer)
    {
        initialize_logging();

        if(size == 0)
        {
            *errcode_ret = CL_INVALID_BUFFER_SIZE;
            return NULL;
        }

        char *fill_ptr = 0;
        size_t size_aug = size;

        if(global_tool_stats_flags & STATS_MEM_OVERHEAD)
        {
            pthread_mutex_lock(&memory_overhead_lock);
            if(internal_create)
            {
                total_overhead_mem += size;
                current_overhead_mem += size;
            }
            else
            {
                total_user_mem += size;
                current_user_mem += size;
            }
            pthread_mutex_unlock(&memory_overhead_lock);
        }

        flags &= ~CL_MEM_WRITE_ONLY;
        flags &= ~CL_MEM_READ_ONLY;
        flags |= CL_MEM_READ_WRITE;

        if(flags & CL_MEM_USE_HOST_PTR)
        {
            fill_ptr = host_ptr;
        }
        else
        {
            fill_ptr = (char*)calloc(sizeof(char), size + POISON_REGIONS*POISON_FILL_LENGTH);

            if(fill_ptr)
            {
                uint32_t offset = 0;
#ifdef UNDERFLOW_CHECK
                memset(fill_ptr, POISON_FILL, POISON_FILL_LENGTH);
                offset = POISON_FILL_LENGTH;
#endif
                if(flags & CL_MEM_COPY_HOST_PTR)
                    memcpy(fill_ptr + offset, host_ptr, size);

                offset += size;
                memset(fill_ptr + offset, POISON_FILL, POISON_FILL_LENGTH);
                flags |= CL_MEM_COPY_HOST_PTR;

                if(global_tool_stats_flags & STATS_MEM_OVERHEAD)
                {
                    pthread_mutex_lock(&memory_overhead_lock);
                    total_overhead_mem += POISON_REGIONS*POISON_FILL_LENGTH;
                    current_overhead_mem += POISON_REGIONS*POISON_FILL_LENGTH;
                    pthread_mutex_unlock(&memory_overhead_lock);
                }
                size_aug += POISON_REGIONS*POISON_FILL_LENGTH;
            }
        }

        if(global_tool_stats_flags & STATS_MEM_OVERHEAD)
        {
            pthread_mutex_lock(&memory_overhead_lock);
            high_user_mem = (high_user_mem > current_user_mem) ? high_user_mem : current_user_mem;
            high_overhead_mem = (high_overhead_mem > current_overhead_mem) ? high_overhead_mem : current_overhead_mem;
            pthread_mutex_unlock(&memory_overhead_lock);
        }

        cl_int internal_err;
        cl_mem main_buff = 0;
        main_buff =
            CreateBuffer( context ,
                    flags ,
                    size_aug ,
                    fill_ptr ,
                    &internal_err );

        if(main_buff && !(flags & CL_MEM_USE_HOST_PTR))
        {
            cl_buffer_region sub_region;
            sub_region.origin = 0;
#ifdef UNDERFLOW_CHECK
            sub_region.origin = POISON_FILL_LENGTH;
#endif
            sub_region.size = size;
            cl_mem_flags passDownFlags, ptrFlags;
            ptrFlags = CL_MEM_USE_HOST_PTR | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR;
            passDownFlags = flags & ~ptrFlags;

            ret =
                CreateSubBuffer( main_buff,
                        passDownFlags,
                        CL_BUFFER_CREATE_TYPE_REGION,
                        (void*)&sub_region,
                        &internal_err );
            check_cl_error(__FILE__, __LINE__, internal_err);

            if(is_nvidia_platform(context))
            {
                // Force the buffer allocation to contiguous gpu memory space.
                // This will ensure that the whole buffer is in the same memory.
                // Later when we act on the sub-buffer, any overflows will spill
                //   into the main buffer.
                cl_command_queue command_queue;
                int weCreated = !getCommandQueueForContext(context, &command_queue);
                if(!weCreated)
                    clRetainCommandQueue(command_queue);

                void *read_ptr = malloc(size_aug);
                EnqueueReadBuffer(command_queue, main_buff, CL_TRUE, 0, size_aug, read_ptr,
                                    0, NULL, NULL);
                free(read_ptr);

                clReleaseCommandQueue(command_queue);
            }
        }
        else
        {
            ret = main_buff;
        }

        if(fill_ptr && fill_ptr != host_ptr)
            free(fill_ptr);

        if(ret)
        {
            cl_memobj *temp = (cl_memobj*)calloc(sizeof(cl_memobj), 1);
            temp->handle = ret;
            temp->is_image = 0;
            temp->context = context;
            temp->flags = flags;
            temp->size = size;
            if(flags & CL_MEM_USE_HOST_PTR)
                temp->host_ptr = host_ptr;
            else
            {
                temp->main_buff = main_buff;
                temp->host_ptr = NULL;
                temp->has_canary = 1;
            }
            temp->ref_count = 1;

            cl_mem_insert(get_cl_mem_alloc(), temp);
        }

        if(errcode_ret)
            *errcode_ret = internal_err;
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }
    return(ret);
}

CL_API_ENTRY cl_mem CL_API_CALL
    clCreateSubBuffer(
            cl_mem buffer,
            cl_mem_flags flags,
            cl_buffer_create_type buffer_create_type,
            const void *buffer_create_info,
            cl_int *errcode_ret)
{
    cl_mem ret = 0;
    if (CreateSubBuffer)
    {
        initialize_logging();

        cl_memobj *superBuff = cl_mem_find(get_cl_mem_alloc(), buffer);
        cl_buffer_region buffer_region_info = *(cl_buffer_region*)buffer_create_info;
        if(superBuff->has_canary == 1)
        {
            buffer = superBuff->main_buff;
#ifdef UNDERFLOW_CHECK
            buffer_region_info.origin += POISON_FILL_LENGTH;
#endif
        }

        ret =
            CreateSubBuffer(
                    buffer,
                    flags,
                    buffer_create_type,
                    &buffer_region_info,
                    errcode_ret);

        if(ret)
        {
            cl_memobj *temp = (cl_memobj*)calloc(sizeof(cl_memobj), 1);
            temp->handle = ret;
            temp->is_sub = 1;
            temp->context = superBuff->context;
            temp->flags = flags;
            temp->size = buffer_region_info.size;
            temp->origin = buffer_region_info.origin;
            if(superBuff->host_ptr)
                temp->host_ptr = (char*)superBuff->host_ptr + temp->origin;
            else
                temp->host_ptr = NULL;
            temp->ref_count = 1;

            cl_mem_insert(get_cl_mem_alloc(), temp);
        }
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }
    return(ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetImageInfo(cl_mem image,
        cl_mem_info param_name, size_t param_value_size,
        void *param_value, size_t *param_value_size_ret)
{
    cl_int ret = 0;
    if ( GetImageInfo )
    {
        ret = GetImageInfo(image, param_name,
                param_value_size,
                param_value,
                param_value_size_ret);

        uint8_t dim_req = 0;
        switch(param_name)
        {
            case CL_IMAGE_WIDTH:
            case CL_IMAGE_HEIGHT:
            case CL_IMAGE_DEPTH:
            case CL_IMAGE_ARRAY_SIZE:
                dim_req = 1;
        }

        size_t *p_val = param_value;
        if(dim_req && *p_val != 0)
        {
            cl_memobj *m1 = cl_mem_find(get_cl_mem_alloc(), image);
            if(m1 && m1->is_image)
            {
                uint64_t w, h, d, a;
                w = m1->image_desc.image_width;
                h = m1->image_desc.image_height;
                d = m1->image_desc.image_depth;
                a = m1->image_desc.image_array_size;

                switch(param_name)
                {
                    case CL_IMAGE_WIDTH:
                        w = (w > 1) ? w - IMAGE_POISON_WIDTH : w;
                        *p_val = w;
                        break;
                    case CL_IMAGE_HEIGHT:
                        h = (h > 1) ? h - IMAGE_POISON_HEIGHT : h;
                        *p_val = h;
                        break;
                    case CL_IMAGE_DEPTH:
                        d = (d > 1) ? d - IMAGE_POISON_DEPTH : d;
                        *p_val = d;
                        break;
                    case CL_IMAGE_ARRAY_SIZE:
                        if(a > 1)
                        {
                            if(h > 1)
                                a -= IMAGE_POISON_DEPTH;
                            else
                                a -= IMAGE_POISON_HEIGHT;
                        }
                        *p_val = a;
                }
            }
        }
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }
    return(ret);
}

#ifdef CL_VERSION_1_2
    CL_API_ENTRY cl_mem CL_API_CALL
clCreateImage(cl_context              context ,
        cl_mem_flags            flags ,
        const cl_image_format * image_format ,
        const cl_image_desc *   image_desc ,
        void *                  host_ptr ,
        cl_int *                errcode_ret )
{
    cl_mem ret = 0;
    if ( CreateImage )
    {
        if (!opencl_broken_images())
        {
            cl_memobj *temp = (cl_memobj*)calloc(sizeof(cl_memobj), 1);
            temp->is_image = 1;
            temp->context = context;
            temp->flags = flags;

            temp->image_format.image_channel_order = image_format->image_channel_order;
            temp->image_format.image_channel_data_type = image_format->image_channel_data_type;

            temp->image_desc.image_type = image_desc->image_type;
            temp->image_desc.image_width = image_desc->image_width;
            temp->image_desc.image_height = image_desc->image_height;
            temp->image_desc.image_depth = image_desc->image_depth;
            temp->image_desc.image_array_size = image_desc->image_array_size;
            temp->image_desc.image_row_pitch = image_desc->image_row_pitch;
            temp->image_desc.image_slice_pitch = image_desc->image_slice_pitch;
            temp->image_desc.num_mip_levels = image_desc->num_mip_levels;
            temp->image_desc.num_samples = image_desc->num_samples;
            temp->image_desc.buffer = image_desc->buffer;

            //doing some error correcting here
            //some of these fields may be unchecked for certain image types
            //i want to be able to use these fields generically so i'm enforcing standards here
            if(image_desc->image_type == CL_MEM_OBJECT_IMAGE1D)
            {
                temp->image_desc.image_height = 1;
                temp->image_desc.image_depth = 1;
                temp->image_desc.image_array_size = 1;
            }
            else if(image_desc->image_type == CL_MEM_OBJECT_IMAGE1D_BUFFER)
            {
                temp->image_desc.image_height = 1;
                temp->image_desc.image_depth = 1;
                temp->image_desc.image_array_size = 1;
            }
            else if(image_desc->image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY)
            {
                temp->image_desc.image_height = 1;
                temp->image_desc.image_depth = 1;
            }
            else if(image_desc->image_type == CL_MEM_OBJECT_IMAGE2D)
            {
                temp->image_desc.image_depth = 1;
                temp->image_desc.image_array_size = 1;
            }
            else if(image_desc->image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY)
            {
                temp->image_desc.image_depth = 1;
            }
            else if(image_desc->image_type == CL_MEM_OBJECT_IMAGE3D)
            {
                temp->image_desc.image_array_size = 1;
            }
            else
            {
                det_fprintf(stderr, "failed to find image type");
                exit(-1);
            }

            size_t dataSize = getImageDataSize(image_format);
            temp->size = temp->image_desc.image_width * temp->image_desc.image_height * temp->image_desc.image_depth * temp->image_desc.image_array_size * dataSize;
            uint64_t img_size = temp->size;
            if(global_tool_stats_flags & STATS_MEM_OVERHEAD)
            {
                pthread_mutex_lock(&memory_overhead_lock);
                if(internal_create)
                {
                    total_overhead_mem += img_size;
                    current_overhead_mem += img_size;
                }
                else
                {
                    total_user_mem += img_size;
                    current_user_mem += img_size;
                }
                pthread_mutex_unlock(&memory_overhead_lock);
            }

            char *fill_ptr = NULL;

            if(flags & CL_MEM_USE_HOST_PTR || image_desc->buffer)
            {
                fill_ptr = host_ptr;
            }
            else
            {
                expandLastDimForFill(&temp->image_desc);
                temp->size = temp->image_desc.image_width * temp->image_desc.image_height * temp->image_desc.image_depth * temp->image_desc.image_array_size * dataSize;
                if(global_tool_stats_flags & STATS_MEM_OVERHEAD)
                {
                    pthread_mutex_lock(&memory_overhead_lock);
                    total_overhead_mem += temp->size - img_size;
                    current_overhead_mem += temp->size - img_size;
                    pthread_mutex_unlock(&memory_overhead_lock);
                }

                allocFlatImageCopy(&fill_ptr, host_ptr, temp);

                temp->flags = flags | CL_MEM_COPY_HOST_PTR;
                // reset the pitches because they no longer exist after
                // the flat image copy
                temp->image_desc.image_row_pitch = 0;
                temp->image_desc.image_slice_pitch = 0;
                temp->has_canary = 1;
            }

            if(global_tool_stats_flags & STATS_MEM_OVERHEAD)
            {
                pthread_mutex_lock(&memory_overhead_lock);
                high_user_mem = (high_user_mem > current_user_mem) ? high_user_mem : current_user_mem;
                high_overhead_mem = (high_overhead_mem > current_overhead_mem) ? high_overhead_mem : current_overhead_mem;
                pthread_mutex_unlock(&memory_overhead_lock);
            }

            if (temp->flags & CL_MEM_COPY_HOST_PTR)
            {
                // Bug workaround.
                // On the AMD ROCm software stack, CL_MEM_COPY_HOST_PTR causes
                // memory corruption on the CPU side. As such, we replace the
                // implicit copy with an explicit copy to make things work.
                cl_int internal_err = CL_SUCCESS;
                temp->flags &= ~CL_MEM_COPY_HOST_PTR;
                ret = CreateImage(context, temp->flags, image_format,
                        &temp->image_desc, NULL, errcode_ret);

                if (errcode_ret != NULL)
                    *errcode_ret = internal_err;
                temp->handle = ret;

                if(internal_err != CL_IMAGE_FORMAT_NOT_SUPPORTED &&
                        internal_err != CL_INVALID_IMAGE_FORMAT_DESCRIPTOR)
                {
                    const size_t zero_origin[3] = {0,0,0};
                    const size_t region[3] = {temp->image_desc.image_width,
                        temp->image_desc.image_height,
                        temp->image_desc.image_depth};

                    cl_command_queue command_queue;
                    getCommandQueueForContext(context, &command_queue);
                    internal_err = EnqueueWriteImage(command_queue, ret, CL_BLOCKING,
                            zero_origin, region, 0, 0, fill_ptr, 0, NULL, NULL);
                    check_cl_error(__FILE__, __LINE__, internal_err);
                }
            }
            else
            {
                ret = CreateImage(context, temp->flags, image_format,
                        &temp->image_desc, fill_ptr, errcode_ret);
            }

            if(fill_ptr && fill_ptr != host_ptr)
                free(fill_ptr);

            temp->host_ptr = NULL;
            if(flags & CL_MEM_USE_HOST_PTR)
                temp->host_ptr = host_ptr;

            temp->ref_count = 1;
            temp->detector_internal_buffer = 0; // will set this outside if need be.
            cl_mem_insert(get_cl_mem_alloc(), temp);
        }
        else
        {
            return CreateImage(context, flags, image_format, image_desc,
                    host_ptr, errcode_ret);
        }
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }
    return(ret);
}
#endif // CL_VERSION_1_2

    CL_API_ENTRY cl_mem CL_API_CALL
clCreateImage2D(cl_context              context ,
        cl_mem_flags            flags ,
        const cl_image_format * image_format ,
        size_t image_width ,
        size_t image_height ,
        size_t image_row_pitch ,
        void *                  host_ptr ,
        cl_int *                errcode_ret )
{
    cl_mem ret = 0;
    if ( CreateImage2D )
    {
        if (!opencl_broken_images())
        {
            cl_memobj *temp = (cl_memobj*)calloc(sizeof(cl_memobj), 1);
            temp->is_image = 1;
            temp->context = context;
            temp->flags = flags;

            temp->image_format.image_channel_order = image_format->image_channel_order;
            temp->image_format.image_channel_data_type = image_format->image_channel_data_type;

            temp->image_desc.image_type = CL_MEM_OBJECT_IMAGE2D;
            temp->image_desc.image_width = image_width;
            temp->image_desc.image_height = image_height;
            temp->image_desc.image_depth = 1;
            temp->image_desc.image_array_size = 1;
            temp->image_desc.image_row_pitch = image_row_pitch;
            temp->image_desc.image_slice_pitch = 0;
            temp->image_desc.num_mip_levels = 0;
            temp->image_desc.num_samples = 0;
            temp->image_desc.buffer = NULL;

            size_t dataSize = getImageDataSize(image_format);
            temp->size = image_width * image_height * dataSize;
            uint64_t img_size = temp->size;
            if(global_tool_stats_flags & STATS_MEM_OVERHEAD)
            {
                pthread_mutex_lock(&memory_overhead_lock);
                if(internal_create)
                {
                    total_overhead_mem += img_size;
                    current_overhead_mem += img_size;
                }
                else
                {
                    total_user_mem += img_size;
                    current_user_mem += img_size;
                }
                pthread_mutex_unlock(&memory_overhead_lock);
            }

            char *fill_ptr = NULL;

            if(flags & CL_MEM_USE_HOST_PTR)
            {
                fill_ptr = host_ptr;
            }
            else
            {
                expandLastDimForFill(&temp->image_desc);
                temp->size = temp->image_desc.image_width * temp->image_desc.image_height * temp->image_desc.image_depth * temp->image_desc.image_array_size * dataSize;
                if(global_tool_stats_flags & STATS_MEM_OVERHEAD)
                {
                    pthread_mutex_lock(&memory_overhead_lock);
                    total_overhead_mem += temp->size - img_size;
                    current_overhead_mem += temp->size - img_size;
                    pthread_mutex_unlock(&memory_overhead_lock);
                }

                allocFlatImageCopy(&fill_ptr, host_ptr, temp);

                temp->flags = flags | CL_MEM_COPY_HOST_PTR;
                temp->has_canary = 1;
            }

            if(global_tool_stats_flags & STATS_MEM_OVERHEAD)
            {
                pthread_mutex_lock(&memory_overhead_lock);
                high_user_mem = (high_user_mem > current_user_mem) ? high_user_mem : current_user_mem;
                high_overhead_mem = (high_overhead_mem > current_overhead_mem) ? high_overhead_mem : current_overhead_mem;
                pthread_mutex_unlock(&memory_overhead_lock);
            }

            if (temp->flags & CL_MEM_COPY_HOST_PTR)
            {
                // Bug workaround.
                // On the AMD ROCm software stack, CL_MEM_COPY_HOST_PTR causes
                // memory corruption on the CPU side. As such, we replace the
                // implicit copy with an explicit copy to make things work.
                cl_int internal_err = CL_SUCCESS;
                temp->flags &= ~CL_MEM_COPY_HOST_PTR;
                ret = CreateImage2D(context, temp->flags, image_format,
                        temp->image_desc.image_width,
                        temp->image_desc.image_height,
                        0, NULL, &internal_err);

                if (errcode_ret != NULL)
                    *errcode_ret = internal_err;
                temp->handle = ret;

                if(internal_err != CL_IMAGE_FORMAT_NOT_SUPPORTED &&
                        internal_err != CL_INVALID_IMAGE_FORMAT_DESCRIPTOR)
                {
                    const size_t zero_origin[3] = {0,0,0};
                    const size_t region[3] = {temp->image_desc.image_width,
                        temp->image_desc.image_height, 1};

                    cl_command_queue command_queue;
                    getCommandQueueForContext(context, &command_queue);
                    internal_err = EnqueueWriteImage(command_queue, ret,
                            CL_BLOCKING, zero_origin, region, 0, 0, fill_ptr, 0,
                            NULL, NULL);
                    check_cl_error(__FILE__, __LINE__, internal_err);
                }
            }
            else
            {
                ret = CreateImage2D(context, temp->flags, image_format,
                        temp->image_desc.image_width,
                        temp->image_desc.image_height,
                        0, fill_ptr, errcode_ret);
            }

            if(fill_ptr && fill_ptr != host_ptr)
                free(fill_ptr);

            temp->host_ptr = NULL;
            if(flags & CL_MEM_USE_HOST_PTR)
                temp->host_ptr = host_ptr;

            temp->ref_count = 1;
            temp->detector_internal_buffer = 0; // will set this outside if need be.
            cl_mem_insert(get_cl_mem_alloc(), temp);
        }
        else
        {
            return CreateImage2D(context, flags, image_format, image_width,
                    image_height, image_row_pitch, host_ptr, errcode_ret);
        }
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }
    return(ret);
}

    CL_API_ENTRY cl_mem CL_API_CALL
clCreateImage3D(cl_context              context ,
        cl_mem_flags            flags ,
        const cl_image_format * image_format ,
        size_t image_width ,
        size_t image_height ,
        size_t image_depth ,
        size_t image_row_pitch ,
        size_t image_slice_pitch ,
        void *                  host_ptr ,
        cl_int *                errcode_ret )
{
    cl_mem ret = 0;
    if ( CreateImage3D )
    {
        if (!opencl_broken_images())
        {
            cl_memobj *temp = (cl_memobj*)calloc(sizeof(cl_memobj), 1);
            temp->is_image = 1;
            temp->context = context;
            temp->flags = flags;

            temp->image_format.image_channel_order = image_format->image_channel_order;
            temp->image_format.image_channel_data_type = image_format->image_channel_data_type;

            temp->image_desc.image_type = CL_MEM_OBJECT_IMAGE3D;
            temp->image_desc.image_width = image_width;
            temp->image_desc.image_height = image_height;
            temp->image_desc.image_depth = image_depth;
            temp->image_desc.image_array_size = 1;
            temp->image_desc.image_row_pitch = image_row_pitch;
            temp->image_desc.image_slice_pitch = image_slice_pitch;
            temp->image_desc.num_mip_levels = 0;
            temp->image_desc.num_samples = 0;
            temp->image_desc.buffer = NULL;

            size_t dataSize = getImageDataSize(image_format);
            temp->size = image_width * image_height * image_depth * dataSize;
            uint64_t img_size = temp->size;
            if(global_tool_stats_flags & STATS_MEM_OVERHEAD)
            {
                pthread_mutex_lock(&memory_overhead_lock);
                if(internal_create)
                {
                    total_overhead_mem += img_size;
                    current_overhead_mem += img_size;
                }
                else
                {
                    total_user_mem += img_size;
                    current_user_mem += img_size;
                }
                pthread_mutex_unlock(&memory_overhead_lock);
            }

            char *fill_ptr = NULL;

            if(flags & CL_MEM_USE_HOST_PTR)
            {
                fill_ptr = host_ptr;
            }
            else
            {
                expandLastDimForFill(&temp->image_desc);
                temp->size = temp->image_desc.image_width * temp->image_desc.image_height * temp->image_desc.image_depth * temp->image_desc.image_array_size * dataSize;
                if(global_tool_stats_flags & STATS_MEM_OVERHEAD)
                {
                    pthread_mutex_lock(&memory_overhead_lock);
                    total_overhead_mem += temp->size - img_size;
                    current_overhead_mem += temp->size - img_size;
                    pthread_mutex_unlock(&memory_overhead_lock);
                }

                allocFlatImageCopy(&fill_ptr, host_ptr, temp);

                temp->flags = flags | CL_MEM_COPY_HOST_PTR;
                temp->has_canary = 1;
            }

            if(global_tool_stats_flags & STATS_MEM_OVERHEAD)
            {
                pthread_mutex_lock(&memory_overhead_lock);
                high_user_mem = (high_user_mem > current_user_mem) ? high_user_mem : current_user_mem;
                high_overhead_mem = (high_overhead_mem > current_overhead_mem) ? high_overhead_mem : current_overhead_mem;
                pthread_mutex_unlock(&memory_overhead_lock);
            }

            if (temp->flags & CL_MEM_COPY_HOST_PTR)
            {
                // Bug workaround.
                // On the AMD ROCm software stack, CL_MEM_COPY_HOST_PTR causes
                // memory corruption on the CPU side. As such, we replace the
                // implicit copy with an explicit copy to make things work.
                cl_int internal_err = CL_SUCCESS;
                temp->flags &= ~CL_MEM_COPY_HOST_PTR;
                ret = CreateImage3D(context, temp->flags, image_format,
                        temp->image_desc.image_width,
                        temp->image_desc.image_height,
                        temp->image_desc.image_depth,
                        0, 0, NULL, &internal_err);

                if (errcode_ret != NULL)
                    *errcode_ret = internal_err;
                temp->handle = ret;

                if(internal_err != CL_IMAGE_FORMAT_NOT_SUPPORTED &&
                        internal_err != CL_INVALID_IMAGE_FORMAT_DESCRIPTOR)
                {
                    const size_t zero_origin[3] = {0,0,0};
                    const size_t region[3] = {temp->image_desc.image_width,
                        temp->image_desc.image_height, temp->image_desc.image_depth};

                    cl_command_queue command_queue;
                    getCommandQueueForContext(context, &command_queue);
                    internal_err = EnqueueWriteImage(command_queue, ret, CL_BLOCKING,
                            zero_origin, region, 0, 0, fill_ptr, 0, NULL, NULL);
                    check_cl_error(__FILE__, __LINE__, internal_err);
                }
            }
            else
            {
                ret = CreateImage3D(context, temp->flags, image_format,
                        temp->image_desc.image_width,
                        temp->image_desc.image_height,
                        temp->image_desc.image_depth,
                        0, 0, fill_ptr, errcode_ret);
            }

            if(fill_ptr && fill_ptr != host_ptr)
                free(fill_ptr);

            temp->host_ptr = NULL;
            if(flags & CL_MEM_USE_HOST_PTR)
                temp->host_ptr = host_ptr;

            temp->ref_count = 1;
            temp->detector_internal_buffer = 0; // will set this outside if need be.
            cl_mem_insert(get_cl_mem_alloc(), temp);
        }
        else
        {
            return CreateImage3D(context, flags, image_format, image_width,
                    image_height, image_depth, image_row_pitch,
                    image_slice_pitch, host_ptr, errcode_ret);
        }
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }
    return(ret);
}


CL_API_ENTRY cl_int CL_API_CALL
clRetainMemObject(cl_mem memobj )
{
    cl_int err = INT_MIN;
    if ( RetainMemObject )
    {
        initialize_logging();
        cl_memobj *findme = cl_mem_find(get_cl_mem_alloc(), memobj);
        if (findme != NULL)
            findme->ref_count++;
        err = RetainMemObject( memobj );
        if(findme && findme->main_buff)
            err = RetainMemObject( findme->main_buff );
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }
    return(err);
}

void releaseInternalMemObject(cl_mem memobj)
{
    uint8_t backup = internal_create;
    internal_create = 1;
    clReleaseMemObject(memobj);
    internal_create = backup;
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseMemObject(cl_mem memobj )
{
    cl_int err = INT_MIN;
    if ( ReleaseMemObject )
    {
        initialize_logging();
        cl_memobj *findme = cl_mem_find(get_cl_mem_alloc(), memobj);
        cl_mem main_buff = 0;
        if (findme != NULL)
        {
            if(findme->main_buff)
                main_buff = findme->main_buff;

            findme->ref_count--;
            if (findme->ref_count == 0)
            {
                if(global_tool_stats_flags & STATS_MEM_OVERHEAD)
                {
                    pthread_mutex_lock(&memory_overhead_lock);
                    if(internal_create)
                    {
                        current_overhead_mem -= findme->size;
                        if(!findme->is_image)
                            current_overhead_mem -= POISON_REGIONS*POISON_FILL_LENGTH;
                    }
                    else if( !findme->has_canary )
                        current_user_mem -= findme->size;
                    else if(findme->is_image)
                    {
                        uint64_t w, h, d, a;
                        size_t dataSize;
                        uint64_t img_size;
                        w = findme->image_desc.image_width;
                        h = findme->image_desc.image_height;
                        d = findme->image_desc.image_depth;
                        a = findme->image_desc.image_array_size;

                        if(a > 1)
                        {
                            if(h > 1)
                                a -= IMAGE_POISON_DEPTH;
                            else
                                a -= IMAGE_POISON_HEIGHT;
                        }
                        w = (w > 1) ? w - IMAGE_POISON_WIDTH : w;
                        h = (h > 1) ? h - IMAGE_POISON_HEIGHT : h;
                        d = (d > 1) ? d - IMAGE_POISON_DEPTH : d;

                        dataSize = getImageDataSize(&findme->image_format);
                        img_size = w * h * d * a * dataSize;

                        current_user_mem -= img_size;
                        current_overhead_mem -= (findme->size - img_size);
                    }
                    else
                    {
                        current_user_mem -= findme->size;
                        current_overhead_mem -= POISON_REGIONS*POISON_FILL_LENGTH;
                    }
                    pthread_mutex_unlock(&memory_overhead_lock);
                }

                cl_memobj *temp;
                temp = cl_mem_remove(get_cl_mem_alloc(), memobj);
                if(temp != NULL)
                    cl_mem_delete(temp);
            }
        }

        err = ReleaseMemObject( memobj );
        if(main_buff)
            err = ReleaseMemObject( main_buff );
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }
    return(err);
}

CL_API_ENTRY void* CL_API_CALL
clEnqueueMapBuffer(cl_command_queue command_queue,
            cl_mem buffer,
            cl_bool blocking_map,
            cl_map_flags map_flags,
            size_t offset,
            size_t size,
            cl_uint num_events_in_wait_list,
            const cl_event *event_wait_list,
            cl_event *event,
            cl_int *errcode_ret)
{
    void *ret = NULL;
    if(EnqueueMapBuffer)
    {
        cl_mem main_buffer = buffer;
        size_t offset_aug = offset;
        cl_memobj *m1 = cl_mem_find(get_cl_mem_alloc(), buffer);
        if(m1 && m1->main_buff)
        {
            offset_aug += POISON_FILL_LENGTH;
            main_buffer = m1->main_buff;
        }

        ret = EnqueueMapBuffer(command_queue, main_buffer, blocking_map, map_flags, offset_aug, size, num_events_in_wait_list, event_wait_list, event, errcode_ret);
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }

    return ret;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueUnmapMemObject(cl_command_queue command_queue,
            cl_mem memobj,
            void *mapped_ptr,
            cl_uint num_events_in_wait_list,
            const cl_event *event_wait_list,
            cl_event *event)
{
    cl_int err = INT_MIN;
    if(EnqueueUnmapMemObject)
    {
        cl_mem main_buffer = memobj;
        cl_memobj *m1 = cl_mem_find(get_cl_mem_alloc(), memobj);
        if(m1 && m1->main_buff)
        {
            main_buffer = m1->main_buff;
        }

        err = EnqueueUnmapMemObject(command_queue, main_buffer, mapped_ptr, num_events_in_wait_list, event_wait_list, event);
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }

    return err;
}


CL_API_ENTRY cl_int CL_API_CALL
clRetainKernel(cl_kernel     kernel )
{
    cl_int err = INT_MIN;
    if ( RetainKernel )
    {
        initialize_logging();
        kernel_info *temp;
        temp = kinfo_find(get_kern_list(), kernel);
        if (temp != NULL)
            temp->ref_count++;
        err = RetainKernel(kernel);
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }
    return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseKernel(cl_kernel    kernel )
{
    cl_int err = INT_MIN;
    if (ReleaseKernel)
    {
        initialize_logging();
        kernel_info *temp;
        temp = kinfo_find(get_kern_list(), kernel);
        if (temp != NULL)
        {
            temp->ref_count--;
            if (temp->ref_count == 0)
            {
                temp = kinfo_remove(get_kern_list(), kernel);
                kinfo_delete(temp);
            }
        }
        if (kernel)
        {
            // Purposefully leak kernels in detector. We currently do this because
            // the AMD APP SDK OpenCL runtime has a memory corruption error that
            // happens somewhere inside clReleaseKernel(). We trigger this somewhat
            // frequently. Skipping the real ReleaseKernel avoids this issue.
            err = CL_SUCCESS;
        }
        else
            err = CL_INVALID_KERNEL;
        //err = ReleaseKernel(kernel);
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }
    return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clSetKernelArg(cl_kernel     kernel ,
        cl_uint       arg_index ,
        size_t        arg_size ,
        const void *  arg_value )
{
    cl_int err = INT_MIN;
    if (SetKernelArg)
    {
        initialize_logging();
        err = SetKernelArg(kernel, arg_index, arg_size, arg_value);

        if(err == CL_SUCCESS)
        {
            kernel_info *kern_temp;
            kernel_arg *arg_temp;
            kern_temp = kinfo_find(get_kern_list(), kernel);

            if(kern_temp == NULL)
            {
                kern_temp = (kernel_info*)calloc(sizeof(kernel_info), 1);
                kern_temp->handle = kernel;
                kern_temp->arg_list = NULL;
                kern_temp->ref_count = 1;
                kinfo_insert(get_kern_list(), kern_temp);
            }

            arg_temp = karg_remove(&(kern_temp->arg_list), arg_index);
            if(arg_temp != NULL)
            {
                if (arg_temp->value != NULL)
                    free(arg_temp->value);
                arg_temp->value = NULL;
                //free(arg_temp);
                //arg_temp = NULL;
            }
            else
            {
                arg_temp = calloc(sizeof(kernel_arg), 1);
                arg_temp->handle = arg_index;
            }

            arg_temp->size = arg_size;
            if(arg_value != NULL)
            {
                arg_temp->value = calloc(arg_size, 1);
                memcpy(arg_temp->value, arg_value, arg_size);
            }
            else
            {
                arg_temp->value = NULL;
            }
            arg_temp->buffer = NULL;
            arg_temp->svm_buffer = NULL;
            if(arg_size == sizeof(cl_mem) && arg_value != NULL)
            {
                cl_memobj *m1 = cl_mem_find(get_cl_mem_alloc(), *(cl_mem*)arg_value);
                arg_temp->buffer = m1; //m1 is 0 if not present
            }

            karg_insert(&(kern_temp->arg_list), arg_temp);
        }
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }
    return(err);
}

#ifdef CL_VERSION_2_0
CL_API_ENTRY cl_int CL_API_CALL
clSetKernelArgSVMPointer(
        cl_kernel     kernel,
        cl_uint       arg_index,
        const void *  arg_value)
{
    cl_int err = INT_MIN;
    if (SetKernelArgSVMPointer)
    {
        initialize_logging();
        if (arg_value == NULL)
        {
            // Apparently, the AMD OpenCL 2.0 implementation does not properly
            // check to see if arg_value is a "valid value", per the spec. As
            // such, it silently accepts NULL values here and then fails to
            // properly run the kernel later. This action is a fixup that
            // will stop this problem from happening for applications that
            // correctly check the return value of clSetKernelArgSVMPointer().
            return CL_INVALID_ARG_VALUE;
        }

        // Start by calling the real set kernel parameter. This will tell us
        // whether the set parameter failed or not. If it failed, we do not
        // need to insert anything into the knernel arguments list.
        err = SetKernelArgSVMPointer(kernel, arg_index, arg_value);

        if (err == CL_SUCCESS)
        {
            // If the real set kernel argument succeeded, we must now keep
            // track of this argument's information in a list of kernel args.
            // This is later used to detect duplicate kernel arguments.
            kernel_info *kern_temp;
            kernel_arg *arg_temp;

            kern_temp = kinfo_find(get_kern_list(), kernel);
            if(kern_temp == NULL)
            {
                // This is the first time we've called a function on this
                // kernel. Create the structure that will hold its information.
                kern_temp = (kernel_info*)calloc(sizeof(kernel_info), 1);
                if (kern_temp != NULL)
                {
                    kern_temp->handle = kernel;
                    kern_temp->arg_list = NULL;
                    kern_temp->ref_count = 1;
                    kinfo_insert(get_kern_list(), kern_temp);
                }
                else
                {
                    char * err_text = strerror(errno);
                    det_fprintf(stderr, "Failed to allocate kern_temp near %s:%d\n",
                        __FILE__, __LINE__);
                    det_fprintf(stderr, "Error reason: %s\n", err_text);
                    exit(-1);
                }
            }

            // If we have previously set the kernel argument for this kernel at
            // this index, we are now over-writing it. Remove the old value.
            // If no old value existed, this is the first time we are setting
            // the arg at this index for this kernel. Create it.
            arg_temp = karg_remove(&(kern_temp->arg_list), arg_index);
            if(arg_temp != NULL)
            {
                if (arg_temp->value != NULL)
                    free(arg_temp->value);
                arg_temp->value = NULL;
                free(arg_temp);
                arg_temp = NULL;
            }
            arg_temp = calloc(sizeof(kernel_arg), 1);
            if (arg_temp == NULL)
            {
                char * err_text = strerror(errno);
                det_fprintf(stderr, "Failed to allocate arg_temp near %s:%d\n",
                        __FILE__, __LINE__);
                det_fprintf(stderr, "Error reason: %s\n", err_text);
                exit(-1);
            }
            arg_temp->handle = arg_index;
            // Doing this weird memory copy because arg_value comes in as a const.
            // We want to put it into a non-const value.
            // As such, we will just copy the pointer by value using memcpy.
            void **value_ptr = calloc(sizeof(void*), 1);
            memcpy(value_ptr, &arg_value, sizeof(void*));
            arg_temp->value = (void*)value_ptr;

            // The regular non-SVM buffer should always be NULL, since we are
            // setting this kernel argument with the SetSVMPointer function.
            arg_temp->buffer = NULL;
            cl_svm_memobj *m1 = cl_svm_mem_find(get_cl_svm_mem_alloc(), arg_value);
            if (m1 == NULL && !canaryAccessAllowed())
            {
                det_fprintf(stderr, "DETECTOR WARNING: Passing in a pointer to "
                        "clSetKernelArgSVMPointer() that was not allocated "
                        "with clSVMAlloc().\n");
            }
            arg_temp->svm_buffer = m1;
            karg_insert(&(kern_temp->arg_list), arg_temp);
        }
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }
    return(err);
}
#endif

#ifdef CL_VERSION_2_0
void *internalSVMAlloc(cl_context     context,
        cl_svm_mem_flags    flags,
        size_t              size,
        unsigned int        alignment)
{
    void *ret = NULL;

    // This tool does not currently support SVM atomics.
    // Our tool runs the kernel independently of any CPU work (so that we
    // can check the buffers after it runs).
    // With CL_MEM_SVM_ATOMICS, the kernel could require the CPU to do work
    // in parallel to the GPU kernel (e.g. feeding it data). We can't do
    // this right now.
    if (flags & CL_MEM_SVM_ATOMICS)
        return NULL;

    // Workaround for the fact that fine-grained buffers are not properly
    // freed in Linux. If we keep allocating them, we will quickly run
    // out of resources. As such, we aggressively buffer them and find any
    // of the proper size with the right flags and context.
    if (flags & CL_MEM_SVM_FINE_GRAIN_BUFFER)
    {
        // Always set read-write so that all FG SVM buffers can be
        // used by everyone who wants one.
        flags = CL_MEM_READ_WRITE | CL_MEM_SVM_FINE_GRAIN_BUFFER;
        ret = find_free_svm(context, size, alignment);
    }

    if (ret == NULL)
    {
        ret = SVMAlloc(context, flags, size, alignment);
        if (ret != NULL && (flags & CL_MEM_SVM_FINE_GRAIN_BUFFER))
            cl_svm_fg_alloc(context, ret, flags, size, alignment);
    }

    return ret;
}

/* Shared Virtual Memory Object APIs */
CL_API_ENTRY void* CL_API_CALL
clSVMAlloc(cl_context           context,
            cl_svm_mem_flags    flags,
            size_t              size,
            unsigned int        alignment)
{
    void *ret = NULL;
    if ( SVMAlloc )
    {
        size_t size_aug = size;

        initialize_logging();

        if(global_tool_stats_flags & STATS_MEM_OVERHEAD)
        {
            pthread_mutex_lock(&memory_overhead_lock);
            if(internal_create)
            {
                total_overhead_mem += size + POISON_REGIONS*POISON_FILL_LENGTH;
                current_overhead_mem += size + POISON_REGIONS*POISON_FILL_LENGTH;
            }
            else
            {
                total_user_mem += size;
                current_user_mem += size;
                total_overhead_mem += POISON_REGIONS*POISON_FILL_LENGTH;
                current_overhead_mem += POISON_REGIONS*POISON_FILL_LENGTH;
            }

            high_user_mem = (high_user_mem > current_user_mem) ? high_user_mem : current_user_mem;
            high_overhead_mem = (high_overhead_mem > current_overhead_mem) ? high_overhead_mem : current_overhead_mem;
            pthread_mutex_unlock(&memory_overhead_lock);
        }
        size_aug += POISON_REGIONS*POISON_FILL_LENGTH;

        ret = internalSVMAlloc(context, flags, size_aug, alignment);

        if (ret == NULL)
            return NULL;

        void * user_ptr;
        user_ptr = ret;
#ifdef UNDERFLOW_CHECK
        user_ptr = (char*)user_ptr + POISON_FILL_LENGTH;
#endif

        cl_int cl_err;
        cl_command_queue cmdQueue;
        int weCreated = !getCommandQueueForContext(context, &cmdQueue);
        if(!weCreated)
            clRetainCommandQueue(cmdQueue);

        uint32_t offset = size;
        cl_event finish[2];
#ifdef UNDERFLOW_CHECK
        cl_err = clEnqueueSVMMemFill(cmdQueue, (char*)ret, &poisonFill_8b, sizeof(uint8_t), POISON_FILL_LENGTH, 0, 0, &finish[1]);
        check_cl_error(__FILE__, __LINE__, cl_err);

        offset += POISON_FILL_LENGTH;
#endif
        cl_err = clEnqueueSVMMemFill(cmdQueue, (char*)ret + offset, &poisonFill_8b, sizeof(uint8_t), POISON_FILL_LENGTH, 0, 0, &finish[0]);
        check_cl_error(__FILE__, __LINE__, cl_err);

        cl_svm_memobj *temp = (cl_svm_memobj*)calloc(sizeof(cl_svm_memobj), 1);
        temp->handle = user_ptr;
        temp->main_buff = ret;
        temp->context = context;
        temp->flags = flags;
        temp->size = size;
        temp->alignment = alignment;
        temp->detector_internal_buffer = 0; // will set this outside if need be.
        cl_svm_mem_insert(get_cl_svm_mem_alloc(), temp);

        clWaitForEvents(POISON_REGIONS, finish);

        clReleaseCommandQueue(cmdQueue);

        ret = user_ptr;
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }
    return(ret);
}

CL_API_ENTRY void CL_API_CALL
clSVMFree(cl_context    context,
           void *       svm_pointer)
{
    if (SVMFree)
    {
        initialize_logging();
        cl_svm_memobj *temp;
        temp = cl_svm_mem_remove(get_cl_svm_mem_alloc(), svm_pointer);
        void *main_svm = 0;

        if(global_tool_stats_flags & STATS_MEM_OVERHEAD)
        {
            pthread_mutex_lock(&memory_overhead_lock);
            if(temp != NULL)
            {
                main_svm = temp->main_buff;
                if(internal_create)
                    current_overhead_mem -= temp->size + POISON_REGIONS*POISON_FILL_LENGTH;
                else
                {
                    current_user_mem -= temp->size;
                    current_overhead_mem -= POISON_REGIONS*POISON_FILL_LENGTH;
                }
            }
            pthread_mutex_unlock(&memory_overhead_lock);
        }

        if(temp != NULL)
            cl_svm_mem_delete(temp);

        /*
         * If this was a previously-allocated fine-grained buffer, don't free
         * it. The OpenCL runtime does not properly free these buffers,
         * and further allocations will eventually fill up our limited SVM
         * space, causing out-of-resource errors. Instead, we cache them and
         * try to reuse them for the next-requested alloc. Don't call
         * real SVMFree on this, just in case the runtime eventually decides
         * to do the free and we end up with use-after-free errors.
         * Coarse-grained buffers call SVMFree like normal.
         */
        if (main_svm && cl_svm_fg_free(context, main_svm))
            SVMFree(context, main_svm);
        if (cl_svm_fg_free(context, svm_pointer))
            SVMFree(context, svm_pointer);
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }
    return;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueSVMFree(cl_command_queue command_queue,
        cl_uint         num_svm_pointers,
        void *          svm_pointers[],
        void (CL_CALLBACK  *pfn_free_func)
            (cl_command_queue queue,
             cl_uint num_svm_pointers,
             void *svm_pointers[],
             void* user_data),
        void *          user_data,
        cl_uint         num_events,
        const cl_event *event_list,
        cl_event *      event)
{
    cl_int err = INT_MIN;
    if(EnqueueSVMFree)
    {
        initialize_logging();
        // We want to finish what's in the command queue so that we don't
        // try to remove detector-internal stuff that is then later used
        // by a kernel or transfer that is queued but not yet running.
        clFinish(command_queue);
        // Delete from the detector-internal lists before we actually call the
        // real SVM free function to prevent any weird use-after-free stuff.
        cl_context context;
        cl_int cl_err = clGetCommandQueueInfo(command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);
        check_cl_error(__FILE__, __LINE__, cl_err);
        for (cl_uint i = 0; i < num_svm_pointers; i++)
        {
            cl_svm_memobj *temp = cl_svm_mem_remove(get_cl_svm_mem_alloc(), svm_pointers[i]);
            cl_svm_mem_delete(temp);
            cl_svm_fg_free(context, svm_pointers[i]);
        }

        err = EnqueueSVMFree(command_queue, num_svm_pointers, svm_pointers,
                    pfn_free_func, user_data, num_events, event_list, event);
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }
    return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueSVMMap(cl_command_queue command_queue,
            cl_bool blocking_map,
            cl_map_flags map_flags,
            void *svm_ptr,
            size_t size,
            cl_uint num_events_in_wait_list,
            const cl_event *event_wait_list,
            cl_event *event)
{
    cl_int err = INT_MIN;
    if(EnqueueSVMMap)
    {
        void *main_svm = svm_ptr;
        size_t size_aug = size;

        cl_svm_memobj *m1;
        m1 = cl_svm_mem_find(get_cl_svm_mem_alloc(), svm_ptr);
        if(m1)
        {
            main_svm = m1->main_buff;
            size_aug += POISON_FILL_LENGTH;
        }

        err = EnqueueSVMMap(command_queue, blocking_map, map_flags, main_svm, size_aug, num_events_in_wait_list, event_wait_list, event);
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }
    return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueSVMUnmap(cl_command_queue command_queue,
            void *svm_ptr,
            cl_uint num_events_in_wait_list,
            const cl_event *event_wait_list,
            cl_event *event)
{
    cl_int err = INT_MIN;
    if(EnqueueSVMUnmap)
    {
        void *main_svm = svm_ptr;

        cl_svm_memobj *m1;
        m1 = cl_svm_mem_find(get_cl_svm_mem_alloc(), svm_ptr);
        if(m1)
        {
            main_svm = m1->main_buff;
        }

        err = EnqueueSVMUnmap(command_queue, main_svm, num_events_in_wait_list, event_wait_list, event);
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }
    return(err);
}

#endif

/****************************************************************/
cl_int runNDRangeKernel(launchOclKernelStruct *ocl_args)
{
    if (EnqueueNDRangeKernel)
    {
        return EnqueueNDRangeKernel(ocl_args->command_queue,
                ocl_args->kernel, ocl_args->work_dim,
                ocl_args->global_work_offset, ocl_args->global_work_size,
                ocl_args->local_work_size, ocl_args->num_events_in_wait_list,
                ocl_args->event_wait_list, ocl_args->event);
    }
    else
    {
        CL_MSG("Real EnqueueNDRangeKernel not found");
    }
    return 0;
}

/********** call from interseptor  *****************************/
static cl_int kernelLaunchFunc(void * thread_args_)
{
    struct timeval total_stop, total_start, enq_stop, enq_start;
    if(global_tool_stats_flags & STATS_KERN_ENQ_TIME)
    {
        gettimeofday(&total_start, NULL);
    }

    cl_int cl_err;

    //----------------------------------SETUP-----------------------------------
    launchOclKernelStruct *ocl_args = (launchOclKernelStruct *)thread_args_;

    cl_event internal_event;
    cl_event *external_event = ocl_args->event;
    ocl_args->event = &internal_event;

    //----------------------------------END-SETUP-------------------------------

    //----------------------------------COPY-ARGS-------------------------------
    uint32_t *dupe = NULL;
    cl_kernel org_kernel = 0;
    cl_kernel temp_kernel = 0;

    uint32_t nargs;
    kernel_info *kinfo;

    //getting the information grabbed about this kernel
    kinfo = kinfo_find(get_kern_list(), ocl_args->kernel);
    cl_err = clGetKernelInfo(ocl_args->kernel, CL_KERNEL_NUM_ARGS, sizeof(nargs), &nargs, 0);
    check_cl_error(__FILE__, __LINE__, cl_err);

    org_kernel = ocl_args->kernel;

    cl_context ctx;
    clGetCommandQueueInfo(ocl_args->command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &ctx, 0);

    findDuplicates(nargs, kinfo->arg_list, &dupe); //duplicate arguments list

    temp_kernel = createPoisonedKernel(ocl_args->command_queue, ocl_args->kernel, dupe);

    //--------------------------------END-COPY-ARGS-----------------------------

    //------------------------------------RUN-----------------------------------
    ocl_args->kernel = temp_kernel;

    if(global_tool_stats_flags & STATS_KERN_ENQ_TIME)
    {
        gettimeofday(&enq_start, NULL);
    }
    cl_err = runNDRangeKernel( ocl_args );
    if(global_tool_stats_flags & STATS_KERN_ENQ_TIME)
    {
        gettimeofday(&enq_stop, NULL);
    }
    check_cl_error(__FILE__, __LINE__, cl_err);

    // internal_event is used as the output event for runNDRangeKernel. Because
    // we want the verifyBufferInBounds work to run after the kernel is
    // finished, we use that as an input event for the verify function.
    // If the user passed in an output event to this API call.
    //
    // However, this replacement means later calls to clGetEventProfilingInfo
    // may fail (since it would try to profile our internal detector kernels).
    // As such, we have a list that maps our "internal" events to the real
    // user events.
    internal_create = 1;
    allowCanaryAccess();

    verifyBufferInBounds(ocl_args->command_queue, ocl_args->kernel, dupe,
            &internal_event, external_event);

    disallowCanaryAccess();
    internal_create = 0;

    // Later, if the user wants to profile the output event of this enqueue,
    // we will check the list to see what real NDRangeKernelEnqueue event
    // contains the profiling informatino. In this case, internal_event holds
    // the real profiling info.
    if (external_event != NULL)
    {
        cl_evt_info *evt_info = cl_event_find(get_cl_evt_list(),
                *external_event);
        if (evt_info == NULL)
        {
            evt_info = calloc(sizeof(cl_evt_info), 1);
            evt_info->ref_count = 1;
        }
        evt_info->handle = *external_event;
        evt_info->internal_event = internal_event;
        cl_event_insert(get_cl_evt_list(), evt_info);
    }

    //------------------------------------END-RUN-------------------------------

    // -----------------Deleting Host Pointers and Cleaning Up------------------

    if(temp_kernel != org_kernel)
    {
        copyKernelBuffers(org_kernel, ocl_args->kernel, dupe, ocl_args->command_queue);

        delPoisonKernel(temp_kernel, dupe);
    }

    if (dupe != NULL)
        free(dupe);
    dupe = NULL;

    // -----------------Deleting Host Pointers and Cleaning Up----------------
    ocl_args->kernel = org_kernel;


    //------------------------------------TEARDOWN------------------------------
    ocl_args->event = external_event;
    //----------------------------------END-TEARDOWN----------------------------

    if(global_tool_stats_flags & STATS_KERN_ENQ_TIME)
    {
        gettimeofday(&total_stop, NULL);
        uint64_t total_durr_us, enq_durr_us;

        total_durr_us = timeval_diff_us(&total_stop, &total_start);
        enq_durr_us = timeval_diff_us(&enq_stop, &enq_start);

        FILE *perf_out_f = NULL;
        perf_out_f = fopen(global_tool_stats_outfile, "a");

        fprintf(perf_out_f, "%lu, %lu, %lu\n", total_durr_us, enq_durr_us, total_durr_us - enq_durr_us);
        fclose(perf_out_f);
    }

    return(cl_err);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueNDRangeKernel(cl_command_queue command_queue,
        cl_kernel         kernel,
        cl_uint           work_dim,
        const size_t *    global_work_offset,
        const size_t *    global_work_size,
        const size_t *    local_work_size,
        cl_uint           num_events,
        const cl_event *  event_list,
        cl_event *        event)
{
    cl_wrapper_init();
    cl_int err = INT_MIN;
    if (EnqueueNDRangeKernel)
    {
        initialize_logging();
        launchOclKernelStruct ocl_args;
        ocl_args.command_queue =  command_queue;
        ocl_args.kernel = kernel;
        ocl_args.work_dim = work_dim;
        ocl_args.global_work_offset = global_work_offset;
        ocl_args.global_work_size = global_work_size;
        ocl_args.local_work_size = local_work_size;
        ocl_args.num_events_in_wait_list = num_events;
        ocl_args.event_wait_list = event_list;
        ocl_args.event = event;

        //do work of swapping out buffers in a function here
        err = kernelLaunchFunc((void*)&ocl_args);
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }
    return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueTask(cl_command_queue   command_queue ,
        cl_kernel          kernel ,
        cl_uint            num_events,
        const cl_event *   event_list ,
        cl_event *         event )
{
    cl_int err = INT_MIN;
    if ( EnqueueTask )
    {
        initialize_logging();
        err = EnqueueTask(command_queue, kernel, num_events, event_list,
                    event);
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }
    return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueNativeKernel(cl_command_queue   command_queue,
        void (CL_CALLBACK * user_func)(void *),
        void *             args,
        size_t             cb_args,
        cl_uint            num_mem_objects,
        const cl_mem *     mem_list,
        const void **      args_mem_loc,
        cl_uint            num_events,
        const cl_event *   event_list,
        cl_event *         event)
{
    cl_int err = INT_MIN;
    if (EnqueueNativeKernel)
    {
        initialize_logging();
        err = EnqueueNativeKernel(command_queue,
                user_func, args, cb_args, num_mem_objects, mem_list,
                args_mem_loc, num_events, event_list, event);
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }
    return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetEventProfilingInfo(cl_event event,
        cl_profiling_info param_name, size_t param_value_size,
        void *param_value, size_t *param_value_size_ret)
{
    cl_int err = INT_MIN;
    if (GetEventProfilingInfo)
    {
        initialize_logging();
        // When an enqueued task (especially a kernel launch) are sent through
        // our detector, we may actually do multiple internal tasks.
        // These internal tasks may need ordering, so we will use OpenCL events
        // inside the tool. However, the original API call must return an event
        // that can be waited on so that all appropriate work (including our
        // buffer overflow checksare) are done. These internal tasks may
        // include the actual work expected by the user, while the event that
        // the API returns may be the last piece of work from our checker.
        // This means that when they want profiling information, they want info
        // from one of these internal events. As such, we have a map of user-
        // visible events to the "internal" event that they actually want the
        // profiling info from.
        cl_evt_info *evt_info = cl_event_find(get_cl_evt_list(), event);
        if (evt_info != NULL && evt_info->internal_event != 0)
            event = evt_info->internal_event;
        err = GetEventProfilingInfo(event, param_name, param_value_size,
                param_value, param_value_size_ret);
    }
    else
    {
        CL_MSG("Event profiling info NOT FOUND!");
    }
    return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainEvent(cl_event event)
{
    cl_int err = INT_MIN;
    if (RetainEvent)
    {
        initialize_logging();
        cl_evt_info *evt_info = cl_event_find(get_cl_evt_list(), event);
        if (evt_info == NULL)
        {
            evt_info = calloc(sizeof(cl_evt_info), 1);
            evt_info->handle = event;
            evt_info->ref_count = 1;
            evt_info->internal_event = 0;
            cl_event_insert(get_cl_evt_list(), evt_info);
        }
        evt_info->ref_count++;
        err = RetainEvent(event);
    }
    else
    {
        CL_MSG("RetainEvent NOT FOUND!");
    }
    return(err);
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseEvent(cl_event event)
{
    cl_int err = INT_MIN;
    if (ReleaseEvent)
    {
        initialize_logging();
        // If there is a detector-internal event associated with this
        // user-visible event, then we should only release it whenever the
        // user-visible event will be completely released.
        cl_evt_info *evt_info = cl_event_find(get_cl_evt_list(), event);
        if (evt_info != NULL)
        {
            evt_info->ref_count--;
            if (evt_info->ref_count == 0 && evt_info->internal_event != 0)
            {
                cl_int temp_err = ReleaseEvent(evt_info->internal_event);
                check_cl_error(__FILE__, __LINE__, temp_err);
            }
            if (evt_info->ref_count == 0)
            {
                cl_evt_info* del = cl_event_remove(get_cl_evt_list(), event);
                cl_event_delete(del);
            }
        }
        err = ReleaseEvent(event);
    }
    else
    {
        CL_MSG("ReleaseEvent NOT FOUND!");
    }
    return(err);
}

/* Enqueued Commands APIs */
    CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReadBuffer(cl_command_queue     command_queue ,
        cl_mem               buffer ,
        cl_bool              blocking_read ,
        size_t               offset ,
        size_t               size ,
        void *               ptr ,
        cl_uint              num_events ,
        const cl_event *     event_list ,
        cl_event *           event )
{
    cl_int err = CL_SUCCESS;


    if (EnqueueReadBuffer)
    {
        if( !apiBufferOverflowCheck("clEnqueueReadBuffer", buffer, offset, size) )
        {
            err =
                EnqueueReadBuffer(command_queue ,
                        buffer ,
                        blocking_read ,
                        offset ,
                        size ,
                        ptr ,
                        num_events ,
                        event_list ,
                        event );
        }
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }

    return(err);
}

    CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReadBufferRect(cl_command_queue     command_queue ,
        cl_mem               buffer ,
        cl_bool              blocking_read ,
        const size_t *       buffer_offset ,
        const size_t *       host_offset ,
        const size_t *       region ,
        size_t               buffer_row_pitch ,
        size_t               buffer_slice_pitch ,
        size_t               host_row_pitch ,
        size_t               host_slice_pitch ,
        void *               ptr ,
        cl_uint              num_events ,
        const cl_event *     event_list ,
        cl_event *           event )
{
    cl_int err = CL_SUCCESS;
    if (EnqueueReadBufferRect)
    {
        if( !apiBufferRectOverflowCheck("clEnqueueReadBufferRect", buffer, buffer_offset, region, buffer_row_pitch, buffer_slice_pitch) )
        {
            err =
                EnqueueReadBufferRect(command_queue ,
                        buffer ,
                        blocking_read ,
                        buffer_offset ,
                        host_offset ,
                        region ,
                        buffer_row_pitch ,
                        buffer_slice_pitch ,
                        host_row_pitch ,
                        host_slice_pitch ,
                        ptr ,
                        num_events ,
                        event_list ,
                        event );
        }
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }
    return(err);
}

    CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWriteBuffer(cl_command_queue    command_queue ,
        cl_mem              buffer ,
        cl_bool             blocking_write ,
        size_t              offset ,
        size_t              size ,
        const void *        ptr ,
        cl_uint             num_events ,
        const cl_event *    event_list ,
        cl_event *          event )
{
    cl_int err = CL_SUCCESS;
    if (EnqueueWriteBuffer)
    {
        if( !apiBufferOverflowCheck("clEnqueueWriteBuffer", buffer, offset, size) )
        {
            err =
                EnqueueWriteBuffer(command_queue ,
                        buffer ,
                        blocking_write ,
                        offset ,
                        size ,
                        ptr ,
                        num_events ,
                        event_list ,
                        event );
        }
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }

    return(err);
}

    CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWriteBufferRect(cl_command_queue     command_queue ,
        cl_mem               buffer ,
        cl_bool              blocking_write ,
        const size_t *       buffer_offset ,
        const size_t *       host_offset ,
        const size_t *       region ,
        size_t               buffer_row_pitch ,
        size_t               buffer_slice_pitch ,
        size_t               host_row_pitch ,
        size_t               host_slice_pitch ,
        const void *         ptr ,
        cl_uint              num_events ,
        const cl_event *     event_list ,
        cl_event *           event )
{
    cl_int err = CL_SUCCESS;
    if (EnqueueWriteBufferRect)
    {
        if( !apiBufferRectOverflowCheck("clEnqueueWriteBufferRect", buffer, buffer_offset, region, buffer_row_pitch, buffer_slice_pitch) )
        {
            err =
                EnqueueWriteBufferRect(command_queue ,
                        buffer ,
                        blocking_write ,
                        buffer_offset ,
                        host_offset ,
                        region ,
                        buffer_row_pitch ,
                        buffer_slice_pitch ,
                        host_row_pitch ,
                        host_slice_pitch ,
                        ptr ,
                        num_events ,
                        event_list ,
                        event );
        }
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }

    return(err);
}

    CL_API_ENTRY cl_int CL_API_CALL
clEnqueueFillBuffer(cl_command_queue    command_queue ,
        cl_mem              buffer ,
        const void *        pattern ,
        size_t              pattern_size ,
        size_t              offset ,
        size_t              size ,
        cl_uint             num_events ,
        const cl_event *    event_list ,
        cl_event *          event )
{
    cl_int err = CL_SUCCESS;
    if (EnqueueFillBuffer)
    {
        if( !apiBufferOverflowCheck("clEnqueueFillBuffer", buffer, offset, size) )
        {
            err =
                EnqueueFillBuffer(command_queue ,
                        buffer ,
                        pattern ,
                        pattern_size ,
                        offset ,
                        size ,
                        num_events ,
                        event_list ,
                        event );
        }
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }
    return(err);
}

    CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyBuffer(cl_command_queue     command_queue ,
        cl_mem               src_buffer ,
        cl_mem               dst_buffer ,
        size_t               src_offset ,
        size_t               dst_offset ,
        size_t               size ,
        cl_uint              num_events ,
        const cl_event *     event_list ,
        cl_event *           event )
{
    cl_int err = CL_SUCCESS;
    if (EnqueueCopyBuffer)
    {
        if( !apiBufferOverflowCheck("clEnqueueCopyBuffer", src_buffer, src_offset, size)
            && !apiBufferOverflowCheck("clEnqueueCopyBuffer", dst_buffer, dst_offset, size) )
        {
            err =
                EnqueueCopyBuffer(command_queue ,
                        src_buffer ,
                        dst_buffer ,
                        src_offset ,
                        dst_offset ,
                        size ,
                        num_events ,
                        event_list ,
                        event );
        }
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }

    return(err);
}

    CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyBufferRect(cl_command_queue     command_queue ,
        cl_mem               src_buffer ,
        cl_mem               dst_buffer ,
        const size_t *       src_origin ,
        const size_t *       dst_origin ,
        const size_t *       region ,
        size_t               src_row_pitch ,
        size_t               src_slice_pitch ,
        size_t               dst_row_pitch ,
        size_t               dst_slice_pitch ,
        cl_uint              num_events ,
        const cl_event *     event_list ,
        cl_event *           event )
{
    cl_int err = CL_SUCCESS;
    if (EnqueueCopyBufferRect)
    {
        if( !apiBufferRectOverflowCheck("clEnqueueCopyBufferRect", src_buffer, src_origin, region, src_row_pitch, src_slice_pitch)
            && !apiBufferRectOverflowCheck("clEnqueueCopyBufferRect", dst_buffer, dst_origin, region, dst_row_pitch, dst_slice_pitch) )
        {
            err =
                EnqueueCopyBufferRect(command_queue ,
                        src_buffer ,
                        dst_buffer ,
                        src_origin ,
                        dst_origin ,
                        region ,
                        src_row_pitch ,
                        src_slice_pitch ,
                        dst_row_pitch ,
                        dst_slice_pitch ,
                        num_events ,
                        event_list ,
                        event );
        }
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }
    return(err);
}

    CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReadImage(cl_command_queue      command_queue ,
        cl_mem                image ,
        cl_bool               blocking_read ,
        const size_t          origin[3] ,
        const size_t          region[3] ,
        size_t                row_pitch ,
        size_t                slice_pitch ,
        void *                ptr ,
        cl_uint               num_events ,
        const cl_event *      event_list ,
        cl_event *            event )
{
    cl_int err = CL_SUCCESS;
    if ( EnqueueReadImage )
    {
        if( !apiImageOverflowCheck("clEnqueueReadImage", image, origin, region) )
        {
            err = EnqueueReadImage(command_queue, image, blocking_read,
                    origin, region, row_pitch, slice_pitch, ptr,
                    num_events, event_list, event);
        }
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }
    return(err);
}

    CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWriteImage(cl_command_queue     command_queue ,
        cl_mem               image ,
        cl_bool              blocking_write ,
        const size_t         origin[3] ,
        const size_t         region[3] ,
        size_t               input_row_pitch ,
        size_t               input_slice_pitch ,
        const void *         ptr ,
        cl_uint              num_events ,
        const cl_event *     event_list ,
        cl_event *           event )
{
    cl_int err = CL_SUCCESS;

    if ( EnqueueWriteImage )
    {
        if( !apiImageOverflowCheck("clEnqueueWriteImage", image, origin, region) )
        {
            err =
                EnqueueWriteImage(command_queue ,
                        image ,
                        blocking_write ,
                        origin ,
                        region ,
                        input_row_pitch ,
                        input_slice_pitch ,
                        ptr ,
                        num_events ,
                        event_list ,
                        event );
        }
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }

    return(err);
}

    CL_API_ENTRY cl_int CL_API_CALL
clEnqueueFillImage(cl_command_queue    command_queue ,
        cl_mem              image ,
        const void *        fill_color ,
        const size_t        origin[3] ,
        const size_t        region[3] ,
        cl_uint             num_events ,
        const cl_event *    event_list ,
        cl_event *          event )
{
    cl_int err = CL_SUCCESS;
    if ( EnqueueFillImage )
    {
        if( !apiImageOverflowCheck("clEnqueueFillImage", image, origin, region) )
        {
            err =
                EnqueueFillImage( command_queue ,
                        image ,
                        fill_color ,
                        origin ,
                        region ,
                        num_events ,
                        event_list ,
                        event );
        }
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }

    return(err);
}

    CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyImage(cl_command_queue      command_queue ,
        cl_mem                src_image ,
        cl_mem                dst_image ,
        const size_t          src_origin[3] ,
        const size_t          dst_origin[3] ,
        const size_t          region[3] ,
        cl_uint               num_events ,
        const cl_event *      event_list ,
        cl_event *            event )
{
    cl_int err = CL_SUCCESS;
    if ( EnqueueCopyImage )
    {
        if( !apiImageOverflowCheck("clEnqueueCopyImage", src_image, src_origin, region)
            && !apiImageOverflowCheck("clEnqueueCopyImage", dst_image, dst_origin, region) )
        {
            err =
                EnqueueCopyImage(command_queue ,
                        src_image ,
                        dst_image ,
                        src_origin ,
                        dst_origin ,
                        region ,
                        num_events ,
                        event_list ,
                        event );
        }
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }

    return(err);
}

    CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyImageToBuffer(cl_command_queue  command_queue ,
        cl_mem            src_image ,
        cl_mem            dst_buffer ,
        const size_t      src_origin[3] ,
        const size_t      region[3] ,
        size_t            dst_offset ,
        cl_uint           num_events ,
        const cl_event *  event_list ,
        cl_event *        event )
{
    cl_int err = CL_SUCCESS;
    if ( EnqueueCopyImageToBuffer )
    {
        cl_memobj *m1 = cl_mem_find(get_cl_mem_alloc(), src_image);
		int run_real_call = 0;
		if (m1 != NULL)
		{
			size_t dataSize = getImageDataSize(&m1->image_format);
			size_t row_pitch = region[0] * dataSize;
			size_t slice_pitch = region[1] * row_pitch;
			size_t dst_origin[] = {dst_offset, 0, 0};

			if( !apiImageOverflowCheck("clEnqueueCopyImageToBuffer", src_image, src_origin, region)
					&& !apiBufferRectOverflowCheck("clEnqueueCopyImageToBuffer", dst_buffer, dst_origin, region, row_pitch, slice_pitch) )
			{
				run_real_call = 1;
			}
		}
		else
			run_real_call = 1;

		if (run_real_call)
		{
			err = EnqueueCopyImageToBuffer(command_queue, src_image,
					dst_buffer, src_origin, region, dst_offset,
					num_events, event_list, event);
		}
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }

    return(err);
}

    CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyBufferToImage(cl_command_queue  command_queue ,
        cl_mem            src_buffer ,
        cl_mem            dst_image ,
        size_t            src_offset ,
        const size_t      dst_origin[3] ,
        const size_t      region[3] ,
        cl_uint           num_events ,
        const cl_event *  event_list ,
        cl_event *        event )
{
    cl_int err = CL_SUCCESS;
    if ( EnqueueCopyBufferToImage )
    {
        cl_memobj *m1 = cl_mem_find(get_cl_mem_alloc(), dst_image);
        int run_real_call = 0;
        if (m1 != NULL)
        {
            size_t dataSize = getImageDataSize(&m1->image_format);
            size_t row_pitch = region[0] * dataSize;
            size_t slice_pitch = region[1] * row_pitch;
            size_t src_origin[] = {src_offset, 0, 0};

            if( !apiBufferRectOverflowCheck("clEnqueueCopyBufferToImage", src_buffer, src_origin, region, row_pitch, slice_pitch)
                    && !apiImageOverflowCheck("clEnqueueCopyBufferToImage", dst_image, dst_origin, region) )
            {
                run_real_call = 1;
            }
        }
        else
            run_real_call = 1;

        if (run_real_call)
        {
            err = EnqueueCopyBufferToImage(command_queue, src_buffer,
                    dst_image, src_offset, dst_origin, region,
                    num_events, event_list, event);
        }
    }
    else
    {
        CL_MSG("NOT FOUND!");
    }

    return(err);
}

