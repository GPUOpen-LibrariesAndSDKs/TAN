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

#ifndef __REVERBERATE_OCL__
#define __REVERBERATE_OCL__


#include "IIRLowLat.h"
#include "IIRLowLat_OCL.h"




#define __FLOAT__ float

#ifdef _DEBUG
#define __TRACER_SZ__	(1<<12)
#define __TRACER_MASK__	( __TRACER_SZ__ - 1)
#define __TRACER_WRITE__( data, N, E) memcpy(&_amd_data_tracer[_amd_tracer_ptr], data, N*sizeof(uint)); \
	memset(&_amd_event_tracer[_amd_tracer_ptr], E, N);\
	_amd_tracer_ptr += N;\
	_amd_tracer_ptr &= __TRACER_MASK__;
#define __TRACE_RESET__( N )  _amd_tracer_ptr += (N & __TRACER_MASK__);

#if 0 //def __MAIN_TRACER__
uint _amd_data_tracer[__TRACER_SZ__];
char _amd_event_tracer[__TRACER_SZ__];
uint _amd_tracer_ptr = 0;
#else
extern uint _amd_data_tracer[__TRACER_SZ__];
extern char _amd_event_tracer[__TRACER_SZ__];
extern uint _amd_tracer_ptr;
#endif

#else
#define __TRACER_WRITE__( data, N, E)
#define __TRACE_RESET__( N )

#endif

#define __DEFAULT_BLOCK_SIZE__   1024
#define __DEFAULT_NUM_CHANNELS__ 4
#define __DEFAULT_REVERB_LN__    2*124

// 0, 1, reserved
#define __DIRECT_CONV_PROGRAM_INDEX__    2
#define __COMPLEX_MAD_PROGRAM_INDEX__    3
#define __SCATTER_GATHER_PROGRAM_INDEX__ 4
#define __COMPLEX_MAD2_PROGRAM_INDEX__   5
#define __AMD_FHT_PROGRAM_INDEX__        6
#define __AMD_FHTBIG_PROGRAM_INDEX__     7
#define __AMD_FHTMAD_PROGRAM_INDEX__     8
#define __AMD_FHT2_PROGRAM_INDEX__		 9
#define __AMD_FIR_PROGRAM_INDEX__        10
// uniq shared pipeline id 
#define __OCLREVERB_PIPELINE_ID__		(uint)-2
#define __OCLREVERB_TERMINATED_ID__  (uint)-1
// preallocated number of cahnnels (blocks)
#define __PREALLOC_NUM__CHANNELS    2 * 128

#define __STATE_ACTIVE__					1
#define __STATE_SETUP_RVRB__				2
#define __STATE_UNSETUP_RVRB__				4
#define __STATE_ACTIVE_RVRB__				8
#define __STATE_SETUP_FIR__				    16
#define __STATE_UNSETUP_FIR__				32
#define __STATE_ACTIVE_FIR__				64
#define __STATE_ASK_TOINIT__				128
#define __STATE_ABOUTTOTERMINATE__   (uint)0x8000000


typedef struct {


	OCLBuffer stream_in[__NUM_STREAMS__];
	OCLBuffer stream_out[__NUM_STREAMS__];
// 2nd stream seperate output
	OCLBuffer stream2_out[__NUM_STREAMS__];
	uint shrd_out_offset;
	uint shrd_in_offset;
	void ** stream2_data_ptrs;
	cl_event read_event;
} STREAM_INOUT;



typedef struct _ProjPlan {
	int plan_id;
	size_t conv_kernel_ln; // in sample
	size_t conv_buffer1_ln; // original, multiple of frame_ln in samples, might be more than conv_kernel
	size_t conv_buffer_ln; // in samples, might be more than conv_kernel
//	float *conv_norm; // 1./ sum of all element per channel
	void * conv_kernel_sys;
	void ** conv_krnl_chnls_f;
	void ** conv_chnls_f;
	int conv_blocks;    // number conv blocks, if needed;
	int conv_blocks1;    // number conv blocks, if needed;
	int conv_buffer_ln_log2;   // closest log2
	int conv_NumChannels;    //   number of channels to be processed
	int frame_max_ln;
	int frame_max_ln_staged;
	int frame_ln;       // in samples
	int frame_ln_log2;
	int frame2_ln;
	int frame2_ln_log2;
	int in_que_ln;
	int group_ln_log2;
	int num_inputs;
	int num_input_subchannels;
	int num_outputs;
	int num_output_subchannels;
	int nmb_channels;
	int *fir_sz_staged;
	void ** fir_ptr_staged;
	int n_firs_staged;
	int local_mem_buffer_log2; 
	__int64 run;
	int run_flags;
	int nmb_loops;
	void * kernelFFTpass1;
	void * kernelFFTpass;
	void * streamBlockInOutpass;
	void * streamFFTpass1;
	void * streamFFTpass;
	void * complexMADpass;
	void * streamIFFTpass1;
	void * streamIFFTpass;
	void * DirectConvpass;
	void * GPUConvpass;
	void * CPUConvpass;
	void * OclConfig;
	void * AmdSched;
	void * Stat;
	void * FIR;
	void *firArray;
	int verification;
	int timing;
	uint state;
	char * oclexe_path;
	int stream_index;
	void** input_blocks[__NUM_STREAMS__];
	void** output_blocks[__NUM_STREAMS__];
	int nSamples[__NUM_STREAMS__];
	cl_command_queue OCLqueue[__NUM_OCL_QUEUES__];
    unsigned int num_IR_buffers; // The number of IR buffers created by the user to enable IR update, there should be at least three for the current, old and the new (update). The assumption is old buffer canbe reached by subtracting one from the current, and is kept intact during the IR update (crossfade) process.
    int xfade_state;
    int mad2_remaining_bins;
    int first_ir_upload;
} ProjPlan;

void ReverbDebugOutput(ProjPlan *plan, char * msg);
int OCLInit(ProjPlan *plan, uint init_flags, amf::AMFComputePtr amf_compute_conv=0, amf::AMFComputePtr amf_compute_update=0);
int OCLTerminate(ProjPlan *plan);
void DecreaseGlobalReferenceCounter(ProjPlan *plan);
int ReverbOCLInitialize(ProjPlan * plan, amdOCLRvrb *new_plan, const char * ocl_kernels_path, int init_flags, amf::AMFComputePtr amf_compute_conv = 0, amf::AMFComputePtr amf_compute_update = 0);
int ReverbOCLTerminateDeffered(ProjPlan * plan);


HSA_OCL_ConfigS * GetOclConfig(ProjPlan * plan);
cl_context GetOCLContext(ProjPlan *plan);
amdAudShcheduler * GetScheduler(ProjPlan * plan);
ProjPlan * ReverbFromHandle(amdOCLRvrb rvrb);
amdOCLRvrb ReverbFromPtr(ProjPlan * plan);
ProjPlan * CreatePlan(int init_flags);


void graalSetupParams(ProjPlan * plan, int init_flags);
int ReverbOCLInitializeInternal(ProjPlan * plan, const char * ocl_kernels_path, int init_flags);

int GetPlanID(ProjPlan * plan);
int ReverbOCLSetup(ProjPlan * rvrb);

int ReverbOCLSetupProcess(ProjPlan * plan);
void clearBuffers(ProjPlan * plan);
int ReverbOCLSetupReverbKernel(ProjPlan * rvrb);
int ReverbSetBlockSize2(ProjPlan * plan, int flame2_ln);
int ReverbOCLSetupInternal(ProjPlan * plan);

__int64 GetRunCounter(ProjPlan *plan);
void SetRunCounter(ProjPlan *plan, __int64 new_val);
int ReverbGetState(ProjPlan *plan);
void ReverbSetState(ProjPlan *plan, int new_state);
void ReverbOrState(ProjPlan *plan, int new_state);
void ReverbAndState(ProjPlan *plan, int new_state);

int ReverbGetNConvBlocks(ProjPlan * plan);
int ReverbGetNConvBlocksToFFT(ProjPlan * plan);
int ReverbGetNDataBlocksToFFT(ProjPlan * plan);
int ReverbOCLIsActive(ProjPlan * plan);


cl_command_queue GetOCLQueue(ProjPlan *plan);
int GetOCLWavefrontLg2(ProjPlan *plan);

int CreateProgram2(ProjPlan *plan, const char *program_nm, int index, char * options);

int SetValue(cl_command_queue queue, OCLBuffer * buf, cl_float value);
int SetValue(cl_command_queue queue, OCLBuffer * buf, unsigned int value, size_t offset, size_t size);
int ReleaseCollElement(ProjPlan *plan, void * element);


int ComplexMulAddSetup(ProjPlan * plan);
size_t ComplexMulAddMemreq(ProjPlan * plan);
int ComplexMulAddSetupProcess(ProjPlan * plan);


int ComplexMulAddRun(HSA_OCL_ConfigS* config, ProjPlan * plan, uint run);
int ComplexMulAddDeinit(HSA_OCL_ConfigS* config, ProjPlan * plan);
int ReverbFlushHistory(ProjPlan* plan, int channelId);


#endif
