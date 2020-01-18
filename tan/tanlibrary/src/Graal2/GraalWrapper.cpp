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

#include "GraalWrapper.h"
#include "IIRLowLat.h"
#include "IIRLowLat_OCL.h"
#include "Reverberate_OCL.h"
#include "amdFHT.h"
#include <omp.h>

void GraalWrapper::terminate() 
{
	if (m_handle == 0) return;
	graalTerminate(m_handle);
	m_handle = 0;
}
bool GraalWrapper::ready4IrUpdate()
{
    if (m_handle == 0)
        return true;
    ProjPlan* plan = ReverbFromHandle(m_handle);
    return (plan->xfade_state == 0);
}

GraalWrapper::~GraalWrapper()
{
	terminate();
}

int
GraalWrapper::flush(int channelId)
{
    return graalFlush(m_handle, channelId);
}

int GraalWrapper::Init(
    amf::TANContextPtr& pContextTAN,
    amf::AMFComputePtr& pConvolution,
    amf::AMFComputePtr& pUpdate,
    int n_max_channels,
    int max_conv_kernel_size,
    int buffer_size,
    int number_ir_buffer_sets)
{
//#ifndef _WIN32
//	return -3;
//#endif
    m_amfComputeConv = pConvolution;
    m_amfComputeUpdate = pUpdate;
    m_pContextTAN = pContextTAN;
    //// Sanity check to make sure context and queues assciated with A<F compute devices and TAN context are consistent
    //if (m_amfComputeConv->GetNativeCommandQueue() != pContextTAN->GetOpenCLConvQueue() ||
    //    m_amfComputeUpdate->GetNativeCommandQueue() != pContextTAN->GetOpenCLGeneralQueue())
    //{
    //    printf("Command queues associated with AMF compute devices different from the ones in the TAN context\n");
    //    return -1;
    //}
    //if (m_amfComputeConv->GetNativeContext() != pContextTAN->GetOpenCLContext() ||
    //    m_amfComputeUpdate->GetNativeContext() != pContextTAN->GetOpenCLContext())
    //{
    //    printf("Context associated with AMF compute devices different from the ones in the TAN context\n");
    //    return -2;
    //}
    // Setting the falgs 
    int init_flags = (  heterogen | FHT_2streams | fht | fft | fir);
    int ret =  ReverbDriverInit(&m_handle, buffer_size, n_max_channels, 1, init_flags);
    ret |= graalInit(m_handle, max_conv_kernel_size, n_max_channels, number_ir_buffer_sets);
    m_oclIrBuff.init(m_handle, init_flags, pContextTAN->GetOpenCLContext(), n_max_channels, max_conv_kernel_size);
    return ret;
}

int GraalWrapper::UploadOCLIRs(
    int _n_channels,
    const cl_mem * _ir_buf_ptrs,  // arbitrary cl_mem ptrs
    const int*  _conv_lens,
    int* _channel_ids,
    unsigned int _ir_version
)
{
    int err = 0;
    if (fht > 0)
    {
        if (m_oclIrBuff.copy(_n_channels, _ir_buf_ptrs, _conv_lens, _channel_ids) != 0)
        {
            printf("Error copying the IR buffers %s %d\n",__FILE__,__LINE__);
        }
        m_oclIrBuff.sync();
    }
    else
    {
        return -1; // not implemented
    }
    return err;
}

int GraalWrapper::UploadHostIRs(
    int _n_channels,
    float** _ir_buf_ptrs,  // arbitrary cl_mem ptrs
    const int*  _conv_lens,
    int* _channel_ids,
    unsigned int _ir_version
)
{
    //if (!kern_fl) return -1;
    int err = 0;
    if (fht > 0)
    {

        err = graalReverbSetupReverbKernelFromHostBuffers(m_handle, _n_channels, _channel_ids, _ir_version, _ir_buf_ptrs, _conv_lens);
    }
    else
    {
        return -1; // not implemented
    }
    return err;
}


int GraalWrapper::process( __FLOAT__ ** input, __FLOAT__ ** output, cl_mem* cl_output, int numSamples, int numChans, int* channel_ids, unsigned int _ir_version, int xfade_state)
{
    int ret = 0;

    ProjPlan* tplan = ReverbFromHandle(m_handle);
    if (graalReverbIsActive(tplan))
    {
        ret = graalReverbProcessing(ReverbFromPtr(tplan), input, output, cl_output,numSamples, numChans, channel_ids, _ir_version, 0, xfade_state);
    }
    return 0;
}


int GraalWrapper::ReverbDriverInit(graalHandle* new_plan, int block_size, int n_channels, int subchannels, int verification)
{
    int err = 0;
    amdOCLRvrb plan;

    //err = graalInitialize(&plan, NULL, verification, m_amfComputeConv, m_amfComputeUpdate);
	
	err = graalInitialize(&plan, NULL, verification, m_pContextTAN->GetOpenCLConvQueue(), m_pContextTAN->GetOpenCLGeneralQueue());

    // actually should reposrt the number
    err = graalReverbSetNChannels(plan, n_channels, subchannels, n_channels, subchannels);

    err = graalReverbSetBlockSize(plan, 1024);
    err = graalReverbSetBlockSize(plan, block_size);

    *new_plan = plan;

    return(err);
}
int
GraalWrapper::updateConv(int numChans, unsigned int _ir_version, int* _channel_ids)
{
    int err = 0;
    if (m_oclIrBuff.getNumChannels() != 0) 
    {
        // If the OCL buffers are used by the user to send in the IRs, otherwise graalReverbSetupReverbKernelFromHostBuffers() is already called for host buffers
        err |= graalReverbSetupKernelFromContiguousOCLBuffer(m_handle, numChans, _channel_ids, _ir_version, m_oclIrBuff.getUnifiedClMemBuff(), m_oclIrBuff.getChannelSampleSize());
    }
    err |= graalIRUploadDoProc(m_handle, numChans, _channel_ids, _ir_version);
    return err;
}

int
GraalWrapper::copyResponses(int numChans, unsigned int _from_ir_version, unsigned int _to_ir_version, int* _channel_ids)
{
    int err = graalCopyPrevIRsProc(m_handle, numChans, _from_ir_version, _to_ir_version, _channel_ids);
    ProjPlan * plan = ReverbFromHandle(m_handle);
    cl_command_queue queue = plan->OCLqueue[1];// General queue
    clFinish(queue);
    return err;
}
int 
GraalWrapper::OCLIRBuff::init(graalHandle handle, int init_flags, cl_context  context, int numChannels, int kernelSize)
{
    // create and initialize the OCL buffers. The buffers are created with a length potentially longer than user IR's to cover an even number of the longer partition size. 
    ProjPlan * plan = ReverbFromHandle(handle);
    queue = plan->OCLqueue[1];// General queue
    max_num_channels = numChannels;
    int frame_ln = (init_flags & __INIT_FLAG_2STREAMS__) ? plan->frame2_ln : plan->frame_ln;
    singleBufferSize = frame_ln * plan->conv_blocks * sizeof(cl_float);
    conv_ptrs = new cl_mem[max_num_channels];
    conv_lens = new int[max_num_channels];
    max_kernel_length = kernelSize;
    unifiedBufferSize = singleBufferSize * max_num_channels;
    int clErr = CL_SUCCESS;
    unified_conv_ptr = clCreateBuffer(context, CL_MEM_READ_WRITE, unifiedBufferSize, NULL, &clErr);
    clearBuffer();
    for (int i = 0; i < max_num_channels; i++)
    {
        cl_buffer_region region;
        region.origin = i*singleBufferSize;
        region.size = singleBufferSize;
        conv_ptrs[i] = clCreateSubBuffer(
            unified_conv_ptr, CL_MEM_READ_WRITE, CL_BUFFER_CREATE_TYPE_REGION, &region, &clErr);
    }
    n_active_channels = 0;
    clFinish(queue);
	m_initialized = true;
    return clErr;
}
int  
GraalWrapper::OCLIRBuff::copy(int _n_channels,
          const cl_mem * _conv_ptrs,  // arbitrary cl_mem ptrs
          const int*  _conv_lens, 
          int* channel_ids)
{
    int err = CL_SUCCESS;
    //err |= clearBuffer();
    for (int i = 0; i < _n_channels; i++)
    {
        int chId = channel_ids[i];
        int len = (max_kernel_length < _conv_lens[i]) ? max_kernel_length : _conv_lens[i];
        err |= clEnqueueCopyBuffer(queue, _conv_ptrs[i], conv_ptrs[chId], 0, 0 , len * sizeof(cl_float),0, NULL, NULL );
        int zeroPadLen = max_kernel_length - _conv_lens[i];
        if (zeroPadLen > 0)
        {
            cl_float zero = 0;
            err|= clEnqueueFillBuffer(queue, conv_ptrs[chId], &zero, sizeof(cl_float), len * sizeof(cl_float), zeroPadLen * sizeof(cl_float), 0, NULL, NULL);
        }
        conv_lens[chId] = len;
    }
    n_active_channels = _n_channels;
    return err;
}

GraalWrapper::OCLIRBuff::~OCLIRBuff()
{
	if (!m_initialized)
		return;
    for (int i= 0; i < max_num_channels; i++)
    {
        clReleaseMemObject(conv_ptrs[i]);
    }
    clReleaseMemObject(unified_conv_ptr);
    delete[] conv_ptrs;
    delete[] conv_lens;
    max_num_channels = 0;
    n_active_channels = 0;
}

int
GraalWrapper::OCLIRBuff::clearBuffer()
{
    cl_float zero = 0;
    int err = clEnqueueFillBuffer(queue, unified_conv_ptr, &zero, sizeof(cl_float), 0, unifiedBufferSize, 0, NULL, NULL);
    return err;
}