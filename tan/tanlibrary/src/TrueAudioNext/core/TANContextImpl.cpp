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

#include "TANContextImpl.h"
#include "TANTraceAndDebug.h"


#include "tanlibrary/include/TrueAudioNext.h"   //TAN
#include "public/common/TraceAdapter.h"         //AMF
#include "public/common/AMFFactory.h"           //AMF


#include "tanlibrary/src/clFFT-master/src/include/clfft.h"

typedef unsigned int uint;
#include "tanlibrary/src/Graal/graalConv.hpp"

#ifndef MAKE_FULL_VERSION
#define MAKE_FULL_VERSION(VERSION_MAJOR, VERSION_MINOR, VERSION_RELEASE, VERSION_BUILD_NUM)    ( (amf_uint64(VERSION_MAJOR) << 48ull) | (amf_uint64(VERSION_MINOR) << 32ull) | (amf_uint64(VERSION_RELEASE) << 16ull)  | amf_uint64(VERSION_BUILD_NUM))
#endif
#ifndef GET_MAJOR_VERSION
#define GET_MAJOR_VERSION(x)      ((x >> 48ull) & 0xFFFF)
#endif
#ifndef GET_MINOR_VERSION
#define GET_MINOR_VERSION(x)      ((x >> 32ull) & 0xFFFF)
#endif
#ifndef GET_SUBMINOR_VERSION
#define GET_SUBMINOR_VERSION(x)   ((x >> 16ull) & 0xFFFF)
#endif
#ifndef GET_BUILD_VERSION
#define GET_BUILD_VERSION(x)      ((x >>  0ull) & 0xFFFF)
#endif

BOOL g_AMFFactoryInitialized = FALSE;

using namespace amf;

amf_long TANContextImpl::m_clfftReferences = 0;

TAN_SDK_LINK AMF_RESULT        AMF_CDECL_CALL TANCreateContext(
    amf_uint64 version,
    amf::TANContext** ppContext
)
{
    amf::AMFSetCustomTracer(GetTANTracer());
    amf::AMFSetCustomDebugger(GetTANDebugger());

    //TOD when new version is needed vesion chacing should be extended
    AMF_RETURN_IF_FALSE(
        GET_MAJOR_VERSION(version) == 1, AMF_TAN_UNSUPPORTED_VERSION, 
        L"unsupported version %d.%d.%d.%d",
        (int)GET_MAJOR_VERSION(version),
        (int)GET_MINOR_VERSION(version),
        (int)GET_SUBMINOR_VERSION(version),
        (int)GET_BUILD_VERSION(version));

    *ppContext = new TANContextImpl();
    AMF_RETURN_IF_FALSE((NULL != *ppContext), AMF_NOT_INITIALIZED, L"TANContext == NULL");
    (*ppContext)->Acquire();
    return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
TAN_SDK_LINK AMF_RESULT        AMF_CDECL_CALL TANSetCacheFolder(const wchar_t* path)
{
    if (FALSE == g_AMFFactoryInitialized)
    {
        wprintf(L"AMF Factory NOT initialized");
        return AMF_NOT_INITIALIZED;
    }

    // component: converter
    AMF_RESULT res = g_AMFFactory.GetFactory()->SetCacheFolder(path);
    return res;
}
//-------------------------------------------------------------------------------------------------
TAN_SDK_LINK const wchar_t*    AMF_CDECL_CALL TANGetCacheFolder()
{
    if (FALSE == g_AMFFactoryInitialized)
    {
        wprintf(L"AMF Factory NOT initialized");
        return NULL;
    }
    return g_AMFFactory.GetFactory()->GetCacheFolder();
}
//-------------------------------------------------------------------------------------------------

TANContextImpl::TANContextImpl(void)
    : m_clfftInitialized(false),
      m_oclContext(0),
      m_oclDeviceId(0),
      m_oclGeneralQueue(nullptr),
      m_oclConvQueue(nullptr),
      m_pContextAMF(nullptr)
{
    // initialize AMF
    AMF_RESULT res = g_AMFFactory.Init();
    AMF_ASSERT_OK(res, L"AMF Factory Failed to initialize");
    if (AMF_OK != res)
        return;

    // Create default CPU AMF context.
    res = g_AMFFactory.GetFactory()->CreateContext(&m_pContextAMF);
    AMF_ASSERT_OK(res, L"CreateContext() failed");
}
//-------------------------------------------------------------------------------------------------
TANContextImpl::~TANContextImpl(void)
{
    Terminate();
    m_pContextAMF.Release();
    g_AMFFactory.Terminate();
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT AMF_STD_CALL TANContextImpl::Terminate()
{
    // Destroy clFft library.
    if (m_clfftInitialized && amf_atomic_dec(&m_clfftReferences) == 0)
    {
        clfftTeardown();
    }
    m_clfftInitialized = false;

    // Terminate AMF contexts.
    m_pComputeGeneral.Release();
    m_pComputeConvolution.Release();
    m_pDeviceAMF.Release();

    m_oclContext = 0;
    m_oclDeviceId = 0;
    return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT AMF_STD_CALL TANContextImpl::InitOpenCL(
    cl_context pClContext)
{
    AMF_RETURN_IF_FALSE(m_oclContext == 0, AMF_ALREADY_INITIALIZED);
    AMF_RETURN_IF_FALSE(pClContext != nullptr, AMF_INVALID_ARG, L"pClContext == nullptr");

    // Remove default CPU AMF context.
    m_pComputeGeneral.Release();

    // Check context for correctness.
    {
        cl_int deviceCount = 0;
        AMF_RETURN_IF_CL_FAILED(clGetContextInfo(static_cast<cl_context>(pClContext),
            CL_CONTEXT_NUM_DEVICES,
            sizeof(deviceCount), &deviceCount, nullptr),
            L"pContext is not a valid cl_context");
        AMF_RETURN_IF_FALSE(deviceCount > 0, AMF_INVALID_ARG,
            L"pContext is not a valid cl_context");
    }

    AMF_RETURN_IF_FAILED(CreateCompute(&m_pComputeGeneral,
        nullptr));
    m_oclDeviceId = static_cast<cl_device_id>(m_pComputeGeneral->GetNativeDeviceID());

    AMF_RETURN_IF_FAILED(CreateCompute(&m_pComputeConvolution,
        nullptr));

    // Initialize clFft library here.
    AMF_RETURN_IF_FAILED(InitClfft(), L"Cannot initialize CLFFT");

    // Finish initialization (used as flag).

    m_oclContext = static_cast<cl_context>(m_pContextAMF->GetOpenCLContext());
    m_oclGeneralQueue = static_cast<cl_command_queue>(m_pComputeGeneral->GetNativeCommandQueue());
    m_oclConvQueue = static_cast<cl_command_queue>(m_pComputeConvolution->GetNativeCommandQueue());
    return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT AMF_STD_CALL TANContextImpl::InitOpenCL(
    cl_command_queue pConvolutionQueue,
    cl_command_queue pGeneralQueue)
{
    AMF_RETURN_IF_FALSE(m_oclContext == 0, AMF_ALREADY_INITIALIZED);

    // Remove default CPU AMF context and OpenCL queue.
    m_pComputeGeneral.Release();

    // Keep provided queue.
    m_oclGeneralQueue = pGeneralQueue;
    m_oclConvQueue = pConvolutionQueue;

    // Create AMFContexts.
    cl_context pClContext = nullptr;
    cl_device_id pClDeviceId = nullptr;
    if (m_oclGeneralQueue)
    {
        AMF_RETURN_IF_FAILED(CreateCompute(&m_pComputeGeneral,
            static_cast<cl_command_queue>(m_oclGeneralQueue)));
        pClContext = static_cast<cl_context>(m_pComputeGeneral->GetNativeContext());
        m_oclDeviceId = static_cast<cl_device_id>(m_pComputeGeneral->GetNativeDeviceID());

        AMF_RETURN_IF_FAILED(CreateCompute(&m_pComputeConvolution,
            static_cast<cl_command_queue>(m_oclConvQueue)));
    }
    else {
        AMF_RETURN_IF_FAILED(CreateCompute(&m_pComputeConvolution,
            static_cast<cl_command_queue>(m_oclConvQueue)));
        pClContext = static_cast<cl_context>(m_pComputeConvolution->GetNativeContext());
        m_oclDeviceId = static_cast<cl_device_id>(m_pComputeConvolution->GetNativeDeviceID());

        AMF_RETURN_IF_FAILED(CreateCompute(&m_pComputeGeneral,
            static_cast<cl_command_queue>(m_oclGeneralQueue)));
    }

    // Initialize clFft library here.
    AMF_RETURN_IF_FAILED(InitClfft(), L"Cannot initialize CLFFT");

    // Finish initialization (used as flag).
    m_oclContext = static_cast<cl_context>(pClContext);

#define FULL_VERSION(VERSION_MAJOR, VERSION_MINOR, VERSION_RELEASE, VERSION_BUILD_NUM) ( (amf_uint64(VERSION_MAJOR) << 48ull) | (amf_uint64(VERSION_MINOR) << 32ull) | (amf_uint64(VERSION_RELEASE) << 16ull)  | amf_uint64(VERSION_BUILD_NUM))

    if (g_AMFFactory.AMFQueryVersion() <= FULL_VERSION(1,3,0,4) )
    {
        clRetainCommandQueue(m_oclGeneralQueue);
        clRetainCommandQueue(m_oclConvQueue);
    }

    return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
cl_context AMF_STD_CALL TANContextImpl::GetOpenCLContext()
{
    return m_oclContext;
}
//-------------------------------------------------------------------------------------------------
cl_command_queue AMF_STD_CALL TANContextImpl::GetOpenCLGeneralQueue()
{
    return m_oclGeneralQueue;
}
//-------------------------------------------------------------------------------------------------
cl_command_queue AMF_STD_CALL TANContextImpl::GetOpenCLConvQueue()
{
    return m_oclConvQueue;
}

//-------------------------------------------------------------------------------------------------
AMF_RESULT amf::TANContextImpl::InitOpenCLInt(cl_command_queue pClCommandQueue)
{
    if (nullptr == m_pContextAMF)
    {
        return AMF_NO_INTERFACE;
    }
    if (m_pContextAMF->GetOpenCLCommandQueue() != NULL)
    {
        return AMF_OK;
    }
    AMF_RESULT res = AMF_FAIL;


    if (pClCommandQueue != NULL)
    {
        res = m_pContextAMF->InitOpenCL(pClCommandQueue);
        AMF_RETURN_IF_FAILED(res, L"InitOpenCL() failed");
        AMFComputeFactoryPtr pOCLFactory;
        res = m_pContextAMF->GetOpenCLComputeFactory(&pOCLFactory);
        AMF_RETURN_IF_FAILED(res, L"GetOpenCLComputeFactory() failed");

        amf_int32 deviceCount = pOCLFactory->GetDeviceCount();
        for (amf_int32 i = 0; i < deviceCount; i++)
        {
            AMFComputeDevicePtr         pDeviceAMF;
            res = pOCLFactory->GetDeviceAt(i, &pDeviceAMF);
            if (m_pContextAMF->GetOpenCLDeviceID() == pDeviceAMF->GetNativeDeviceID())
            {
                m_pDeviceAMF = pDeviceAMF;
                int streams = 0;
                m_pDeviceAMF->GetProperty(AMF_AUDIO_CONVOLUTION_MAX_STREAMS, &streams);
                break; //TODO:AA
            }
        }
    }
    else
    {
        AMFComputeFactoryPtr pOCLFactory;
        res = m_pContextAMF->GetOpenCLComputeFactory(&pOCLFactory);
        AMF_RETURN_IF_FAILED(res, L"GetOpenCLComputeFactory() failed");

        amf_int32 deviceCount = pOCLFactory->GetDeviceCount();
        for (amf_int32 i = 0; i < deviceCount; i++)
        {
            res = pOCLFactory->GetDeviceAt(i, &m_pDeviceAMF);
            int streams = 0;
            m_pDeviceAMF->GetProperty(AMF_AUDIO_CONVOLUTION_MAX_STREAMS, &streams);
            break; //TODO:AA
        }
        m_pContextAMF->InitOpenCLEx(m_pDeviceAMF);
    }

    // Check for the device to be OpenCL 2.x compatible.
    if (!m_oclDeviceId)
    {
        cl_device_id oclDeviceId = static_cast<cl_device_id>(m_pDeviceAMF->GetNativeDeviceID());
        size_t paramSize;
        cl_int status = clGetDeviceInfo(
            oclDeviceId,
            CL_DEVICE_VERSION,
            0,
            NULL,
            &paramSize);
        AMF_RETURN_IF_CL_FAILED(status, L"clGetDeviceInfo(CL_DRIVER_VERSION) failed");

        char *clVersionStr = static_cast<char*>(_alloca(paramSize));
        status = clGetDeviceInfo(
            oclDeviceId,
            CL_DEVICE_VERSION,
            paramSize,
            clVersionStr,
            &paramSize);
        AMF_RETURN_IF_CL_FAILED(status, L"clGetDeviceInfo(CL_DRIVER_VERSION) failed");

        int majorRev, minorRev;
        AMF_RETURN_IF_FALSE(sscanf_s(clVersionStr, "OpenCL %d.%d", &majorRev, &minorRev) == 2,
            AMF_UNEXPECTED, L"OpenCL version string has unexpected format");
        AMF_RETURN_IF_FALSE(majorRev >= 2, AMF_OPENCL_FAILED,
            L"Unsupported graphics device. "
            L"Required CL_DEVICE_OPENCL_C_VERSION 2.0 or higher");
    }
    return AMF_OK;
}

//-------------------------------------------------------------------------------------------------
AMF_RESULT amf::TANContextImpl::CreateCompute(
    AMFCompute** pCompute,
    cl_command_queue pClCommandQueue,
    amf_uint32 reservedCuCount
    )
{
    AMF_RESULT res = AMF_FAIL;
    res = InitOpenCLInt(pClCommandQueue);
    AMF_RETURN_IF_FAILED(res, L"InitOpenCLInt() failed");

    if (pClCommandQueue == NULL)
    {
        res = m_pDeviceAMF->CreateCompute(NULL, pCompute); // reserved
    }
    else
    {
        res = m_pDeviceAMF->CreateComputeEx(pClCommandQueue, pCompute); // reserved
    }
    AMF_RETURN_IF_FAILED(res, L"GetCompute() failed");
    return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
#if 0
    if (pClContext && !pClCommandQueue)
    {
        cl_device_id pClDevice = m_oclDeviceId;
        if (!m_oclDeviceId)
        {
            cl_uint devicesCnt;
            AMF_RETURN_IF_CL_FAILED(clGetContextInfo(pClContext, CL_CONTEXT_NUM_DEVICES,
                                                     sizeof(devicesCnt), &devicesCnt, nullptr),
                                    L"clGetContextInfo() failed");

            amf_vector<cl_device_id> devices(devicesCnt);
            AMF_RETURN_IF_CL_FAILED(clGetContextInfo(pClContext, CL_CONTEXT_DEVICES,
                                                     devices.size() * sizeof(cl_device_id),
                                                     &devices[0], nullptr),
                                    L"clGetContextInfo() failed");

            pClDevice = devices[0];
        }

        auto profileEvents = 0; //TODO:AA AMFPerformanceMonitorLogger::Get().IsMonitoring();
        cl_command_queue_properties clQueueProp = profileEvents ? CL_QUEUE_PROFILING_ENABLE : 0;
        cl_int status = CL_SUCCESS;
        pClCommandQueue = clCreateCommandQueue(pClContext, pClDevice, clQueueProp, &status);
        AMF_RETURN_IF_CL_FAILED(status, L"CreateOpenCLCommandQueue::clCreateCommandQueue failed");
    }

    //////////////////////////////
    // TODO need to change interface and provide queue

    AMF_RESULT res = AMF_FAIL;
    //    m_pContextAMF->InitOpenCL(NULL);
    AMFComputeFactoryPtr pOCLFactory;
    res = m_pContextAMF->GetOpenCLComputeFactory(&pOCLFactory);
    AMF_RETURN_IF_FAILED(res, L"GetOpenCLComputeFactory() failed");

    amf_int32 deviceCount = pOCLFactory->GetDeviceCount();
    AMFComputeDevicePtr pDeviceAMF;
    for (amf_int32 i = 0; i < deviceCount; i++)
    {
        res = pOCLFactory->GetDeviceAt(i, &pDeviceAMF);
        int streams = 0;
        pDeviceAMF->GetProperty(AMF_AUDIO_CONVOLUTION_MAX_STREAMS, &streams);
        break; //TODO:AA
    }

#if 1   //TODO:AA init opencl by given command queue
    res = m_pContextAMF->InitOpenCL(pClCommandQueue);
    AMF_RETURN_IF_FAILED(res, L"InitOpenCL() failed");
#else   //TODO:AA init opencl by special device
    res = m_pContextAMF->InitOpenCLEx(pDeviceAMF);
    AMF_RETURN_IF_FAILED(res, L"InitOpenCLEx() failed");
#endif

    res = pDeviceAMF->CreateCompute(NULL, &pCompute); // reserved
    AMF_RETURN_IF_FAILED(res, L"GetCompute() failed");
    /*  //TODO:AA 
    // Create context on its base.
    AMF_RETURN_IF_FAILED(AMFCreateContext(&pContext), L"Cannot create context");
    AMF_RETURN_IF_FAILED(pContext->InitOpenCL(pClCommandQueue), L"AMFContext::InitOpenCL failed");
    */

    // Check if new cl_command_queue belongs to the same cl_context.
    AMF_RETURN_IF_FALSE(!pClContext || !pClCommandQueue ||
                        pClContext == pCompute->GetNativeContext(), AMF_INVALID_ARG,
                        L"cl_command_queue provided belongs to different cl_context");

    if (pClContext && !pClCommandQueue)
    {
        AMF_RETURN_IF_CL_FAILED(clReleaseCommandQueue(pClCommandQueue),
                                 L"clReleaseCommandQueue() failed");
    }

    // Check for the device to be OpenCL 2.x compatible.
    if (!m_oclDeviceId)
    {
        cl_device_id oclDeviceId = static_cast<cl_device_id>(pCompute->GetNativeDeviceID());
        size_t paramSize;
        cl_int status = clGetDeviceInfo(
            oclDeviceId,
            CL_DEVICE_VERSION,
            0,
            NULL,
            &paramSize);
        AMF_RETURN_IF_CL_FAILED(status, L"clGetDeviceInfo(CL_DRIVER_VERSION) failed");

        char *clVersionStr = static_cast<char*>(_alloca(paramSize));
        status = clGetDeviceInfo(
            oclDeviceId,
            CL_DEVICE_VERSION,
            paramSize,
            clVersionStr,
            &paramSize);
        AMF_RETURN_IF_CL_FAILED(status, L"clGetDeviceInfo(CL_DRIVER_VERSION) failed");

        int majorRev, minorRev;
        AMF_RETURN_IF_FALSE(sscanf_s(clVersionStr, "OpenCL %d.%d", &majorRev, &minorRev) == 2,
                            AMF_UNEXPECTED, L"OpenCL version string has unexpected format");
        AMF_RETURN_IF_FALSE(majorRev >= 2, AMF_OPENCL_FAILED,
                            L"Unsupported graphics device. "
                            L"Required CL_DEVICE_OPENCL_C_VERSION 2.0 or higher");
    }

    return AMF_OK;
}
#endif
//-------------------------------------------------------------------------------------------------
AMF_RESULT amf::TANContextImpl::InitClfft()
{
    clfftSetupData setupData;
    AMF_RETURN_IF_FALSE(clfftInitSetupData(&setupData) == CLFFT_SUCCESS, AMF_UNEXPECTED,
                        L"Cannot initialize FFT component");
    AMF_RETURN_IF_FALSE(clfftSetup(&setupData) == CLFFT_SUCCESS, AMF_UNEXPECTED,
                        L"Cannot setup FFT component");
    amf_atomic_inc(&m_clfftReferences);
    m_clfftInitialized = true;

    return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
