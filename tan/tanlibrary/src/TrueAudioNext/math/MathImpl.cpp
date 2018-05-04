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
//

#include "MathImpl.h"
#include "../core/TANContextImpl.h"
#include "public/common/AMFFactory.h"
#include "../../common/OCLHelper.h"

#include "CLKernel_VectorComplexDivision.h"
#include "CLKernel_VectorComplexMultiply.h"
#include "CLKernel_VectorComplexSum.h"
//#include "VectorComplexDivision.cl.h"
//#include "VectorComplexMultiply.cl.h"
//#include "VectorComplexSum.cl.h"

#include <memory>

#define AMF_FACILITY L"TANMathImpl"

using namespace amf;

//-------------------------------------------------------------------------------------------------
//public-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
static const AMFEnumDescriptionEntry AMF_MEMORY_ENUM_DESCRIPTION[] =
{
#if AMF_BUILD_OPENCL
    { AMF_MEMORY_OPENCL, L"OpenCL" },
#endif
    { AMF_MEMORY_HOST, L"CPU" },
    { AMF_MEMORY_UNKNOWN, 0 }  // This is end of description mark
};
//-------------------------------------------------------------------------------------------------
TAN_SDK_LINK AMF_RESULT AMF_CDECL_CALL TANCreateMath(
    amf::TANContext* pContext, 
    amf::TANMath** ppComponent
    )
{
    TANContextImplPtr contextImpl(pContext);
    *ppComponent = new TANMathImpl( pContext );
    (*ppComponent)->Acquire();
    return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
TANMathImpl::TANMathImpl(TANContext *pContextTAN) :
    m_pContextTAN(pContextTAN),
	///TODO:AAA     m_pContextAMF(pContextAMF),
	m_pDeviceCompute(NULL),
    m_eOutputMemoryType(AMF_MEMORY_HOST),
    m_gpuDivisionRunNum(0),
    m_gpuMultiplicationRunNum(0),
	m_iInternalSwapBuffer1Size_MulAccu(0),
	m_iInternalSwapBuffer2Size_MulAccu(0),
	m_pInternalSwapBuffer2_MulAccu(nullptr),
	m_pInternalSwapBuffer1_MulAccu(nullptr),
	m_iInternalBufferIn1Size_MulAccu(0),
	m_iInternalBufferIn2Size_MulAccu(0),
	m_iInternalBufferOutSize_MulAccu(0),
	m_pInternalBufferOut_MulAccu(nullptr),
	m_pInternalBufferIn1_MulAccu(nullptr),
	m_pInternalBufferIn2_MulAccu(nullptr),
	m_iInternalBufferIn1Size_Division(0),
	m_iInternalBufferIn2Size_Division(0),
	m_iInternalBufferOutSize_Division(0),
	m_pInternalBufferOut_Division(nullptr),
	m_pInternalBufferIn1_Division(nullptr),
	m_pInternalBufferIn2_Division(nullptr),
	m_iInternalBufferIn1Size_Multiply(0),
	m_iInternalBufferIn2Size_Multiply(0),
	m_iInternalBufferOutSize_Multiply(0),
	m_pInternalBufferOut_Multiply(nullptr),
	m_pInternalBufferIn1_Multiply(nullptr),
	m_pInternalBufferIn2_Multiply(nullptr)
{
    AMFPrimitivePropertyInfoMapBegin
        AMFPropertyInfoEnum(TAN_OUTPUT_MEMORY_TYPE, L"Output Memory Type", AMF_MEMORY_HOST, AMF_MEMORY_ENUM_DESCRIPTION, false),
    AMFPrimitivePropertyInfoMapEnd
}
//-------------------------------------------------------------------------------------------------
TANMathImpl::~TANMathImpl(void)
{
    Terminate();
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL TANMathImpl::Init()
{
	AMF_RETURN_IF_FALSE(!m_pDeviceCompute, AMF_ALREADY_INITIALIZED, L"Already initialized");
	AMF_RETURN_IF_FALSE(m_pContextTAN != NULL, AMF_WRONG_STATE,
		L"Cannot initialize after termination");

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
AMF_RESULT  AMF_STD_CALL TANMathImpl::InitCpu()
{
	// No device setup needs to occur here; we're done!
	return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL TANMathImpl::InitGpu()
{

	AMF_RETURN_IF_FALSE(!m_pDeviceCompute, AMF_ALREADY_INITIALIZED, L"Already initialized");
	AMF_RETURN_IF_FALSE((m_pContextTAN != NULL), AMF_WRONG_STATE,
		L"Cannot initialize after termination");

	amf_int64 tmp = 0;
	GetProperty(TAN_OUTPUT_MEMORY_TYPE, &tmp);
	m_eOutputMemoryType = (AMF_MEMORY_TYPE)tmp;

	AMFPrograms* pPrograms = NULL;
	g_AMFFactory.GetFactory()->GetPrograms(&pPrograms);

	TANContextImplPtr contextImpl(m_pContextTAN);
	m_pDeviceCompute = contextImpl->GetGeneralCompute();
	if (NULL == m_pDeviceCompute)
	{
		return AMF_OK;
	}

	bool OCLKernel_Err = false;
	// for now register source kernels
	if (m_pKernelComplexDiv == nullptr)
	{	
		OCLKernel_Err = GetOclKernel(m_pKernelComplexDiv, m_pDeviceCompute, m_pContextTAN->GetOpenCLGeneralQueue(), "VectorComplexDivision", VectorComplexDivision_Str, VectorComplexDivisionCount,
			"VectorComplexDiv", "");
		if (!OCLKernel_Err){ printf("Failed to initialize Kernel\n"); return AMF_FAIL;}
	}
	if (m_pKernelComplexMul == 0)
	{
		OCLKernel_Err = GetOclKernel(m_pKernelComplexMul, m_pDeviceCompute, m_pContextTAN->GetOpenCLGeneralQueue(), "VectorComplexMul", VectorComplexMultiply_Str, VectorComplexMultiplyCount,
			"VectorComplexMul", "");
		if (!OCLKernel_Err){ printf("Failed to initialize Kernel\n"); return AMF_FAIL; }
	}
	if (m_pKernelComplexSum == 0)
	{
		OCLKernel_Err = GetOclKernel(m_pKernelComplexSum, m_pDeviceCompute, m_pContextTAN->GetOpenCLGeneralQueue(), "VectorComplexSum", VectorComplexSum_Str, VectorComplexSumCount,
			"VectorComplexSum", "");
		if (!OCLKernel_Err){ printf("Failed to initialize Kernel\n"); return AMF_FAIL; }
	}


	return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT TANMathImpl::AdjustInternalBufferSize(
	cl_mem* _buffer, 
	amf_size* size, // in bytes
	const amf_size requiredSize)
{
	if (requiredSize > *size)
	{
		cl_int clErr = 0;
		*size = requiredSize;
		// Bigger buffer size, need to create a new auxiliary buffer
		if (*_buffer != nullptr)
		{
			clErr = clReleaseMemObject(*_buffer);
			if (clErr != CL_SUCCESS) { printf("Unable to release auxiliary buffer"); return AMF_FAIL; }
			*_buffer = nullptr;
		}
		*_buffer = clCreateBuffer(m_pContextTAN->GetOpenCLContext(), CL_MEM_READ_WRITE, *size,
			NULL, &clErr);
		if (clErr != CL_SUCCESS) { printf("Failed to create auxiliary array in OpenCL"); return AMF_FAIL; }
		cl_float filled = 0.0;
		clErr = clEnqueueFillBuffer(m_pContextTAN->GetOpenCLGeneralQueue(), m_pInternalBufferIn1_Multiply, &filled, sizeof(cl_float), 0, *size, 0, NULL, NULL);
	}
	return AMF_OK;
}

//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL TANMathImpl::Terminate()
{
	m_pDeviceCompute = nullptr;

	m_pDeviceCompute = nullptr;
    m_pContextTAN = nullptr;
    m_pKernelComplexDiv = nullptr;
    m_pKernelComplexMul = nullptr;
    return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT TANMathImpl::ComplexMultiplication(
	const float* const inputBuffers1[],
	const float* const inputBuffers2[],
	float *outputBuffers[],
	amf_uint32 channels,
    amf_size countOfComplexNumbers
	)
{
	AMF_RETURN_IF_FALSE(inputBuffers1 != NULL, AMF_INVALID_ARG, L"inputBuffer1 == NULL");
	AMF_RETURN_IF_FALSE(inputBuffers2 != NULL, AMF_INVALID_ARG, L"inputBuffer2 == NULL");
	AMF_RETURN_IF_FALSE(outputBuffers != NULL, AMF_INVALID_ARG, L"outputBuffer == NULL");
	AMF_RETURN_IF_FALSE(channels > 0, AMF_INVALID_ARG, L"channels == 0");
    AMF_RETURN_IF_FALSE(countOfComplexNumbers > 0, AMF_INVALID_ARG, L"countOfComplexNumbers == 0");

	for (amf_size channelId = 0; channelId < channels; channelId++)
	{
		AMF_RETURN_IF_FALSE(inputBuffers1[channelId] != NULL, AMF_INVALID_ARG, L"inputBuffers1[%u] == NULL", channelId);
		AMF_RETURN_IF_FALSE(inputBuffers2[channelId] != NULL, AMF_INVALID_ARG, L"inputBuffers2[%u] == NULL", channelId);
		AMF_RETURN_IF_FALSE(outputBuffers[channelId] != NULL, AMF_INVALID_ARG, L"outputBuffers[%u] == NULL", channelId);

		AMF_RESULT result = AMF_FAIL;
        result = ComplexMultiplication(inputBuffers1[channelId], inputBuffers2[channelId], outputBuffers[channelId], countOfComplexNumbers);
		AMF_RETURN_IF_FAILED(result);
	}
	return AMF_OK;
}
//-----------------------------------------------------------------------------------------------------
AMF_RESULT TANMathImpl::ComplexMultiplyAccumulate(
	const float* const inputBuffers1[],
	const float* const inputBuffers2[],
	float *accumbuffers[],
	amf_uint32 channels,
    amf_size countOfComplexNumbers)
{
	AMF_RETURN_IF_FALSE(inputBuffers1 != NULL, AMF_INVALID_ARG, L"inputBuffers1 == NULL");
	AMF_RETURN_IF_FALSE(inputBuffers2 != NULL, AMF_INVALID_ARG, L"inputBuffers1 == NULL");
	AMF_RETURN_IF_FALSE(accumbuffers!= NULL, AMF_INVALID_ARG, L"accumbuffers == NULL");
	AMF_RETURN_IF_FALSE(channels > 0, AMF_INVALID_ARG, L"channels == 0");
    AMF_RETURN_IF_FALSE(countOfComplexNumbers > 0, AMF_INVALID_ARG, L"countOfComplexNumbers == 0");

	for (amf_size channelId = 0; channelId < channels; channelId++)
	{
		AMF_RETURN_IF_FALSE(inputBuffers1[channelId] != NULL, AMF_INVALID_ARG, L"inputBuffers1[%u] == NULL", channelId);
		AMF_RETURN_IF_FALSE(inputBuffers2[channelId] != NULL, AMF_INVALID_ARG, L"inputBuffers2[%u] == NULL", channelId);
		AMF_RETURN_IF_FALSE(accumbuffers[channelId] != NULL, AMF_INVALID_ARG, L"outputBuffers[%u] == NULL", channelId);

		AMF_RESULT result = AMF_FAIL;
		result = ComplexMultiplyAccumulate(
            inputBuffers1[channelId], inputBuffers2[channelId], accumbuffers[channelId], countOfComplexNumbers);
		AMF_RETURN_IF_FAILED(result);
	}
	return AMF_OK;
}

AMF_RESULT TANMathImpl::ComplexMultiplyAccumulate(
	const cl_mem inputBuffers1[],
	const amf_size buffers1OffsetInSamples[],
	const cl_mem inputBuffers2[],
	const amf_size buffers2OffsetInSamples[],
	cl_mem accumBuffers[],
	const amf_size accumBuffersOffsetInSamples[],
	amf_uint32 channels,
    amf_size countOfComplexNumbers)
{
	if (m_pKernelComplexSum == 0)
	{
		return AMF_OPENCL_FAILED;
	}

	AMF_RETURN_IF_FALSE(inputBuffers1 != NULL, AMF_INVALID_ARG, L"inputBuffer == NULL");
	AMF_RETURN_IF_FALSE(inputBuffers2 != NULL, AMF_INVALID_ARG, L"inputBuffer == NULL");
	AMF_RETURN_IF_FALSE(accumBuffers!= NULL, AMF_INVALID_ARG, L"accumBuffers == NULL");
	AMF_RETURN_IF_FALSE(channels > 0, AMF_INVALID_ARG, L"channels == 0");
    AMF_RETURN_IF_FALSE(countOfComplexNumbers > 0, AMF_INVALID_ARG, L"countOfComplexNumbers == 0");
    AMF_RETURN_IF_FALSE(!(countOfComplexNumbers & 1), AMF_INVALID_ARG, L"countOfComplexNumbers is odd not supported by OpenCL kernel");

	for (amf_size channelId = 0; channelId < channels; channelId++)
	{
		AMF_RETURN_IF_FALSE(inputBuffers1[channelId] != NULL, AMF_INVALID_ARG, L"inputBuffers1[%u] == NULL", channelId);
		AMF_RETURN_IF_FALSE(inputBuffers2[channelId] != NULL, AMF_INVALID_ARG, L"inputBuffers2[%u] == NULL", channelId);
		AMF_RETURN_IF_FALSE(accumBuffers[channelId] != NULL, AMF_INVALID_ARG, L"accumBuffers[%u] == NULL", channelId);

		AMF_RESULT result = AMF_FAIL;
		result = ComplexMultiplyAccumulate(
			inputBuffers1[channelId], buffers1OffsetInSamples[channelId],
			inputBuffers2[channelId], buffers2OffsetInSamples[channelId],
            accumBuffers[channelId], accumBuffersOffsetInSamples[channelId], countOfComplexNumbers
			);
		AMF_RETURN_IF_FAILED(result);
	}
	return AMF_OK;
}

//-------------------------------------------------------------------------------------------------
AMF_RESULT TANMathImpl::ComplexMultiplication(
	const cl_mem inputBuffers1[],
	const amf_size buffers1OffsetInSamples[],
	const cl_mem inputBuffers2[],
	const amf_size buffers2OffsetInSamples[],
	cl_mem outputBuffers[],
	const amf_size outputBuffersOffsetInSamples[],
	amf_uint32 channels,
    amf_size countOfComplexNumbers
	)
{
	if (m_pKernelComplexMul == 0)
	{
		return AMF_OPENCL_FAILED;
	}

	AMF_RETURN_IF_FALSE(inputBuffers1 != NULL, AMF_INVALID_ARG, L"inputBuffer1 == NULL");
	AMF_RETURN_IF_FALSE(inputBuffers2 != NULL, AMF_INVALID_ARG, L"inputBuffer2 == NULL");
	AMF_RETURN_IF_FALSE(outputBuffers != NULL, AMF_INVALID_ARG, L"outputBuffer == NULL");
	AMF_RETURN_IF_FALSE(channels > 0, AMF_INVALID_ARG, L"channels == 0");
    AMF_RETURN_IF_FALSE(countOfComplexNumbers > 0, AMF_INVALID_ARG, L"countOfComplexNumbers == 0");
    AMF_RETURN_IF_FALSE(!(countOfComplexNumbers & 1), AMF_INVALID_ARG, L"countOfComplexNumbers is odd not supported by OpenCL kernel");

	for (amf_size channelId = 0; channelId < channels; channelId++)
	{
		AMF_RETURN_IF_FALSE(inputBuffers1[channelId] != NULL, AMF_INVALID_ARG, L"inputBuffers1[%u] == NULL", channelId);
		AMF_RETURN_IF_FALSE(inputBuffers2[channelId] != NULL, AMF_INVALID_ARG, L"inputBuffers2[%u] == NULL", channelId);
		AMF_RETURN_IF_FALSE(outputBuffers[channelId] != NULL, AMF_INVALID_ARG, L"outputBuffers[%u] == NULL", channelId);

		AMF_RESULT result = AMF_FAIL;
		result = ComplexMultiplication(	inputBuffers1[channelId], buffers1OffsetInSamples[channelId],
										inputBuffers2[channelId], buffers2OffsetInSamples[channelId],
										outputBuffers[channelId], outputBuffersOffsetInSamples[channelId],
                                        countOfComplexNumbers);
		AMF_RETURN_IF_FAILED(result);
	}
	return AMF_OK;
}

//-------------------------------------------------------------------------------------------------
AMF_RESULT TANMathImpl::ComplexDivision(
	const float* const inputBuffers1[],
	const float* const inputBuffers2[],
	float *outputBuffers[],
	amf_uint32 channels,
    amf_size countOfComplexNumbers
	)
{
	AMF_RETURN_IF_FALSE(inputBuffers1 != NULL, AMF_INVALID_ARG, L"inputBuffer1 == NULL");
	AMF_RETURN_IF_FALSE(inputBuffers2 != NULL, AMF_INVALID_ARG, L"inputBuffer2 == NULL");
	AMF_RETURN_IF_FALSE(outputBuffers != NULL, AMF_INVALID_ARG, L"outputBuffer == NULL");
	AMF_RETURN_IF_FALSE(channels > 0, AMF_INVALID_ARG, L"channels == 0");
    AMF_RETURN_IF_FALSE(countOfComplexNumbers > 0, AMF_INVALID_ARG, L"countOfComplexNumbers == 0");

	for (amf_size channelId = 0; channelId < channels; channelId++)
	{
		AMF_RETURN_IF_FALSE(inputBuffers1[channelId] != NULL, AMF_INVALID_ARG, L"inputBuffers1[%u] == NULL", channelId);
		AMF_RETURN_IF_FALSE(inputBuffers2[channelId] != NULL, AMF_INVALID_ARG, L"inputBuffers2[%u] == NULL", channelId);
		AMF_RETURN_IF_FALSE(outputBuffers[channelId] != NULL, AMF_INVALID_ARG, L"outputBuffers[%u] == NULL", channelId);
		AMF_RESULT result = AMF_FAIL;
        result = ComplexDivision(inputBuffers1[channelId], inputBuffers2[channelId], outputBuffers[channelId], countOfComplexNumbers);
		AMF_RETURN_IF_FAILED(result);
	}
	return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT TANMathImpl::ComplexDivision(
	const cl_mem inputBuffers1[],
	const amf_size buffers1OffsetInSamples[],
	const cl_mem inputBuffers2[],
	const amf_size buffers2OffsetInSamples[],
	cl_mem outputBuffers[],
	const amf_size outputBuffersOffsetInSamples[],
	amf_uint32 channels,
    amf_size countOfComplexNumbers
	)
{
	if (m_pKernelComplexDiv == 0)
	{
		return AMF_OPENCL_FAILED;
	}

	AMF_RETURN_IF_FALSE(inputBuffers1 != NULL, AMF_INVALID_ARG, L"inputBuffer1 == NULL");
	AMF_RETURN_IF_FALSE(inputBuffers2 != NULL, AMF_INVALID_ARG, L"inputBuffer2 == NULL");
	AMF_RETURN_IF_FALSE(outputBuffers != NULL, AMF_INVALID_ARG, L"outputBuffer == NULL");
	AMF_RETURN_IF_FALSE(channels > 0, AMF_INVALID_ARG, L"channels == 0");
    AMF_RETURN_IF_FALSE(countOfComplexNumbers > 0, AMF_INVALID_ARG, L"countOfComplexNumbers == 0");
    AMF_RETURN_IF_FALSE(!(countOfComplexNumbers & 1), AMF_INVALID_ARG,
		L"countOfComplexNumbers is odd, not supported by OpenCL kernel");

	for (amf_size channelId = 0; channelId < channels; channelId++)
	{
		AMF_RETURN_IF_FALSE(inputBuffers1[channelId] != NULL, AMF_INVALID_ARG, L"inputBuffers1[%u] == NULL", channelId);
		AMF_RETURN_IF_FALSE(inputBuffers2[channelId] != NULL, AMF_INVALID_ARG, L"inputBuffers2[%u] == NULL", channelId);
		AMF_RETURN_IF_FALSE(outputBuffers[channelId] != NULL, AMF_INVALID_ARG, L"outputBuffers[%u] == NULL", channelId);
		AMF_RESULT result = AMF_FAIL;
		result = ComplexDivision(	inputBuffers1[channelId], buffers1OffsetInSamples[channelId],
									inputBuffers2[channelId], buffers2OffsetInSamples[channelId],
									outputBuffers[channelId], outputBuffersOffsetInSamples[channelId],
                                    countOfComplexNumbers);
		AMF_RETURN_IF_FAILED(result);
	}
	return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
//protected----------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
AMF_RESULT TANMathImpl::ComplexMultiplication(
    const float inputBuffer1[],
    const float inputBuffer2[],
    float outputBuffer[],
    amf_size countOfComplexNumbers
)
{
    AMF_RETURN_IF_FALSE(inputBuffer1 != NULL, AMF_INVALID_ARG, L"inputBuffer1 == NULL");
    AMF_RETURN_IF_FALSE(inputBuffer2 != NULL, AMF_INVALID_ARG, L"inputBuffer2 == NULL");
    AMF_RETURN_IF_FALSE(outputBuffer != NULL, AMF_INVALID_ARG, L"outputBuffer == NULL");

    AMF_RETURN_IF_FALSE(countOfComplexNumbers > 0, AMF_INVALID_ARG, L"countOfComplexNumbers == 0");
 
	// If GPU Context is initialized then run it on GPU
	if (m_pContextTAN->GetOpenCLContext())
	{
		// Adjust internal buffer size
		cl_int clErr = 0;;
        amf_size requiredbuffersize = countOfComplexNumbers * sizeof(float) * 2; //2 floats per complex sample
		AdjustInternalBufferSize(&m_pInternalBufferOut_Multiply, &m_iInternalBufferOutSize_Multiply, requiredbuffersize);
		AdjustInternalBufferSize(&m_pInternalBufferIn1_Multiply, &m_iInternalBufferIn1Size_Multiply, requiredbuffersize);
		AdjustInternalBufferSize(&m_pInternalBufferIn2_Multiply, &m_iInternalBufferIn2Size_Multiply, requiredbuffersize);
		cl_command_queue cmdQueue = m_pContextTAN->GetOpenCLGeneralQueue();
		clErr = clEnqueueWriteBuffer(cmdQueue, m_pInternalBufferIn1_Multiply, CL_TRUE, 0, requiredbuffersize, inputBuffer1, 0, NULL, NULL);
		if (clErr != CL_SUCCESS){ printf("Failed to copy from HOST to OPENCL memory"); return AMF_FAIL; }
		clErr = clEnqueueWriteBuffer(cmdQueue, m_pInternalBufferIn2_Multiply, CL_TRUE, 0, requiredbuffersize, inputBuffer2, 0, NULL, NULL);
		if (clErr != CL_SUCCESS){ printf("Failed to copy from HOST to OPENCL memory"); return AMF_FAIL; }
        ComplexMultiplication(m_pInternalBufferIn1_Multiply, 0, m_pInternalBufferIn2_Multiply, 0, m_pInternalBufferOut_Multiply, 0, countOfComplexNumbers);
		clErr = clEnqueueReadBuffer(cmdQueue, m_pInternalBufferOut_Multiply, CL_TRUE, 0, requiredbuffersize, outputBuffer, 0, NULL, NULL);
		if (clErr != CL_SUCCESS){ printf("Failed to copy from OPENCL to HOST memory"); return AMF_FAIL; }
	}
	else
	{
        for (amf_size id = 0; id < countOfComplexNumbers; id++)
		{
			amf_size j;
			float ar, ai, br, bi;
			j = (id << 1);
			ar = inputBuffer1[j];
			ai = inputBuffer1[j + 1];
			br = inputBuffer2[j];
			bi = inputBuffer2[j + 1];
			outputBuffer[j] = ar*br - ai*bi;
			outputBuffer[j + 1] = ar*bi + ai*br;
		}
	}

    return AMF_OK;
}
//------------------------------------------------------------------------------------------------
AMF_RESULT TANMathImpl::ComplexMultiplyAccumulate(
	const float inputBuffer1[], 
	const float inputBuffer2[], 
	float accumBuffer[], 
    amf_size countOfComplexNumbers)
{
	AMF_RETURN_IF_FALSE(inputBuffer1 != NULL, AMF_INVALID_ARG, L"inputBuffer1 == NULL");
	AMF_RETURN_IF_FALSE(inputBuffer2 != NULL, AMF_INVALID_ARG, L"inputBuffer2 == NULL");
	AMF_RETURN_IF_FALSE(accumBuffer != NULL, AMF_INVALID_ARG, L"outputBuffer == NULL");

    AMF_RETURN_IF_FALSE(countOfComplexNumbers > 0, AMF_INVALID_ARG, L"countOfComplexNumbers == 0");


	if (m_pContextTAN->GetOpenCLContext())
	{
		// Adjust internal buffer size
		cl_int clErr = 0;;
        amf_size requiredbuffersize = countOfComplexNumbers * sizeof(float)*2;
		AdjustInternalBufferSize(&m_pInternalBufferIn1_MulAccu, &m_iInternalBufferIn1Size_MulAccu, requiredbuffersize);
		AdjustInternalBufferSize(&m_pInternalBufferIn2_MulAccu, &m_iInternalBufferIn2Size_MulAccu, requiredbuffersize);
		AdjustInternalBufferSize(&m_pInternalBufferOut_MulAccu, &m_iInternalBufferOutSize_MulAccu, 2 * sizeof(float));
		cl_command_queue cmdQueue = m_pContextTAN->GetOpenCLGeneralQueue();
		clErr = clEnqueueWriteBuffer(cmdQueue, m_pInternalBufferIn1_MulAccu, CL_TRUE, 0, requiredbuffersize, inputBuffer1, 0, NULL, NULL);
		if (clErr != CL_SUCCESS){ printf("Failed to copy from HOST to OPENCL memory"); return AMF_FAIL; }
		clErr = clEnqueueWriteBuffer(cmdQueue, m_pInternalBufferIn2_MulAccu, CL_TRUE, 0, requiredbuffersize, inputBuffer2, 0, NULL, NULL);
		if (clErr != CL_SUCCESS){ printf("Failed to copy from HOST to OPENCL memory"); return AMF_FAIL; }
        ComplexMultiplyAccumulate(m_pInternalBufferIn1_MulAccu, 0, m_pInternalBufferIn2_MulAccu, 0, m_pInternalBufferOut_MulAccu, 0, countOfComplexNumbers);
		clErr = clEnqueueReadBuffer(cmdQueue, m_pInternalBufferOut_MulAccu, CL_TRUE, 0, 2 * sizeof(float), accumBuffer, 0, NULL, NULL);
		if (clErr != CL_SUCCESS){ printf("Failed to copy from OPENCL to HOST memory"); return AMF_FAIL; }
	}
	else
	{
        for (amf_size id = 0; id < countOfComplexNumbers; id++)
		{
			amf_size j;
			float ar, ai, br, bi;
			j = (id << 1);
			ar = inputBuffer1[j];
			ai = inputBuffer1[j + 1];
			br = inputBuffer2[j];
			bi = inputBuffer2[j + 1];
			accumBuffer[0] += (ar*br - ai*bi);
			accumBuffer[1] += (ar*bi + ai*br);
		}
	}
	return AMF_OK;
}

//-------------------------------------------------------------------------------------------------
AMF_RESULT TANMathImpl::ComplexMultiplication(
	const cl_mem inputBuffer1,
    const amf_size buffer1OffsetInSamples,
	const cl_mem inputBuffer2,
    const amf_size buffer2OffsetInSamples,
	cl_mem outputBuffer,
    const amf_size outputBufferOffsetInSamples,
    amf_size countOfComplexNumbers
)
{
    if (m_pKernelComplexMul == 0)
    {
        return AMF_OPENCL_FAILED;
    }

    AMF_RETURN_IF_FALSE(inputBuffer1 != NULL, AMF_INVALID_ARG, L"inputBuffer1 == NULL");
    AMF_RETURN_IF_FALSE(inputBuffer2 != NULL, AMF_INVALID_ARG, L"inputBuffer2 == NULL");
    AMF_RETURN_IF_FALSE(outputBuffer != NULL, AMF_INVALID_ARG, L"outputBuffer == NULL");
    AMF_RETURN_IF_FALSE(countOfComplexNumbers > 0, AMF_INVALID_ARG, L"countOfComplexNumbers == 0");
    AMF_RETURN_IF_FALSE(!(countOfComplexNumbers & 1), AMF_INVALID_ARG,
                        L"countOfComplexNumbers is odd, not supported by OpenCL kernel");

    AMFLock lock(&m_sect);

	cl_uint index = 0;

	cl_int clErr;
	// Set kernel arguments (additional arugments if needed)
//////    AMF_RETURN_IF_FAILED(clKernel->SetArgBuffer(index++, inputBuffer1));
	clErr = clSetKernelArg(m_pKernelComplexMul, index++, sizeof(cl_mem), &inputBuffer1);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
//////    AMF_RETURN_IF_FAILED(clKernel->SetArgInt64(index++, buffer1OffsetInSamples));
	clErr = clSetKernelArg(m_pKernelComplexMul, index++, sizeof(amf_int64), &buffer1OffsetInSamples);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
//////    AMF_RETURN_IF_FAILED(m_pKernelComplexMul->SetArgBuffer(index++, inputBuffer2));
	clErr = clSetKernelArg(m_pKernelComplexMul, index++, sizeof(cl_mem), &inputBuffer2);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
//////    AMF_RETURN_IF_FAILED(m_pKernelComplexMul->SetArgInt64(index++, buffer2OffsetInSamples));
	clErr = clSetKernelArg(m_pKernelComplexMul, index++, sizeof(amf_int64), &buffer2OffsetInSamples);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
//////    AMF_RETURN_IF_FAILED(m_pKernelComplexMul->SetArgBuffer(index++, outputBuffer));
	clErr = clSetKernelArg(m_pKernelComplexMul, index++, sizeof(cl_mem), &outputBuffer);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
//////    AMF_RETURN_IF_FAILED(m_pKernelComplexMul->SetArgInt64(index++, outputBufferOffsetInSamples));
	clErr = clSetKernelArg(m_pKernelComplexMul, index++, sizeof(amf_int64), &outputBufferOffsetInSamples);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
//////    AMF_RETURN_IF_FAILED(m_pKernelComplexMul->SetArgInt32(index++, static_cast<amf_int32>(numOfSamplesToProcess / 4)));
    amf_int32 numOfSamplesToProcessDivBy4 = static_cast<amf_int32>(countOfComplexNumbers / 2);  
	clErr = clSetKernelArg(m_pKernelComplexMul, index++, sizeof(amf_int32), &numOfSamplesToProcessDivBy4);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
	
	cl_command_queue cmdQueue = m_pContextTAN->GetOpenCLGeneralQueue();

    amf_size global[3] = { countOfComplexNumbers / 2, 0, 0 };
    amf_size local[3] = { 0, 0, 0 };
	clErr = clEnqueueNDRangeKernel(cmdQueue, m_pKernelComplexMul, 1, NULL, global, NULL, 0, NULL, NULL);
	if (clErr != CL_SUCCESS) { printf("Failed to enqueue OpenCL kernel\n"); return AMF_FAIL; }

    return AMF_OK;
}

AMF_RESULT TANMathImpl::ComplexMultiplyAccumulate(
	const cl_mem inputBuffer1, 
	const amf_size buffer1OffsetInSamples, 
	const cl_mem inputBuffer2, 
	const amf_size buffer2OffsetInSamples, 
	cl_mem accumBuffer, 
	const amf_size accumBufferOffsetInSamples, 
    amf_size countOfComplexNumbers)
{

	if (m_pKernelComplexSum == 0)
	{
		return AMF_OPENCL_FAILED;
	}

	amf_size oclWorkGroupSize = 64;
    amf_size numOfSamplesInVector4 = countOfComplexNumbers >> 1;

	AMF_RETURN_IF_FALSE(inputBuffer1 != NULL, AMF_INVALID_ARG, L"inputBuffer1 == NULL");
	AMF_RETURN_IF_FALSE(inputBuffer2 != NULL, AMF_INVALID_ARG, L"inputBuffer2 == NULL");
	AMF_RETURN_IF_FALSE(accumBuffer != NULL, AMF_INVALID_ARG, L"accumBuffer == NULL");
    AMF_RETURN_IF_FALSE(countOfComplexNumbers > 0, AMF_INVALID_ARG, L"countOfComplexNumbers == 0");
    AMF_RETURN_IF_FALSE(!(countOfComplexNumbers & 1), AMF_INVALID_ARG,
		L"countOfComplexNumbers is odd, not supported by OpenCL kernel");

	AMFLock lock(&m_sect);
	cl_int clErr;
	
	amf_size numofGroup = numOfSamplesInVector4 / oclWorkGroupSize;
	amf_size oclLocalBufferSize = sizeof(float) * 4 * oclWorkGroupSize;

	// Adjust internal Input buffer;
    amf_size requiredInternalBuffersize = countOfComplexNumbers * sizeof(float)*2;
	AdjustInternalBufferSize(&m_pInternalSwapBuffer1_MulAccu, &m_iInternalSwapBuffer1Size_MulAccu, requiredInternalBuffersize);
	AdjustInternalBufferSize(&m_pInternalSwapBuffer2_MulAccu, &m_iInternalSwapBuffer2Size_MulAccu, requiredInternalBuffersize);


    AMF_RESULT result = ComplexMultiplication(inputBuffer1, buffer1OffsetInSamples, inputBuffer2, buffer2OffsetInSamples, m_pInternalSwapBuffer1_MulAccu, 0, countOfComplexNumbers);
	if (result != AMF_OK) return result;

	// Calculate iteration
	amf_size numOfIteration = 0;

	cl_ulong aoffset = 0;
	cl_uint pInputFlt_arg_index = 0;
	cl_uint pAuxiliary_arg_index = 1;
	cl_uint pResultFlt_arg_index = 2;
	cl_uint rOffsetInFloats_arg_index = 3;
	cl_uint countInQuadFloats_arg_index = 4;
	
	cl_command_queue cmdQueue = m_pContextTAN->GetOpenCLGeneralQueue();
	// Set kernel arguments (additional arugments if needed)
	

	clErr = clSetKernelArg(m_pKernelComplexSum, pAuxiliary_arg_index, sizeof(float) * 4 * oclWorkGroupSize, NULL);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }

	clErr = clSetKernelArg(m_pKernelComplexSum, rOffsetInFloats_arg_index, sizeof(amf_int64), &aoffset);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }

	// GPU Summation
	cl_mem input_mem = nullptr;
	cl_mem output_mem = nullptr;
	amf_size threshold = oclWorkGroupSize;
	amf_size leftOver = numOfSamplesInVector4;
	for (;leftOver > threshold;)
	{
		if ((numOfIteration % 2) == 0)
		{
			input_mem = m_pInternalSwapBuffer1_MulAccu;
			output_mem = m_pInternalSwapBuffer2_MulAccu;
		}
		else
		{
			input_mem = m_pInternalSwapBuffer2_MulAccu;
			output_mem = m_pInternalSwapBuffer1_MulAccu;
		}
		cl_float filled = 0.0;
		clErr = clEnqueueFillBuffer(cmdQueue, output_mem, &filled, sizeof(cl_float), 0, requiredInternalBuffersize, 0, NULL, NULL);
		if (clErr != CL_SUCCESS) { printf("Failed to zero output buffer"); return AMF_FAIL; }
		if (clErr != CL_SUCCESS) { printf("Faield to wait for ocl event"); return AMF_FAIL; }
		clErr = clSetKernelArg(m_pKernelComplexSum, pInputFlt_arg_index, sizeof(amf_int64), &input_mem);
		if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
		clErr = clSetKernelArg(m_pKernelComplexSum, pResultFlt_arg_index, sizeof(amf_int64), &output_mem);
		if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
		clErr = clSetKernelArg(m_pKernelComplexSum, countInQuadFloats_arg_index, sizeof(amf_int64), &leftOver);
		if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
		amf_size global[3] = { leftOver,0, 0 };
		clErr = clEnqueueNDRangeKernel(cmdQueue, m_pKernelComplexSum, 1, NULL, global, NULL, 0, NULL, NULL);
		if (clErr != CL_SUCCESS) { printf("Failed to enqueue OpenCL kernel\n"); return AMF_FAIL; }
		numOfIteration++;
		leftOver = (leftOver + oclWorkGroupSize - 1) / oclWorkGroupSize / 2;
	}
	
	// CPU Summation
	cl_mem resultBuffferOCL = nullptr;
	numOfIteration % 2 == 0 ? resultBuffferOCL = m_pInternalSwapBuffer1_MulAccu : resultBuffferOCL = m_pInternalSwapBuffer2_MulAccu;

	float* resultBufferHost = (float*)clEnqueueMapBuffer(cmdQueue, resultBuffferOCL, CL_TRUE, CL_MAP_READ | CL_MAP_WRITE, 0, leftOver * 4 * sizeof(float), 0, NULL,NULL,&clErr);
	//ocl_queue.push_back(oclevent);
	if (clErr != CL_SUCCESS) { printf("Faield to map OCL Buffer"); return AMF_FAIL; }
	float* outputbufferhost = (float*)clEnqueueMapBuffer(cmdQueue, accumBuffer, CL_TRUE, CL_MAP_READ | CL_MAP_WRITE, (accumBufferOffsetInSamples) * sizeof(float), 2 * sizeof(float), 0, NULL, NULL, &clErr);
	if (clErr != CL_SUCCESS) { printf("Faield to map OCL Buffer"); return AMF_FAIL; }
	for (amf_size index = 0; index < leftOver*4; index++)
	{
		amf_size temp = index & 1;
		float a = resultBufferHost[index];
		float b = outputbufferhost[temp];
		outputbufferhost[temp] += resultBufferHost[index];
	}
	clErr = clEnqueueUnmapMemObject(cmdQueue, resultBuffferOCL, resultBufferHost, 0,NULL,NULL);
	if (clErr != CL_SUCCESS) { printf("Faield to unmap OCL Buffer"); return AMF_FAIL; }
	clErr = clEnqueueUnmapMemObject(cmdQueue, accumBuffer, outputbufferhost, 0,NULL,NULL);
	if (clErr != CL_SUCCESS) { printf("Faield to map OCL Buffer"); return AMF_FAIL; }
	return AMF_OK;
}

//-------------------------------------------------------------------------------------------------
AMF_RESULT TANMathImpl::ComplexDivision(
    const float inputBuffer1[],
    const float inputBuffer2[],
    float outputBuffer[],
    amf_size countOfComplexNumbers
)
{
    AMF_RETURN_IF_FALSE(inputBuffer1 != NULL, AMF_INVALID_ARG, L"inputBuffer1 == NULL");
    AMF_RETURN_IF_FALSE(inputBuffer2 != NULL, AMF_INVALID_ARG, L"inputBuffer2 == NULL");
    AMF_RETURN_IF_FALSE(outputBuffer != NULL, AMF_INVALID_ARG, L"outputBuffer == NULL");

    AMF_RETURN_IF_FALSE(countOfComplexNumbers > 0, AMF_INVALID_ARG, L"countOfComplexNumbers == 0");

	if (m_pContextTAN->GetOpenCLContext())
	{
		// Adjust internal buffer size
		cl_int clErr = 0;;
        amf_size requiredbuffersize = countOfComplexNumbers * sizeof(float) * 2;
		AdjustInternalBufferSize(&m_pInternalBufferOut_Division, &m_iInternalBufferOutSize_Division, requiredbuffersize);
		AdjustInternalBufferSize(&m_pInternalBufferIn1_Division, &m_iInternalBufferIn1Size_Division, requiredbuffersize);
		AdjustInternalBufferSize(&m_pInternalBufferIn2_Division, &m_iInternalBufferIn2Size_Division, requiredbuffersize);
		cl_command_queue cmdQueue = m_pContextTAN->GetOpenCLGeneralQueue();
		clErr = clEnqueueWriteBuffer(cmdQueue, m_pInternalBufferIn1_Division, CL_TRUE, 0, requiredbuffersize, inputBuffer1, 0, NULL, NULL);
		if (clErr != CL_SUCCESS){ printf("Failed to copy from HOST to OPENCL memory"); return AMF_FAIL; }
		clErr = clEnqueueWriteBuffer(cmdQueue, m_pInternalBufferIn2_Division, CL_TRUE, 0, requiredbuffersize, inputBuffer2, 0, NULL, NULL);
		if (clErr != CL_SUCCESS){ printf("Failed to copy from HOST to OPENCL memory"); return AMF_FAIL; }
        ComplexDivision(m_pInternalBufferIn1_Division, 0, m_pInternalBufferIn2_Division, 0, m_pInternalBufferOut_Division, 0, countOfComplexNumbers);
		clErr = clEnqueueReadBuffer(cmdQueue, m_pInternalBufferOut_Division, CL_TRUE, 0, requiredbuffersize, outputBuffer, 0, NULL, NULL);
		if (clErr != CL_SUCCESS){ printf("Failed to copy from OPENCL to HOST memory"); return AMF_FAIL; }
	}
	else
	{
        for (amf_size id = 0; id < countOfComplexNumbers; id++)
		{
			amf_size j;
			float ar, ai, br, bi;
			j = (id << 1);
			ar = inputBuffer1[j];
			ai = inputBuffer1[j + 1];
			br = inputBuffer2[j];
			bi = inputBuffer2[j + 1];
			float d = br*br + bi*bi;
			outputBuffer[j] = (ar*br + ai*bi) / d;
			outputBuffer[j + 1] = (-ar*bi + ai*br) / d;
		}
	}
    return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT TANMathImpl::ComplexDivision(
	const cl_mem inputBuffer1,
    const amf_size buffer1OffsetInSamples,
	const cl_mem inputBuffer2,
    const amf_size buffer2OffsetInSamples,
	cl_mem outputBuffer,
    const amf_size outputBufferOffsetInSamples,
    amf_size countOfComplexNumbers
)
{
    AMF_RETURN_IF_FALSE(inputBuffer1 != NULL, AMF_INVALID_ARG, L"inputBuffer1 == NULL");
    AMF_RETURN_IF_FALSE(inputBuffer2 != NULL, AMF_INVALID_ARG, L"inputBuffer2 == NULL");
    AMF_RETURN_IF_FALSE(outputBuffer != NULL, AMF_INVALID_ARG, L"outputBuffer == NULL");

    AMF_RETURN_IF_FALSE(countOfComplexNumbers > 0, AMF_INVALID_ARG, L"countOfComplexNumbers == 0");
    AMF_RETURN_IF_FALSE(!(countOfComplexNumbers & 1), AMF_INVALID_ARG,
                        L"countOfComplexNumbers is odd, not supported by OpenCL kernel");

    AMFLock lock(&m_sect);

	cl_uint index = 0;

	cl_int clErr;
	
//////    AMF_RETURN_IF_FAILED(m_pKernelComplexDiv->SetArgBuffer(index++, inputBuffer1));
	clErr = clSetKernelArg(m_pKernelComplexDiv, index++, sizeof(cl_mem), &inputBuffer1);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
//////    AMF_RETURN_IF_FAILED(m_pKernelComplexDiv->SetArgInt64(index++, buffer1OffsetInSamples));
	clErr = clSetKernelArg(m_pKernelComplexDiv, index++, sizeof(amf_int64), &buffer1OffsetInSamples);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
//////    AMF_RETURN_IF_FAILED(m_pKernelComplexDiv->SetArgBuffer(index++, inputBuffer2));
	clErr = clSetKernelArg(m_pKernelComplexDiv, index++, sizeof(cl_mem), &inputBuffer2);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
//////    AMF_RETURN_IF_FAILED(m_pKernelComplexDiv->SetArgInt64(index++, buffer2OffsetInSamples));
	clErr = clSetKernelArg(m_pKernelComplexDiv, index++, sizeof(amf_int64), &buffer2OffsetInSamples);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
//////    AMF_RETURN_IF_FAILED(m_pKernelComplexDiv->SetArgBuffer(index++, outputBuffer));
	clErr = clSetKernelArg(m_pKernelComplexDiv, index++, sizeof(cl_mem), &outputBuffer);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
//////    AMF_RETURN_IF_FAILED(m_pKernelComplexDiv->SetArgInt64(index++, outputBufferOffsetInSamples));
	clErr = clSetKernelArg(m_pKernelComplexDiv, index++, sizeof(amf_int64), &outputBufferOffsetInSamples);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
//////    AMF_RETURN_IF_FAILED(m_pKernelComplexDiv->SetArgInt32(index++, static_cast<amf_int32>(numOfSamplesToProcess / 4)));
    amf_int32 numOfSamplesToProcessDivBy4 = static_cast<amf_int32>(countOfComplexNumbers / 2);
	clErr = clSetKernelArg(m_pKernelComplexDiv, index++, sizeof(amf_int32), &numOfSamplesToProcessDivBy4);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }

	cl_command_queue cmdQueue = m_pContextTAN->GetOpenCLGeneralQueue();

    amf_size global[3] = { countOfComplexNumbers / 2, 0, 0 };
    amf_size local[3] = { 0, 0, 0 };
	clErr = clEnqueueNDRangeKernel(cmdQueue, m_pKernelComplexDiv, 1, NULL, global, NULL, 0, NULL, NULL);
	if (clErr != CL_SUCCESS) { printf("Failed to enqueue OpenCL kernel\n"); return AMF_FAIL; }

    return AMF_OK;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
