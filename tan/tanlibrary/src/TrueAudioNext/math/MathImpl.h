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
///  @file   MathImpl.h
///  @brief  TANMath interface implementation
///-------------------------------------------------------------------------
#pragma once
#include "tanlibrary/include/TrueAudioNext.h"	//TAN
#include "public/include/core/Context.h"        //AMF
#include "public/include/components/Component.h"//AMF
#include "public/common/PropertyStorageExImpl.h"

namespace amf
{
    class TANMathImpl :
        public virtual AMFInterfaceImpl < AMFPropertyStorageExImpl< TANMath> >    {
    public:
        typedef AMFInterfacePtr_T<TANMathImpl> Ptr;

        TANMathImpl(TANContext *pContextTAN);
        virtual ~TANMathImpl(void);

        // interface access
        AMF_BEGIN_INTERFACE_MAP
            AMF_INTERFACE_CHAIN_ENTRY(AMFInterfaceImpl< AMFPropertyStorageExImpl <TANMath> >)
        AMF_END_INTERFACE_MAP

        //TANMath interface
        virtual AMF_RESULT  AMF_STD_CALL Init();
        virtual AMF_RESULT  AMF_STD_CALL Terminate();
        virtual TANContext* AMF_STD_CALL GetContext()	{ return m_pContextTAN; }

        virtual AMF_RESULT ComplexMultiplication(	const float* const inputBuffers1[],
                                                    const float* const inputBuffers2[],
                                                    float *outputBuffers[],
                                                    amf_uint32 channels,
                                                    amf_size countOfComplexNumbers) override;

        virtual AMF_RESULT ComplexMultiplication(	const cl_mem inputBuffers1[],
                                                    const amf_size buffers1OffsetInSamples[],
                                                    const cl_mem inputBuffers2[],
                                                    const amf_size buffers2OffsetInSamples[],
                                                    cl_mem outputBuffers[],
                                                    const amf_size outputBuffersOffsetInSamples[],
                                                    amf_uint32 channels,
                                                    amf_size countOfComplexNumbers) override;

		virtual AMF_RESULT ComplexMultiplyAccumulate(	const float* const inputBuffers1[], 
														const float* const inputBuffers2[],
														float *accumbuffers[],
														amf_uint32 channels,
														amf_size numOfSamplesToProcess) override;
		virtual AMF_RESULT ComplexMultiplyAccumulate(const cl_mem inputBuffers1[],
													const amf_size buffers1OffsetInSamples[],
													const cl_mem inputBuffers2[],
													const amf_size buffers2OffsetInSamples[],
													cl_mem accumBuffers[],
													const amf_size accumBuffersOffsetInSamples[],
													amf_uint32 channels,
                                                    amf_size countOfComplexNumbers) override;

        virtual AMF_RESULT ComplexDivision(	const float* const inputBuffers1[],
                                            const float* const inputBuffers2[],
                                            float *outputBuffers[],
                                            amf_uint32 channels,
                                            amf_size numOfSamplesToProcess) override;
        virtual AMF_RESULT ComplexDivision(	const cl_mem inputBuffers1[],
                                            const amf_size buffers1OffsetInSamples[],
                                            const cl_mem inputBuffers2[],
                                            const amf_size buffers2OffsetInSamples[],
                                            cl_mem outputBuffers[],
                                            const amf_size outputBuffersOffsetInSamples[],
                                            amf_uint32 channels,
                                            amf_size countOfComplexNumbers) override;

    protected:
        virtual AMF_RESULT ComplexMultiplication(
        	const float inputBuffer1[],
            const float inputBuffer2[],
            float outputBuffer[],
            amf_size countOfComplexNumbers);
		
        virtual AMF_RESULT ComplexMultiplyAccumulate(
			const float inputBuffer1[],
			const float inputBuffer2[],
			float accumBuffer[],
            amf_size countOfComplexNumbers);
        
        virtual AMF_RESULT ComplexMultiplication(
            const cl_mem inputBuffer1,
            const amf_size buffer1OffsetInSamples,
            const cl_mem inputBuffer2,
            const amf_size buffer2OffsetInSamples,
            cl_mem outputBuffer,
            const amf_size outputBufferOffsetInSamples,
            amf_size countOfComplexNumbers);
		
        virtual AMF_RESULT ComplexMultiplyAccumulate(
			const cl_mem inputBuffer1,
			const amf_size buffer1OffsetInSamples,
			const cl_mem inputBuffer2,
			const amf_size buffer2OffsetInSamples,
			cl_mem accumBuffer,
			const amf_size accumBufferOffsetInSamples,
            amf_size countOfComplexNumbers);

        virtual AMF_RESULT ComplexDivision(
            const float inputBuffer1[],
            const float inputBuffer2[],
            float outputBuffer[],
            amf_size countOfComplexNumbers);
        virtual AMF_RESULT ComplexDivision(
            const cl_mem inputBuffer1,
            const amf_size buffer1OffsetInSamples,
            const cl_mem inputBuffer2,
            const amf_size buffer2OffsetInSamples,
            cl_mem outputBuffer,
            const amf_size outputBufferOffsetInSamples,
            amf_size countOfComplexNumbers);

    protected:
        TANContextPtr               m_pContextTAN;
        AMFComputePtr               m_pDeviceCompute;

	
        cl_kernel			m_pKernelComplexDiv = nullptr;
        cl_kernel			m_pKernelComplexMul = nullptr;
		cl_kernel			m_pKernelComplexSum = nullptr;

        AMF_MEMORY_TYPE             m_eOutputMemoryType;
        AMFCriticalSection          m_sect;

        amf_uint32 m_gpuMultiplicationRunNum;
        amf_uint32 m_gpuDivisionRunNum;

		// multiply accumulate internal buffer
		cl_mem	m_pInternalSwapBuffer1_MulAccu;
		cl_mem	m_pInternalSwapBuffer2_MulAccu;
		cl_mem	m_pInternalBufferIn1_MulAccu;
		cl_mem	m_pInternalBufferIn2_MulAccu;
		amf_size m_iInternalSwapBuffer1Size_MulAccu;
		amf_size m_iInternalSwapBuffer2Size_MulAccu;
       	amf_size m_iInternalBufferIn1Size_MulAccu;
		amf_size m_iInternalBufferIn2Size_MulAccu;     
		cl_mem	m_pInternalBufferOut_MulAccu;
		amf_size m_iInternalBufferOutSize_MulAccu;

		// Division internal buffer
		cl_mem	m_pInternalBufferIn1_Division;
		cl_mem	m_pInternalBufferIn2_Division;
		cl_mem	m_pInternalBufferOut_Division;
		amf_size m_iInternalBufferIn1Size_Division;
		amf_size m_iInternalBufferIn2Size_Division;
		amf_size m_iInternalBufferOutSize_Division;

		// Multiplication internal buffer
		cl_mem	m_pInternalBufferIn1_Multiply;
		cl_mem	m_pInternalBufferIn2_Multiply;
		cl_mem	m_pInternalBufferOut_Multiply;
		amf_size m_iInternalBufferIn1Size_Multiply;
		amf_size m_iInternalBufferIn2Size_Multiply;
		amf_size m_iInternalBufferOutSize_Multiply;

    private:
        virtual AMF_RESULT  AMF_STD_CALL InitCpu();
        virtual AMF_RESULT  AMF_STD_CALL InitGpu();
		AMF_RESULT AdjustInternalBufferSize(
			cl_mem* _buffer, 
			amf_size* size, 
			const amf_size requiredSize
		);
    };
} //amf
