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

#include <CL/cl.h>
#include <string>
#include "detector_defines.h"

#include "gpu_check_kernels.h"

//canary length must be a multiple of 4 (to do check in 32 bit words) (word comparisons)
//canary length must be a multiple of the local group size (usually larger than previous requirement) (work_group_scan)
//
//compareWithPoison - compare word with poison, then find first byte in word that differs
//findCorruption - parse through cl_mem and svm buffers, find words that do not match canaries
const char *buffer_copy_canary_src =
"uint compareWithPoison(uint poison,\n\
                            uint localBuff,\n\
                            uint index,\n\
                            __global uchar *B)\n\
{\n\
    uint ret = INT_MAX;\n\
    if(poison != ((__global uint*)B)[index])\n\
    {\n\
        uint i;\n\
        for(i=0; i < 4; i++)\n\
        {\n\
            if((poison & 0xFF) != B[4*index+i])\n\
            {\n\
                ret = 4*localBuff + i;\n\
                break;\n\
            }\n\
        }\n\
    }\n\
    return ret;\n\
}\n\n\
__kernel void findCorruption(uint canaryLen,\n\
                            uint buffEnd,\n\
                            uint svmEnd,\n\
                            uint poison,\n\
                            __global uint *B,\n"
#ifdef CL_VERSION_2_0
"                            __global uint *C,\n"
#endif
"                            __global uint *first)\n\
{\n\
    int tid = get_global_id(0);\n\
    if(tid >= svmEnd) return;\n\
    uint buffID = tid / canaryLen;\n\
    uint localBuff = tid % canaryLen;\n\
    int lid = get_local_id(0);\n\
    uint ret = INT_MAX;\n\
    if(tid < buffEnd)\n\
    {\n\
        ret = compareWithPoison(poison, localBuff, tid, (__global uchar*)B);\n\
    }\n"
#ifdef CL_VERSION_2_0
"    else\n\
    {\n\
        ret = compareWithPoison(poison, localBuff, tid - buffEnd, (__global uchar*)C);\n\
    }\n\
    uint wgRet = work_group_scan_inclusive_min(ret);\n\
    if(lid == get_local_size(0)-1 || tid == svmEnd-1)\n\
    {\n\
        atomic_min((global uint*)&first[buffID], (uint)wgRet);\n\
    }\n"
#else
"    atomic_min((global unsigned int*)&first[buffID], ret);\n"
#endif
"}\n\n\
__kernel void findCorruptionNoSVM(uint canaryLen,\n\
                                uint buffEnd,\n\
                                uint svmEnd,\n\
                                uint poison,\n\
                                __global uint *B,\n\
                                __global uint *first)\n\
{\n\
    int tid = get_global_id(0);\n\
    if(tid >= svmEnd) return;\n\
    uint buffID = tid / canaryLen;\n\
    uint localBuff = tid % canaryLen;\n\
    int lid = get_local_id(0);\n\
    uint ret = INT_MAX;\n\
    if(tid < buffEnd)\n\
    {\n\
        ret = compareWithPoison(poison, localBuff, tid, (__global uchar*)B);\n\
    }\n"
#ifdef CL_VERSION_2_0
"    uint wgRet = work_group_scan_inclusive_min(ret);\n\
    if(lid == get_local_size(0)-1 || tid == svmEnd-1)\n\
    {\n\
        atomic_min((global uint*)&first[buffID], (uint)wgRet);\n\
    }\n"
#else
"    atomic_min((global uint*)&first[buffID], ret);\n"
#endif
"}";

const char * get_buffer_copy_canary_src(void)
{
    return buffer_copy_canary_src;
}

const char *image_copy_canary_src =
"__kernel void findCorruption(uchar poison,\n\
                            uint num_buff,\n\
                            __global uint *ends,\n\
                            __global uchar *B,\n\
                            __global uint *first)\n\
{\n\
    int tid = get_global_id(0);\n\
    uint buffID;\n\
    for(buffID=0; buffID < num_buff && tid >= ends[buffID]; buffID++){}\n\
    if(buffID >= num_buff) return;\n\
    uint localBuff = tid - ((buffID > 0) ? ends[buffID-1] : 0);\n\
    uint ret = INT_MAX;\n\
    if(poison != B[tid])\n\
    {\n\
        ret = localBuff;\n\
        atomic_min((global unsigned int*)&first[buffID], (unsigned int)ret);\n\
    }\n\
}";

const char * get_image_copy_canary_src(void)
{
    return image_copy_canary_src;
}

//compareWithPoison works when the canary pointer is word aligned
//if not use compareWithPoisonByte (slower)
const char *single_buffer_src =
" \n\
uint compareWithPoison(uint poison,\n\
                       uint index,\n\
                       __global uint *canary)\n\
{\n\
    uint retval = INT_MAX;\n\
    uint word = poison ^ canary[index];\n\
    if(word)\n\
    {\n\
        uint i;\n\
        for(i=0; i < 4; i++)\n\
        {\n\
            if(((uchar*)&word)[i])\n\
            {\n\
                retval = 4*index + i;\n\
                break;\n\
            }\n\
        }\n\
        canary[index] = poison;\n\
    }\n\
    return retval;\n\
}\n\
uint compareWithPoisonByte(char poison,\n\
                       uint index,\n\
                       __global char *canary)\n\
{\n\
    uint ret = INT_MAX;\n\
    uint i;\n\
    uint tmpRet = INT_MAX;\n\
    for(i=0; i < 4; i++)\n\
    {\n\
        if(poison != canary[index+i])\n\
        {\n\
            tmpRet = index+i;\n\
            canary[index+i] = poison;\n\
        }\n\
        if(tmpRet < ret) ret = tmpRet;\n\
    }\n\
    return ret;\n\
}\n\
\n\
__kernel void locateDiffParts(uint length,\n\
                            uint buffID,\n\
                            uint poison,\n\
                            uint offset,\n\
                            __global uchar *B,\n\
                            __global uint *first)\n\
{\n\
    int tid = get_global_id(0);\n\
    if(tid >= length) return;\n\
    int lid = get_local_id(0);\n\
    uint ret = INT_MAX;\n\
    __global uint *canary = (__global uint*)(B+offset);\n\
    int diff = offset % 4;\n\
    if(diff)\n\
    {\n\
        ret = compareWithPoisonByte((char)(0xFF&poison), tid*4, (__global char*)canary);\n\
    }\n\
    else\n\
    {\n\
        ret = compareWithPoison(poison, tid, canary);\n\
    }\n"
#ifdef CL_VERSION_2_0
"    uint wgRet = work_group_scan_inclusive_min(ret);\n\
    if(lid == get_local_size(0)-1 || tid == length-1)\n\
    {\n\
        atomic_min(&first[buffID], wgRet);\n\
    }\n"
#else
    "atomic_min(&first[buffID], ret);\n"
#endif
"}";

const char * get_single_buffer_src(void)
{
    return single_buffer_src;
}

std::string buffer_and_ptr_copy_src =
"uint compareWithPoison(uint poison,\n\
                            uint localBuff,\n\
                            uint index,\n\
                            __global uchar *B)\n\
{\n\
    uint ret = INT_MAX;\n\
    if(poison != ((__global uint*)B)[index])\n\
    {\n\
        uint i;\n\
        for(i=0; i < 4; i++)\n\
        {\n\
            if((poison & 0xFF) != B[4*index+i])\n\
            {\n\
                ret = 4*localBuff + i;\n\
                break;\n\
            }\n\
        }\n\
        ((__global uint*)B)[index] = poison;\n\
    }\n\
    return ret;\n\
}\n\n\
__kernel void locateDiffSVMPtr(uint length,\n\
                            uint endBuffs,\n\
                            uint endSVM,\n\
                            uint poison,\n\
                            __global uint *B,\n\
                            __global ulong *C,\n\
                            __global uint *first)\n\
{\n\
    int tid = get_global_id(0);\n\
    if(tid >= endSVM) return;\n\
    int lid = get_local_id(0);\n\
    uint buffID = tid/(length);\n\
    uint localBuff = tid % length;\n\
    uint ret = INT_MAX;\n\
    if(tid < endBuffs)\n\
    {\n\
        ret = compareWithPoison(poison, localBuff, tid, (__global uchar*)B);\n\
    }\n"
#ifdef CL_VERSION_2_0
"    else\n\
    {\n\
        uint index = tid % (length/"+std::to_string(POISON_REGIONS)+");\n\
        __global uint *val_ptr = (__global uint*)C[(tid - endBuffs) / (length/"+std::to_string(POISON_REGIONS)+")];\n\
        ret = compareWithPoison(poison, localBuff, index, (__global uchar*)val_ptr);\n\
    }\n"
    "uint wgRet = work_group_scan_inclusive_min(ret);\n\
    if(lid == get_local_size(0)-1)\n\
    {\n\
        atomic_min((global uint*)&first[buffID], (uint)wgRet);\n\
    }\n"
#else
    "atomic_min((__global uint*)&first[buffID], (uint)ret);\n"
#endif
"}";

const char * get_buffer_and_ptr_copy_src(void)
{
    return buffer_and_ptr_copy_src.c_str();
}
