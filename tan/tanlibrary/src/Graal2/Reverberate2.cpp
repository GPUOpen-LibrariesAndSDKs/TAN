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
#include "Reverberate2_OCL.h"
#include "DirectConv_OCL.h"
#include "amdFHT_OCL.h"
#include "amdFIR_OCL.h"

///////////////////////////////////////////////////////////////
//  INTERNALS
//
//
//////////////////////////////////////////////////////////////////

#if defined DEFINE_AMD_OPENCL_EXTENSION
#ifndef CL_MEM_USE_PERSISTENT_MEM_AMD
#define CL_MEM_USE_PERSISTENT_MEM_AMD       (1 << 6)
#endif
#endif



static
int streamInOutPassDeinit(ProjPlan * plan) {
int err = 0;
STREAM_INOUT * in_out_plan = (STREAM_INOUT *)plan->streamBlockInOutpass;
    if ( in_out_plan ) {
		for ( int i = 0; i < __NUM_STREAMS__ && err == CL_SUCCESS; i++) {
			err |= ReleaseOCLBuffer(&in_out_plan->stream_in[i]);
			err |= ReleaseOCLBuffer(&in_out_plan->stream_out[i]);
			err |= ReleaseOCLBuffer(&in_out_plan->stream2_out[i]);
		}

		if ( in_out_plan->stream2_data_ptrs ) {
			free(in_out_plan->stream2_data_ptrs);
		}


		free(in_out_plan);
		plan->streamBlockInOutpass = NULL;
	}
	return err;
}



//////////////////////////////////////////////
//
// real resource deallocation
//
//////////////////////////////////////////////

static
int ReverbOCLUnsetupInternal(ProjPlan * plan) {
int err = 0;

	amdAudShcheduler * sched = GetScheduler(plan);

	/*if ( ReverbGetState(plan) & __STATE_UNSETUP_RVRB__ )*/ {
		if (SchedGetInitFlags(sched) & __INIT_FLAG_FHT__ ) {
			err |= ReverbOCLDeinitFHT( plan);
		} else {

		}
		err |= DirectConvCPUDeinit(plan);
		err |= streamInOutPassDeinit(plan);
	}
	firDirectDeinit(plan);

		
	return(err);
}


//////////////////////////////////////////////
//
// resource deallocation
//
//////////////////////////////////////////////
int ReverbOCLUnsetup(ProjPlan * plan) {
int err = 0;

	err = ReverbOCLUnsetupInternal(plan);
	
	return(err);
}


int prepareKernels(ProjPlan * plan, int init_flags, const cl_mem kernel, int _n_channels, int* _channel_ids, unsigned int _ir_version, const int in_channel_sample_stride)
{
    amdFHT_OCL *fht_plan = (amdFHT_OCL *)plan->GPUConvpass;
    cl_command_queue queue = plan->OCLqueue[1];// general queue//GetOCLQueue(plan);
    int err = 0;
    int frame_ln, frame1_ln;
    
    frame1_ln = frame_ln = plan->frame_ln;
    int conv_blocks1 = plan->conv_blocks;

    //	uint init_flags = SchedGetInitFlags(GetScheduler(plan));
    if (init_flags & __INIT_FLAG_2STREAMS__)
    {
        frame_ln = plan->frame2_ln;
        frame1_ln = plan->frame_ln;
        conv_blocks1 = plan->in_que_ln;
    }
    int stream1_chnl_stride = conv_blocks1 * frame1_ln * 2;
    int long_strm_chnl_stride =  plan->conv_blocks * frame_ln * 2;
    err |= CopyToDevice2(queue, &fht_plan->updateChannelMap, _channel_ids, fht_plan->updateChannelMap.len, 0, false);
    {
        size_t global[3] = { size_t(frame1_ln), size_t(conv_blocks1), size_t(_n_channels) };
        size_t local[3] = { size_t((frame1_ln>256) ? 256 : frame1_ln), 1, 1 };
        cl_kernel partition_kernel = fht_plan->fht_kernels[PARTITION_KERNEL];
        int index = 0;
        err |= clSetKernelArg(partition_kernel, index++, sizeof(cl_mem), &kernel);
        err |= clSetKernelArg(partition_kernel, index++, sizeof(cl_mem), &fht_plan->IR1[_ir_version].mem);
        err |= clSetKernelArg(partition_kernel, index++, sizeof(cl_mem), &fht_plan->updateChannelMap.mem);
        err |= clSetKernelArg(partition_kernel, index++, sizeof(int), &_n_channels);
        err |= clSetKernelArg(partition_kernel, index++, sizeof(int), &conv_blocks1);
        err |= clSetKernelArg(partition_kernel, index++, sizeof(int), &in_channel_sample_stride);
        err |= clSetKernelArg(partition_kernel, index++, sizeof(int), &stream1_chnl_stride);
        err |= clSetKernelArg(partition_kernel, index++, sizeof(int), &frame1_ln);
        err |= clEnqueueNDRangeKernel(queue, partition_kernel, 3, NULL, global, local, 0, NULL, NULL);
    }
    {
        size_t global[3] = { size_t(frame_ln), size_t(plan->conv_blocks), size_t(_n_channels) };
        size_t local[3] = { size_t((frame_ln>256) ? 256 : frame_ln), 1, 1 };
        cl_kernel partition_kernel = fht_plan->fht_kernels[PARTITION_KERNEL];
        int index = 0;
        err |= clSetKernelArg(partition_kernel, index++, sizeof(cl_mem), &kernel);
        err |= clSetKernelArg(partition_kernel, index++, sizeof(cl_mem), &fht_plan->IR2[_ir_version].mem);
        err |= clSetKernelArg(partition_kernel, index++, sizeof(cl_mem), &fht_plan->updateChannelMap.mem);
        err |= clSetKernelArg(partition_kernel, index++, sizeof(int), &_n_channels);
        err |= clSetKernelArg(partition_kernel, index++, sizeof(int), &plan->conv_blocks);
        err |= clSetKernelArg(partition_kernel, index++, sizeof(int), &in_channel_sample_stride);
        err |= clSetKernelArg(partition_kernel, index++, sizeof(int), &long_strm_chnl_stride);
        err |= clSetKernelArg(partition_kernel, index++, sizeof(int), &frame_ln);
        err |= clEnqueueNDRangeKernel(queue, partition_kernel, 3, NULL, global, local, 0, NULL, NULL);
    }
    return err;
}

void createHostBuffers(ProjPlan * plan)
{
    amdFHT_OCL *fht_plan = (amdFHT_OCL *)plan->GPUConvpass;
    if (fht_plan->IR1_host)
        delete fht_plan->IR1_host;

    if (fht_plan->IR2_host)
        delete fht_plan->IR2_host;

    fht_plan->IR1_host = (__FLOAT__*)malloc(fht_plan->IR1[0].len);
    memset(fht_plan->IR1_host, 0, fht_plan->IR1[0].len);

    fht_plan->IR2_host = (__FLOAT__*)malloc(fht_plan->IR2[0].len);
    memset(fht_plan->IR2_host, 0, fht_plan->IR2[0].len);

}
int prepareKernels(ProjPlan * plan, int init_flags, float ** host_kern, int _n_channels, int* _channel_ids, unsigned int _ir_version, const int* _conv_lens)
{
    amdFHT_OCL *fht_plan = (amdFHT_OCL *)plan->GPUConvpass;
    size_t stream1_sz = fht_plan->IR1[_ir_version].len;
    size_t long_stream_sz = fht_plan->IR2[_ir_version].len;
    if (!fht_plan->IR1_host || !fht_plan->IR1_host) 
        createHostBuffers(plan);
    __FLOAT__ * host_kern1 = fht_plan->IR1_host;
    __FLOAT__ * host_kern_exp = fht_plan->IR2_host;
    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    int frame_ln, frame1_ln;

    frame1_ln = frame_ln = plan->frame_ln;
    int conv_blocks1 = plan->conv_blocks;

    //	uint init_flags = SchedGetInitFlags(GetScheduler(plan));
    if (init_flags & __INIT_FLAG_2STREAMS__)
    {
        frame_ln = plan->frame2_ln;
        frame1_ln = plan->frame_ln;
        conv_blocks1 = plan->in_que_ln;
    }

    size_t stream1_chnl_stride = conv_blocks1 * frame1_ln * 2;

    size_t long_strm_chnl_stride = frame_ln * plan->conv_blocks * 2;

    size_t host_chnl_stride = frame_ln * plan->conv_blocks;

    // prepare upload data
    __FLOAT__ ** in_data = host_kern;
    __FLOAT__ * upload_data = host_kern_exp;
    int val = 0;
    cl_command_queue queue = plan->OCLqueue[1];// general queue//GetOCLQueue(plan);
    clEnqueueFillBuffer(queue, fht_plan->IR2[_ir_version].mem, &val, sizeof(val), 0, fht_plan->IR2[_ir_version].len, 0, NULL, NULL);
    clEnqueueFillBuffer(queue, fht_plan->IR1[_ir_version].mem, &val, sizeof(val), 0, fht_plan->IR1[_ir_version].len, 0, NULL, NULL);
    int chnl_off_i = 0, chnl_off_o = 0;

    for (int c = 0; c < _n_channels; c++)
    {
        chnl_off_o = long_strm_chnl_stride*_channel_ids[c];
        // each block is appndedw with 0s of the same size
        for (int b = 0; b < plan->conv_blocks-1; b++)
        {
            // current + appended 0
            memcpy(&upload_data[chnl_off_o + b*frame_ln * 2], &in_data[c][b*frame_ln], frame_ln * sizeof(__FLOAT__));

        }
        int b = plan->conv_blocks - 1;
        int len = _conv_lens[c] - frame_ln*(plan->conv_blocks - 1);
        if(len >0)
            memcpy(&upload_data[chnl_off_o + b*frame_ln * 2], &in_data[c][b*frame_ln], len * sizeof(__FLOAT__));
    }
    // upload data

    in_data = host_kern;
    upload_data = host_kern1;
    chnl_off_i = 0;
    for (int c = 0; c < _n_channels; c++, chnl_off_i += host_chnl_stride)
    {
        chnl_off_o = stream1_chnl_stride*_channel_ids[c];
        for (int b = 0; b < conv_blocks1; b++)
        {
            // current + appended 0
            memcpy(&upload_data[chnl_off_o + b*frame1_ln * 2], &in_data[c][b*frame1_ln], frame1_ln * sizeof(__FLOAT__));
        }
    }
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    int err = CopyToDevice2(queue, &(fht_plan->IR1[_ir_version]), fht_plan->IR1_host, fht_plan->IR1[_ir_version].len);
    err |= CopyToDevice2(queue, &(fht_plan->IR2[_ir_version]), fht_plan->IR2_host, fht_plan->IR2[_ir_version].len);
    return err;
}

void cpuHeterogenouosProcessing(ProjPlan * plan, int numChans, int init_flags)
{
    amdFHT_OCL *fht_plan = (amdFHT_OCL *)plan->GPUConvpass;
    __FLOAT__ * in_data = fht_plan->IR1_host;
    int frame_ln, frame1_ln;
    frame1_ln = frame_ln = plan->frame_ln;
    int conv_blocks1 = plan->conv_blocks;

    //	uint init_flags = SchedGetInitFlags(GetScheduler(plan));

    size_t stream1_sz = 0;
    size_t stream1_chnl_stride = 0;
    if (init_flags & __INIT_FLAG_2STREAMS__)
    {
        frame_ln = plan->frame2_ln;
        frame1_ln = plan->frame_ln;
        conv_blocks1 = plan->in_que_ln;

    }

    stream1_chnl_stride = conv_blocks1 * frame1_ln * 2;

    __FLOAT__ * cpu_data = (__FLOAT__*)malloc(numChans * frame1_ln * 2 * sizeof(__FLOAT__));

    // CPU processing

    int chnl_off_i = 0;
    int chnl_off_o = 0;
    for (int c = 0; c < numChans; c++,
            chnl_off_i += stream1_chnl_stride,
            chnl_off_o += frame1_ln * 2)
    {

        memcpy(&cpu_data[chnl_off_o], &in_data[chnl_off_i], frame1_ln * 2 * sizeof(__FLOAT__));

    }

    // move the kerenl's first bin into a CPU processor
    // do direct FHT of the bin
    SetupCPUFHTConvKernel(plan, cpu_data);
    if (cpu_data)
    {
        free(cpu_data);
    }
}

/////////////////////////////////////////////////////////////////
//
// 2 non-uniform streams:
// short: frame_ln x (frame2_ln / frame_ln) x n_channels
//
// long : frame2_ln x (conv_ln/ frame2_ln) x n_channels 
//////////////////////////////////////////////////////////////////
#if 1
int ReverbOCLMoveKernelIntoFHTPipeline2(ProjPlan * plan, int numChans, int* channel_ids, unsigned int _ir_version, int init_flags) {
int err = 0;
// load a kernel into the GPU pipeline

		if ( init_flags & __INIT_FLAG_2STREAMS__ ) {
// GPU IR setup for 2 streams
			err |= ReverbOCLSetupFHT( plan , numChans, channel_ids, _ir_version);
		}	else {
// GPU IR setup for 1 stream
			err |= ReverbOCLSetupFHT( plan, numChans, channel_ids, _ir_version);
		}

	return(err);
}
#endif
///////////////////////////////////////////////////////////////
//
// presetup scatter-gater or input/output of the pipeline
//
//////////////////////////////////////////////////////////////////
int ReverbOCLSetupSG( ProjPlan * plan) {
	int err = 0;
// streaming initialization
	uint init_flags = SchedGetInitFlags(GetScheduler(plan));

	STREAM_INOUT * in_out_plan = (STREAM_INOUT *)plan->streamBlockInOutpass;

	if ( !in_out_plan ) {
		in_out_plan = (STREAM_INOUT *)malloc(sizeof(STREAM_INOUT));
		if ( !in_out_plan ) {
			printf("Failed to allocate in/out plan\n");
			return (-1);
		}
		memset(in_out_plan, 0, sizeof(STREAM_INOUT));
	}
	int mem_req = 0;
	if ( ReverbGetState(plan) & __STATE_SETUP_RVRB__ ) {

		for ( int i = 0; i < __NUM_STREAMS__; i++) {

			in_out_plan->stream_in[i].len = plan->frame_ln * sizeof(__FLOAT__) * ReverbGetNDataBlocksToFFT(plan) * plan->in_que_ln;
			mem_req += in_out_plan->stream_in[i].len;

			if ( init_flags & __INIT_FLAG_2FFT_STREAMS__ ) {
				in_out_plan->stream2_out[i].len = plan->frame2_ln * sizeof(__FLOAT__) * ReverbGetNDataBlocksToFFT(plan);
				mem_req += in_out_plan->stream2_out[i].len;
			}

		}

		if ( init_flags & __INIT_FLAG_2FFT_STREAMS__ ) {
			in_out_plan->stream2_data_ptrs = (void**)malloc(sizeof(__FLOAT__*) * plan->conv_NumChannels);
		}
	}

	if ( (ReverbGetState(plan) & __STATE_SETUP_RVRB__) ||  (ReverbGetState(plan) & __STATE_SETUP_FIR__)) {

		for ( int i = 0; i < __NUM_STREAMS__; i++) {

			in_out_plan->stream_out[i].len = plan->frame_ln * sizeof(__FLOAT__) * ReverbGetNDataBlocksToFFT(plan);
			mem_req += in_out_plan->stream_out[i].len;
		}

	}
	plan->streamBlockInOutpass = in_out_plan;

	return(err);
}

///////////////////////////////////////////////////////////////
//
//	setup scatter-gater or input/output of the pipeline
//
//////////////////////////////////////////////////////////////////
int ReverbOCLSetupScatterGather(ProjPlan * plan) {
	int err = 0;
// scatter gather
    cl_command_queue commandQueue = GetOCLQueue(plan);
	uint init_flags = SchedGetInitFlags(GetScheduler(plan));	
	STREAM_INOUT * in_out_plan = (STREAM_INOUT *)plan->streamBlockInOutpass;
	if ( ReverbGetState(plan) & __STATE_SETUP_RVRB__ ) {
		for ( int i = 0; i < __NUM_STREAMS__ && err == CL_SUCCESS; i++) {
			int mod_flags = (!(init_flags & __INIT_FLAG_FIR__)) ? CL_MEM_READ_ONLY | CL_MEM_USE_PERSISTENT_MEM_AMD /*  CL_MEM_ALLOC_HOST_PTR*/: 0;
			err |= CreateOCLBuffer2(GetOCLContext(plan), &in_out_plan->stream_in[i], mod_flags );
			SetValue(commandQueue, &in_out_plan->stream_in[i], 0.0);

			if ( init_flags & __INIT_FLAG_2FFT_STREAMS__ ) {
				int mod_flag = ((init_flags & __INIT_FLAG_2STREAMS__)) ? 0 :  CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR;
				err |= CreateOCLBuffer2(GetOCLContext(plan), &in_out_plan->stream2_out[i], mod_flag );
				SetValue(commandQueue, &in_out_plan->stream2_out[i], 0.0);
			}

		}
		if ( !(init_flags & __INIT_FLAG_FIR__) )
		{

// map input queue for the entire lifespan
			if ( !in_out_plan->stream_in[0].sys_map ) {
				in_out_plan->stream_in[0].sys_map = (__FLOAT__*)clEnqueueMapBuffer (commandQueue,
 				in_out_plan->stream_in[0].mem,
 				CL_TRUE,
 				CL_MAP_WRITE,
 				0,
 				in_out_plan->stream_in[0].len,
 				0,
 				NULL,
 				NULL,
 				&err);
			}
			if ( err ) {
				printf("Failed to map input buffers: %d\n", err);
				return (err);
			}
		}
	
	}

	if ( (ReverbGetState(plan) & __STATE_SETUP_RVRB__) || (ReverbGetState(plan) & __STATE_SETUP_FIR__)) {
		for ( int i = 0; i < __NUM_STREAMS__ && err == CL_SUCCESS; i++) {

			if ( !in_out_plan->stream_out[i].mem ) {
				err |= CreateOCLBuffer2(GetOCLContext(plan), &in_out_plan->stream_out[i], CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR );
				SetValue(commandQueue, &in_out_plan->stream_out[i], 0.0);
			}
		}
	}
	return (err);
}


///////////////////////////////////////////////////////////////
//
// move newly arriving block into teh pipeline
// with 1 uniforme pipelien it's a double buffering
// with 2 non-uniform streams it's a inpiut queue into the 1st straem
// and accumulate double buffering for teh 2nd stream
//
//////////////////////////////////////////////////////////////////
int ReverbOCLMoveDataIntoPipeline(ProjPlan *plan, __FLOAT__ ** input, int numSamples, int numChans, int* channel_ids) {
	int err = 0;
__FLOAT__ *stream_data = NULL;
// than write out current input 
	uint init_flags = SchedGetInitFlags(GetScheduler(plan));
	STREAM_INOUT * in_out_plan = (STREAM_INOUT *)plan->streamBlockInOutpass;
	__int64 last_counter = GetRunCounter(plan);
	int que_index =  (last_counter) % plan->in_que_ln;
	int curr_stream = 0;
    auto queue = GetOCLQueue(plan);//conv queue
// input data is a queue of input blocks realized as acyclic array per each sub-channel
	stream_data = (__FLOAT__ *)in_out_plan->stream_in[curr_stream].sys_map;

	for (int j = 0; j < numChans; j++) {
#ifdef _WIN32
        int chId = channel_ids[j];
        __FLOAT__ * block_ptr = stream_data + plan->frame_ln * (que_index + plan->in_que_ln * chId);
		memcpy(block_ptr, input[j], numSamples * sizeof(__FLOAT__));
#else
        // Replacing memory mapping with explicit memory OCL write 
        size_t block_offset = plan->frame_ln * (que_index + plan->in_que_ln * j);
        CopyToDevice2(queue, &in_out_plan->stream_in[curr_stream], input[j], numSamples * sizeof(__FLOAT__), block_offset * sizeof(__FLOAT__));
#endif
        
// TODO: MOVE INTO KERNEL
        if (plan->frame_ln > numSamples)
        {
#ifdef _WIN32
            memset((block_ptr + numSamples), 0, (plan->frame_ln - numSamples) * sizeof(__FLOAT__));
#else
            // Replacing memory mapping with explicit memory OCL write 
            SetValue(queue, &in_out_plan->stream_in[curr_stream], 0, (block_offset + numSamples) * sizeof(__FLOAT__), (plan->frame_ln - numSamples) * sizeof(__FLOAT__));
#endif
        }
	}



	return (err);
}
///////////////////////////////////////////////////////////////

// take a double buffered data from the 2nd non-uniform stream
// on every round take the 1stream block size *2 worth of data from a proper position.
// map once in multiplier time
// unmap once in maltiplier time
//
// map_unmap == 0 - return pointer
// map_unmap == 1 - map
// map_unmap == 2 - unmap
//////////////////////////////////////////////////////////////////


int ReverbOCLGetStream2Data(ProjPlan * plan, int numChans) {
	int err = 0;
	STREAM_INOUT * in_out_plan = (STREAM_INOUT *)plan->streamBlockInOutpass;
	int curr_stream = 0;
	int map_unmap = 0;
	cl_command_queue stream2Q = GetOCLQueue(plan);
    int multi = plan->frame2_ln_log2 - plan->frame_ln_log2;
	__int64 current_counter = GetRunCounter(plan);
	__int64 next_counter = current_counter + 1;
// time to update 2nd stream
	curr_stream = ((current_counter >> multi) & 1);
	if ( ((current_counter >> multi) << multi) == current_counter ) {
		map_unmap = 1;

	} else if ( ((next_counter >> multi) << multi) == next_counter ) {
		map_unmap = 2;

	}


	if ( (map_unmap == 1) && !in_out_plan->stream2_out[curr_stream].sys_map ) {
		in_out_plan->stream2_out[curr_stream].sys_map = (__FLOAT__*)clEnqueueMapBuffer (stream2Q,

 			in_out_plan->stream2_out[curr_stream].mem,
 			CL_TRUE,
 			CL_MAP_READ,
 			0,
 			in_out_plan->stream2_out[curr_stream].len,
 			0,
 			NULL,
 			NULL,
 			&err);

	}
 

	int index =  (int)(current_counter % (__int64)(1 << multi));
	int stream2_chnl_stride = ((plan->frame_ln) << multi);
	for (int i = 0; i < numChans; i++ ) {
	  ((__FLOAT__**)in_out_plan->stream2_data_ptrs)[i] = (__FLOAT__*)in_out_plan->stream2_out[curr_stream].sys_map + i*stream2_chnl_stride + index * plan->frame_ln;
	}


	if (  (map_unmap == 2) && in_out_plan->stream2_out[curr_stream].sys_map ) {
		clEnqueueUnmapMemObject(stream2Q,
 			in_out_plan->stream2_out[curr_stream].mem,
 			in_out_plan->stream2_out[curr_stream].sys_map,
 			0,
 			NULL,
 			NULL);
		in_out_plan->stream2_out[curr_stream].sys_map = NULL;
	}


	return err;
}



///////////////////////////////////////////////////////////////
//
// sum up 2 non-uniform data streams
//
//////////////////////////////////////////////////////////////////

int ReverbOCLSum2Streams(ProjPlan * plan,__FLOAT__ ** output, int numChans) {
	int ret = 0;
	STREAM_INOUT * in_out_plan = (STREAM_INOUT *)plan->streamBlockInOutpass;
	for(int i = 0; i < numChans; i++ ) {
		for(int j = 0; j < plan->frame_ln; j++) {
			output[i][j] += ((__FLOAT__**)in_out_plan->stream2_data_ptrs)[i][j];
		}
	}
	return(ret);
}



//#define __CPU_ONLY
///////////////////////////////////////////////////////////////
//
// full hetergenious non-uniform pipeline:
// block on the previous convolution of the 1st stream
// get data from the 1st stream (previous round)
// get a proper block from the 2nd stream (previous round)
// move newly arrived block into teh GPU pipeline
// launch the 1st stream pipeline
// launch the 2nd stream pipeline
// sum data from 2 streams
// do direct FHT/MAD/ inverse FHT of the arrived block on CPU
// add the first block convolution with the tail convolution
// return the result to a caller
//
//////////////////////////////////////////////////////////////////
int ReverbOCLProcessing2FHTsHtrgs(ProjPlan *plan, __FLOAT__ ** input, __FLOAT__ ** output, int numSamples, int numChans, int* channel_ids, unsigned int _ir_version, int flags) {
	int err =  CL_SUCCESS;

	STREAM_INOUT * in_out_plan = (STREAM_INOUT *)plan->streamBlockInOutpass;
	int curr_stream = 0;
	__FLOAT__ * output_buffers = NULL;

#ifndef __CPU_ONLY // CPU only


	__int64 current_counter = GetRunCounter(plan);
#if 1
// read previous results first
// read from 1 stream
	if ( current_counter > 0 ) {
		err = clWaitForEvents (	1, &in_out_plan->read_event);
		err |= clReleaseEvent(in_out_plan->read_event);
		output_buffers = (__FLOAT__*)in_out_plan->stream_out[curr_stream].sys_map;
	
		for (int j = 0; j < plan->nmb_channels; j++) {
			__FLOAT__ * block_ptr = output_buffers + plan->frame_ln * j;
			memcpy(output[j], block_ptr, numSamples * sizeof(__FLOAT__));
		}
	
		clEnqueueUnmapMemObject(GetOCLQueue(plan),
 			in_out_plan->stream_out[curr_stream].mem,
 			in_out_plan->stream_out[curr_stream].sys_map,
 			0,
 			NULL,
 			NULL);

		in_out_plan->stream_out[curr_stream].sys_map = NULL;
	} else {
		for (int j = 0; j < plan->nmb_channels; j++) {
			memset(output[j], 0, numSamples * sizeof(__FLOAT__));
		}
	}	
#else
	ReverbOCLProcessingReadSamples(plan,  output, numSamples, channel_ids, 0);

#endif

// move data into pipeline
	err = ReverbOCLMoveDataIntoPipeline(plan, input, numSamples, numChans, channel_ids);



// do the 2nd stream processing
	ReverbOCLRunFHT2(plan, _ir_version, numChans);


// do the 1st stream processing
	ReverbOCLRunFHT1(plan, _ir_version, numChans);

// ask for the data in the next round
#if 1
	in_out_plan->stream_out[curr_stream].sys_map = (__FLOAT__*)clEnqueueMapBuffer (GetOCLQueue(plan),

 			in_out_plan->stream_out[curr_stream].mem,
 			CL_FALSE,
 			CL_MAP_READ,
 			0,
 			in_out_plan->stream_out[curr_stream].len,
 			0,
 			NULL,
 			&in_out_plan->read_event,
 			&err);
#endif

	clFlush(GetOCLQueue(plan));

	
#endif // CPU only

// process and add the CPU part
#if 1

	ReverbFHTCPUProcessing(plan, input, output, numSamples, 0);
	
#endif
	return err;
}


///////////////////////////////////////////////////////////////
//
// non-uniform pipeline, 0 CPU footprint:
//
// move newly arrived block into teh GPU pipeline
// launch the 1st stream pipeline
// launch the 2nd stream pipeline
// sum data from 2 streams
//
// return the result to a caller
//
//////////////////////////////////////////////////////////////////
int ReverbOCLProcessing2FHTs(ProjPlan *plan, __FLOAT__ ** input, __FLOAT__ ** output, cl_mem* cl_output, int numSamples, int numChans, int* channel_ids, unsigned int _ir_version, int flags) {
    
    // move data into pipeline
    if (ReverbOCLMoveDataIntoPipeline(plan, input, numSamples, numChans, channel_ids) != 0)
    {
        printf("Faield moving the input data to the pipeline\n");
        return -1;
    }

    // do the 1st stream processing
    if (ReverbOCLRunFHT1(plan, _ir_version, numChans) != 0)
    {
        printf("Failed running the first stream's convolution\n");
        return -2;
    }
     
	if (!cl_output)
	{
		if (ReverbOCLProcessingReadSamples(plan, output, numSamples, numChans, channel_ids, 0) != 0)
		{
			printf("Failed reading back the output samples\n");
			return -3;
		}
	}
	else
	{
		STREAM_INOUT * in_out_plan = (STREAM_INOUT *)plan->streamBlockInOutpass;
		int curr_stream = 0; //GetStreamIndex(plan);
		int status = 0;
		if (in_out_plan->stream_out[curr_stream].mem)
		{
			for (int j = 0; j < numChans; j++)
			{		
				int chId =  channel_ids[j];
				int offset = plan->frame_ln * chId * sizeof(float);
				
				status = clEnqueueCopyBuffer(
					GetOCLQueue(plan),
					in_out_plan->stream_out[curr_stream].mem,
					cl_output[j],
					offset,
					0,
					numSamples * sizeof(float),
					0,
					NULL,
					NULL);				
			}			
		}

		if (status != CL_SUCCESS)
		{
			printf("Failed reading back the OCL output samples\n");
			return -3;
		}
	} 

    // Enqueue the 2nd stream processing, it'll be executed outside of the critical latency path
    if (ReverbOCLRunFHT2(plan, _ir_version, numChans) != 0)
    {
        printf("Faield running the second stream's convolution\n");
        return -4;
    }

	return 0;
}


///////////////////////////////////////////////////////////////
//
// hetergenious uniform pipeline:
// block on the previous convolution of the 1st stream
// get data from the 1st stream (previous round)
//
// move newly arrived block into teh GPU pipeline
// launch the 1st stream pipeline
//
// do direct FHT/MAD/ inverse FHT of the arrived block on CPU
// add the first block convolution with the tail convolution
// return the result to a caller
//
//////////////////////////////////////////////////////////////////
int ReverbOCLProcessing1FHTHtrgs(ProjPlan *plan, __FLOAT__ ** input, __FLOAT__ ** output, int numSamples, int numChans, int* channel_ids, unsigned int _ir_version, int flags) {
	int err =  CL_SUCCESS;


	STREAM_INOUT * in_out_plan = (STREAM_INOUT *)plan->streamBlockInOutpass;
	int curr_stream = 0;
	__FLOAT__ * output_buffers = NULL;

#ifndef __CPU_ONLY // CPU only

	__int64 current_counter = GetRunCounter(plan);
#if 1
// read previous results first
// read from 1 stream
	if ( current_counter > 0 ) {
		err = clWaitForEvents (	1, &in_out_plan->read_event);
		err |= clReleaseEvent(in_out_plan->read_event);
		output_buffers = (__FLOAT__*)in_out_plan->stream_out[curr_stream].sys_map;
	
		for (int j = 0; j < plan->nmb_channels; j++) {
			__FLOAT__ * block_ptr = output_buffers + plan->frame_ln * j;
			memcpy(output[j], block_ptr, numSamples * sizeof(__FLOAT__));
		}
	
		clEnqueueUnmapMemObject(GetOCLQueue(plan),
 			in_out_plan->stream_out[curr_stream].mem,
 			in_out_plan->stream_out[curr_stream].sys_map,
 			0,
 			NULL,
 			NULL);

		in_out_plan->stream_out[curr_stream].sys_map = NULL;
	} else {
		for (int j = 0; j < plan->nmb_channels; j++) {
			memset(output[j], 0, numSamples * sizeof(__FLOAT__));
		}
	}	
#else
	ReverbOCLProcessingReadSamples(plan,  output, numSamples, numChans, channel_ids, 0);

#endif


// move data into pipeline
	err = ReverbOCLMoveDataIntoPipeline(plan, input, numSamples, numChans, channel_ids);


// do the 1st stream processing
	ReverbOCLRunFHT1(plan, _ir_version, numChans);


// ask for the data in the next round
#if 1
	in_out_plan->stream_out[curr_stream].sys_map = (__FLOAT__*)clEnqueueMapBuffer (GetOCLQueue(plan),

 			in_out_plan->stream_out[curr_stream].mem,
 			CL_FALSE,
 			CL_MAP_READ,
 			0,
 			in_out_plan->stream_out[curr_stream].len,
 			0,
 			NULL,
 			&in_out_plan->read_event,
 			&err);
#endif

	clFlush(GetOCLQueue(plan));



#endif // CPU only


// process and add the CPU part
#if 1

	ReverbFHTCPUProcessing(plan, input, output, numSamples, 0);
	
#endif
	return err;
}


///////////////////////////////////////////////////////////////
//
// uniform pipeline, 0 CPU footprint:
//
// move newly arrived block into teh GPU pipeline
// launch the 1st stream pipeline

// return the result to a caller
//
//////////////////////////////////////////////////////////////////
int ReverbOCLProcessing1FHT(ProjPlan *plan, __FLOAT__ ** input, __FLOAT__ ** output, int numSamples, int numChans, int* channel_ids, unsigned int _ir_version, int flags) {
	int err =  CL_SUCCESS;

	err = ReverbOCLMoveDataIntoPipeline(plan, input, numSamples, numChans, channel_ids);

// do the 1st stream processing
	ReverbOCLRunFHT1(plan, _ir_version, numChans);

	ReverbOCLProcessingReadSamples(plan,  output, numSamples, numChans, channel_ids, 0);

	return err;
}


///////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////
int ReverbOCLProcessing2FHT1onCPU(ProjPlan *plan, __FLOAT__ ** input, __FLOAT__ ** output, int numSamples, int numChans, int* channel_ids, unsigned int _ir_version, int flags) {
	int err =  CL_SUCCESS;
	int stream_id = 0;
	uint init_flags = SchedGetInitFlags(GetScheduler(plan));
	amdFHT_OCL *fht_plan = (amdFHT_OCL *)plan->GPUConvpass;
	OCLBuffer  *curr_fht1mad;	
	__int64 current_counter = GetRunCounter(plan);
	curr_fht1mad = &fht_plan->invFHT11;


#ifndef __CPU_ONLY // CPU only



#if 1
// read previous results first
// read from 1 stream

	if ( fht_plan->read_event ) {
		err = clWaitForEvents (	1, &fht_plan->read_event);
		err |= clReleaseEvent(fht_plan->read_event);
	}
	memcpy(fht_plan->fhtmad_data, curr_fht1mad->sys_map, curr_fht1mad->len);
		clEnqueueUnmapMemObject(GetOCLQueue(plan),
 			curr_fht1mad->mem,
 			curr_fht1mad->sys_map,
 			0,
 			NULL,
 			NULL);
//		curr_fht1mad->sys_map = NULL;

	ReverbOCLProcessingReadSamples(plan,  output, numSamples, numChans, channel_ids, 0);
//	clFlush(GetOCLQueue(plan));
#endif
// get second stream pointers
// output buffer switch inside
	err = ReverbOCLGetStream2Data(plan, numChans);

// use the dat to drive CPU direct transfor and send it to FHT! GPU pipeline
	ReverbOCLRunDirFHT1onCPU(plan, input, numChans);
// do the 1st stream processing
	ReverbOCLRunFHT1(plan, _ir_version, numChans);
//	clFlush(GetOCLQueue(plan));
// do th 2nd stream processing
	ReverbOCLRunFHT2(plan, _ir_version, numChans);

// ask for the data in the next round
#if 1
	curr_fht1mad->sys_map = (__FLOAT__*)clEnqueueMapBuffer (GetOCLQueue(plan),

 			curr_fht1mad->mem,
 			CL_FALSE,
 			CL_MAP_READ,
 			0,
 			curr_fht1mad->len,
 			0,
 			NULL,
 			&fht_plan->read_event,
 			&err);
#endif



	clFlush(GetOCLQueue(plan));





#endif // CPU only


// process and add the CPU part
#if 1

// get the direct transform sum it with FHT1 MAD
// do inverse transfom
// add it to the FHT2 inv transform
		ReverbOCLRunInvFHT1onCPU(plan, output, numChans);

// add 2 streams together
		ReverbOCLSum2Streams(plan, output , numChans);

#endif
	return err;
}


int ReverbOCLMoveChannelMappings(ProjPlan *plan, int* _channel_ids)
{
    amdFHT_OCL *fht_plan = (amdFHT_OCL *)plan->GPUConvpass;
    // move the channle mapping data into the associated OCL buffers
    cl_command_queue queue = GetOCLQueue(plan);
    int err = CopyToDevice2(queue, &fht_plan->convChannelMap, _channel_ids, fht_plan->convChannelMap.len, 0, false);
    if (err != CL_SUCCESS)
    {
        printf("Faield copying the channel mapping data to the device err = %d\n");
    }
    return err;
}
///////////////////////////////////////////////////////////////
//
// the main entry point.
// calls different pipeline inplemetetion depending on the 
// instansiation flags and (possiblly) on some hueristics
//
//////////////////////////////////////////////////////////////////
int ReverbOCLProcessing(ProjPlan *plan, __FLOAT__ ** input, __FLOAT__ ** output, cl_mem* cl_output, int numSamples, int numChans, int* _channel_ids, unsigned int _ir_version, int flags, StatisticsPerRun * stat, int xfade_state) {
	int err = 0;

	uint init_flags = SchedGetInitFlags(GetScheduler(plan));
	double t = 0;
	//double t0, t1;

	plan->run_flags = flags;
//l_event data_out_event;

	//t0 = mach_absolute_time();
    manageXFadeState(plan, xfade_state);
    ReverbOCLMoveChannelMappings(plan, _channel_ids);

	if ( init_flags & __INIT_FLAG_2STREAMS__) {
		if (init_flags & __INIT_FLAG_HETEROGEN__)  {
			if ( init_flags & __INIT_FLAG_FHT__ ) {
				if ( init_flags & __INIT_FLAG_FHT1_ONCPU__ ) {
					err = ReverbOCLProcessing2FHT1onCPU(plan, input, output, numSamples, numChans, _channel_ids, _ir_version, flags);
				} else {
					err = ReverbOCLProcessing2FHTsHtrgs(plan, input, output, numSamples, numChans, _channel_ids, _ir_version, flags);
				}
			}
			else {
	//			err = ReverbOCLProcessing2FFTsHtrgs(plan, input, output, numSamples, flags);
			}

		} else {
			if ( init_flags & __INIT_FLAG_FHT__ ) {
				err = ReverbOCLProcessing2FHTs(plan, input, output, cl_output, numSamples, numChans, _channel_ids, _ir_version, flags );
			} else {
		//		err = ReverbOCLProcessing2FFTs(plan, input, output, numSamples, flags);
			}
		}
	} else {
		if (init_flags & __INIT_FLAG_HETEROGEN__)  {
			if ( init_flags & __INIT_FLAG_FHT__ ) {
				ReverbOCLProcessing1FHTHtrgs(plan, input, output, numSamples, numChans, _channel_ids, _ir_version, flags);
			} else if (init_flags & __INIT_FLAG_HETEROGEN__)  {	
	//			err = ReverbOCLProcessing1FFTsHtrgs(plan, input, output, numSamples, flags);
			}
		} else if ( (init_flags & __INIT_FLAG_FHT__) && !(init_flags & __INIT_FLAG_FIR__)){
			err = ReverbOCLProcessing1FHT(plan, input, output, numSamples, numChans, _channel_ids, _ir_version, flags);
		} else if ( !(init_flags & __INIT_FLAG_FHT__) && (init_flags & __INIT_FLAG_FIR__)){
			err = firDirectProcessing(plan, input, output, numSamples, numChans, _channel_ids, flags);
		}
	}

    ReverbOCLEndBlock(plan);

#if 0
	t1 = mach_absolute_time(); 

	t = subtractTimes(t1, t0);

	stat->processing_time = t;

    int msg_interval =
#ifdef _DEBUG
		500
#else
		1000
#endif
		;
	if (GetPlanID(plan)==0 && GetRunCounter(plan) % msg_interval == 0 && GetRunCounter(plan) > 0 ) {
		printf("passed run %u\n", GetRunCounter(plan));
	}
#endif

	return(err);
}




/////////////////////////////////////////////////////////
//
// runtime internals
//
//////////////////////////////////////////////////////////
int ReverbOCLEndBlock(ProjPlan *plan) {
int ret = 0;

	__int64 last_counter = GetRunCounter(plan);
	SetRunCounter(plan, last_counter+1);
	return(ret);
}

int ReverbOCLGetOutput(HSA_OCL_ConfigS* config, ProjPlan * plan, void ** stream_data, int numChans, cl_event *map_event) {
int err = 0;
STREAM_INOUT * in_out_plan = (STREAM_INOUT *)plan->streamBlockInOutpass;
int curr_stream = 0; //GetStreamIndex(plan);

	if ( !in_out_plan->stream_out[curr_stream].sys_map ) {
		in_out_plan->stream_out[curr_stream].sys_map = (__FLOAT__*)clEnqueueMapBuffer (GetOCLQueue(plan),

 			in_out_plan->stream_out[curr_stream].mem,
 			/*( map_event ) ? CL_FALSE :*/ CL_TRUE,
 			CL_MAP_READ,
 			0,
 			in_out_plan->stream_out[curr_stream].len,
 			0,
 			NULL,
 			map_event,
 			&err);
		if ( map_event ) {
			err = clWaitForEvents (	1, map_event);
			err |= clReleaseEvent(*map_event);
		}
	}

	*stream_data = in_out_plan->stream_out[curr_stream].sys_map;
//    printf("Waiting for data strm: %d q: 0x%x\n", curr_stream, (uint)GetOCLQueue(plan));
	return err;
}

int ReverbOCLRelOutput(HSA_OCL_ConfigS* config, ProjPlan * plan, int numChans, cl_event *unmap_event) {
int err = 0;

STREAM_INOUT * in_out_plan = (STREAM_INOUT *)plan->streamBlockInOutpass;
int curr_stream = 0; //GetStreamIndex(plan);

	if ( in_out_plan->stream_out[curr_stream].sys_map ) {
		clEnqueueUnmapMemObject(GetOCLQueue(plan),
 			in_out_plan->stream_out[curr_stream].mem,
 			in_out_plan->stream_out[curr_stream].sys_map,
 			0,
 			NULL,
 			unmap_event);
		if ( unmap_event ) {
			err = clWaitForEvents (	1, unmap_event);
			err |= clReleaseEvent(*unmap_event);
		}
		in_out_plan->stream_out[curr_stream].sys_map = NULL;
	}


	return err;
}

int ReverbOCLProcessingReadSamples(ProjPlan *plan, __FLOAT__ ** output, int numSamples, int numChans, int* channel_ids, int flags) {
int err =  CL_SUCCESS;
HSA_OCL_ConfigS * config = GetOclConfig(plan);
__FLOAT__ *output_buffers = NULL;
cl_event data_out_event = 0;
	__int64 current_counter = GetRunCounter(plan);
   int multi = plan->frame2_ln_log2 - plan->frame_ln_log2;
// blocking map
	ReverbOCLGetOutput(config, plan, (void **) &output_buffers, numChans, NULL/*&data_out_event*/);
//	err = clWaitForEvents(1,&data_out_event);
//	clReleaseEvent(data_out_event);


	for (int j = 0; j < numChans; j++) {
        int chId = channel_ids[j];
		__FLOAT__ * block_ptr = output_buffers + plan->frame_ln * chId;
		memcpy(output[j], block_ptr, numSamples * sizeof(__FLOAT__));
	}


    ReverbOCLRelOutput(config, plan, numChans, NULL/*&data_out_event*/);

 //  printf("get data from strm: %d q: 0x%x\n",  GetStreamIndex(plan), (uint)GetOCLQueue(plan));
//	err = clWaitForEvents(1,&data_out_event);
//	clReleaseEvent(data_out_event);
	return err;
}


#if 0
int ReverbOCLSetupReverbKernelProcess(ProjPlan * plan) {
int err = 0;

	uint init_flags = SchedGetInitFlags(GetScheduler(plan));
	if (init_flags & __INIT_FLAG_FHT__   && ( ReverbGetState(plan) & __STATE_SETUP_RVRB__ ))  {
		err = ReverbOCLMoveKernelIntoFHTPipeline2(plan, init_flags);
	} else {
//		err = ReverbOCLMoveKernelIntoPipeline2(plan);
	}


	return(err);


}
#endif

