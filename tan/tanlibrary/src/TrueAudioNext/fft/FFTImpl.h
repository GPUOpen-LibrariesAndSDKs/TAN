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
#include <unordered_map>
#define MAX_CACHE_POWER 64

namespace amf
{
    class TANFFTImpl
        : public virtual AMFInterfaceImpl < AMFPropertyStorageExImpl< TANFFT> >
    {
    public:
        typedef AMFInterfacePtr_T<TANFFTImpl> Ptr;

        TANFFTImpl(TANContext *pContextTAN, bool useConvQueue);
        virtual ~TANFFTImpl(void);

// interface access
        AMF_BEGIN_INTERFACE_MAP
            AMF_INTERFACE_CHAIN_ENTRY(AMFInterfaceImpl< AMFPropertyStorageExImpl <TANFFT> >)
        AMF_END_INTERFACE_MAP

//TANFFT interface
        AMF_RESULT	AMF_STD_CALL Init()         override;
        AMF_RESULT  AMF_STD_CALL Terminate()    override;
        TANContext* AMF_STD_CALL GetContext()   override    { return m_pContextTAN; }
        
        // log2len - arrays' length should be power of 2, the true lenght is expected to be
        //           (2 ^ log2len) * (2 * sizeof(float)) (due to complex numbers).
        AMF_RESULT  AMF_STD_CALL Transform(TAN_FFT_TRANSFORM_DIRECTION direction,
                                           amf_uint32 log2len,
                                           amf_uint32 channels,
                                           float* ppBufferInput[],
                                           float* ppBufferOutput[]) override;
        AMF_RESULT  AMF_STD_CALL Transform(TAN_FFT_TRANSFORM_DIRECTION direction,
                                           amf_uint32 log2len,
                                           amf_uint32 channels,
                                           cl_mem pBufferInput[],
                                           cl_mem pBufferOutput[]) override;
    private:
		//first 32 bit-> log2length, second 32 bit -> num of channel
		std::unordered_map<amf_uint, size_t> m_pCLFFTHandleMap;

        size_t getFFTPlan(int log2len, int numOfChannels);
        AMF_RESULT virtual AMF_STD_CALL InitCpu();
        AMF_RESULT virtual AMF_STD_CALL InitGpu();

    protected:
        AMF_RESULT virtual AMF_STD_CALL TransformImplCpu(TAN_FFT_TRANSFORM_DIRECTION direction,
                                                         amf_size log2len,
                                                         amf_size channels,
                                                         float* ppBufferInput[],
                                                         float* ppBufferOutput[]);
		AMF_RESULT virtual AMF_STD_CALL TransformImplGPUBatched(
														TAN_FFT_TRANSFORM_DIRECTION direction,
														amf_size log2len,
														amf_size channels,
														cl_mem pBufferInput,
														cl_mem pBufferOutput);
        AMF_RESULT virtual AMF_STD_CALL TransformImplGpu(TAN_FFT_TRANSFORM_DIRECTION direction,
                                                         amf_size log2len,
                                                         amf_size channels,
                                                         cl_mem pBufferInput[],
                                                         cl_mem pBufferOutput[]);

    private:
		void clearInternalBuffers();
		void AdjustInternalBufferSize(size_t desireSizeInSampleLog2, size_t numofChannel);
        TANContextPtr               m_pContextTAN;

        AMFComputeKernelPtr         m_pKernelCopy;
        AMF_MEMORY_TYPE             m_eOutputMemoryType;
        AMFCriticalSection          m_sect;

		amf_size m_iInternalBufferSizeInBytes = 0;
        cl_mem          m_pInputsOCL;
        cl_mem          m_pOutputsOCL;

        bool                        m_doProcessingOnGpu;
        bool                        m_useConvQueue;
    };

    // Internal function used only from TANConvolution class.
    AMF_RESULT TANCreateFFT(amf::TANContext *pContext,
        amf::TANFFT** ppComponent,
        bool useConvQueue);
    ////-------------------------------------------------------------------------------------------------
    //AMF_RESULT TANCreateFFT(
    //    amf::TANContextPtr pContext,
    //    amf::TANFFT **ppComponent,
    //    amf::AMFComputePtr pAmfContext
    //    );
} //amf
