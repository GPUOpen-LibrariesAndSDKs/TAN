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
#define __TYPE__               __FLOAT__


__kernel 
void SetValue(__global  __TYPE__ * Buffer,
              __TYPE__ Value
			) 
{
    uint id = get_global_id(0);
	Buffer[id] = Value;
}

__kernel
void CopyIRKernel(const __global __FLOAT__ * IR_In,
                  __global __FLOAT2__ *      IR_Out,
				  uint Block_log2,
				  uint IR_In_sz)
{
    uint id = get_global_id(0);
	uint id_rem = id - ((id >> Block_log2) << Block_log2);
	uint id_out = ((id >> Block_log2) << (Block_log2+1)) + id_rem;
    __FLOAT2__ IR = (__FLOAT2__)(0,0);
	IR_Out[id_out + (1<<Block_log2)] = IR;
	IR.s0 = (id < IR_In_sz) ? IR_In[id] : 0;
	IR_Out[id_out] = IR;

//	printf("%d %d %f %f\n", id, id_out, IR_Out[id_out].s0, IR_Out[id_out + (1<<Block_log2)].s0);
}

__kernel
void CopyIRKernel16bitInter(const __global uint * IR_In,
                  __global __FLOAT2__ *  IR_Out,
				   uint Block_log2,
				   uint IR_In_sz)
{
    uint id = get_global_id(0);
	uint glbl_sz = get_global_size(0);
	uint id_rem = id - ((id >> Block_log2) << Block_log2);
	uint id_out = ((id >> Block_log2) << (Block_log2+1)) + id_rem;
    __FLOAT2__ IR_L = (__FLOAT2__)(0,0);
	__FLOAT2__ IR_R = (__FLOAT2__)(0,0);

	IR_Out[id_out + (1<<Block_log2)] = IR_L;
	IR_Out[id_out + (1<<Block_log2) + (glbl_sz<<1)] = IR_L;

	uint IR = (id < IR_In_sz) ? IR_In[id] : 0;

	int i_L = (((int)(IR & 0xffff) << 16) >> 16);
	int i_R = ((int)IR  >> 16);
	IR_L.s0 = (__FLOAT__)i_L;
	IR_R.s0 = (__FLOAT__)i_R;
	IR_Out[id_out] = IR_L;
// double each
	IR_Out[id_out + (glbl_sz<<1)] = IR_R;
/*
	if ( id < 64 ) {
	   printf("GPU in: %d %d   %f %f\n", id, id_out + (glbl_sz <<1), IR_L.s0, IR_R.s0);
	}
*/

}

__kernel
void CopyIRKernel16bitInterSum(const __global uint * IR_In,
//                               __global __FLOAT2__ * IR_Out,
							   __global __FLOAT2__ * sum_out, 
				              uint Block_log2,
				              uint IR_In_sz)
{
    uint id = get_global_id(0);
	uint glbl_sz = get_global_size(0);
	uint groupID = get_group_id(0);
	uint lcl_id = get_local_id(0);
	uint id_rem = id - ((id >> Block_log2) << Block_log2);
	uint id_out = ((id >> Block_log2) << (Block_log2+1)) + id_rem;

	__local __FLOAT2__ sum_buffer[256];

	sum_buffer[lcl_id] = (__FLOAT2__)(0,0);

	barrier(CLK_LOCAL_MEM_FENCE);

    __FLOAT2__ IR_L = (__FLOAT2__)(0,0);
	__FLOAT2__ IR_R = (__FLOAT2__)(0,0);

//	IR_Out[id_out + (1<<Block_log2)] = IR_L;
//	IR_Out[id_out + (1<<Block_log2) + (glbl_sz<<1)] = IR_L;

	uint IR = (id < IR_In_sz) ? IR_In[id] : 0;

	int i_L = (((int)(IR & 0xffff) << 16) >> 16);
	int i_R = ((int)IR  >> 16);
	IR_L.s0 = (__FLOAT__)i_L ;
	IR_R.s0 = (__FLOAT__)i_R ;
// prefix sum
    __FLOAT2__ my_sum = (__FLOAT2__)(IR_L.s0, IR_R.s0);

	sum_buffer[lcl_id] = my_sum;
	barrier(CLK_LOCAL_MEM_FENCE);
// 
    if ( lcl_id < 128 ) {
         my_sum += sum_buffer[lcl_id + 128];
		 sum_buffer[lcl_id] = my_sum;
	}
	barrier(CLK_LOCAL_MEM_FENCE);
    if ( lcl_id < 64 ) {
         my_sum += sum_buffer[lcl_id + 64];
		 sum_buffer[lcl_id] = my_sum;
	}
	barrier(CLK_LOCAL_MEM_FENCE);
    if ( lcl_id < 32 ) {
         my_sum += sum_buffer[lcl_id + 32];
		 sum_buffer[lcl_id] = my_sum;
	}
	barrier(CLK_LOCAL_MEM_FENCE);
    if ( lcl_id < 16 ) {
         my_sum += sum_buffer[lcl_id + 16];
		 sum_buffer[lcl_id] = my_sum;
	}
	barrier(CLK_LOCAL_MEM_FENCE);
    if ( lcl_id < 8 ) {
         my_sum += sum_buffer[lcl_id + 8];
		 sum_buffer[lcl_id] = my_sum;
	}
	barrier(CLK_LOCAL_MEM_FENCE);
    if ( lcl_id < 4 ) {
         my_sum += sum_buffer[lcl_id + 4];
		 sum_buffer[lcl_id] = my_sum;
	}
	barrier(CLK_LOCAL_MEM_FENCE);
    if ( lcl_id < 2 ) {
         my_sum += sum_buffer[lcl_id + 2];
		 sum_buffer[lcl_id] = my_sum;
	}
	barrier(CLK_LOCAL_MEM_FENCE);
    if ( lcl_id == 0 ) {
		my_sum += sum_buffer[1];
		sum_out[groupID] = my_sum;
	}

//	IR_Out[id_out] = IR_L;
// double each
//	IR_Out[id_out + (glbl_sz<<1)] = IR_R;
#if 0
	if ( groupID == 193 /*&& id < 256*/ ) {
	   printf("GPU in: %d %d  %d  %f %f\n", lcl_id, id, id_out + (glbl_sz <<1), IR_L.s0, IR_R.s0);
	}
#endif
#if 0
	if ( /*(groupID & 1) == 0 &&*/lcl_id == 0) {
	   printf("GPU sum: %d %f %f\n", groupID, sum_out[groupID].s0, sum_out[groupID].s1);
	}
#endif

}

__kernel
void ItermPrefixSum(__global __FLOAT2__ * sum_out, uint loop_limit, uint total_limit)
{
    uint id = get_global_id(0);
	uint glbl_sz = get_global_size(0);
	uint groupID = get_group_id(0);
	uint lcl_id = get_local_id(0);

	__local __FLOAT2__ sum_buffer[256];
// prefix sum
    __FLOAT2__ my_sum = (__FLOAT2__)(0, 0);
// all previous sums
	for (int i = id, k = 0; i < total_limit && k < loop_limit; i += 256, k++ ) {
	     my_sum += sum_out[i];
	}

	sum_buffer[lcl_id] = my_sum;

	barrier(CLK_LOCAL_MEM_FENCE);
// 
    if ( lcl_id < 128 ) {
         my_sum += sum_buffer[lcl_id + 128];
		 sum_buffer[lcl_id] = my_sum;
	}
	barrier(CLK_LOCAL_MEM_FENCE);
    if ( lcl_id < 64 ) {
         my_sum += sum_buffer[lcl_id + 64];
		 sum_buffer[lcl_id] = my_sum;
	}
	barrier(CLK_LOCAL_MEM_FENCE);
    if ( lcl_id < 32 ) {
         my_sum += sum_buffer[lcl_id + 32];
		 sum_buffer[lcl_id] = my_sum;
	}
	barrier(CLK_LOCAL_MEM_FENCE);
    if ( lcl_id < 16 ) {
         my_sum += sum_buffer[lcl_id + 16];
		 sum_buffer[lcl_id] = my_sum;
	}
	barrier(CLK_LOCAL_MEM_FENCE);
    if ( lcl_id < 8 ) {
         my_sum += sum_buffer[lcl_id + 8];
		 sum_buffer[lcl_id] = my_sum;
	}
	barrier(CLK_LOCAL_MEM_FENCE);
    if ( lcl_id < 4 ) {
         my_sum += sum_buffer[lcl_id + 4];
		 sum_buffer[lcl_id] = my_sum;
	}
	barrier(CLK_LOCAL_MEM_FENCE);
    if ( lcl_id < 2 ) {
         my_sum += sum_buffer[lcl_id + 2];
		 sum_buffer[lcl_id] = my_sum;
	}
	barrier(CLK_LOCAL_MEM_FENCE);
    if ( lcl_id == 0 ) {
		my_sum += sum_buffer[1];
		sum_out[groupID] = my_sum;;
	}

}

//////////////////////////////////////////////////
// the left shift by 1 due to a complex part that has to be generated , with 0 value

__kernel
void KernelNormalizer(const __global uint * IR_In,
                    __global __FLOAT2__ * IR_Out,
					  const __global __FLOAT2__ * sum_out, 
				      uint Block_log2,
				      uint IR_In_sz,
					  uint IR_Mem_sz,
					  uint IR_Mem_offset
                    )
{
    uint id = get_global_id(0);
	uint glbl_sz = get_global_size(0);
	uint groupID = get_group_id(0);
	uint lcl_id = get_local_id(0);
	uint id_rem = id - ((id >> Block_log2) << Block_log2);
	uint id_out = ((id >> Block_log2) << (Block_log2+1)) + id_rem + (IR_Mem_offset << (1 + Block_log2));


	__FLOAT2__ normalizer = sum_out[0];

    __FLOAT2__ IR_L = (__FLOAT2__)(0,0);
	__FLOAT2__ IR_R = (__FLOAT2__)(0,0);

	IR_Out[id_out + (1<<Block_log2)] = IR_L;
	IR_Out[id_out + (1<<Block_log2) + (IR_Mem_sz<<1)] = IR_L;

	uint IR = (id < IR_In_sz) ? IR_In[id] : 0;

	int i_L = (((int)(IR & 0xffff) << 16) >> 16);
	int i_R = ((int)IR  >> 16);
// normalize
	IR_L.s0 = (normalizer.s0 == 0 )? 0 : (__FLOAT__)i_L / normalizer.s0;
	IR_R.s0 = (normalizer.s1 == 0 )? 0 : (__FLOAT__)i_R / normalizer.s1;

// write out
	IR_Out[id_out] = IR_L;
// double each
	IR_Out[id_out + (IR_Mem_sz<<1)] = IR_R;
/*
	if ( id == 0 )
	{
	    printf("GPU norm: %f %f %f %f\n", normalizer.s0, normalizer.s1, IR_L.s0, IR_R.s0);
	}
*/

}


__kernel void partitionIR(
    __global  float* in,
    __global  float* ir,
    __global  const int* channels_map,
    int num_channels,
    int num_blocks,
    int in_channel_stride,
    int ir_channel_stride,
    int block_sz
)
{
    int sampleId = get_global_id(0);
    int blockId = get_global_id(1);
    int chId =  channels_map[get_global_id(2)];
    if (blockId >= num_blocks) return;
    if (sampleId >= block_sz) return;
    int in_channel_offset = in_channel_stride*chId;
    int ir_channel_offset = ir_channel_stride*chId;
    int block_offset = blockId * block_sz;
    int ir_block_offset = block_offset * 2;
    int in_block_offset = block_offset;
    ir[ir_channel_offset + ir_block_offset + sampleId] = in[in_channel_offset + in_block_offset + sampleId];
    ir[ir_channel_offset + ir_block_offset + block_sz + sampleId] = 0;
}





