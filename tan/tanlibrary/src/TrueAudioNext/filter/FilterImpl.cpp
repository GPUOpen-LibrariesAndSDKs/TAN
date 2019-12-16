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
#define _USE_MATH_DEFINES
#include <cmath>

#include "FilterImpl.h"
#include "../core/TANContextImpl.h"     //TAN

#include <memory>

#define AMF_FACILITY L"TANFilterImpl"

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
TAN_SDK_LINK AMF_RESULT AMF_CDECL_CALL TANCreateFilter(
    amf::TANContext* pContext, 
    amf::TANFilter** ppComponent
    )
{
    TANContextImplPtr contextImpl(pContext);
    *ppComponent = new TANFilterImpl(pContext);
    (*ppComponent)->Acquire();

    return AMF_OK;
}

//-------------------------------------------------------------------------------------------------
TANFilterImpl::TANFilterImpl(TANContext *pContextTAN) :
    m_pContextTAN(pContextTAN)
{ 

    TANCreateFFT(pContextTAN, &m_pFft);
    m_pFft->Init();
    m_eqFilter = nullptr;
}

//-------------------------------------------------------------------------------------------------
TANFilterImpl::~TANFilterImpl(void)
{
    Terminate();
}

//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL TANFilterImpl::Terminate()
{
    AMFLock lock(&m_sect);

    m_pContextTAN.Release();

    if (m_eqFilter != nullptr){
        delete []m_eqFilter;
        m_eqFilter = nullptr;
    }
    return AMF_OK;
}

//-------------------------------------------------------------------------------------------------
AMF_RESULT  AMF_STD_CALL TANFilterImpl::Init()
{
    AMF_RETURN_IF_FALSE(m_pContextTAN != NULL, AMF_WRONG_STATE,
        L"Cannot initialize after termination");

    //if (m_pContextTAN->GetOpenCLContext())

    AMF_RETURN_IF_FAILED(TANCreateFFT(m_pContextTAN, &m_pFft));
    AMF_RETURN_IF_FAILED(m_pFft->Init());

    m_eqFilter = new float[1 << (EQ_FILTER_LOG2LEN + 1)]; // array of complex numbers

	return AMF_OK;
}



/**************************************************************************************************
freq	-	compute the frequency corresponding to a point in an FFT:
**************************************************************************************************/
float TANFilterImpl::freq(int i, int samplesPerSec, int fftLen)
{
    float f;
    float inDelta = (float)(1.0 / (float)samplesPerSec);

	f = (float)((float)(i) / ((float)fftLen*inDelta));	

    return(f);
}


AMF_RESULT  AMF_STD_CALL TANFilterImpl::generate10BandEQ(amf_uint32 log2len,
    float sampleRate,
    float *impulseResponse,
    float dbLevels[10])
{
    static float centreFreqTab[10] = { 31, 62, 125, 250, 500, 1000, 2000, 4000, 8000, 16000 };
    int impulseLength = 1;
    impulseLength <<= log2len;
    memset(impulseResponse, 0, impulseLength*sizeof(float));

    int fftLen = 1 << (EQ_FILTER_LOG2LEN);

    float *impulse = m_eqFilter;
    memset(impulse, 0, sizeof(float)* fftLen*2);
    impulse[0] = 1.0;

	AMF_RETURN_IF_FAILED(m_pFft->Transform(TAN_FFT_TRANSFORM_DIRECTION_FORWARD, EQ_FILTER_LOG2LEN, 1, &impulse, &impulse) != AMF_OK)

    // filter it...
    int tabIdx = 0;
    float levels[11];
    for (int i = 0; i < 10; i++){
        levels[i] = powf(10.0, float(dbLevels[i] / 20.0));
    }
	levels[10] = levels[9];

    for (int j = 0; j < fftLen/2; j++){
        float f1 = 0.0;
        float f2 = 0.0;
        float l1, l2;
        float f = freq(j, sampleRate, fftLen);
        if (f > centreFreqTab[tabIdx] && tabIdx < 10)
            ++tabIdx;

        if (tabIdx > 0){
            f1 = centreFreqTab[tabIdx-1];
            l1 = levels[tabIdx - 1];
        }
        else {
            f1 = 0;
            l1 = levels[tabIdx];
        }
        f2 = centreFreqTab[tabIdx];
        l2 = levels[tabIdx];

        if (f > f2){
            f2 = f;
        }

        // interpolate:
        float x = (1.0 + cos((f - f1) / (f2 - f1) * 3.141592653589))/2.0;
        float l = l1*x + l2*(1.0 - x);
        impulse[j *2] *= l;
        impulse[j*2 + 1] *= l;

        int k = fftLen - j - 2;

        impulse[k * 2] *= l;
        impulse[k * 2 + 1] *= l;

     }


	AMF_RETURN_IF_FAILED(m_pFft->Transform(TAN_FFT_TRANSFORM_DIRECTION_BACKWARD, EQ_FILTER_LOG2LEN, 1, &impulse, &impulse) != AMF_OK)

    // extract real part of each sample
    for (int k = 0; k < fftLen && k < 2 * impulseLength; k += 2){
            impulseResponse[k >> 1] = impulse[k];
    }

    return AMF_OK;
}
