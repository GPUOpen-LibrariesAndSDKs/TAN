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
#include "IIRLowLat_OCL.h"
#include "DirectConv_OCL.h"
#include "amdFHT_OCL.h"

#define min(a,b)(a < b ? a : b)

float m_dataBuff[128];
static const int MAD_CYCLE_OFFSET = 2;
static const int IFHT_CYCLE_OFFSET = 1;

static const int XFADE_MAD_CYCLE_OFFSET = 3;
static const int XFADE_IFHT_OLD_IR_CYCLE_OFFSET = 2;


void manageXFadeState(ProjPlan * plan, int xfade_flag)
{
    if (xfade_flag)
    {
        if (plan->xfade_state != FHT_OCL_XFADE_OVER)
        {
            printf("A new xfade_flag comes in, while xfade is still in process, cannot be hanled\n");
            return;
        }
        plan->xfade_state = FHT_OCL_XFADE_PENDING; // crossfade process starts
        if (plan->first_ir_upload)
        {
            // Make the exception for the first IR upload only, go to the last step quickly, no need to process with the old IR which is empty.
            plan->first_ir_upload = 0;
            plan->xfade_state = FHT_OCL_XFADE_STREAM1_IN_PROGRESS;
        }
    }
    int num_cycles = 1 << (plan->frame2_ln_log2 - plan->frame_ln_log2);
    int curr_cycle = GetRunCounter(plan) % num_cycles;
    if (plan->xfade_state == FHT_OCL_XFADE_STREAM1_IN_PROGRESS)
    {
        plan->xfade_state = FHT_OCL_XFADE_OVER;// if last state of the crossfade process is done, just reset the flag and crossfade is over
    }
    if (curr_cycle == 0)
    {

        if (plan->xfade_state == FHT_OCL_XFADE_STREAM2_IN_PROGRESS)
        {
            plan->xfade_state = FHT_OCL_XFADE_STREAM1_IN_PROGRESS; // crossfade process moves to a new state, where the double MAD for the first (smaller) convolution stream is done, it takes a single cycle to be over
        }
        if (plan->xfade_state == FHT_OCL_XFADE_PENDING)
        {
            plan->xfade_state = FHT_OCL_XFADE_STREAM2_IN_PROGRESS; // crossfade process moves to a new state, where the double MAD for the second (larger) convolution stream starts, it takes num_cycles for finishing the crossfade of the second stream
        }

    }
}

bool stream2XfadeInProgress(ProjPlan * plan)
{
    return (plan->xfade_state == FHT_OCL_XFADE_STREAM2_IN_PROGRESS);
}
bool stream1XfadeInProgress(ProjPlan * plan)
{
    int curr_cycle = GetRunCounter(plan) % (1 << (plan->frame2_ln_log2 - plan->frame_ln_log2));
    return (plan->xfade_state == FHT_OCL_XFADE_STREAM1_IN_PROGRESS && curr_cycle == 0);
}
bool stream1XfadePending(ProjPlan * plan)
{
    return (plan->xfade_state == FHT_OCL_XFADE_PENDING || plan->xfade_state == FHT_OCL_XFADE_STREAM2_IN_PROGRESS);
}
bool stream2XfadePending(ProjPlan * plan)
{
    return (plan->xfade_state == FHT_OCL_XFADE_PENDING);
}
///////////////////////////////////////////////////////////////
//
// Copy processed IRs from an older IR buffer to a new IR buffer. Used as part of the stop channel capability
//
//////////////////////////////////////////////////////////////////

int ReverbOCLCopyResponse(ProjPlan* plan, int numChans, unsigned int _from_ir_version, unsigned int _to_ir_version, int* _channel_ids)
{
    int err = 0;
    amdFHT_OCL *fht_plan = (amdFHT_OCL *)plan->GPUConvpass;

    uint init_flags = SchedGetInitFlags(GetScheduler(plan));

    cl_command_queue queue = plan->OCLqueue[1];// general queue// GetOCLQueue(plan);
    for (int i = 0; i < numChans; i++)
    {
        int n_conv_blocks = (init_flags & __INIT_FLAG_2STREAMS__) ? plan->in_que_ln : plan->conv_blocks;
        int block_sz = plan->frame_ln * 2;
        int chnl_stride = n_conv_blocks * block_sz * sizeof(cl_float);
        int offset = chnl_stride*_channel_ids[i];
        err |= clEnqueueCopyBuffer(queue, fht_plan->IR1[_from_ir_version].mem, fht_plan->IR1[_to_ir_version].mem, offset, offset, chnl_stride, 0, NULL, NULL);
    }
    if (init_flags & __INIT_FLAG_2STREAMS__)
    {
        for (int i = 0; i < numChans; i++)
        {
            int block_sz = plan->frame2_ln * 2;
            int n_conv_blocks = plan->conv_blocks;
            int chnl_stride = n_conv_blocks * block_sz * sizeof(cl_float);
            int offset = chnl_stride*_channel_ids[i];
            err |= clEnqueueCopyBuffer(queue, fht_plan->IR2[_from_ir_version].mem, fht_plan->IR2[_to_ir_version].mem, offset, offset, chnl_stride, 0, NULL, NULL);
        }
    }
    return err;
}
///////////////////////////////////////////////////////////////
//
// load a kernel into the GPU pipeline
//
//////////////////////////////////////////////////////////////////
int ReverbOCLSetupFHT(ProjPlan * plan, int numChans, int* _channel_ids, unsigned int _ir_version) {
    
	int err = 0;

	amdFHT_OCL *fht_plan = (amdFHT_OCL *)plan->GPUConvpass;

	uint init_flags = SchedGetInitFlags(GetScheduler(plan));

	//hack orig  cl_command_queue queue = plan->OCLqueue[1];// general queue// GetOCLQueue(plan);
	//hack this makes startup loud cl_command_queue queue = plan->OCLqueue[0];// general queue// GetOCLQueue(plan); //hack
	//clFinish(plan->OCLqueue[1]);
	//
	//cl_command_queue queue = plan->OCLqueue[0];
	//cl_command_queue queue2 = plan->OCLqueue[1];

	cl_command_queue queue = plan->OCLqueue[1];

	err |= CopyToDevice2(queue, &fht_plan->updateChannelMap, _channel_ids, fht_plan->updateChannelMap.len, 0, false);
	//hack err |= CopyToDevice2(queue2, &fht_plan->updateChannelMap, _channel_ids, fht_plan->updateChannelMap.len, 0, false);
	//clFinish(queue2);

	 {

// IR1
		__FLOAT__ *sincos1;
		int block_sz =  plan->frame_ln * 2;
// input queue length either 2 for a single stream or
// multiple * 2 for 2 streams
		int n_conv_blocks = ( init_flags & __INIT_FLAG_2STREAMS__ ) ? plan->in_que_ln : plan->conv_blocks;
	
		size_t l_wk[3] = {256,1,1};
		size_t g_wk[3] = {1,1,1};

		int wk_grp_per_fhtblock = 1;


		l_wk[0] = min(block_sz/2, 256);

		g_wk[0] = l_wk[0]*wk_grp_per_fhtblock*n_conv_blocks;
		g_wk[1] = numChans;

		cl_kernel ir1_kernel = fht_plan->fht_kernels[FHT1_OCL];

// run direct FHT for the 1st stream
		int n_arg = 0;
// direct FHT
		__FLOAT__ scale = (__FLOAT__)1.;

		int chnl_stride = n_conv_blocks * block_sz;
		err = clSetKernelArg(ir1_kernel, n_arg++, sizeof(cl_mem), &(fht_plan->IR1[_ir_version].mem));
		err |= clSetKernelArg(ir1_kernel, n_arg++, sizeof(cl_mem), &(fht_plan->IR1[_ir_version].mem));
		err |= clSetKernelArg(ir1_kernel, n_arg++, sizeof(cl_mem), &fht_plan->bireverse1.mem);
		err |= clSetKernelArg(ir1_kernel, n_arg++, sizeof(cl_mem), &fht_plan->sincos1.mem);
        err |= clSetKernelArg(ir1_kernel, n_arg++, sizeof(cl_mem), &(fht_plan->updateChannelMap.mem));
		err |= clSetKernelArg(ir1_kernel, n_arg++, sizeof(__FLOAT__), &scale);
		err |= clSetKernelArg(ir1_kernel, n_arg++, sizeof(__FLOAT__), &block_sz);
		err |= clSetKernelArg(ir1_kernel, n_arg++, sizeof(__FLOAT__), &chnl_stride);
		if(err != CL_SUCCESS) {
			printf("IR1 in: failed to set kernel arguments with error %d\n", err);
			return -1;
		}

		err = clEnqueueNDRangeKernel(queue, ir1_kernel , 2, NULL, g_wk, l_wk, 0, NULL, NULL);

		if(err != CL_SUCCESS) {
			printf("IR1 in: failed to launch kernel with error %d\n", err);
			return -1;
		}
// verification

		if ( false ) {
			int err = -1;
			CopyFromDevice(queue, &(fht_plan->IR1[_ir_version]));
			for ( int i = 0; i < n_conv_blocks * numChans; i++) {
				//err = FHT_verify((const __FLOAT__ *) FHT1_in + block_sz, (const __FLOAT__ *)fht_plan->IR1.sys + block_sz, block_sz, 0, block_sz, (__FLOAT__)1. );
				if ( err >= 0 ) {
					break;
				}
			}
			if ( err < 0 ) {
				printf("IR1 FHT %d %d is correct\n",block_sz,n_conv_blocks*2);
			}
		}

	}

#if 1
// IR2 
	if ( init_flags & __INIT_FLAG_2STREAMS__ ) {
		__FLOAT__ *sincos2;
		short *bit_reverse2;
		int block_sz =  plan->frame2_ln * 2;
		int n_conv_blocks = plan->conv_blocks;

		size_t l_wk[3] = {256,1,1};
		size_t g_wk[3] = {1,1,1};

		int wk_grp_per_fhtblock = 1;
	
		g_wk[0] = l_wk[0]*wk_grp_per_fhtblock*n_conv_blocks;
		g_wk[1] = numChans;
	
		cl_kernel ir2_kernel = fht_plan->fht_kernels[FHT2_OCL];

// run direct FHT for the 1st stream
	int n_arg = 0;
// direct FHT
		__FLOAT__ scale = (__FLOAT__)1.;
	
		int chnl_stride = n_conv_blocks * block_sz;
		err = clSetKernelArg(ir2_kernel, n_arg++, sizeof(cl_mem), &(fht_plan->IR2[_ir_version].mem));
		err |= clSetKernelArg(ir2_kernel, n_arg++, sizeof(cl_mem), &(fht_plan->IR2[_ir_version].mem));
		err |= clSetKernelArg(ir2_kernel, n_arg++, sizeof(cl_mem), &fht_plan->bireverse2.mem);
		err |= clSetKernelArg(ir2_kernel, n_arg++, sizeof(cl_mem), &fht_plan->sincos2.mem);
        err |= clSetKernelArg(ir2_kernel, n_arg++, sizeof(cl_mem), &(fht_plan->updateChannelMap.mem));
		err |= clSetKernelArg(ir2_kernel, n_arg++, sizeof(__FLOAT__), &scale);
		err |= clSetKernelArg(ir2_kernel, n_arg++, sizeof(__FLOAT__), &block_sz);
		err |= clSetKernelArg(ir2_kernel, n_arg++, sizeof(__FLOAT__), &chnl_stride);

		if(err != CL_SUCCESS) {
			printf("IR2 in: failed to set kernel arguments with error %d\n", err);
			return -1;
		}

		err = clEnqueueNDRangeKernel(queue, ir2_kernel , 2, NULL, g_wk, l_wk, 0, NULL, NULL);

		if(err != CL_SUCCESS) {
			printf("IR2 in: failed to launch kernel with error %d\n", err);
			return -1;
		}
// verification
		
		if ( false ) {
			int err = -1;
			CopyFromDevice(queue, &(fht_plan->IR2[_ir_version]));
			for ( int i = 0; i < n_conv_blocks * numChans; i++) {
				//err = FHT_verify((const __FLOAT__ *) FHT2_in + block_sz, (const __FLOAT__ *)fht_plan->IR2.sys + block_sz, block_sz, 0, block_sz, (__FLOAT__)1. );
				if ( err >= 0 ) {
					break;
				}
			}
			if ( err < 0 ) {
				printf("IR2 FHT %d %d is correct\n",block_sz,n_conv_blocks*2);
			}
		}
	}

#endif    
	clFinish(queue); //hack
	return err;
}




///////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////

int ReverbOCLRunDirFHT1onCPU(ProjPlan * plan, __FLOAT__ ** input, int numChans) {
	int err = 0;

// than write out current input 
	STREAM_INOUT * in_out_plan = (STREAM_INOUT *)plan->streamBlockInOutpass;
	amdFHT_OCL *fht_plan = (amdFHT_OCL *)plan->GPUConvpass;
	DirectConvPass *DCPass = (DirectConvPass*)plan->CPUConvpass;

	short * bitrvrs = (short*)DCPass->FHT_bitrvrs;
	__FLOAT__ *tsincos = (__FLOAT__ *)DCPass->FHT_sincos;

	__int64 last_counter = GetRunCounter(plan);
	int curr_que_index =  (last_counter) % plan->in_que_ln;
	int prev_que_index = (curr_que_index - 1  < 0 )  ? plan->in_que_ln - 1 : curr_que_index - 1;
int curr_stream = 0;

cl_command_queue commandQueue = GetOCLQueue(plan);
    if ( !in_out_plan->stream_in[curr_stream].sys_map ) {
		in_out_plan->stream_in[curr_stream].sys_map = (__FLOAT__*)clEnqueueMapBuffer (commandQueue,
 			in_out_plan->stream_in[curr_stream].mem,
 		    CL_TRUE,
 			CL_MAP_WRITE,
 			0,
 			in_out_plan->stream_in[curr_stream].len,
 			0,
 			NULL,
 			NULL,
 			&err);
	}

// input data is a queue of input blocks realized as acyclic array per each sub-channel
	__FLOAT__ * stream_data = (__FLOAT__ *)in_out_plan->stream_in[curr_stream].sys_map;
	__FLOAT__ * fht_data = (__FLOAT__*)fht_plan->dirFHT11.sys_map;

// do direct FHT transfer on CPU
// push transformed data into FHT1 pipeline

	for (int j = 0; j < numChans; j++) {
		DirectCPUConvChannel* chnl_conv = &DCPass->channels[j];
		__FLOAT__ *new_data = input[j];
				// the prev block of incoming data
		__FLOAT__ *prev_data = 	(__FLOAT__ *)chnl_conv->data_window;
		__FLOAT__ * curr_block_ptr = stream_data + plan->frame_ln * (curr_que_index + plan->in_que_ln * j);

		__FLOAT__ *FHT_window = (__FLOAT__ *)chnl_conv->FFT_data_window;
		__FLOAT__ * fht_block = fht_data + plan->frame_ln * 2 * j;

		((FHT_DIRFUNC)DCPass->FHT_proc[0])(FHT_window, new_data, prev_data, tsincos, bitrvrs);
		  // move new into previous
		memcpy(curr_block_ptr, new_data, plan->frame_ln * sizeof(__FLOAT__));
		memcpy(prev_data, new_data, plan->frame_ln * sizeof(__FLOAT__));
		memcpy(fht_block,FHT_window,plan->frame_ln * 2 * sizeof(__FLOAT__)); 

	}

	return (err);
}

///////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////

int ReverbOCLRunInvFHT1onCPU(ProjPlan * plan, __FLOAT__ ** output, int numChans) {
	int err = 0;

// than write out current input 

	amdFHT_OCL *fht_plan = (amdFHT_OCL *)plan->GPUConvpass;
	DirectConvPass *DCPass = (DirectConvPass*)plan->CPUConvpass;
	short * bitrvrs = (short*)DCPass->FHT_bitrvrs;
	__FLOAT__ *tsincos = (__FLOAT__ *)DCPass->FHT_sincos;


// do mad with the block
// add the tail
// do inverse FHT of the sum

	int N = plan->frame_ln * 2;

	for (int j = 0; j < numChans; j++) {
		DirectCPUConvChannel* chnl_conv = &DCPass->channels[j];
		__FLOAT__ *FHT_IR = (__FLOAT__*)chnl_conv->FFT_conv_window;
		__FLOAT__* add_block = (__FLOAT__*)fht_plan->fhtmad_data + plan->frame_ln * 2 * j;
		__FLOAT__ *FHT_window = (__FLOAT__ *)chnl_conv->FFT_data_window;
		__FLOAT__ *FHT_window2 = (__FLOAT__*)chnl_conv->FHT_data2_window;
		((FHT_INVFUNC)DCPass->FHT_proc[1])(FHT_window2, FHT_window, FHT_IR, tsincos, bitrvrs, add_block);

			__FLOAT__ * out_data = output[j];
		for(int i = 0; i < N/2; i++ ) {
			out_data[i] = (FHT_window2[i] / ((__FLOAT__) N * 2));
		}
	}

	return (err);
}

///////////////////////////////////////////////////////////////
//
// does MAD with uniform pipeline - summation
// 
//////////////////////////////////////////////////////////////////
static
int ReverbOCLRunFHT1Mad1(ProjPlan * plan, unsigned int _ir_version, int numChans) {
	int err = 0;
// RUN FHT MAD with accumulation
	cl_command_queue queue = GetOCLQueue(plan);
	uint init_flags = SchedGetInitFlags(GetScheduler(plan));
	size_t l_wk[3] = {256,1,1};
	size_t g_wk[3] = {1,1,1};

	amdFHT_OCL *fht_plan = (amdFHT_OCL *)plan->GPUConvpass;
    cl_kernel fhtmad_kernel = stream1XfadeInProgress(plan) ? fht_plan->fht_kernels[FHT1_OCL_MAD_HEAD_XFADE] : fht_plan->fht_kernels[FHT1_OCL_MAD_HEAD]; 

	__int64 current_counter = GetRunCounter(plan);
    int total_n_bins = (init_flags & __INIT_FLAG_2STREAMS__) ? plan->in_que_ln : plan->conv_blocks;// Number of bins for MAD is different between uniform and non-uniform cases
	int block_sz = plan->frame_ln * 2;
	int current_bin =  (current_counter) % total_n_bins; //  % __NUMBER_BINS__
	int chnl_stride = total_n_bins * block_sz;
	int IR_bin_shift = ( init_flags & __INIT_FLAG_HETEROGEN__ ) ? 1 : 0;
	int first_interval = 1;
	int n_loop_bins = fht_plan->accum_loops;
	//total_n_bins -= IR_bin_shift;
	int current_run = (total_n_bins + n_loop_bins - 1) / n_loop_bins;
	int accum_chnl_stride = current_run * block_sz;
	
	cl_mem dirFHT = (init_flags & __INIT_FLAG_FHT1_ONCPU__ ) ? fht_plan->dirFHT11.mem : fht_plan->dirFHT10.mem;
	cl_mem invFHT = (init_flags & __INIT_FLAG_FHT1_ONCPU__ ) ? fht_plan->invFHT11.mem : fht_plan->invFHT10[0].mem;

	l_wk[0] = min(block_sz/2, 256);

	int wk_grp_per_fhtblock = (block_sz/2) / l_wk[0];
	g_wk[0] = l_wk[0]*wk_grp_per_fhtblock;
	g_wk[1] = current_run;
	g_wk[2] = numChans;

    int old_ir_version = (_ir_version - 1 + plan->num_IR_buffers) % plan->num_IR_buffers;
    int ir_version = stream1XfadePending(plan) ? old_ir_version : _ir_version;

	int n_arg = 0;

    err = clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(cl_mem), &(fht_plan->IR1[ir_version].mem));
	err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(cl_mem), &dirFHT);
	err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(cl_mem), &fht_plan->accum10.mem);// The history input FHT frames 
	err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(cl_mem), &fht_plan->accum11[0].mem);
    err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(cl_mem), &fht_plan->convChannelMap.mem);
	err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(int), &accum_chnl_stride);
	err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(int), &chnl_stride);
	err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(int), &IR_bin_shift);
	err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(int), &n_loop_bins);
	err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(int), &total_n_bins);
	err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(int), &current_bin);
    
    if(stream1XfadeInProgress(plan))
    {
        err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(cl_mem), &(fht_plan->IR1[old_ir_version].mem));
        err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(cl_mem), &fht_plan->accum11[1].mem);
    }
    if(err != CL_SUCCESS) {
      printf("FHTMAD1 head: failed to set kernel arguments with error %d\n", err);
      return -1;
    }

	err = clEnqueueNDRangeKernel(queue, fhtmad_kernel , 3, NULL, g_wk, l_wk, 0, NULL, NULL);

    if(err != CL_SUCCESS) {
        printf("FHTMAD1 head: failed to launch kernel with error %d\n", err);
        return -1;
    }

	if ( current_run >= 1 ) {

		fhtmad_kernel = stream1XfadeInProgress(plan) ? fht_plan->fht_kernels[FHT1_OCL_MAD_TAIL_XFADE] : fht_plan->fht_kernels[FHT1_OCL_MAD_TAIL];
		total_n_bins = current_run;
		current_run = (total_n_bins + n_loop_bins - 1) / n_loop_bins;

		l_wk[0] = min(block_sz, 256);
		int wk_grp_per_fhtblock = (block_sz / l_wk[0]);
		g_wk[0] = l_wk[0]*wk_grp_per_fhtblock;	

		while(true) {


			g_wk[1] = current_run;

			int n_arg = 0;
			err = clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(cl_mem), &fht_plan->accum11[0].mem);
			err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(cl_mem), &invFHT);
            err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(cl_mem), &fht_plan->convChannelMap.mem);
			err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(int), &accum_chnl_stride);
			err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(int), &n_loop_bins);
			err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(int), &total_n_bins);
            if (stream1XfadeInProgress(plan))
            {
                err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(cl_mem), &fht_plan->accum11[1].mem);
                err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(cl_mem), &fht_plan->invFHT10[1].mem);
            }
			if(err != CL_SUCCESS) {
				printf("FHTMAD1 tail: failed to set kernel arguments with error %d\n", err);
				return -1;
			}

			err = clEnqueueNDRangeKernel(queue, fhtmad_kernel , 3, NULL, g_wk, l_wk, 0, NULL, NULL);

			if(err != CL_SUCCESS) {
				printf("FHTMAD1 tail: failed to launch kernel with error %d\n", err);
				return -1;
			}

			if ( current_run ==1 ) {
				break;
			}
			total_n_bins = current_run;
			current_run = (total_n_bins + n_loop_bins - 1) / n_loop_bins;
		}
	}
	return (err);
}
///////////////////////////////////////////////////////////////
//
// does MAD with non-uniform pipeline - accumulation
//
//////////////////////////////////////////////////////////////////
#if 0
static
int ReverbOCLRunFHT1Mad2(ProjPlan * plan, unsigned int _ir_version, int numChans, int xfade_state ) {
	int err = 0;
// RUN FHT MAD with accumulation
	cl_command_queue queue = GetOCLQueue(plan);
	uint init_flags = SchedGetInitFlags(GetScheduler(plan));
	size_t l_wk[3] = {256,1,1};
	size_t g_wk[3] = {1,1,1};

	amdFHT_OCL *fht_plan = (amdFHT_OCL *)plan->GPUConvpass;
	cl_kernel fhtmad_kernel = fht_plan->fht_kernels[FHT1_OCL_MAD];
	__int64 current_counter = GetRunCounter(plan);
	int total_n_bins =  plan->in_que_ln;
	int block_sz = plan->frame_ln * 2;
	int current_bin =  (current_counter) % total_n_bins; //  % __NUMBER_BINS__
	int chnl_stride = total_n_bins * block_sz;
	int IR_bin_shift = ( init_flags & __INIT_FLAG_HETEROGEN__ ) ? 1 : 0;
	int first_interval = 1;

//	__int64 current_counter = GetRunCounter(plan);
	
	cl_mem dirFHT = (init_flags & __INIT_FLAG_FHT1_ONCPU__ ) ? fht_plan->dirFHT11.mem : fht_plan->dirFHT10.mem;
	cl_mem invFHT = (init_flags & __INIT_FLAG_FHT1_ONCPU__ ) ? fht_plan->invFHT11.mem : fht_plan->invFHT10.mem;

	l_wk[0] = min(block_sz/2, 256);

	int wk_grp_per_fhtblock = (block_sz/2) / l_wk[0];
	g_wk[0] = l_wk[0]*wk_grp_per_fhtblock;
	g_wk[1] = total_n_bins - IR_bin_shift;
	g_wk[2] = numChans;

	int n_arg = 0;

    err = clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(cl_mem), &(fht_plan->IR1[_ir_version].mem));
	err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(cl_mem), &dirFHT);
	err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(cl_mem), &fht_plan->accum10.mem);
	err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(cl_mem), &invFHT);
	err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(int), &chnl_stride);
	err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(int), &block_sz);
	err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(int), &IR_bin_shift);
	err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(int), &first_interval);
	err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(int), &total_n_bins);
	err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(int), &current_bin);

    if(err != CL_SUCCESS) {
      printf("FHTMAD1 2: failed to set kernel arguments with error %d\n", err);
      return -1;
    }

	err = clEnqueueNDRangeKernel(queue, fhtmad_kernel , 3, NULL, g_wk, l_wk, 0, NULL, NULL);

    if(err != CL_SUCCESS) {
        printf("FHTMAD1 2: failed to launch kernel with error %d\n", err);
        return -1;
    }

	return (err);
}
#endif
///////////////////////////////////////////////////////////////
//
// algorithm:
// the kernel is presetup is a following fashion:
// split into bins size of block_sz (see), second half is padded with 0 (on the right)
// direct FHT has been performed over each bin

// on each newly block arrival
// it's appended with the previous block (on the right)
// direct FHT is performed

// uniform convolution:
// the resulting block is moved into the FHHTStore cyclic array buffer according to the block's current counter modulo the number of kernel bins.
// MAD is performed between kernel bins and FHTStore bins
// the 0 kernel bin MADs with the newly arrived bin,
// the 1s kerenel bin MADs with a (newly arrived - 1) FHTStrore bin,
// the 2nd kerenel bin MADs with a (newly arrived - 2) FHTStrore bin,
// etc.
// All resulting bins are summed up
// the sum undergoes the inverse FHT
//
// non-uniform convolution:
// stream 1 - MAD with muliplier*2 number of bins;
// stream 2 - keep up to multiplier number of input blocks;
//            do forward transform over the large block every (round + 1) % multiplier == 0
//            do MAD with only 1 / multiplier number of big blocks
//            do inverse transform over resulting sum every (round + 1) % multiplier == 0
//            sum with stream 2 only one block from a large stream 2 block at index round % multiplier.

//////////////////////////////////////////////////////////////////
int ReverbOCLRunFHT1(ProjPlan * plan, unsigned int _ir_version, int numChans) {
	int err = 0;

	uint init_flags = SchedGetInitFlags(GetScheduler(plan));

	int in_que_ln = plan->in_que_ln;
	int total_n_bins =  ( init_flags & __INIT_FLAG_2STREAMS__ ) ? plan->in_que_ln : plan->conv_blocks;					  
	__int64 current_counter = GetRunCounter(plan);
	int current_bin = 0; //  % __NUMBER_BINS__
	cl_command_queue queue = GetOCLQueue(plan);

    
	size_t l_wk[3] = {256,1,1};
	size_t g_wk[3] = {1,1,1};

	amdFHT_OCL *fht_plan = (amdFHT_OCL *)plan->GPUConvpass;
// direct FHT
	cl_kernel dirfht1_kernel = fht_plan->fht_kernels[FHT1_OCL_FROMQUE];


	int block_sz = plan->frame_ln * 2;
	int block_sz_log2 = plan->frame_ln_log2 + 1;
	int wk_grp_per_fhtblock = 1;
	int local_mem_sz = 0;

	l_wk[0] = min(block_sz/2, 256);

	STREAM_INOUT * in_out_plan = (STREAM_INOUT *)(plan->streamBlockInOutpass);
	int curr_stream = 0;
	__FLOAT__ scale = (__FLOAT__)1.;
	int chnl_stride = plan->in_que_ln * (block_sz/2);
	int n_arg = 0;

	if ( !(init_flags & __INIT_FLAG_FHT1_ONCPU__ )) { 


		g_wk[0] = l_wk[0]*wk_grp_per_fhtblock;
		g_wk[1] = numChans;

		current_bin =  (current_counter) % plan->in_que_ln; //  % __NUMBER_BINS__



// input channels

		err = clSetKernelArg(dirfht1_kernel, n_arg++, sizeof(cl_mem), &in_out_plan->stream_in[curr_stream].mem);
		err |= clSetKernelArg(dirfht1_kernel, n_arg++, sizeof(cl_mem), &fht_plan->dirFHT10.mem);
		err |= clSetKernelArg(dirfht1_kernel, n_arg++, sizeof(cl_mem), &fht_plan->bireverse1.mem);
		err |= clSetKernelArg(dirfht1_kernel, n_arg++, sizeof(cl_mem), &fht_plan->sincos1.mem);
        err |= clSetKernelArg(dirfht1_kernel, n_arg++, sizeof(cl_mem), &fht_plan->convChannelMap.mem);
		err |= clSetKernelArg(dirfht1_kernel, n_arg++, sizeof(__FLOAT__), &scale);
		err |= clSetKernelArg(dirfht1_kernel, n_arg++, sizeof(int), &chnl_stride);
		err |= clSetKernelArg(dirfht1_kernel, n_arg++, sizeof(int), &plan->in_que_ln);
		err |= clSetKernelArg(dirfht1_kernel, n_arg++, sizeof(int), &plan->frame_ln);
		err |= clSetKernelArg(dirfht1_kernel, n_arg++, sizeof(int), &current_bin);

		if(err != CL_SUCCESS) {
			printf("FHT1 in: failed to set kernel arguments with error %d\n", err);
			return -1;
		}

		err = clEnqueueNDRangeKernel(queue, dirfht1_kernel , 2, NULL, g_wk, l_wk, 0, NULL, NULL);

		if(err != CL_SUCCESS) {
			printf("FHT1 in: failed to launch kernel with error %d\n", err);
			return -1;
		}

// verification

	
		if ( plan->run_flags &  __PROCESSING_FLAG_VERIFY_TRANSFORM1__) {
			int err = -1;
			CopyFromDevice(queue, &in_out_plan->stream_in[curr_stream]);
			CopyFromDevice(queue, &fht_plan->dirFHT10);
			__FLOAT__ * data_in = (__FLOAT__*)malloc(plan->frame_ln * 2 * numChans * sizeof(__FLOAT__));
// extract input data
			int buf0_off = current_bin * plan->frame_ln;
			int buf1_off = ((current_bin - 1) < 0 ? plan->in_que_ln - 1 : (current_bin - 1)) * plan->frame_ln;

			for(int i = 0; i < numChans; i++ ) {
				int chnl_off = i * plan->frame_ln * plan->in_que_ln;
	
				memcpy(data_in,(__FLOAT__*)in_out_plan->stream_in[curr_stream].sys + buf0_off +  chnl_off, plan->frame_ln * sizeof(__FLOAT__));
					memcpy(data_in + plan->frame_ln,(__FLOAT__*)in_out_plan->stream_in[curr_stream].sys + buf1_off +  chnl_off, plan->frame_ln * sizeof(__FLOAT__));
				err = FHT_verify((const __FLOAT__ *) data_in, (const __FLOAT__ *)fht_plan->dirFHT10.sys + i*plan->frame_ln*2, plan->frame_ln*2, 0, plan->frame_ln*2, (__FLOAT__)1. );
				if ( err >= 0 ) {
					break;
				}


			}
			if ( err < 0 ) {
				printf("Data1 dir FHT %d %d is correct\n", plan->frame_ln*2, numChans);
			}

			free(data_in);
		}

	}

// MAD
    // Direct summation is used for both uniform and non-uniform
    if (ReverbOCLRunFHT1Mad1(plan, _ir_version, numChans) != 0)
    {
        printf("First stream MAD failed\n");
        return -1;
    }
     


// RUN inverse FHT
// verification

	if ( !(init_flags & __INIT_FLAG_FHT1_ONCPU__ )) {

		if ( init_flags & __INIT_FLAG_VER_TRANSFORM1__) {
			CopyFromDevice(queue, &fht_plan->invFHT10[0]);
		}
		int multi = plan->frame2_ln_log2 - plan->frame_ln_log2;
		__int64 current_counter = GetRunCounter(plan);

		STREAM_INOUT * in_out_plan = (STREAM_INOUT *)plan->streamBlockInOutpass;

// output stream chnl stride
	    int stream2_chnl_stride = ((plan->frame_ln) << multi);
		cl_kernel invfht_kernel = stream1XfadeInProgress(plan) ? fht_plan->fht_kernels[FHT1_OCL_FROMSTREAM_XFADE] : fht_plan->fht_kernels[FHT1_OCL_FROMSTREAM];
		n_arg = 0;
		block_sz = plan->frame_ln * 2;
		scale = (__FLOAT__)1. / (__FLOAT__)block_sz;
		chnl_stride = block_sz;
		block_sz_log2 = plan->frame_ln_log2 + 1;
		local_mem_sz = block_sz * 2;

		err = clSetKernelArg(invfht_kernel, n_arg++, sizeof(cl_mem), &fht_plan->invFHT10[0].mem);
		err |= clSetKernelArg(invfht_kernel, n_arg++, sizeof(cl_mem), &in_out_plan->stream_out[0].mem);
		err |= clSetKernelArg(invfht_kernel, n_arg++, sizeof(cl_mem), &fht_plan->bireverse1.mem);
		err |= clSetKernelArg(invfht_kernel, n_arg++, sizeof(cl_mem), &fht_plan->sincos1.mem);
        err |= clSetKernelArg(invfht_kernel, n_arg++, sizeof(cl_mem), &fht_plan->convChannelMap.mem);
		err |= clSetKernelArg(invfht_kernel, n_arg++, sizeof(__FLOAT__), &scale);
		err |= clSetKernelArg(invfht_kernel, n_arg++, sizeof(int), &chnl_stride);
		err |= clSetKernelArg(invfht_kernel, n_arg++, sizeof(int), &block_sz);
		err |= clSetKernelArg(invfht_kernel, n_arg++, sizeof(int), &current_bin);
//		err |= clSetKernelArg(invfht_kernel, n_arg++, sizeof(int), &block_sz_log2);
// local mem
//		err |= clSetKernelArg(invfht_kernel, n_arg++, local_mem_sz * sizeof(__FLOAT__), NULL);
        if (stream1XfadeInProgress(plan))
        {
            err = clSetKernelArg(invfht_kernel, n_arg++, sizeof(cl_mem), &fht_plan->invFHT10[1].mem);
        }
		if ( init_flags & __INIT_FLAG_2STREAMS__ ) {
// for 2 streams the second stream's addition is inside FHT inverse
// HETEROGENIOUS CASE: it's a previous output buffer!!!
// select 2nd stream output
			curr_stream = (init_flags & __INIT_FLAG_HETEROGEN__) ? (((current_counter+1) >> multi) & 1) : ((current_counter >> multi) & 1);
// select 2 stream output sub-block
			int index2 =  (init_flags & __INIT_FLAG_HETEROGEN__) ? (int)((current_counter+1) % (__int64)(1 << multi)) : (int)(current_counter % (__int64)(1 << multi));
			err |= clSetKernelArg(invfht_kernel, n_arg++, sizeof(cl_mem), &in_out_plan->stream2_out[curr_stream].mem);
			err |= clSetKernelArg(invfht_kernel, n_arg++, sizeof(int), &index2);
			err |= clSetKernelArg(invfht_kernel, n_arg++, sizeof(int), &stream2_chnl_stride);
		}
		if(err != CL_SUCCESS) {
			printf("INVFHT1 out: failed to set kernel arguments with error %d\n", err);
			return -1;
		}

		wk_grp_per_fhtblock = 1;

		l_wk[0] = min(block_sz/2, 256);		

		g_wk[0] = l_wk[0]*wk_grp_per_fhtblock;
		g_wk[1] = numChans;
		g_wk[2] = 1;

		err = clEnqueueNDRangeKernel(queue, invfht_kernel , 2, NULL, g_wk, l_wk, 0, NULL, NULL);

		if(err != CL_SUCCESS) {
			printf("INVFHT1 out: failed to launch kernel with error %d\n", err);
			return -1;
		}

// verification


		if (  plan->run_flags &  __PROCESSING_FLAG_VERIFY_TRANSFORM1__) {
			int err = -1;
			CopyFromDevice(queue, &in_out_plan->stream_out[0]);
            CopyFromDevice(queue, &fht_plan->invFHT10[0]);

			for(int i = 0; i < numChans; i++ ) {
				int chnl_off = i * block_sz/2 ;
				err = FHT_verify((const __FLOAT__ *) fht_plan->invFHT10[0].sys + i * block_sz, (const __FLOAT__ *)in_out_plan->stream_out[0].sys + i*block_sz/2, block_sz, 1, block_sz/2, (__FLOAT__)0.5 );
				if ( err >= 0 ) {
					break;
				}

			}

			if ( err < 0 ) {
				printf("Data1 inv FHT %d %d is correct\n", block_sz, numChans);
			}

		}
	}


	return (err);
}

int MADSummationDevided2(ProjPlan * plan, unsigned int _ir_version, int numChans, int curr_cycle)
{
    int err = 0;
    int IR_bin_shift = 2;
    int multi = plan->frame2_ln_log2 - plan->frame_ln_log2;
    int num_cycles = (1 << multi) - (stream2XfadeInProgress(plan) ? XFADE_MAD_CYCLE_OFFSET : MAD_CYCLE_OFFSET);
    
    if (curr_cycle >= num_cycles)
        return 0;
    int block_sz = plan->frame2_ln * 2;
    int n_conv_blocks = plan->conv_blocks;
    int current_bin = (GetRunCounter(plan) >> multi) % n_conv_blocks; //  % __NUMBER_BINS__
    cl_command_queue queue = GetOCLQueue(plan);
    uint init_flags = SchedGetInitFlags(GetScheduler(plan));
    size_t l_wk[3] = { 256,1,1 };
    size_t g_wk[3] = { 1,1,1 };
    amdFHT_OCL *fht_plan = (amdFHT_OCL *)plan->GPUConvpass;
    cl_kernel fhtmad_kernel = stream2XfadeInProgress(plan) ? fht_plan->fht_kernels[FHT1_OCL_MAD_DEVIDED_XFADE] : fht_plan->fht_kernels[FHT1_OCL_MAD_DEVIDED]; 
    int chnl_stride = n_conv_blocks * block_sz;
    int total_n_bins = n_conv_blocks;
    int first_cyle = (curr_cycle == 0);
    int last_cyle = (curr_cycle == (num_cycles - 1));
    if (curr_cycle == 0)
    {
        // Set the total workload at the starting cycle
        plan->mad2_remaining_bins = total_n_bins - IR_bin_shift;
    }

    int cycles_remaining = num_cycles - curr_cycle;
    int this_workload = ceil(plan->mad2_remaining_bins / cycles_remaining);
    int bin_offset = plan->mad2_remaining_bins - this_workload;
    l_wk[0] = min(block_sz / 2, 256);

    int wk_grp_per_fhtblock = (block_sz / 2) / l_wk[0];
    g_wk[0] = l_wk[0] * wk_grp_per_fhtblock;
    g_wk[1] = numChans;
    g_wk[2] = 1;
    int curr_stream = ((GetRunCounter(plan) >> multi) & 1);
    int old_ir_version = (_ir_version - 1 + plan->num_IR_buffers) % plan->num_IR_buffers;
    int ir_version = stream2XfadePending(plan) ? old_ir_version : _ir_version;
    int n_arg = 0;
    err = clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(cl_mem), &(fht_plan->IR2[ir_version].mem));
    err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(cl_mem), &fht_plan->dirFHT20[curr_stream].mem);
    err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(cl_mem), &fht_plan->accum20.mem);
    err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(cl_mem), &fht_plan->accum21[0].mem);
    err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(cl_mem), &fht_plan->invFHT20[0].mem);
    err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(cl_mem), &fht_plan->convChannelMap.mem);
    err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(int), &chnl_stride);
    err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(int), &IR_bin_shift);
    err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(int), &total_n_bins);
    err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(int), &current_bin);
    err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(int), &bin_offset);
    err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(int), &this_workload);
    err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(int), &first_cyle);
    err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(int), &last_cyle);
    if (stream2XfadeInProgress(plan))
    {
        err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(cl_mem), &(fht_plan->IR2[old_ir_version].mem));
        err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(cl_mem), &fht_plan->accum21[1].mem);
        err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(cl_mem), &fht_plan->invFHT20[1].mem);
    }
    

    if (err != CL_SUCCESS)
    {
        printf("FHTMAD1 head: failed to set kernel arguments with error %d\n", err);
        return -1;
    }

    err = clEnqueueNDRangeKernel(queue, fhtmad_kernel, 2, NULL, g_wk, l_wk, 0, NULL, NULL);
    // Forcing the kernel execution to be launched on GPU as soon as possible
    err |= clEnqueueReadBuffer(queue, fht_plan->accum21[0].mem, CL_FALSE, 0, 8 * sizeof(float), m_dataBuff, 0, NULL, NULL);

    plan->mad2_remaining_bins -= this_workload;
    if (err != CL_SUCCESS)
    {
        printf("FHTMAD1 head: failed to launch kernel with error %d\n", err);
        return -1;
    }
}

int applyInvFHT2(ProjPlan * plan, int numChans, int ifhtBufIdx, int do_xfade )
{
    amdFHT_OCL *fht_plan = (amdFHT_OCL *)plan->GPUConvpass;
    STREAM_INOUT * in_out_plan = (STREAM_INOUT *)(plan->streamBlockInOutpass);
    int multi = plan->frame2_ln_log2 - plan->frame_ln_log2;
    __int64 current_counter = GetRunCounter(plan);
    __int64 next_counter = current_counter + 1;

    /// switch output buffer
    int curr_stream = ((current_counter >> multi) & 1);
    int next_stream = curr_stream ? 0 : 1;
    int block_sz = plan->frame2_ln * 2;
    cl_kernel invfht_kernel = (do_xfade != 0) ? fht_plan->fht_kernels[FHT2_OCL_FROMSTREAM_XFADE] : fht_plan->fht_kernels[FHT2_OCL_FROMSTREAM];
    int n_arg = 0;

    __FLOAT__ scale = (__FLOAT__)1. / (__FLOAT__)block_sz;
    int chnl_stride = block_sz;
    int current_bin = 0;
    size_t l_wk[3] = { 256,1,1 };
    size_t g_wk[3] = { 1,1,1 };
    int wk_grp_per_fhtblock = 1;
    cl_command_queue queue = GetOCLQueue(plan);
    cl_mem* output = &in_out_plan->stream2_out[next_stream].mem ;

    int err = clSetKernelArg(invfht_kernel, n_arg++, sizeof(cl_mem), &fht_plan->invFHT20[ifhtBufIdx].mem);
    err |= clSetKernelArg(invfht_kernel, n_arg++, sizeof(cl_mem), output);
    err |= clSetKernelArg(invfht_kernel, n_arg++, sizeof(cl_mem), &fht_plan->bireverse2.mem);
    err |= clSetKernelArg(invfht_kernel, n_arg++, sizeof(cl_mem), &fht_plan->sincos2.mem);
    err |= clSetKernelArg(invfht_kernel, n_arg++, sizeof(cl_mem), &fht_plan->convChannelMap.mem);
    err |= clSetKernelArg(invfht_kernel, n_arg++, sizeof(__FLOAT__), &scale);
    err |= clSetKernelArg(invfht_kernel, n_arg++, sizeof(int), &chnl_stride);
    err |= clSetKernelArg(invfht_kernel, n_arg++, sizeof(int), &block_sz);
    err |= clSetKernelArg(invfht_kernel, n_arg++, sizeof(int), &current_bin);
    if (do_xfade != 0)
    {
        int xfade_span = plan->frame_ln;
        err |= clSetKernelArg(invfht_kernel, n_arg++, sizeof(int), &xfade_span);
    }
    if (err != CL_SUCCESS)
    {
        printf("INVFHT2 out: failed to set kernel arguments with error %d\n", err);
        return -1;
    }

    wk_grp_per_fhtblock = 1;
    g_wk[0] = l_wk[0] * wk_grp_per_fhtblock;
    g_wk[1] = numChans;
    g_wk[2] = 1;

    err = clEnqueueNDRangeKernel(queue, invfht_kernel, 2, NULL, g_wk, l_wk, 0, NULL, NULL);
    // Forcing the kernel execution to be launched on GPU as soon as possible
    err |= clEnqueueReadBuffer(queue, *output, CL_FALSE, 0, 8 * sizeof(float), m_dataBuff, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("INVFHT2 out: failed to launch kernel with error %d\n", err);
        return -1;
    }
}
///////////////////////////////////////////////////////////////
//
//How it works:
// on each period - 1
// direct transform of the 2nd stream
// the first MAD of the new transform but at position run_sz * (period-1)
// send a previous MAD out
// next run will overwrite the stage buffer
//
//////////////////////////////////////////////////////////////////
int ReverbOCLRunFHT2(ProjPlan * plan, unsigned int _ir_version,int numChans) {
	int err = 0;
	uint init_flags = SchedGetInitFlags(GetScheduler(plan));

	amdFHT_OCL *fht_plan = (amdFHT_OCL *)plan->GPUConvpass;
	STREAM_INOUT * in_out_plan = (STREAM_INOUT *)(plan->streamBlockInOutpass);
	int curr_stream = 0;
	int n_arg = 0;
	cl_command_queue queue = GetOCLQueue(plan);
// 2nd, main stream, first block
    int multi = plan->frame2_ln_log2 - plan->frame_ln_log2;
	__int64 current_counter = GetRunCounter(plan);
	__int64 next_counter = current_counter + 1;
	int n_conv_blocks = plan->conv_blocks;
	int block_sz = plan->frame2_ln*2;
	size_t l_wk[3] = {256,1,1};
	size_t g_wk[3] = {1,1,1};
	int wk_grp_per_fhtblock = 1;
    int num_cycles = (1 << multi);
    int next_cycle = next_counter % num_cycles;
    int curr_cycle = current_counter % num_cycles;
    // Enough samples are accumulated, time to update 2nd stream
	if ( next_cycle == 0) {
        // First, move the frame into second stream pipeline. This processing cycle is exclusively used for FHT calculation, no MAD or IFHT is done in this cycle
        int curr_stream = ((next_counter >> multi) & 1);
		int in_que_ln = 2;
	    __int64 stream2_index = (current_counter / (__int64)( 1 << multi));
		int que_index = (int)(stream2_index % in_que_ln);
// direct FHT
		cl_kernel dirfht_kernel = fht_plan->fht_kernels[FHT2_OCL_FROMQUE];
		int chnl_stride = in_que_ln * block_sz/2;
        chnl_stride = block_sz;
        __FLOAT__ scale = (__FLOAT__)1.;
		{
			g_wk[0] = l_wk[0]*wk_grp_per_fhtblock;
			g_wk[1] = numChans;
			n_arg = 0;
			err |= clSetKernelArg(dirfht_kernel, n_arg++, sizeof(cl_mem), &in_out_plan->stream_in[0].mem);
			err |= clSetKernelArg(dirfht_kernel, n_arg++, sizeof(cl_mem), &fht_plan->dirFHT20[curr_stream].mem);
			err |= clSetKernelArg(dirfht_kernel, n_arg++, sizeof(cl_mem), &fht_plan->bireverse2.mem);
			err |= clSetKernelArg(dirfht_kernel, n_arg++, sizeof(cl_mem), &fht_plan->sincos2.mem);
            err |= clSetKernelArg(dirfht_kernel, n_arg++, sizeof(cl_mem), &fht_plan->convChannelMap.mem);
			err |= clSetKernelArg(dirfht_kernel, n_arg++, sizeof(__FLOAT__), &scale);
			err |= clSetKernelArg(dirfht_kernel, n_arg++, sizeof(int), &chnl_stride);
			err |= clSetKernelArg(dirfht_kernel, n_arg++, sizeof(int), &in_que_ln);
			err |= clSetKernelArg(dirfht_kernel, n_arg++, sizeof(int), &plan->frame2_ln);
			err |= clSetKernelArg(dirfht_kernel, n_arg++, sizeof(int), &que_index);
			if(err != CL_SUCCESS) {
				printf("Data2 dir FHT in: failed to set kernel arguments with error %d\n", err);
				return -1;
			}

			err = clEnqueueNDRangeKernel(queue, dirfht_kernel , 2, NULL, g_wk, l_wk, 0, NULL, NULL);
            // Forcing the kernel execution to be launched on GPU as soon as possible
            clEnqueueReadBuffer(queue, fht_plan->dirFHT20[curr_stream].mem, CL_FALSE, 0, 8 * sizeof(float), m_dataBuff, 0, NULL, NULL);
		}


		if(err != CL_SUCCESS) {
			printf("Data2 dir FHT in: failed to launch kernel with error %d\n", err);
			return -1;
		 }

// verification

	
		if ( plan->run_flags &  __PROCESSING_FLAG_VERIFY_TRANSFORM2__) {
			int err = -1;
			CopyFromDevice(queue, &in_out_plan->stream_in[0]);
			CopyFromDevice(queue, &fht_plan->dirFHT20[curr_stream]);
			__FLOAT__ * data_in = (__FLOAT__*)malloc(block_sz * numChans * sizeof(__FLOAT__));
// extract input data
			int buf0_off = que_index * block_sz/2;
			int buf1_off = ((que_index - 1) < 0 ? in_que_ln - 1 : (que_index - 1)) * block_sz/2;

			for(int i = 0; i < numChans; i++ ) {
				int chnl_off = i * in_que_ln * block_sz/2;
	
				memcpy(data_in,(__FLOAT__*)in_out_plan->stream_in[0].sys + buf0_off +  chnl_off, (block_sz/2) * sizeof(__FLOAT__));
				memcpy(data_in + block_sz/2,(__FLOAT__*)in_out_plan->stream_in[0].sys + buf1_off +  chnl_off, (block_sz/2) * sizeof(__FLOAT__));
				err = FHT_verify((const __FLOAT__ *) data_in, (const __FLOAT__ *)fht_plan->dirFHT20[curr_stream].sys + i*block_sz, block_sz, 0, block_sz, (__FLOAT__)1. );
				if ( err >= 0 ) {
					break;
				}

			}
			if ( err < 0 ) {
				printf("Data2 dir FHT %d %d is correct\n",block_sz, numChans);
			}

			free(data_in);
		}

	}




	{
#if 0
// RUN FHT MAD with accumulation
		cl_kernel fhtmad_kernel = fht_plan->fht_kernels[FHT2_OCL_MAD];
		int chnl_stride = n_conv_blocks * block_sz;
		int total_n_bins = n_conv_blocks;
        int curr_stream = ((next_counter >> multi) & 1);
		int IR_bin_shift = 2;
		int run_sz = ((total_n_bins - IR_bin_shift + (1 << multi) - 1) >> multi);
	
		__int64 current_counter2 = (current_counter >> multi);

		int run_offset = run_sz * (int)(current_counter % (__int64)( 1 << multi));
		int current_bin =  current_counter2 % n_conv_blocks + run_offset; //  % __NUMBER_BINS__
		int current_IR_shift = IR_bin_shift + run_offset;
		current_bin = (current_bin >= n_conv_blocks) ? current_bin - n_conv_blocks : current_bin;
// can be negative for a small kernel and large FHT
		int current_run = ((n_conv_blocks - IR_bin_shift - run_offset) > run_sz)
							? run_sz
							: (n_conv_blocks  - IR_bin_shift - run_offset);
		if ( current_run > 0 ) {
	// 0 == mod(1<< multi)
		int first_interval = ((current_counter2 << multi) == current_counter);

		wk_grp_per_fhtblock = (1 << (plan->frame2_ln_log2 - 8));

		g_wk[0] = l_wk[0]*wk_grp_per_fhtblock;
		g_wk[1] = current_run;
		g_wk[2] = numChans;

		int n_arg = 0;

		err = clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(cl_mem), &(fht_plan->IR2[_ir_version].mem));
		err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(cl_mem), &fht_plan->dirFHT20[curr_stream].mem);
		err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(cl_mem), &fht_plan->accum20.mem);
		err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(cl_mem), &fht_plan->invFHT20[0].mem);
        err |= clSetKernelArg(fromq_reverse, n_arg++, sizeof(cl_mem), &fht_plan->convChannelMap.mem);
		err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(int), &chnl_stride);
		err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(int), &block_sz);
		err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(int), &current_IR_shift);
		err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(int), &first_interval);
		err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(int), &total_n_bins);
		err |= clSetKernelArg(fhtmad_kernel, n_arg++, sizeof(int), &current_bin);

		if(err != CL_SUCCESS) {
			printf("FHTMAD2: failed to set kernel arguments with error %d\n", err);
			return -1;
		}


		err = clEnqueueNDRangeKernel(queue, fhtmad_kernel , 3, NULL, g_wk, l_wk, 0, NULL, NULL);

		if(err != CL_SUCCESS) {
			printf("FHTMAD2: failed to launch kernel with error %d\n", err);
			return -1;
		}

		}
#else 
    // RUN FHT MAD with summation  
    int num_cycles = 1 << multi;
    MADSummationDevided2(plan, _ir_version, numChans, curr_cycle);

#endif
	}


// RUN inverse FHT
    if (stream2XfadeInProgress(plan) && (next_cycle == (num_cycles - XFADE_IFHT_OLD_IR_CYCLE_OFFSET)))
    {
        // finish the old IR's IFHT
        applyInvFHT2(plan, numChans, 1, 0);
    }
    
	if (next_cycle == (num_cycles - IFHT_CYCLE_OFFSET)) {
		uint init_flags = SchedGetInitFlags(GetScheduler(plan));
		if ( init_flags & __INIT_FLAG_VER_TRANSFORM2__) {
			CopyFromDevice(queue, &fht_plan->invFHT20[0]);
		}
        // finish the new IR's IFHT and crossfade with the old IR's IFHT results if in the crossfade process
        bool doXfade = stream2XfadeInProgress(plan);
        applyInvFHT2(plan, numChans, 0, doXfade);

// verification


		if ( plan->run_flags &  __PROCESSING_FLAG_VERIFY_TRANSFORM2__ ) {
			int err = -1;
			CopyFromDevice(queue, &in_out_plan->stream2_out[curr_stream]);


			for(int i = 0; i < numChans; i++ ) {
				int chnl_off = i * block_sz/2 ;
				err = FHT_verify((const __FLOAT__ *) fht_plan->invFHT20[0].sys + i * block_sz, (const __FLOAT__ *)in_out_plan->stream2_out[curr_stream].sys + i*block_sz/2, block_sz, 1, block_sz/2, (__FLOAT__)0.5 );
				if ( err >= 0 ) {
					break;
				}

			}

			if ( err < 0 ) {
				printf("Data2 inv FHT %d %d is correct\n", block_sz, numChans);
			}

		}
	}

	return (err);
}


///////////////////////////////////////////////////////////////
//
// release FHT resources
//
//////////////////////////////////////////////////////////////////
int ReverbOCLDeinitFHT( ProjPlan * plan) {
	int err = 0;
	amdFHT_OCL *fhtOCL = (amdFHT_OCL *)plan->GPUConvpass;
	if ( fhtOCL) {
	
		for(int i = 0; i < FHT2_OCL_KERNELS_TOTAL; i++ ) {
			if ( fhtOCL->fht_kernels[i] ) {
				clReleaseKernel(fhtOCL->fht_kernels[i]);
				fhtOCL->fht_kernels[i] = 0;
			}
		}
		if ( fhtOCL->fhtmad_data ) {
			free(fhtOCL->fhtmad_data);
			fhtOCL->fhtmad_data = 0;
		}
        for (int i = 0; i < plan->num_IR_buffers; i++)
        {
            ReleaseOCLBuffer(&fhtOCL->IR1[i]);
            ReleaseOCLBuffer(&fhtOCL->IR2[i]);
        }
		delete [] (fhtOCL->IR1);
		delete [] (fhtOCL->IR2);

		ReleaseOCLBuffer(&fhtOCL->bireverse1);
		ReleaseOCLBuffer(&fhtOCL->sincos1);
		ReleaseOCLBuffer(&fhtOCL->bireverse2);
		ReleaseOCLBuffer(&fhtOCL->sincos2);
		ReleaseOCLBuffer(&fhtOCL->dirFHT10);
		ReleaseOCLBuffer(&fhtOCL->dirFHT11);
        for(int i = 0; i < 2; i++)
            ReleaseOCLBuffer(&fhtOCL->dirFHT20[i]);
		ReleaseOCLBuffer(&fhtOCL->dirFHT21);
        for (int i = 0; i < 2; i++)
            ReleaseOCLBuffer(&fhtOCL->invFHT10[i]);
		ReleaseOCLBuffer(&fhtOCL->invFHT11);
        for(int i = 0; i < 2; i++)
            ReleaseOCLBuffer(&fhtOCL->invFHT20[i]);
		ReleaseOCLBuffer(&fhtOCL->accum10);
        for (int i = 0; i < 2; i++)
            ReleaseOCLBuffer(&fhtOCL->accum11[i]);
		ReleaseOCLBuffer(&fhtOCL->accum20);
        for (int i = 0; i < 2; i++)
            ReleaseOCLBuffer(&fhtOCL->accum21[i]);
		ReleaseOCLBuffer(&fhtOCL->syncs);
        ReleaseOCLBuffer(&fhtOCL->convChannelMap);
        ReleaseOCLBuffer(&fhtOCL->updateChannelMap);
		free(plan->GPUConvpass);
		plan->GPUConvpass = 0;
	}
	return(err);
}
