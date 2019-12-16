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
///  @file   TANContextImpl.h
///  @brief  AMFContext interface implementation
///-------------------------------------------------------------------------
#pragma once

#include "tanlibrary/include/TrueAudioNext.h"   //TAN
#include "public/common/PropertyStorageImpl.h"  //AMF
#include "public/include/core/Context.h"        //AMF

#include <CL/cl.h>

namespace amf
{
    class TANContextImpl :
        public AMFInterfaceImpl < AMFPropertyStorageImpl <TANContext> >
    {
    public:
        TANContextImpl(void);
        virtual ~TANContextImpl(void);

        AMF_BEGIN_INTERFACE_MAP
            AMF_INTERFACE_ENTRY(TANContext)
            AMF_INTERFACE_CHAIN_ENTRY(AMFInterfaceImpl< AMFPropertyStorageImpl <TANContext> >)
            AMF_END_INTERFACE_MAP

            //TANContext interface
            AMF_RESULT AMF_STD_CALL Terminate() override;
        AMF_RESULT AMF_STD_CALL InitOpenCL(cl_context pContext) override;
        AMF_RESULT AMF_STD_CALL InitOpenCL(cl_command_queue pGeneralQueue,
            cl_command_queue pConvolutionQueue) override;

        cl_context   AMF_STD_CALL GetOpenCLContext() override;
        cl_command_queue	AMF_STD_CALL	GetOpenCLGeneralQueue() override;
        cl_command_queue	AMF_STD_CALL	GetOpenCLConvQueue() override;

		AMF_RESULT AMF_STD_CALL InitOpenMP(int nThreads) override;

        // Internal methods.
        ////TODO:AA AMFContextPtr GetGeneralContext() const       { return m_pContextAMF; }
        AMFComputePtr GetGeneralCompute() const       { return m_pComputeGeneral; }
        AMFComputePtr GetConvolutionCompute() const   { return m_pComputeConvolution; }

    protected:
        enum QueueType { eConvQueue, eGeneralQueue };
        virtual AMF_RESULT InitOpenCLInt(cl_command_queue pClCommandQueue, QueueType queueType);

        virtual AMF_RESULT InitClfft();

        bool checkOpenCL2_XCompatibility(cl_command_queue cmdQueue);

    private:
        cl_context                  m_oclGeneralContext;
        cl_context                  m_oclConvContext;
        cl_command_queue			m_oclGeneralQueue;
        cl_command_queue			m_oclConvQueue;
        cl_device_id                m_oclGeneralDeviceId;
        cl_device_id                m_oclConvDeviceId;

        AMFContextPtr               m_pContextGeneralAMF;
        AMFContextPtr               m_pContextConvolutionAMF;
        AMFComputePtr               m_pComputeGeneral;
        AMFComputePtr               m_pComputeConvolution;
        AMFComputeDevicePtr         m_pGeneralDeviceAMF;
        AMFComputeDevicePtr         m_pConvolutionDeviceAMF;



        bool m_clfftInitialized;
        static amf_long m_clfftReferences; // Only one instance of the library can exist at a time.


#if AMF_BUILD_OPENCL
        AMFDeviceComputePtr m_pDeviceOpenCL;
#endif //AMF_BUILD_OPENCL

#if AMF_BUILD_MCL && AMF_BUILD_DIRECTX9
        AMFDeviceComputePtr m_pDeviceMCL_DX9;
#endif //AMF_BUILD_MCL && AMF_BUILD_DIRECTX9

#if AMF_BUILD_MCL && AMF_BUILD_DIRECTX11
        AMFDeviceComputePtr m_pDeviceMCL_DX11;
#endif //AMF_BUILD_MCL && AMF_BUILD_DIRECTX11

        AMFCriticalSection m_sync;
    };
    typedef AMFInterfacePtr_T<TANContextImpl> TANContextImplPtr;

} // namespace amf
