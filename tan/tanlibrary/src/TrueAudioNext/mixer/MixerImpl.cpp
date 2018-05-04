//
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
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
#include "MixerImpl.h"
#include "../core/TANContextImpl.h"
#include "public/common/AMFFactory.h"
#include "../../common/OCLHelper.h"
#include "../../common/cpucaps.h"

#include <math.h>

#include "CLKernel_Mixer.h"
#define AMF_FACILITY L"TANMixerImpl"


using namespace amf;

bool TANMixerImpl::useSSE2 = InstructionSet::SSE2();

static const AMFEnumDescriptionEntry AMF_MEMORY_ENUM_DESCRIPTION[] = 
{
#if AMF_BUILD_OPENCL
    {AMF_MEMORY_OPENCL,     L"OpenCL"},
#endif
    {AMF_MEMORY_HOST,       L"CPU"},
    {AMF_MEMORY_UNKNOWN,    0}  // This is end of description mark
};
//-------------------------------------------------------------------------------------------------
TAN_SDK_LINK AMF_RESULT AMF_CDECL_CALL TANCreateMixer(
    amf::TANContext* pContext, 
    amf::TANMixer** ppComponent
    )
{
    TANContextImplPtr contextImpl(pContext);
    *ppComponent = new TANMixerImpl(pContext, NULL );
    (*ppComponent)->Acquire();
    return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
TANMixerImpl::TANMixerImpl(TANContext *pContextTAN, AMFContext* pContextAMF) :
    m_pContextTAN(pContextTAN),
    m_pContextAMF(pContextAMF),
    m_pCommandQueueCl(nullptr),
    m_eOutputMemoryType(AMF_MEMORY_HOST)
{
    AMFPrimitivePropertyInfoMapBegin
        AMFPropertyInfoEnum(TAN_OUTPUT_MEMORY_TYPE ,  L"Output Memory Type", AMF_MEMORY_HOST, AMF_MEMORY_ENUM_DESCRIPTION, false),
    AMFPrimitivePropertyInfoMapEnd
}
//-------------------------------------------------------------------------------------------------
TANMixerImpl::~TANMixerImpl(void)
{
    Terminate();
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL TANMixerImpl::Init(
    amf_size buffer_size,
    int num_channels
	) 

{
    AMFLock lock(&m_sect);
    m_bufferSize = buffer_size;
    m_numChannels = num_channels;
    AMF_RETURN_IF_FALSE(!m_pDeviceAMF, AMF_ALREADY_INITIALIZED, L"Already initialized");
    AMF_RETURN_IF_FALSE(!m_pCommandQueueCl, AMF_ALREADY_INITIALIZED, L"Already initialized");
    AMF_RETURN_IF_FALSE((NULL != m_pContextTAN), AMF_WRONG_STATE,
    L"Cannot initialize after termination");

    // Determine how to initialize based on context, CPU for CPU and GPU for GPU
    if (m_pContextTAN->GetOpenCLContext())
    {
        return InitGpu();
    }
    else
    {
        return InitCpu();
    }
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL TANMixerImpl::InitCpu()
{
    // No device setup needs to occur here; we're done!
    return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL TANMixerImpl::InitGpu()
{
    cl_int ret;
    AMF_RESULT res = AMF_OK;

    /* OpenCL Initialization */

    // Given some command queue, retrieve the cl_context...
    m_pCommandQueueCl = m_pContextTAN->GetOpenCLGeneralQueue();
    ret = clGetCommandQueueInfo(m_pCommandQueueCl, CL_QUEUE_CONTEXT, sizeof(cl_context), 
        &m_pContextCl, NULL);
        AMF_RETURN_IF_CL_FAILED(ret,  L"Cannot retrieve cl_context from cl_command_queue.");

    // ...and cl_device from it
    ret = clGetCommandQueueInfo(m_pCommandQueueCl, CL_QUEUE_DEVICE, sizeof(cl_device_id), 
        &m_pDeviceCl, NULL);
    AMF_RETURN_IF_CL_FAILED(ret, L"Cannot retrieve cl_device_id from cl_command_queue.");

    // Retain the queue for use
    ret = clRetainCommandQueue(m_pCommandQueueCl);
    AMF_RETURN_IF_CL_FAILED(ret, L"Failed to retain command queue.");

    amf_int64 tmp = 0;
    GetProperty(TAN_OUTPUT_MEMORY_TYPE, &tmp);
    m_eOutputMemoryType = (AMF_MEMORY_TYPE)tmp;
    TANContextImplPtr contextImpl(m_pContextTAN);
    m_pDeviceAMF = contextImpl->GetGeneralCompute();

    m_internalBuff = clCreateBuffer(m_pContextTAN->GetOpenCLContext(), CL_MEM_READ_WRITE, m_bufferSize * m_numChannels * sizeof(float), nullptr, &ret);
    AMF_RETURN_IF_CL_FAILED(ret, L"Failed to create CL buffer");
    //... Preparing OCL Kernel
    bool OCLKenel_Err = false;
    OCLKenel_Err = GetOclKernel(m_clMix, m_pDeviceAMF, contextImpl->GetOpenCLGeneralQueue(), "Mixer", Mixer, MixerCount, "Mixer", "");
    if (!OCLKenel_Err){ printf("Failed to compile Mixer Kernel"); return AMF_FAIL; }
	m_OCLInitialized = true;
    return res;
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL TANMixerImpl::Terminate()
{
    AMFLock lock(&m_sect);

    m_pDeviceAMF = NULL;
    if (m_pContextTAN->GetOpenCLContext() != nullptr)
    {
        AMF_RETURN_IF_CL_FAILED(clReleaseKernel(m_clMix), L"Failed to release cl kernel");

        m_clMix = nullptr;

    }
    m_pDeviceCl = NULL;
    if (m_pCommandQueueCl)
    {
        cl_int ret = clReleaseCommandQueue(m_pCommandQueueCl);
        AMF_RETURN_IF_CL_FAILED(ret, L"Failed to release command queue.");
    }
    m_pCommandQueueCl = NULL;
    m_pContextAMF = NULL;
    m_pContextTAN = NULL;
    
    return AMF_OK;
}


AMF_RESULT  AMF_STD_CALL    TANMixerImpl::Mix(
    float* ppBufferInput[],
    float* ppBufferOutput
    )
{
	amf_size numOfSamplesToProcess = m_bufferSize;
	int numChannels = m_numChannels;
    for (int idx = 0; idx <numChannels; idx++) 
    {
        int k = 0;
        int n = numOfSamplesToProcess;
        while (n >= 8 && useSSE2)
        {
            register __m256 *out, *in;
            out = (__m256 *)&ppBufferOutput[k];
            in = (__m256 *)&ppBufferInput[idx][k];
            *out = (idx == 0) ? *in : _mm256_add_ps(*out, *in);
            k += 8;
            n -= 8;
        }
        while (n > 0) {
            ppBufferOutput[k] = (idx == 0) ? ppBufferInput[idx][k] : (ppBufferOutput[k] + ppBufferInput[idx][k]);
            k++;
            n--;
        }
    }
    return AMF_OK;
}

// For contigous cl_mem input buffers
AMF_RESULT  AMF_STD_CALL    TANMixerImpl::Mix(
    cl_mem pBufferInput,
    cl_mem pBufferOutput,
    amf_size inputStride
    )
{
	if (!m_OCLInitialized) return AMF_FAIL;
	amf_size numOfSamplesToProcess = m_bufferSize;
    int input_stride = inputStride;
    int num_channels = m_numChannels;
    cl_mem input_buf = pBufferInput;
    cl_mem output_buf = pBufferOutput;
    int argIndex = 0;
    cl_int clErr = clSetKernelArg(m_clMix, argIndex++, sizeof(cl_mem), &input_buf);
    if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument\n"); return AMF_FAIL; }
    clErr = clSetKernelArg(m_clMix, argIndex++, sizeof(cl_mem), &output_buf);
    if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument\n"); return AMF_FAIL; }
    clErr = clSetKernelArg(m_clMix, argIndex++, sizeof(int), &input_stride);
    if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument\n"); return AMF_FAIL; }
    clErr = clSetKernelArg(m_clMix, argIndex++, sizeof(int), &num_channels);
    if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument\n"); return AMF_FAIL; }

    amf_size global[3] = { numOfSamplesToProcess, 0, 0 };
    amf_size local[3] = { (numOfSamplesToProcess>256) ? 256 : numOfSamplesToProcess, 0, 0 };
    //amf_size local[3] = { 1, 0, 0 };
    int status = clEnqueueNDRangeKernel(m_pContextTAN->GetOpenCLGeneralQueue(), m_clMix, 1, NULL, global, local, 0, NULL, NULL);
    AMF_RETURN_IF_CL_FAILED(status, L"Failed to enqueue OCL kernel");
    return AMF_OK;
}

// For disjoint cl_mem input buffers
AMF_RESULT  AMF_STD_CALL    TANMixerImpl::Mix(
    cl_mem pBufferInput[],
    cl_mem pBufferOutput
    )
{
	if (!m_OCLInitialized) return AMF_FAIL;
    // Copy data into the internal contiguous buffers
    for (int i = 0; i < m_numChannels; i++)
    {
        int status = clEnqueueCopyBuffer(
            m_pContextTAN->GetOpenCLGeneralQueue(),
            pBufferInput[i],
            m_internalBuff,
            0,
            i * m_bufferSize * sizeof(float),
            m_bufferSize * sizeof(float),
            0,
            NULL,
            NULL);
		AMF_RETURN_IF_CL_FAILED(status, L"Failed to enqueue OCL copy");
    }
	AMF_RESULT ret = Mix(m_internalBuff, pBufferOutput, m_bufferSize);
	return ret;
}

