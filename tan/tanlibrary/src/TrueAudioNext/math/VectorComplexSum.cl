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

#define Lx        64
__kernel
__attribute__((reqd_work_group_size(Lx, 1, 1)))
void VectorComplexSum(
    __global float* pInputFlt,      ///< [in ] 0
	__local float* pAuxiliary,     	///< [in ] 1
    __global float* pResultFlt,		///< [out] 2
	ulong rOffsetInFloats,			///< [in ] 3
    ulong countInQuadFloats         ///< [in ] 4
)
{
    // Lay-out of real and imaginary numbers in input vectors
    // R Im R Im R Im ....
    // Four float numbers are read in
    // | X | X | X | X    |
    //   R   Im  R   Im
    // Real elements are on xz elements
    // Imaginary elements are on yw elements

    int globalID = get_global_id(0);
	int groupID = get_group_id(0);
	
	
	if (globalID > countInQuadFloats)
		return;
	int localID = get_local_id(0);
	int offset = get_local_size(0) >> 1;
	
	pResultFlt += rOffsetInFloats;
	
	__global float4* pInput = (__global float4*)pInputFlt;
    __local float4* pAux = (__local float4*)pAuxiliary;
    __global float2* pResult = (__global float2*)pResultFlt;
	//printf("GroupdID: %d GlobalID:%d offset:%d In: %d %2.2v4hlf", groupID, globalID, countInQuadFloats, offset,pInput[globalID]);
	pAux[localID] = pInput[globalID];
	barrier(CLK_LOCAL_MEM_FENCE);
	
	while(offset > 0){
		barrier(CLK_LOCAL_MEM_FENCE);
		if (localID < offset)
			pAux[localID] += pAux[localID + offset];
		offset = offset >> 1;
	}
	barrier(CLK_LOCAL_MEM_FENCE);
	if(localID == 0){
		pResult[groupID].xy = pAux[0].xy + pAux[0].zw;
	}
}