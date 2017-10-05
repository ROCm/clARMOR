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


/*! \file cl_interceptor_internal.h
 * This file is used to by the OpenCL LD_PRELOAD interceptor to declare the
 * wrapper functions for each of the OpenCL APIs that it uses.
 * Each of these is a function pointer that will be used to point to the
 * *real* OpenCL function. Our LD_PRELOAD wrapper will take over the name
 * of the real function, do some work, then call the appropriate real
 * function pointer (in most cases -- in some cases, we never call the real
 * function because it has bugs.)
 *
 * Portability function headers added for stuff like the Win32 platform.
 * Including these blank definitions here so that code checkers and compilers
 * that didn't properly find cl_platform will know that we can just ignore them
 */

#ifndef __CL_INTERCEPTOR_INTERNAL_H
#define __CL_INTERCEPTOR_INTERNAL_H

#ifndef CL_API_ENTRY
    #define CL_API_ENTRY
#endif
#ifndef CL_API_CALL
    #define CL_API_CALL
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef CL_API_ENTRY cl_int
    (CL_API_CALL * interceptor_clGetDeviceInfo)(
            cl_device_id device,
            cl_device_info param_name,
            size_t param_value_size,
            void *param_value,
            size_t *param_value_size_ret);

/* Command Queue APIs */
typedef CL_API_ENTRY cl_command_queue
    (CL_API_CALL * interceptor_clCreateCommandQueue)(
            cl_context context,
            cl_device_id device,
            cl_command_queue_properties properties,
            cl_int *errcode_ret);

#ifdef CL_VERSION_2_0
typedef CL_API_ENTRY cl_command_queue
    (CL_API_CALL * interceptor_clCreateCommandQueueWithProperties)(
            cl_context context,
            cl_device_id device,
            const cl_queue_properties *properties,
            cl_int *errcode_ret);
#endif // CL_VERSION_2_0

typedef CL_API_ENTRY cl_int
    (CL_API_CALL * interceptor_clRetainCommandQueue)(
            cl_command_queue command_queue);

typedef CL_API_ENTRY cl_int
    (CL_API_CALL * interceptor_clReleaseCommandQueue)(
            cl_command_queue command_queue);


/* Memory Object APIs */
typedef CL_API_ENTRY cl_int
    (CL_API_CALL * interceptor_clGetMemObjectInfo)(
            cl_mem memobj,
            cl_mem_info param_name,
            size_t param_value_size,
            void *param_value,
            size_t *param_value_size_ret);

typedef CL_API_ENTRY cl_mem
    (CL_API_CALL * interceptor_clCreateBuffer)(
            cl_context context,
            cl_mem_flags flags,
            size_t size,
            void *host_ptr,
            cl_int *errcode_ret);

typedef CL_API_ENTRY cl_mem
    (CL_API_CALL * interceptor_clCreateSubBuffer)(
            cl_mem buffer,
            cl_mem_flags flags,
            cl_buffer_create_type buffer_create_type,
            const void *buffer_create_info,
            cl_int *errcode_ret);

typedef CL_API_ENTRY cl_int
    (CL_API_CALL * interceptor_clGetImageInfo)(
            cl_mem image,
            cl_image_info param_name,
            size_t param_value_size,
            void *param_value,
            size_t *param_value_size_ret);

#ifdef CL_VERSION_1_2
typedef CL_API_ENTRY cl_mem
    (CL_API_CALL * interceptor_clCreateImage)(
            cl_context context,
            cl_mem_flags flags,
            const cl_image_format *image_format,
            const cl_image_desc *image_desc,
            void *host_ptr,
            cl_int *errcode_ret);
#endif

typedef CL_API_ENTRY cl_mem
    (CL_API_CALL * interceptor_clCreateImage2D)(
            cl_context context,
            cl_mem_flags flags,
            const cl_image_format *image_format,
            size_t image_width,
            size_t image_height,
            size_t image_row_pitch,
            void * host_ptr,
            cl_int * errcode_ret);

typedef CL_API_ENTRY cl_mem
    (CL_API_CALL * interceptor_clCreateImage3D)(
            cl_context context,
            cl_mem_flags flags,
            const cl_image_format *image_format,
            size_t image_width,
            size_t image_height,
            size_t image_depth,
            size_t image_row_pitch,
            size_t image_slice_pitch,
            void *host_ptr,
            cl_int *errcode_ret);

typedef CL_API_ENTRY cl_int
(CL_API_CALL * interceptor_clRetainMemObject)(cl_mem memobj);

typedef CL_API_ENTRY cl_int
(CL_API_CALL * interceptor_clReleaseMemObject)(cl_mem memobj);


/* SVM APIs */
#ifdef CL_VERSION_2_0
typedef CL_API_ENTRY void*
    (CL_API_CALL * interceptor_clSVMAlloc)(
            cl_context context,
            cl_svm_mem_flags flags,
            size_t size,
            unsigned int alignment);

typedef CL_API_ENTRY void
    (CL_API_CALL * interceptor_clSVMFree)(
            cl_context context,
            void * svm_pointer);

typedef CL_API_ENTRY cl_int
    (CL_API_CALL * interceptor_clEnqueueSVMFree)(
            cl_command_queue command_queue,
            cl_uint num_svm_pointers,
            void * svm_pointers[],
            void (CL_CALLBACK *pfn_free_func)(
                cl_command_queue queue,
                cl_uint num_svm_pointers,
                void * svm_pointers[],
                void * user_data),
            void *user_data,
            cl_uint num_events_in_wait_list,
            const cl_event *event_wait_list,
            cl_event *event);

typedef CL_API_ENTRY cl_int
    (CL_API_CALL * interceptor_clEnqueueSVMMap)(
            cl_command_queue command_queue,
            cl_bool blocking_map,
            cl_map_flags map_flags,
            void *svm_ptr,
            size_t size,
            cl_uint num_events_in_wait_list,
            const cl_event *event_wait_list,
            cl_event *event);

typedef CL_API_ENTRY cl_int
    (CL_API_CALL * interceptor_clEnqueueSVMMemcpy)(
            cl_command_queue command_queue,
            cl_bool blocking_copy,
            void *dst_ptr,
            const void *src_ptr,
            size_t size,
            cl_uint num_events,
            const cl_event *event_list,
            cl_event *event);

typedef CL_API_ENTRY cl_int
    (CL_API_CALL * interceptor_clEnqueueSVMMemFill)(
            cl_command_queue command_queue,
            void *svm_ptr,
            const void *pattern,
            size_t pattern_size,
            size_t size,
            cl_uint num_events,
            const cl_event *event_list,
            cl_event *event);

typedef CL_API_ENTRY cl_int
    (CL_API_CALL * interceptor_clEnqueueSVMUnmap)(
            cl_command_queue command_queue,
            void *svm_ptr,
            cl_uint num_events_in_wait_list,
            const cl_event *event_wait_list,
            cl_event *event);

#endif // CL_VERSION_2_0


/* Kernel APIs */
typedef CL_API_ENTRY cl_int
    (CL_API_CALL * interceptor_clRetainKernel)(cl_kernel kernel);

typedef CL_API_ENTRY cl_int
    (CL_API_CALL * interceptor_clReleaseKernel)(cl_kernel kernel);

typedef CL_API_ENTRY cl_int
    (CL_API_CALL * interceptor_clEnqueueNDRangeKernel)(
            cl_command_queue command_queue,
            cl_kernel kernel,
            cl_uint work_dim,
            const size_t *global_work_offset,
            const size_t *global_work_size,
            const size_t *local_work_size,
            cl_uint num_events_in_wait_list,
            const cl_event *event_wait_list,
            cl_event * event);

typedef CL_API_ENTRY cl_int
    (CL_API_CALL * interceptor_clEnqueueTask)(cl_command_queue command_queue,
            cl_kernel kernel,
            cl_uint num_events_in_wait_list,
            const cl_event *event_wait_list,
            cl_event *event);

typedef CL_API_ENTRY cl_int
    (CL_API_CALL * interceptor_clEnqueueNativeKernel)(
            cl_command_queue command_queue,
            void (CL_CALLBACK * user_func)(void *),
            void *args,
            size_t cb_args,
            cl_uint num_mem_objects,
            const cl_mem *mem_list,
            const void **args_mem_loc,
            cl_uint num_events_in_wait_list,
            const cl_event *event_wait_list,
            cl_event *event);


/* Kernel argument APIs */
typedef CL_API_ENTRY cl_int
    (CL_API_CALL * interceptor_clSetKernelArg)(
            cl_kernel kernel,
            cl_uint arg_index,
            size_t arg_size,
            const void *arg_value);

#ifdef CL_VERSION_2_0
typedef CL_API_ENTRY cl_int
    (CL_API_CALL * interceptor_clSetKernelArgSVMPointer)(
            cl_kernel kernel,
            cl_uint arg_index,
            const void *arg_value);
#endif


/* Event APIs */
typedef CL_API_ENTRY cl_int
    (CL_API_CALL * interceptor_clGetEventProfilingInfo)(
            cl_event event,
            cl_profiling_info param_name,
            size_t param_value_size,
            void *param_value,
            size_t *param_value_size_ret);

typedef CL_API_ENTRY cl_int
    (CL_API_CALL * interceptor_clRetainEvent)(cl_event event);

typedef CL_API_ENTRY cl_int
    (CL_API_CALL * interceptor_clReleaseEvent)(cl_event event);


/* Enqueued Commands APIs */
typedef CL_API_ENTRY cl_int
    (CL_API_CALL * interceptor_clEnqueueReadBuffer)(
            cl_command_queue    /* command_queue */,
            cl_mem              /* buffer */,
            cl_bool             /* blocking_read */,
            size_t              /* offset */,
            size_t              /* size */,
            void *              /* ptr */,
            cl_uint             /* num_events_in_wait_list */,
            const cl_event *    /* event_wait_list */,
            cl_event *          /* event */);

typedef CL_API_ENTRY cl_int
    (CL_API_CALL * interceptor_clEnqueueReadBufferRect)(
            cl_command_queue    /* command_queue */,
            cl_mem              /* buffer */,
            cl_bool             /* blocking_read */,
            const size_t *      /* buffer_offset */,
            const size_t *      /* host_offset */,
            const size_t *      /* region */,
            size_t              /* buffer_row_pitch */,
            size_t              /* buffer_slice_pitch */,
            size_t              /* host_row_pitch */,
            size_t              /* host_slice_pitch */,
            void *              /* ptr */,
            cl_uint             /* num_events_in_wait_list */,
            const cl_event *    /* event_wait_list */,
            cl_event *          /* event */);

typedef CL_API_ENTRY cl_int
    (CL_API_CALL * interceptor_clEnqueueWriteBuffer)(
            cl_command_queue   /* command_queue */,
            cl_mem             /* buffer */,
            cl_bool            /* blocking_write */,
            size_t             /* offset */,
            size_t             /* size */,
            const void *       /* ptr */,
            cl_uint            /* num_events_in_wait_list */,
            const cl_event *   /* event_wait_list */,
            cl_event *         /* event */);

typedef CL_API_ENTRY cl_int
    (CL_API_CALL * interceptor_clEnqueueWriteBufferRect)(
            cl_command_queue    /* command_queue */,
            cl_mem              /* buffer */,
            cl_bool             /* blocking_write */,
            const size_t *      /* buffer_offset */,
            const size_t *      /* host_offset */,
            const size_t *      /* region */,
            size_t              /* buffer_row_pitch */,
            size_t              /* buffer_slice_pitch */,
            size_t              /* host_row_pitch */,
            size_t              /* host_slice_pitch */,
            const void *        /* ptr */,
            cl_uint             /* num_events_in_wait_list */,
            const cl_event *    /* event_wait_list */,
            cl_event *          /* event */);

typedef CL_API_ENTRY cl_int
    (CL_API_CALL * interceptor_clEnqueueFillBuffer)(
            cl_command_queue   /* command_queue */,
            cl_mem             /* buffer */,
            const void *       /* pattern */,
            size_t             /* pattern_size */,
            size_t             /* offset */,
            size_t             /* size */,
            cl_uint            /* num_events_in_wait_list */,
            const cl_event *   /* event_wait_list */,
            cl_event *         /* event */);

typedef CL_API_ENTRY cl_int
    (CL_API_CALL * interceptor_clEnqueueCopyBuffer)(
            cl_command_queue    /* command_queue */,
            cl_mem              /* src_buffer */,
            cl_mem              /* dst_buffer */,
            size_t              /* src_offset */,
            size_t              /* dst_offset */,
            size_t              /* size */,
            cl_uint             /* num_events_in_wait_list */,
            const cl_event *    /* event_wait_list */,
            cl_event *          /* event */);

typedef CL_API_ENTRY cl_int
    (CL_API_CALL * interceptor_clEnqueueCopyBufferRect)(
            cl_command_queue    /* command_queue */,
            cl_mem              /* src_buffer */,
            cl_mem              /* dst_buffer */,
            const size_t *      /* src_origin */,
            const size_t *      /* dst_origin */,
            const size_t *      /* region */,
            size_t              /* src_row_pitch */,
            size_t              /* src_slice_pitch */,
            size_t              /* dst_row_pitch */,
            size_t              /* dst_slice_pitch */,
            cl_uint             /* num_events_in_wait_list */,
            const cl_event *    /* event_wait_list */,
            cl_event *          /* event */) ;

typedef CL_API_ENTRY cl_int
    (CL_API_CALL * interceptor_clEnqueueReadImage)(
            cl_command_queue     /* command_queue */,
            cl_mem               /* image */,
            cl_bool              /* blocking_read */,
            const size_t *       /* origin[3] */,
            const size_t *       /* region[3] */,
            size_t               /* row_pitch */,
            size_t               /* slice_pitch */,
            void *               /* ptr */,
            cl_uint              /* num_events_in_wait_list */,
            const cl_event *     /* event_wait_list */,
            cl_event *           /* event */);

typedef CL_API_ENTRY cl_int
    (CL_API_CALL * interceptor_clEnqueueWriteImage)(
            cl_command_queue    /* command_queue */,
            cl_mem              /* image */,
            cl_bool             /* blocking_write */,
            const size_t *      /* origin[3] */,
            const size_t *      /* region[3] */,
            size_t              /* input_row_pitch */,
            size_t              /* input_slice_pitch */,
            const void *        /* ptr */,
            cl_uint             /* num_events_in_wait_list */,
            const cl_event *    /* event_wait_list */,
            cl_event *          /* event */);

typedef CL_API_ENTRY cl_int
    (CL_API_CALL * interceptor_clEnqueueFillImage)(
            cl_command_queue   /* command_queue */,
            cl_mem             /* image */,
            const void *       /* fill_color */,
            const size_t *     /* origin[3] */,
            const size_t *     /* region[3] */,
            cl_uint            /* num_events_in_wait_list */,
            const cl_event *   /* event_wait_list */,
            cl_event *         /* event */) ;

typedef CL_API_ENTRY cl_int
    (CL_API_CALL * interceptor_clEnqueueCopyImage)(
            cl_command_queue     /* command_queue */,
            cl_mem               /* src_image */,
            cl_mem               /* dst_image */,
            const size_t *       /* src_origin[3] */,
            const size_t *       /* dst_origin[3] */,
            const size_t *       /* region[3] */,
            cl_uint              /* num_events_in_wait_list */,
            const cl_event *     /* event_wait_list */,
            cl_event *           /* event */);

typedef CL_API_ENTRY cl_int
    (CL_API_CALL * interceptor_clEnqueueCopyImageToBuffer)(
            cl_command_queue /* command_queue */,
            cl_mem           /* src_image */,
            cl_mem           /* dst_buffer */,
            const size_t *   /* src_origin[3] */,
            const size_t *   /* region[3] */,
            size_t           /* dst_offset */,
            cl_uint          /* num_events_in_wait_list */,
            const cl_event * /* event_wait_list */,
            cl_event *       /* event */);

typedef CL_API_ENTRY cl_int
    (CL_API_CALL * interceptor_clEnqueueCopyBufferToImage)(
            cl_command_queue /* command_queue */,
            cl_mem           /* src_buffer */,
            cl_mem           /* dst_image */,
            size_t           /* src_offset */,
            const size_t *   /* dst_origin[3] */,
            const size_t *   /* region[3] */,
            cl_uint          /* num_events_in_wait_list */,
            const cl_event * /* event_wait_list */,
            cl_event *       /* event */);

#ifdef __cplusplus
}
#endif
#endif // __CL_INTERCEPTOR_INTERNAL_H
