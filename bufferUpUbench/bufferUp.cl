
__kernel void sumArr( uint length, 
                    __global char *A, 
                    __global uint *out)
{
    int tid = get_global_id(0);
    if(tid >= length) return;
    int lid = get_local_id(0);

    uint ret = work_group_scan_inclusive_add(A[tid]);
    if(lid == get_local_size(0)-1 || tid == length-1)
    {
        //out[0] = A[tid];
        atomic_add(out, ret);
    }
}


__kernel void sumArrMid( uint length, 
                    __global char *A, 
                    __global uint *out)
{
    int tid = get_global_id(0);
    if(tid >= length) return;
    int lid = get_local_id(0);

    uint i;
    uint feedback=0;
    for(i=1; i < 10000; i++)
    {
        feedback += A[tid] / i;
    }

    uint ret = work_group_scan_inclusive_add(feedback);
    if(lid == get_local_size(0)-1 || tid == length-1)
    {
        //out[0] = A[tid];
        atomic_add(out, ret);
    }
}

__kernel void sumArrHvy( uint length, 
                    __global char *A, 
                    __global uint *out)
{
    int tid = get_global_id(0);
    if(tid >= length) return;
    int lid = get_local_id(0);

    uint i;
    uint feedback=0;
    for(i=1; i < 1000000; i++)
    {
        feedback += A[tid] / i;
    }

    uint ret = work_group_scan_inclusive_add(feedback);
    if(lid == get_local_size(0)-1 || tid == length-1)
    {
        //out[0] = A[tid];
        atomic_add(out, ret);
    }
}

__kernel void sumArrQ2( uint length, 
                    __global char *A, 
                    __global uint *out)
{
    sumArr(length, A, out);
}

__kernel void sumArrQ4( uint length, 
                    __global char *A, 
                    __global char *B, 
                    __global char *C, 
                    __global uint *out)
{
    sumArr(length, A, out);
}

__kernel void sumArrQ6( uint length, 
                    __global char *A, 
                    __global char *B, 
                    __global char *C, 
                    __global char *D, 
                    __global char *E, 
                    __global uint *out)
{
    sumArr(length, A, out);
}

__kernel void sumArrQ8( uint length, 
                    __global char *A, 
                    __global char *B, 
                    __global char *C, 
                    __global char *D, 
                    __global char *E, 
                    __global char *F, 
                    __global char *G, 
                    __global uint *out)
{
    sumArr(length, A, out);
}

__kernel void sumArrQ10( uint length, 
                    __global char *A, 
                    __global char *B, 
                    __global char *C, 
                    __global char *D, 
                    __global char *E, 
                    __global char *F, 
                    __global char *G, 
                    __global char *H, 
                    __global char *I, 
                    __global uint *out)
{
    sumArr(length, A, out);
}

__kernel void sumArrQ12( uint length, 
                    __global char *A, 
                    __global char *B, 
                    __global char *C, 
                    __global char *D, 
                    __global char *E, 
                    __global char *F, 
                    __global char *G, 
                    __global char *H, 
                    __global char *I, 
                    __global char *J, 
                    __global char *K, 
                    __global uint *out)
{
    sumArr(length, A, out);
}

__kernel void sumArrQ14( uint length, 
                    __global char *A, 
                    __global char *B, 
                    __global char *C, 
                    __global char *D, 
                    __global char *E, 
                    __global char *F, 
                    __global char *G, 
                    __global char *H, 
                    __global char *I, 
                    __global char *J, 
                    __global char *K, 
                    __global char *L, 
                    __global char *M, 
                    __global uint *out)
{
    sumArr(length, A, out);
}

__kernel void sumArrQ16( uint length, 
                    __global char *A, 
                    __global char *B, 
                    __global char *C, 
                    __global char *D, 
                    __global char *E, 
                    __global char *F, 
                    __global char *G, 
                    __global char *H, 
                    __global char *I, 
                    __global char *J, 
                    __global char *K, 
                    __global char *L, 
                    __global char *M, 
                    __global char *N, 
                    __global char *O, 
                    __global uint *out)
{
    sumArr(length, A, out);
}

__kernel void sumArrQ18( uint length, 
                    __global char *A, 
                    __global char *B, 
                    __global char *C, 
                    __global char *D, 
                    __global char *E, 
                    __global char *F, 
                    __global char *G, 
                    __global char *H, 
                    __global char *I, 
                    __global char *J, 
                    __global char *K, 
                    __global char *L, 
                    __global char *M, 
                    __global char *N, 
                    __global char *O, 
                    __global char *P, 
                    __global char *Q, 
                    __global uint *out)
{
    sumArr(length, A, out);
}

__kernel void sumArrQ20( uint length, 
                    __global char *A, 
                    __global char *B, 
                    __global char *C, 
                    __global char *D, 
                    __global char *E, 
                    __global char *F, 
                    __global char *G, 
                    __global char *H, 
                    __global char *I, 
                    __global char *J, 
                    __global char *K, 
                    __global char *L, 
                    __global char *M, 
                    __global char *N, 
                    __global char *O, 
                    __global char *P, 
                    __global char *Q, 
                    __global char *R, 
                    __global char *S, 
                    __global uint *out)
{
    sumArr(length, A, out);
}


__kernel void sumArrMidQ2( uint length, 
                    __global char *A, 
                    __global uint *out)
{
    sumArrMid(length, A, out);
}

__kernel void sumArrMidQ4( uint length, 
                    __global char *A, 
                    __global char *B, 
                    __global char *C, 
                    __global uint *out)
{
    sumArrMid(length, A, out);
}

__kernel void sumArrMidQ6( uint length, 
                    __global char *A, 
                    __global char *B, 
                    __global char *C, 
                    __global char *D, 
                    __global char *E, 
                    __global uint *out)
{
    sumArrMid(length, A, out);
}

__kernel void sumArrMidQ8( uint length, 
                    __global char *A, 
                    __global char *B, 
                    __global char *C, 
                    __global char *D, 
                    __global char *E, 
                    __global char *F, 
                    __global char *G, 
                    __global uint *out)
{
    sumArrMid(length, A, out);
}

__kernel void sumArrMidQ10( uint length, 
                    __global char *A, 
                    __global char *B, 
                    __global char *C, 
                    __global char *D, 
                    __global char *E, 
                    __global char *F, 
                    __global char *G, 
                    __global char *H, 
                    __global char *I, 
                    __global uint *out)
{
    sumArrMid(length, A, out);
}

__kernel void sumArrMidQ12( uint length, 
                    __global char *A, 
                    __global char *B, 
                    __global char *C, 
                    __global char *D, 
                    __global char *E, 
                    __global char *F, 
                    __global char *G, 
                    __global char *H, 
                    __global char *I, 
                    __global char *J, 
                    __global char *K, 
                    __global uint *out)
{
    sumArrMid(length, A, out);
}

__kernel void sumArrMidQ14( uint length, 
                    __global char *A, 
                    __global char *B, 
                    __global char *C, 
                    __global char *D, 
                    __global char *E, 
                    __global char *F, 
                    __global char *G, 
                    __global char *H, 
                    __global char *I, 
                    __global char *J, 
                    __global char *K, 
                    __global char *L, 
                    __global char *M, 
                    __global uint *out)
{
    sumArrMid(length, A, out);
}

__kernel void sumArrMidQ16( uint length, 
                    __global char *A, 
                    __global char *B, 
                    __global char *C, 
                    __global char *D, 
                    __global char *E, 
                    __global char *F, 
                    __global char *G, 
                    __global char *H, 
                    __global char *I, 
                    __global char *J, 
                    __global char *K, 
                    __global char *L, 
                    __global char *M, 
                    __global char *N, 
                    __global char *O, 
                    __global uint *out)
{
    sumArrMid(length, A, out);
}

__kernel void sumArrMidQ18( uint length, 
                    __global char *A, 
                    __global char *B, 
                    __global char *C, 
                    __global char *D, 
                    __global char *E, 
                    __global char *F, 
                    __global char *G, 
                    __global char *H, 
                    __global char *I, 
                    __global char *J, 
                    __global char *K, 
                    __global char *L, 
                    __global char *M, 
                    __global char *N, 
                    __global char *O, 
                    __global char *P, 
                    __global char *Q, 
                    __global uint *out)
{
    sumArrMid(length, A, out);
}

__kernel void sumArrMidQ20( uint length, 
                    __global char *A, 
                    __global char *B, 
                    __global char *C, 
                    __global char *D, 
                    __global char *E, 
                    __global char *F, 
                    __global char *G, 
                    __global char *H, 
                    __global char *I, 
                    __global char *J, 
                    __global char *K, 
                    __global char *L, 
                    __global char *M, 
                    __global char *N, 
                    __global char *O, 
                    __global char *P, 
                    __global char *Q, 
                    __global char *R, 
                    __global char *S, 
                    __global uint *out)
{
    sumArrMid(length, A, out);
}

__kernel void sumArrHvyQ2( uint length, 
                    __global char *A, 
                    __global uint *out)
{
    sumArrHvy(length, A, out);
}

__kernel void sumArrHvyQ4( uint length, 
                    __global char *A, 
                    __global char *B, 
                    __global char *C, 
                    __global uint *out)
{
    sumArrHvy(length, A, out);
}

__kernel void sumArrHvyQ6( uint length, 
                    __global char *A, 
                    __global char *B, 
                    __global char *C, 
                    __global char *D, 
                    __global char *E, 
                    __global uint *out)
{
    sumArrHvy(length, A, out);
}

__kernel void sumArrHvyQ8( uint length, 
                    __global char *A, 
                    __global char *B, 
                    __global char *C, 
                    __global char *D, 
                    __global char *E, 
                    __global char *F, 
                    __global char *G, 
                    __global uint *out)
{
    sumArrHvy(length, A, out);
}

__kernel void sumArrHvyQ10( uint length, 
                    __global char *A, 
                    __global char *B, 
                    __global char *C, 
                    __global char *D, 
                    __global char *E, 
                    __global char *F, 
                    __global char *G, 
                    __global char *H, 
                    __global char *I, 
                    __global uint *out)
{
    sumArrHvy(length, A, out);
}

__kernel void sumArrHvyQ12( uint length, 
                    __global char *A, 
                    __global char *B, 
                    __global char *C, 
                    __global char *D, 
                    __global char *E, 
                    __global char *F, 
                    __global char *G, 
                    __global char *H, 
                    __global char *I, 
                    __global char *J, 
                    __global char *K, 
                    __global uint *out)
{
    sumArrHvy(length, A, out);
}

__kernel void sumArrHvyQ14( uint length, 
                    __global char *A, 
                    __global char *B, 
                    __global char *C, 
                    __global char *D, 
                    __global char *E, 
                    __global char *F, 
                    __global char *G, 
                    __global char *H, 
                    __global char *I, 
                    __global char *J, 
                    __global char *K, 
                    __global char *L, 
                    __global char *M, 
                    __global uint *out)
{
    sumArrHvy(length, A, out);
}

__kernel void sumArrHvyQ16( uint length, 
                    __global char *A, 
                    __global char *B, 
                    __global char *C, 
                    __global char *D, 
                    __global char *E, 
                    __global char *F, 
                    __global char *G, 
                    __global char *H, 
                    __global char *I, 
                    __global char *J, 
                    __global char *K, 
                    __global char *L, 
                    __global char *M, 
                    __global char *N, 
                    __global char *O, 
                    __global uint *out)
{
    sumArrHvy(length, A, out);
}

__kernel void sumArrHvyQ18( uint length, 
                    __global char *A, 
                    __global char *B, 
                    __global char *C, 
                    __global char *D, 
                    __global char *E, 
                    __global char *F, 
                    __global char *G, 
                    __global char *H, 
                    __global char *I, 
                    __global char *J, 
                    __global char *K, 
                    __global char *L, 
                    __global char *M, 
                    __global char *N, 
                    __global char *O, 
                    __global char *P, 
                    __global char *Q, 
                    __global uint *out)
{
    sumArrHvy(length, A, out);
}

__kernel void sumArrHvyQ20( uint length, 
                    __global char *A, 
                    __global char *B, 
                    __global char *C, 
                    __global char *D, 
                    __global char *E, 
                    __global char *F, 
                    __global char *G, 
                    __global char *H, 
                    __global char *I, 
                    __global char *J, 
                    __global char *K, 
                    __global char *L, 
                    __global char *M, 
                    __global char *N, 
                    __global char *O, 
                    __global char *P, 
                    __global char *Q, 
                    __global char *R, 
                    __global char *S, 
                    __global uint *out)
{
    sumArrHvy(length, A, out);
}



__kernel void test_1d(write_only image1d_t buffer, uint width) {
    uint i = get_global_id(0);
    int coord = i;
    if (i < width) {
        write_imagef(buffer, coord, (float)i);
    }
}

__kernel void test_2d(write_only image2d_t buffer, uint width, 
                      uint height) {
    uint i = get_global_id(0);
    uint j = get_global_id(1);
    int2 coord = {i,j};
    if (i < width && j < height) {
        write_imagef(buffer, coord, (float)i);
    }
}

__kernel void test_3d(write_only image3d_t buffer, uint width, 
                      uint height, uint depth) {
    uint i = get_global_id(0);
    uint j = get_global_id(1);
    uint k = get_global_id(2);
    int4 coord = {i,j,k,0};
    if (i < width && j < height) {
        write_imagef(buffer, coord, (float)i);
    }
}

__kernel void test_2d_2buff(write_only image2d_t buffer, uint width, 
                    uint height,
                    read_only image2d_t buffer2
                      ) {

    test_2d(buffer, width, height);
}

__kernel void test_2d_4buff(write_only image2d_t buffer, uint width, 
                    uint height,
                    read_only image2d_t buffer2,
                    read_only image2d_t buffer3,
                    read_only image2d_t buffer4
                      ) {

    test_2d(buffer, width, height);
}

__kernel void test_2d_6buff(write_only image2d_t buffer, uint width, 
                    uint height,
                    read_only image2d_t buffer2,
                    read_only image2d_t buffer3,
                    read_only image2d_t buffer4,
                    read_only image2d_t buffer5,
                    read_only image2d_t buffer6
                      ) {

    test_2d(buffer, width, height);
}

__kernel void test_2d_8buff(write_only image2d_t buffer, uint width, 
                    uint height,
                    read_only image2d_t buffer2,
                    read_only image2d_t buffer3,
                    read_only image2d_t buffer4,
                    read_only image2d_t buffer5,
                    read_only image2d_t buffer6,
                    read_only image2d_t buffer7,
                    read_only image2d_t buffer8
                      ) {

    test_2d(buffer, width, height);
}

__kernel void test_2d_10buff(write_only image2d_t buffer, uint width, 
                    uint height,
                    read_only image2d_t buffer2,
                    read_only image2d_t buffer3,
                    read_only image2d_t buffer4,
                    read_only image2d_t buffer5,
                    read_only image2d_t buffer6,
                    read_only image2d_t buffer7,
                    read_only image2d_t buffer8,
                    read_only image2d_t buffer9,
                    read_only image2d_t buffer10
                      ) {

    test_2d(buffer, width, height);
}

__kernel void test_2d_12buff(write_only image2d_t buffer, uint width, 
                    uint height,
                    read_only image2d_t buffer2,
                    read_only image2d_t buffer3,
                    read_only image2d_t buffer4,
                    read_only image2d_t buffer5,
                    read_only image2d_t buffer6,
                    read_only image2d_t buffer7,
                    read_only image2d_t buffer8,
                    read_only image2d_t buffer9,
                    read_only image2d_t buffer10,
                    read_only image2d_t buffer11,
                    read_only image2d_t buffer12
                      ) {

    test_2d(buffer, width, height);
}

__kernel void test_2d_14buff(write_only image2d_t buffer, uint width, 
                    uint height,
                    read_only image2d_t buffer2,
                    read_only image2d_t buffer3,
                    read_only image2d_t buffer4,
                    read_only image2d_t buffer5,
                    read_only image2d_t buffer6,
                    read_only image2d_t buffer7,
                    read_only image2d_t buffer8,
                    read_only image2d_t buffer9,
                    read_only image2d_t buffer10,
                    read_only image2d_t buffer11,
                    read_only image2d_t buffer12,
                    read_only image2d_t buffer13,
                    read_only image2d_t buffer14
                      ) {

    test_2d(buffer, width, height);
}

__kernel void test_2d_16buff(write_only image2d_t buffer, uint width, 
                    uint height,
                    read_only image2d_t buffer2,
                    read_only image2d_t buffer3,
                    read_only image2d_t buffer4,
                    read_only image2d_t buffer5,
                    read_only image2d_t buffer6,
                    read_only image2d_t buffer7,
                    read_only image2d_t buffer8,
                    read_only image2d_t buffer9,
                    read_only image2d_t buffer10,
                    read_only image2d_t buffer11,
                    read_only image2d_t buffer12,
                    read_only image2d_t buffer13,
                    read_only image2d_t buffer14,
                    read_only image2d_t buffer15,
                    read_only image2d_t buffer16
                      ) {

    test_2d(buffer, width, height);
}

__kernel void test_2d_18buff(write_only image2d_t buffer, uint width, 
                    uint height,
                    read_only image2d_t buffer2,
                    read_only image2d_t buffer3,
                    read_only image2d_t buffer4,
                    read_only image2d_t buffer5,
                    read_only image2d_t buffer6,
                    read_only image2d_t buffer7,
                    read_only image2d_t buffer8,
                    read_only image2d_t buffer9,
                    read_only image2d_t buffer10,
                    read_only image2d_t buffer11,
                    read_only image2d_t buffer12,
                    read_only image2d_t buffer13,
                    read_only image2d_t buffer14,
                    read_only image2d_t buffer15,
                    read_only image2d_t buffer16,
                    read_only image2d_t buffer17,
                    read_only image2d_t buffer18
                      ) {

    test_2d(buffer, width, height);
}

__kernel void test_2d_20buff(write_only image2d_t buffer, uint width, 
                    uint height,
                    read_only image2d_t buffer2,
                    read_only image2d_t buffer3,
                    read_only image2d_t buffer4,
                    read_only image2d_t buffer5,
                    read_only image2d_t buffer6,
                    read_only image2d_t buffer7,
                    read_only image2d_t buffer8,
                    read_only image2d_t buffer9,
                    read_only image2d_t buffer10,
                    read_only image2d_t buffer11,
                    read_only image2d_t buffer12,
                    read_only image2d_t buffer13,
                    read_only image2d_t buffer14,
                    read_only image2d_t buffer15,
                    read_only image2d_t buffer16,
                    read_only image2d_t buffer17,
                    read_only image2d_t buffer18,
                    read_only image2d_t buffer19,
                    read_only image2d_t buffer20
                      ) {

    test_2d(buffer, width, height);
}
