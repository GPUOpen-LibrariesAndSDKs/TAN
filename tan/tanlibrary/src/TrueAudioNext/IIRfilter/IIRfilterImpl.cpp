// 
// MIT license 
// 
// Copyright (c) 2018 Advanced Micro Devices, Inc. All rights reserved.
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
// See https://en.wikipedia.org/wiki/Infinite_impulse_response

#define _USE_MATH_DEFINES

#include "IIRfilterImpl.h"
#include "../core/TANContextImpl.h"     
#include "public/common/AMFFactory.h"
#include "../../common/OCLHelper.h"
#include "../../../../common/cpucaps.h"

#include <math.h>

#include "CLKernel_IIRfilter.h"
#define AMF_FACILITY L"IIRfilterImpl"

using namespace amf;

static const AMFEnumDescriptionEntry AMF_MEMORY_ENUM_DESCRIPTION[] =
{
#if AMF_BUILD_OPENCL
	{ AMF_MEMORY_OPENCL, L"OpenCL" },
#endif
	{ AMF_MEMORY_HOST, L"CPU" },
	{ AMF_MEMORY_UNKNOWN, 0 }  // This is end of description mark
};
//-------------------------------------------------------------------------------------------------
TAN_SDK_LINK AMF_RESULT AMF_CDECL_CALL TANCreateIIRfilter(
	amf::TANContext* pContext,
	amf::TANIIRfilter** ppComponent
)
{
	TANContextImplPtr contextImpl(pContext);
	*ppComponent = new TANIIRfilterImpl(pContext);
	(*ppComponent)->Acquire();

	return AMF_OK;
}

//-------------------------------------------------------------------------------------------------
TANIIRfilterImpl::TANIIRfilterImpl(TANContext *pContextTAN) :
	m_pContextTAN(pContextTAN),
	m_numInputTaps(0),
	m_numOutputTaps(0),
	m_channels(0),
	m_inputTaps(NULL),
	m_outputTaps(NULL),
	m_inputHistory(NULL),
	m_outputHistory(NULL),
	m_clTempIn(NULL),
	m_clTempOut(NULL),
	m_clInputTaps(NULL),
	m_clOutputTaps(NULL),
	m_clInputHistory(NULL),
	m_clOutputHistory(NULL),
	m_clInOutHistPos(NULL),
	m_doProcessOnGpu(0)
{
	for (int i = 0; i < MAX_CHANNELS; i++) {
		m_clTempInSubBufs[i] = NULL;
		m_clTempOutSubBufs[i] = NULL;
	}
}

//-------------------------------------------------------------------------------------------------
TANIIRfilterImpl::~TANIIRfilterImpl(void)
{
	Terminate();
}

//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL TANIIRfilterImpl::Terminate()
{
	AMFLock lock(&m_sect);
    if (m_inputTaps != NULL)
    {
        delete[] m_inputTaps;
        m_inputTaps = NULL;
    }
    if (m_outputTaps != NULL)
    {
        delete[] m_outputTaps;
        m_outputTaps = NULL;
    }
    if (m_inputHistory != NULL)
    {
        delete[] m_inputHistory;
        m_inputHistory = NULL;
    }
    if (m_outputHistory != NULL)
    {
        delete[] m_outputHistory;
        m_outputHistory = NULL;
    }

    if (m_pContextTAN->GetOpenCLContext() != nullptr)
    {
        if (m_kernel_IIRfilter)
        {
            clReleaseKernel(m_kernel_IIRfilter);
            m_kernel_IIRfilter = NULL;
        }

        if (m_clTempIn)
        {
            clReleaseMemObject(m_clTempIn);
            m_clTempIn = NULL;
        }

        for (int i = 0; i < m_channels; i++)
        {
            if (m_clTempInSubBufs[i] == NULL) continue;
            clReleaseMemObject(m_clTempInSubBufs[i]);
            m_clTempInSubBufs[i] = NULL;
        }

        for (int i = 0; i < m_channels; i++)
        {
            if (m_clTempOutSubBufs[i] == NULL) continue;
            clReleaseMemObject(m_clTempOutSubBufs[i]);
            m_clTempOutSubBufs[i] = NULL;
        }

        if (m_clTempOut)
        {
            clReleaseMemObject(m_clTempOut);
            m_clTempOut = NULL;
        }

        if (m_clInputTaps)
        {
            clReleaseMemObject(m_clInputTaps);
            m_clInputTaps = NULL;
        }

        if (m_clOutputTaps)
        {
            clReleaseMemObject(m_clOutputTaps);
            m_clOutputTaps = NULL;
        }

        if (m_clInputHistory)
        {
            clReleaseMemObject(m_clInputHistory);
            m_clInputHistory = NULL;
        }

        if (m_clOutputHistory)
        {
            clReleaseMemObject(m_clOutputHistory);
            m_clOutputHistory = NULL;
        }

        if (m_clInOutHistPos)
        {
            clReleaseMemObject(m_clInOutHistPos);
            m_clInOutHistPos = NULL;
        }

    }
    if (m_pCommandQueueCl)
    {
        clReleaseCommandQueue(m_pCommandQueueCl);
        m_pCommandQueueCl = NULL;
    }

	m_pContextTAN.Release();

	return AMF_OK;
}

//-------------------------------------------------------------------------------------------------
AMF_RESULT	AMF_STD_CALL	TANIIRfilterImpl::Init(
	amf_uint32 numInputTaps,
	amf_uint32 numOutputTaps,
	amf_uint32 bufferSizeInSamples,
	amf_uint32 channels)
{
	m_numSamples = bufferSizeInSamples;
	m_numInputTaps = numInputTaps;
	m_numOutputTaps = numOutputTaps;
	m_channels = channels;
	m_bufSize = bufferSizeInSamples * sizeof(float);
	m_inputHistPos = 0;
	m_outputHistPos = 0;	

	if (m_inputTaps != NULL) {
		delete[] m_inputTaps;
		m_inputTaps = NULL;
	}
	if (m_outputTaps != NULL) {
		delete[] m_outputTaps;
		m_outputTaps = NULL;
	}
	if (m_inputHistory != NULL) {
		delete[] m_inputHistory;
		m_inputHistory = NULL;
	}
	if (m_outputHistory != NULL) {
		delete[] m_outputHistory;
		m_outputHistory = NULL;
	}

	m_inputTaps = new float *[m_channels];
	m_outputTaps = new float *[m_channels];
	m_inputHistory = new float *[m_channels];
	m_outputHistory = new float *[m_channels];

	for (int i = 0; i < m_channels; i++) {
		m_inputTaps[i] = new float[m_numInputTaps];
		m_outputTaps[i] = new float[m_numOutputTaps];
		m_inputHistory[i] = new float[m_numInputTaps];
		m_outputHistory[i] = new float[m_numOutputTaps];
		memset(m_inputTaps[i], 0, m_numInputTaps * sizeof(float));
		memset(m_outputTaps[i], 0, m_numOutputTaps * sizeof(float));
		memset(m_inputHistory[i], 0, m_numInputTaps * sizeof(float));
		memset(m_outputHistory[i], 0, m_numOutputTaps * sizeof(float));

	}

	// Determine how to initialize based on context, CPU for CPU and GPU for GPU
	if (m_pContextTAN->GetOpenCLContext())	
	{
		m_doProcessOnGpu = true;
		return InitGpu();
	}
	else
	{
		return InitCpu();
	}
}

//-------------------------------------------------------------------------------------------------
AMF_RESULT	AMF_STD_CALL	TANIIRfilterImpl::InitGpu()
{
	cl_int ret;
	AMF_RESULT res = AMF_OK;

	/* OpenCL Initialization */

	// Given some command queue, retrieve the cl_context...
	m_pCommandQueueCl = m_pContextTAN->GetOpenCLGeneralQueue();
	ret = clGetCommandQueueInfo(m_pCommandQueueCl, CL_QUEUE_CONTEXT, sizeof(cl_context), &m_pContextCl, NULL);

	AMF_RETURN_IF_CL_FAILED(ret, L"Cannot retrieve cl_context from cl_command_queue.");
	
	// ...and cl_device from it
	ret = clGetCommandQueueInfo(m_pCommandQueueCl, CL_QUEUE_DEVICE, sizeof(cl_device_id),
		&m_pDeviceCl, NULL);
	AMF_RETURN_IF_CL_FAILED(ret, L"Cannot retrieve cl_device_id from cl_command_queue.");

	// Retain the queue for use
	ret = clRetainCommandQueue(m_pCommandQueueCl);
	AMF_RETURN_IF_CL_FAILED(ret, L"Failed to retain command queue.");

	TANContextImplPtr contextImpl(m_pContextTAN);
	m_pDeviceAMF = contextImpl->GetGeneralCompute();

	m_clTempIn = clCreateBuffer(
		m_pContextCl, CL_MEM_READ_WRITE, m_bufSize*m_channels, nullptr, &ret);
	AMF_RETURN_IF_CL_FAILED(ret, L"Failed to create buffer");

	m_clTempOut = clCreateBuffer(
		m_pContextCl, CL_MEM_READ_WRITE, m_bufSize*m_channels, nullptr, &ret);
	AMF_RETURN_IF_CL_FAILED(ret, L"Failed to create buffer");
	
	int pat = 0;
	m_clInputTaps = clCreateBuffer(
		m_pContextCl, CL_MEM_READ_WRITE, sizeof(float)*m_channels*m_numInputTaps, nullptr, &ret);
	AMF_RETURN_IF_CL_FAILED(ret, L"Failed to create buffer");

	m_clOutputTaps = clCreateBuffer(
		m_pContextCl, CL_MEM_READ_WRITE, sizeof(float)*m_channels*m_numOutputTaps, nullptr, &ret);
	AMF_RETURN_IF_CL_FAILED(ret, L"Failed to create buffer");

	m_clInputHistory = clCreateBuffer(
		m_pContextCl, CL_MEM_READ_WRITE, sizeof(float)*m_channels*m_numInputTaps, nullptr, &ret);
	AMF_RETURN_IF_CL_FAILED(ret, L"Failed to create buffer");

	clEnqueueFillBuffer(m_pCommandQueueCl, m_clInputHistory, (const void *)&pat, sizeof(pat), 0, m_channels*m_numInputTaps * sizeof(float), 0, nullptr, nullptr);

	m_clOutputHistory = clCreateBuffer(
		m_pContextCl, CL_MEM_READ_WRITE, sizeof(float)*m_channels*m_numOutputTaps, nullptr, &ret);
	AMF_RETURN_IF_CL_FAILED(ret, L"Failed to create buffer");

	clEnqueueFillBuffer(m_pCommandQueueCl, m_clOutputHistory, (const void *)&pat, sizeof(pat), 0, m_channels*m_numOutputTaps * sizeof(float), 0, nullptr, nullptr);

	m_clInOutHistPos = clCreateBuffer(
		m_pContextCl, CL_MEM_READ_WRITE, sizeof(float) * 2, nullptr, &ret);
	AMF_RETURN_IF_CL_FAILED(ret, L"Failed to create buffer");

	clEnqueueFillBuffer(m_pCommandQueueCl, m_clInOutHistPos, (const void *)&pat, sizeof(pat), 0, sizeof(float) * 2, 0, nullptr, nullptr);
	
	for (amf_uint32 i = 0; i < m_channels; i++)
	{
		cl_buffer_region region;
		region.origin = i*m_bufSize;
		region.size = m_bufSize;
		m_clTempInSubBufs[i] = clCreateSubBuffer(
			m_clTempIn, CL_MEM_READ_WRITE, CL_BUFFER_CREATE_TYPE_REGION, &region, &ret);
	}
	
	for (amf_uint32 i = 0; i < m_channels; i++)
	{
		cl_buffer_region region;
		region.origin = i*m_bufSize;
		region.size = m_bufSize;
		m_clTempOutSubBufs[i] = clCreateSubBuffer(
			m_clTempOut, CL_MEM_READ_WRITE, CL_BUFFER_CREATE_TYPE_REGION, &region, &ret);
	}

	//... Preparing OCL Kernel
	int OCLKenel_Err = GetOclKernel(m_kernel_IIRfilter, m_pDeviceAMF, contextImpl->GetOpenCLGeneralQueue(), "IIRfilter", IIRfilter, IIRfilterCount, "IIRfilter", "");
	if (!OCLKenel_Err) { printf("Failed to compile IIRFilter Kernel IIR_Filter"); return AMF_FAIL; }
	return res;
}

//-------------------------------------------------------------------------------------------------
AMF_RESULT	AMF_STD_CALL	TANIIRfilterImpl::InitCpu()
{
	return AMF_OK;
}

AMF_RESULT AMF_STD_CALL TANIIRfilterImpl::UpdateIIRResponses(float* ppInputResponse[], float* ppOutputResponse[],
	amf_size inResponseSz, amf_size outResponseSz,
	const amf_uint32 flagMasks[],   // Masks of flags from enum TAN_IIR_CHANNEL_FLAG, can be NULL.
	const amf_uint32 operationFlags // Mask of flags from enum TAN_IIR_OPERATION_FLAG.
)
{
	if (inResponseSz > m_numInputTaps) {
		inResponseSz = m_numInputTaps;
	}
	if (outResponseSz > m_numOutputTaps) {
		outResponseSz = m_numOutputTaps;
	}

	for (int i = 0; i < m_channels; i++) {
		memset(m_inputTaps[i], 0, m_numInputTaps * sizeof(float));
		memset(m_outputTaps[i], 0, m_numOutputTaps * sizeof(float));

		memcpy(m_inputTaps[i], ppInputResponse[i], inResponseSz * sizeof(float));
		memcpy(m_outputTaps[i], ppOutputResponse[i], outResponseSz * sizeof(float));
	}	

	if (m_doProcessOnGpu)
	{
		float* inputTaps = new float[m_channels*m_numInputTaps];
		float* outputTaps = new float[m_channels*m_numOutputTaps];

		for (int chan = 0; chan < m_channels; chan++)
		{
			for (int i = 0; i < m_numInputTaps; i++)
			{
				inputTaps[m_numInputTaps*chan + i] = m_inputTaps[chan][i];
			}

			for (int i = 0; i < m_numOutputTaps; i++)
			{
				outputTaps[m_numOutputTaps*chan + i] = m_outputTaps[chan][i];
			}
		}

		clEnqueueWriteBuffer(this->m_pContextTAN->GetOpenCLGeneralQueue(), m_clInputTaps, CL_TRUE, 0, sizeof(float) * m_numInputTaps*m_channels, inputTaps, 0, NULL, NULL);
		clEnqueueWriteBuffer(this->m_pContextTAN->GetOpenCLGeneralQueue(), m_clOutputTaps, CL_TRUE, 0, sizeof(float) * m_numOutputTaps*m_channels, outputTaps, 0, NULL, NULL);
		delete[] inputTaps;
		delete[] outputTaps;
	}
			
	return AMF_OK;
}

AMF_RESULT  AMF_STD_CALL    TANIIRfilterImpl::ProcessDirect(float* ppBufferInput[],
	float* ppBufferOutput[],
	amf_size numOfSamplesToProcess,
	const amf_uint32 flagMasks[],    // Masks of flags from enum TAN_IIR_CHANNEL_FLAG, can be NULL.
	amf_size *pNumOfSamplesProcessed // Can be NULL.
)
{
	if (pNumOfSamplesProcessed)
	{
		*pNumOfSamplesProcessed = 0;
	}

	//Todo:: move to funcion
	float sample;
	for (int sn = 0; sn < numOfSamplesToProcess; sn++) {
		// compute next sample for each channel
		for (int chan = 0; chan < m_channels; chan++) {
			sample = 0.0;
			m_inputHistory[chan][m_inputHistPos] = ppBufferInput[chan][sn];

			//FIR part
			for (int k = 0; k < m_numInputTaps; k++) {
				sample += m_inputTaps[chan][k] * m_inputHistory[chan][(m_inputHistPos + m_numInputTaps - k) % m_numInputTaps];
			}
			//IIR part
			for (int l = 0; l < m_numOutputTaps; l++) {
				sample += m_outputTaps[chan][l] * m_outputHistory[chan][(m_outputHistPos + m_numOutputTaps - l) % m_numOutputTaps];
			}
			ppBufferOutput[chan][sn] = sample;
		}

		m_inputHistPos = ++m_inputHistPos % m_numInputTaps;
		m_outputHistPos = ++m_outputHistPos % m_numOutputTaps;

		for (int chan = 0; chan < m_channels; chan++) {
			m_outputHistory[chan][m_outputHistPos] = ppBufferOutput[chan][sn];
		}
	}

	if (pNumOfSamplesProcessed)
	{
		*pNumOfSamplesProcessed = numOfSamplesToProcess;
	}

	return AMF_OK;
}

AMF_RESULT  AMF_STD_CALL    TANIIRfilterImpl::Process(float* ppBufferInput[],
	float* ppBufferOutput[],
	amf_size numOfSamplesToProcess,
	const amf_uint32 flagMasks[],    // Masks of flags from enum TAN_IIR_CHANNEL_FLAG, can be NULL.
	amf_size *pNumOfSamplesProcessed // Can be NULL.
)
{
	cl_context context = m_pContextTAN->GetOpenCLContext();

	if (pNumOfSamplesProcessed)
	{
		*pNumOfSamplesProcessed = 0;
	}

	//Copy to temp OCL buffer from host
	for (int chan = 0; chan < m_channels; chan++)
	{
		float * ptr = ppBufferInput[chan];
		clEnqueueWriteBuffer(m_pCommandQueueCl, m_clTempInSubBufs[chan], 1, 0, numOfSamplesToProcess*sizeof(float), ppBufferInput[chan], 0, nullptr, nullptr);
	}

	IIRFilterProcessGPU(m_clTempIn, m_clTempOut, m_numSamples, flagMasks, pNumOfSamplesProcessed);	

	//Write to host from CL output buffer
	for (int chan = 0; chan < m_channels; chan++)
	{
		float * ptr = ppBufferOutput[chan];
		clEnqueueReadBuffer(m_pCommandQueueCl, m_clTempOutSubBufs[chan], 1, 0, numOfSamplesToProcess* sizeof(float), ppBufferOutput[chan], 0, nullptr, nullptr);
	}

	if (pNumOfSamplesProcessed)
	{
		*pNumOfSamplesProcessed = numOfSamplesToProcess;
	}

	return AMF_OK;
}


AMF_RESULT  AMF_STD_CALL    TANIIRfilterImpl::Process(
	cl_mem ppBufferInput[],
	cl_mem ppBufferOutput[],
	amf_size numOfSamplesToProcess,
	const amf_uint32 flagMasks[],    // Masks of flags from enum TAN_IIR_CHANNEL_FLAG, can be NULL.
	amf_size *pNumOfSamplesProcessed // Can be NULL.
)
{
	return AMF_OK;
}

AMF_RESULT  AMF_STD_CALL    TANIIRfilterImpl::IIRFilterProcessGPU(
	cl_mem inputBuf,
	cl_mem outputBuf,
	amf_size numOfSamplesToProcess,
	const amf_uint32 flagMasks[],    // Masks of flags from enum TAN_IIR_CHANNEL_FLAG, can be NULL.
	amf_size *pNumOfSamplesProcessed // Can be NULL.
)
{
	AMF_RESULT res = AMF_OK;
	
	// Create as many work-items as there are conversions needed to be done and execute
	size_t local[3] = { 128, 1, 0 };
	size_t global[3] = { 1, m_channels, 0 };

	int status = 0;
	cl_int argCounter = 0;

	status |= clSetKernelArg(m_kernel_IIRfilter, argCounter++, sizeof(cl_mem), &inputBuf);
	status |= clSetKernelArg(m_kernel_IIRfilter, argCounter++, sizeof(cl_mem), &m_clInputHistory);
	status |= clSetKernelArg(m_kernel_IIRfilter, argCounter++, sizeof(cl_mem), &m_clOutputHistory);
	status |= clSetKernelArg(m_kernel_IIRfilter, argCounter++, sizeof(cl_mem), &m_clInputTaps);
	status |= clSetKernelArg(m_kernel_IIRfilter, argCounter++, sizeof(cl_mem), &m_clOutputTaps);
	status |= clSetKernelArg(m_kernel_IIRfilter, argCounter++, sizeof(cl_mem), &outputBuf);
	status |= clSetKernelArg(m_kernel_IIRfilter, argCounter++, sizeof(cl_mem), &m_clInOutHistPos);
	status |= clSetKernelArg(m_kernel_IIRfilter, argCounter++, sizeof(cl_int), &m_numInputTaps);
	status |= clSetKernelArg(m_kernel_IIRfilter, argCounter++, sizeof(cl_int), &m_numOutputTaps);
	status |= clSetKernelArg(m_kernel_IIRfilter, argCounter++, sizeof(cl_int), &numOfSamplesToProcess);
	status = clEnqueueNDRangeKernel(m_pCommandQueueCl, m_kernel_IIRfilter, 2, NULL, global, local, 0, NULL, NULL);

	if (status != CL_SUCCESS) { printf("Failed to enqueue OpenCL kernel\n"); return AMF_FAIL; }
	return AMF_OK;
}


