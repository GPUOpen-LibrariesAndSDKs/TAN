// 
// MIT license 
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
#define _USE_MATH_DEFINES
#include <cmath>

#include "FFTImpl.h"
#include "../core/TANContextImpl.h"     //TAN

#include "tanlibrary/src/clFFT-master/src/include/clfft.h"
#include <memory>

#define AMF_FACILITY L"TANFFTImpl"

using namespace amf;

static const AMFEnumDescriptionEntry AMF_MEMORY_ENUM_DESCRIPTION[] = 
{
#if AMF_BUILD_OPENCL
    {AMF_MEMORY_OPENCL,     L"OpenCL"},
#endif
    {AMF_MEMORY_HOST,       L"CPU"},
    {AMF_MEMORY_UNKNOWN,    0}  // This is end of description mark
};
//-------------------------------------------------------------------------------------------------
TAN_SDK_LINK AMF_RESULT AMF_CDECL_CALL TANCreateFFT(
    amf::TANContext* pContext, 
    amf::TANFFT** ppComponent
    )
{
    TANContextImplPtr contextImpl(pContext);
    *ppComponent = new TANFFTImpl(pContext, false);
    (*ppComponent)->Acquire();
    return AMF_OK;
}

//-------------------------------------------------------------------------------------------------
 AMF_RESULT  amf::TANCreateFFT(
    amf::TANContext* pContext,
    amf::TANFFT** ppComponent,
    bool useConvQueue
    )
{
    TANContextImplPtr contextImpl(pContext);
    *ppComponent = new TANFFTImpl(pContext, useConvQueue);
    (*ppComponent)->Acquire();
    return AMF_OK;
}

//-------------------------------------------------------------------------------------------------
TANFFTImpl::TANFFTImpl(TANContext *pContextTAN, bool useConvQueue) :
    m_pContextTAN(pContextTAN),
    m_eOutputMemoryType(AMF_MEMORY_HOST),
    m_maxFftLog2Len(0),
    m_maxNumChannels(0),
    m_pInputsOCL(nullptr),
    m_pOutputsOCL(nullptr),
    m_useConvQueue(false)
{
    m_useConvQueue = useConvQueue;
    AMFPrimitivePropertyInfoMapBegin
        AMFPropertyInfoEnum(TAN_OUTPUT_MEMORY_TYPE ,  L"Output Memory Type", AMF_MEMORY_HOST, AMF_MEMORY_ENUM_DESCRIPTION, false),
    AMFPrimitivePropertyInfoMapEnd
}
//-------------------------------------------------------------------------------------------------
TANFFTImpl::~TANFFTImpl(void)
{
    Terminate();
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL TANFFTImpl::Init()
{
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
AMF_RESULT  AMF_STD_CALL TANFFTImpl::InitCpu()
{
    /* AMF Initialization */

    AMFLock lock(&m_sect);

    AMF_RETURN_IF_FALSE( (NULL != m_pContextTAN), AMF_WRONG_STATE,
    L"Cannot initialize after termination");

    m_doProcessingOnGpu = false;

    return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL TANFFTImpl::InitGpu()
{
    /* AMF Initialization */

    AMFLock lock(&m_sect);

    AMF_RETURN_IF_FALSE(m_pInputsOCL == nullptr, AMF_ALREADY_INITIALIZED, L"Already initialized");
    AMF_RETURN_IF_FALSE( (nullptr != m_pContextTAN), AMF_WRONG_STATE,
        L"Cannot initialize after termination");

    amf_int64 tmp = 0;
    GetProperty(TAN_OUTPUT_MEMORY_TYPE, &tmp);
    m_eOutputMemoryType = (AMF_MEMORY_TYPE)tmp;

    cl_context context = m_pContextTAN->GetOpenCLContext();
    cl_command_queue cmdQueue = m_pContextTAN->GetOpenCLGeneralQueue();
    
    m_doProcessingOnGpu = (nullptr != context);

    if (m_doProcessingOnGpu){
        cl_int status;
        m_maxNumChannels = 2;
        m_maxFftLog2Len = 12;
        m_pInputsOCL = new cl_mem[m_maxNumChannels];
        m_pOutputsOCL = new cl_mem[m_maxNumChannels];
        for (amf_uint32 i = 0; i < m_maxNumChannels; i++){
            m_pInputsOCL[i] = clCreateBuffer(context, CL_MEM_READ_WRITE, (1i64 << (m_maxFftLog2Len + 1)) * sizeof(float), NULL, &status);
            m_pOutputsOCL[i] = clCreateBuffer(context, CL_MEM_READ_WRITE, (1i64 << (m_maxFftLog2Len + 1)) * sizeof(float), NULL, &status);
            // zero buffers ... shouldn't need this:
            float fill = 0.0;
            status = clEnqueueFillBuffer(cmdQueue, m_pInputsOCL[i], &fill, sizeof(float), 0, (1i64 << (m_maxFftLog2Len + 1)) * sizeof(float), 0, NULL, NULL);
            status = clEnqueueFillBuffer(cmdQueue, m_pOutputsOCL[i], &fill, sizeof(float), 0, (1i64 << (m_maxFftLog2Len + 1)) * sizeof(float), 0, NULL, NULL);

        }
    }

    return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL TANFFTImpl::Terminate()
{
    AMFLock lock(&m_sect);

    m_pContextTAN.Release();

    m_pKernelCopy.Release();

    //release ocl bufers
    if (m_pInputsOCL != nullptr){
		for (amf_uint32 n = 0; n < m_maxNumChannels; n++) {
            clReleaseMemObject(m_pInputsOCL[n]);
            m_pInputsOCL[n] = nullptr;
        }
        delete m_pInputsOCL;
        m_pInputsOCL = nullptr;
    }
    if (m_pOutputsOCL != nullptr){
		for (amf_uint32 n = 0; n < m_maxNumChannels; n++) {
            clReleaseMemObject(m_pOutputsOCL[n]);
            m_pOutputsOCL[n] = nullptr;
        }
        delete m_pOutputsOCL;
        m_pOutputsOCL = nullptr;
    }


    return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL    TANFFTImpl::Transform(
    TAN_FFT_TRANSFORM_DIRECTION direction, 
    amf_uint32 log2len,
    amf_uint32 channels,
    float* ppBufferInput[],
    float* ppBufferOutput[]
)
{
    AMFLock lock(&m_sect);
    AMF_RESULT res = AMF_OK;

    AMF_RETURN_IF_FALSE(ppBufferInput != NULL, AMF_INVALID_ARG, L"pBufferInput == NULL");
    AMF_RETURN_IF_FALSE(ppBufferOutput != NULL, AMF_INVALID_ARG, L"pBufferOutput == NULL");
    AMF_RETURN_IF_FALSE(channels > 0, AMF_INVALID_ARG, L"channels == 0");
    AMF_RETURN_IF_FALSE(log2len > 0, AMF_INVALID_ARG, L"log2len == 0");
    AMF_RETURN_IF_FALSE(log2len < sizeof(amf_size) * 8, AMF_INVALID_ARG, L"log2len is too big");
    AMF_RETURN_IF_FALSE(direction == TAN_FFT_TRANSFORM_DIRECTION_FORWARD ||
        direction == TAN_FFT_TRANSFORM_DIRECTION_BACKWARD,
        AMF_INVALID_ARG, L"Invalid conversion type");

    if (m_doProcessingOnGpu) {

        cl_int status;
        // because we are using complex numbers, we need double length
        log2len++;
        if (channels > m_maxNumChannels || log2len > m_maxFftLog2Len) {
            //release old bufers
            if (m_pInputsOCL != nullptr){
				for (amf_uint32 n = 0; n < m_maxNumChannels; n++) {
                    clReleaseMemObject( m_pInputsOCL[n]);
                    m_pInputsOCL[n] = nullptr;
                }
                delete m_pInputsOCL;
                m_pInputsOCL = nullptr;
            }
            if (m_pOutputsOCL != nullptr){
				for (amf_uint32 n = 0; n < m_maxNumChannels; n++) {
                    clReleaseMemObject(m_pOutputsOCL[n]);
                    m_pOutputsOCL[n] = nullptr;
                }
                delete m_pOutputsOCL;
                m_pOutputsOCL = nullptr;
            }

            // create new buffers
            m_maxNumChannels = channels;
            m_maxFftLog2Len = log2len;
            cl_uint length = (1i64 << m_maxFftLog2Len) * sizeof(float);


            m_pInputsOCL = new cl_mem[m_maxNumChannels];
            m_pOutputsOCL = new cl_mem[m_maxNumChannels];

            cl_context context = m_pContextTAN->GetOpenCLContext();
            cl_command_queue cmdQueue = m_pContextTAN->GetOpenCLGeneralQueue();

			for (amf_uint32 i = 0; i < m_maxNumChannels; i++){
                m_pInputsOCL[i] = clCreateBuffer(context, CL_MEM_READ_WRITE, length, NULL, &status);
                m_pOutputsOCL[i] = clCreateBuffer(context, CL_MEM_READ_WRITE, length, NULL, &status);
                float fill = 0.0;
                status = clEnqueueFillBuffer(cmdQueue, m_pInputsOCL[i], &fill, sizeof(float), 0, length, 0, NULL, NULL);
                status = clEnqueueFillBuffer(cmdQueue, m_pOutputsOCL[i], &fill, sizeof(float), 0, length, 0, NULL, NULL);
            }
        }
        cl_uint length = (1i64 << log2len) * sizeof(float);
        cl_command_queue cmdQueue = static_cast<cl_command_queue>(m_pContextTAN->GetOpenCLGeneralQueue());

		for (amf_uint32 i = 0; i < channels; i++){
            clEnqueueWriteBuffer(cmdQueue, m_pInputsOCL[i], CL_TRUE, 0, length, ppBufferInput[i],0, NULL, NULL);

        }
        
        res = TransformImplGpu(direction, log2len, channels, m_pInputsOCL, m_pOutputsOCL);

		for (amf_uint32 i = 0; i < channels; i++){
            clEnqueueReadBuffer(cmdQueue, m_pOutputsOCL[i], CL_TRUE, 0, length, ppBufferOutput[i],0, NULL, NULL);

        }
        return res;
    }
    
    const amf_size bufferSize = (1 << log2len) * 2; // Double the size for complex.

    // process
    res = TransformImplCpu(direction, log2len, channels, ppBufferInput, ppBufferOutput);
    AMF_RETURN_IF_FAILED(res, L"Transform() failed");

    return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL    TANFFTImpl::Transform(
    TAN_FFT_TRANSFORM_DIRECTION direction, 
    amf_uint32 log2len,
    amf_uint32 channels,
    cl_mem pBufferInput[],
    cl_mem pBufferOutput[]
)
{
    AMF_RETURN_IF_FALSE(pBufferInput != NULL, AMF_INVALID_ARG, L"pBufferInput == NULL");
    AMF_RETURN_IF_FALSE(pBufferOutput != NULL, AMF_INVALID_ARG, L"pBufferOutput == NULL");
    AMF_RETURN_IF_FALSE(channels > 0, AMF_INVALID_ARG, L"channels == 0");
    AMF_RETURN_IF_FALSE(log2len > 0, AMF_INVALID_ARG, L"log2len == 0");
    AMF_RETURN_IF_FALSE(log2len < sizeof(amf_size) * 8, AMF_INVALID_ARG, L"log2len is too big");
    AMF_RETURN_IF_FALSE(direction == TAN_FFT_TRANSFORM_DIRECTION_FORWARD ||
                        direction == TAN_FFT_TRANSFORM_DIRECTION_BACKWARD,
                        AMF_INVALID_ARG, L"Invalid conversion type");

    if (!m_doProcessingOnGpu) {
        return AMF_INVALID_ARG;
    }
    AMFLock lock(&m_sect);
    
    AMF_RESULT res = AMF_OK;
    // process
    res = TransformImplGpu(direction, log2len, channels, pBufferInput, pBufferOutput);
    AMF_RETURN_IF_FAILED(res, L"Transform() failed");

    return AMF_OK;
}

//-------------------------------------------------------------------------------------------------

void *TANFFTImpl::getFFTPlan(int log2len)
{
    clfftStatus status = CLFFT_SUCCESS;

    void * ret = NULL;
 
    cl_context context = static_cast<cl_context>(m_pContextTAN->GetOpenCLContext());
    cl_command_queue cmdQueue;
    if (m_useConvQueue){
        cmdQueue = static_cast<cl_command_queue>(m_pContextTAN->GetOpenCLConvQueue());
    }
    else {
        cmdQueue = static_cast<cl_command_queue>(m_pContextTAN->GetOpenCLGeneralQueue());
    }

    clfftPlanHandle *plan = new clfftPlanHandle;
    const size_t fftLength = 1 << log2len;
    status = clfftCreateDefaultPlan(plan, context, CLFFT_1D, &fftLength);
    assert(status == CLFFT_SUCCESS);

    clfftSetLayout(*plan, CLFFT_COMPLEX_INTERLEAVED, CLFFT_COMPLEX_INTERLEAVED);
    clfftSetResultLocation(*plan, CLFFT_OUTOFPLACE);

    clfftBakePlan(*plan, 1, &cmdQueue, NULL, NULL);

    ret = (void *)plan;


    return ret;
}

//-------------------------------------------------------------------------------------------------

/**************************************************************************************************
FftCPU, adapted from code by Stephan Bernsee

COPYRIGHT 1999-2009 Stephan M. Bernsee
The Wide Open License (WOL)
Permission to use, copy, modify, distribute and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice and this license appear in all source copies.

**************************************************************************************************/
AMF_RESULT AMF_STD_CALL TANFFTImpl::TransformImplCpu(
    TAN_FFT_TRANSFORM_DIRECTION direction,
    amf_size log2len,
    amf_size channels,
    float* ppBufferInput[],
    float* ppBufferOutput[]
    )
{
    const amf_size fftFrameSize = (amf_size)pow(2.0, (double)log2len);

    int sign = (direction == TAN_FFT_TRANSFORM_DIRECTION_FORWARD) ? -1 : 1;

    float wr, wi, arg, *p1, *p2, temp;
    float tr, ti, ur, ui, *p1r, *p1i, *p2r, *p2i;
    amf_uint32 bitm, j, le, le2, k, idx;

    for (amf_size i = 0; i < channels; i++){
        if (ppBufferInput[i] != ppBufferOutput[i]) {
            memcpy(ppBufferOutput[i], ppBufferInput[i], fftFrameSize * 2 * sizeof(float));
        }
    }

    for (amf_size i = 2; i < 2 * fftFrameSize - 2; i += 2) {
        for (bitm = 2, j = 0; bitm < 2 * fftFrameSize; bitm <<= 1) {
            if (i & bitm) j++;
            j <<= 1;
        }
        if (i < j) {
            for (idx = 0; idx < channels; idx++){
                p1 = ppBufferOutput[idx] + i; p2 = ppBufferOutput[idx] + j;
                temp = *p1; *(p1++) = *p2;
                *(p2++) = temp; temp = *p1;
                *p1 = *p2; *p2 = temp;
            }
        }
    }
    for (k = 0, le = 2; k < (amf_size)(log((float)fftFrameSize) / log(2.) + .5); k++) {
        le <<= 1;
        le2 = le >> 1;
        ur = 1.0;
        ui = 0.0;
        arg = (float)(M_PI / (le2 >> 1));
        wr = cos(arg);
        wi = sign*sin(arg);
        for (j = 0; j < le2; j += 2) {
            for (idx = 0; idx < channels; idx++){
                p1r = ppBufferOutput[idx] + j; p1i = p1r + 1;
                p2r = p1r + le2; p2i = p2r + 1;
                for (amf_size i = j; i < 2 * fftFrameSize; i += le) {
                    tr = *p2r * ur - *p2i * ui;
                    ti = *p2r * ui + *p2i * ur;
                    *p2r = *p1r - tr; *p2i = *p1i - ti;
                    *p1r += tr; *p1i += ti;
                    p1r += le; p1i += le;
                    p2r += le; p2i += le;
                }
            }
            tr = ur*wr - ui*wi;
            ui = ur*wi + ui*wr;
            ur = tr;
        }
    }

    // Riemann sum.
    if (direction == TAN_FFT_TRANSFORM_DIRECTION_BACKWARD)
    {
        for (idx = 0; idx < channels; idx++){
            for (int k = 0; k < 2 * fftFrameSize; k++){
                ppBufferOutput[idx][k] /= fftFrameSize;
            }
        }
    }

    return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT AMF_STD_CALL TANFFTImpl::TransformImplGpu(
    TAN_FFT_TRANSFORM_DIRECTION direction,
    amf_size log2len,
    amf_size channels,
    cl_mem pBufferInput[],
    cl_mem pBufferOutput[]
    )
{
    AMF_RETURN_IF_FALSE(m_doProcessingOnGpu, AMF_UNEXPECTED, L"Internal error");

    const amf_size fftFrameSize = (amf_size)pow(2.0, (double)log2len);

    clfftPlanHandle *plan = (clfftPlanHandle *)getFFTPlan(static_cast<int>(log2len));
    cl_command_queue cmdQueue = static_cast<cl_command_queue>(m_pContextTAN->GetOpenCLGeneralQueue());
    for (amf_size ch = 0; ch < channels; ch++)
    {
        clfftStatus status = clfftEnqueueTransform(*plan,
            direction == TAN_FFT_TRANSFORM_DIRECTION_FORWARD ? CLFFT_FORWARD : CLFFT_BACKWARD,
            1, /*num queues and out events*/
            &cmdQueue, /*command queue*/
            0, /*num wait events*/
            NULL, /*wait events*/
            NULL, /*out event*/
            &pBufferInput[ch],/*input buf*/
            &pBufferOutput[ch], /*output buf*/
            NULL /*tmp buffer*/);
        AMF_RETURN_IF_FALSE(status == CLFFT_SUCCESS, AMF_UNEXPECTED);
    }

    return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
