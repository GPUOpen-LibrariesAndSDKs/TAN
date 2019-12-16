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

#ifndef __DIRECTCONV_OCL__
#define __DIRECTCONV_OCL__


#include "IIRLowLat.h"
#include "IIRLowLat_OCL.h"
#include "Reverberate_OCL.h"
#include "amdFHT_OCL.h"
#define __FLOAT__ float

typedef struct _DirectCPUConvChannel {
	void *data_window;
	void *conv_window;
	void *FFT_conv_window;
	void *FFT_data_window;
	void *FHT_data2_window;
} DirectCPUConvChannel;

typedef struct _DirectConvPass {
	DirectCPUConvChannel *channels;
	void *FHT_bitrvrs;
	void *FHT_sincos;
	void *FHT_routine;
	void *FHT_proc[2];
} DirectConvPass;


int DirectConvCPUSetup(ProjPlan * plan);
int DirectConvGPUSetup(ProjPlan * plan);
int DirectConvCPUDeinit(ProjPlan * plan);
int DirectConvGPUDeinit(ProjPlan * plan);
int SetupCPUDCKernel(ProjPlan * plan, void * convKernel);
int SetupCPUFFTConvKernel(ProjPlan * plan, void * convKernel);
int ReverbDCCPUProcessing(ProjPlan *plan, __FLOAT__ ** input, __FLOAT__ ** output, int n_samples, int flags);
int ReverbFFTCPUProcessing(ProjPlan *plan, __FLOAT__ ** input, __FLOAT__ ** output, int n_samples, int flags);
void ReverbDirFHTCPUProc(FHT_FUNC fht_routine, __FLOAT__ * FHT_window, __FLOAT__ *new_data, __FLOAT__ *prev_data, __FLOAT__ *tsincos, short * bitrvrs, int N);
int ReverbFHTCPUProcessing(ProjPlan *plan, __FLOAT__ ** input, __FLOAT__ ** output, int n_samples, int flags);

int ReverbCPUConv(ProjPlan *plan, void * conv_data, int conv_data_ln, void * kern_data, void *in_data, int in_data_sz, int start_pos, int conv_window);

int SetupCPUFHTConvKernel(ProjPlan * plan, void * convKernel);

#endif