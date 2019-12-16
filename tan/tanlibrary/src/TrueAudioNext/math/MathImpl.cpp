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
#include "CLKernel_VectorComplexMultiplyAccumulate.h"

//#include "VectorComplexDivision.cl.h"
//#include "VectorComplexMultiply.cl.h"
//#include "VectorComplexSum.cl.h"
#include "../../common/cpucaps.h"
#include <immintrin.h>

#include <memory>
#include <omp.h>

#define AMF_FACILITY L"TANMathImpl"

using namespace amf;

//const InstructionSet::InstructionSet_Internal InstructionSet::CPU_Rep;
bool TANMathImpl::useAVX256 = true; // InstructionSet::AVX() && InstructionSet::FMA();
bool TANMathImpl::useAVX512 = true; // InstructionSet::AVX512F() && InstructionSet::FMA();

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
    *ppComponent = new TANMathImpl( pContext, false );
    (*ppComponent)->Acquire();
    return AMF_OK;
}

AMF_RESULT  amf::TANCreateMath(
	amf::TANContext* pContext,
	amf::TANMath** ppComponent,
	bool useConvolutionQueue)
{
	TANContextImplPtr contextImpl(pContext);
	*ppComponent = new TANMathImpl(pContext, useConvolutionQueue);
	(*ppComponent)->Acquire();
	return AMF_OK;
}

//-------------------------------------------------------------------------------------------------
TANMathImpl::TANMathImpl(TANContext *pContextTAN, bool useConvQueue) :
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
	m_pInternalBufferIn2_Multiply(nullptr),
    m_pKernelComplexMul(nullptr),
    m_pKernelComplexMulAccum(nullptr)

{
	m_useConvQueue = useConvQueue;
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
	// ToDo use common function with FFTimpl, maybe?
	// setup OpenMP, if enabled
	int numThreads = 0;
	int numProcs = 1;
	numProcs = omp_get_num_procs();
	numThreads = numProcs / 4;
	if (numThreads == 0)
		numThreads = 1;
	omp_set_num_threads(numThreads);
	//numThreads = omp_get_max_threads();

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
	//m_pDeviceCompute = contextImpl->GetGeneralCompute();
	// ToDo needs to be user selectable
	m_pDeviceCompute = contextImpl->GetConvolutionCompute();

	if (NULL == m_pDeviceCompute)
	{
		return AMF_OK;
	}

	if (m_useConvQueue) {
		m_clQueue = m_pContextTAN->GetOpenCLConvQueue();
	}
	else {
		m_clQueue = m_pContextTAN->GetOpenCLGeneralQueue();
	}

	bool OCLKernel_Err = false;
	// for now register source kernels
	if (m_pKernelComplexDiv == nullptr)
	{	
		OCLKernel_Err = GetOclKernel(m_pKernelComplexDiv, m_pDeviceCompute, m_clQueue, "VectorComplexDivision", VectorComplexDivision_Str, VectorComplexDivisionCount,
			"VectorComplexDiv", "");
		if (!OCLKernel_Err){ printf("Failed to initialize Kernel\n"); return AMF_FAIL;}
	}
	if (m_pKernelComplexMul == 0)
	{
		OCLKernel_Err = GetOclKernel(m_pKernelComplexMul, m_pDeviceCompute, m_clQueue, "VectorComplexMul", VectorComplexMultiply_Str, VectorComplexMultiplyCount,
			"VectorComplexMul", "");
		if (!OCLKernel_Err){ printf("Failed to initialize Kernel\n"); return AMF_FAIL; }
	}
	if (m_pKernelComplexSum == 0)
	{
		OCLKernel_Err = GetOclKernel(m_pKernelComplexSum, m_pDeviceCompute, m_clQueue, "VectorComplexSum", VectorComplexSum_Str, VectorComplexSumCount,
			"VectorComplexSum", "");
		if (!OCLKernel_Err){ printf("Failed to initialize Kernel\n"); return AMF_FAIL; }
	}
    if (m_pKernelComplexMulAccum == 0)
    {
        OCLKernel_Err = GetOclKernel(m_pKernelComplexMulAccum, m_pDeviceCompute, m_clQueue, "VectorComplexMulAccum", VectorComplexMultiplyAccumulate_Str, VectorComplexMultiplyAccumulateCount,
            "VectorComplexMulAccum", "");
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
		clErr = clEnqueueFillBuffer(m_clQueue, m_pInternalBufferIn1_Multiply, &filled, sizeof(cl_float), 0, *size, 0, NULL, NULL);
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

	//for (amf_size channelId = 0; channelId < channels; channelId++)
	//{
	//	AMF_RETURN_IF_FALSE(inputBuffers1[channelId] != NULL, AMF_INVALID_ARG, L"inputBuffers1[%u] == NULL", channelId);
	//	AMF_RETURN_IF_FALSE(inputBuffers2[channelId] != NULL, AMF_INVALID_ARG, L"inputBuffers2[%u] == NULL", channelId);
	//	AMF_RETURN_IF_FALSE(accumbuffers[channelId] != NULL, AMF_INVALID_ARG, L"outputBuffers[%u] == NULL", channelId);

	//	AMF_RESULT result = AMF_FAIL;
	//	result = ComplexMultiplyAccumulate(
 //           inputBuffers1[channelId], inputBuffers2[channelId], accumbuffers[channelId], countOfComplexNumbers);
	//	AMF_RETURN_IF_FAILED(result);
	//}
	if (m_pContextTAN->GetOpenCLContext())
	{
		for (amf_size channelId = 0; channelId < channels; channelId++)
		{
			// Adjust internal buffer size
			cl_int clErr = 0;;
			amf_size requiredbuffersize = countOfComplexNumbers * sizeof(float) * 2;
			AdjustInternalBufferSize(&m_pInternalBufferIn1_MulAccu, &m_iInternalBufferIn1Size_MulAccu, requiredbuffersize);
			AdjustInternalBufferSize(&m_pInternalBufferIn2_MulAccu, &m_iInternalBufferIn2Size_MulAccu, requiredbuffersize);
			AdjustInternalBufferSize(&m_pInternalBufferOut_MulAccu, &m_iInternalBufferOutSize_MulAccu, requiredbuffersize);

			clErr = clEnqueueWriteBuffer(m_clQueue, m_pInternalBufferIn1_MulAccu, CL_TRUE, 0, requiredbuffersize, inputBuffers1[channelId], 0, NULL, NULL);
			if (clErr != CL_SUCCESS) { printf("Failed to copy from HOST to OPENCL memory"); return AMF_FAIL; }
			clErr = clEnqueueWriteBuffer(m_clQueue, m_pInternalBufferIn2_MulAccu, CL_TRUE, 0, requiredbuffersize, inputBuffers2[channelId], 0, NULL, NULL);
			if (clErr != CL_SUCCESS) { printf("Failed to copy from HOST to OPENCL memory"); return AMF_FAIL; }
			clErr = clEnqueueWriteBuffer(m_clQueue, m_pInternalBufferOut_MulAccu, CL_TRUE, 0, requiredbuffersize, accumbuffers[channelId], 0, NULL, NULL);
			if (clErr != CL_SUCCESS) { printf("Failed to copy from HOST to OPENCL memory"); return AMF_FAIL; }


			ComplexMultiplyAccumulate(m_pInternalBufferIn1_MulAccu, m_pInternalBufferIn2_MulAccu,  m_pInternalBufferOut_MulAccu, 1, countOfComplexNumbers, 0,0,0,0);

			clErr = clEnqueueReadBuffer(m_clQueue, m_pInternalBufferOut_MulAccu, CL_TRUE, 0, requiredbuffersize, accumbuffers[channelId], 0, NULL, NULL);
			if (clErr != CL_SUCCESS) { printf("Failed to copy from OPENCL to HOST memory"); return AMF_FAIL; }
		}
	}
	else
	{
		if (useAVX256) {
			// use AVX and FMA intrinsics to process 8 samples per pass
			register __m256 arReg, aiReg, brReg, biReg, crReg, ciReg, zero;
			register __m128 t1Reg, t2Reg;
			_mm256_zeroall();
			///tmp = _mm256_set1_ps(hf);

			float testa[16];
			//float testb[16];
			//int idxR[16];
			//int idxI[16];
			//for (int k = 0; k < 16; k++) {
			//	idxR[k] = 4 * (2 * k);
			//	idxI[k] = 4 * (2 * k + 1);
			//}
			
			int channelId = 0;
			int id = 0;
			int k = 0;
			// omp fail ...
//#pragma omp parallel default(none) shared(channels,inputBuffers1,inputBuffers2)
//#pragma omp for private(channelId,id,k) schedule(static) // schedule(guided) nowait //schedule(static) 
			for (channelId = 0; channelId < channels; channelId++)
			{
				const float *pIn1 = inputBuffers1[channelId];
				const float *pIn2 = inputBuffers2[channelId];
				float *pAcc = accumbuffers[channelId];

				for ( id = 0; id < countOfComplexNumbers; id += 8)
				{
					for (int k = 0; k < 8; k++) {
						((float *)&arReg)[k] = pIn1[2 * k];
						((float *)&aiReg)[k] = pIn1[2 * k + 1];
						((float *)&brReg)[k] = pIn2[2 * k];
						((float *)&biReg)[k] = pIn2[2 * k + 1];
						((float *)&crReg)[k] = pAcc[2 * k];
						((float *)&ciReg)[k] = pAcc[2 * k + 1];
					}

					crReg = _mm256_add_ps(_mm256_fmsub_ps(arReg, brReg, _mm256_mul_ps(aiReg, biReg)), crReg);
					ciReg = _mm256_add_ps(_mm256_fmadd_ps(arReg, biReg, _mm256_mul_ps(aiReg, brReg)), ciReg);

					pIn1 += 16;
					pIn2 += 16;

					for ( k = 0; k < 8; k++) {
						pAcc[2 * k] = ((float *)&crReg)[k];
						pAcc[2 * k + 1] = ((float *)&ciReg)[k];
					}
					pAcc += 16;
				}
			}
		}
		else {
			for (amf_size channelId = 0; channelId < channels; channelId++) 
			{
				for (amf_size id = 0; id < countOfComplexNumbers; id++)
				{
					amf_size j;
					float ar, ai, br, bi;
					j = (id << 1);
					ar = inputBuffers1[channelId][j];
					ai = inputBuffers1[channelId][j + 1];
					br = inputBuffers2[channelId][j];
					bi = inputBuffers2[channelId][j + 1];
					accumbuffers[channelId][j] += (ar*br - ai*bi);
					accumbuffers[channelId][j + 1] += (ar*bi + ai*br);
				}
			}
		}
	}

	return AMF_OK;
}


AMF_RESULT TANMathImpl::PlanarComplexMultiplyAccumulate(const float* const inputBuffers1[],
	const float* const inputBuffers2[],
	float *accumbuffers[],
	amf_uint32 channels,
	amf_size countOfComplexNumbers,
	amf_uint riPlaneSpacing)
{
	AMF_RETURN_IF_FALSE(inputBuffers1 != NULL, AMF_INVALID_ARG, L"inputBuffers1 == NULL");
	AMF_RETURN_IF_FALSE(inputBuffers2 != NULL, AMF_INVALID_ARG, L"inputBuffers2 == NULL");
	AMF_RETURN_IF_FALSE(accumbuffers != NULL, AMF_INVALID_ARG, L"accumbuffers == NULL");
	AMF_RETURN_IF_FALSE(channels > 0, AMF_INVALID_ARG, L"channels == 0");
	AMF_RETURN_IF_FALSE(countOfComplexNumbers > 0, AMF_INVALID_ARG, L"countOfComplexNumbers == 0");

	if (m_pContextTAN->GetOpenCLContext())
	{
		return AMF_NOT_IMPLEMENTED;

		//for (amf_size channelId = 0; channelId < channels; channelId++)
		//{
		//	// Adjust internal buffer size
		//	cl_int clErr = 0;;
		//	amf_size requiredbuffersize = countOfComplexNumbers * sizeof(float) * 2;
		//	AdjustInternalBufferSize(&m_pInternalBufferIn1_MulAccu, &m_iInternalBufferIn1Size_MulAccu, requiredbuffersize);
		//	AdjustInternalBufferSize(&m_pInternalBufferIn2_MulAccu, &m_iInternalBufferIn2Size_MulAccu, requiredbuffersize);
		//	AdjustInternalBufferSize(&m_pInternalBufferOut_MulAccu, &m_iInternalBufferOutSize_MulAccu, requiredbuffersize);

		//	clErr = clEnqueueWriteBuffer(m_clQueue, m_pInternalBufferIn1_MulAccu, CL_TRUE, 0, requiredbuffersize, inputBuffers1[channelId], 0, NULL, NULL);
		//	if (clErr != CL_SUCCESS) { printf("Failed to copy from HOST to OPENCL memory"); return AMF_FAIL; }
		//	clErr = clEnqueueWriteBuffer(m_clQueue, m_pInternalBufferIn2_MulAccu, CL_TRUE, 0, requiredbuffersize, inputBuffers2[channelId], 0, NULL, NULL);
		//	if (clErr != CL_SUCCESS) { printf("Failed to copy from HOST to OPENCL memory"); return AMF_FAIL; }
		//	clErr = clEnqueueWriteBuffer(m_clQueue, m_pInternalBufferOut_MulAccu, CL_TRUE, 0, requiredbuffersize, accumbuffers[channelId], 0, NULL, NULL);
		//	if (clErr != CL_SUCCESS) { printf("Failed to copy from HOST to OPENCL memory"); return AMF_FAIL; }


		//	ComplexMultiplyAccumulate(m_pInternalBufferIn1_MulAccu, m_pInternalBufferIn2_MulAccu, m_pInternalBufferOut_MulAccu, 1, countOfComplexNumbers, 0, 0, 0);

		//	clErr = clEnqueueReadBuffer(m_clQueue, m_pInternalBufferOut_MulAccu, CL_TRUE, 0, requiredbuffersize, accumbuffers[channelId], 0, NULL, NULL);
		//	if (clErr != CL_SUCCESS) { printf("Failed to copy from OPENCL to HOST memory"); return AMF_FAIL; }
		//}
	}
	else
	{
#ifdef AVX512SUPPORT 
	if (useAVX512) {
			// use AVX and FMA intrinsics to process 8 samples per pass
			_mm512_setzero_ps();

			int channelId = 0;
			int id = 0;
			int k = 0;
			__m512 *arReg, *aiReg, *brReg, *biReg, *crReg, *ciReg;

			for (channelId = 0; channelId < channels; channelId++)
			{
				arReg = (__m512 *)inputBuffers1[channelId];
				aiReg = (__m512 *)&inputBuffers1[channelId][riPlaneSpacing];
				brReg = (__m512 *)inputBuffers2[channelId];
				biReg = (__m512 *)&inputBuffers2[channelId][riPlaneSpacing];
				crReg = (__m512 *)accumbuffers[channelId];
				ciReg = (__m512 *)&accumbuffers[channelId][riPlaneSpacing];

				for (id = 0; id < countOfComplexNumbers; id += 16)
				{
					*crReg = _mm512_add_ps(_mm512_fmsub_ps(*arReg, *brReg, _mm512_mul_ps(*aiReg, *biReg)), *crReg);
					*ciReg = _mm512_add_ps(_mm512_fmadd_ps(*arReg, *biReg, _mm512_mul_ps(*aiReg, *brReg)), *ciReg);

					arReg++;
					aiReg++;
					brReg++;
					biReg++;
					crReg++;
					ciReg++;
				}
			}

		} else
#endif
		if (useAVX256) {
			// use AVX and FMA intrinsics to process 8 samples per pass
			_mm256_zeroall();

			//int channelId = 0;
			//int id = 0;
		
//int nThreads = omp_get_num_threads();
//nThreads = omp_get_max_threads();

#pragma omp parallel default(none) shared(channels,inputBuffers1,inputBuffers2,accumbuffers,countOfComplexNumbers,riPlaneSpacing)
#pragma omp for  
		for (int channelId = 0; channelId < (int)channels; channelId++)
			{
			__m256 *arReg, *aiReg, *brReg, *biReg, *crReg, *ciReg;
				arReg = (__m256 *)inputBuffers1[channelId];
				aiReg = (__m256 *)&inputBuffers1[channelId][riPlaneSpacing];
				brReg = (__m256 *)inputBuffers2[channelId];
				biReg = (__m256 *)&inputBuffers2[channelId][riPlaneSpacing];
				crReg = (__m256 *)accumbuffers[channelId];
				ciReg = (__m256 *)&accumbuffers[channelId][riPlaneSpacing];

				for (int id = 0; id < countOfComplexNumbers; id += 8)
				{
					*crReg = _mm256_add_ps(_mm256_fmsub_ps(*arReg, *brReg, _mm256_mul_ps(*aiReg, *biReg)), *crReg);
					*ciReg = _mm256_add_ps(_mm256_fmadd_ps(*arReg, *biReg, _mm256_mul_ps(*aiReg, *brReg)), *ciReg);

					arReg++;
					aiReg++;
					brReg++;
					biReg++;
					crReg++;
					ciReg++;
				}
			}
		}
		else {
#pragma omp parallel default(none) shared(channels,inputBuffers1,inputBuffers2,accumbuffers,riPlaneSpacing,countOfComplexNumbers)
#pragma omp for 
			for (int channelId = 0; channelId < (int)channels; channelId++)
			{
				for (amf_size id = 0; id < countOfComplexNumbers; id++)
				{
					float ar, ai, br, bi;

					ar = inputBuffers1[channelId][id];
					ai = inputBuffers1[channelId][id + riPlaneSpacing];
					br = inputBuffers2[channelId][id];
					bi = inputBuffers2[channelId][id + riPlaneSpacing];

					accumbuffers[channelId][id] += (ar*br - ai*bi);
					accumbuffers[channelId][id + riPlaneSpacing] += (ar*bi + ai*br);
				}
			}
		}
	}

	return AMF_OK;
}
#ifdef USE_IPP
AMF_RESULT TANMathImpl::IPPComplexMultiplyAccumulate(const float* const inputBuffers1[],
	const float* const inputBuffers2[],
	float *accumbuffers[],
	float *workbuffer[],
	amf_uint32 channels,
	amf_size numOfSamplesToProcess) {
	IppStatus ret;
	int chan;
#pragma omp parallel default(none) private(chan,ret) shared(channels, inputBuffers1, inputBuffers2, workbuffer, numOfSamplesToProcess)
#pragma omp for
	for (chan = 0; chan < channels; chan++) {
		ret = ippsMulPack_32f(inputBuffers1[chan], inputBuffers2[chan], workbuffer[chan], numOfSamplesToProcess);
		ret = ippsAdd_32f(workbuffer[chan], accumbuffers[chan], accumbuffers[chan], numOfSamplesToProcess);
	}
	return AMF_OK;
	return AMF_NOT_SUPPORTED;
}
#endif


//AMF_RESULT TANMathImpl::ComplexMultiplyAccumulate(
//	const cl_mem inputBuffers1[],
//	const amf_size buffers1OffsetInSamples[],
//	const cl_mem inputBuffers2[],
//	const amf_size buffers2OffsetInSamples[],
//	cl_mem accumBuffers[],
//	const amf_size accumBuffersOffsetInSamples[],
//	amf_uint32 channels,
//    amf_size countOfComplexNumbers)
//{
//	if (m_pKernelComplexSum == 0)
//	{
//		return AMF_OPENCL_FAILED;
//	}
//
//	AMF_RETURN_IF_FALSE(inputBuffers1 != NULL, AMF_INVALID_ARG, L"inputBuffer == NULL");
//	AMF_RETURN_IF_FALSE(inputBuffers2 != NULL, AMF_INVALID_ARG, L"inputBuffer == NULL");
//	AMF_RETURN_IF_FALSE(accumBuffers!= NULL, AMF_INVALID_ARG, L"accumBuffers == NULL");
//	AMF_RETURN_IF_FALSE(channels > 0, AMF_INVALID_ARG, L"channels == 0");
//    AMF_RETURN_IF_FALSE(countOfComplexNumbers > 0, AMF_INVALID_ARG, L"countOfComplexNumbers == 0");
//    AMF_RETURN_IF_FALSE(!(countOfComplexNumbers & 1), AMF_INVALID_ARG, L"countOfComplexNumbers is odd not supported by OpenCL kernel");
//
//	for (amf_size channelId = 0; channelId < channels; channelId++)
//	{
//		AMF_RETURN_IF_FALSE(inputBuffers1[channelId] != NULL, AMF_INVALID_ARG, L"inputBuffers1[%u] == NULL", channelId);
//		AMF_RETURN_IF_FALSE(inputBuffers2[channelId] != NULL, AMF_INVALID_ARG, L"inputBuffers2[%u] == NULL", channelId);
//		AMF_RETURN_IF_FALSE(accumBuffers[channelId] != NULL, AMF_INVALID_ARG, L"accumBuffers[%u] == NULL", channelId);
//
//		AMF_RESULT result = AMF_FAIL;
//		result = ComplexMultiplyAccumulate(
//			inputBuffers1[channelId], buffers1OffsetInSamples[channelId],
//			inputBuffers2[channelId], buffers2OffsetInSamples[channelId],
//            accumBuffers[channelId], accumBuffersOffsetInSamples[channelId], countOfComplexNumbers
//			);
//		AMF_RETURN_IF_FAILED(result);
//	}
//	return AMF_OK;
//}


 AMF_RESULT TANMathImpl::ComplexMultiplyAccumulate(
	const cl_mem inputBuffers1,
	const cl_mem inputBuffers2,
	cl_mem accumBuffers,
	amf_uint32 channels,
	amf_size countOfComplexNumbers,
	amf_size inputOffset1,
	amf_size inputOffset2,
	amf_size inputStride1,
	amf_size inputStride2)
{
	AMF_RETURN_IF_FALSE(inputBuffers1 != NULL, AMF_INVALID_ARG, L"inputBuffer == NULL");
	AMF_RETURN_IF_FALSE(inputBuffers2 != NULL, AMF_INVALID_ARG, L"inputBuffer == NULL");
	AMF_RETURN_IF_FALSE(accumBuffers != NULL, AMF_INVALID_ARG, L"accumBuffers == NULL");
	AMF_RETURN_IF_FALSE(channels > 0, AMF_INVALID_ARG, L"channels == 0");
	AMF_RETURN_IF_FALSE(countOfComplexNumbers > 0, AMF_INVALID_ARG, L"countOfComplexNumbers == 0");
	AMF_RETURN_IF_FALSE(!(countOfComplexNumbers & 1), AMF_INVALID_ARG, L"countOfComplexNumbers is odd not supported by OpenCL kernel");


	AMFLock lock(&m_sect);

	cl_uint index = 0;
	int zero = 0;

	int outputStride = countOfComplexNumbers * 2;
	cl_int clErr;
	// Set kernel arguments (additional arugments if needed)
	clErr = clSetKernelArg(m_pKernelComplexMulAccum, index++, sizeof(cl_mem), &inputBuffers1);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
	clErr = clSetKernelArg(m_pKernelComplexMulAccum, index++, sizeof(cl_mem), &inputBuffers2);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
	clErr = clSetKernelArg(m_pKernelComplexMulAccum, index++, sizeof(cl_mem), &accumBuffers);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
	clErr = clSetKernelArg(m_pKernelComplexMulAccum, index++, sizeof(int), &inputOffset1);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
	clErr = clSetKernelArg(m_pKernelComplexMulAccum, index++, sizeof(int), &inputOffset2);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
	clErr = clSetKernelArg(m_pKernelComplexMulAccum, index++, sizeof(int), &inputStride1);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
	clErr = clSetKernelArg(m_pKernelComplexMulAccum, index++, sizeof(int), &inputStride2);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
	clErr = clSetKernelArg(m_pKernelComplexMulAccum, index++, sizeof(int), &outputStride);
	if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }

	amf_size global[3] = { countOfComplexNumbers / 2, channels, 0 };
	amf_size local[3] = { 0, 0, 0 };
	clErr = clEnqueueNDRangeKernel(m_clQueue, m_pKernelComplexMulAccum, 2, NULL, global, NULL, 0, NULL, NULL);
	if (clErr != CL_SUCCESS) { printf("Failed to enqueue OpenCL kernel\n"); return AMF_FAIL; }

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
		clErr = clEnqueueWriteBuffer(m_clQueue, m_pInternalBufferIn1_Multiply, CL_TRUE, 0, requiredbuffersize, inputBuffer1, 0, NULL, NULL);
		if (clErr != CL_SUCCESS){ printf("Failed to copy from HOST to OPENCL memory"); return AMF_FAIL; }
		clErr = clEnqueueWriteBuffer(m_clQueue, m_pInternalBufferIn2_Multiply, CL_TRUE, 0, requiredbuffersize, inputBuffer2, 0, NULL, NULL);
		if (clErr != CL_SUCCESS){ printf("Failed to copy from HOST to OPENCL memory"); return AMF_FAIL; }
        ComplexMultiplication(m_pInternalBufferIn1_Multiply, 0, m_pInternalBufferIn2_Multiply, 0, m_pInternalBufferOut_Multiply, 0, countOfComplexNumbers);
		clErr = clEnqueueReadBuffer(m_clQueue, m_pInternalBufferOut_Multiply, CL_TRUE, 0, requiredbuffersize, outputBuffer, 0, NULL, NULL);
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
		clErr = clEnqueueWriteBuffer(m_clQueue, m_pInternalBufferIn1_MulAccu, CL_TRUE, 0, requiredbuffersize, inputBuffer1, 0, NULL, NULL);
		if (clErr != CL_SUCCESS){ printf("Failed to copy from HOST to OPENCL memory"); return AMF_FAIL; }
		clErr = clEnqueueWriteBuffer(m_clQueue, m_pInternalBufferIn2_MulAccu, CL_TRUE, 0, requiredbuffersize, inputBuffer2, 0, NULL, NULL);
		if (clErr != CL_SUCCESS){ printf("Failed to copy from HOST to OPENCL memory"); return AMF_FAIL; }
        ComplexMultiplyAccumulate(m_pInternalBufferIn1_MulAccu, 0, m_pInternalBufferIn2_MulAccu, 0, m_pInternalBufferOut_MulAccu, 0, countOfComplexNumbers);
		clErr = clEnqueueReadBuffer(m_clQueue, m_pInternalBufferOut_MulAccu, CL_TRUE, 0, 2 * sizeof(float), accumBuffer, 0, NULL, NULL);
		if (clErr != CL_SUCCESS){ printf("Failed to copy from OPENCL to HOST memory"); return AMF_FAIL; }
	}
    else
    {
        if (useAVX256){
            // use AVX and FMA intrinsics to process 8 samples per pass
            register __m256 arReg, aiReg, brReg, biReg, crReg, ciReg, zero;
            register __m128 t1Reg, t2Reg;
            _mm256_zeroall();
            ///tmp = _mm256_set1_ps(hf);

 			const float *pIn1 = inputBuffer1;
			const float *pIn2 = inputBuffer2;
			float *pAcc = accumBuffer;

            for (amf_size id = 0; id < countOfComplexNumbers; id += 8)
            {
				for (int k = 0; k < 8; k++){
				    ((float *)&arReg)[k] = pIn1[ 2 * k];
				    ((float *)&aiReg)[k] = pIn1[ 2 * k + 1];
				    ((float *)&brReg)[k] = pIn2[ 2 * k];
				    ((float *)&biReg)[k] = pIn2[ 2 * k + 1];
				    ((float *)&crReg)[k] = pAcc[ 2 * k];
				    ((float *)&ciReg)[k] = pAcc[ 2 * k + 1];
				}

				pIn1 += 16;
				pIn2 += 16;

                crReg = _mm256_add_ps(_mm256_fmsub_ps(arReg, brReg, _mm256_mul_ps(aiReg, biReg)),crReg);
                ciReg = _mm256_add_ps(_mm256_fmadd_ps(arReg, biReg, _mm256_mul_ps(aiReg, brReg)),ciReg);

 				for (int k = 0; k < 8; k++) {
					pAcc[ 2 * k] = ((float *)&crReg)[k];
					pAcc[ 2 * k + 1] = ((float *)&ciReg)[k];
				}
				pAcc += 16;
            }
        }
        else {
            for (amf_size id = 0; id < countOfComplexNumbers; id++)
            {
                amf_size j;
                float ar, ai, br, bi;
                j = (id << 1);
                ar = inputBuffer1[j];
                ai = inputBuffer1[j + 1];
                br = inputBuffer2[j];
                bi = inputBuffer2[j + 1];
                accumBuffer[j] += (ar*br - ai*bi);
                accumBuffer[j + 1] += (ar*bi + ai*br);
            }
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
	

    amf_size global[3] = { countOfComplexNumbers / 2, 0, 0 };
    amf_size local[3] = { 0, 0, 0 };
	clErr = clEnqueueNDRangeKernel(m_clQueue, m_pKernelComplexMul, 1, NULL, global, NULL, 0, NULL, NULL);
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
		clErr = clEnqueueFillBuffer(m_clQueue, output_mem, &filled, sizeof(cl_float), 0, requiredInternalBuffersize, 0, NULL, NULL);
		if (clErr != CL_SUCCESS) { printf("Failed to zero output buffer"); return AMF_FAIL; }
		if (clErr != CL_SUCCESS) { printf("Faield to wait for ocl event"); return AMF_FAIL; }
		clErr = clSetKernelArg(m_pKernelComplexSum, pInputFlt_arg_index, sizeof(amf_int64), &input_mem);
		if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
		clErr = clSetKernelArg(m_pKernelComplexSum, pResultFlt_arg_index, sizeof(amf_int64), &output_mem);
		if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
		clErr = clSetKernelArg(m_pKernelComplexSum, countInQuadFloats_arg_index, sizeof(amf_int64), &leftOver);
		if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
		amf_size global[3] = { leftOver,0, 0 };
		clErr = clEnqueueNDRangeKernel(m_clQueue, m_pKernelComplexSum, 1, NULL, global, NULL, 0, NULL, NULL);
		if (clErr != CL_SUCCESS) { printf("Failed to enqueue OpenCL kernel\n"); return AMF_FAIL; }
		numOfIteration++;
		leftOver = (leftOver + oclWorkGroupSize - 1) / oclWorkGroupSize / 2;
	}
	
	// CPU Summation
	cl_mem resultBuffferOCL = nullptr;
	numOfIteration % 2 == 0 ? resultBuffferOCL = m_pInternalSwapBuffer1_MulAccu : resultBuffferOCL = m_pInternalSwapBuffer2_MulAccu;

	float* resultBufferHost = (float*)clEnqueueMapBuffer(m_clQueue, resultBuffferOCL, CL_TRUE, CL_MAP_READ | CL_MAP_WRITE, 0, leftOver * 4 * sizeof(float), 0, NULL,NULL,&clErr);
	//ocl_queue.push_back(oclevent);
	if (clErr != CL_SUCCESS) { printf("Faield to map OCL Buffer"); return AMF_FAIL; }
	float* outputbufferhost = (float*)clEnqueueMapBuffer(m_clQueue, accumBuffer, CL_TRUE, CL_MAP_READ | CL_MAP_WRITE, (accumBufferOffsetInSamples) * sizeof(float), 2 * sizeof(float), 0, NULL, NULL, &clErr);
	if (clErr != CL_SUCCESS) { printf("Faield to map OCL Buffer"); return AMF_FAIL; }
	for (amf_size index = 0; index < leftOver*4; index++)
	{
		amf_size temp = index & 1;
		float a = resultBufferHost[index];
		float b = outputbufferhost[temp];
		outputbufferhost[temp] += resultBufferHost[index];
	}
	clErr = clEnqueueUnmapMemObject(m_clQueue, resultBuffferOCL, resultBufferHost, 0,NULL,NULL);
	if (clErr != CL_SUCCESS) { printf("Faield to unmap OCL Buffer"); return AMF_FAIL; }
	clErr = clEnqueueUnmapMemObject(m_clQueue, accumBuffer, outputbufferhost, 0,NULL,NULL);
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

		clErr = clEnqueueWriteBuffer(m_clQueue, m_pInternalBufferIn1_Division, CL_TRUE, 0, requiredbuffersize, inputBuffer1, 0, NULL, NULL);
		if (clErr != CL_SUCCESS){ printf("Failed to copy from HOST to OPENCL memory"); return AMF_FAIL; }
		clErr = clEnqueueWriteBuffer(m_clQueue, m_pInternalBufferIn2_Division, CL_TRUE, 0, requiredbuffersize, inputBuffer2, 0, NULL, NULL);
		if (clErr != CL_SUCCESS){ printf("Failed to copy from HOST to OPENCL memory"); return AMF_FAIL; }
        ComplexDivision(m_pInternalBufferIn1_Division, 0, m_pInternalBufferIn2_Division, 0, m_pInternalBufferOut_Division, 0, countOfComplexNumbers);
		clErr = clEnqueueReadBuffer(m_clQueue, m_pInternalBufferOut_Division, CL_TRUE, 0, requiredbuffersize, outputBuffer, 0, NULL, NULL);
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


    amf_size global[3] = { countOfComplexNumbers / 2, 0, 0 };
    amf_size local[3] = { 0, 0, 0 };
	clErr = clEnqueueNDRangeKernel(m_clQueue, m_pKernelComplexDiv, 1, NULL, global, NULL, 0, NULL, NULL);
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
