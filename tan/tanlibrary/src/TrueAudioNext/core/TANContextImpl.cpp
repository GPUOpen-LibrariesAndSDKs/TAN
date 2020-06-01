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
#include <omp.h>
#include "TANContextImpl.h"
#include "TANTraceAndDebug.h"


#include "tanlibrary/include/TrueAudioNext.h"   //TAN
#include "public/common/TraceAdapter.h"         //AMF
#include "public/common/AMFFactory.h"           //AMF


#include "tanlibrary/src/clFFT-master/src/include/clFFT.h"

#ifndef CLQUEUE_REFCOUNT
#define CLQUEUE_REFCOUNT( clqueue ) { \
		cl_uint refcount = 0; \
		clGetCommandQueueInfo(clqueue, CL_QUEUE_REFERENCE_COUNT, sizeof(refcount), &refcount, NULL); \
		printf("\nFILE:%s line:%d Queue %llX ref count: %d\r\n", __FILE__ , __LINE__, clqueue, refcount); \
}
#endif

#ifndef DBG_CLRELEASE
#define DBG_CLRELEASE( clqueue, qname ) { \
		cl_uint refcount = 0; \
		clReleaseCommandQueue(clqueue); \
		clGetCommandQueueInfo(clqueue, CL_QUEUE_REFERENCE_COUNT, sizeof(refcount), &refcount, NULL); \
		printf("\nFILE:%s line:%d %s %llX ref count: %d\r\n", __FILE__ , __LINE__,qname, clqueue, refcount); \
}
#endif

typedef unsigned int uint;
#include "tanlibrary/src/Graal/GraalConv.hpp"

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

bool g_AMFFactoryInitialized = false;

using namespace amf;

amf_long TANContextImpl::m_clfftReferences = 0;

TAN_SDK_LINK AMF_RESULT        AMF_CDECL_CALL TANCreateContext(
    amf_uint64 version,
    amf::TANContext** ppContext
)
{
    // initialize AMF
    AMF_RESULT res = g_AMFFactory.Init();
    if (AMF_OK != res){
        amf::AMFSetCustomTracer(GetTANTracer());
        amf::AMFSetCustomDebugger(GetTANDebugger());
    }
    else {
        g_AMFFactory.GetDebug()->AssertsEnable(true);
        g_AMFFactory.GetTrace()->EnableWriter(AMF_TRACE_WRITER_FILE, true);
#ifdef _DEBUG
        g_AMFFactory.GetTrace()->SetWriterLevel(AMF_TRACE_WRITER_FILE, AMF_TRACE_TEST);
#else
        g_AMFFactory.GetTrace()->SetWriterLevel(AMF_TRACE_WRITER_FILE, AMF_TRACE_ERROR);
#endif
    }

	//AMF_ASSERT_OK(res, L"AMF Factory Failed to initialize");
	// missing Amf library is informational only, Mac doesn't have amf.
	AMFTraceInfo(L"TANContext", L"AMF Factory Failed to initialize\n");


    //TOD when new version is needed version checking should be extended
    AMF_RETURN_IF_FALSE(
        GET_MAJOR_VERSION(version) == 1, AMF_TAN_UNSUPPORTED_VERSION,
        L"unsupported version %d.%d.%d.%d",
        (int)GET_MAJOR_VERSION(version),
        (int)GET_MINOR_VERSION(version),
        (int)GET_SUBMINOR_VERSION(version),
        (int)GET_BUILD_VERSION(version));

    AMFTraceInfo(L"TANContext", L"TAN SDK version %d.%d.%d.%d\n",
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
    if (!g_AMFFactoryInitialized)
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
    if (!g_AMFFactoryInitialized)
    {
        wprintf(L"AMF Factory NOT initialized");
        return NULL;
    }
    return g_AMFFactory.GetFactory()->GetCacheFolder();
}
//-------------------------------------------------------------------------------------------------

TANContextImpl::TANContextImpl(void)
    : m_clfftInitialized(false),
      m_oclGeneralContext(0),
      m_oclGeneralDeviceId(0),
      m_oclConvContext(0),
      m_oclConvDeviceId(0),
      m_oclGeneralQueue(nullptr),
      m_oclConvQueue(nullptr),
      m_pContextGeneralAMF(nullptr),
      m_pContextConvolutionAMF(nullptr)
{
    AMF_RESULT res;

	// disable OpenMP threading by default:
	omp_set_num_threads(1);

    // Create default CPU AMF context.
    if (g_AMFFactory.GetFactory() != NULL) {
        AMF_ASSERT_OK(g_AMFFactory.GetFactory()->CreateContext(&m_pContextGeneralAMF), L"CreateContext() failed");
        AMF_ASSERT_OK(g_AMFFactory.GetFactory()->CreateContext(&m_pContextConvolutionAMF), L"CreateContext() failed");
    }
}
//-------------------------------------------------------------------------------------------------
TANContextImpl::~TANContextImpl(void)
{
    Terminate();
    m_pContextGeneralAMF.Release();
    m_pContextConvolutionAMF.Release();
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

    //m_oclGeneralContext = 0;
    //m_oclConvContext = 0;

    if (m_oclGeneralQueue)
    {
        DBG_CLRELEASE(m_oclGeneralQueue,"m_oclGeneralQueue");
		m_oclGeneralQueue = NULL;
    }

    if (m_oclConvQueue)
    {
        DBG_CLRELEASE(m_oclConvQueue,"m_oclConvQueue");
		m_oclConvQueue = NULL;
    }

    if (m_oclGeneralContext != 0)
    {
        clReleaseContext(m_oclGeneralContext);
    }
    if (m_oclConvContext != 0)
    {
        clReleaseContext(m_oclConvContext);
    }

    m_oclGeneralDeviceId = 0;
    m_oclConvDeviceId = 0;

   return AMF_OK;
}

bool TANContextImpl::checkOpenCL2_XCompatibility(cl_command_queue cmdQueue)
{
    cl_device_id device_id = 0;
    clGetCommandQueueInfo(cmdQueue, CL_QUEUE_DEVICE, sizeof(device_id), &device_id, NULL);

    char deviceVersion[256];
    memset(deviceVersion, 0, 256);
    clGetDeviceInfo(device_id, CL_DEVICE_VERSION, 256, deviceVersion, NULL);
    cl_device_type clDeviceType  = CL_DEVICE_TYPE_GPU;
    clGetDeviceInfo(device_id, CL_DEVICE_TYPE, sizeof(clDeviceType), &clDeviceType, NULL);

    bool isOpenCL2_XSupported = false;

    int majorRev, minorRev;
    if (sscanf_s(deviceVersion, "OpenCL %d.%d", &majorRev, &minorRev) == 2)
    {
        if (majorRev >= 2) {
            isOpenCL2_XSupported = true;
        }
    }

    // we only care about GPU devices:
    return isOpenCL2_XSupported || (clDeviceType != CL_DEVICE_TYPE_GPU);
}

//-------------------------------------------------------------------------------------------------
AMF_RESULT AMF_STD_CALL TANContextImpl::InitOpenCL(
    cl_context pClContext)
{
    AMF_RETURN_IF_FALSE(m_oclConvContext == 0, AMF_ALREADY_INITIALIZED);
    AMF_RETURN_IF_FALSE(pClContext != nullptr, AMF_INVALID_ARG, L"pClContext == nullptr");

    // Remove default CPU AMF context.
    m_pComputeGeneral.Release();

    // Check context for correctness.
    cl_int deviceCount = 0;
    {

        AMF_RETURN_IF_CL_FAILED(clGetContextInfo(static_cast<cl_context>(pClContext),
                                                 CL_CONTEXT_NUM_DEVICES,
                                                 sizeof(deviceCount), &deviceCount, nullptr),
                                L"pContext is not a valid cl_context");
        AMF_RETURN_IF_FALSE(deviceCount > 0, AMF_INVALID_ARG,
                            L"pContext is not a valid cl_context");
    }

    cl_device_id* devices = new cl_device_id[deviceCount];

    AMF_RETURN_IF_CL_FAILED(clGetContextInfo(static_cast<cl_context>(pClContext),
                                             CL_CONTEXT_DEVICES,
                                             sizeof(cl_device_id)*deviceCount, devices, nullptr),
                            L"could not retrieve the device ids from context");
    cl_int error;
    auto oclConvQueue = clCreateCommandQueue(pClContext, devices[0], NULL, &error);
    AMF_RETURN_IF_FALSE(error == CL_SUCCESS, AMF_FAIL,
                        L"cannot create the conv command queue");

    auto oclGeneralQueue = clCreateCommandQueue(pClContext, devices[0], NULL, &error);
    AMF_RETURN_IF_FALSE(error == CL_SUCCESS, AMF_FAIL,
                        L"cannot create the general command queue");

    for (int idx = 0; idx < deviceCount; idx++)
    {
        if (NULL != devices[idx])
            clReleaseDevice(devices[idx]);
    }

    delete [] devices;

    AMF_RETURN_IF_FALSE(((NULL != oclGeneralQueue) && (NULL != oclConvQueue)), AMF_FAIL, L"Cannot create the queues");
    InitOpenCL(oclGeneralQueue, oclConvQueue);

    return AMF_OK;
}
//-------------------------------------------------------------------------------------------------
AMF_RESULT AMF_STD_CALL TANContextImpl::InitOpenCL(
    cl_command_queue pConvolutionQueue,
    cl_command_queue pGeneralQueue)
{
    AMF_RETURN_IF_FALSE(m_oclConvContext == 0, AMF_ALREADY_INITIALIZED);
    AMF_RETURN_IF_FALSE(InitOpenCLInt(pConvolutionQueue,QueueType::eConvQueue) == AMF_OK, AMF_FAIL, L"Could not initialize using the convolution queue");
    AMF_RETURN_IF_FALSE(InitOpenCLInt(pGeneralQueue, QueueType::eGeneralQueue) == AMF_OK, AMF_FAIL, L"Could not initialize using the general queue")

    // Initialize clFft library here.
    AMF_RETURN_IF_FAILED(InitClfft(), L"Cannot initialize CLFFT");

    return AMF_OK;
}

//-------------------------------------------------------------------------------------------------
AMF_RESULT AMF_STD_CALL TANContextImpl::InitOpenMP(int nThreads)
{
	omp_set_num_threads(nThreads);
	if (omp_get_max_threads() >= nThreads) {
		return AMF_OK;
	}
	omp_set_num_threads(1);
	return AMF_FAIL;
}

//-------------------------------------------------------------------------------------------------
cl_context AMF_STD_CALL TANContextImpl::GetOpenCLContext()
{
    // to do: return both??
    return m_oclConvContext;
}
//-------------------------------------------------------------------------------------------------
cl_command_queue AMF_STD_CALL TANContextImpl::GetOpenCLGeneralQueue()
{
    return m_pComputeGeneral
        ? cl_command_queue(m_pComputeGeneral->GetNativeCommandQueue())
        : m_oclGeneralQueue;
}
//-------------------------------------------------------------------------------------------------
cl_command_queue AMF_STD_CALL TANContextImpl::GetOpenCLConvQueue()
{
    return m_pComputeConvolution
        ? cl_command_queue(m_pComputeConvolution->GetNativeCommandQueue())
        : m_oclConvQueue;
}

//-------------------------------------------------------------------------------------------------
AMF_RESULT amf::TANContextImpl::InitOpenCLInt(cl_command_queue queue, QueueType queueType)
{
    if (!queue)
    {
        return AMF_INVALID_ARG;
    }

    AMFContextPtr& pAMFContext = (queueType == eConvQueue)
        ? m_pContextConvolutionAMF
        : m_pContextGeneralAMF;
    cl_context& clContext = (queueType == eConvQueue)
        ? m_oclConvContext
        : m_oclGeneralContext;
    cl_device_id& device = (queueType == eConvQueue)
        ? m_oclConvDeviceId
        : m_oclGeneralDeviceId;
    cl_command_queue& selectedQueue = (queueType == eConvQueue)
        ? m_oclConvQueue
        : m_oclGeneralQueue;

    AMFComputePtr& pCompute = (queueType == eConvQueue)
        ? m_pComputeConvolution
        : m_pComputeGeneral;

	selectedQueue = queue;

    clGetCommandQueueInfo(selectedQueue, CL_QUEUE_DEVICE, sizeof(device), &device, NULL);
    cl_device_type clDeviceType = CL_DEVICE_TYPE_GPU;
    clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(clDeviceType), &clDeviceType, NULL);
    clGetCommandQueueInfo(selectedQueue, CL_QUEUE_CONTEXT, sizeof(clContext), &clContext, NULL);
    //AMF_RETURN_IF_FALSE(checkOpenCL2_XCompatibility(selectedQueue), AMF_NO_DEVICE, L"Device has no OpenCL 2.0 support.");

    // Setting the AMFContext device type
    if(pAMFContext)
    {
        int amfDeviceType = (clDeviceType == CL_DEVICE_TYPE_GPU) ? AMF_CONTEXT_DEVICE_TYPE_GPU : AMF_CONTEXT_DEVICE_TYPE_CPU;
        pAMFContext->SetProperty(AMF_CONTEXT_DEVICE_TYPE, amfDeviceType);

        // Initializing the AMFContexts, and getting the AMFCompute from it
        AMFCompute* pAMFCompute = NULL;
        AMF_RESULT res = pAMFContext->InitOpenCL(selectedQueue);
        AMF_RETURN_IF_FAILED(res, L"InitOpenCL() failed");
        pAMFContext->GetCompute(AMF_MEMORY_OPENCL, &pAMFCompute);
        AMF_RETURN_IF_FALSE(pAMFCompute != NULL, AMF_FAIL, L"Could not get the AMFCompute.");
        pCompute = pAMFCompute;
	}
	else
    {
        clRetainCommandQueue(selectedQueue);
	}
	CLQUEUE_REFCOUNT(selectedQueue);

    return AMF_OK;
}

//-------------------------------------------------------------------------------------------------

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
