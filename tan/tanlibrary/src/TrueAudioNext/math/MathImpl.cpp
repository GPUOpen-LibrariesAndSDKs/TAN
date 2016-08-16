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

#include "VectorComplexDivision.cl.h"
#include "VectorComplexMultiply.cl.h"

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
    m_gpuMultiplicationRunNum(0)
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

	// for now register source kernels
	if (m_pKernelComplexDiv == 0)
	{
		AMF_KERNEL_ID kernelId;
		if (pPrograms)
		{
			AMF_RETURN_IF_FAILED(pPrograms->RegisterKernelSource(
				&kernelId, L"VectorComplexDivision", "VectorComplexDiv", VectorComplexDivisionCount,
				VectorComplexDivision, NULL));
		}


		AMF_RETURN_IF_FAILED(m_pDeviceCompute->GetKernel(kernelId, &m_pKernelComplexDiv),
			L"GetKernel() failed");
	}
	if (m_pKernelComplexMul == 0)
	{
		AMF_KERNEL_ID kernelId;
		if (pPrograms)
		{
			AMF_RETURN_IF_FAILED(pPrograms->RegisterKernelSource(
				&kernelId, L"VectorComplexMul", "VectorComplexMul", VectorComplexMultiplyCount,
				VectorComplexMultiply, NULL));
		}

		AMF_RETURN_IF_FAILED(m_pDeviceCompute->GetKernel(kernelId, &m_pKernelComplexMul),
			L"GetKernel() failed");
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
	amf_size numOfSamplesToProcess
	)
{
	AMF_RETURN_IF_FALSE(inputBuffers1 != NULL, AMF_INVALID_ARG, L"inputBuffer1 == NULL");
	AMF_RETURN_IF_FALSE(inputBuffers2 != NULL, AMF_INVALID_ARG, L"inputBuffer2 == NULL");
	AMF_RETURN_IF_FALSE(outputBuffers != NULL, AMF_INVALID_ARG, L"outputBuffer == NULL");
	AMF_RETURN_IF_FALSE(channels > 0, AMF_INVALID_ARG, L"channels == 0");
	AMF_RETURN_IF_FALSE(numOfSamplesToProcess > 0, AMF_INVALID_ARG, L"numOfSamplesToProcess == 0");

	for (amf_size channelId = 0; channelId < channels; channelId++)
	{
		AMF_RETURN_IF_FALSE(inputBuffers1[channelId] != NULL, AMF_INVALID_ARG, L"inputBuffers1[%u] == NULL", channelId);
		AMF_RETURN_IF_FALSE(inputBuffers2[channelId] != NULL, AMF_INVALID_ARG, L"inputBuffers2[%u] == NULL", channelId);
		AMF_RETURN_IF_FALSE(outputBuffers[channelId] != NULL, AMF_INVALID_ARG, L"outputBuffers[%u] == NULL", channelId);

		AMF_RESULT result = AMF_FAIL;
		result = ComplexMultiplication(inputBuffers1[channelId], inputBuffers2[channelId], outputBuffers[channelId], numOfSamplesToProcess);
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
	amf_size numOfSamplesToProcess
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
	AMF_RETURN_IF_FALSE(numOfSamplesToProcess > 0, AMF_INVALID_ARG, L"numOfSamplesToProcess == 0");
	AMF_RETURN_IF_FALSE(!(numOfSamplesToProcess & 1), AMF_INVALID_ARG, L"numOfSamplesToProcess is odd while the input is expected to be complex values");
	AMF_RETURN_IF_FALSE(!(numOfSamplesToProcess & 3), AMF_NOT_SUPPORTED, L"numOfSamplesToProcess is not multiple of 4, not supported by OpenCL kernel");

	for (amf_size channelId = 0; channelId < channels; channelId++)
	{
		AMF_RETURN_IF_FALSE(inputBuffers1[channelId] != NULL, AMF_INVALID_ARG, L"inputBuffers1[%u] == NULL", channelId);
		AMF_RETURN_IF_FALSE(inputBuffers2[channelId] != NULL, AMF_INVALID_ARG, L"inputBuffers2[%u] == NULL", channelId);
		AMF_RETURN_IF_FALSE(outputBuffers[channelId] != NULL, AMF_INVALID_ARG, L"outputBuffers[%u] == NULL", channelId);

		AMF_RESULT result = AMF_FAIL;
		result = ComplexMultiplication(	inputBuffers1[channelId], buffers1OffsetInSamples[channelId],
										inputBuffers2[channelId], buffers2OffsetInSamples[channelId],
										outputBuffers[channelId], outputBuffersOffsetInSamples[channelId],
										numOfSamplesToProcess);
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
	amf_size numOfSamplesToProcess
	)
{
	AMF_RETURN_IF_FALSE(inputBuffers1 != NULL, AMF_INVALID_ARG, L"inputBuffer1 == NULL");
	AMF_RETURN_IF_FALSE(inputBuffers2 != NULL, AMF_INVALID_ARG, L"inputBuffer2 == NULL");
	AMF_RETURN_IF_FALSE(outputBuffers != NULL, AMF_INVALID_ARG, L"outputBuffer == NULL");
	AMF_RETURN_IF_FALSE(channels > 0, AMF_INVALID_ARG, L"channels == 0");
	AMF_RETURN_IF_FALSE(numOfSamplesToProcess > 0, AMF_INVALID_ARG, L"numOfSamplesToProcess == 0");

	for (amf_size channelId = 0; channelId < channels; channelId++)
	{
		AMF_RETURN_IF_FALSE(inputBuffers1[channelId] != NULL, AMF_INVALID_ARG, L"inputBuffers1[%u] == NULL", channelId);
		AMF_RETURN_IF_FALSE(inputBuffers2[channelId] != NULL, AMF_INVALID_ARG, L"inputBuffers2[%u] == NULL", channelId);
		AMF_RETURN_IF_FALSE(outputBuffers[channelId] != NULL, AMF_INVALID_ARG, L"outputBuffers[%u] == NULL", channelId);
		AMF_RESULT result = AMF_FAIL;
		result = ComplexDivision(inputBuffers1[channelId], inputBuffers2[channelId], outputBuffers[channelId], numOfSamplesToProcess);
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
	amf_size numOfSamplesToProcess
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
	AMF_RETURN_IF_FALSE(numOfSamplesToProcess > 0, AMF_INVALID_ARG, L"numOfSamplesToProcess == 0");
	AMF_RETURN_IF_FALSE(!(numOfSamplesToProcess & 1), AMF_INVALID_ARG,
		L"numOfSamplesToProcess is odd while the input is expected to be complex values");
	AMF_RETURN_IF_FALSE(!(numOfSamplesToProcess & 3), AMF_NOT_SUPPORTED,
		L"numOfSamplesToProcess is not multiple of 4, not supported by OpenCL kernel");

	for (amf_size channelId = 0; channelId < channels; channelId++)
	{
		AMF_RETURN_IF_FALSE(inputBuffers1[channelId] != NULL, AMF_INVALID_ARG, L"inputBuffers1[%u] == NULL", channelId);
		AMF_RETURN_IF_FALSE(inputBuffers2[channelId] != NULL, AMF_INVALID_ARG, L"inputBuffers2[%u] == NULL", channelId);
		AMF_RETURN_IF_FALSE(outputBuffers[channelId] != NULL, AMF_INVALID_ARG, L"outputBuffers[%u] == NULL", channelId);
		AMF_RESULT result = AMF_FAIL;
		result = ComplexDivision(	inputBuffers1[channelId], buffers1OffsetInSamples[channelId],
									inputBuffers2[channelId], buffers2OffsetInSamples[channelId],
									outputBuffers[channelId], outputBuffersOffsetInSamples[channelId],
									numOfSamplesToProcess);
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
    amf_size numOfSamplesToProcess
)
{
    AMF_RETURN_IF_FALSE(inputBuffer1 != NULL, AMF_INVALID_ARG, L"inputBuffer1 == NULL");
    AMF_RETURN_IF_FALSE(inputBuffer2 != NULL, AMF_INVALID_ARG, L"inputBuffer2 == NULL");
    AMF_RETURN_IF_FALSE(outputBuffer != NULL, AMF_INVALID_ARG, L"outputBuffer == NULL");

    AMF_RETURN_IF_FALSE(numOfSamplesToProcess > 0, AMF_INVALID_ARG, L"numOfSamplesToProcess == 0");
    AMF_RETURN_IF_FALSE(!(numOfSamplesToProcess & 1), AMF_INVALID_ARG,
                        L"numOfSamplesToProcess is odd while the input is expected to be complex values");

    for (amf_size id = 0; id < numOfSamplesToProcess / 2; id++)
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
    amf_size numOfSamplesToProcess
)
{
    if (m_pKernelComplexMul == 0)
    {
        return AMF_OPENCL_FAILED;
    }

    AMF_RETURN_IF_FALSE(inputBuffer1 != NULL, AMF_INVALID_ARG, L"inputBuffer1 == NULL");
    AMF_RETURN_IF_FALSE(inputBuffer2 != NULL, AMF_INVALID_ARG, L"inputBuffer2 == NULL");
    AMF_RETURN_IF_FALSE(outputBuffer != NULL, AMF_INVALID_ARG, L"outputBuffer == NULL");
    AMF_RETURN_IF_FALSE(numOfSamplesToProcess > 0, AMF_INVALID_ARG, L"numOfSamplesToProcess == 0");
    AMF_RETURN_IF_FALSE(!(numOfSamplesToProcess & 1), AMF_INVALID_ARG,
                        L"numOfSamplesToProcess is odd while the input is expected to be complex values");
    AMF_RETURN_IF_FALSE(!(numOfSamplesToProcess & 3), AMF_NOT_SUPPORTED,
                        L"numOfSamplesToProcess is not multiple of 4, not supported by OpenCL kernel");

    AMFLock lock(&m_sect);

	cl_uint index = 0;

	cl_int clErr;
	cl_kernel clKernel = (cl_kernel)m_pKernelComplexMul->GetNative();
	// Set kernel arguments (additional arugments if needed)
//////    AMF_RETURN_IF_FAILED(clKernel->SetArgBuffer(index++, inputBuffer1));
	clErr = clSetKernelArg(clKernel, index++, sizeof(cl_mem), &inputBuffer1);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
//////    AMF_RETURN_IF_FAILED(clKernel->SetArgInt64(index++, buffer1OffsetInSamples));
	clErr = clSetKernelArg(clKernel, index++, sizeof(amf_int64), &buffer1OffsetInSamples);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
//////    AMF_RETURN_IF_FAILED(m_pKernelComplexMul->SetArgBuffer(index++, inputBuffer2));
	clErr = clSetKernelArg(clKernel, index++, sizeof(cl_mem), &inputBuffer2);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
//////    AMF_RETURN_IF_FAILED(m_pKernelComplexMul->SetArgInt64(index++, buffer2OffsetInSamples));
	clErr = clSetKernelArg(clKernel, index++, sizeof(amf_int64), &buffer2OffsetInSamples);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
//////    AMF_RETURN_IF_FAILED(m_pKernelComplexMul->SetArgBuffer(index++, outputBuffer));
	clErr = clSetKernelArg(clKernel, index++, sizeof(cl_mem), &outputBuffer);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
//////    AMF_RETURN_IF_FAILED(m_pKernelComplexMul->SetArgInt64(index++, outputBufferOffsetInSamples));
	clErr = clSetKernelArg(clKernel, index++, sizeof(amf_int64), &outputBufferOffsetInSamples);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
//////    AMF_RETURN_IF_FAILED(m_pKernelComplexMul->SetArgInt32(index++, static_cast<amf_int32>(numOfSamplesToProcess / 4)));
	amf_int32 numOfSamplesToProcessDivBy4 = static_cast<amf_int32>(numOfSamplesToProcess / 4);
	clErr = clSetKernelArg(clKernel, index++, sizeof(amf_int32), &numOfSamplesToProcessDivBy4);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
	
	cl_command_queue cmdQueue = m_pContextTAN->GetOpenCLGeneralQueue();

	amf_size global[3] = { numOfSamplesToProcess / 4, 0, 0 };
    amf_size local[3] = { 0, 0, 0 };
	clErr = clEnqueueNDRangeKernel( cmdQueue, clKernel, 1, NULL, global, NULL, 0, NULL, NULL);
	if (clErr != CL_SUCCESS) { printf("Failed to enqueue OpenCL kernel\n"); return AMF_FAIL; }

    return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT TANMathImpl::ComplexDivision(
    const float inputBuffer1[],
    const float inputBuffer2[],
    float outputBuffer[],
    amf_size numOfSamplesToProcess
)
{
    AMF_RETURN_IF_FALSE(inputBuffer1 != NULL, AMF_INVALID_ARG, L"inputBuffer1 == NULL");
    AMF_RETURN_IF_FALSE(inputBuffer2 != NULL, AMF_INVALID_ARG, L"inputBuffer2 == NULL");
    AMF_RETURN_IF_FALSE(outputBuffer != NULL, AMF_INVALID_ARG, L"outputBuffer == NULL");

    AMF_RETURN_IF_FALSE(numOfSamplesToProcess > 0, AMF_INVALID_ARG, L"numOfSamplesToProcess == 0");
    AMF_RETURN_IF_FALSE(!(numOfSamplesToProcess & 1), AMF_INVALID_ARG,
                        L"numOfSamplesToProcess is odd while the input is expected to be complex values");

    for (amf_size id = 0; id < numOfSamplesToProcess / 2; id++)
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
    amf_size numOfSamplesToProcess
)
{
    AMF_RETURN_IF_FALSE(inputBuffer1 != NULL, AMF_INVALID_ARG, L"inputBuffer1 == NULL");
    AMF_RETURN_IF_FALSE(inputBuffer2 != NULL, AMF_INVALID_ARG, L"inputBuffer2 == NULL");
    AMF_RETURN_IF_FALSE(outputBuffer != NULL, AMF_INVALID_ARG, L"outputBuffer == NULL");

    AMF_RETURN_IF_FALSE(numOfSamplesToProcess > 0, AMF_INVALID_ARG, L"numOfSamplesToProcess == 0");
    AMF_RETURN_IF_FALSE(!(numOfSamplesToProcess & 1), AMF_INVALID_ARG,
                        L"numOfSamplesToProcess is odd while the input is expected to be complex values");
    AMF_RETURN_IF_FALSE(!(numOfSamplesToProcess & 3), AMF_NOT_SUPPORTED,
                        L"numOfSamplesToProcess is not multiple of 4, not supported by OpenCL kernel");

    AMFLock lock(&m_sect);

	cl_uint index = 0;

	cl_int clErr;
	cl_kernel clKernel = (cl_kernel)m_pKernelComplexDiv->GetNative();
	
//////    AMF_RETURN_IF_FAILED(m_pKernelComplexDiv->SetArgBuffer(index++, inputBuffer1));
	clErr = clSetKernelArg(clKernel, index++, sizeof(cl_mem), &inputBuffer1);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
//////    AMF_RETURN_IF_FAILED(m_pKernelComplexDiv->SetArgInt64(index++, buffer1OffsetInSamples));
	clErr = clSetKernelArg(clKernel, index++, sizeof(amf_int64), &buffer1OffsetInSamples);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
//////    AMF_RETURN_IF_FAILED(m_pKernelComplexDiv->SetArgBuffer(index++, inputBuffer2));
	clErr = clSetKernelArg(clKernel, index++, sizeof(cl_mem), &inputBuffer2);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
//////    AMF_RETURN_IF_FAILED(m_pKernelComplexDiv->SetArgInt64(index++, buffer2OffsetInSamples));
	clErr = clSetKernelArg(clKernel, index++, sizeof(amf_int64), &buffer2OffsetInSamples);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
//////    AMF_RETURN_IF_FAILED(m_pKernelComplexDiv->SetArgBuffer(index++, outputBuffer));
	clErr = clSetKernelArg(clKernel, index++, sizeof(cl_mem), &outputBuffer);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
//////    AMF_RETURN_IF_FAILED(m_pKernelComplexDiv->SetArgInt64(index++, outputBufferOffsetInSamples));
	clErr = clSetKernelArg(clKernel, index++, sizeof(amf_int64), &outputBufferOffsetInSamples);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
//////    AMF_RETURN_IF_FAILED(m_pKernelComplexDiv->SetArgInt32(index++, static_cast<amf_int32>(numOfSamplesToProcess / 4)));
	amf_int32 numOfSamplesToProcessDivBy4 = static_cast<amf_int32>(numOfSamplesToProcess / 4);
	clErr = clSetKernelArg(clKernel, index++, sizeof(amf_int32), &numOfSamplesToProcessDivBy4);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }

	cl_command_queue cmdQueue = m_pContextTAN->GetOpenCLGeneralQueue();

	amf_size global[3] = { numOfSamplesToProcess / 4, 0, 0 };
    amf_size local[3] = { 0, 0, 0 };
	clErr = clEnqueueNDRangeKernel(cmdQueue, clKernel, 1, NULL, global, NULL, 0, NULL, NULL);
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
