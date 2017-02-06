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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ********************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <CL/cl.h>

char *help_string = \
"\nThis application is for accessing device information through the generic\n"\
"OpenCL API calls. This is preferable over attempting to use\n"\
"vendor specific calls.\n"\
"\n"\
"options:\n"\
    "\t-c\n"\
        "\t\tuse this to find a CPU OpenCL device\n"\
        "\t\t1 found, 0 otherwise\n"\
    "\t-g\n"\
        "\t\tuse this to find a GPU OpenCL device\n"\
        "\t\t1 found, 0 otherwise\n"\
    "\t-n\n"
        "\t\tfind the number of cus for the selected device\n"\
        "\t\tprints number of cus for device\n"\
    "\t-h\n"\
        "\t\tprint the help\n"\
"\n"\
"Note:\n"\
"either -c, -g, or both must be selected or function will immediately return\n\n";

/*!
 * This function is for accessing device information through the generic
 * OpenCL API calls. This is preferable over attempting to use
 * vendor specific calls.
 *
 * basic options:
 *  -c
 *      use this to find a CPU OpenCL device
 *      if used by itself returns 1 for found (0 for not)
 *      if used with -n returns number of cus
 *  -g
 *      use this to find a GPU OpenCL device
 *      if used by itself returns 1 for found (0 for not)
 *      if used with -n returns number of cus
 *  -n
 *      find the number of cus for the selected device
 *  -h
 *      print the help
 *
 * Note:
 *  either -c, -g, or both must be selected or function will immediately return
 *
 */
int main(int argc, char** argv)
{
    uint32_t find_cpu = 0;
    uint32_t find_gpu = 0;
    uint32_t get_cus = 0;
    int argi;

    if(argc < 2)
    {
        fprintf(stdout, "%s", help_string);
        return 0;
    }
    for(argi = 1; argi < argc && argv[argi][0] == '-'; argi++)
    {
        switch(argv[argi][1])
        {
            case 'n':
                get_cus = 1;
                break;
            case 'c':
                find_cpu = 1;
                break;
            case 'g':
                find_gpu = 1;
                break;
            default:
                fprintf(stdout, "%s", help_string);
                return 0;
        }
    }

    if(find_cpu == find_gpu && find_cpu == 0)
    {
        fprintf(stdout, "%s", help_string);
        return 0;
    }

    cl_uint num_platforms;

    clGetPlatformIDs(0, NULL, &num_platforms);

    cl_uint num_cus = 0;
    uint32_t has_cpu = 0;
    uint32_t has_gpu = 0;

    if(num_platforms > 0)
    {
        cl_platform_id *platforms = calloc(sizeof(cl_platform_id), num_platforms);
        clGetPlatformIDs(num_platforms, platforms, NULL);

        uint32_t i;
        for(i = 0; i < num_platforms; i++)
        {
            cl_device_id *devices = NULL;
            cl_uint num_devices = 1;
            cl_device_type dev_type = CL_DEVICE_TYPE_ALL;

            if(find_cpu == find_gpu)
                dev_type = CL_DEVICE_TYPE_ALL;
            else if(find_cpu)
                dev_type = CL_DEVICE_TYPE_CPU;
            else if(find_gpu)
                dev_type = CL_DEVICE_TYPE_GPU;

            clGetDeviceIDs(platforms[i], dev_type, 0, devices, &num_devices);

            if(num_devices > 0)
            {
                if(find_cpu)
                    has_cpu = 1;
                if(find_gpu)
                    has_gpu = 1;

                devices = calloc(sizeof(cl_device_id), num_devices);

                clGetDeviceIDs(platforms[i], dev_type, num_devices, devices, NULL);

                uint32_t j;
                for(j = 0; j < num_devices; j++)
                {
                    cl_uint this_cus;
                    clGetDeviceInfo(devices[j], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &this_cus, NULL);

                    if(num_cus == 0 || (this_cus > 0 && this_cus < num_cus))
                        num_cus = this_cus;
                }

                free(devices);
            }
        }

        free(platforms);
    }

    if(get_cus)
        fprintf(stdout, "%u\n", num_cus);
    else
        fprintf(stdout, "%u\n", (has_cpu | has_gpu) );


    return 0;
}
