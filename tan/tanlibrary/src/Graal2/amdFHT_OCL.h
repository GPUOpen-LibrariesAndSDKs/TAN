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

#ifndef __AMD_FHT_OCL_H__
#define __AMD_FHT_OCL_H__

#include "amdFHT.h"


// http://www.elektronika.kvalitne.cz/ATMEL/necoteorie/transformation/AVRFHT/AVRFHT.html
/* DHT convolution
Z[k] =  (X[k] *( Y[k] + Y[N-k]) + X[N-k] * ( Y[k] - Y[N-k] ) ) / 2 
Z[N-k] = (X[N-k] * ( Y[k] + Y[N-k]) - X[k] * ( Y[k] - Y[N-k] ) ) / 2
*/

typedef enum {
    FHT_OCL_XFADE_OVER = 0,
    FHT_OCL_XFADE_STREAM1_IN_PROGRESS,
    FHT_OCL_XFADE_STREAM2_IN_PROGRESS,
    FHT_OCL_XFADE_PENDING,
} FHT_OCL_XFADE_STATE;

typedef enum {
	FHT1_OCL,
	FHT1_OCL_FROMQUE_REVERSE,
	FHT1_OCL_FROMQUE,
	FHT1_OCL_FROMSTREAM_REVERSE,
	FHT1_OCL_FROMSTREAM,
    FHT1_OCL_FROMSTREAM_XFADE,
	FHT1_OCL_MAD,
	FHT1_OCL_MAD_HEAD,
    FHT1_OCL_MAD_HEAD_XFADE,
    FHT1_OCL_MAD_DEVIDED,
    FHT1_OCL_MAD_DEVIDED_XFADE,
	FHT1_OCL_MAD_TAIL,
    FHT1_OCL_MAD_TAIL_XFADE,
	FHT2_OCL,
	FHT2_OCL_FROMQUE_REVERSE,
	FHT2_OCL_FROMQUE,
	FHT2_OCL_FROMSTREAM_REVERSE,
	FHT2_OCL_FROMSTREAM,
    FHT2_OCL_FROMSTREAM_XFADE,
	FHT2_OCL_MAD,
	FHTG_OCL,
    PARTITION_KERNEL,
	FHT2_OCL_KERNELS_TOTAL,
} FHT_OCL_KERNEL;


typedef struct s_amdFHT_OCL{
	// kernels

	cl_kernel fht_kernels[FHT2_OCL_KERNELS_TOTAL];
	OCLBuffer* IR1;
	OCLBuffer* IR2;
	OCLBuffer bireverse1;
	OCLBuffer bireverse2;
	OCLBuffer sincos1;
	OCLBuffer sincos2;
	OCLBuffer dirFHT10;
	OCLBuffer dirFHT11;
	OCLBuffer dirFHT20[2];
	OCLBuffer dirFHT21;
	OCLBuffer invFHT10[2];// For the first convolution stream. One used for the old IR one for new
	OCLBuffer invFHT11;
	OCLBuffer invFHT20[2];// For the second convolution stream. One used for the old IR one for new
	OCLBuffer accum10;
	OCLBuffer accum11[2];// For the first convolution stream. One used for the old IR one for new
	OCLBuffer accum20;
	OCLBuffer accum21[2];// For the second convolution stream. One used for the old IR one for new
    OCLBuffer updateChannelMap;
    OCLBuffer convChannelMap;
	OCLBuffer syncs;
	int accum_loops;
    __FLOAT__ * IR1_host;
    __FLOAT__ * IR2_host;
	void *FHT_routine1;
	void *FHT_routine2;
	cl_event read_event;
	void * fhtmad_data;
} amdFHT_OCL;


int ReverbOCLSetupFHT( ProjPlan * plan, int numChans, int* channel_ids, unsigned int _ir_version);
int ReverbOCLCopyResponse(ProjPlan* plan, int numChans, unsigned int _from_ir_version, unsigned int _to_ir_version, int* _channel_ids);
int ReverbOCLDeinitFHT( ProjPlan * plan);

int ReverbOCLRunFHT1(ProjPlan * plan, unsigned int _ir_version, int numChans );
int ReverbOCLRunFHT2(ProjPlan * plan, unsigned int _ir_version, int numChans );
int ReverbOCLRunDirFHT1onCPU(ProjPlan * plan, __FLOAT__ ** input, int numChan);
int ReverbOCLRunInvFHT1onCPU(ProjPlan * plan, __FLOAT__ ** output, int numChan);

void manageXFadeState(ProjPlan * plan, int xfade_flag);
bool stream1XfadeInProgress(ProjPlan * plan);

#endif