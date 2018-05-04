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