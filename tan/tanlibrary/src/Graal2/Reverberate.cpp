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

#include "IIRLowLat.h"
#include "IIRLowLat_OCL.h"
#include "Reverberate2_OCL.h"
#include "DirectConv_OCL.h"
#include "amdFHT_OCL.h"
#include "amdFIR_OCL.h"
#include "public/common/AMFFactory.h"

#define min(a,b)(a < b ? a : b)
#define max(a,b)(a > b ? a : b)

static
int streamInOutPassDeinit(ProjPlan * plan);

#if defined DEFINE_AMD_OPENCL_EXTENSION
#ifndef CL_MEM_USE_PERSISTENT_MEM_AMD
#define CL_MEM_USE_PERSISTENT_MEM_AMD       (1 << 6)
#endif
#endif






/****************************************************************************************
Reverb internals
****************************************************************************************/


HSA_OCL_ConfigS * GetOclConfig(ProjPlan * plan) {
	HSA_OCL_ConfigS * ret = (HSA_OCL_ConfigS * )plan->OclConfig;

	return ret;
}


amdAudShcheduler * GetScheduler(ProjPlan * plan) {
	amdAudShcheduler * ret = (amdAudShcheduler *)plan->AmdSched;
	return ret;
}

int GetPlanID(ProjPlan * plan) {
	return(plan->plan_id);
}



cl_context GetOCLContext(ProjPlan *plan) {
	return(GetContext(GetOclConfig(plan)));
}

cl_command_queue GetOCLQueue(ProjPlan *plan) {
cl_command_queue ret = 0;

   ret = plan->OCLqueue[0];
   return ret;
}

int GetOCLWavefrontLg2(ProjPlan *plan) {
	return(GetWavefrontLg2(GetOclConfig(plan)));
}

int ReverbOCLInitializeInternal(ProjPlan * plan, const char * ocl_kernels_path, int init_flags) {
int err = 0;

// default
	plan->verification = init_flags & __INIT_FLAG_VERIFY__;

	plan->frame_ln = __DEFAULT_BLOCK_SIZE__;

	plan->frame2_ln_log2 = 10;
	plan->frame2_ln = ( 1 << plan->frame2_ln_log2 );
	plan->in_que_ln = 2;

	plan->nmb_channels = __DEFAULT_NUM_CHANNELS__;
	plan->conv_kernel_ln = __DEFAULT_REVERB_LN__;


    if ( ocl_kernels_path ) {
		plan->oclexe_path = (char*)malloc(strlen(ocl_kernels_path) + 1);
		strcpy(plan->oclexe_path, ocl_kernels_path); //HACK//
	}
	return (err);
}

ProjPlan * CreatePlan(int init_flags) {
ProjPlan * plan = NULL;
	plan = (ProjPlan *)malloc(sizeof(ProjPlan));
	if ( plan ) {
		memset(plan, 0, sizeof(ProjPlan));
	}
	else {
		printf("Can not create pipiline: out of system resources\n");
	}
	return(plan);
}

static
void CompileAllKernels( ProjPlan * plan) {
    CreateProgram2(plan, "amdFHT_kernels", __AMD_FHT_PROGRAM_INDEX__, "-cl-fp32-correctly-rounded-divide-sqrt");
	CreateProgram2(plan, "amdFHTbig_kernels", __AMD_FHTBIG_PROGRAM_INDEX__, "-cl-fp32-correctly-rounded-divide-sqrt");
	CreateProgram2(plan, "amdFHTmad_kernels", __AMD_FHTMAD_PROGRAM_INDEX__, "-cl-fp32-correctly-rounded-divide-sqrt");
	CreateProgram2(plan, "amdFHT2_kernels", __AMD_FHT2_PROGRAM_INDEX__, "-cl-fp32-correctly-rounded-divide-sqrt");
	const char * fir_src_nm = "amdFIR_kernels";
	CreateProgram2(plan, fir_src_nm, __AMD_FIR_PROGRAM_INDEX__, "-cl-fp32-correctly-rounded-divide-sqrt");
	CreateProgram2(plan, "Util_kernels", __UTIL_PROGRAM_INDEX__, "-cl-fp32-correctly-rounded-divide-sqrt");
}

int ReverbOCLInitialize(ProjPlan * plan, amdOCLRvrb *new_plan, const char * ocl_kernels_path, int init_flags,
                                 amf::AMFComputePtr amf_compute_conv, amf::AMFComputePtr amf_compute_update) {
int err = 0;
	err = OCLInit(plan, init_flags , amf_compute_conv, amf_compute_update);
	plan->Stat = (StatisticsPerRun *)malloc(sizeof(StatisticsPerRun));
	if (err || !plan->Stat)  {
		err = -1;
	    printf("Failed to start OpenCL with error %d\n", err);

		OCLTerminate(plan);
		free(plan);
	    exit(err);
	}

	err = ReverbOCLInitializeInternal(plan, ocl_kernels_path, init_flags);
// inital setup - kernel compile

	*new_plan = (amdOCLRvrb)plan;

// compiler kerenl once
	CompileAllKernels(plan);

// HERE we are setting global block counter to 0
	SetRunCounter(plan, 0);
	return err;
}

int ReverbOCLInitialize(ProjPlan * plan, amdOCLRvrb *new_plan, const char * ocl_kernels_path, int init_flags,
	cl_command_queue OCLqueue_conv, cl_command_queue OCLqueue_update) {
	int err = 0;
	err = OCLInit(plan, init_flags, OCLqueue_conv, OCLqueue_update);
	plan->Stat = (StatisticsPerRun *)malloc(sizeof(StatisticsPerRun));
	if (err || !plan->Stat) {
		err = -1;
		printf("Failed to start OpenCL with error %d\n", err);

		OCLTerminate(plan);
		free(plan);
		exit(err);
	}

	err = ReverbOCLInitializeInternal(plan, ocl_kernels_path, init_flags);
	// inital setup - kernel compile

	*new_plan = (amdOCLRvrb)plan;

	// compiler kerenl once
	CompileAllKernels(plan);

	// HERE we are setting global block counter to 0
	SetRunCounter(plan, 0);
	return err;
}



int ReverbOCLTerminateDeffered(ProjPlan * plan) {
	int err = 0;

	ReverbSetState(plan,(ReverbGetState(plan) | __STATE_UNSETUP_FIR__ | __STATE_UNSETUP_RVRB__));
    ReverbOCLUnsetup(plan);



		/*if (plan->oclexe_path) {
			free(plan->oclexe_path);
			plan->oclexe_path= 0;
		}*/

		if ( plan->conv_kernel_sys ) {
			free(plan->conv_kernel_sys);
			plan->conv_kernel_sys = 0;
		}

// verification buf
		if ( plan->conv_krnl_chnls_f ) {
			free(plan->conv_krnl_chnls_f);
			plan->conv_krnl_chnls_f = 0;
		}
		if ( plan->conv_chnls_f ) {
			free(plan->conv_chnls_f);
			plan->conv_chnls_f = 0;
		}

	ReverbSetState(plan,(ReverbGetState(plan) & ~( __STATE_UNSETUP_FIR__ | __STATE_UNSETUP_RVRB__) ));



	return err;
}

////////////////////////////////////////////////////////
// moves the cached block size into actuality
// calculates major pipeline parameters
// possibly
// selects unifor/non-unifiorm path
//
////////////////////////////////////////////////////////

int ReverbSetBlockSize2(ProjPlan * plan, int flame2_ln) {
	int ret = 0;

	int init_flags = SchedGetInitFlags(GetScheduler(plan));
	if ( (init_flags & __INIT_FLAG_FIR__) ) {
		init_flags &= ~__INIT_FLAG_2STREAMS__;
		init_flags &= ~__INIT_FLAG_HETEROGEN__;
		 SchedSetInitFlags(GetScheduler(plan),init_flags);
	}

	plan->frame_max_ln = plan->frame_max_ln_staged;
	plan->frame_ln_log2 = (int)ceil(log((double)plan->frame_max_ln)/log(2.));
	plan->frame_ln = (1 << plan->frame_ln_log2);

	if ( flame2_ln == 0 ) {
		int log_diff = 2;
		if (plan->frame_ln_log2 < 8) {
			log_diff = 4;
		} else if (plan->frame_ln_log2 < 10) {
			log_diff = 3;
		}
		plan->frame2_ln_log2 = plan->frame_ln_log2 + log_diff;
	} else {
		plan->frame2_ln_log2 = (int)ceil(log((double)flame2_ln)/log(2.));
	}

	plan->frame2_ln = ( 1 << plan->frame2_ln_log2 );
	plan->in_que_ln = (SchedGetInitFlags(GetScheduler(plan)) & __INIT_FLAG_2STREAMS__) ? (1 << ( plan->frame2_ln_log2 - plan->frame_ln_log2)) * 2 : 2;

	return ret;
}




int ReverbOCLIsActive(ProjPlan * plan) {
	int ret = 0;

	int init_flags = SchedGetInitFlags(GetScheduler(plan));

	int active = ((init_flags & __INIT_FLAG_FIR__) && !(init_flags & __INIT_FLAG_FHT__)) ?
					(ReverbGetState(plan) & __STATE_ACTIVE_FIR__)
					: (ReverbGetState(plan) & __STATE_ACTIVE__);

	ret = active;

	return ret;
}

int ReverbGetNConvBlocks(ProjPlan * plan) {
int ret = 0;
	if ( plan ) {
		amdAudShcheduler * sched = GetScheduler(plan);
		bool ffts2 = (SchedGetInitFlags(sched) & __INIT_FLAG_2FFT_STREAMS__)? true:false;
		int frame_ln = (ffts2) ? plan->frame2_ln : plan->frame_ln;
		ret = (((plan->conv_kernel_ln + frame_ln) - 1) / (frame_ln));
	}
	return(ret);
}

int ReverbGetNConvBlocksToFFT(ProjPlan * plan) {
int ret = 0;
    if ( plan ) {
		ret = plan->conv_blocks * plan->conv_NumChannels;
	}
	return(ret);
}

int ReverbGetNDataBlocksToFFT(ProjPlan * plan) {
int ret = 0;

	ret = plan->nmb_channels;

	return(ret);
}


__int64 GetRunCounter(ProjPlan *plan) {
__int64 ret = -1;
	ret = plan->run;
	return (ret);
}

void SetRunCounter(ProjPlan *plan, __int64 new_val) {

	plan->run = new_val;

}



int ReverbGetState(ProjPlan *plan) {
int ret = plan->state;
	return (ret);
}

void ReverbSetState(ProjPlan *plan, int new_state) {
	plan->state = new_state;
}

void ReverbOrState(ProjPlan *plan, int new_state) {
	plan->state |= new_state;
}

void ReverbAndState(ProjPlan *plan, int new_state) {
	plan->state &= new_state;
}

///////////////////////////////////////////////
// SETUP
/////////////////////////////////////////////////

void clearBuffers(ProjPlan * plan)
{
    cl_command_queue queue = plan->OCLqueue[0];
    STREAM_INOUT * in_out_plan = (STREAM_INOUT *)plan->streamBlockInOutpass;
    amdFHT_OCL *fht_plan = (amdFHT_OCL *)plan->GPUConvpass;
    SetValue(queue, &(fht_plan->dirFHT10), 0.0);
    for(int i = 0; i < 2; i++)
        SetValue(queue, &(fht_plan->dirFHT20[i]), 0.0);
    for (int i = 0; i < 2; i++)
        SetValue(queue, &(fht_plan->invFHT10[i]), 0.0);
    for (int i = 0; i < 2; i++)
        SetValue(queue, &(fht_plan->invFHT20[i]), 0.0);
    SetValue(queue, &(fht_plan->accum10), 0.0);
    SetValue(queue, &(fht_plan->accum20), 0.0);
    SetValue(queue, &(in_out_plan->stream2_out[0]), 0.0);
    SetValue(queue, &(in_out_plan->stream2_out[1]), 0.0);
    SetValue(queue, &(in_out_plan->stream_in[0]), 0.0);
    SetValue(queue, &(in_out_plan->stream_in[1]), 0.0);
    SetRunCounter(plan, 0);
    clFinish(queue);
}

int ReverbOCLSetupProcess(ProjPlan * plan) {
int err = 0;
	uint init_flags = SchedGetInitFlags(GetScheduler(plan));
 // scatter-gather or input/output presetup setup
	ReverbOCLSetupSG( plan);
	err = ReverbOCLSetupScatterGather(plan);

	if ( ReverbGetState(plan) & __STATE_SETUP_RVRB__ ) {

 // CPU processing presetup
		err |= DirectConvCPUSetup(plan);
        plan->xfade_state = 0;
        plan->first_ir_upload = 1;
		if (init_flags & __INIT_FLAG_FHT__  )  {

            plan->GPUConvpass = malloc(sizeof(amdFHT_OCL));
            assert(plan->GPUConvpass);
            memset(plan->GPUConvpass, 0, sizeof(amdFHT_OCL));


            amdFHT_OCL *fht_plan = (amdFHT_OCL *)plan->GPUConvpass;
            int frame_ln, frame1_ln;

            __FLOAT__ * host_kern1 = NULL;
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
            cl_command_queue queue = plan->OCLqueue[1];
            cl_context ctxt = GetOCLContext(plan);
            // IR1
            __FLOAT__ *sincos1;

            int block_sz = plan->frame_ln * 2;
            int n_conv_blocks = (init_flags & __INIT_FLAG_2STREAMS__) ? plan->in_que_ln : plan->conv_blocks;
            short * bit_reverse1;
            // get fht1 lookup tables
            FHTInit(&sincos1, &bit_reverse1, (FHT_FUNC *)&fht_plan->FHT_routine1, block_sz);

            fht_plan->bireverse1.len = block_sz * sizeof(short);
            if ((err = CreateOCLBuffer2(ctxt, &fht_plan->bireverse1, CL_MEM_USE_PERSISTENT_MEM_AMD | CL_MEM_READ_ONLY)) == CL_SUCCESS)
            {
                fht_plan->bireverse1.sys = bit_reverse1;
                CopyToDevice(queue, &fht_plan->bireverse1);
            }

            fht_plan->sincos1.len = block_sz * sizeof(__FLOAT__);
            if ((err = CreateOCLBuffer2(ctxt, &fht_plan->sincos1, CL_MEM_USE_PERSISTENT_MEM_AMD | CL_MEM_READ_ONLY)) == CL_SUCCESS)
            {
                fht_plan->sincos1.sys = sincos1;
                CopyToDevice(queue, &fht_plan->sincos1);
            }
            fht_plan->updateChannelMap.len = plan->conv_NumChannels * sizeof(cl_int);
            if ((err = CreateOCLBuffer2(ctxt, &fht_plan->updateChannelMap, CL_MEM_USE_PERSISTENT_MEM_AMD| CL_MEM_READ_ONLY)) != CL_SUCCESS)
            {
                printf("Could not initialize the updateChannelIds buffer\n");
                return err;
            }
            fht_plan->convChannelMap.len = plan->conv_NumChannels * sizeof(cl_int);
            if ((err = CreateOCLBuffer2(ctxt, &fht_plan->convChannelMap, CL_MEM_USE_PERSISTENT_MEM_AMD| CL_MEM_READ_ONLY)) != CL_SUCCESS)
            {
                printf("Could not initialize the processChannelIds buffer\n");
                return err;
            }
            char ir_kernel_nm[1024];
            char fromq_kernel_nm[1024];
            char froms_kernel_nm[1024];
            char froms_xfade_kernel_nm[1024];
            sprintf(ir_kernel_nm,"amdFHT%d", block_sz);
            sprintf(fromq_kernel_nm,"amdFHT%dFromQ", block_sz);
            sprintf(froms_kernel_nm,"amdFHT%dFromS", block_sz);
            sprintf(froms_xfade_kernel_nm, "amdFHT%dFromSXFade", block_sz);
            fht_plan->fht_kernels[FHT1_OCL] = CreateOCLKernel2(
                plan,
                ir_kernel_nm,
                block_sz > 2048 ? __AMD_FHTBIG_PROGRAM_INDEX__ : __AMD_FHT_PROGRAM_INDEX__,
                ComputeDeviceIdx::COMPUTE_DEVICE_UPDATE
                );
            fht_plan->fht_kernels[FHT1_OCL_FROMQUE] = CreateOCLKernel2(
                plan,
                fromq_kernel_nm,
                block_sz > 2048 ? __AMD_FHTBIG_PROGRAM_INDEX__ : __AMD_FHT_PROGRAM_INDEX__
                );
            // for 2 streams the second stream's addition is inside FHT inverse
            int out_strm_src = block_sz > 2048
                ? __AMD_FHTBIG_PROGRAM_INDEX__
                : (init_flags & __INIT_FLAG_2STREAMS__) ? __AMD_FHT2_PROGRAM_INDEX__ : __AMD_FHT_PROGRAM_INDEX__;
            fht_plan->fht_kernels[FHT1_OCL_FROMSTREAM] = CreateOCLKernel2(plan, froms_kernel_nm, out_strm_src);
            fht_plan->fht_kernels[FHT1_OCL_FROMSTREAM_XFADE] = CreateOCLKernel2(plan, froms_xfade_kernel_nm, out_strm_src);


            stream1_chnl_stride = conv_blocks1 * frame1_ln * 2;
            stream1_sz = plan->conv_NumChannels * stream1_chnl_stride * sizeof(__FLOAT__);


            size_t long_strm_chnl_stride = frame_ln * plan->conv_blocks * 2;
            size_t long_stream_sz = plan->conv_NumChannels* long_strm_chnl_stride * sizeof(__FLOAT__);

            //double* sum_sq = (double*)malloc(sizeof(double) * plan->conv_NumChannels);

            fht_plan->IR1 = new OCLBuffer[plan->num_IR_buffers];
            fht_plan->IR2 = new OCLBuffer[plan->num_IR_buffers];
            for (int i = 0; i < plan->num_IR_buffers; i++)
            {
                fht_plan->IR1[i].len = stream1_sz;
                err |= CreateOCLBuffer2(ctxt, &fht_plan->IR1[i], 0);
                err |= SetValue(queue, &fht_plan->IR1[i], 0.0);

                fht_plan->IR2[i].len = long_stream_sz;
                err |= CreateOCLBuffer2(ctxt, &fht_plan->IR2[i], 0);
                err |= SetValue(queue, &fht_plan->IR2[i], 0.0);
            }
            fht_plan->IR1_host = NULL;// will be created if needed host_kern1;
            fht_plan->IR2_host = NULL;// will be created later if needed host_kern_exp;
            if (err != CL_SUCCESS)
            {
                printf("Could not initialize the IR buffers\n");
                return err;
            }
            // allocate buffers for input direct fht transform, stream1
            fht_plan->dirFHT10.len = plan->conv_NumChannels * block_sz * sizeof(__FLOAT__);
            if ((err = CreateOCLBuffer2(ctxt, &fht_plan->dirFHT10, 0)) == CL_SUCCESS)
            {
                SetValue(queue, &(fht_plan->dirFHT10), 0.0);
            }
            for (int i = 0; i < 2; i++)
            {
                fht_plan->invFHT10[i].len = plan->conv_NumChannels * block_sz * sizeof(__FLOAT__);
                if ((err = CreateOCLBuffer2(ctxt, &fht_plan->invFHT10[i], 0)) == CL_SUCCESS)
                {
                    SetValue(queue, &(fht_plan->invFHT10[i]), 0.0);
                }
            }

            // here is an attemt to run s single FHT for CPU and non cpu pipeline but on CPU
            // it did note bode well due to restricted parllelizam of CPU and GPU in the case.
            // not used right now but should work
            if (init_flags & __INIT_FLAG_FHT1_ONCPU__)
            {
                fht_plan->dirFHT11.len = plan->conv_NumChannels * block_sz * sizeof(__FLOAT__);
                if ((err = CreateOCLBuffer2(ctxt, &fht_plan->dirFHT11, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR)) == CL_SUCCESS)
                {
                    // map from the beggining and unmap only at deinit.
                    // data cosistency is guranteed by map call at the begining of the loop

                    MapOCLBuffer(&fht_plan->dirFHT11, queue, CL_MAP_WRITE_INVALIDATE_REGION);
                }

                fht_plan->invFHT11.len = plan->conv_NumChannels * block_sz * sizeof(__FLOAT__);
                if ((err = CreateOCLBuffer2(ctxt, &fht_plan->invFHT11, CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR)) == CL_SUCCESS)
                {
                    // map of the file is going to be a batch breaker.
                    // data cosistency is guranteed by map call at the begining of the loop
                    // the 1st call returns all 0s
                    SetValue(queue, &(fht_plan->invFHT11), 0.0);
                    MapOCLBuffer(&fht_plan->invFHT11, queue, CL_MAP_READ);
                }
                fht_plan->fhtmad_data = malloc(plan->conv_NumChannels * block_sz * sizeof(__FLOAT__));

            }

            // the buffer has 2 purposes depending on the pipeline arrangment -
            // uniform (1 stream) case it keeps all previous FHT transforms
            // non-uniform case,it's accumulator for each kernel bin

            fht_plan->accum10.len = plan->conv_NumChannels * n_conv_blocks * block_sz * sizeof(__FLOAT__);
            if ((err = CreateOCLBuffer2(ctxt, &fht_plan->accum10, 0)) == CL_SUCCESS)
            {
                //SetValue(plan, &fht_plan->accum10, 0);
                SetValue(queue, &fht_plan->accum10, 0.0);
            }
            fht_plan->accum_loops = 16;

            if (init_flags & __INIT_FLAG_2STREAMS__)
            {
                fht_plan->fht_kernels[FHT1_OCL_MAD] = CreateOCLKernel2(plan, "FHTMultAddAccum", __AMD_FHTMAD_PROGRAM_INDEX__);
            }


            fht_plan->fht_kernels[FHT1_OCL_MAD_HEAD] = CreateOCLKernel2(plan, "FHTMultAddHead", __AMD_FHTMAD_PROGRAM_INDEX__);
            fht_plan->fht_kernels[FHT1_OCL_MAD_HEAD_XFADE] = CreateOCLKernel2(plan, "FHTMultAddHeadXFade", __AMD_FHTMAD_PROGRAM_INDEX__);
            fht_plan->fht_kernels[FHT1_OCL_MAD_DEVIDED] = CreateOCLKernel2(plan, "FHTMultAddHeadDevided", __AMD_FHTMAD_PROGRAM_INDEX__);
            fht_plan->fht_kernels[FHT1_OCL_MAD_DEVIDED_XFADE] = CreateOCLKernel2(plan, "FHTMultAddHeadDevidedXFade", __AMD_FHTMAD_PROGRAM_INDEX__);
            fht_plan->fht_kernels[FHT1_OCL_MAD_TAIL] = CreateOCLKernel2(plan, "FHTMultAddTail", __AMD_FHTMAD_PROGRAM_INDEX__);
            fht_plan->fht_kernels[FHT1_OCL_MAD_TAIL_XFADE] = CreateOCLKernel2(plan, "FHTMultAddTailXFade", __AMD_FHTMAD_PROGRAM_INDEX__);

            // it keeps partial sums in 1 stream case
            int n_sums = (n_conv_blocks + fht_plan->accum_loops - 1) / fht_plan->accum_loops;

            // For summation-based second stream MAD
            int block_sz2 = plan->frame2_ln * 2;
            int n_conv_blocks2 = plan->conv_blocks;
            int accum_loop2 = n_conv_blocks2;// loop through all bins but split the work among cycles
            int num_sum2 = 1;// A single accumulator buffer that keeps the final results
            for (int i = 0; i < 2; i++)
            {
                fht_plan->accum21[i].len = plan->conv_NumChannels * num_sum2 * block_sz2 * sizeof(__FLOAT__);
                if ((err = CreateOCLBuffer2(ctxt, &fht_plan->accum21[i], 0)) == CL_SUCCESS)
                {
                    SetValue(queue, &fht_plan->accum21[i], 0.0);
                }
                fht_plan->accum11[i].len = plan->conv_NumChannels * n_sums * block_sz * sizeof(__FLOAT__);
                if ((err = CreateOCLBuffer2(ctxt, &fht_plan->accum11[i], 0)) == CL_SUCCESS)
                {
                    SetValue(queue, &fht_plan->accum11[i], 0.0);
                }
            }

            fht_plan->fht_kernels[PARTITION_KERNEL] = CreateOCLKernel2(plan, "partitionIR", __UTIL_PROGRAM_INDEX__ , ComputeDeviceIdx::COMPUTE_DEVICE_UPDATE);

            if (init_flags & __INIT_FLAG_2STREAMS__)
            {
                __FLOAT__ *sincos2;
                short *bit_reverse2;
                int block_sz = plan->frame2_ln * 2;
                int n_conv_blocks = plan->conv_blocks;
                // get fht2 lookup tables
                FHTInit(&sincos2, &bit_reverse2, (FHT_FUNC *)&fht_plan->FHT_routine2, block_sz);

                fht_plan->bireverse2.len = block_sz * sizeof(short);
                if ((err = CreateOCLBuffer2(ctxt, &fht_plan->bireverse2, CL_MEM_READ_ONLY)) == CL_SUCCESS)
                {
                    fht_plan->bireverse2.sys = bit_reverse2;
                    CopyToDevice(queue, &fht_plan->bireverse2);
                }

                fht_plan->sincos2.len = block_sz * sizeof(__FLOAT__);
                if ((err = CreateOCLBuffer2(ctxt, &fht_plan->sincos2, CL_MEM_READ_ONLY)) == CL_SUCCESS)
                {
                    fht_plan->sincos2.sys = sincos2;
                    CopyToDevice(queue, &fht_plan->sincos2);
                }


                sprintf(ir_kernel_nm,"amdFHT%d", block_sz);
                sprintf(fromq_kernel_nm,"amdFHT%dFromQ", block_sz);
                sprintf(froms_kernel_nm,"amdFHT%dFromS", block_sz);
                sprintf(froms_xfade_kernel_nm, "amdFHT%dFromSXFade", block_sz);

                int prog_index = (block_sz <= 2048) ? __AMD_FHT_PROGRAM_INDEX__ : __AMD_FHTBIG_PROGRAM_INDEX__;
                fht_plan->fht_kernels[FHT2_OCL] = CreateOCLKernel2(plan, ir_kernel_nm, prog_index, ComputeDeviceIdx::COMPUTE_DEVICE_UPDATE);
                fht_plan->fht_kernels[FHT2_OCL_FROMQUE] = CreateOCLKernel2(plan, fromq_kernel_nm, prog_index);
                fht_plan->fht_kernels[FHT2_OCL_FROMSTREAM] = CreateOCLKernel2(plan, froms_kernel_nm, prog_index);
                fht_plan->fht_kernels[FHT2_OCL_FROMSTREAM_XFADE] = CreateOCLKernel2(plan, froms_xfade_kernel_nm, prog_index);

                for (int i = 0; i < 2; i++)
                {
                    fht_plan->dirFHT20[i].len = plan->conv_NumChannels * block_sz * sizeof(__FLOAT__);
                    if ((err = CreateOCLBuffer2(ctxt, &fht_plan->dirFHT20[i], 0)) == CL_SUCCESS)
                    {
                        SetValue(queue, &fht_plan->dirFHT20[i], 0.0);
                    }
                }
                for (int i = 0; i < 2; i++)
                {
                    fht_plan->invFHT20[i].len = plan->conv_NumChannels * block_sz * sizeof(__FLOAT__);
                    if ((err = CreateOCLBuffer2(ctxt, &fht_plan->invFHT20[i], 0)) == CL_SUCCESS)
                    {
                        SetValue(queue, &fht_plan->invFHT20[i], 0.0);
                    }
                }
                fht_plan->accum20.len = plan->conv_NumChannels * n_conv_blocks * block_sz * sizeof(__FLOAT__);
                if ((err = CreateOCLBuffer2(ctxt, &fht_plan->accum20, 0)) == CL_SUCCESS)
                {
                    //SetValue(plan, &fht_plan->accum20, 0);
                    SetValue(queue, &fht_plan->accum20, 0.0);
                }
#if 0
                fht_plan->syncs.len = plan->conv_NumChannels * sizeof(uint);
                if ((err = CreateOCLBuffer2(ctxt, &fht_plan->syncs, 0)) == CL_SUCCESS)
                {
                    SetValue(plan, &fht_plan->syncs, 0);
                }
#endif
                // fht mad kernel setup

                fht_plan->fht_kernels[FHT2_OCL_MAD] = CreateOCLKernel2(plan, "FHTMultAddAccum", __AMD_FHTMAD_PROGRAM_INDEX__);
                fht_plan->fht_kernels[FHT2_OCL_FROMSTREAM_REVERSE] = CreateOCLKernel2(plan, "amdFromQReverse", __AMD_FHT2_PROGRAM_INDEX__);
            }
            fht_plan->fht_kernels[FHTG_OCL] = CreateOCLKernel2(plan, "amdFHTGlbl", __AMD_FHT2_PROGRAM_INDEX__);
            clFinish(queue);

		} else {
//		err = ReverbOCLMoveKernelIntoPipeline2(plan);
		}
	}

	if ( ReverbGetState(plan) & __STATE_SETUP_FIR__ ) {
		firDirectSetup(plan);
	}

	return err;
}

///////////////////////////////////
// major parameters setup
///////////////////////////////////
void graalSetupParams(ProjPlan * plan, int init_flags) {
	bool s2 = (init_flags & __INIT_FLAG_2FFT_STREAMS__)? true:false;
	plan->frame_ln_log2 = (int)ceil(log((double)plan->frame_max_ln)/log(2.));
	plan->frame_ln = (1 << plan->frame_ln_log2);
    plan->in_que_ln = (s2) ? (1 << ( plan->frame2_ln_log2 - plan->frame_ln_log2)) * 2 : 2;

	plan->group_ln_log2 = max(min(8, plan->frame_ln_log2), GetOCLWavefrontLg2(plan));
	plan->local_mem_buffer_log2 = 10;
	plan->conv_blocks = (s2) ? (plan->conv_kernel_ln + plan->frame2_ln -1) / plan->frame2_ln : (plan->conv_kernel_ln + plan->frame_ln -1) / plan->frame_ln;
	plan->conv_blocks1 = (plan->conv_kernel_ln + plan->frame_ln -1) / plan->frame_ln;
	plan->conv_buffer1_ln = plan->conv_blocks1 *  plan->frame_ln;
	plan->conv_buffer_ln = plan->conv_blocks * ((s2) ? plan->frame2_ln : plan->frame_ln);
	plan->conv_buffer_ln_log2 = (int)ceil(log((double)plan->conv_buffer_ln)/log(2.));
}


int ReverbOCLSetupInternal(ProjPlan * plan) {
	int err = 0;

	amdAudShcheduler * sched = GetScheduler(plan);
// release all resources from a previous kernel dowwload if it ever happpened
//	ReverbOCLUnsetup(plan);

// move a new block size from a cached state into actuallity
// here there might be some heurisric to switch from a 2 streams to a 1 stream pipline and back.
	ReverbSetBlockSize2(plan, 0);

// running parameters
	graalSetupParams(plan, SchedGetInitFlags(sched));

// the rest of the real setup
	err = ReverbOCLSetupProcess(plan);


	return err;
}


///////////////////////////
// SET START run counter to 0 !!!
//  plan->run = 0;
////////////////////////////

int ReverbOCLSetup(ProjPlan * plan) {
	int err = 0;
// HERE we are setting global block counter to 0
//	SetRunCounter(plan, 0);

	err = ReverbOCLSetupInternal(plan);

	return err;
}



//////////////////////////////////////////////////
// KERNEL setup
/////////////////////////////////////////////////


// length in seconds
int ReverbOCLSetupReverbKernel(ProjPlan * plan) {
	int err = 0;

// stop playing
    ReverbSetState(plan, (ReverbGetState(plan) & ~__STATE_ACTIVE__));

	ReverbSetState(plan,(ReverbGetState(plan) | __STATE_SETUP_RVRB__ | __STATE_UNSETUP_RVRB__));

	ReverbOCLSetupInternal(plan);
	ReverbSetState(plan,(ReverbGetState(plan) & ~(__STATE_SETUP_RVRB__ | __STATE_UNSETUP_RVRB__)));
// start playing
// IsActive external interface
	ReverbSetState(plan,(ReverbGetState(plan) | __STATE_ACTIVE__));

	return err;
}


//////////////////////////////////////////////////
// Flushing the input history buffers
/////////////////////////////////////////////////
int ReverbFlushHistory(ProjPlan* plan, int channelId)
{
    int ret = 0;
    cl_command_queue queue = plan->OCLqueue[0];
    STREAM_INOUT * in_out_plan = (STREAM_INOUT *)plan->streamBlockInOutpass;

    amdFHT_OCL *fht_plan = (amdFHT_OCL *)plan->GPUConvpass;

    //1- Clearing the input staging buffer (time domain samples)  clear the whole circular buffer for the requested channel
    int chnl_stride = plan->frame_ln * plan->in_que_ln * sizeof(__FLOAT__);
    SetValue(queue, &in_out_plan->stream_in[0], 0.0, chnl_stride * channelId, chnl_stride);

    // 2- Cleaning the FHT history buffers
    uint init_flags = SchedGetInitFlags(GetScheduler(plan));
    {
        // Clear histpry buffer of all past fht transforms, stream1
        int total_n_bins = (init_flags & __INIT_FLAG_2STREAMS__) ? plan->in_que_ln : plan->conv_blocks;//Number of bins for MAD is different between uniform and non-uniform cases
        int block_sz = plan->frame_ln * 2;
        int chnl_stride = total_n_bins * block_sz * sizeof(__FLOAT__);
        SetValue(queue, &fht_plan->accum10, 0.0, chnl_stride * channelId , chnl_stride );
        //
        SetValue(queue, &fht_plan->accum11[0], 0.0, channelId * block_sz * sizeof(__FLOAT__), block_sz * sizeof(__FLOAT__));
        SetValue(queue, &fht_plan->accum11[1], 0.0, channelId * block_sz * sizeof(__FLOAT__), block_sz * sizeof(__FLOAT__));
        //
        SetValue(queue, &fht_plan->invFHT10[1], 0.0, channelId * block_sz * sizeof(__FLOAT__), block_sz * sizeof(__FLOAT__));
        SetValue(queue, &fht_plan->invFHT10[0], 0.0, channelId * block_sz * sizeof(__FLOAT__), block_sz * sizeof(__FLOAT__));
    }

    if (init_flags & __INIT_FLAG_2STREAMS__)
    {
        // Clear histpry buffer of all past fht transforms, stream2
        int total_n_bins = plan->conv_blocks;
        int block_sz = plan->frame2_ln * 2;
        int chnl_stride = total_n_bins * block_sz * sizeof(__FLOAT__);
        SetValue(queue, &fht_plan->accum20, 0.0, chnl_stride * channelId , chnl_stride);
        //
        SetValue(queue, &fht_plan->accum21[0], 0.0, channelId * block_sz * sizeof(__FLOAT__), block_sz * sizeof(__FLOAT__));
        SetValue(queue, &fht_plan->accum21[1], 0.0, channelId * block_sz * sizeof(__FLOAT__), block_sz * sizeof(__FLOAT__));
        //
        SetValue(queue, &fht_plan->invFHT20[1], 0.0, channelId * block_sz * sizeof(__FLOAT__), block_sz * sizeof(__FLOAT__));
        SetValue(queue, &fht_plan->invFHT20[0], 0.0, channelId * block_sz * sizeof(__FLOAT__), block_sz * sizeof(__FLOAT__));
        //
        SetValue(queue, &in_out_plan->stream2_out[0], 0.0, channelId * plan->frame2_ln * sizeof(__FLOAT__), plan->frame2_ln * sizeof(__FLOAT__));
        SetValue(queue, &in_out_plan->stream2_out[1], 0.0, channelId * plan->frame2_ln * sizeof(__FLOAT__), plan->frame2_ln * sizeof(__FLOAT__));


    }
    return ret;
}
