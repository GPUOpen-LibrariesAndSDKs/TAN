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

#ifndef __AMD_FIR_OCL_H__
#define __AMD_FIR_OCL_H__

#include <math.h>



typedef enum {
	FIR_OCL,
	FIR_OCL_KERNELS_TOTAL,
} FIR_OCL_KERNEL;


typedef struct _FIR_plan {

	OCLBuffer data_t;
	OCLBuffer * firs_t;
	int * firs_sz;
    int n_firs;
	cl_kernel FIR_kernel[FIR_OCL_KERNELS_TOTAL];

} FIR_PLAN;
////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////
int firLoad(ProjPlan * plan, firHandle * firHandleArray, __FLOAT__ ** firPtrs, int * fir_sz, int n_firs);


////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////
int firUpload(ProjPlan * plan, __FLOAT__ * firPtr, int fir_sz);

////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////
int firUnload(ProjPlan * plan);


////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////
int firSetup(ProjPlan * plan, ProjPlan * firPlan, __FLOAT__ *firPtr, int fir_sz);

////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////
int firDirectSetup(ProjPlan * plan);

////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////
int firDirectRun(ProjPlan * plan);

////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////
int firDirectDeinit(ProjPlan * plan);


///////////////////////////////////////////////////////////////
//
// move newly arriving block into the pipeline
//
//////////////////////////////////////////////////////////////////
int firMoveDataIntoPipeline(ProjPlan *plan, __FLOAT__ ** input, int numSamples);


////////////////////////////////////////////////////////
//
// FIR stand-alone processing
//
////////////////////////////////////////////////////////
int firDirectProcessing(ProjPlan * plan, __FLOAT__ ** input, __FLOAT__ ** output, int numSamples, int numChans, int* channel_ids, int flags);

#endif