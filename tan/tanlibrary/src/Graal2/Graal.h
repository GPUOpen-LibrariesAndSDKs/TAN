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

#ifndef __AMDGRAAL_OCL__
#define __AMDGRAAL_OCL__

#include <CL/cl.h>
#include "public/common/AMFFactory.h"

#define __FLOAT__ float
typedef unsigned int uint;



typedef void * graalHandle;
typedef graalHandle amdOCLRvrb;
typedef graalHandle firHandle;

#define __INIT_FLAG_VERIFY__			1
#define __INIT_FLAG_GLOBAL_QUEUES__		2
#define __INIT_FLAG_MULTITHREAD__		4
#define __INIT_FLAG_HETEROGEN__			8
#define __INIT_FLAG_SHARED_PIPELINE__   16
#define __INIT_FLAG_HETEROGEN_FFT__     32
#define __INIT_FLAG_FFT__				__INIT_FLAG_HETEROGEN_FFT__
#define __INIT_FLAG_FHT__				64
#define __INIT_FLAG_2FFT_STREAMS__      128
#define __INIT_FLAG_2STREAMS__			__INIT_FLAG_2FFT_STREAMS__
#define __INIT_FLAG_VER_TRANSFORM1__	512
#define __INIT_FLAG_VER_TRANSFORM2__	1024
#define __INIT_FLAG_FHT1_ONCPU__		2048
#define __INIT_FLAG_FIR__				4096

#define __PROCESSING_FLAG_GPU__					0
#define __PROCESSING_FLAG_CPU__					1
#define __PROCESSING_FLAG_VERIFY_LOAD__          2
#define __PROCESSING_FLAG_VERIFY_TRANSFORM1__    4
#define __PROCESSING_FLAG_VERIFY_TRANSFORM2__    8


/**
***************************************************************************************************
*   graalInitialize
*
*   @brief
*		Initializes reverberation pipeline.
*		Each new reverberation or FIR filter has to have its own initalization.
**
*   @return
*       If successful then 0 is returned.  Any other value is an error condition.
*
***************************************************************************************************
*/

int graalInitialize(
	graalHandle * handle,				///< [out] handle to the pipeline.
	const char * ocl_kernels_path,		///< [in]  path to the set of OCL kernels. Default - working directory.
	int init_flags,						///< [in] initialization flags (see below).
    const amf::AMFComputePtr & amf_compute_conv,       ///< [in] the AMF compute device used for passing the convolution queue and compiling and caching the kernels, if not provided will use ocl API to compile
    const amf::AMFComputePtr & amf_compute_update      ///< [in] the AMF compute device used for passing the general queue and compiling and caching the kernels, if not provided will use ocl API to compile
	);

int graalInitialize(
	graalHandle * handle,				///< [out] handle to the pipeline.
	const char * ocl_kernels_path,		///< [in]  path to the set of OCL kernels. Default - working directory.
	int init_flags,						///< [in] initialization flags (see below).
	cl_command_queue OCLqueue_conv = 0,       ///< [in] the OpenCL queue used for passing the convolution queue and compiling and caching the kernels, if not provided will use ocl API to compile
	cl_command_queue OCLqueue_update = 0      ///< [in] the OpenCL queue device used for passing the general queue and compiling and caching the kernels, if not provided will use ocl API to compile
	);

/**
**************************************************************************************************
Possible initialization flag combinations:
__INIT_FLAG_FHT__  - uniform convolution.|
__INIT_FLAG_FHT__ | __INIT_FLAG_HETEROGEN__ - uniform heterogeneous convolution.
__INIT_FLAG_FHT__ | __INIT_FLAG_2STREAMS__  - non-uniform convolution.
__INIT_FLAG_FHT__ | __INIT_FLAG_2STREAMS__ | __INIT_FLAG_HETEROGEN__ - non-uniform heterogeneous convolution.
__INIT_FLAG_FIR__ - FIR pipeline.
*************************************************************************************************
*/


/**
***************************************************************************************************
*   graalTerminate
*
*   @brief
*		Terminate the reverberation pipeline.
*		Each reverberation or FIR filter has to be terminated.
**
*   @return
*       If successful then 0 is returned.  Any other value is an error condition.
*
***************************************************************************************************
*/
int graalTerminate(
	graalHandle handle			///< [in] handle to the pipeline.
	);


/**
***************************************************************************************************
*   graalReverbSetBlockSize
*
*   @brief
*		Set an input block size - number of samples that an application is going to pass to the reverberation pipeline in a single round.
*		The same block size - number of samples - an appplication is expected to get back after convolution.
**
*   @return
*       If successful then 0 is returned.  Any other value is an error condition.
*
***************************************************************************************************
*/
int graalReverbSetBlockSize(
	graalHandle handle,			///< [in] handle to the pipeline.
	int block_size				///< [in] block size in samples.
	);

/**
***************************************************************************************************
*   graalReverbSetNChannels
*
*   @brief
*		Set a combination of input/output channels and subchannels.
*       Supported combination:
*       1,2,1,2
**
*   @return
*       If successful then 0 is returned.  Any other value is an error condition.
*
***************************************************************************************************
*/
int graalReverbSetNChannels(
	graalHandle handle,			///< [in] handle to the pipeline.
	int n_input_channels,		///< [in] # of input streams.
	int n_input_subchannels,	///< [in] # of input channels.
	int n_output_channels,		///< [in] # of output streams.
	int n_output_subchannels	///< [in] # of output channels.
	);


int graalReverbGetBlockSize(graalHandle handle);
int graalReverbGetNChannels(graalHandle handle);

/**
***************************************************************************************************
*   graalReverbSetupReverbKernelFromFile
*
*   @brief
*		Loads a reveberation kernel from an audio file.
*       Supported file format:
*			.wav, only stereo, 16 bit format.
*		What does it do:
*			reads;
*			unpacks;
*			converts to float;
*			loads 1 filter block into the CPU part of a pipeline;
*			does direct FHT transform, bin size = blocksize * 2, second half is padded with 0s;
*			loads the filter into the 1st GPU part of the pipeline;
*			does direct FHT transform, bin size = blocksize * 2, the same padding;
*			loads the filter into the 2nd GPU part of the pipeline, bin size = blocksize * multiplier * 2
**
*   @return
*       If successful then 0 is returned.  Any other value is an error condition.
*
***************************************************************************************************
*/

int graalReverbSetupKernelFromContiguousOCLBuffer(
    amdOCLRvrb rvrb,
    int _n_channels,
    int* _channel_ids,
    unsigned int _ir_version,
    const cl_mem  _conv_ptrs,  // arbitrary cl_mem ptrs
    const int  _max_conv_lens
);

int graalReverbSetupReverbKernelFromOCLBuffer(
    amdOCLRvrb rvrb,
    int _n_channels,
    unsigned int _ir_version,
    const cl_mem * _conv_ptrs,  // arbitrary cl_mem ptrs
    const int*  _conv_lens
    );

int graalReverbSetupReverbKernelFromHostBuffers(
    amdOCLRvrb rvrb,
    int _n_channels,
    int* _channel_ids,
    unsigned int _ir_version,
    float ** _conv_ptrs,  // arbitrary cl_mem ptrs
    const int*  _conv_lens
);
///////////////////////////////////////////////////////////////////////
//  Receives information about the IRs and setsup the OCL
//////////////////////////////////////////////////////////////////////////

int graalInit(
    amdOCLRvrb rvrb,
    int kernelSize,
    int numChannels,
    unsigned int n_ir_buffers
);

int graalInit(
    amdOCLRvrb rvrb,
    const char * file_loc,
    unsigned int n_ir_buffers
);

int graalFlush(
    graalHandle rvrb,
    int channelId
);

/**
***************************************************************************************************
*
*	graalReverbIsActive
*
*   @brief
*		Indicates that all activities related to the reverberation filrer loading are complete.
**
*   @return
*       If successful then 1 is returned, otherwise 0.
*
***************************************************************************************************
*/
int graalReverbIsActive(
	graalHandle handle	///< [in] handle to the pipeline.
	);


/**
***************************************************************************************************
*
*	graalReverbProcessing
*
*   @brief
*		Processes an audio stream block (with sub-channels) and returns the convolved data to the caller.
*		this is the library's main entry point.
*		calls different pipeline inplemetetion depending on the instantiation flags (see).
*
* algorithm:
* the kernel has been presetup (see ReverbOCLSetupReverbKernelFromFile).
*
* on each newly block arrival (new round)
* the sample block is appended with the previous block (on the right);
* direct FHT is performed.
*
* uniform convolution:
* the resulting block is moved into the FHHTStore cyclic array buffer according to the block's current counter modulo the number of kernel bins.
* MAD is performed between kernel bins and FHTStore bins.
* the 0 kernel bin MADs with the newly arrived bin,
* the 1s kerenel bin MADs with a (newly arrived - 1) FHTStrore bin,
* the 2nd kerenel bin MADs with a (newly arrived - 2) FHTStrore bin,
* etc.
* All resulting bins are summed up.
* the sum undergoes the inverse FHT and is returned to the caller.
*
* non-uniform convolution:
* multiplier is a non-uniformity ration: ratio between the original block size and the stream 2 large block size.
* stream 1 - does uniform convolution with firts muliplier*2 number of bins;
* stream 2 - gathers a multiplier number of input blocks then:
*            does the forward transform over a large block;
*            every round does the MAD with only the (filter length in blocks / multiplier) number of large blocks, staring frpm 2nd large block;
*            does the inverse transform over resulting sum of the previous stream 2 large block.
*            sum up a subblock of a large block at index round % multiplier with the stream 1 sum.
**
*   @return
*       If successful then 1 is returned. Any other value is an error condition.
*
***************************************************************************************************
*/

int graalReverbProcessing(
	graalHandle handle,		///< [in] handle to the pipeline.
	__FLOAT__ ** input,		///< [in] ptrs on input blocks, each block represents a subchannel.
	__FLOAT__ ** output,	///< [out] ptrs on output blocks, each block represents a subchannel.
	cl_mem* cl_output,      ///< [out] gpu ptrs on output blocks, each block represents a subchannel.
	int numSamples,			///< [in] number of samples in the current round, cannnot be larger than block size.
	int numChans,
	int* channel_ids,
	unsigned int _ir_version,/// [in] IR buffer used for convolution
	int flags,				///< [in] run-time flags.
	int xfade_state = 0           ///< [in] crossfade state.
	);

int graalIRUploadDoProc(amdOCLRvrb rvrb, int numChans, int* _channel_ids, unsigned int _ir_version);

int graalCopyPrevIRsProc(amdOCLRvrb rvrb, int numChans, unsigned int _from_ir_version, unsigned int _to_ir_version, int* _channel_ids);
/**
***************************************************************************************************
*   graalFIRUpload
*
*   @brief
*		Uploads a FIR filter in the plane format with samples of the __FLOAT__ type.
*		Layout:
*		fir_sz samples of channel 0, fir_sz samples of channels 1, etc.
*       # number of channels is defined in graalReverbSetNChannels (see).
**
*   @return
*       If successful then 0 is returned.  Any other value is an error condition.
*
***************************************************************************************************
*/
int graalFIRUpload(
	graalHandle handle,		///< [in] handle to the pipeline.
	__FLOAT__ * firPtrs,	///< [in] ptr on the filter in plane format.
	int fir_sz				///< [in] length of the filter (onle plane).
	);


#endif