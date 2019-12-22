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

#include "IIRLowLat.h"
#include "Reverberate2_OCL.h"
#include "amdFIR_OCL.h"
#include <iostream>
#include <vector>
#include <omp.h>

#if defined DEFINE_AMD_OPENCL_EXTENSION
#ifndef CL_MEM_USE_PERSISTENT_MEM_AMD
#define CL_MEM_USE_PERSISTENT_MEM_AMD       (1 << 6)
#endif
#endif


////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////

int firVerifyBlock(ProjPlan * plan, __FLOAT__ *g_out, __FLOAT__ *in, __FLOAT__* fir,
					int block_sz, int fir_sz, int in_sz, int in_index, int in_chnl_stride, int fir_chnl_stride, int out_chnl_stride) {
	int err = 0;
	__FLOAT__ *c_out = (__FLOAT__*)malloc(out_chnl_stride * plan->nmb_channels * sizeof(__FLOAT__));
	assert(c_out);
	int l, j, k;
	#pragma omp parallel for private(l,j, k)
	for(int i = 0; i < plan->nmb_channels; i++ ) {
		for( l = 0; l < block_sz; l++ ) {
			c_out[l + out_chnl_stride * i] = 0;
			for( j = 0, k = in_index * block_sz + l; j < fir_sz; j++, k-- ) {
				k = (k < 0) ? in_sz + k : k;
				c_out[l + out_chnl_stride * i] += in[k + in_chnl_stride * i] * fir[j + fir_chnl_stride * i];

			}
		}
	}


	for(int i = 0; i < plan->nmb_channels; i++ ) {
		for( int l = 0;  l < block_sz; l++  ) {
			if ( abs(c_out[l + out_chnl_stride * i] - g_out[l + out_chnl_stride * i]) > 0.0001) {
				printf("FIR filter mismatch: %d %d %f %f\n", i, l, g_out[l + out_chnl_stride * i], c_out[l + out_chnl_stride * i]);
				err |= 1;
				break;
			}
		}
	}

	free(c_out);

	return err;
}
////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////
int firSetup(ProjPlan * plan, ProjPlan * firPlan, __FLOAT__ *firPtr, int fir_sz) {
	int err = 0;
// stop playing
	ReverbSetState(firPlan,(ReverbGetState(firPlan) & ~__STATE_ACTIVE__));

	firPlan->conv_kernel_sys = firPtr;

// the actual kerenel lengih in samples and the block size are major parameters of the pipeline
	firPlan->conv_kernel_ln = fir_sz;


// move a new block size from a cached state into actuallity 
// here there might be some heurisric to switch from a 2 streams to a 1 stream pipline and back.
	ReverbSetBlockSize2(firPlan, 0);


	amdAudShcheduler * sched = GetScheduler(plan);
	int init_flags = SchedGetInitFlags(sched);
/// setup a properinitflags here !!!
// alway 1 stream
	init_flags &= __INIT_FLAG_2STREAMS__;
// alway non-geregenious !!!?????
	init_flags &= __INIT_FLAG_HETEROGEN__;

// running parameters
	graalSetupParams(firPlan, init_flags);


// start playing
// IsActive external interface
	ReverbSetState(firPlan,(ReverbGetState(firPlan) | __STATE_ACTIVE__));
	return err;
}


////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////
int firDirectSetup(ProjPlan * plan) {
	int err = 0;
	plan->FIR = malloc(sizeof(FIR_PLAN));
	assert(plan->FIR);
	memset(plan->FIR ,0, sizeof(FIR_PLAN));
	FIR_PLAN *fir_plan = (FIR_PLAN *)plan->FIR;
	uint init_flags = SchedGetInitFlags(GetScheduler(plan));

	fir_plan->FIR_kernel[FIR_OCL] = CreateOCLKernel2(plan, "amdFIR", __AMD_FIR_PROGRAM_INDEX__);
	cl_context ctxt = GetOCLContext(plan);
	cl_command_queue commandQueue = GetOCLQueue(plan);
	fir_plan->n_firs = plan->n_firs_staged;
	fir_plan->firs_t = (OCLBuffer * )malloc(sizeof(OCLBuffer) * fir_plan->n_firs);
	fir_plan->firs_sz = (int*)malloc(sizeof(int) * fir_plan->n_firs);
	assert(fir_plan->firs_t && fir_plan->firs_sz);

	memset(fir_plan->firs_t, 0, sizeof(OCLBuffer) * fir_plan->n_firs);
	memset(fir_plan->firs_sz, 0, sizeof(int) * fir_plan->n_firs);

// move FIR time domain into video memory
	for( int i = 0; i < fir_plan->n_firs; i++ ) {
		fir_plan->firs_sz[i] = plan->fir_sz_staged[i];
		int n_blocks = (fir_plan->firs_sz[i] + plan->frame_ln - 1) / plan->frame_ln;
		int out_channel_stride = n_blocks * plan->frame_ln;
		fir_plan->firs_t[i].len = out_channel_stride * plan->nmb_channels * sizeof(__FLOAT__);
		
		if ( (err = CreateOCLBuffer(ctxt, &fir_plan->firs_t[i], CL_MEM_READ_ONLY) ) == CL_SUCCESS) {
			int in_channel_stride = fir_plan->firs_sz[i];
			__FLOAT__ * host_side = (__FLOAT__ *)fir_plan->firs_t[i].sys;

		    for(int j = 0; j < plan->nmb_channels; j++) {
				memset(host_side + j*out_channel_stride, 0, out_channel_stride * sizeof(__FLOAT__));
				memcpy(host_side + j*out_channel_stride, (__FLOAT__*)plan->fir_ptr_staged[i] + j * in_channel_stride, in_channel_stride * sizeof(__FLOAT__));
	
			}
			CopyToDevice(commandQueue, &fir_plan->firs_t[i]);

		}
	}

// make the input queue for the time domain filter
	int n_data_blocks = (plan->fir_sz_staged[0] + plan->frame_ln - 1) / plan->frame_ln;
	int data_chnl_stride = n_data_blocks * plan->frame_ln * 2;
	fir_plan->data_t.len = data_chnl_stride * plan->nmb_channels * sizeof(__FLOAT__);

	if ( (err = CreateOCLBuffer2(ctxt, &fir_plan->data_t, CL_MEM_READ_ONLY  | /*CL_MEM_ALLOC_HOST_PTR */ CL_MEM_USE_PERSISTENT_MEM_AMD)) == CL_SUCCESS) {
		SetValue(commandQueue, &fir_plan->data_t, 0.0);
	}
	{


// map input queue for the entire lifespan
			if ( !fir_plan->data_t.sys_map ) {
				fir_plan->data_t.sys_map = (__FLOAT__*)clEnqueueMapBuffer (commandQueue,
 				fir_plan->data_t.mem,
 				CL_TRUE,
 				CL_MAP_WRITE,
 				0,
 				fir_plan->data_t.len,
 				0,
 				NULL,
 				NULL,
 				&err);
			}
			if ( err ) {
				printf("Failed to map input buffers: %d\n", err);
				return (err);
			}
			memset(fir_plan->data_t.sys_map, 0, fir_plan->data_t.len);
	}

	return err;
}


////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////
int firDirectFIR(ProjPlan * plan,
	            cl_mem in,
				cl_mem fir,
				cl_mem out,
				int fir_sz,
				int block_sz,
				int in_index,
				int in_chnl_stride,
				int fir_chnl_stride,
				int out_index,
				int out_chnl_stride,
				cl_kernel fir_kernel) {
	int err = 0;
	int n_arg = 0;
	int n_data_blocks = in_chnl_stride / block_sz;

	cl_command_queue commandQueue = GetOCLQueue(plan);

	size_t l_wk[3] = {64,1,1};
	size_t g_wk[3] = {block_sz * l_wk[0], size_t(plan->nmb_channels),1};

	n_arg = 0;

	err = clSetKernelArg(fir_kernel, n_arg++, sizeof(cl_mem), &in);
	err |= clSetKernelArg(fir_kernel, n_arg++, sizeof(cl_mem), &fir);
	err |= clSetKernelArg(fir_kernel, n_arg++, sizeof(cl_mem), &out);
	err |= clSetKernelArg(fir_kernel, n_arg++, sizeof(int), &fir_sz);
	err |= clSetKernelArg(fir_kernel, n_arg++, sizeof(int),	&block_sz);
	err |= clSetKernelArg(fir_kernel, n_arg++, sizeof(int),	&in_index);
	err |= clSetKernelArg(fir_kernel, n_arg++, sizeof(int), &in_chnl_stride);
	err |= clSetKernelArg(fir_kernel, n_arg++, sizeof(int), &fir_chnl_stride);
	err |= clSetKernelArg(fir_kernel, n_arg++, sizeof(int), &out_index);
	err |= clSetKernelArg(fir_kernel, n_arg++, sizeof(int), &out_chnl_stride);

	if(err != CL_SUCCESS) {
		printf("IR1 in: failed to set kernel arguments with error %d\n", err);
		exit(1);
	}

	err = clEnqueueNDRangeKernel(commandQueue, fir_kernel , 2, NULL, g_wk, l_wk, 0, NULL, NULL);

	if(err != CL_SUCCESS) {
		printf("IR1 in: failed to launch kernel with error %d\n", err);
		exit(1);
	}


	return err;
}



////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////
int firDirectRun(ProjPlan * plan) {
	int err = 0;
	int n_arg = 0;
	int block_sz = plan->frame_ln;
	FIR_PLAN *fir_plan = (FIR_PLAN *)plan->FIR;
	STREAM_INOUT * in_out_plan = (STREAM_INOUT *)plan->streamBlockInOutpass;
	cl_mem fir_out = in_out_plan->stream_out[0].mem;
	int out_index = 0;
	int out_chnl_stride = block_sz;
// arguments
	__int64 current_counter = GetRunCounter(plan);
	int fir_sz = fir_plan->firs_sz[0];


	int in_chnl_stride = fir_plan->data_t.len / (plan->nmb_channels * sizeof(__FLOAT__));
	int n_data_blocks = in_chnl_stride / block_sz;
	int in_index = ( int )(current_counter % n_data_blocks);
	int fir_chnl_stride = fir_plan->firs_t[0].len / (plan->nmb_channels * sizeof(__FLOAT__));



	cl_command_queue commandQueue = GetOCLQueue(plan);
	cl_kernel fir_kernel = fir_plan->FIR_kernel[FIR_OCL];
	size_t l_wk[3] = {64,1,1};
	size_t g_wk[3] = {block_sz * l_wk[0], size_t(plan->nmb_channels),1};

	n_arg = 0;

	err = clSetKernelArg(fir_kernel, n_arg++, sizeof(cl_mem), &fir_plan->data_t.mem);
	err |= clSetKernelArg(fir_kernel, n_arg++, sizeof(cl_mem), &fir_plan->firs_t[0].mem);
	err |= clSetKernelArg(fir_kernel, n_arg++, sizeof(cl_mem), &fir_out);
	err |= clSetKernelArg(fir_kernel, n_arg++, sizeof(int), &fir_sz);
	err |= clSetKernelArg(fir_kernel, n_arg++, sizeof(int),	&block_sz);
	err |= clSetKernelArg(fir_kernel, n_arg++, sizeof(int),	&in_index);
	err |= clSetKernelArg(fir_kernel, n_arg++, sizeof(int), &in_chnl_stride);
	err |= clSetKernelArg(fir_kernel, n_arg++, sizeof(int), &fir_chnl_stride);
	err |= clSetKernelArg(fir_kernel, n_arg++, sizeof(int), &out_index);
	err |= clSetKernelArg(fir_kernel, n_arg++, sizeof(int), &out_chnl_stride);

	if(err != CL_SUCCESS) {
		printf("IR1 in: failed to set kernel arguments with error %d\n", err);
		exit(1);
	}

	err = clEnqueueNDRangeKernel(commandQueue, fir_kernel , 2, NULL, g_wk, l_wk, 0, NULL, NULL);

	if(err != CL_SUCCESS) {
		printf("IR1 in: failed to launch kernel with error %d\n", err);
		exit(1);
	}
	amdAudShcheduler * sched = GetScheduler(plan);
	int init_flags = SchedGetInitFlags(sched);
	if ( plan->run_flags &  __PROCESSING_FLAG_VERIFY_TRANSFORM1__) {
		int err = -1;
		CopyFromDevice(commandQueue, &in_out_plan->stream_out[0]);
		firVerifyBlock(plan,
						(__FLOAT__ *)in_out_plan->stream_out[0].sys,
						(__FLOAT__ *)fir_plan->data_t.sys_map,
						(__FLOAT__*) fir_plan->firs_t[0].sys,
					     block_sz, fir_sz, in_chnl_stride, in_index, in_chnl_stride, fir_chnl_stride, out_chnl_stride);
	

	}

	return err;
}




///////////////////////////////////////////////////////////////
//
// move newly arriving block into the pipeline
//
//////////////////////////////////////////////////////////////////
int firMoveDataIntoPipeline(ProjPlan *plan, __FLOAT__ ** input, int numSamples) {
	int err = 0;
	__FLOAT__ *stream_data = NULL;
	FIR_PLAN *fir_plan = (FIR_PLAN *)plan->FIR;
	int block_sz = plan->frame_ln;
	int in_chnl_stride = fir_plan->data_t.len / (plan->nmb_channels * sizeof(__FLOAT__));
	int n_data_blocks = in_chnl_stride / block_sz;
	__int64 current_counter = GetRunCounter(plan);
	int in_index = ( int )(current_counter % n_data_blocks);

// input data is a queue of input blocks realized as acyclic array per each sub-channel
	stream_data = (__FLOAT__ *)fir_plan->data_t.sys_map;


	for (int j = 0; j < plan->nmb_channels; j++) {
		__FLOAT__ * block_ptr = stream_data + plan->frame_ln * in_index + in_chnl_stride * j;
		memcpy(block_ptr, input[j], numSamples * sizeof(__FLOAT__));
// TODO: MOVE INTO KERNEL
        memset((block_ptr + numSamples), 0, (plan->frame_ln - numSamples) * sizeof(__FLOAT__));

	}



	return (err);
}

////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////
int firDirectProcessing(ProjPlan * plan, __FLOAT__ ** input, __FLOAT__ ** output, int numSamples, int numChans, int* channel_ids, int flags) {
	int err = 0;
	err = firMoveDataIntoPipeline(plan, input, numSamples);
	err |= firDirectRun(plan);
	err |= ReverbOCLProcessingReadSamples(plan,  output, numSamples, numChans, channel_ids, 0);
	return err;
}


////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////
int firUpload(ProjPlan * plan, __FLOAT__ * firPtr, int fir_sz) {
	int ret = 0;
	int n_firs = 1;

	ReverbSetState(plan,(ReverbGetState(plan) & ~__STATE_ACTIVE_FIR__));

	ReverbSetState(plan,(ReverbGetState(plan) | __STATE_SETUP_FIR__ | __STATE_UNSETUP_FIR__));

	firDirectDeinit( plan);

	plan->fir_sz_staged = (int*)malloc(sizeof(int) * n_firs);
	plan->fir_ptr_staged = (void**)malloc(sizeof(void*) * n_firs);
	assert(plan->fir_sz_staged && plan->fir_ptr_staged);
	plan->fir_sz_staged[0] = fir_sz;
	plan->fir_ptr_staged[0] = firPtr;
	plan->n_firs_staged = n_firs;

	ReverbOCLSetupInternal(plan);

	ReverbSetState(plan,(ReverbGetState(plan) & ~( __STATE_SETUP_FIR__ | __STATE_UNSETUP_FIR__)));
	ReverbSetState(plan,(ReverbGetState(plan) | __STATE_ACTIVE_FIR__));
	return ret;
}



////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////
int firDirectDeinit(ProjPlan * plan) {
	int err = 0;
	if ( plan->fir_sz_staged ) {
		free(plan->fir_sz_staged);
		plan->fir_sz_staged = 0;
	}
	if ( plan->fir_ptr_staged ) {
		free(plan->fir_ptr_staged);
	}

	plan->n_firs_staged = 0;
	FIR_PLAN *fir_plan = (FIR_PLAN *)plan->FIR;
	if ( fir_plan ) {
		ReleaseOCLBuffer(&fir_plan->data_t);

		for (int i = 0; i < fir_plan->n_firs; i++ ) {
			ReleaseOCLBuffer(&fir_plan->firs_t[i]);
		}

		if ( fir_plan->firs_t ) {
			free(fir_plan->firs_t);
			fir_plan->firs_t = 0;
		}
		if ( fir_plan->firs_sz ) {
			free(fir_plan->firs_sz);
			fir_plan->firs_sz = 0;
		}
		free(plan->FIR);

		plan->FIR = 0;
	}

	return err;
}
#if 0
////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////
int firAllocSort(ProjPlan * plan, ProjPlan **Firs, int * fir_sz, int n_firs) {
	int err = 0;
	amdAudShcheduler * sched = GetScheduler(plan);
	Firs = (ProjPlan **)malloc(n_firs * sizeof(ProjPlan *));
	assert(Firs);
// sort in ascend order
// allocate in descend order
	for ( int i = 0; i < n_firs; i++ ) {
		ProjPlan *newFir = CreatePlan(SchedGetInitFlags(sched));
		assert(newFir);
        ReverbOCLInitializeInternal(newFir, plan->oclexe_path, SchedGetInitFlags(sched));
		newFir->frame_max_ln_staged = plan->frame_max_ln_staged;
	    newFir->OclConfig = plan->OclConfig;
		newFir->AmdSched = plan->AmdSched;
		newFir->OCLqueue[0] = plan->OCLqueue[0];
		Firs[i] = newFir;
	}

	return err;
}
#endif
////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////
int firLoad(ProjPlan * plan, firHandle * firHandleArray, __FLOAT__ ** firPtrs, int * fir_sz, int n_firs) {
	int ret = 0;

	if (plan->firArray) {
// unload
		plan->firArray = 0;
	}
	amdAudShcheduler * sched = GetScheduler(plan);
    std:: vector<ProjPlan*> * firArray = new std:: vector<ProjPlan*>;

	for(int i = 0; i < n_firs; i++) {
		ProjPlan *newFir = CreatePlan(SchedGetInitFlags(sched));
        ReverbOCLInitializeInternal(newFir, plan->oclexe_path, SchedGetInitFlags(sched));
		newFir->frame_max_ln_staged = plan->frame_max_ln_staged;
	    newFir->OclConfig = plan->OclConfig;
		newFir->AmdSched = plan->AmdSched;
		newFir->OCLqueue[0] = plan->OCLqueue[0];
// setup here

		firSetup(plan, newFir, firPtrs[i], fir_sz[i]);
		firArray->push_back(newFir);
		firHandleArray[i] = (firHandle)newFir;
	}
	plan->firArray = firArray;
	return ret;
}


////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////
int firUnload(ProjPlan * plan) {
	int ret = 0;

	if (plan->firArray) {
// unload
		std:: vector<ProjPlan*> * firArray = (std:: vector<ProjPlan*> *)plan->firArray;
		while(firArray->size() > 0 ) {
			ProjPlan *newFir = firArray->back();
		// clean here
			firArray->pop_back();
		}
		delete firArray;
		plan->firArray = 0;
	}

	return ret;
}