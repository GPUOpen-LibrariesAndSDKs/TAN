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

// Max LDS size: 64KB = 65536 Byte = 16384 float element
// Max convolution length for LDS float to work: 16384 - Lx (local size)

/*
__kernel
//__kernel __attribute__((reqd_work_group_size(256,1,1)))
    void TimeDomainConvolutionGeneral(    
    __global    float*  pInput,		   ///< [in]	
	__constant  float*  filter,        ///< [in]
	__global    float*  pOutput,       ///< [out]
	__local     float*  pLocalBuffer,  ///< [in] size of the LDS
	int         filterLength,          ///< [in] Filter/convolution length
	int         inputSize,             ///< [in] Size of the buffer processed per kernel run
	int         paddedInputSize,       ///< [in] 
	int         inputOffset,           ///< [in]
	int         outputOffset           ///< [in]
	)
	{
		if ( get_global_id(0) > inputSize )
			return;
		int localSize = get_local_size(0);
		int readPerWorkItem = (filterLength + localSize) / localSize;
		
		for (int i = 0; i < readPerWorkItem; i++ )
		{
			int ldsIndex = get_local_id(0) + localSize * i;
			//int inputIndex = get_global_id(0) - filterLength/2 + localSize * i; // look into future and past
			int inputIndex = get_global_id(0) - filterLength + localSize * i; // look into the past only
			inputIndex += inputOffset;
			inputIndex = min(max(inputIndex, 0), paddedInputSize);
			pLocalBuffer[ldsIndex] = pInput[inputIndex];
		}
		
		barrier(CLK_LOCAL_MEM_FENCE);
		
		float outTemp = 0;
		
		for (int i = 0; i < filterLength; i++)
		{
			int ldsIndex = get_local_id(0) + i;
			outTemp += pLocalBuffer[ldsIndex] * filter[i];			
		}
		
		pOutput[get_global_id(0) + outputOffset] = outTemp;		
		
	}


#define LocalSize 256
#define FilterLength 2048	
__kernel
__kernel __attribute__((reqd_work_group_size(LocalSize, 1, 1)))
    void TimeDomainConvolutionFixedLength(    
    __global    float*  pInput,		   ///< [in]	
	__constant  float*  filter,        ///< [in]
	__global    float*  pOutput,       ///< [out]
	int         inputSize,             ///< [in] Size of the buffer processed per kernel run
	int         paddedInputSize,       ///< [in] 
	int         inputOffset,           ///< [in]
	int         outputOffset           ///< [in]
	)
	{
		if ( get_global_id(0) > inputSize )
			return;
		
		int ldsSize = FilterLength + LocalSize;
		__local float localBuffer[FilterLength + LocalSize];

		int readPerWorkItem = ldsSize / LocalSize;
		
		for (int i = 0; i < readPerWorkItem; i++ )
		{
			int ldsIndex = get_local_id(0) + LocalSize * i;
			int inputIndex = get_global_id(0) - FilterLength/2 + LocalSize * i;
			inputIndex += inputOffset;
			inputIndex = min(max(inputIndex, 0), paddedInputSize);
			localBuffer[ldsIndex] = pInput[inputIndex];
		}
		
		barrier(CLK_LOCAL_MEM_FENCE);
		
		float outTemp = 0;
		
		for (int i = 0; i < FilterLength; i++)
		{
			int ldsIndex = get_local_id(0) + i;
			outTemp += localBuffer[ldsIndex] * filter[i];			
		}
		
		pOutput[get_global_id(0) + outputOffset] = outTemp;		
		
	}	
	
__kernel
__kernel __attribute__((reqd_work_group_size(LocalSize, 1, 1)))
    void TimeDomainConvolutionFixedLengthAtomic(    
    __global    float*  pInput,		   ///< [in]	
	__constant  float*  filter,        ///< [in]
	volatile __global   int*  pOutput, ///< [out]
	int         inputSize,             ///< [in] Size of the buffer processed per kernel run
	int         paddedInputSize,       ///< [in] 
	int         inputOffset,           ///< [in]
	int         outputOffset           ///< [in]
	)
	{
		if ( get_global_id(0) > inputSize )
			return;
		
		int ldsSize = FilterLength + LocalSize;
		__local float localBuffer[FilterLength + LocalSize];

		int readPerWorkItem = ldsSize / LocalSize;
		
		for (int i = 0; i < readPerWorkItem; i++ )
		{
			int ldsIndex = get_local_id(0) + LocalSize * i;
			int inputIndex = get_global_id(0) - FilterLength/2 + LocalSize * i;
			inputIndex += inputOffset;
			inputIndex = min(max(inputIndex, 0), paddedInputSize);
			localBuffer[ldsIndex] = pInput[inputIndex];
		}
		
		barrier(CLK_LOCAL_MEM_FENCE);
		
		for ( int i = 0; i < FilterLength/LocalSize; i++ )
		{
			for ( int j = 0; j < LocalSize; j++)
			{
				int ldsIndex = get_local_id(0) + LocalSize * i + j;
				int filterIndex = get_local_id(0) + LocalSize * i;
				int outIndex = get_global_id(0) - get_local_id(0) + j;
				float outTemp = localBuffer[ldsIndex] * filter[filterIndex];
				atomic_add(&pOutput[outIndex], (int)(outTemp*1024.0f));
			}
		}
		
	}

*/

__kernel
//__kernel __attribute__((reqd_work_group_size(LocalSize, 1, 1)))
void TimeDomainConvolution(
__global    float*  histBuf,	   ///< [in]	
int         convLength,            ///< [in] convolution length
int         bufPos,                ///< [in]
int         dataLength,            ///< [in] Size of the buffer processed per kernel run
int         firstNonZero,          ///< [in]
int         lastNonZero,           ///< [in]
__global    float*  filter,        ///< [in]
__global    float*  pOutput        ///< [out]
)
{
    int grpid = get_group_id(0);
    int thrdid = get_local_id(0);
    int grptot = get_num_groups(0);
    int grpsz = get_local_size(0);

    int srcOffset = grpid*grpsz;
    int endOffset = srcOffset + grpsz;

    if (endOffset > dataLength)
        endOffset = dataLength;

    //if (get_global_id(0) > inputSize)
     //   return;

    //int ldsSize = FilterLength + LocalSize;
    //__local float localBuffer[FilterLength + LocalSize];

    //int readPerWorkItem = ldsSize / LocalSize;

    //for (int j = 0; j < datalength; j++){
    for (int j = srcOffset; j < endOffset; j++){
            pOutput[j] = 0.0;
        for (int k = firstNonZero; k < lastNonZero; k++){
            pOutput[j] += histBuf[(bufPos + j - k + convLength) % convLength] * filter[k];
        }
    }
}