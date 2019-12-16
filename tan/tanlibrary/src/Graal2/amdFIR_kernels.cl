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
/////////////////////////////////////////////////////
//
//
/////////////////////////////////////////////////////

__kernel 
void amdFIR(__global const __FLOAT__ * in,
			__global const __FLOAT__ * fir,
			__global __FLOAT__ * out,
			int fir_sz,
			int block_sz,
			int in_index,
			int in_chnl_stride,
			int fir_chnl_stride,
			int out_index,
			int out_chnl_stride
				) 
{
	int lcl_id = get_local_id(0);
	int grp_id = get_group_id(0);
	int chnl = get_group_id(1);
	__local __FLOAT__ stage[64 + 32];
	stage[lcl_id] = 0;
	stage[64 + lcl_id/2] = 0;
	barrier(CLK_LOCAL_MEM_FENCE);
	int data_chnl_off = mul24(in_chnl_stride, chnl);
	int fir_chnl_off = mul24(fir_chnl_stride, chnl); 
	int fir_index, data_index;
	int fir_off, data_off;
// MAD
    __FLOAT__ sum = 0;
	for (fir_index = lcl_id, data_index = grp_id - lcl_id + mul24(in_index, block_sz); fir_index < fir_sz; fir_index += 64, data_index -= 64) {
		data_index = (data_index < 0 ) ? in_chnl_stride + data_index : data_index;
		fir_off = fir_chnl_off + fir_index;
		data_off = data_chnl_off + data_index; 
		sum += fir[fir_off] * in[data_off];
#if 0
		if ( fir_index <= 1 && grp_id == 1 && chnl == 0) {
			printf("%d %d %d %f %f %f\n", grp_id, fir_index, data_index, sum, fir[fir_off], in[data_off]);
		}
#endif

	}

// REDUCTION
	stage[lcl_id] = sum;
	barrier(CLK_LOCAL_MEM_FENCE);
	for(int i = 32; i > 0; i >>= 1 ) {
			sum += stage[lcl_id + i];
			stage[lcl_id] = sum;
			barrier(CLK_LOCAL_MEM_FENCE);
#if 0
		if ( lcl_id == 0 && grp_id == 0 ) {
			printf("%d %f\n", lcl_id, stage[lcl_id]);
		}
#endif

	}

// our filtered sample
	int out_off = mad24(out_chnl_stride, chnl, out_index);
	if ( lcl_id == 0 ) {
		out[out_off+ grp_id] = sum;
	}
} 
