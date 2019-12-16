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

#define OVERFLOW_WARNING 1

__kernel void shortToShort(
	__global	short*	inputBuffer,	///< [in]
				long	inputStep,		///< [in]
				long	inputOffset,	///< [in]
	__global	short*	outputBuffer,	///< [out]
				long	outputStep,		///< [in]
				long	outputOffset	///< [in]
	)
{
	int gid = get_global_id(0);
	outputBuffer[(gid * outputStep) + outputOffset] = inputBuffer[(gid * inputStep) + inputOffset];
}

__kernel void floatToFloat(
	__global	float*	inputBuffer,	///< [in]
				long	inputStep,		///< [in]
				long	inputOffset,	///< [in]
	__global	float*	outputBuffer,	///< [out]
				long	outputStep,		///< [in]
				long	outputOffset	///< [in]
	)
{
	int gid = get_global_id(0);
	outputBuffer[(gid * outputStep) + outputOffset] = inputBuffer[(gid * inputStep) + inputOffset];
}

__kernel void shortToFloat(
	__global	short*	inputBuffer,	///< [in]
				long	inputStep,		///< [in]
				long	inputOffset,	///< [in]
	__global	float*	outputBuffer,	///< [out]
				long	outputStep,		///< [in]
				long	outputOffset,	///< [in]
				float	conversionGain	///< [in]
	)
{
	float scale = conversionGain / SHRT_MAX;

	int gid = get_global_id(0);
	outputBuffer[(gid * outputStep) + outputOffset] = convert_float( inputBuffer[(gid * inputStep) + inputOffset] ) * scale;
}

__kernel void floatToShort(
	__global	float*	inputBuffer,	///< [in]
				long	inputStep,		///< [in]
				long	inputOffset,	///< [in]
	__global	short*	outputBuffer,	///< [out]
				long	outputStep,		///< [in]
				long	outputOffset,	///< [in]
				float	conversionGain,	///< [in]
	__global	int*	overflowError	///< [out]
	)
{
	int gid = get_global_id(0);

	float scale = SHRT_MAX * conversionGain;
	float f = inputBuffer[(gid * inputStep) + inputOffset] * scale;

	if ( f > SHRT_MAX)
	{
		f = SHRT_MAX;
		*overflowError = OVERFLOW_WARNING;
	}

	if ( f < SHRT_MIN)
	{
		f = SHRT_MIN;
		*overflowError = OVERFLOW_WARNING;
	}

	outputBuffer[(gid * outputStep) + outputOffset] = convert_short( f );
}
