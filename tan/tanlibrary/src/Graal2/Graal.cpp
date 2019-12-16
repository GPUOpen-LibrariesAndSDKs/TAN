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

#define __MAIN_TRACER__
#include "public/common/Thread.h"
#include "IIRLowLat.h"
#include "Reverberate2_OCL.h"
#include "amdFIR_OCL.h"
#include "amdFHT_OCL.h"

//amf::AMFCriticalSection  m_sectUpdate;

/***************************************************************************************
Reverb interface
****************************************************************************************/

ProjPlan * ReverbFromHandle(amdOCLRvrb rvrb) {
	return((ProjPlan *)rvrb);
}

amdOCLRvrb ReverbFromPtr(ProjPlan * plan) {
	return((amdOCLRvrb)plan);
}

StatisticsPerRun * GetStat(amdOCLRvrb rvrb) {
	ProjPlan * plan = ReverbFromHandle(rvrb);
	StatisticsPerRun * ret = (StatisticsPerRun *)plan->Stat;
	return ret;
}




int graalInitialize(graalHandle *new_plan, const char * ocl_kernels_path, int init_flags, 
                    amf::AMFComputePtr amf_compute_conv, amf::AMFComputePtr amf_compute_update){
int err = 0;
ProjPlan * plan = NULL;

    if ( !new_plan ) {
		err = -1;
		return(err);
	}


	plan = CreatePlan(init_flags);
// reverbirate conv kernel initialization
	
	ReverbOCLInitialize(plan, new_plan, ocl_kernels_path, init_flags, amf_compute_conv, amf_compute_update);


    return(err);
}



int graalTerminate(graalHandle rvrb) {
	int err = 0;
	ProjPlan * plan = ReverbFromHandle(rvrb);

	ReverbOCLTerminateDeffered(plan);
	if( plan->Stat) {
		free(plan->Stat);
	}

	OCLTerminate(plan);

	free(plan);	
	return err;
}

int graalReverbSetBlockSize(amdOCLRvrb rvrb, int block_size) {
	int ret = 0;
	ProjPlan * plan = ReverbFromHandle(rvrb);

// cache it in case it'll be changed before the kernel is loaded.
	plan->frame_max_ln_staged = block_size;


	return ret;
}

int graalReverbSetNChannels(amdOCLRvrb rvrb, int n_input_channels, int n_input_subchannels, int n_output_channels, int n_output_subchannels) {
	int ret = 0;
	ProjPlan * plan = ReverbFromHandle(rvrb);
	plan->nmb_channels = n_input_channels * n_input_subchannels;
	plan->num_inputs = n_input_channels;
	plan->num_input_subchannels = n_input_subchannels;
	return ret;
}

int graalReverbGetBlockSize(amdOCLRvrb rvrb) {
	int ret = 0;
	ProjPlan * plan = ReverbFromHandle(rvrb);
	ret = plan->frame_max_ln;
	return ret;
}

int graalReverbGetNChannels(amdOCLRvrb rvrb) {
	int ret = 0;
	ProjPlan * plan = ReverbFromHandle(rvrb);
	ret = plan->nmb_channels;
	return ret;
}

int graalReverbIsActive(amdOCLRvrb rvrb) {
int ret = 0;

    ret = ReverbOCLIsActive(ReverbFromHandle(rvrb));

	return ret;
}

///////////////////////////////////////////////////////////////
//
// the main entry point.
// calls different pipeline inplemetetion depending on the 
// instansiation flags and (possiblly) on some hueristics
//
//////////////////////////////////////////////////////////////////
int graalReverbProcessing(amdOCLRvrb rvrb, __FLOAT__ ** input, __FLOAT__ ** output, cl_mem* cl_output, int numSamples, int numChans, int* channel_ids, unsigned int _ir_version, int flags, int xfade_state) {
	int err = 0;
	ProjPlan *plan = ReverbFromHandle(rvrb);
	StatisticsPerRun * stat = GetStat(rvrb);
	err = ReverbOCLProcessing(plan, input, output, cl_output, numSamples, numChans, channel_ids, _ir_version, flags, stat, xfade_state);
	return (err);
}

__int64 ReverbOCLGetRunCounter(amdOCLRvrb rvrb) {
	return(GetRunCounter(ReverbFromHandle(rvrb)));
}

int graalCopyPrevIRsProc(amdOCLRvrb rvrb, int numChans, unsigned int _from_ir_version, unsigned int _to_ir_version, int* _channel_ids)
{
    int err = 0;
    ProjPlan * plan = ReverbFromHandle(rvrb);
    uint init_flags = SchedGetInitFlags(GetScheduler(plan));
    if (init_flags & __INIT_FLAG_FHT__)
    {
        err = ReverbOCLCopyResponse(plan, numChans, _from_ir_version, _to_ir_version, _channel_ids);
        // TODO: Implement for hetergenous case
        //if (init_flags & __INIT_FLAG_HETEROGEN__)
    }
    return err;
}

int graalIRUploadDoProc(amdOCLRvrb rvrb, int numChans, int* _channel_ids, unsigned int _ir_version)
{
    int err = 0;
    ProjPlan * plan = ReverbFromHandle(rvrb);
    uint init_flags = SchedGetInitFlags(GetScheduler(plan));
    if (init_flags & __INIT_FLAG_FHT__)
    {
        //amf::AMFLock lock(&m_sectUpdate);
        err = ReverbOCLMoveKernelIntoFHTPipeline2(plan, numChans, _channel_ids, _ir_version, init_flags);
        if (init_flags & __INIT_FLAG_HETEROGEN__)
            cpuHeterogenouosProcessing(plan, numChans, init_flags);
    }
    return err;
}


int graalReverbSetupReverbKernelFromHostBuffers(
    amdOCLRvrb rvrb,
    int _n_channels,
    int* _channel_ids,
    unsigned int _ir_version,
    float** _conv_ptrs,  // arbitrary host ptrs
    const int*  _conv_lens
)
{
    int err = 0;
    ProjPlan * plan = ReverbFromHandle(rvrb);

    // actual setup
    uint init_flags = SchedGetInitFlags(GetScheduler(plan));
    if (init_flags & __INIT_FLAG_FHT__)
    {
        prepareKernels(plan, init_flags, _conv_ptrs, _n_channels, _channel_ids, _ir_version, _conv_lens);
    }

    return(err);
}

int graalReverbSetupKernelFromContiguousOCLBuffer(
    amdOCLRvrb rvrb,
    int _n_channels,
    int* _channel_ids,
    unsigned int _ir_version,
    const cl_mem  _conv_ptr,  
    const int  _max_conv_lens
)
{
    int err = 0;
    ProjPlan * plan = ReverbFromHandle(rvrb);

    // actual setup

    uint init_flags = SchedGetInitFlags(GetScheduler(plan));
    if (init_flags & __INIT_FLAG_FHT__)
    {
        prepareKernels(plan, init_flags, _conv_ptr, _n_channels, _channel_ids, _ir_version, _max_conv_lens);
    }

    return(err);
}

///////////////////////////////////////////////////////////////////////
//  Receives information about the IRs and setsup the OCL
//
//
//////////////////////////////////////////////////////////////////////////

int graalInit(amdOCLRvrb rvrb, int kernelSize, int numChannels, unsigned int n_ir_buffers)
{
    int err = 0;
    ProjPlan * plan = ReverbFromHandle(rvrb);
    // the actual kerenel lengih in samples and the block size are major parameters of the pipeline
    plan->conv_kernel_ln = kernelSize;
    plan->conv_NumChannels = numChannels;
    plan->num_IR_buffers = n_ir_buffers;
    err = ReverbOCLSetupReverbKernel(plan);
    return(err);
}

///////////////////////////////////////////////////////////////////
//
// 
////////////////////////////////////////////////////////////////////
/*
* Must NOT be called asynchronously with the convolution process function calls
*/
int graalFlush(graalHandle rvrb, int channelId)
{
    int ret = 0;
    ProjPlan * plan = ReverbFromHandle(rvrb);
    ret = ReverbFlushHistory(plan, channelId);
    return ret;
}

/************************************************************
*** FIRs
************************************************************/

///////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////
int graalLoadFIRs(graalHandle rvrb, firHandle * firHandleArray, __FLOAT__ ** firPtrs, int * fir_sz, int n_firs) {
	int ret = 0;
	ProjPlan * plan = ReverbFromHandle(rvrb);
	ret = firLoad(plan, firHandleArray, firPtrs, fir_sz, n_firs);
	return(ret);
}

///////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////
int graalFIRUpload(graalHandle rvrb, __FLOAT__ * firPtr, int fir_sz) {
	int ret = 0;
	ProjPlan * plan = ReverbFromHandle(rvrb);
	ret = firUpload(plan, firPtr, fir_sz);
	return ret;
}

///////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////
int graalUnloadFIRs(graalHandle rvrb) {
	int ret = 0;
	ProjPlan * plan = ReverbFromHandle(rvrb);
	ret = firUnload(plan);
	return ret;
}
