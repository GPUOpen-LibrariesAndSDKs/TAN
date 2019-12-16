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

#ifndef __REVERBERATE2_OCL__
#define __REVERBERATE2_OCL__

#include "Reverberate_OCL.h"

int ReverbOCLProcessing(ProjPlan *plan, __FLOAT__ ** input, __FLOAT__ ** output, cl_mem* cl_output, int numSamples, int numChans,  int* channel_ids, unsigned int _ir_version, int flags, StatisticsPerRun * stat, int xfade_state);

int ReverbOCLMoveKernelIntoFHTPipeline2(ProjPlan * plan, int numChans, int* channel_ids, unsigned int _ir_version,  int init_flags);
void cpuHeterogenouosProcessing(ProjPlan * plan, int numChans, int init_flags);

int prepareKernels(ProjPlan * plan, int init_flags, float ** host_kern, int _n_channels, int* _channel_ids, unsigned int _ir_version, const int* kernel_len);// Array of host buffers, one per each channel
int prepareKernels(ProjPlan * plan, int init_flags, const cl_mem cl_kernels, int _n_channels, int* _channel_ids, unsigned int _ir_version, const int in_channel_sample_stride);// Contiguous OCL buffers, most optimized

int ReverbOCLSetupSG( ProjPlan * plan);

int ReverbOCLSetupScatterGather(ProjPlan * plan);
int ReverbOCLSetupComplexMAD(ProjPlan * plan);
int ReverbOCLSetupComplexMADProcessing(ProjPlan * plan);

// FFT processing routines
int ReverbOCLMoveDataIntoPipeline(ProjPlan *plan, __FLOAT__ ** input, int numSamples, int numChans, int* channel_ids);
int ReverbOCLGetStream2Data(ProjPlan * plan);

int ReverbOCLSum2Streams(ProjPlan * plan,__FLOAT__ ** output );

int ReverbOCLdoProcessing2(ProjPlan *plan, int nCPUProcessedBlocks);
int ReverbOCLdoProcessing1(ProjPlan *plan, int nCPUProcessedBlocks);

int ReverbOCLProcessing2FHTsHtrgs(ProjPlan *plan, __FLOAT__ ** input, __FLOAT__ ** output, int numSamples, int numChans, int flags);

int ReverbOCLUnsetup(ProjPlan * rvrb);

int ReverbOCLProcessingReadSamples(ProjPlan *plan, __FLOAT__ ** input, int numSamples, int numChans, int* channel_ids, int flags);
//int ReverbOCLSetupReverbKernelProcess(ProjPlan * plan);

// EXTERNAL INTERFACE
int ReverbOCLProcessing(amdOCLRvrb rvrb, __FLOAT__ ** input, __FLOAT__ ** output, cl_mem* cl_output, int numSamples, int numChans, int* channel_ids, unsigned int _ir_version, int flags);

int ReverbOCLEndBlock(ProjPlan *plan);



#endif