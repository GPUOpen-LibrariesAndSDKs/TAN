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
///-------------------------------------------------------------------------
///  @file   TANFFTImpl.h
///  @brief  TANFFT interface implementation
///-------------------------------------------------------------------------
#pragma once
#include "tanlibrary/include/TrueAudioNext.h"   //TAN
#include "public/include/core/Context.h"        //AMF
#include "public/include/components/Component.h"//AMF
#include "public/common/PropertyStorageExImpl.h"

#define MAX_CACHE_POWER 64

#define EQ_FILTER_LOG2LEN 13

namespace amf
{
    class TANFilterImpl
        : public virtual AMFInterfaceImpl < AMFPropertyStorageExImpl< TANFilter> >
    {
    public:
        typedef AMFInterfacePtr_T<TANFilterImpl> Ptr;

        TANFilterImpl(TANContext *pContextTAN);
        virtual ~TANFilterImpl(void);

// interface access
        AMF_BEGIN_INTERFACE_MAP
            AMF_INTERFACE_CHAIN_ENTRY(AMFInterfaceImpl< AMFPropertyStorageExImpl <TANFilter> >)
        AMF_END_INTERFACE_MAP

//TANFFT interface
        AMF_RESULT	AMF_STD_CALL Init()         override;
        AMF_RESULT  AMF_STD_CALL Terminate()    override;
        TANContext* AMF_STD_CALL GetContext()   override    { return m_pContextTAN; }
        
        // log2len - arrays' length should be power of 2, the true lenght is expected to be
        //           (2 ^ log2len) * (2 * sizeof(float)) (due to complex numbers).

        virtual AMF_RESULT  AMF_STD_CALL generate10BandEQ(amf_uint32 log2len,
                                                float sampleRate,
                                                float *impulseResponse,
                                                float dbLevels[10]) override;

    protected:


    private:
        float freq(int i, int samplesPerSec, int fftLen);

        TANContextPtr               m_pContextTAN;
        AMFCriticalSection          m_sect;

        TANFFTPtr m_pFft;

        float *m_eqFilter;


    };
} //amf
