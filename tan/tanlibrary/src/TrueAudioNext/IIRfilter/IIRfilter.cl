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

__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

__kernel
void IIRfilter(
	__global    float*  bufferInput,	///< [in]	
	__global    float*  inputHistory,	///< [in]	
	__global    float*  outputHistory,	///< [in]	
	__global    float*  inputTaps,		///< [in]	
	__global    float*  outputTaps,		///< [in]	
	__global    float*  bufferOutput,	///< [in]
	__global    int*  	inOutPos,		///< [in]
	int     numInputTaps,
	int     numOutputTaps,
	int		numSamples
)
{
	uint idx = get_global_id(0);
	uint chan = get_global_id(1);

	global float* pBufferInput = &bufferInput[chan * numSamples];
	global float* pInputHistory = &inputHistory[chan * numInputTaps];
	global float* pOutputHistory = &outputHistory[chan * numOutputTaps];
	global float* pInputTaps = &inputTaps[chan * numInputTaps];
	global float* pOutputTaps = &outputTaps[chan * numOutputTaps];
	global float* pBufferOutput = &bufferOutput[chan * numSamples];

	int		inputHistPos = inOutPos[0];
	int		outputHistPos = inOutPos[1];

	
	float sample;
	for (int sn = 0; sn < numSamples; sn++)
	{
		sample = 0.0;
		pInputHistory[inputHistPos] = pBufferInput[sn];

		//FIR part
		for (int k = 0; k < numInputTaps; k++) {
			sample += pInputTaps[k] * pInputHistory[(inputHistPos + numInputTaps - k) % numInputTaps];
		}

		//IIR part
		for (int l = 0; l < numOutputTaps; l++) {
			sample += pOutputTaps[l] * pOutputHistory[(outputHistPos + numOutputTaps - l) % numOutputTaps];
		}
		pBufferOutput[sn] = sample;

		++inputHistPos;
		++outputHistPos;

		inputHistPos = (inputHistPos % numInputTaps);
		outputHistPos = (outputHistPos % numOutputTaps);
		pOutputHistory[outputHistPos] = pBufferOutput[sn];

	}

	inOutPos[0] = inputHistPos;
	inOutPos[1] = outputHistPos;
}



