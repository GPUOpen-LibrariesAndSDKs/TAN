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

#pragma OPENCL EXTENSION cl_amd_printf : enable




//It will copy date from the source that is blockLenth long
//to the out, the pad it with padLegnth zeros.  Offsets and 
//in/out lengths are to allow for wrap around
__kernel 
void amdPadFFTBlock(__global const float * in,
				__global float * out,
				int inOffset,
				int inLength,
				int outOffset,
				int outLength,
				int blockLength,
				int padLength)  {
	int glbl_id = get_global_id(0); 

	int blockNumber = (glbl_id) / (blockLength + padLength);
	int inIndex = ((glbl_id + inOffset - blockNumber * padLength) % inLength);
	int outIndex = ((glbl_id + outOffset) % outLength);

	bool isPad = (glbl_id % (blockLength + padLength)) >= blockLength;
	
	out[outIndex] = isPad ? 0 : in[inIndex];
}

//Used for taking data from real to complex representation
__kernel 
void amdInterleaveFFTBlock(__global const float * in,
				__global float * out)  {
	int glbl_id = get_global_id(0); 

	out[glbl_id*2] = in[glbl_id];
	out[glbl_id*2 + 1] = 0;
}

__kernel 
void amdDeinterleaveFFTBlock(__global const float * in,
				__global float * out)  {
	int glbl_id = get_global_id(0); 

	out[glbl_id] = in[glbl_id*2];
}

//The MultiChan version just can handle multiple channels at a time
//fill the 'out' sequentially with 'n' blocks of data by indexing into 'in' based on channels and curr/prev
__kernel 
void amdInterleaveFFTBlockMultiChan(__global const float * in,
				__global float * out,
				__global const int * channelMap,
				__global const int * currInputMap,
				__global const int * prevInputMap,
				int outBlockLength, /*size of the blocks we are writing to */
				int inBlockLength, /*size of the current and previous block half*/
				int num_input_blocks
				)  {
	int glbl_id = get_global_id(0); 
	int n = glbl_id / (outBlockLength/2); //divide by two as the output is complex nums
	int ch = channelMap[n];
	//the following will be values 0 - (num_input_blocks_ - 1) and point to which sub-block of the input is previous or current
	int currInput = currInputMap[n]; 
	int prevInput = prevInputMap[n];

	//The index of this block
	int i = glbl_id - n * (outBlockLength/2);//divide by two as the output is complex nums

	if (i < inBlockLength) //fill first half with previous values
		out[glbl_id*2]     = in[ch*inBlockLength*num_input_blocks + prevInput*inBlockLength + i];
	else if (i < 2*inBlockLength) //fill second half with current values
		out[glbl_id*2]     = in[ch*inBlockLength*num_input_blocks + currInput*inBlockLength + i - inBlockLength];
	else //no more data to fill so pad with zeros
		out[glbl_id*2]     = 0;
	
	//imaginary number always 0
	out[glbl_id*2 + 1] = 0;
}


__kernel 
void amdMADFFTBlock(__global const float * inSignal,
				__global const float * inIR,
				__global float *out,
				int blockOffset,
				int blockLength,
				int maxBlockNum
				)  {
	int glbl_id = get_global_id(0);

	int inIndex = (glbl_id + blockOffset*blockLength) % (maxBlockNum*blockLength);

	out[glbl_id] = inIR[glbl_id] * inSignal[inIndex];
}

// out[i] = sum_n_over_blocks( in[i + n* blocklength])
__kernel 
void amdAccumulateBlocks(__global const float * in,
				__global float * out,
				int blockLength,
				int maxBlockNum
				)  {
	int glbl_id = get_global_id(0);

	out[glbl_id] = in[glbl_id];
	for (int block = 1; block < maxBlockNum; block++)
	{
		out[glbl_id] += in[glbl_id + block*blockLength];
	}
}


float2 complexMul(const float2 A, const float2 B)
{
	float2 C;
	C.s0 = A.s0 * B.s0 - (A.s1 * B.s1);
	C.s1 = A.s0 * B.s1 + B.s0 * A.s1;

	return C;
}

//Multiply the corresponding frequency components together and accumulate over all blocks
// out[i] = sum_n_over_blocks( inSignal[i + (n + offset)* blocklength] * inIR[i + (n + offset)* blocklength])
__kernel 
void amdMADAccBlocks(__global const float2 * inSignal,
				__global const float2 * inIR,
				__global float2 * out,
				int blockOffset,
				int blockLength,
				int maxBlockNum
				)
{
	int glbl_id = 2 * get_global_id(0);
	int inSigIndex = glbl_id + blockOffset*blockLength;
	int inIRIndex = glbl_id;

	//first block of IR multiplied by currentBlock of Signal
	out[glbl_id/2] = complexMul(inSignal[inSigIndex/2], inIR[inIRIndex/2]);
	for (int block = 1; block < maxBlockNum; block++)
	{
		//next block of IR multiplied by previous block of Signal
		inSigIndex = glbl_id + (blockOffset - block)*blockLength;
		inIRIndex = glbl_id + block*blockLength;
		if (inSigIndex < 0)
			inSigIndex += maxBlockNum*blockLength;
		if (inIRIndex > maxBlockNum*blockLength)
			inIRIndex -= maxBlockNum*blockLength;

		out[glbl_id/2] += complexMul(inSignal[inSigIndex/2], inIR[inIRIndex/2]);
	}
}

//multi-channel version of amdMADAccBlocks
__kernel 
void amdMADAccBlocksMultiChan(__global const float2 * inSignal,
				__global const float2 * inIR,
				__global float2 * out,
				__global const int * channelMap,
				__global const int * setMap,
				__global const int * blockNumberMap,
				int blockLength,
				int maxBlockNum,
				int numSets
				)
{
	int i = get_global_id(0); //sample index
	int n = get_global_id(1); //input channel index
	int ch = channelMap[n];
	int set = setMap[n];
	int startBlock = blockNumberMap[n];

	int inSigIndex = ch * blockLength * maxBlockNum + startBlock * blockLength + i;//index into this channel's section, then into the current block, then the right sample
	int inIRIndex = ((ch * numSets + set ) * maxBlockNum) * blockLength + i;
	int outIndex = n * blockLength + i;
	
	if (n == 1 &&
		((i < 5)
		) )
	{
		//printf("ch %d i %d n %d block %d inSigIndex %d inIRIndex %d outIndex %d\n", ch, i, n, 0, inSigIndex, inIRIndex, outIndex);
	}
	//first block of IR multiplied by currentBlock of Signal
	out[outIndex] = complexMul(inSignal[inSigIndex], inIR[inIRIndex]);
	for (int block = 1; true && block < maxBlockNum; block++)
	{
		int curBlock = startBlock - block;
		if (curBlock < 0)
			curBlock += maxBlockNum;

		inSigIndex = ch * blockLength * maxBlockNum + curBlock * blockLength + i;
		inIRIndex = ((ch * numSets + set) * maxBlockNum + block) * blockLength + i;

		//if (n == 0 &&
		//	((i < 5) ||
		//	 (i > 2040 && i < 2050)
		//	) )
		//	printf("ch %d i %d n %d block %d inSigIndex %d inIRIndex %d outIndex %d\n", ch, i, n, block, inSigIndex, inIRIndex, outIndex);
		if (n == 0 && i == 0)
		{
			//printf("ch %d i %d n %d block %d inSigIndex %d inIRIndex %d outIndex %d\n", ch, i, n, block, inSigIndex, inIRIndex, outIndex);
		}

		out[outIndex] += complexMul(inSignal[inSigIndex], inIR[inIRIndex]);
	}
}


//Takes the FFTed input signal and puts it to the right place in the signal history
__kernel 
void amdSigHistoryInsertMultiChan(__global const float * in,
				__global float * out,
				__global const int * channelMap,
				__global const int * blockNumberMap,
				int blockLength,
				int maxBlockNum
				)
{
	int i = get_global_id(0); //sample index
	int n = get_global_id(1); //input channel index
	int ch = channelMap[n];
	int startBlock = blockNumberMap[n];

	int inIndex = n * blockLength + i;
	int outIndex = ch * blockLength * maxBlockNum + startBlock * blockLength + i;//index into this channel's section, then into the current block, then the right sample

	
	if (n == 1 &&
		((i < 5)
		) )
	{
		//printf("ch %d i %d n %d block %d inSigIndex %d inIRIndex %d outIndex %d\n", ch, i, n, 0, inSigIndex, inIRIndex, outIndex);
	}
	
	out[outIndex] = in[inIndex];
}