#define Lx		256
#define Float2Int   67108864.0f



__kernel
    __attribute__((reqd_work_group_size(Lx, 1, 1)))
    void Fill(    
    __global uint4*  intResponse,		   ///< [in]	
	__global float4* floatResponse         ///< [out ]
    )
{
	int x = get_global_id(0); 	
	floatResponse[x] = convert_float4(intResponse[x])/Float2Int;
	intResponse[x] = 0;

}