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
#include <cmath>

#include "IIRfilterImpl.h"
#include "../core/TANContextImpl.h"     //TAN

#include <memory>

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
m_outputHistory(NULL)
{

}

//-------------------------------------------------------------------------------------------------
TANIIRfilterImpl::~TANIIRfilterImpl(void)
{
    if (m_inputTaps != NULL){
        delete [] m_inputTaps;
        m_inputTaps = NULL;
    }
    if (m_outputTaps != NULL){
        delete [] m_outputTaps;
        m_outputTaps = NULL;
    }
    if (m_inputHistory != NULL){
        delete[] m_inputHistory;
        m_inputHistory = NULL;
    }
    if (m_outputHistory != NULL){
        delete[] m_outputHistory;
        m_outputHistory = NULL;
    }

    Terminate();
}

//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL TANIIRfilterImpl::Terminate()
{
    AMFLock lock(&m_sect);

    m_pContextTAN.Release();

    return AMF_OK;
}

//-------------------------------------------------------------------------------------------------
AMF_RESULT	AMF_STD_CALL	TANIIRfilterImpl::Init(
    amf_uint32 numInputTaps,
    amf_uint32 numOutputTaps,
    amf_uint32 channels)
{
    m_numInputTaps = numInputTaps;
    m_numOutputTaps = numOutputTaps;
    m_channels = channels;
    m_inputHistPos = 0;
    m_outputHistPos = 0;

    //Init for CPU:
    if (m_inputTaps != NULL){
        delete [] m_inputTaps;
        m_inputTaps = NULL;
    }
    if (m_outputTaps != NULL){
        delete [] m_outputTaps;
        m_outputTaps = NULL;
    }
    if (m_inputHistory != NULL){
        delete[] m_inputHistory;
        m_inputHistory = NULL;
    }
    if (m_outputHistory != NULL){
        delete[] m_outputHistory;
        m_outputHistory = NULL;
    }


    m_inputTaps = new float *[channels];
    m_outputTaps = new float *[channels];
    m_inputHistory = new float *[channels];
    m_outputHistory = new float *[channels];

    for (int i = 0; i < m_channels; i++){
        m_inputTaps[i] = new float[m_numInputTaps];
        m_outputTaps[i] = new float[m_numOutputTaps];
        m_inputHistory[i] = new float[m_numInputTaps];
        m_outputHistory[i] = new float[m_numOutputTaps];
        memset(m_inputTaps[i], 0, m_numInputTaps*sizeof(float));
        memset(m_outputTaps[i], 0, m_numOutputTaps*sizeof(float));
        memset(m_inputHistory[i], 0, m_numInputTaps*sizeof(float));
        memset(m_outputHistory[i], 0, m_numOutputTaps*sizeof(float));

    }
    return AMF_OK;
}


AMF_RESULT AMF_STD_CALL TANIIRfilterImpl::UpdateIIRResponses(float* ppInputResponse[], float* ppOutputResponse[],
    amf_size inResponseSz, amf_size outResponseSz,
    const amf_uint32 flagMasks[],   // Masks of flags from enum TAN_IIR_CHANNEL_FLAG, can be NULL.
    const amf_uint32 operationFlags // Mask of flags from enum TAN_IIR_OPERATION_FLAG.
    )
{

    if (inResponseSz > m_numInputTaps){
        inResponseSz = m_numInputTaps;
    }
    if (outResponseSz > m_numOutputTaps){
        outResponseSz = m_numOutputTaps;
    }

    for (int i = 0; i < m_channels; i++){
        memset(m_inputTaps[i], 0, m_numInputTaps*sizeof(float));
        memset(m_outputTaps[i], 0, m_numOutputTaps*sizeof(float));

        memcpy(m_inputTaps[i], ppInputResponse[i], inResponseSz*sizeof(float));
        memcpy(m_outputTaps[i], ppOutputResponse[i], outResponseSz*sizeof(float));
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

    float sample;
    for (int sn = 0; sn < numOfSamplesToProcess; sn++){
        // compute next sample for each channel
        for (int chan = 0; chan < m_channels; chan++){
            sample = 0.0;
            m_inputHistory[chan][m_inputHistPos] = ppBufferInput[chan][sn];

            //FIR part
            for (int k = 0; k < m_numInputTaps; k++){
                sample += m_inputTaps[chan][k] * m_inputHistory[chan][(m_inputHistPos + m_numInputTaps - k) % m_numInputTaps];
            }
            //IIR part
            for (int l = 0; l < m_numOutputTaps; l++){
                sample += m_outputTaps[chan][l] * m_outputHistory[chan][(m_outputHistPos + m_numOutputTaps - l) % m_numOutputTaps];
            }
            ppBufferOutput[chan][sn] = sample;
        }

        m_inputHistPos = ++m_inputHistPos % m_numInputTaps;
        m_outputHistPos = ++m_outputHistPos % m_numOutputTaps;

        for (int chan = 0; chan < m_channels; chan++){
            m_outputHistory[chan][m_outputHistPos] = ppBufferOutput[chan][sn];
        }

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

    return AMF_NOT_IMPLEMENTED;
}

