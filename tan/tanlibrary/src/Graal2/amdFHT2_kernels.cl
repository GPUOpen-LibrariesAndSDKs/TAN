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


////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////
__kernel 
void amdFromQReverse(__global const __FLOAT__ * in,
				__global __FLOAT__ * out,
				__global const short * gbitreverse,
                __global const int* channels_map,
				int chnl_stride,
				int que_ln,
				int frame_ln,
				int current_bin )  {
	int glbl_id = get_global_id(0);
	int chnl = channels_map[get_group_id(1)];
	int chnl_off  = chnl_stride * chnl;
	int bin = (glbl_id < get_global_size(0)/2) ? current_bin : (current_bin - 1);
	int in_id = (glbl_id < get_global_size(0)/2) ? glbl_id : glbl_id - get_global_size(0)/2;
	bin = (bin < 0) ? que_ln - 1 : bin;

	int bin_off = bin*frame_ln;
	out[gbitreverse[glbl_id] + get_global_size(0) * chnl] = in[in_id + chnl_off + bin_off];
	
}
////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////
__kernel 
void amdFromSReverse(__global const __FLOAT__ * in,
				__global __FLOAT__ * out,
				__global const short * gbitreverse,
                __global const  int* channels_map,
				int chnl_stride ) {
	int glbl_id = get_global_id(0);
	int chnl = channels_map[get_group_id(1)];
	int chnl_off  = chnl_stride * chnl;


	out[gbitreverse[glbl_id] + chnl_off] = in[glbl_id + chnl_off];
}



void FHTIterationGlbl(__global __FLOAT__ * data,
	const __global __FLOAT__ * ang,	
	int n,
	int n2,
	int k,
	int limit ) {
	__global __FLOAT__ * A;
	__global __FLOAT__ * B;
	__global __FLOAT__ * C;
	__global __FLOAT__ * D;
	__FLOAT__ a;
	__FLOAT__ b;
	__FLOAT__ c;
	__FLOAT__ d;
	__FLOAT__ e;
	__FLOAT__ f;
	int i, j, ang_off;

			i = (int)( (float)k / (float)(n /2 + 1));
			j =  k - mul24(i , (n/2 + 1));

			int diff = (j==0) ? n : j;

			__FLOAT__ flip_sign = (__FLOAT__)((j==0)? -1 : 1);

			ang_off = mul24(j ,(n2 <<1));


			A = &data[mul24((n<<1),i)	+ j];
			B = &data[mul24((n<<1),i) + n + j];


			C=&data[mul24((n<<1),i) + n - diff];
			D=&data[mul24((n<<1),i) + (n<<1) - diff];
	
         
			__FLOAT__ dsin=ang[ang_off];
			__FLOAT__ dcos=ang[ang_off + 1];

			a = *A;
			b = *B;
			c = *C;
			d = *D;

		    barrier(CLK_GLOBAL_MEM_FENCE);

            e = b*dcos + d*dsin;
            f = b*dsin - d*dcos;

			f *= flip_sign;
			if ( k < limit ) {
				*B = a-e;
				*A = a+e;
				*D = c-f;
				*C = c+f;
			}
		    barrier(CLK_GLOBAL_MEM_FENCE);
}


__kernel 
void amdFHTGlbl(__global __FLOAT__ * in_out,
				__global const __FLOAT__ * gsincos,
//				__global uint * gsync,
				__FLOAT__ scale,
				int chnl_stride,
				int log2N,
				int log2_n
//				int sync_pattern,
//				int dummy_0
				)
 {
	int k = get_global_id(0);
	int lcl_id = get_local_id(0);
	int chnl = get_group_id(1);
	int chnl_off  = chnl_stride * chnl;
    int n, n2;
 //   for(int log2_n = 0; log2_n < log2N; log2_n++)
    {
		int log2_n2 = log2N - 1 - log2_n;
		n = (1 << log2_n);
		n2 = (1 << log2_n2);
		int limit = mad24(n2 , n/2, n2);
		FHTIterationGlbl(in_out + chnl_off, gsincos, n, n2, k, limit );
/*
		uint lsync;
		if ( lcl_id == 0 ) {
			lsync = atomic_add(&gsync[chnl], 1);
			printf("%d %d\n", get_group_id(0), lsync);
			while( lsync < 2 ) { //((lsync >> sync_pattern) << sync_pattern) != lsync && lsync > 0 ) {
				lsync = atom_add(&gsync[chnl], dummy_0);
			}
			printf("%d %d\n", get_group_id(0), lsync);
			return;
		}
*/
	}
}

void FHTIteration(__local __FLOAT__ * data,
                  __global const __FLOAT__ * ang,
                  int n,
                  int n2,
                  int k)
{
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

/////////////////////////////////////////////////////
//
//
/////////////////////////////////////////////////////



#undef __GROUP_SZ__
#undef N
#undef LOG2_N
#define __GROUP_SZ__ 32
#define N 64
#define LOG2_N 6
__kernel
void amdFHT64FromS(__global const __FLOAT__ * in,
                    __global __FLOAT__ * out,
                    __global const short * gbitreverse,
                    __global const __FLOAT__ * gsincos,
                   __global const int* channels_map,
                    __FLOAT__ scale,
                    int chnl_stride,
                    int frame_ln,
                    int current_bin,
                    __global const __FLOAT__ *out2,
                    int index2,
                    int chnl_stride2
)
{
    int lcl_id = get_local_id(0);
    int chnl = channels_map[get_group_id(1)];
    int chnl_off = chnl_stride * chnl;

    __local __FLOAT__ data[N];
    for (int i = lcl_id; i < N; i += __GROUP_SZ__)
    {
        //data[l_sbitrevers[i]] = in[i + chnl_off];
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

            FHTIteration(data, gsincos, n, n2, k);
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    int out2_off = mul24(chnl_stride2, chnl) + mul24(index2, N / 2);
    for (int i = lcl_id; i < N / 2; i += __GROUP_SZ__)
    {
        out[i + (frame_ln / 2)  * chnl] = (data[i] * scale * (__FLOAT__)0.5) + out2[out2_off + i];
    }

}

__kernel
void amdFHT64FromSXFade(__global const __FLOAT__ * in1,
                         __global __FLOAT__ * out,
                         __global const short * gbitreverse,
                         __global const __FLOAT__ * gsincos,
                         __global const int* channels_map,
                         __FLOAT__ scale,
                         int chnl_stride,
                         int frame_ln,
                         int current_bin,
                         __global const __FLOAT__ * in2,
                         __global const __FLOAT__ *out2,
                         int index2,
                         int chnl_stride2
)
{
    int lcl_id = get_local_id(0);
    int chnl = channels_map[get_group_id(1)];
    int chnl_off = chnl_stride * chnl;

    __local __FLOAT__ data1[N];
    __local __FLOAT__ data2[N];
    for (int i = lcl_id; i < N; i += __GROUP_SZ__)
    {
        data1[gbitreverse[i]] = in1[i + chnl_off];
        data2[gbitreverse[i]] = in2[i + chnl_off];
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
            FHTIteration(data1, gsincos, n, n2, k);
            FHTIteration(data2, gsincos, n, n2, k);
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }
    int out2_off = mul24(chnl_stride2, chnl) + mul24(index2, N / 2);
    for (int i = lcl_id; i < N / 2; i += __GROUP_SZ__)
    {
        float fadeInCoeff = (float)i / (float)(N / 2 - 1);
        float fadeOutCoeff = 1.0 - fadeInCoeff;
        out[i + (frame_ln / 2)  * chnl] = ((fadeInCoeff*data1[i] + fadeOutCoeff*data2[i])* scale * (__FLOAT__)0.5) + out2[out2_off + i];
    }
}

/////////////////////////////////////////////////////
//
//
/////////////////////////////////////////////////////




#define __GROUP_SZ__ 64
#define N 128
#define LOG2_N 7
__kernel 
void amdFHT128FromS(__global const __FLOAT__ * in,
				__global __FLOAT__ * out,
				__global const short * gbitreverse,
				__global const __FLOAT__ * gsincos,
                __global const int* channels_map,
				__FLOAT__ scale,
				int chnl_stride,
				int frame_ln,
				int current_bin,
				__global const __FLOAT__ *out2,
				int index2,
				int chnl_stride2
				) 
{
    int lcl_id = get_local_id(0);
    int chnl = channels_map[get_group_id(1)];
    int chnl_off = chnl_stride * chnl;

    __local __FLOAT__ data[N];
    for (int i = lcl_id; i < N; i += __GROUP_SZ__)
    {
        //data[l_sbitrevers[i]] = in[i + chnl_off];
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

            FHTIteration(data, gsincos, n, n2, k);
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }

	int out2_off = mul24(chnl_stride2, chnl) + mul24(index2, N/2) ;
	for( int i = lcl_id; i < N/2; i+= __GROUP_SZ__) {
		out[i + (frame_ln/2)  * chnl] = (data[i] * scale * (__FLOAT__)0.5) + out2[out2_off + i];
	}

} 

__kernel
void amdFHT128FromSXFade(__global const __FLOAT__ * in1,
                         __global __FLOAT__ * out,
                         __global const short * gbitreverse,
                         __global const __FLOAT__ * gsincos,
                         __global const int* channels_map,
                         __FLOAT__ scale,
                         int chnl_stride,
                         int frame_ln,
                         int current_bin,
                         __global const __FLOAT__ * in2,
                         __global const __FLOAT__ *out2,
                         int index2,
                         int chnl_stride2
)
{
    int lcl_id = get_local_id(0);
    int chnl = channels_map[get_group_id(1)];
    int chnl_off = chnl_stride * chnl;

    __local __FLOAT__ data1[N];
    __local __FLOAT__ data2[N];
    for (int i = lcl_id; i < N; i += __GROUP_SZ__)
    {
        data1[gbitreverse[i]] = in1[i + chnl_off];
        data2[gbitreverse[i]] = in2[i + chnl_off];
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
            FHTIteration(data1, gsincos, n, n2, k);
            FHTIteration(data2, gsincos, n, n2, k);
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }
    int out2_off = mul24(chnl_stride2, chnl) + mul24(index2, N / 2);
    for (int i = lcl_id; i < N / 2; i += __GROUP_SZ__)
    {
        float fadeInCoeff = (float)i / (float)(N / 2 - 1);
        float fadeOutCoeff = 1.0 - fadeInCoeff;
        out[i + (frame_ln / 2)  * chnl] = ((fadeInCoeff*data1[i] + fadeOutCoeff*data2[i])* scale * (__FLOAT__)0.5) + out2[out2_off + i];
    }
}

#undef __GROUP_SZ__
#undef N
#undef LOG2_N
#define __GROUP_SZ__ 128
#define N 256
#define LOG2_N 8
__kernel
void amdFHT256FromS(__global const __FLOAT__ * in,
                    __global __FLOAT__ * out,
                    __global const short * gbitreverse,
                    __global const __FLOAT__ * gsincos,
                    __global const int* channels_map,
                    __FLOAT__ scale,
                    int chnl_stride,
                    int frame_ln,
                    int current_bin,
                    __global const __FLOAT__ *out2,
                    int index2,
                    int chnl_stride2
)
{
    int lcl_id = get_local_id(0);
    int chnl = channels_map[get_group_id(1)];
    int chnl_off = chnl_stride * chnl;

    __local __FLOAT__ data[N];
    for (int i = lcl_id; i < N; i += __GROUP_SZ__)
    {
        //data[l_sbitrevers[i]] = in[i + chnl_off];
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

            FHTIteration(data, gsincos, n, n2, k);
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    int out2_off = mul24(chnl_stride2, chnl) + mul24(index2, N / 2);
    for (int i = lcl_id; i < N / 2; i += __GROUP_SZ__)
    {
        out[i + (frame_ln / 2)  * chnl] = (data[i] * scale * (__FLOAT__)0.5) + out2[out2_off + i];
    }

}

__kernel
void amdFHT256FromSXFade(__global const __FLOAT__ * in1,
                         __global __FLOAT__ * out,
                         __global const short * gbitreverse,
                         __global const __FLOAT__ * gsincos,
                         __global const int* channels_map,
                         __FLOAT__ scale,
                         int chnl_stride,
                         int frame_ln,
                         int current_bin,
                         __global const __FLOAT__ * in2,
                         __global const __FLOAT__ *out2,
                         int index2,
                         int chnl_stride2
)
{
    int lcl_id = get_local_id(0);
    int chnl = channels_map[get_group_id(1)];
    int chnl_off = chnl_stride * chnl;

    __local __FLOAT__ data1[N];
    __local __FLOAT__ data2[N];
    for (int i = lcl_id; i < N; i += __GROUP_SZ__)
    {
        data1[gbitreverse[i]] = in1[i + chnl_off];
        data2[gbitreverse[i]] = in2[i + chnl_off];
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
            FHTIteration(data1, gsincos, n, n2, k);
            FHTIteration(data2, gsincos, n, n2, k);
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }
    int out2_off = mul24(chnl_stride2, chnl) + mul24(index2, N / 2);
    for (int i = lcl_id; i < N / 2; i += __GROUP_SZ__)
    {
        float fadeInCoeff = (float)i / (float)(N / 2 - 1);
        float fadeOutCoeff = 1.0 - fadeInCoeff;
        out[i + (frame_ln / 2)  * chnl] = ((fadeInCoeff*data1[i] + fadeOutCoeff*data2[i])* scale * (__FLOAT__)0.5) +out2[out2_off + i];
    }
}


#undef __GROUP_SZ__
#undef N
#undef LOG2_N
#define __GROUP_SZ__ 256
#define N 512
#define LOG2_N 9

__kernel 
void amdFHT512FromS(__global const __FLOAT__ * in,
				__global __FLOAT__ * out,
				__global const short * gbitreverse,
				__global const __FLOAT__ * gsincos,
                __global const int* channels_map,
				__FLOAT__ scale,
				int chnl_stride,
				int frame_ln,
				int current_bin,
				__global const __FLOAT__ *out2,
				int index2,
				int chnl_stride2
				) 
{
    int lcl_id = get_local_id(0);
    int chnl = channels_map[get_group_id(1)];
    int chnl_off = chnl_stride * chnl;

    __local __FLOAT__ data[N];
    for (int i = lcl_id; i < N; i += __GROUP_SZ__)
    {
        //data[l_sbitrevers[i]] = in[i + chnl_off];
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

            FHTIteration(data, gsincos, n, n2, k);
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }
	int out2_off = mul24(chnl_stride2, chnl) + mul24(index2, N/2) ;
	for( int i = lcl_id; i < N/2; i+= __GROUP_SZ__) {
		out[i + (frame_ln/2)  * chnl] = (data[i] * scale * (__FLOAT__)0.5) + out2[out2_off + i];
	}
} 

__kernel
void amdFHT512FromSXFade(__global const __FLOAT__ * in1,
                    __global __FLOAT__ * out,
                    __global const short * gbitreverse,
                    __global const __FLOAT__ * gsincos,
                    __global const int* channels_map,
                    __FLOAT__ scale,
                    int chnl_stride,
                    int frame_ln,
                    int current_bin,
                    __global const __FLOAT__ * in2,
                    __global const __FLOAT__ *out2,
                    int index2,
                    int chnl_stride2
)
{
    int lcl_id = get_local_id(0);
    int chnl = channels_map[get_group_id(1)];
    int chnl_off = chnl_stride * chnl;

    __local __FLOAT__ data1[N];
    __local __FLOAT__ data2[N];
    for (int i = lcl_id; i < N; i += __GROUP_SZ__)
    {
        data1[gbitreverse[i]] = in1[i + chnl_off];
        data2[gbitreverse[i]] = in2[i + chnl_off];
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
            FHTIteration(data1, gsincos, n, n2, k);
            FHTIteration(data2, gsincos, n, n2, k);
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }
    int out2_off = mul24(chnl_stride2, chnl) + mul24(index2, N / 2);
    for (int i = lcl_id; i < N / 2; i += __GROUP_SZ__)
    {
        float fadeInCoeff = (float)i / (float)(N / 2 - 1);
        float fadeOutCoeff = 1.0 - fadeInCoeff;
        out[i + (frame_ln / 2)  * chnl] = ((fadeInCoeff*data1[i] + fadeOutCoeff*data2[i])* scale * (__FLOAT__)0.5) + out2[out2_off + i];
    }
}



#undef __GROUP_SZ__
#undef N
#undef LOG2_N
#define __GROUP_SZ__ 256
#define N 1024
#define LOG2_N 10
__kernel 
void amdFHT1024FromS(__global const __FLOAT__ * in,
				__global __FLOAT__ * out,
				__global const short * gbitreverse,
				__global const __FLOAT__ * gsincos,
                __global const int* channels_map,
				__FLOAT__ scale,
				int chnl_stride,
				int frame_ln,
				int current_bin,
				__global const __FLOAT__ *out2,
				int index2,
				int chnl_stride2
				) 
{
    int lcl_id = get_local_id(0);
    int chnl = channels_map[get_group_id(1)];
    int chnl_off = chnl_stride * chnl;

    __local __FLOAT__ data[N];
    for (int i = lcl_id; i < N; i += __GROUP_SZ__)
    {
        //data[l_sbitrevers[i]] = in[i + chnl_off];
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

            FHTIteration(data, gsincos, n, n2, k);
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }
	int out2_off = mul24(chnl_stride2, chnl) + mul24(index2, N/2) ;
	for( int i = lcl_id; i < N/2; i+= __GROUP_SZ__) {
		out[i + (frame_ln/2)  * chnl] = (data[i] * scale * (__FLOAT__)0.5) + out2[out2_off + i];
	}

} 
__kernel
void amdFHT1024FromSXFade(__global const __FLOAT__ * in1,
                         __global __FLOAT__ * out,
                         __global const short * gbitreverse,
                         __global const __FLOAT__ * gsincos,
                         __global const int* channels_map,
                         __FLOAT__ scale,
                         int chnl_stride,
                         int frame_ln,
                         int current_bin,
                         __global const __FLOAT__ * in2,
                         __global const __FLOAT__ *out2,
                         int index2,
                         int chnl_stride2
)
{
    int lcl_id = get_local_id(0);
    int chnl = channels_map[get_group_id(1)];
    int chnl_off = chnl_stride * chnl;

    __local __FLOAT__ data1[N];
    __local __FLOAT__ data2[N];
    for (int i = lcl_id; i < N; i += __GROUP_SZ__)
    {
        data1[gbitreverse[i]] = in1[i + chnl_off];
        data2[gbitreverse[i]] = in2[i + chnl_off];
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
            FHTIteration(data1, gsincos, n, n2, k);
            FHTIteration(data2, gsincos, n, n2, k);
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }
    int out2_off = mul24(chnl_stride2, chnl) + mul24(index2, N / 2);
    for (int i = lcl_id; i < N / 2; i += __GROUP_SZ__)
    {
        float fadeInCoeff = (float)i / (float)(N / 2 - 1);
        float fadeOutCoeff = 1.0 - fadeInCoeff;
        out[i + (frame_ln / 2)  * chnl] = ((fadeInCoeff*data1[i] + fadeOutCoeff*data2[i])* scale * (__FLOAT__)0.5) + out2[out2_off + i];
    }
}
#undef __GROUP_SZ__
#undef N
#undef LOG2_N
#define __GROUP_SZ__ 256
#define N 2048
#define LOG2_N 11
__kernel 
void amdFHT2048FromS(__global const __FLOAT__ * in,
				__global __FLOAT__ * out,
				__global const short * gbitreverse,
				__global const __FLOAT__ * gsincos,
                __global const int* channels_map,
				__FLOAT__ scale,
				int chnl_stride,
				int frame_ln,
				int current_bin,
				__global const __FLOAT__ *out2,
				int index2,
				int chnl_stride2
				) 
{
    int lcl_id = get_local_id(0);
    int chnl = channels_map[get_group_id(1)];
    int chnl_off = chnl_stride * chnl;

    __local __FLOAT__ data[N];
    for (int i = lcl_id; i < N; i += __GROUP_SZ__)
    {
        //data[l_sbitrevers[i]] = in[i + chnl_off];
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

            FHTIteration(data, gsincos, n, n2, k);
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }

	int out2_off = mul24(chnl_stride2, chnl) + mul24(index2, N/2) ;
	for( int i = lcl_id; i < N/2; i+= __GROUP_SZ__) {
		out[i + (frame_ln/2)  * chnl] = (data[i] * scale * (__FLOAT__)0.5) + out2[out2_off + i];
	}

} 

__kernel
void amdFHT2048FromSXFade(__global const __FLOAT__ * in1,
                         __global __FLOAT__ * out,
                         __global const short * gbitreverse,
                         __global const __FLOAT__ * gsincos,
                         __global const int* channels_map,
                         __FLOAT__ scale,
                         int chnl_stride,
                         int frame_ln,
                         int current_bin,
                         __global const __FLOAT__ * in2,
                         __global const __FLOAT__ *out2,
                         int index2,
                         int chnl_stride2
)
{
    int lcl_id = get_local_id(0);
    int chnl = channels_map[get_group_id(1)];
    int chnl_off = chnl_stride * chnl;
    __local __FLOAT__ data1[N];
    __local __FLOAT__ data2[N];
    for (int i = lcl_id; i < N; i += __GROUP_SZ__)
    {
        data1[gbitreverse[i]] = in1[i + chnl_off];
        data2[gbitreverse[i]] = in2[i + chnl_off];
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
            FHTIteration(data1, gsincos, n, n2, k);
            FHTIteration(data2, gsincos, n, n2, k);
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }
    int out2_off = mul24(chnl_stride2, chnl) + mul24(index2, N / 2);
    for (int i = lcl_id; i < N / 2; i += __GROUP_SZ__)
    {
        float fadeInCoeff = (float)i / (float)(N / 2 - 1);
        float fadeOutCoeff = 1.0 - fadeInCoeff;
        out[i + (frame_ln / 2)  * chnl] = ((fadeInCoeff*data1[i] + fadeOutCoeff*data2[i])* scale * (__FLOAT__)0.5) + out2[out2_off + i];
    }
}
