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

//#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#pragma OPENCL EXTENSION cl_amd_printf : enable
#pragma OPENCL EXTENSION cl_amd_fp64 : enable

#define __FLOAT__              float
#define __FLOAT2__             float2

//It will copy date from the source that is blockLenth long
//to the out, the pad it with padLegnth zeros.  Offsets and
//in/out lengths are to allow for wrap around
__kernel
void amdPadFFTBlock(__global const float * in,
				__global float * out,
				__global const int * inOffset,
				int inLength,
				__global const int * outOffset,
				int outLength,
				int blockLength,
				int padLength, 
				int channelCount)  {
	int glbl_id = get_global_id(0);
	int chnl_id = get_global_id(1);
	if(chnl_id >= channelCount){ return; }
	int blockNumber = (glbl_id) / (blockLength + padLength);
	int inIndex = ((glbl_id + inOffset[chnl_id] - blockNumber * padLength) % inLength);
	int outIndex = ((glbl_id + outOffset[chnl_id]) % outLength);

	bool isPad = (glbl_id % (blockLength + padLength)) >= blockLength;
	
	out[outIndex] = isPad ? 0 : in[inIndex];
}

#if 0
void FHTIterationG(__local __FLOAT__ * data,
	__constant __FLOAT__ * ang,	
	int n,
	int n2,
	int k ) {
	__local __FLOAT__ * A;
	__local __FLOAT__ * B;
	__local __FLOAT__ * C;
	__local __FLOAT__ * D;
	__FLOAT__ a;
	__FLOAT__ b;
	__FLOAT__ c;
	__FLOAT__ d;
	__FLOAT__ e;
	__FLOAT__ f;
	int i, j, ang_off;
		if (k < mad24(n2 , n/2, n2)) {

			i = (int)( (float)k / (float)(n /2 + 1));
			j =  k - mul24(i , (n/2 + 1));

			int diff = (j==0) ? n : j;

			__FLOAT__ flip_sign = (__FLOAT__)((j==0)? -1 : 1);

			ang_off = mul24(j ,(n2 <<1));


			A = &data[mad24((n<<1),i, j)];
			B = &data[mad24((n<<1),i, n + j)];


			C=&data[mad24((n<<1),i, n - diff)];
			D=&data[mad24((n<<1),i, (n<<1) - diff)];
	
		
			__FLOAT__ dsin=ang[ang_off];
			__FLOAT__ dcos=ang[ang_off + 1];

			a = *A;
			b = *B;
			c = *C;
			d = *D;

			e = b*dcos + d*dsin;
			f = b*dsin - d*dcos;

			f *= flip_sign;



				*B = a-e;
				*A = a+e;
				*D = c-f;
				*C = c+f;
			}

			barrier(CLK_LOCAL_MEM_FENCE);

}
#else

void FHTIterationG(__local __FLOAT__ * data,
	__constant __FLOAT__ * ang,	
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
//	if (k < mad24(n2 , n/2, n2))
	{

			i = (int)( (float)k / (float)(n /2 + 1));
			j =  k - mul24(i , (n/2 + 1));

			int diff = (j==0) ? n : j;

			__FLOAT__ flip_sign = (__FLOAT__)((j==0)? -1 : 1);

			ang_off = mul24(j ,(n2 <<1));

			int a_off = mad24((n<<1),i, j);
			a = data[a_off];            // *A
			b = data[a_off + n];        // *B

			int c_off = mad24((n<<1),i, n - diff);
			c=data[c_off];			    // *C
			d=data[c_off + n];          // *D
			
			__FLOAT__ dsin=ang[ang_off];
			__FLOAT__ dcos=ang[ang_off + 1];


			e = b*dcos + d*dsin;
			f = b*dsin - d*dcos;

			f *= flip_sign;


			data[a_off + n] = a-e;		// *B
			data[a_off] = a+e;			// *A
			data[c_off + n] = c-f;      // *D
			data[c_off] = c+f;			// *C
	}

//	barrier(CLK_LOCAL_MEM_FENCE);

}

void FHTIterationG2(__local char * data,
	__constant char * ang,	
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
//	if (k < mad24(n2 , n/2, n2))
	{

			i = (int)( (float)k / (float)(n /2 + 1));
			j =  k - mul24(i , (n/2 + 1));

			int diff = (j==0) ? n : j;

			__FLOAT__ flip_sign = (__FLOAT__)((j==0)? -1 : 1);

			ang_off = mul24(j ,(n2 <<1));

			int a_off = mad24((n<<1),i, j);
			a = *(__local float*)&data[(a_off << 2)];            // *A
			b = *(__local float*)&data[(a_off + n) << 2];        // *B

			int c_off = mad24((n<<1),i, n - diff);
			c=*(__local float*)&data[(c_off) << 2];			    // *C
			d=*(__local float*)&data[(c_off + n) << 2];          // *D
			
			__FLOAT__ dsin=*(__constant float*)&ang[(ang_off) << 2];
			__FLOAT__ dcos=*(__constant float*)&ang[(ang_off + 1) << 2];


			e = b*dcos + d*dsin;
			f = b*dsin - d*dcos;

			f *= flip_sign;


			*(__local float*)&data[(a_off + n) << 2] = a-e;		// *B
			*(__local float*)&data[(a_off) << 2] = a+e;			// *A
			*(__local float*)&data[(c_off + n) << 2] = c-f;      // *D
			*(__local float*)&data[(c_off) << 2] = c+f;			// *C
	}

//	barrier(CLK_LOCAL_MEM_FENCE);

}

#endif

//#define _GROUP_SZ 256
//#define _N 2048
//#define _LOG2_N 11
#define _N_TO_READ (1 << (_K0_LOG2_N - _K0_LOG2_GROUP_SZ))



/////////////////////////////////////////////////////
//
//
/////////////////////////////////////////////////////

__kernel
void amdFHTUploadConv(__global const char * in,
				__global __FLOAT__ * out,
				__constant short * gbitreverse,
				__constant char * gsincos,
				int kern_ln,
				int in_chnl_stride,
				int out_chnl_stride
				)
{
	int lcl_id = get_local_id(0);
	int grp_id = get_group_id(0);
	int chnl = get_group_id(1);


	__local char data[_K0_N<<2];

	for( int i = lcl_id; i < _K0_N; i+= _K0_GROUP_SZ) {
		*(__local float*)&data[i<<2] = 0;
	}

	barrier(CLK_LOCAL_MEM_FENCE);

// read data with bit reverse
// second half shouled be padded with 0
	for( int i = lcl_id, k = lcl_id + grp_id * _K0_N/2; i < _K0_N/2 && k < kern_ln; i+= _K0_GROUP_SZ, k+= _K0_GROUP_SZ) {
		*(__local float*)&data[gbitreverse[i]<<2] = *(__global float*)&in[k<<2];

	}
	barrier(CLK_LOCAL_MEM_FENCE);

	int n=1;
	int n2=_K0_N/2;
	for(int log2_n = 0, log2_n2 = _K0_LOG2_N - 1; log2_n < _K0_LOG2_N; log2_n++, log2_n2--)
	{
		n = (1 << log2_n);
		n2 = (1 << log2_n2);

		for ( int k = lcl_id; (k < mad24(n2 , n/2, n2)); k+= _K0_GROUP_SZ) {

			FHTIterationG2(data, gsincos,	n, n2, k );

		}
		barrier(CLK_LOCAL_MEM_FENCE);

	}

	for( int i = lcl_id; i < _K0_N; i+= _K0_GROUP_SZ) {
		out[i + grp_id * _K0_N] = *(__local float*)&data[i<<2];

	}

}


/////////////////////////////////////////////////////
//
//
/////////////////////////////////////////////////////



__kernel
void amdFHTConvIn(__global const char * in,
				__global __FLOAT__ * out,
				__constant short * gbitreverse,
				__constant char * gsincos,
				__constant uint * channels_map,
				__constant uint * versions_map,
				__constant uint * lens_map,
				__global uint * rount_counters,
				__global float* history_data,
				uint in_version_stride,
				uint in_chnl_stride,
				uint out_version_stride,
				uint out_chnl_stride,
				uint version_stride,
				uint data_version_stride,
				uint data_channel_stride
				)
{
	uint lcl_id = get_local_id(0);
	uint glb_id = get_global_id(0);
	uint grp_id = get_group_id(0);
	uint chnl = get_group_id(1);
	uint chnl_id = channels_map[chnl];
	uint upload_id = versions_map[chnl];
	uint kern_ln = lens_map[mad24(version_stride,upload_id, chnl_id)];

	__local char data[_K0_N<<2];

	for( int i = lcl_id; i < _K0_N; i+= _K0_GROUP_SZ) {
		*(__local float*)&data[i<<2] = 0;
	}

	barrier(CLK_LOCAL_MEM_FENCE);

// read data with bit reverse
// second half shouled be padded with 0
	uint in_off = upload_id * in_version_stride + chnl_id * in_chnl_stride;
	for( uint i = lcl_id, k = lcl_id + grp_id * _K0_N/2; i < _K0_N/2 && k < kern_ln; i+= _K0_GROUP_SZ, k+= _K0_GROUP_SZ) {
		*(__local float*)&data[gbitreverse[i]<<2] = *(__global float*)&in[(in_off + k)<<2];

	}
	barrier(CLK_LOCAL_MEM_FENCE);

	int n=1;
	int n2=_K0_N/2;
	for(int log2_n = 0, log2_n2 = _K0_LOG2_N - 1; log2_n < _K0_LOG2_N; log2_n++, log2_n2--)
	{
		n = (1 << log2_n);
		n2 = (1 << log2_n2);

		for ( int k = lcl_id; (k < mad24(n2 , n/2, n2)); k+= _K0_GROUP_SZ) {

			FHTIterationG2(data, gsincos,	n, n2, k );

		}
		barrier(CLK_LOCAL_MEM_FENCE);

	}

// write out kernel
	uint kern_off = out_version_stride * upload_id + out_chnl_stride * chnl_id;
	for( uint i = lcl_id; i < _K0_N; i+= _K0_GROUP_SZ) {
		out[i + grp_id * _K0_N  + kern_off] = *(__local float*)&data[i<<2];
	}


// DO WE NEED THAT ???????

#if 0
// zero out data history
	uint data_off = data_channel_stride * chnl_id + mul24(grp_id, (uint)_K0_N);
	for( uint i = lcl_id; i < _K0_N; i+= _K0_GROUP_SZ) {
		history_data[i  + data_off] = 0;
	}
// reset counters
	if ( glb_id == 0 )
	{
		rount_counters[chnl_id] = 0;
	}
#endif
}


__kernel
void amdFHTAdvanceTime(__constant uint * channels_map,
				__global uint * round_counters,
				int step
				)
{
	uint glbl_id = get_local_id(0);
	uint chnl_id = channels_map[glbl_id];
// reset counters
	round_counters[chnl_id] += step;
}





__kernel
void amdFHTPushIn(__global const char * in,
				__global __FLOAT__ * out,
				__constant short * gbitreverse,
				__constant char * gsincos,
				uint n_in_blocks,     // # of blocks kept in input staging
				uint in_version_stride,
				uint in_chnl_stride,
				uint out_version_stride,
				uint out_chnl_stride,
				uint n_conv_blocks,
				uint version_stride,
				__constant uint * channels_map,
				__constant uint * versions_map,
				__constant uint * round_counters,
				uint counter2
				)
{
	int lcl_id = get_local_id(0);
	int grp_id = get_group_id(0);
	int chnl_index = get_group_id(1);

	__local char data[(_K0_N) << 2];
	uint chnl = channels_map[chnl_index];
	uint version = versions_map[chnl_index];
	uint counter = round_counters[chnl];

// read data with bit reverse
	uint in_off = (in_chnl_stride * chnl);

// modulo
// first half
	uint index0 = counter % n_in_blocks;
// second half
	uint index1 = (((int)index0 - 1) < 0) ? n_in_blocks - 1 : index0 - 1;

	uint output_index = counter % n_conv_blocks;

	int index[(_N_TO_READ >> 1)];
	
	for( int i = 0; i < (_N_TO_READ >> 1); i++ )
	{
		index[i] = gbitreverse[lcl_id + (i << _K0_LOG2_GROUP_SZ)];
	}

	for( int i = 0; i < (_N_TO_READ >> 1); i++ )
	{
		*(__local float*)&data[(index[i]<<2)] = *(__global float*)&in[(lcl_id + (i << _K0_LOG2_GROUP_SZ) + mul24(index0, (uint)_K0_N/2) + in_off)<<2];
	}


	// previous input block

	for( int i = 0; i < (_N_TO_READ >> 1); i++ )
	{
		index[i] = gbitreverse[lcl_id + (i << _K0_LOG2_GROUP_SZ) + _K0_N/2];
	}

	for( int i = 0; i < (_N_TO_READ >> 1); i++ )
	{
		*(__local float*)&data[(index[i]<<2)] = *(__global float*)&in[(lcl_id + (i << _K0_LOG2_GROUP_SZ) +  + mul24(index1, (uint)_K0_N/2) + in_off)<<2];
	}

	barrier(CLK_LOCAL_MEM_FENCE);


	int n=1;
	int n2=_K0_N/2;
	for(int log2_n = 0, log2_n2 = _K0_LOG2_N - 1; log2_n < _K0_LOG2_N; log2_n++, log2_n2--)
	{
		n = (1 << log2_n);
		n2 = (1 << log2_n2);

		for ( int k = lcl_id; (k < mad24(n2 , n/2, n2)); k+= _K0_GROUP_SZ) {

			FHTIterationG2(data, gsincos,	n, n2, k );

		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}

	uint out_off = ( out_chnl_stride * chnl) + output_index *_K0_N;
	for( int i = lcl_id; i < _K0_N; i+= _K0_GROUP_SZ) {
		out[i + out_off] = *(__local float*)&data[i<<2];
/*
		if ( chnl == 0 )
		{
			printf("in: %d %f\n", lcl_id, *(__local float*)&data[i<<2]);
		}
*/
	}



}





__kernel
void amdFHTPushOut(__global const __FLOAT__ * in,
				__global __FLOAT__ * out,
				__constant short * gbitreverse,
				__constant __FLOAT__ * gsincos,
				uint in_version_stride,
				uint in_chnl_stride,
				uint out_version_stride,
				uint out_chnl_stride,
				float scale,
				uint counter_version_stride,
				__constant uint * channels_map,
				__constant uint * versions_map,
				__global uint * round_counters,
				int advance_time
				)
{
	int lcl_id = get_local_id(0);
	int grp_id = get_group_id(0);
	int chnl_index = get_group_id(1);

	__local __FLOAT__ data[_K0_N];
	uint chnl_id = channels_map[chnl_index];  // channel
	uint upload_id = versions_map[chnl_index]; // version

// read data with bit reverse


	for( int i = lcl_id; i < _K0_N; i+= _K0_GROUP_SZ) {
		data[gbitreverse[i]] = in[i + in_chnl_stride * chnl_id + in_version_stride *upload_id];
	}

	barrier(CLK_LOCAL_MEM_FENCE);


	int n=1;
	int n2=_K0_N/2;
	for(int log2_n = 0, log2_n2 = _K0_LOG2_N - 1; log2_n < _K0_LOG2_N; log2_n++, log2_n2--)
	{
		n = (1 << log2_n);
		n2 = (1 << log2_n2);

		for ( int k = lcl_id;k < mad24(n2 , n/2, n2); k+= _K0_GROUP_SZ) {

			FHTIterationG(data, gsincos,	n, n2, k );
		}
		barrier(CLK_LOCAL_MEM_FENCE);

	}

	for( int i = lcl_id; i < _K0_N/2; i+= _K0_GROUP_SZ) {
		out[i + mul24(out_version_stride, upload_id) + mul24(out_chnl_stride, chnl_id)] = data[i] * scale;
/*
		if ( chnl_id == 0 )
		{
			printf("in: %d %f\n", lcl_id, data[i] * scale);
		}
*/
	}

// update counters at the end of the round
		// current counter
	if ( advance_time && lcl_id == 0 )
	{
		uint counter = round_counters[chnl_id];
		round_counters[chnl_id] = counter + 1;
	}
}


/* DHT convolution
Z[k] =  (X[k] *( Y[k] + Y[N-k]) + X[N-k] * ( Y[k] - Y[N-k] ) ) / 2
Z[N-k] = (X[N-k] * ( Y[k] + Y[N-k]) - X[k] * ( Y[k] - Y[N-k] ) ) / 2
X0 == XN etc
below the division by 2 deffered to the final scaling
*/

void FHTmad(__FLOAT__ * z_k, __FLOAT__ * z_N_k, __FLOAT__ x_k, __FLOAT__ x_N_k, __FLOAT__ y_k, __FLOAT__ y_N_k) {
	*z_k = x_k * ( y_k + y_N_k ) + x_N_k * ( y_k - y_N_k);
	*z_N_k = x_N_k * ( y_k + y_N_k ) - x_k * ( y_k - y_N_k);
}
/////////////////////////////////////////////////////
//
// global size is total number of processed paires (N/2 * 2)  -1
/////////////////////////////////////////////////////



__kernel
void FHTMultAddHead2(__global const float * IR,
						__global const float * FHTStore,
						__global float * Accum,
						uint accum_version_stride,
						uint accum_chnl_stride,
						uint IR_version_stride,
						uint chnl_stride,				
						uint IR_bin_shift,
						uint n_bins,        // # bins in the buffer
						uint n_loop_bins,
						uint counter_version_stride,
						__constant uint * channels_map,
						__constant uint * versions_map,
						__constant uint * round_counters,
						int STR_bin_shift     // time position, either 0, no shift, -1 previous time
					  )
{

	uint k = get_global_id(0);
	uint N_k = _K0_N - k;
	uint chunk_id = get_global_id(1);  //  accumulator block we start from
	uint chnl_id = channels_map[get_global_id(2)];  // channel
	uint upload_id = versions_map[get_global_id(2)]; // version
	// current counter
	uint counter = round_counters[chnl_id];

	uint current_bin = (counter + STR_bin_shift) % n_bins;
	uint str_off = chnl_stride * chnl_id;
	uint IR_off = IR_version_stride * upload_id + chnl_stride * chnl_id;
	uint accum_offset = chunk_id << _K0_LOG2_N;
	uint accum_chnl_offset = accum_chnl_stride * chnl_id + accum_version_stride * upload_id;
	uint bin_id = (((int)current_bin - (int)chunk_id) < 0) ? (int)n_bins + ((int)current_bin - (int)chunk_id) : (current_bin - chunk_id);
	uint ir_id = chunk_id + IR_bin_shift;
	uint IR_offset;
	uint store_offset;

/*
	if ( chnl_id == 0 && k==0 && chunk_id == 0)
	{
		printf("K: %d %d %d %d\n", ir_id, bin_id, n_loop_bins, total_n_bins);
	}
*/
	__FLOAT__ ir_k, ir_N_k;
	__FLOAT__ fht_k, fht_N_k;
	__FLOAT__ t_accum_k, t_accum_N_k;
	__FLOAT__ accum_k = 0, accum_N_k = 0;

	N_k = (k == 0 ) ? _K0_N/2 : N_k;

	for(uint i = 0; i < n_loop_bins && ir_id < n_bins; i++, ir_id += get_global_size(1), bin_id = (int)bin_id - get_global_size(1) )
	{

		bin_id = ((int)bin_id < 0) ? (int)n_bins + (int)bin_id : bin_id;
		IR_offset = ir_id << _K0_LOG2_N;
		store_offset = bin_id << _K0_LOG2_N;
		uint ir_off = IR_off + IR_offset;
		uint store_off = str_off + store_offset;
		ir_k = IR[(ir_off + k)];
		ir_N_k = IR[(ir_off + N_k)];
		fht_k = FHTStore[(store_off +k)];
		fht_N_k = FHTStore[(store_off + N_k)];


		FHTmad(&t_accum_k, &t_accum_N_k, ir_k, ir_N_k, fht_k, fht_N_k);
		t_accum_k = (k==0) ? ir_k * fht_k * (__FLOAT__)2. : t_accum_k;
		t_accum_N_k = (k==0) ? ir_N_k * fht_N_k  * (__FLOAT__)2. : t_accum_N_k;


		accum_k += t_accum_k;
		accum_N_k += t_accum_N_k;	
/*
		if ( chnl_id == 0 && i== 0 && k < 16)
		{
			printf("in: %d %f %f\n", k, t_accum_k, t_accum_N_k);
		}
*/

	}

	uint accum_off = (accum_chnl_offset + accum_offset);
	Accum[(accum_off + k)] = accum_k;
	Accum[(accum_off + N_k)] = accum_N_k;

}



__kernel
void FHTMultAddTail(__global __FLOAT__ * Accum,
					__constant uint * versions_map,
					__constant int * channels_map,
					uint accum_version_stride,
					uint accum_chnl_stride,
					uint n_loop_bins,
					uint total_n_bins
					)
{

	uint k = get_global_id(0);
	uint chunk_id = get_global_id(1);
	uint chnl_id = channels_map[get_global_id(2)];
	uint upload_id = versions_map[get_global_id(2)]; // version

	
	uint channel_off = accum_chnl_stride * chnl_id + accum_version_stride * upload_id;
	uint accum_offset = chunk_id << _K0_LOG2_N;

	__FLOAT__ accum = 0;
	for(uint i = 0, c = chunk_id; i < n_loop_bins && c < total_n_bins; i++, c += get_global_size(1)) {

		uint off = channel_off + (c << _K0_LOG2_N);
		accum += Accum[k + off];
	}

	Accum[channel_off + accum_offset + k] = accum;
/*
	if ( chnl_id == 0)
	{
		printf("acc out: %d %d %f\n", k, channel_off + accum_offset + k, accum);
	}
*/

}


/*-------------------------------------------------------------------------------------------------
Head-Tail algorithms
--------------------------------------------------------------------------------------------------*/
__kernel
void amdFHTConvHead1(__global const char * in, // pipelone input
				__global const float * IR,    // filter
				__global const float * Accum, // acuumulator for the tail
				__global float * Hist,       // direct transform data history
				__global __FLOAT__ * out,    // pipeline output
				__constant short * gbitreverse,
				__constant char * gsincos,
				uint n_in_blocks,     // # of blocks kept in input staging
				uint n_conv_blocks,  // # of conv blocks (total)
				float scale,         // inverse conv scale
				int prev_input,    // use previous input
				int advance_time,  // advance time counter
				uint in_version_stride,
				uint in_chnl_stride,
				uint hist_version_stride,
				uint hist_chnl_stride,
				uint IR_version_stride,
				uint IR_chnl_stride,
				uint accum_version_stride,
				uint accum_chnl_stride,
				uint counter_version_stride,
				uint out_version_stride,
				uint out_chnl_stride,
				__constant uint * channels_map,
				__constant uint * versions_map,
				__global uint * round_counters
				)
{
	int lcl_id = get_local_id(0);
	int grp_id = get_group_id(0);
	int chnl_index = get_global_id(1);

	__local char data[(_K0_N) << 2];
	uint chnl = channels_map[chnl_index];
	uint version = versions_map[chnl_index];
	uint counter = round_counters[chnl];
// position in the history buffer
	uint output_index = counter % n_conv_blocks;
	uint hist_off = ( hist_chnl_stride * chnl) + output_index *_K0_N;

/*--------------------------------------------------------------------------------------------
	direct transform
--------------------------------------------------------------------------------------------*/
	if ( !prev_input)
	{
// read data with bit reverse
// first half
		uint in_off = (in_chnl_stride * chnl);
// modulo
		uint index0 = counter % n_in_blocks;
		uint index1 = (((int)index0 - 1) < 0) ? n_in_blocks - 1 : index0 - 1;


		int index[(_N_TO_READ >> 1)];
	
		for( int i = 0; i < (_N_TO_READ >> 1); i++ )
		{
			index[i] = gbitreverse[lcl_id + (i << _K0_LOG2_GROUP_SZ)];
		}

		for( int i = 0; i < (_N_TO_READ >> 1); i++ )
		{
			*(__local float*)&data[(index[i]<<2)] = *(__global float*)&in[(lcl_id + (i << _K0_LOG2_GROUP_SZ) + mul24(index0, (uint)_K0_N/2) + in_off)<<2];

/*
		if ( chnl == 0 && lcl_id == 0 && i < 8)
		{
			printf("in0: %d %f\n", i, *(__local float*)&data[(index[i]<<2)]);
		}
*/
		}


	// previous input block

		for( int i = 0; i < (_N_TO_READ >> 1); i++ )
		{
			index[i] = gbitreverse[lcl_id + (i << _K0_LOG2_GROUP_SZ) + _K0_N/2];
		}

		for( int i = 0; i < (_N_TO_READ >> 1); i++ )
		{
			*(__local float*)&data[(index[i]<<2)] = *(__global float*)&in[(lcl_id + (i << _K0_LOG2_GROUP_SZ) + mul24(index1, (uint)_K0_N/2) + in_off)<<2];

/*
		if ( chnl == 0  && lcl_id == 0 && i < 8)
		{
			printf("in1: %d %f\n", i, *(__local float*)&data[(index[i]<<2)]);
		}

*/
		}

		barrier(CLK_LOCAL_MEM_FENCE);

// forward FHT
		int n=1;
		int n2=_K0_N/2;
		for(int log2_n = 0, log2_n2 = _K0_LOG2_N - 1; log2_n < _K0_LOG2_N; log2_n++, log2_n2--)
		{
			n = (1 << log2_n);
			n2 = (1 << log2_n2);

			for ( int k = lcl_id; (k < mad24(n2 , n/2, n2)); k+= _K0_GROUP_SZ) {

				FHTIterationG2(data, gsincos,	n, n2, k );

			}
	
			barrier(CLK_LOCAL_MEM_FENCE);
		}


// write to the history buffer
// this's going to be used in the tail part of the algorithm

		for( int i = lcl_id; i < _K0_N; i+= _K0_GROUP_SZ)
		{
			Hist[i + hist_off] = *(__local float*)&data[i<<2];
/*
		if ( chnl == 0 && i < 8)
		{
			printf("in: %d %f\n", lcl_id, *(__local float*)&data[i<<2]);
		}
*/
		}

	}  // !_prev_input
	else
	{
// read previous input from the history buffer


		for( int i = lcl_id; i < _K0_N; i+= _K0_GROUP_SZ)
		{
			*(__local float*)&data[i<<2] = Hist[i + hist_off];
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}
/*----------------------------------------------------------------------------------------------
 CMAD
 Add tail
-------------------------------------------------------------------------------------------*/
	uint accum_offset = accum_chnl_stride * chnl + accum_version_stride * version;
// 0- block
	uint IR_off = IR_version_stride * version + IR_chnl_stride * chnl;

	float2 cmad[(_N_TO_READ>>1)];

	for( uint i = 0, k = lcl_id; i < (_N_TO_READ>>1); i++, k+= _K0_GROUP_SZ)
	{
		__FLOAT__ ir_k, ir_N_k;
		__FLOAT__ fht_k, fht_N_k;
		__FLOAT__ t_k, t_N_k;
		__FLOAT__ tail_k, tail_N_k;

		uint N_k = _K0_N - k;
		N_k = (k == 0 ) ? _K0_N/2 : N_k;
		ir_k = IR[(IR_off + k)];
		ir_N_k = IR[(IR_off + N_k)];
		fht_k = *(__local float*)&data[k<<2];
		fht_N_k = *(__local float*)&data[N_k<<2];
		tail_k = Accum[accum_offset + k];
		tail_N_k = Accum[accum_offset + N_k];

		FHTmad(&t_k, &t_N_k, ir_k, ir_N_k, fht_k, fht_N_k);
		cmad[i].s0 = (k==0) ? ir_k * fht_k * (__FLOAT__)2. : t_k;
		cmad[i].s1 = (k==0) ? ir_N_k * fht_N_k  * (__FLOAT__)2. : t_N_k;
		cmad[i].s0 += tail_k;
		cmad[i].s1 += tail_N_k;
/*
		if ( chnl == 0 && k < 8 )
		{
			printf("acc in: %d %f %f %f  %d %f %f %f\n", k, ir_k, fht_k, tail_k, N_k, ir_N_k, fht_N_k, tail_N_k);
		}
*/	

	}

	barrier(CLK_LOCAL_MEM_FENCE);

/*----------------------------------------------------------------------------------------------
 bit reverse
 inverse transform
-------------------------------------------------------------------------------------------*/
	for( uint i = 0, k = lcl_id; i < (_N_TO_READ >> 1); i++, k+= _K0_GROUP_SZ)
	{
		uint N_k = _K0_N - k;
		N_k = (k == 0 ) ? _K0_N/2 : N_k;
		*(__local float*)&data[(gbitreverse[k]) <<2] = cmad[i].s0;
		*(__local float*)&data[(gbitreverse[N_k]) << 2] = cmad[i].s1;

	}

	barrier(CLK_LOCAL_MEM_FENCE);

	{
		int n=1;
		int n2=_K0_N/2;
		for(int log2_n = 0, log2_n2 = _K0_LOG2_N - 1; log2_n < _K0_LOG2_N; log2_n++, log2_n2--)
		{
			n = (1 << log2_n);
			n2 = (1 << log2_n2);

			for ( int k = lcl_id;k < mad24(n2 , n/2, n2); k+= _K0_GROUP_SZ) {

				FHTIterationG2(data, gsincos,	n, n2, k );
			}
			barrier(CLK_LOCAL_MEM_FENCE);
		}
	}

// write out into pipeline output buffer
	for( int i = lcl_id; i < _K0_N/2; i+= _K0_GROUP_SZ) {
		out[i + mul24(out_version_stride, version) + mul24(out_chnl_stride, chnl)] = *(__local float*)&data[i<<2] * scale;
/*
		if ( chnl == 0 )
		{
			printf("in: %d %f\n", lcl_id, *(__local float*)&data[i<<2] * scale);
		}
*/
	}

// update counters at the end of the round
		// current counter

	if ( advance_time && lcl_id == 0 )
	{
		uint counter = round_counters[chnl];
		round_counters[chnl] = counter + 1;
	}



}
