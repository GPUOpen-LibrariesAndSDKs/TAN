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

#ifndef __IIRLOWLAT_OCL__
#define __IIRLOWLAT_OCL__

#include "IIRLowLat.h"
#include "public/common/AMFFactory.h"  
#define __NUM_OCL_QUEUES__ 2
#define __NUM_STREAMS__ 2
#define __MAX_N_INSTNCES__ 2048

typedef enum {
    COMPUTE_DEVICE_CONV = 0,
    COMPUTE_DEVICE_UPDATE
} ComputeDeviceIdx;

static
double subtractTimes( double endTime, double startTime )
{
    double difference = endTime - startTime;
    static double conversion = 0.0;
    
    if( conversion == 0.0 )
    {
#if __APPLE__
        mach_timebase_info_data_t info;
        kern_return_t err = mach_timebase_info( &info );
        
		//Convert the timebase into seconds
        if( err == 0  )
			conversion = 1e-9 * (double) info.numer / (double) info.denom;
#endif
#ifdef _WIN32
		conversion = 1.;
#endif
    }
    
    return conversion * (double) difference;
}

typedef struct {
	uint len;
	void * sys;
	void * sys_map;
	cl_mem mem;
// 1 = external mem
	int flags;
	cl_context context;
	cl_command_queue commandQueue;
} OCLBuffer;


#define __MAX_N_KERNELS__     64
#define __MAX_N_PROGS__  64

#define __UTIL_PROGRAM_INDEX__   0

typedef int (* AMDOCLAUDIOPROC) (void *);

typedef struct _HSA_OCL_ConfigS
{
  cl_context context;
	int que_index;
	cl_command_queue OCLqueue[__NUM_OCL_QUEUES__];
  cl_device_id dev_id;
  int gpu_max_cus;
  int gpu_wavefront_ln_log2;
  __int64 gpu_max_memory_sz;
  __int64 gpu_max_mem_alloc_sz;

  cl_program program[__MAX_N_PROGS__];
  char *program_nm[__MAX_N_PROGS__];
  char * program_src[__MAX_N_PROGS__];
  int program_src_sz[__MAX_N_PROGS__];
  char *options[__MAX_N_PROGS__];
  int kernel_workgroup_size;
  cl_kernel kernels[__MAX_N_PROGS__][__MAX_N_KERNELS__];
  char *kernel_nm[__MAX_N_PROGS__][__MAX_N_KERNELS__];

// TO DO:
  void * schedulers[1]; // pointers on schedulers froma different initialization threads
  amf::AMFComputePtr amf_compute_conv;
  amf::AMFComputePtr amf_compute_update;
} HSA_OCL_ConfigS;


typedef struct {
// TO DO:
// id of initialization thread
	int init_thread_id;
// TO: PER INIT THREAD
	int instance_counter;
	void ** instances;
	int ext_appearence_counter;
	int appearence_counter;
	uint * order_of_appearence;
	AMDOCLAUDIOPROC * sceduled_jobs;
	int n_streams;

	__int64 data_block_counter;
// initialization flags set once!!!
	uint init_flags;
// to restire from
	uint base_init_flags;
// plan to share pipeline between all plans
	void * shared_plan;
} amdAudShcheduler;

typedef struct _ProjPlan ProjPlan;
#include "Reverberate_OCL.h"

int SetupHSAOpenCL(HSA_OCL_ConfigS** OCLConfigs, uint init_flags, cl_context context, cl_command_queue queue1, cl_command_queue queue2);
int CleanupHSAOpenCL(HSA_OCL_ConfigS** OCLConfigs);
amdAudShcheduler * GetSchedInstance(HSA_OCL_ConfigS* OCLConfigs, int sched_id);
int CreateScheduler(amdAudShcheduler ** sched, uint init_flags);
int DestructScheduler(amdAudShcheduler ** sched);

int SchedGetBaseInitFlags(amdAudShcheduler * sched);
int SchedGetInitFlags(amdAudShcheduler * sched);
int SchedSetInitFlags(amdAudShcheduler * sched, int init_flags);

int SchedGetInstanceCounter(amdAudShcheduler * sched);
void* const * SchedGetInstances(amdAudShcheduler * sched);

void SchedSetBlockCounter(amdAudShcheduler * sched, __int64 new_val);
__int64 SchedGetBlockCounter(amdAudShcheduler * sched);


int CreateContext(HSA_OCL_ConfigS *config);
int CreateStreamsQueue(HSA_OCL_ConfigS *config, cl_command_queue * Ques, int n_ques);
int CreateCommandQueue(HSA_OCL_ConfigS *config);

void GetInstanceInfo(HSA_OCL_ConfigS *config);
HSA_OCL_ConfigS* GetOCLConfig(void);
cl_command_queue GetGlobalQueue(amdAudShcheduler * sched, HSA_OCL_ConfigS *config);

cl_kernel CreateOCLKernel2(ProjPlan *plan, const char * kernel_nm, int prog_index, ComputeDeviceIdx compDevIdx = ComputeDeviceIdx::COMPUTE_DEVICE_CONV);


cl_context GetContext(HSA_OCL_ConfigS *config);
int GetWavefrontLg2(HSA_OCL_ConfigS *config);

int CreateOCLBuffer(cl_context context,OCLBuffer * buf, uint buffer_flags);
// without symmetrical system buffer
int CreateOCLBuffer2(cl_context context,OCLBuffer * buf, uint buffer_flags);
// set external memobj
void SetMemOCLBuffer(OCLBuffer * buf, cl_mem memObj, size_t len);


int  MapOCLBuffer(OCLBuffer * buf, cl_command_queue commandQueue, uint map_flags);
int UnmapOCLBuffer(cl_context context,OCLBuffer * buf);
bool IsMapped(OCLBuffer * buf);

int CopyToDevice(cl_command_queue commandQueue, OCLBuffer * buf, bool force = true);
// sys buffer should have exact buf->len lenght
int CopyToDevice2(cl_command_queue commandQueue, OCLBuffer * buf, void * sys, size_t sys_len, size_t offset = 0, bool force = true);
int CopyFromDevice(cl_command_queue commandQueue, OCLBuffer * buf);
int ReleaseOCLBuffer(OCLBuffer * buf);



#endif