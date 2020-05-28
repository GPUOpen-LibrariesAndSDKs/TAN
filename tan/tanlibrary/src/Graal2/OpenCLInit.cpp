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

#include <string>
#include "IIRLowLat.h"
#include "IIRLowLat_OCL.h"
#include <vector>
#include <array>
#include "OclKernels/CLKernel_amdFHT_kernels.h"
#include "OclKernels/CLKernel_amdFHT2_kernels.h"
#include "OclKernels/CLKernel_amdFHTmad_kernels.h"
#include "OclKernels/CLKernel_amdFHTbig_kernels.h"
#include "OclKernels/CLKernel_amdFIR_kernels.h"
#include "OclKernels/CLKernel_Util_kernels.h"
#include "public/common/AMFFactory.h"
#include "../clFFT-master/src/include/sharedLibrary.h"
#include "../common/OCLHelper.h"

typedef char * LPSTR;

#ifndef _WIN32
typedef void * HMODULE;
#endif

#ifndef _WIN32
    #define WINAPI
#endif

static int OclCounter = 0;
static HSA_OCL_ConfigS* OCLConfig = NULL;
static amdAudShcheduler* AmdScheduler = NULL;
typedef unsigned long DWORD;


int OCLInit(ProjPlan *plan, uint init_flags,  amf::AMFComputePtr amf_compute_conv, amf::AMFComputePtr amf_compute_update) {
int ret = 0;
// TODO: mutex
	if (0 == OclCounter++ ) {
		SetupHSAOpenCL(&OCLConfig, init_flags, (cl_context)amf_compute_conv->GetNativeContext(), (cl_command_queue)amf_compute_conv->GetNativeCommandQueue(), (cl_command_queue)amf_compute_update->GetNativeCommandQueue());

		CreateScheduler(&AmdScheduler, init_flags);
		OCLConfig->schedulers[0] = AmdScheduler;
        OCLConfig->amf_compute_conv = amf_compute_conv;
        OCLConfig->amf_compute_update = amf_compute_update;
	}

	plan->AmdSched = AmdScheduler;
	plan->OclConfig = OCLConfig;


	if ( !(SchedGetInitFlags(GetScheduler(plan)) & __INIT_FLAG_GLOBAL_QUEUES__) ) {
        if (amf_compute_conv->GetNativeContext() == 0)
        {
            CreateStreamsQueue(GetOclConfig(plan), plan->OCLqueue, __NUM_OCL_QUEUES__);
        }
        else
        {
            plan->OCLqueue[0] = (cl_command_queue)amf_compute_conv->GetNativeCommandQueue();
            plan->OCLqueue[1] = (cl_command_queue)amf_compute_update->GetNativeCommandQueue();
        }
	}
	return(ret);
}

int OCLInit(ProjPlan *plan, uint init_flags, cl_command_queue OCLqueue_conv, cl_command_queue OCLqueue_update) {
	int ret = 0;
	cl_context clContext;
	clGetCommandQueueInfo(OCLqueue_conv, CL_QUEUE_CONTEXT, sizeof(cl_context), &clContext, NULL);

	// TODO: mutex
	if (0 == OclCounter++) {
		SetupHSAOpenCL(&OCLConfig, init_flags, clContext, OCLqueue_conv, OCLqueue_update);

		CreateScheduler(&AmdScheduler, init_flags);
		OCLConfig->schedulers[0] = AmdScheduler;
		OCLConfig->amf_compute_conv = nullptr;
		OCLConfig->amf_compute_update = nullptr;
	}

	plan->AmdSched = AmdScheduler;
	plan->OclConfig = OCLConfig;


	if (!(SchedGetInitFlags(GetScheduler(plan)) & __INIT_FLAG_GLOBAL_QUEUES__)) {
		{
			plan->OCLqueue[0] = OCLqueue_conv;
			plan->OCLqueue[1] = OCLqueue_update;
		}
	}
	return(ret);
}



void DecreaseGlobalReferenceCounter(ProjPlan *plan) {
	OclCounter--;
}

int OCLTerminate(ProjPlan *plan) {
int ret = 0;

#if 0
        // Qeueus are passed from the user does not need to be cleaned and released here
		if ( !(SchedGetInitFlags(GetScheduler(plan)) & __INIT_FLAG_GLOBAL_QUEUES__) ) {
			for (int i = 0; i < __NUM_OCL_QUEUES__; i++) {

				if(plan->OCLqueue[i] != 0) {
					clReleaseCommandQueue(plan->OCLqueue[i]);
					plan->OCLqueue[i] = 0;
				}
			}
		}
#endif
	if ( 0 >= --OclCounter ) {
		DestructScheduler(&AmdScheduler);
		CleanupHSAOpenCL(&OCLConfig);
	}



	return(ret);
}


int CreateScheduler(amdAudShcheduler ** sched, uint init_flags) {
int ret = 0;

	if ( sched ) {
		*sched = (amdAudShcheduler *)malloc(sizeof(amdAudShcheduler));
		if ( !*sched ) {
			printf("CreateScheduler: ERROR: out of system memory\n");
			ret = -1;
			return ret;
		}

		memset(*sched, 0, sizeof(amdAudShcheduler));

		(*sched)->init_flags = init_flags;

		(*sched)->base_init_flags = init_flags;
	}
	return(ret);
}

int DestructScheduler(amdAudShcheduler ** sched) {
int ret = 0;


	if ( sched && (*sched) ) {
		free( (*sched) );
		(*sched) = 0;
	}
	return(ret);
}


int SchedGetBaseInitFlags(amdAudShcheduler * sched) {
int ret = 0;
    if ( sched ) {
		ret = sched->base_init_flags;
	}
	return ret;
}

int SchedGetInitFlags(amdAudShcheduler * sched) {
int ret = 0;
    if ( sched ) {
		ret = sched->init_flags;
	}
	return ret;
}

int SchedSetInitFlags(amdAudShcheduler * sched, int init_flags) {
int ret = 0;
    if ( sched ) {
		ret = sched->init_flags;
		sched->init_flags = init_flags;
	}
	return(ret);
}

int SchedGetInstanceCounter(amdAudShcheduler * sched) {
int ret = 0;
    if ( sched ) {
		ret = sched->instance_counter;
	}
	return ret;
}

void* const * SchedGetInstances(amdAudShcheduler * sched) {
void*const* ret = 0;
	if (sched) {
		ret = (void *const *)sched->instances;
	}
	return(ret);
}

void SchedSetBlockCounter(amdAudShcheduler * sched, __int64 new_val) {
	if ( sched ) {

		sched->data_block_counter = new_val;
	}
}

__int64 SchedGetBlockCounter(amdAudShcheduler * sched) {
__int64 ret = 0;
	if ( sched ) {
		ret = sched->data_block_counter;
	}
	return (ret);
}


int SetupHSAOpenCL(HSA_OCL_ConfigS** OCLConfigs, uint init_flags, cl_context context, cl_command_queue queue1, cl_command_queue queue2)
{
    *OCLConfigs = (HSA_OCL_ConfigS*)malloc(sizeof(HSA_OCL_ConfigS));
    memset(*OCLConfigs, 0, sizeof(HSA_OCL_ConfigS));
    if (context != 0)
    {
        (*OCLConfigs)->context = context;
        (*OCLConfigs)->OCLqueue[0] = queue2;
        cl_device_id device = NULL;
        clGetCommandQueueInfo(queue2, CL_QUEUE_DEVICE, sizeof(cl_device_id), &device, NULL);
        (*OCLConfigs)->dev_id = device;
    }
    else
    {
      // initialize OpenCL here
        int err;
        err = CreateContext(*OCLConfigs);
        if (err == 0) err = CreateCommandQueue(*OCLConfigs);
    }
    // get max device memory alloc size and kernel workgroup size
    GetInstanceInfo(*OCLConfigs);

  return(CL_SUCCESS);
}


int CleanupHSAOpenCL(HSA_OCL_ConfigS** OCLConfigs)
{
  if(!OCLConfigs || !(*OCLConfigs)) return 0;
 // if ( 0 == --OclCounter ) {
  // TODO: clean up OpenCL here

	for (int i = 0; i < __NUM_OCL_QUEUES__; i++) {

       if((*OCLConfigs)->OCLqueue[i] != 0) {
		   clReleaseCommandQueue((*OCLConfigs)->OCLqueue[i]);
		   (*OCLConfigs)->OCLqueue[i] = 0;
	   }
	}


  for (int i = 0; i < __MAX_N_PROGS__; i++) {
       for (int j = 0; j < __MAX_N_KERNELS__; j++) {
		   if ( (*OCLConfigs)->kernel_nm[i][j]) {
			   free( (*OCLConfigs)->kernel_nm[i][j]);
			    (*OCLConfigs)->kernel_nm[i][j] = 0;
		   }

       }
  }
  for (int i = 0; i < __MAX_N_PROGS__; i++) {
       if((*OCLConfigs)->program[i] != 0) clReleaseProgram((*OCLConfigs)->program[i]);
	   if ( (*OCLConfigs)->program_nm[i]) {
		   free((*OCLConfigs)->program_nm[i]);
		   (*OCLConfigs)->program_nm[i] = 0;
	   }
	   if ( (*OCLConfigs)->options[i]) {
		   free((*OCLConfigs)->options[i]);
		   (*OCLConfigs)->options[i] = 0;
	   }
  }
  if((*OCLConfigs)->context != 0) {
	  clReleaseContext((*OCLConfigs)->context);
  }

  free(*OCLConfigs);
  *OCLConfigs= 0;

//  }
  return 0;
}


amdAudShcheduler * GetSchedInstance(HSA_OCL_ConfigS* configs, int sched_id) {
	return((amdAudShcheduler *)configs->schedulers[sched_id]);
}

void * GPUUtilitiesDll = NULL;

bool GetDeviceFromIndex(int deviceIndex, cl_device_id *device, cl_context *context, cl_device_type clDeviceType)
{

    if(!GPUUtilitiesDll)
        GPUUtilitiesDll = LoadSharedLibrary("lib", "GPUUtilities", true);
    if (NULL == GPUUtilitiesDll)
        return false;

    typedef int  (WINAPI *getDeviceAndContextType)(int devIdx, cl_context *pContext, cl_device_id *pDevice, cl_device_type clDeviceType);
    getDeviceAndContextType getDeviceAndContext = nullptr;
    getDeviceAndContext = (getDeviceAndContextType)LoadFunctionAddr(GPUUtilitiesDll, "getDeviceAndContext");
    if (NULL == getDeviceAndContext)
        return false;

    cl_context clContext = NULL;
    cl_device_id clDevice = NULL;
    //get context of open device via index 0,1,2,...
    getDeviceAndContext(deviceIndex, &clContext, &clDevice, clDeviceType);
    if (NULL == clContext)
        return false;
    if (NULL == clDevice)
        return false;

    *device = clDevice;
    *context = clContext;
    return true;
}

std::string GetLastErrorAsString()
{
    //Get the error message, if any.
#if defined( _WIN32 )
    DWORD errorMessageID = ::GetLastError();
#elif defined(__linux__) || defined(__GNU__) || (defined(__FreeBSD_kernel__) && defined(__GLIBC__)) || defined(__APPLE__) || defined(__FreeBSD__)
    DWORD errorMessageID = errno;
#else
    #error "unsupported platform"
#endif
//    if (errorMessageID == 0)
//        return std::string(); //No error message has been recorded
//
//    LPSTR messageBuffer = nullptr;
//    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | //FORMAT_MESSAGE_IGNORE_INSERTS,
//                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), //(LPSTR)&messageBuffer, 0, NULL);
//
//    std::string message(messageBuffer, size);
//
//    //Free the buffer.
//    LocalFree(messageBuffer);
//
//    return message;
    return std::string(); //HACK//
}

int listGpuDeviceNames(char *devNames[], unsigned int count)
{

    int foundCount = 0;

#ifdef __linux__
    typedef int(*listGpuDeviceNamesType)(char *devNames[], unsigned int count);
#else
    typedef int(__cdecl *listGpuDeviceNamesType)(char *devNames[], unsigned int count);
#endif
    listGpuDeviceNamesType listGpuDeviceNames = nullptr;
    if (!GPUUtilitiesDll)
        GPUUtilitiesDll = LoadSharedLibrary("lib", "GPUUtilities", true);
    if (!GPUUtilitiesDll)
        printf("%s\n",GetLastErrorAsString().c_str());
    if (NULL != GPUUtilitiesDll)
    {
        listGpuDeviceNames = (listGpuDeviceNamesType)LoadFunctionAddr(GPUUtilitiesDll, "listGpuDeviceNames");
        if (NULL != listGpuDeviceNames)
        {
            foundCount = listGpuDeviceNames(devNames, count);
        }
        else
        {
            printf("NOT FOUND listGpuDeviceNames in GPUUtils...");
        }
    }
    else
    {
        printf("NOT FOUND GPUUtilities.dll in GPUUtils...");
    }
    return foundCount;
}

int CreateContext(HSA_OCL_ConfigS *config)
{
    const unsigned NUM_DEVICES = 10;
    const unsigned MAX_NAME_LEN = 100;
    std::vector<char*> devNames(NUM_DEVICES);
    std::array<char, MAX_NAME_LEN> names[NUM_DEVICES];
    for (int i = 0; i < NUM_DEVICES; i++)
    {
        names[i].fill('\0');
        devNames[i] = names[i].data();
    }
    int count = listGpuDeviceNames(devNames.data(), NUM_DEVICES);
    printf("# of GPU device(s) %d , Names : \n",count);
    for (int i = 0; i < count; i++) printf("%s\n", devNames[i]);
    cl_device_id dev;
    GetDeviceFromIndex(0,&dev, &config->context, CL_DEVICE_TYPE_GPU);
#if 0
    cl_int errNum;
    cl_uint numPlatforms;
    cl_platform_id firstID;

    config->context = NULL;

    errNum = clGetPlatformIDs(1, &firstID, &numPlatforms);
    if (errNum != CL_SUCCESS) return errNum;

    cl_context_properties contextProperties[] = { CL_CONTEXT_PLATFORM,
        (cl_context_properties)firstID,
        0 };
    config->context = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_GPU, NULL, NULL, &errNum);

    return errNum;
#endif
    return 0;
}

int CreateStreamsQueue(HSA_OCL_ConfigS *config, cl_command_queue * Ques, int n_ques)
{
cl_int err = 0;
#if 0
    cl_command_queue single_q = clCreateCommandQueue(config->context, config->dev_id, NULL, NULL);

	Ques[0] = single_q; //clCreateCommandQueue(config->context, config->dev_id, NULL, NULL);
#else
	for ( int i = 0; i < n_ques; i++) {
		Ques[i] = clCreateCommandQueue(config->context, config->dev_id, NULL, NULL);
		if(Ques[i] == NULL) {
			return -99999999;
		}
  }
#endif
	return(err);
}

int CreateCommandQueue(HSA_OCL_ConfigS *config)
{
  cl_int errNum;
  cl_device_id *devices;
  size_t deviceBufferSize = -1;


  errNum = clGetContextInfo(config->context, CL_CONTEXT_DEVICES, 0, NULL, &deviceBufferSize);
  if(errNum != CL_SUCCESS) return errNum;

  if(deviceBufferSize <=0) return -99999999;

  devices = (cl_device_id*)malloc(deviceBufferSize);

  errNum = clGetContextInfo(config->context, CL_CONTEXT_DEVICES, deviceBufferSize, devices, NULL);

  if(errNum != CL_SUCCESS) return errNum;

  config->dev_id = devices[0];
		free(devices);
  errNum =  CreateStreamsQueue(config, config->OCLqueue, __NUM_OCL_QUEUES__);// Two queues one , general , one convolution

  if(errNum != CL_SUCCESS) {

		return errNum;
  }


  //GetInstanceInfo(config);

  return CL_SUCCESS;
}



static
int CreateProgram(HSA_OCL_ConfigS *config, const char *program_nm, int index, char * options)
{
  cl_int errNum;
  FILE *fp;
  long size;

  if ( config->program[index]
       && config->program_nm[index]
	   && !strcmp(config->program_nm[index], program_nm)
	   && ((options && !strcmp(config->options[index], options)) || (!options && !config->options[index])))
  {
	  return CL_SUCCESS;
  }

  if ( config->program[index] ) {
	  clReleaseProgram(config->program[index]);
	  config->program[index] = 0;
  }

  std::string srcStr;

  if ( config->program_nm[index] ) {
	  free(config->program_nm[index]);
	  config->program_nm[index] = 0;
  }
  if ( config->options[index] ) {
	  free(config->options[index]);
	  config->options[index] = 0;
  }
  if (strcmp("amdFHT_kernels", program_nm) == 0)
  {
      srcStr = (char *)amdFHT_kernels;
      config->program_src[index] = (char *)amdFHT_kernels;
      config->program_src_sz[index] = amdFHT_kernelsCount;
  }
  else if (strcmp("amdFHT2_kernels", program_nm) == 0)
  {
      srcStr = (char *)amdFHT2_kernels;
      config->program_src[index] = (char *)amdFHT2_kernels;
      config->program_src_sz[index] = amdFHT2_kernelsCount;
  }
  else if (strcmp("amdFHTbig_kernels", program_nm) == 0)
  {
      srcStr = (char *)amdFHTbig_kernels;
      config->program_src[index] = (char *)amdFHTbig_kernels;
      config->program_src_sz[index] = amdFHTbig_kernelsCount;
  }
  else if (strcmp("amdFHTmad_kernels", program_nm) == 0)
  {
      srcStr = (char *)amdFHTmad_kernels;
      config->program_src[index] = (char *)amdFHTmad_kernels;
      config->program_src_sz[index] = amdFHTmad_kernelsCount;
  }
  else if (strcmp("amdFIR_kernels", program_nm) == 0)
  {
      srcStr = (char *)amdFIR_kernels;
      config->program_src[index] = (char *)amdFIR_kernels;
      config->program_src_sz[index] = amdFIR_kernelsCount;
  }
  else if (strcmp("Util_kernels", program_nm) == 0)
  {
      srcStr = (char *)Util_kernels;
      config->program_src[index] = (char *)Util_kernels;
      config->program_src_sz[index] = Util_kernelsCount;
  }
  else
  {
      printf("unkown kernel\n");
      config->program_src[index] = NULL;
      config->program_src_sz[index] = 0;
      return -1;
  }

  /*
  if(!config->amf_compute_conv || !config->amf_compute_update)
  {
      // if AMF compute device is not provided compile programs here
      const char* c_string = srcStr.c_str();
      config->program[index] = clCreateProgramWithSource(config->context, 1, (const char**)(&c_string), NULL, NULL);

      if (config->program[index] == NULL)
      {
          printf("Failed to create CL program from source\n");
          return -99999999;
      }

      char *p_build_options = options;
#if 0 //def VERIFICATION_BUFFER
      p_build_options = "-DVERIFICATION_BUFFER";
#endif
#if 0
      printf("Compiling the OCL program %s\n", program_nm, options);

      if (options)
      {
          printf("with options: %s\n", options);
      }
#endif

      errNum = clBuildProgram(config->program[index], 0, NULL, p_build_options, NULL, NULL);
      if (errNum != CL_SUCCESS)
      {
          char buildLog[16384];
          clGetProgramBuildInfo(config->program[index], config->dev_id, CL_PROGRAM_BUILD_LOG, sizeof(buildLog),
                                buildLog, NULL);
          printf("Build Log\n program: %s\n %s\n\n", program_nm, buildLog);
          clReleaseProgram(config->program[index]);
          return errNum;
      }
  }
  */

  int len = (int)strlen(program_nm);
  config->program_nm[index] = (char*)malloc(len+1);
  if ( config->program_nm[index] ) {
       strcpy(config->program_nm[index], program_nm);
  }
  else {
      printf("Failed to create CL program from source\n");
      return -99999999;
  }
  config->options[index] = NULL;
  if (options) {
	len = (int)strlen(options);
	config->options[index] = (char*)malloc(len+1);
	if ( config->options[index] ) {
		   strcpy(config->options[index], options);
	}
	else {
		  printf("Failed to create CL program from source\n");
		return -99999999;
	}
  }


  return CL_SUCCESS;
}

int CreateProgram2(ProjPlan *plan, const char *program_nm, int index, char * options)
{
  cl_int ret = CreateProgram(GetOclConfig(plan), program_nm, index, options);
  return ret;
}

cl_kernel CreateOCLKernel2(ProjPlan *plan, const char * kernel_nm, int prog_index, ComputeDeviceIdx compDevIdx)
{
    auto *config(GetOclConfig(plan));

    amf::AMFComputePtr compute(
        compDevIdx == ComputeDeviceIdx::COMPUTE_DEVICE_CONV
            ? config->amf_compute_conv
            : config->amf_compute_update
        );
    cl_kernel resultKernel = nullptr;

    auto result = GetOclKernel(
      resultKernel,
      nullptr,
      config->OCLqueue[0] ? config->OCLqueue[0] : config->OCLqueue[1],

      config->program_nm[prog_index],
      config->program_src[prog_index],
      config->program_src_sz[prog_index],
      kernel_nm,

      config->options[prog_index]
      );

    if(result)
    {
        return resultKernel;
    }

    printf("\nERROR: Cannot get the kernel %s from the AMF program %s\n", kernel_nm, config->program_nm[prog_index]);

    return nullptr;

    /*
    cl_kernel ret = 0;
	int err = 0;
    if (!GetOclConfig(plan)->amf_compute_conv  || !GetOclConfig(plan)->amf_compute_update)
    {
        // If AMF compute device is not provided by the user just use the raw OCL API to compile and create the kernel objects
        ret = clCreateKernel(GetOclConfig(plan)->program[prog_index], kernel_nm, &err);
        return(ret);
    }
    else
    {
        char* program_nm = GetOclConfig(plan)->program_nm[prog_index];
        char* program_src = GetOclConfig(plan)->program_src[prog_index];
        char* program_options = GetOclConfig(plan)->options[prog_index];
        int program_src_sz = GetOclConfig(plan)->program_src_sz[prog_index];

        amf::AMF_KERNEL_ID amfKernelId = -1;

        int name_len = (int)strlen(program_nm);
        std::wstring wKernelId(program_nm, program_nm +name_len);

        amf::AMFPrograms* pPrograms = nullptr;
        g_AMFFactory.GetFactory()->GetPrograms(&pPrograms);

        if (nullptr != pPrograms)
        {
            if (AMF_OK == pPrograms->RegisterKernelSource(&amfKernelId,
                                                          wKernelId.c_str(),
                                                          kernel_nm,
                                                          program_src_sz,
                                                          reinterpret_cast<const amf_uint8*>(program_src),
                                                          program_options))
            {
                amf::AMFComputeKernel* pAMFComputeKernel = nullptr;

                switch (compDevIdx)
                {
                    case ComputeDeviceIdx::COMPUTE_DEVICE_CONV:
                    {
                        GetOclConfig(plan)->amf_compute_conv->GetKernel(amfKernelId, &pAMFComputeKernel);
                    }
                    break;
                    case ComputeDeviceIdx::COMPUTE_DEVICE_UPDATE:
                    {
                        GetOclConfig(plan)->amf_compute_update->GetKernel(amfKernelId, &pAMFComputeKernel);
                    }
                    break;
                }

                if (nullptr != pAMFComputeKernel)
                {
                    return (cl_kernel)pAMFComputeKernel->GetNative();
                }
                else
                {
                    printf("ERROR: Cannot get the kernel from the AMF program object: %s\n", kernel_nm);
                    return 0;
                }

            }
        }
    }
    */
}

void GetInstanceInfo(HSA_OCL_ConfigS *config)
{
  int err;
  config->gpu_max_mem_alloc_sz = 0;
  config->gpu_max_memory_sz = 0;
  char dev_nm[128];
  err = clGetDeviceInfo(config->dev_id, CL_DEVICE_MAX_MEM_ALLOC_SIZE,
			sizeof(config->gpu_max_mem_alloc_sz), &config->gpu_max_mem_alloc_sz, NULL);

  err |= clGetDeviceInfo(config->dev_id, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(config->gpu_max_memory_sz), &config->gpu_max_memory_sz, NULL);
  err |= clGetDeviceInfo(config->dev_id, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(config->gpu_max_cus), &config->gpu_max_cus, NULL);
  err |= clGetDeviceInfo(config->dev_id, CL_DEVICE_NAME, 127, dev_nm, NULL);
  // TODO: PARAMETERIZE
  config->gpu_wavefront_ln_log2 = 6;
  if(err == CL_SUCCESS)
   printf("\nAMD GPU: %s CUs: %d, max memory, max alloc size (MBytes): %d, %d\n", dev_nm, config->gpu_max_cus, (int)(config->gpu_max_memory_sz/(1024*1024)), (int)(config->gpu_max_mem_alloc_sz/(1024*1024)));


 return;
}


int GetWavefrontLg2(HSA_OCL_ConfigS *config) {
	int ret = 0;
	if ( config ) {
		ret = config->gpu_wavefront_ln_log2;
	}
	return ret;
}

cl_context GetContext(HSA_OCL_ConfigS *config) {
cl_context ret = 0;
   if ( config ) {
	   ret = config->context;
   }
   return ret;
}


cl_command_queue GetGlobalQueue(amdAudShcheduler * sched, HSA_OCL_ConfigS *config) {
cl_command_queue ret = 0;
	if ( config ) {

		ret = config->OCLqueue[ 0];
	}
	return ret;
}


int CreateOCLBuffer(cl_context context,OCLBuffer * buf, uint buffer_flags)
{
  int err;
  int BUFFLAGS = buffer_flags;

   if (!buf || buf->len == 0 )
   {
	    err = -1;
        printf("error buf params\n");
        return err;
   }

   buf->sys = malloc(buf->len);

   if ( !buf->sys )
   {
	    err = -1;
        printf("error buf allocation\n");
        return err;
   }

   buf->mem = clCreateBuffer(context, BUFFLAGS, buf->len, NULL, &err);
   if(err != CL_SUCCESS)
   {
      printf("error creating bufffer: %d\n", err);
      return err;
   }

 	buf->context = context;

   return err;
}

// without symmetrical system buffer
int CreateOCLBuffer2(cl_context context,OCLBuffer * buf, uint buffer_flags)
{
  int err;
  int BUFFLAGS = buffer_flags;

   if (!buf || buf->len == 0 )
   {
	    err = -1;
        printf("error buf params\n");
        return err;
   }


   buf->mem = clCreateBuffer(context, BUFFLAGS, buf->len, NULL, &err);
   if(err != CL_SUCCESS)
   {
      printf("error creating bufffer: %d\n", err);
      return err;
   }
   buf->sys_map = 0;
   buf->sys = 0;
   buf->flags = BUFFLAGS;

	buf->context = context;
   return err;
}

void SetMemOCLBuffer(OCLBuffer * buf, cl_mem memObj, size_t len)
{
	ReleaseOCLBuffer(buf);
	buf->mem = memObj;
	buf->len = len;
// external mem obj
	buf->flags = 1;
}

int  MapOCLBuffer(OCLBuffer * buf, cl_command_queue commandQueue, uint map_flags) {
int err = 0;
	if ( buf && !buf->sys_map && buf->mem ) {
		buf->commandQueue = commandQueue;
		buf->sys_map = (__FLOAT__*)clEnqueueMapBuffer (commandQueue,
 													buf->mem,
													CL_TRUE,
													map_flags, //CL_MAP_WRITE_INVALIDATE_REGION,
 													0,
													buf->len,
													0,
													NULL,
													NULL,
													&err);
	}
	return(err);
}

int UnmapOCLBuffer(OCLBuffer * buf) {
int err = 0;
	if ( buf && buf->sys_map && buf->mem && buf->commandQueue) {
		clEnqueueUnmapMemObject(buf->commandQueue,
								buf->mem,
								buf->sys_map,
								0,
								NULL,
								NULL);
	}
	return(err);
}

bool IsMapped(OCLBuffer * buf) {
	return( buf->sys_map != NULL);
}


int ReleaseOCLBuffer(OCLBuffer * buf)
{
	int err = CL_SUCCESS;
	if ( buf->sys ) {
		free(buf->sys);
	}

	err |= UnmapOCLBuffer(buf);

	if ( buf->mem) {
		err = clReleaseMemObject(buf->mem);
	}
	memset(buf, 0, sizeof(OCLBuffer));

	return err;
}

int SetValue(cl_command_queue queue, OCLBuffer * buf, unsigned int value, size_t offset, size_t size)
{
    int err;
    err = clEnqueueFillBuffer(queue, buf->mem, &value, sizeof(value), offset, size, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("1- error setting value: %d\n", err);
    }
    return(err);
}


int SetValue(cl_command_queue queue, OCLBuffer * buf, cl_float value)
{
    int err;
    err = clEnqueueFillBuffer(queue, buf->mem, &value, sizeof(value), 0, buf->len, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("2- error setting value: %d\n", err);
    }
    return(err);
}

int CopyToDevice(cl_command_queue commandQueue, OCLBuffer * buf, bool force)
{
int err;

	if(buf->len == 0 || !buf->sys || !buf->mem) {
		printf("wrong data\n");
		return(-1);
	}

	err = clEnqueueWriteBuffer(commandQueue, buf->mem, force,0, buf->len, buf->sys, 0, NULL, NULL);

	if(err != CL_SUCCESS) {
		printf("error writing data to device: %d\n", err);
		return err;
	}

	return(err);
}
// sys buffer should have exact buf->len lenght
int CopyToDevice2(cl_command_queue commandQueue, OCLBuffer * buf, void * sys, size_t sys_len, size_t offset, bool force )
{
int err;

	if(buf->len == 0 || !sys || !buf->mem) {
		printf("wrong data\n");
		return(-1);
	}

	err = clEnqueueWriteBuffer(commandQueue, buf->mem, force, offset, sys_len, sys, 0, NULL, NULL);

	if(err != CL_SUCCESS) {
		printf("error writing data to device: %d\n", err);
		return err;
	}

	return(err);
}

int CopyFromDevice(cl_command_queue commandQueue, OCLBuffer * buf)
{
  int err;

	if(buf->len == 0 || (buf->sys && buf->flags) || !buf->mem) {
		printf("wrong data\n");
		return(-1);
    }

	if ( !buf->sys ) {
		buf->sys = malloc(buf->len);

		if ( !buf->sys ) {
			err = -1;
			printf("error buf allocation\n");
			return err;
		}
	}

	err = clEnqueueReadBuffer(commandQueue, buf->mem, CL_TRUE,0, buf->len, buf->sys, 0, NULL, NULL);


	if(err != CL_SUCCESS) {
		printf("error writing data to device: %d\n", err);
		return err;
    }

	return(err);
}


