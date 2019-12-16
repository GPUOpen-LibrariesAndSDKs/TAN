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
///-------------------------------------------------------------------------
///  @file   TANConverterImpl.h
///  @brief  TANConverter interface implementation
///-------------------------------------------------------------------------
#pragma once
#include "tanlibrary/include/TrueAudioNext.h"   //TAN
#include "public/include/core/Context.h"        //AMF
#include "public/include/components/Component.h"//AMF
#include "public/common/PropertyStorageExImpl.h"//AMF


#define USE_SSE2 1

namespace amf
{
    class TANConverterImpl
        : public virtual AMFInterfaceImpl < AMFPropertyStorageExImpl< TANConverter> >
    {
    public:
        typedef AMFInterfacePtr_T<TANConverterImpl> Ptr;

        TANConverterImpl(TANContext *pContextTAN, AMFContext* pContextAMF);
        virtual ~TANConverterImpl(void);

// interface access
        AMF_BEGIN_INTERFACE_MAP
            AMF_INTERFACE_CHAIN_ENTRY(AMFInterfaceImpl< AMFPropertyStorageExImpl <TANConverter> >)
        AMF_END_INTERFACE_MAP

//TANConverter interface
        AMF_RESULT  AMF_STD_CALL Init() override;
        AMF_RESULT  AMF_STD_CALL Terminate() override;
        TANContext* AMF_STD_CALL GetContext() override { return m_pContextTAN; }

        AMF_RESULT  AMF_STD_CALL    Convert(short* inputBuffer, amf_size inputStep,
                                            amf_size numOfSamplesToProcess,
                                            float* outputBuffer, amf_size outputStep,
                                            float conversionGain) override;
        AMF_RESULT  AMF_STD_CALL    Convert(float* inputBuffer, amf_size inputStep,
                                            amf_size numOfSamplesToProcess,
                                            short* outputBuffer, amf_size outputStep, 
                                            float conversionGain, bool* outputClipped = NULL) override;

        AMF_RESULT  AMF_STD_CALL    Convert(short** inputBuffers, amf_size inputStep,
                                            amf_size numOfSamplesToProcess,
                                            float** outputBuffers, amf_size outputStep,
                                            float conversionGain, int count) override;

        AMF_RESULT  AMF_STD_CALL    Convert(float** inputBuffers, amf_size inputStep,
                                            amf_size numOfSamplesToProcess,
                                            short** outputBuffers, amf_size outputStep,
                                            float conversionGain, int count, bool* outputClipped = NULL) override;

		AMF_RESULT  AMF_STD_CALL    Convert(cl_mem inputBuffer, amf_size inputStep,
											amf_size inputOffset,TAN_SAMPLE_TYPE inputType, 

                                            cl_mem outputBuffer, amf_size outputOffset, 
                                            amf_size outputStep, TAN_SAMPLE_TYPE outputType,
                                            
                                            amf_size numOfSamplesToProcess,
                                            float conversionGain, bool* outputClipped = NULL) override;

		AMF_RESULT  AMF_STD_CALL    Convert(cl_mem* inputBuffers, amf_size inputStep,
											amf_size* inputOffsets, TAN_SAMPLE_TYPE inputType,

											cl_mem* outputBuffers, amf_size outputStep,
											amf_size* outputOffsets, TAN_SAMPLE_TYPE outputType,

                                            amf_size numOfSamplesToProcess,
                                            float conversionGain,

                                            int count, bool* outputClipped = NULL) override;

    protected:
        TANContextPtr               m_pContextTAN;
        AMFContextPtr               m_pContextAMF;
        AMFComputePtr               m_pDeviceAMF;

        AMFComputeKernelPtr         m_pKernelCopy;
        AMF_MEMORY_TYPE             m_eOutputMemoryType;
        AMFCriticalSection          m_sect;

        cl_command_queue			m_pCommandQueueCl;
        cl_context					m_pContextCl;
        cl_device_id				m_pDeviceCl;

        cl_program					m_program;
        cl_kernel					m_kernel;

		cl_kernel					m_clkFloat2Short = nullptr;
		cl_kernel					m_clkShort2Short = nullptr;
		cl_kernel					m_clkFloat2Float = nullptr;
		cl_kernel					m_clkShort2Float = nullptr;
        cl_mem                      m_overflowBuffer = NULL;
    private:
        static bool useSSE2;
        AMF_RESULT	AMF_STD_CALL InitCpu();
        AMF_RESULT	AMF_STD_CALL InitGpu();
        AMF_RESULT	AMF_STD_CALL ConvertGpu(amf_handle inputBuffer,
                                            amf_size inputOffset,
                                            amf_size inputStep,
                                            TAN_SAMPLE_TYPE inputType,

                                            amf_handle outputBuffer,
                                            amf_size outputOffset,
                                            amf_size outputStep,
                                            TAN_SAMPLE_TYPE outputType,

                                            amf_size numOfSamplesToProcess,
                                            float conversionGain,
                                            bool* outputClipped = NULL);
    };
} //amf
