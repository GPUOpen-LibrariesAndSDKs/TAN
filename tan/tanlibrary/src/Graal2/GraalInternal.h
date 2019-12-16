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

#ifndef __AMDGRAALINTERNAL_OCL__
#define __AMDGRAALINTERNAL_OCL__

#include <CL/cl.h>
#include "Graal.h"

__int64 ReverbOCLGetRunCounter(amdOCLRvrb rvrb);


//////////////////////////////////////////////////////////////////////////
// the same as above but data is randomly generated
///////////////////////////////////////////////////////////////////////////
int ReverbOCLSetupReverbKernelFromProc(amdOCLRvrb rvrb, const char * proc, int n_samples, int n_subchannels, int bitPersample);




///////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////
int graalLoadFIRs(graalHandle rvrb, firHandle * firHandleArray, __FLOAT__ ** firPtrs, int * fir_sz, int n_firs);


///////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////
int graalUnloadFIRs(graalHandle rvrb);


/*-------------------------------------------------------------------

-------------------------------------------------------------------*/
void ReverbOCLDebugOutput(amdOCLRvrb rvrb, char * msg) ;

/*------------------------------------------------------------------
-------------------------------------------------------------------*/
typedef struct _StatisticsPerRun {
	double processing_time; // in samples
} StatisticsPerRun;

StatisticsPerRun * GetStat(amdOCLRvrb rvrb);
/*-------------------------------------------------------------------
------------------------------------------------------------------*/
#endif
