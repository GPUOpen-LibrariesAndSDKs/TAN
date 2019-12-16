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