#include <stdint.h>
#include <stdio.h>
#include <CL/cl.h>

int main(int argc, char** argv)
{
    cl_platform_id *platforms;
    cl_uint num_platforms;

    clGetPlatformIDs(0, NULL, &num_platforms);

    cl_uint num_cus = 0;

    if(num_platforms > 0)
    {
        platforms = calloc(sizeof(cl_platform_id), num_platforms);
        clGetPlatformIDs(num_platforms, platforms, NULL);

        uint32_t i;
        for(i = 0; i < num_platforms; i++)
        {
            cl_uint num_devices = 1;
            cl_device_id devices[1];
            //clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 0, devices, &num_devices);

            if(num_devices > 0)
            {
                //devices = calloc(sizeof(cl_device_id), num_devices);

                clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, num_devices, devices, NULL);

                uint32_t j;
                for(j = 0; j < num_devices; j++)
                {
                    cl_uint this_cus;
                    clGetDeviceInfo(devices[j], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &this_cus, NULL);

                    if(num_cus == 0 || (this_cus > 0 && this_cus < num_cus))
                        num_cus = this_cus;
                }

                //free(devices);
            }
        }

        free(platforms);
    }

    fprintf(stdout, "%u\n", num_cus);


    return 0;
}
