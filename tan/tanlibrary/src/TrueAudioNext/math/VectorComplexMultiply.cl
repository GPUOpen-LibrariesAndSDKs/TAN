#define Lx        64

__kernel
__attribute__((reqd_work_group_size(Lx, 1, 1)))
void VectorComplexMul(
    __global float* pInputAFlt,     ///< [in ]
    ulong aOffsetInFloats,          ///< [in ]
    __global float* pInputBFlt,     ///< [in ]
    ulong bOffsetInFloats,          ///< [in ]
    __global float*  pResultFlt,    ///< [out]
    ulong rOffsetInFloats,          ///< [in ]
    int countInQuadFloats           ///< [in ]
)
{
    // Lay-out of real and imaginary numbers in input vectors
    // R Im R Im R Im ....
    // Four float numbers are read in
    // | X | X | X | X    |
    //   R   Im  R   Im
    // Real elements are on xz elements
    // Imaginary elements are on yw elements

    int x = get_global_id(0);
    if (x > countInQuadFloats)
        return;

    pInputAFlt += aOffsetInFloats;
    pInputBFlt += bOffsetInFloats;
    pResultFlt += rOffsetInFloats;

    __global float4* pInputA = (__global float4*)pInputAFlt;
    __global float4* pInputB = (__global float4*)pInputBFlt;
    __global float4* pResult = (__global float4*)pResultFlt;

    float4 inA = pInputA[x];
    float4 inB = pInputB[x];

    pResult[x].xz = inA.xz * inB.xz - inA.yw * inB.yw;
    pResult[x].yw = inA.xz * inB.yw + inA.yw * inB.xz;
}