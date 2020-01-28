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
#include "ConverterImpl.h"
#include "../core/TANContextImpl.h"
#include "public/common/AMFFactory.h"
#include "../../common/OCLHelper.h"
#include "cpucaps.h"

#include <math.h>

//#include "Converter.cl.h"
#include "CLKernel_Converter.h"
#define AMF_FACILITY L"TANConverterImpl"


using namespace amf;
//const InstructionSet::InstructionSet_Internal InstructionSet::CPU_Rep;
bool TANConverterImpl::useSSE2 = true; // InstructionSet::SSE2();

static const AMFEnumDescriptionEntry AMF_MEMORY_ENUM_DESCRIPTION[] =
{
#if AMF_BUILD_OPENCL
    {AMF_MEMORY_OPENCL,     L"OpenCL"},
#endif
    {AMF_MEMORY_HOST,       L"CPU"},
    {AMF_MEMORY_UNKNOWN,    0}  // This is end of description mark
};
//-------------------------------------------------------------------------------------------------
TAN_SDK_LINK AMF_RESULT AMF_CDECL_CALL TANCreateConverter(
    amf::TANContext* pContext,
    amf::TANConverter** ppComponent
    )
{
    TANContextImplPtr contextImpl(pContext);
    *ppComponent = new TANConverterImpl(pContext, NULL );
    (*ppComponent)->Acquire();
    return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
TANConverterImpl::TANConverterImpl(TANContext *pContextTAN, AMFContext* pContextAMF) :
    m_pContextTAN(pContextTAN),
    m_pContextAMF(pContextAMF),
    m_pCommandQueueCl(nullptr),
    m_overflowBuffer(NULL),
    m_eOutputMemoryType(AMF_MEMORY_HOST)
{
    AMFPrimitivePropertyInfoMapBegin
        AMFPropertyInfoEnum(TAN_OUTPUT_MEMORY_TYPE ,  L"Output Memory Type", AMF_MEMORY_HOST, AMF_MEMORY_ENUM_DESCRIPTION, false),
    AMFPrimitivePropertyInfoMapEnd
}
//-------------------------------------------------------------------------------------------------
TANConverterImpl::~TANConverterImpl(void)
{
    Terminate();
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL TANConverterImpl::Init()
{
    AMFLock lock(&m_sect);

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
AMF_RESULT  AMF_STD_CALL TANConverterImpl::InitCpu()
{
    // No device setup needs to occur here; we're done!
    return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL TANConverterImpl::InitGpu()
{
    cl_int ret;
    AMF_RESULT res = AMF_OK;

    /* OpenCL Initialization */

    // Given some command queue, retrieve the cl_context...
    m_pCommandQueueCl = m_pContextTAN->GetOpenCLConvQueue();
    ret = clGetCommandQueueInfo(m_pCommandQueueCl, CL_QUEUE_CONTEXT, sizeof(cl_context),
        &m_pContextCl, NULL);
        AMF_RETURN_IF_CL_FAILED(ret,  L"Cannot retrieve cl_context from cl_command_queue.");

    // ...and cl_device from it
    ret = clGetCommandQueueInfo(m_pCommandQueueCl, CL_QUEUE_DEVICE, sizeof(cl_device_id),
        &m_pDeviceCl, NULL);
    AMF_RETURN_IF_CL_FAILED(ret, L"Cannot retrieve cl_device_id from cl_command_queue.");

    // Retain the queue for use
    ret = clRetainCommandQueue(m_pCommandQueueCl);
    printf("Queue %llX +1\r\n", m_pCommandQueueCl);

    AMF_RETURN_IF_CL_FAILED(ret, L"Failed to retain command queue.");

    amf_int64 tmp = 0;
    GetProperty(TAN_OUTPUT_MEMORY_TYPE, &tmp);
    m_eOutputMemoryType = (AMF_MEMORY_TYPE)tmp;
	TANContextImplPtr contextImpl(m_pContextTAN);
	m_pDeviceAMF = contextImpl->GetConvolutionCompute();
    int temp = 0;
    m_overflowBuffer = clCreateBuffer(
        m_pContextCl, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(amf_int32), &temp, &ret);
    AMF_RETURN_IF_CL_FAILED(ret, L"Failed to create buffer");
	//... Preparing OCL Kernel
	bool OCLKenel_Err = false;
	OCLKenel_Err = GetOclKernel(m_clkFloat2Float, m_pDeviceAMF, contextImpl->GetOpenCLConvQueue(), "floatToFloat", Converter, ConverterCount, "floatToFloat", "");
	if (!OCLKenel_Err){ printf("Failed to compile Converter Kernel floatToFloat"); return AMF_FAIL; }
	OCLKenel_Err = GetOclKernel(m_clkFloat2Short, m_pDeviceAMF, contextImpl->GetOpenCLConvQueue(), "floatToShort", Converter, ConverterCount, "floatToShort", "");
	if (!OCLKenel_Err){ printf("Failed to compile Converter Kernel floatToShort"); return AMF_FAIL; }
	OCLKenel_Err = GetOclKernel(m_clkShort2Short, m_pDeviceAMF, contextImpl->GetOpenCLConvQueue(), "shortToShort", Converter, ConverterCount, "shortToShort", "");
	if (!OCLKenel_Err){ printf("Failed to compile Converter Kernel shortToShort"); return AMF_FAIL; }
	OCLKenel_Err = GetOclKernel(m_clkShort2Float, m_pDeviceAMF, contextImpl->GetOpenCLConvQueue(), "shortToFloat", Converter, ConverterCount, "shortToFloat", "");
	if (!OCLKenel_Err){ printf("Failed to compile Converter Kernel shortToFloat"); return AMF_FAIL; }

	return res;
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL TANConverterImpl::Terminate()
{
    AMFLock lock(&m_sect);

    m_pDeviceAMF = NULL;
	if (m_pContextTAN->GetOpenCLContext() != nullptr)
	{
		AMF_RETURN_IF_CL_FAILED(clReleaseKernel(m_clkFloat2Float), L"Failed to release cl kernel");
		AMF_RETURN_IF_CL_FAILED(clReleaseKernel(m_clkFloat2Short), L"Failed to release cl kernel");
		AMF_RETURN_IF_CL_FAILED(clReleaseKernel(m_clkShort2Float), L"Failed to release cl kernel");
		AMF_RETURN_IF_CL_FAILED(clReleaseKernel(m_clkShort2Short), L"Failed to release cl kernel");
		m_clkShort2Short = nullptr;
		m_clkFloat2Float = nullptr;
		m_clkShort2Float = nullptr;
		m_clkFloat2Short = nullptr;
	}
    m_pDeviceCl = NULL;
    if (m_pCommandQueueCl)
    {
        printf("Queue release %llX\r\n", m_pCommandQueueCl);
        cl_int ret = clReleaseCommandQueue(m_pCommandQueueCl);
        AMF_RETURN_IF_CL_FAILED(ret, L"Failed to release command queue.");
    }
    m_pCommandQueueCl = NULL;
    m_pKernelCopy = NULL;
    m_pContextAMF = NULL;
    m_pContextTAN = NULL;
    if (m_overflowBuffer)
    {
        cl_int clErr = clReleaseMemObject(m_overflowBuffer);
        AMF_RETURN_IF_CL_FAILED(clErr, L"Failed to release buffer.");
        m_overflowBuffer = NULL;
    }

    return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL    TANConverterImpl::Convert(
    short* inputBuffer,
    amf_size inputStep,
    amf_size numOfSamplesToProcess,
    float* outputBuffer,
    amf_size outputStep,
    float conversionGain
)
{
    AMF_RETURN_IF_FALSE(inputBuffer != NULL, AMF_INVALID_ARG, L"inputBuffer == NULL");
    AMF_RETURN_IF_FALSE(outputBuffer != NULL, AMF_INVALID_ARG, L"outputBuffer == NULL");
    AMF_RETURN_IF_FALSE(numOfSamplesToProcess > 0, AMF_INVALID_ARG, L"numOfSamplesToProcess <= 0");
    AMF_RETURN_IF_FALSE(conversionGain > 0, AMF_INVALID_ARG, L"conversionGain <= 0");

    AMFLock lock(&m_sect);

    float scale = conversionGain / SHRT_MAX;

   // ToDo use AVX
    //__m256i _mm256_cvtps_epi32 (__m256 a)
     while (numOfSamplesToProcess > 0)
    {
        *outputBuffer = *inputBuffer * scale;

        inputBuffer += inputStep;
        outputBuffer += outputStep;
        numOfSamplesToProcess--;
    }

    return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL    TANConverterImpl::Convert(
    short** inputBuffers,
    amf_size inputStep,

    amf_size numOfSamplesToProcess,

    float** outputBuffers,
    amf_size outputStep,

    float conversionGain,
    int count
    )
{
    AMF_RETURN_IF_FALSE(inputBuffers != NULL, AMF_INVALID_ARG, L"inputBuffer == NULL");
    AMF_RETURN_IF_FALSE(outputBuffers != NULL, AMF_INVALID_ARG, L"outputBuffer == NULL");
    AMF_RETURN_IF_FALSE(numOfSamplesToProcess > 0, AMF_INVALID_ARG, L"numOfSamplesToProcess <= 0");
    AMF_RETURN_IF_FALSE(conversionGain > 0, AMF_INVALID_ARG, L"conversionGain <= 0");

    AMFLock lock(&m_sect);

    float scale = conversionGain / SHRT_MAX;

    // Process an arbitrary number of conversions
    int i;
    for (i = 0; i < count; i++)
    {
        AMF_RETURN_IF_FAILED(Convert(
            inputBuffers[i], inputStep,
            numOfSamplesToProcess,
            outputBuffers[i], outputStep,
            conversionGain));
    }

    return AMF_OK;
}

//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL    TANConverterImpl::Convert(
    float* inputBuffer,
    amf_size inputStep,

    amf_size numOfSamplesToProcess,

    short* outputBuffer,
    amf_size outputStep,

    float conversionGain,
    bool* outputClipped
)
{
    AMF_RETURN_IF_FALSE(inputBuffer != NULL, AMF_INVALID_ARG, L"inputBuffer == NULL");
    AMF_RETURN_IF_FALSE(outputBuffer != NULL, AMF_INVALID_ARG, L"outputBuffer == NULL");
    AMF_RETURN_IF_FALSE(numOfSamplesToProcess > 0, AMF_INVALID_ARG, L"numOfSamplesToProcess <= 0");
    AMF_RETURN_IF_FALSE(conversionGain > 0, AMF_INVALID_ARG, L"conversionGain <= 0");

    AMFLock lock(&m_sect);

    float scale = SHRT_MAX * conversionGain;
    bool clip = false;


    // Go faster with SSE2
    if (useSSE2){
        float fin[8];
        int iout[8];
        short sout[8];
        register __m128 f4a;
        register __m128 f4b;
        register __m128i i4a;
        register __m128i i4b;
        register __m128i *s8;
        while (numOfSamplesToProcess > 0) {

            if (numOfSamplesToProcess >= 8) {
                fin[0] = inputBuffer[0] * scale;
                fin[1] = inputBuffer[inputStep] * scale;
                fin[2] = inputBuffer[2 * inputStep] * scale;
                fin[3] = inputBuffer[3 * inputStep] * scale;
                fin[4] = inputBuffer[4 * inputStep] * scale;
                fin[5] = inputBuffer[5 * inputStep] * scale;
                fin[6] = inputBuffer[6 * inputStep] * scale;
                fin[7] = inputBuffer[7 * inputStep] * scale;

                f4a = *((__m128 *)&fin[0]);
                f4b = *((__m128 *)&fin[4]);

                i4a = *((__m128i *)&iout[0]);
                i4b = *((__m128i *)&iout[4]);

                s8 = (__m128i *)sout;

                // convert with saturation to avoid wrap around
                i4a = _mm_cvtps_epi32(f4a); // convert 4 floats to 4 int32
                i4b = _mm_cvtps_epi32(f4b); // convert 4 floats to 4 int32
                *s8 = _mm_packs_epi32(i4a, i4b); // convert 8 int32 to 8 shorts with saturation

                outputBuffer[0] = sout[0];
                outputBuffer[outputStep] = sout[1];
                outputBuffer[2 * outputStep] = sout[2];
                outputBuffer[3 * outputStep] = sout[3];
                outputBuffer[4 * outputStep] = sout[4];
                outputBuffer[5 * outputStep] = sout[5];
                outputBuffer[6 * outputStep] = sout[6];
                outputBuffer[7 * outputStep] = sout[7];

                inputBuffer += 8 * inputStep;
                outputBuffer += 8 * outputStep;
                numOfSamplesToProcess -= 8;
            }
            else {
                float f;
                long v;
                f = *inputBuffer;
                f *= scale;
                v = int(f);

                if (v > SHRT_MAX) {
                    v = SHRT_MAX;
                    clip |= true;
                }

                if (v < SHRT_MIN) {
                    v = SHRT_MIN;
                    clip |= true;
                }

                *outputBuffer = (short)v;

                inputBuffer += inputStep;
                outputBuffer += outputStep;
                numOfSamplesToProcess--;
            }
        }
    }
    else {
        while (numOfSamplesToProcess > 0) {
            float f;
            long v;
            f = *inputBuffer;
            f *= scale;
            v = int(f);

            if (v > SHRT_MAX) {
                v = SHRT_MAX;
                clip |= true;
            }

            if (v < SHRT_MIN) {
                v = SHRT_MIN;
                clip |= true;
            }

            *outputBuffer = (short)v;

            inputBuffer += inputStep;
            outputBuffer += outputStep;
            numOfSamplesToProcess--;
        }
    }
    if (outputClipped != NULL)
    {
        *outputClipped = clip;
    }
    return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL    TANConverterImpl::Convert(
    float** inputBuffers,
    amf_size inputStep,

    amf_size numOfSamplesToProcess,

    short** outputBuffers,
    amf_size outputStep,

    float conversionGain,
    int count,
    bool* outputClipped)
{
    AMF_RETURN_IF_FALSE(inputBuffers != NULL, AMF_INVALID_ARG, L"inputBuffer == NULL");
    AMF_RETURN_IF_FALSE(outputBuffers != NULL, AMF_INVALID_ARG, L"outputBuffer == NULL");
    AMF_RETURN_IF_FALSE(numOfSamplesToProcess > 0, AMF_INVALID_ARG, L"numOfSamplesToProcess <= 0");
    AMF_RETURN_IF_FALSE(conversionGain > 0, AMF_INVALID_ARG, L"conversionGain <= 0");

    AMFLock lock(&m_sect);

    AMF_RESULT ret = AMF_OK;
    bool clipResult = false;


    // Process an arbitrary number of conversions; record clipping
    for (int i = 0; i < count; i++)
    {
        bool clip = false;
        ret = Convert(
            inputBuffers[i], inputStep,
            numOfSamplesToProcess,
            outputBuffers[i], outputStep,
            conversionGain, (outputClipped != NULL) ? &clip : NULL);
        clipResult = clipResult || clip;

    }
    if (outputClipped != NULL)
    {
        *outputClipped = clipResult;
    }
    return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL    TANConverterImpl::ConvertGpu(
    amf_handle inputBuffer,
	amf_size inputStep,
    amf_size inputOffset,
    TAN_SAMPLE_TYPE inputType,

    amf_handle outputBuffer,
	amf_size outputStep,
    amf_size outputOffset,
    TAN_SAMPLE_TYPE outputType,

    amf_size numOfSamplesToProcess,
    float conversionGain,
    bool* outputClipped
    )
{
    AMF_RESULT res = AMF_OK;
    AMF_KERNEL_ID m_KernelIdConvert;
    cl_int clErr = CL_SUCCESS;
    cl_kernel clKernel = nullptr;

    bool convert = false;
    bool canOverflow = false;

    if (inputType == TAN_SAMPLE_TYPE_FLOAT)
    {
        if (outputType == TAN_SAMPLE_TYPE_SHORT)
        {
			clKernel = m_clkFloat2Short;
            convert = true;
            canOverflow = true;
        }
        else if (outputType == TAN_SAMPLE_TYPE_FLOAT)
        {
			clKernel = m_clkFloat2Float;
        }
    }
    else if (inputType == TAN_SAMPLE_TYPE_SHORT) {
        if (outputType == TAN_SAMPLE_TYPE_FLOAT)
        {
			clKernel = m_clkShort2Float;
            convert = true;
        }
        else if (outputType == TAN_SAMPLE_TYPE_SHORT)
        {
			clKernel = m_clkShort2Short;
        }
    }
    else {
        AMF_RETURN_IF_FAILED(AMF_NOT_IMPLEMENTED, L"Argument types not supported for conversion");
    }

	if (clKernel == nullptr){ printf("Failed to select OCL converter kernel"); return AMF_FAIL; }
    // Set the kernel argument for the AMF device and continue to use OpenCL functions

    cl_uint index = 0;

    amf_int32 overflowBufferOut = 0;

    if (canOverflow && outputClipped != NULL)
    {
        clErr = clEnqueueWriteBuffer(
            m_pCommandQueueCl, m_overflowBuffer, CL_FALSE, 0, sizeof(amf_int32), &overflowBufferOut,
            0, NULL, NULL);
        if (clErr != CL_SUCCESS)
        {
            printf("Could not read from OpenCL buffer\n");
            return AMF_FAIL;
        }
    }
    // Set kernel arguments (additional arugments if needed)
    clErr = clSetKernelArg(clKernel, index++, sizeof(cl_mem), &inputBuffer);
    if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
    clErr = clSetKernelArg(clKernel, index++, sizeof(amf_int64), &inputStep);
    if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
    clErr = clSetKernelArg(clKernel, index++, sizeof(amf_int64), &inputOffset);
    if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
    clErr = clSetKernelArg(clKernel, index++, sizeof(cl_mem), &outputBuffer);
    if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
    clErr = clSetKernelArg(clKernel, index++, sizeof(amf_int64), &outputStep);
    if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
    clErr = clSetKernelArg(clKernel, index++, sizeof(amf_int64), &outputOffset);
    if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
    if (convert)
    {
        clErr = clSetKernelArg(clKernel, index++, sizeof(float), &conversionGain);
        if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
    }
    if (canOverflow)
    {
        clErr = clSetKernelArg(clKernel, index++, sizeof(cl_mem), &m_overflowBuffer);
        if (clErr != CL_SUCCESS) { printf("Failed to set OpenCL argument"); return AMF_FAIL; }
    }

    // Create as many work-items as there are conversions needed to be done and execute
    amf_size global[3] = { numOfSamplesToProcess, 0, 0 };
    amf_size local[3] = { 0, 0, 0 };
    clErr = clEnqueueNDRangeKernel(
        m_pCommandQueueCl, clKernel, 1, NULL, global, NULL, 0, NULL, NULL);
    if (clErr != CL_SUCCESS) { printf("Failed to enqueue OpenCL kernel\n"); return AMF_FAIL; }

    // Retrieve overflow results (if any) and finish OpenCL operations
    if (canOverflow && outputClipped != NULL)
    {
        clErr = clEnqueueReadBuffer(
            m_pCommandQueueCl, m_overflowBuffer, CL_TRUE, 0, sizeof(amf_int32), &overflowBufferOut,
            0, NULL, NULL);
        if (clErr != CL_SUCCESS)
        {
            printf("Could not read from OpenCL buffer\n");
            return AMF_FAIL;
        }
        *outputClipped = overflowBufferOut;
    }
    return res;
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL    TANConverterImpl::Convert(
    cl_mem inputBuffer,
	amf_size inputStep,
    amf_size inputOffset,
    TAN_SAMPLE_TYPE inputType,

    cl_mem outputBuffer,
	amf_size outputStep,
    amf_size outputOffset,
    TAN_SAMPLE_TYPE outputType,

    amf_size numOfSamplesToProcess,
    float conversionGain,
    bool* outputClipped
    )
{
    return Convert(
		&inputBuffer, inputStep, &inputOffset, inputType,
		&outputBuffer, outputStep, &outputOffset, outputType,
        numOfSamplesToProcess, conversionGain,
        1, outputClipped);
}

//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL    TANConverterImpl::Convert(
    cl_mem* inputBuffers,
	amf_size inputStep,
    amf_size* inputOffsets,

    TAN_SAMPLE_TYPE inputType,

    cl_mem* outputBuffers,
	amf_size outputStep,
    amf_size* outputOffsets,
    TAN_SAMPLE_TYPE outputType,

    amf_size numOfSamplesToProcess,
    float conversionGain,
    int count,
    bool* outputClipped
    )
{
    AMF_RETURN_IF_FALSE(inputBuffers != NULL, AMF_INVALID_ARG, L"inputBuffer == NULL");
    AMF_RETURN_IF_FALSE(outputBuffers != NULL, AMF_INVALID_ARG, L"outputBuffer == NULL");
    AMF_RETURN_IF_FALSE(numOfSamplesToProcess > 0, AMF_INVALID_ARG, L"numOfSamplesToProcess <= 0");
    AMF_RETURN_IF_FALSE(conversionGain > 0, AMF_INVALID_ARG, L"conversionGain <= 0");
    AMF_RETURN_IF_FALSE(count > 0, AMF_INVALID_ARG, L"count of conversions <= 0");

    AMFLock lock(&m_sect);

    AMF_RESULT ret = AMF_OK;


    // Process an arbitrary number of conversions; record clipping
    bool clipResult = false;
    for (int i = 0; i < count; i++)
    {
        bool clip = false;
        ret = ConvertGpu(
			inputBuffers[i], inputStep, inputOffsets[i], inputType,
			outputBuffers[i], outputStep, outputOffsets[i], outputType,
            numOfSamplesToProcess, conversionGain, (outputClipped!=NULL) ? &clip : NULL);
        clipResult = clipResult || clip;
    }
    if (outputClipped != NULL)
    {
        *outputClipped = clipResult;
    }
    return AMF_OK;
}


