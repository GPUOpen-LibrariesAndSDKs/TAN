//
// MIT license
//
// Copyright (c) 2019 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#define __FLOAT__              float
#define __FLOAT2__             float2


void FHTIteration_gcs(__local __FLOAT__ * data,
	const __global __FLOAT__ * ang,	
	int n,
	int n2,
	int k ) {
    __FLOAT__ a;
    __FLOAT__ b;
    __FLOAT__ c;
    __FLOAT__ d;
    __FLOAT__ e;
    __FLOAT__ f;
    int i, j, ang_off;
    i = (int)((float)k / (float)(n / 2 + 1));
    j = k - mul24(i, (n / 2 + 1));

    int diff = (j == 0) ? n : j;

    __FLOAT__ flip_sign = (__FLOAT__)((j == 0) ? -1 : 1);

    ang_off = mul24(j, (n2 << 1));


    int a_off = mad24((n << 1), i, j);
    a = data[a_off];            // *A
    b = data[a_off + n];        // *B


    int c_off = mad24((n << 1), i, n - diff);
    c = data[c_off];			    // *C
    d = data[c_off + n];          // *D



    __FLOAT__ dsin = ang[ang_off];
    __FLOAT__ dcos = ang[ang_off + 1];


    e = b*dcos + d*dsin;
    f = b*dsin - d*dcos;

    f *= flip_sign;

    if (k < mad24(n2, n / 2, n2))
    {

        data[a_off + n] = a - e;		// *B
        data[a_off] = a + e;			// *A
        data[c_off + n] = c - f;      // *D
        data[c_off] = c + f;			// *C

    }

}





#undef __GROUP_SZ__
#undef N
#undef LOG2_N
#define __GROUP_SZ__ 256
#define N 4096
#define LOG2_N 12

/////////////////////////////////////////////////////
//
//
/////////////////////////////////////////////////////

__kernel 
void amdFHT4096(__global const __FLOAT__ * in,
				__global __FLOAT__ * out,
				__global const short * gbitreverse,
				__global const __FLOAT__ * gsincos,
                __global const int* channels_map,
				__FLOAT__ scaling,
				int frame_ln,
				int chnl_stride
				) 
{
	int lcl_id = get_local_id(0);
	int grp_id = get_group_id(0);
    int chnl = channels_map[get_group_id(1)];


	__local __FLOAT__ data[N];

	for( int i = lcl_id; i < N; i+= __GROUP_SZ__) {
		data[gbitreverse[i]] = in[i + grp_id * frame_ln + chnl_stride * chnl];
	}
	barrier(CLK_LOCAL_MEM_FENCE);

// transform
    int n=1;
    int n2=N/2; 
	for(int log2_n = 0, log2_n2 = LOG2_N - 1; log2_n < LOG2_N; log2_n++, log2_n2--)
    {
		n = (1 << log2_n);
		n2 = (1 << log2_n2);

		for ( int k = lcl_id; k < N; k+= __GROUP_SZ__) {

			FHTIteration_gcs(data, gsincos,	n, n2, k );
		}
        barrier(CLK_LOCAL_MEM_FENCE);
	}

	for( int i = lcl_id; i < N; i+= __GROUP_SZ__) {
		out[i + grp_id * frame_ln + chnl_stride * chnl] = data[i];
	}
/*
	if ( lcl_id == 0 && grp_id == 0 ) {
		printf("IR2048 FHT: %f %f %f %f\n", data[0], data[1], data[2], data[3]);
	}
*/
} 


__kernel 
void amdFHT4096FromQ(__global const __FLOAT__ * in,
				__global __FLOAT__ * out,
				__global const short * gbitreverse,
				__global const __FLOAT__ * gsincos,
                __global const int* channels_map,
				__FLOAT__ scale,
				int chnl_stride,
				int que_ln,
				int frame_ln,
				int current_bin
				) 
{
	int lcl_id = get_local_id(0);
    int chnl = channels_map[get_group_id(1)];
	int chnl_off  = chnl_stride * chnl;
	int bin_off, prev_bin;

	__local __FLOAT__ data[N];

#if 1
// first half
	bin_off = current_bin*frame_ln;
	for( int i = lcl_id; i < N/2; i+= __GROUP_SZ__) {
		data[gbitreverse[i]] = in[i + chnl_off + bin_off];
	}

// second half
	prev_bin = (current_bin - 1) < 0 ? que_ln - 1 : (current_bin - 1);
	bin_off = prev_bin*frame_ln;
	for( int i = lcl_id; i < N/2; i+= __GROUP_SZ__) {
		data[gbitreverse[N/2 + i]] = in[i + chnl_off + bin_off];
	}
#else
	for( int i = lcl_id; i < N; i+= __GROUP_SZ__) {
		data[i] = in[i + frame_ln * 2 * chnl];
	}
#endif

	barrier(CLK_LOCAL_MEM_FENCE);

// transform
    int n=1;
    int n2=N/2; 
	for(int log2_n = 0, log2_n2 = LOG2_N - 1; log2_n < LOG2_N; log2_n++, log2_n2--)
    {
		n = (1 << log2_n);
		n2 = (1 << log2_n2);

		for ( int k = lcl_id; k < N; k+= __GROUP_SZ__) {

			FHTIteration_gcs(data, gsincos,	n, n2, k );
		}
        barrier(CLK_LOCAL_MEM_FENCE);
	}

	for( int i = lcl_id; i < N; i+= __GROUP_SZ__) {
		out[i + frame_ln * 2 * chnl] = data[i];
	}
/*
	if ( lcl_id == 0 ) {
		printf("FHT2: %f %f %f %f\n", data[0], data[1], data[2], data[3]);
	}
*/
} 

__kernel 
void amdFHT4096FromS(__global const __FLOAT__ * in,
				__global __FLOAT__ * out,
				__global const short * gbitreverse,
				__global const __FLOAT__ * gsincos,
                __global const int* channels_map,
				__FLOAT__ scale,
				int chnl_stride,
				int frame_ln,
				int current_bin
				) 
{
	int lcl_id = get_local_id(0);
    int chnl = channels_map[get_group_id(1)];
	int chnl_off  = chnl_stride * chnl;

	__local __FLOAT__ data[N];


	for( int i = lcl_id; i < N; i+= __GROUP_SZ__) {
		data[gbitreverse[i]] = in[i + chnl_off];
	}

	barrier(CLK_LOCAL_MEM_FENCE);



// transform
    int n=1;
    int n2=N/2; 
	for(int log2_n = 0, log2_n2 = LOG2_N - 1; log2_n < LOG2_N; log2_n++, log2_n2--)
    {
		n = (1 << log2_n);
		n2 = (1 << log2_n2);

		for ( int k = lcl_id; k < N; k+= __GROUP_SZ__) {

			FHTIteration_gcs(data, gsincos,	n, n2, k );
		}
        barrier(CLK_LOCAL_MEM_FENCE);
	}

	for( int i = lcl_id; i < N/2; i+= __GROUP_SZ__) {
		out[i + (frame_ln/2)  * chnl] = (data[i] * scale * (__FLOAT__)0.5);
	}
/*
	if ( lcl_id == 0 && chnl == 0 ) {
		printf("InvFHT2: %f %f %f %f\n", out[0], out[1], out[2], out[3]);
	}
*/
} 

__kernel
void amdFHT4096FromSXFade(__global const __FLOAT__ * in,
                     __global __FLOAT__ * out,
                     __global const short * gbitreverse,
                     __global const __FLOAT__ * gsincos,
                     __global const int* channels_map,
                     __FLOAT__ scale,
                     int chnl_stride,
                     int frame_ln,
                     int current_bin, 
                     int xfade_span
)
{
    int lcl_id = get_local_id(0);
    int chnl = channels_map[get_group_id(1)];
    int chnl_off = chnl_stride * chnl;

    __local __FLOAT__ data[N];


    for (int i = lcl_id; i < N; i += __GROUP_SZ__)
    {
        data[gbitreverse[i]] = in[i + chnl_off];
    }

    barrier(CLK_LOCAL_MEM_FENCE);



    // transform
    int n = 1;
    int n2 = N / 2;
    for (int log2_n = 0, log2_n2 = LOG2_N - 1; log2_n < LOG2_N; log2_n++, log2_n2--)
    {
        n = (1 << log2_n);
        n2 = (1 << log2_n2);

        for (int k = lcl_id; k < N; k += __GROUP_SZ__)
        {

            FHTIteration_gcs(data, gsincos, n, n2, k);
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    for (int i = lcl_id; i < N / 2; i += __GROUP_SZ__)
    {
        float fadeInCoeff = (float)(i) / (float)(xfade_span - 1);
        fadeInCoeff = (fadeInCoeff > 1.0) ? 1.0 : fadeInCoeff;
        float fadeOutCoeff = 1.0 - fadeInCoeff;
        float tempOut = (data[i] * scale * (__FLOAT__)0.5);
        out[i + (frame_ln / 2)  * chnl] = tempOut*fadeInCoeff + out[i + (frame_ln / 2)  * chnl] * fadeOutCoeff;
    }
    /*
    if ( lcl_id == 0 && chnl == 0 ) {
    printf("InvFHT2: %f %f %f %f\n", out[0], out[1], out[2], out[3]);
    }
    */
}

#undef __GROUP_SZ__
#undef N
#undef LOG2_N
#define __GROUP_SZ__ 256
#define N 8192
#define LOG2_N 13

void FHTransformLoop(__local __FLOAT__  *data, __global const __FLOAT__ * gsincos) {
// transform
	int lcl_id = get_local_id(0);
    int n=1;
    int n2=N/2; 
	for(int log2_n = 0, log2_n2 = LOG2_N - 1; log2_n < LOG2_N; log2_n++, log2_n2--)
    {
		n = (1 << log2_n);
		n2 = (1 << log2_n2);

		for ( int k = lcl_id; k < N; k+= __GROUP_SZ__) {

			FHTIteration_gcs(data, gsincos,	n, n2, k );
		}
        barrier(CLK_LOCAL_MEM_FENCE);
	}
}


/////////////////////////////////////////////////////
//
//
/////////////////////////////////////////////////////

__kernel 
void amdFHT8192(__global const __FLOAT__ * in,
				__global __FLOAT__ * out,
				__global const short * gbitreverse,
				__global const __FLOAT__ * gsincos,
                __global const int* channels_map,
				__FLOAT__ scaling,
				int frame_ln,
				int chnl_stride
				) 
{
	int lcl_id = get_local_id(0);
	int grp_id = get_group_id(0);
    int chnl = channels_map[get_group_id(1)];

	__local __FLOAT__ data[N];

	for( int i = lcl_id; i < N; i+= __GROUP_SZ__) {
		data[gbitreverse[i]] = in[i + grp_id * frame_ln + chnl_stride * chnl];
	}
	barrier(CLK_LOCAL_MEM_FENCE);

	FHTransformLoop(data, gsincos);

	for( int i = lcl_id; i < N; i+= __GROUP_SZ__) {
		out[i + grp_id * frame_ln + chnl_stride * chnl] = data[i];
	}

} 

// Currently in/out i the same buffer
// care has to be taken when try to parallelize in a future.
// need to write into diff buffer


__kernel
void amdFHT8192FromQ(__global const __FLOAT__ * in,
                     __global __FLOAT__ * out,
                     __global const short * gbitreverse,
                     __global const __FLOAT__ * gsincos,
                     __global const int* channels_map,
                     __FLOAT__ scale,
                     int chnl_stride,
                     int que_ln,
                     int frame_ln,
                     int current_bin
)
{
    int lcl_id = get_local_id(0);
    int chnl = channels_map[get_group_id(1)];
    int chnl_off = chnl_stride * chnl;
    int bin_off, prev_bin;

    __local __FLOAT__ data[N];

#if 1
    // first half
    bin_off = current_bin*frame_ln;
    for (int i = lcl_id; i < N / 2; i += __GROUP_SZ__)
    {
        data[gbitreverse[i]] = in[i + chnl_off + bin_off];
    }

    // second half
    prev_bin = (current_bin - 1) < 0 ? que_ln - 1 : (current_bin - 1);
    bin_off = prev_bin*frame_ln;
    for (int i = lcl_id; i < N / 2; i += __GROUP_SZ__)
    {
        data[gbitreverse[N / 2 + i]] = in[i + chnl_off + bin_off];
    }
#else
    for (int i = lcl_id; i < N; i += __GROUP_SZ__)
    {
        data[i] = in[i + frame_ln * 2 * chnl];
    }
#endif

    barrier(CLK_LOCAL_MEM_FENCE);

    // transform
    int n = 1;
    int n2 = N / 2;
    for (int log2_n = 0, log2_n2 = LOG2_N - 1; log2_n < LOG2_N; log2_n++, log2_n2--)
    {
        n = (1 << log2_n);
        n2 = (1 << log2_n2);

        for (int k = lcl_id; k < N; k += __GROUP_SZ__)
        {

            FHTIteration_gcs(data, gsincos, n, n2, k);
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    for (int i = lcl_id; i < N; i += __GROUP_SZ__)
    {
        out[i + frame_ln * 2 * chnl] = data[i];
    }
    /*
    if ( lcl_id == 0 ) {
    printf("FHT2: %f %f %f %f\n", data[0], data[1], data[2], data[3]);
    }
    */
}

__kernel 
void amdFHT8192FromS(__global const __FLOAT__ * in,
				__global __FLOAT__ * out,
				__global const short * gbitreverse,
				__global const __FLOAT__ * gsincos,
                __global const int* channels_map,
				__FLOAT__ scale,
				int chnl_stride,
				int frame_ln,
				int current_bin
				) 
{
	int lcl_id = get_local_id(0);
    int chnl = channels_map[get_group_id(1)];
	int chnl_off  = chnl_stride * chnl;

	__local __FLOAT__ data[N];


	for( int i = lcl_id; i < N; i+= __GROUP_SZ__) {
		data[gbitreverse[i]] = in[i + chnl_off];
	}

	barrier(CLK_LOCAL_MEM_FENCE);

	FHTransformLoop(data, gsincos);

	for( int i = lcl_id; i < N/2; i+= __GROUP_SZ__) {
		out[i + (frame_ln/2)  * chnl] = (data[i] * scale * (__FLOAT__)0.5);
	}

} 


__kernel
void amdFHT8192FromSXFade(__global const __FLOAT__ * in,
                     __global __FLOAT__ * out,
                     __global const short * gbitreverse,
                     __global const __FLOAT__ * gsincos,
                     __global const int* channels_map,
                     __FLOAT__ scale,
                     int chnl_stride,
                     int frame_ln,
                     int current_bin, 
                     int xfade_span
)
{
    int lcl_id = get_local_id(0);
    int chnl = channels_map[get_group_id(1)];
    int chnl_off = chnl_stride * chnl;

    __local __FLOAT__ data[N];


    for (int i = lcl_id; i < N; i += __GROUP_SZ__)
    {
        data[gbitreverse[i]] = in[i + chnl_off];
    }

    barrier(CLK_LOCAL_MEM_FENCE);

    FHTransformLoop(data, gsincos);

    for (int i = lcl_id; i < N / 2; i += __GROUP_SZ__)
    {
        float fadeInCoeff = (float)(i) / (float)(xfade_span - 1);
        fadeInCoeff = (fadeInCoeff > 1.0) ? 1.0 : fadeInCoeff;
        float fadeOutCoeff = 1.0 - fadeInCoeff;
        float tempOut = (data[i] * scale * (__FLOAT__)0.5);
        out[i + (frame_ln / 2)  * chnl] = tempOut*fadeInCoeff + out[i + (frame_ln / 2)  * chnl] * fadeOutCoeff;
    }

}