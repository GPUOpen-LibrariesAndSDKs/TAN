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
#include <stdio.h>
#include <string.h>
#include <cstring>

#ifdef _WIN32
#include <io.h>
#endif
#include <CL/cl.h>

#include "public/include/core/Variant.h"        //AMF
#include "public/include/core/Context.h"        //AMF
#include "public/include/core/ComputeFactory.h" //AMF
#include "public/common/AMFFactory.h"           //AMF

#ifdef RTQ_ENABLED
#define AMFQUEPROPERTY L"MaxRealTimeComputeUnits"
#else
#define AMFQUEPROPERTY L"None"
#endif

#include "../GPUUtilities/GpuUtilities.h"

/**
*******************************************************************************
* @fn listGpuDeviceNames
* @brief returns list of installed GPU devices
*
* @param[out] devNames    : Points to empty string array to return device names
* @param[in] count		  : length of devNames

* @return INT number of strings written to devNames array. (<= count)
*         
*******************************************************************************
*/
int listGpuDeviceNamesWrapper(char *devNames[], unsigned int count) {

    int foundCount = 0;

    
    AMF_RESULT res = g_AMFFactory.Init();   // initialize AMF
    if (AMF_OK == res)
    {
        // Create default CPU AMF context.
        amf::AMFContextPtr pContextAMF = NULL;
        res = g_AMFFactory.GetFactory()->CreateContext(&pContextAMF);

        if (AMF_OK == res)
        {
            amf::AMFComputeFactoryPtr pOCLFactory = NULL;
            res = pContextAMF->GetOpenCLComputeFactory(&pOCLFactory);
            if (AMF_OK == res)
            {
                amf_int32 deviceCount = pOCLFactory->GetDeviceCount();
                for (amf_int32 i = 0; i < deviceCount; i++)
                {
                    amf::AMFComputeDevicePtr         pDeviceAMF;
                    res = pOCLFactory->GetDeviceAt(i, &pDeviceAMF);
                    {
                        amf::AMFVariant pName;
                        res = pDeviceAMF->GetProperty(AMF_DEVICE_NAME, &pName);
                        if (AMF_OK == res)
                        {
                            std::strcpy(devNames[i], pName.stringValue);
                            foundCount++;
                        }
                    }
                }
            }
            pOCLFactory.Release();
        }
        pContextAMF.Release();
        g_AMFFactory.Terminate();
    }
    else  //USE OpenCL wrapper
    {
        #ifdef _WIN32
        HMODULE GPUUtilitiesDll = NULL;
        typedef int(__cdecl *listGpuDeviceNamesType)(char *devNames[], unsigned int count);
        listGpuDeviceNamesType listGpuDeviceNames = nullptr;

        GPUUtilitiesDll = LoadLibraryA("GPUUtilities.dll");
        if (NULL != GPUUtilitiesDll)
        {
            listGpuDeviceNames = (listGpuDeviceNamesType)GetProcAddress(GPUUtilitiesDll, "listGpuDeviceNames");
            if (NULL != listGpuDeviceNames)
            {
                foundCount = listGpuDeviceNames(devNames, count);
            }
            else
            {
                MessageBoxA(NULL, "NOT FOUND listGpuDeviceNames", "GPUUtils...", MB_ICONERROR);
            }
        }
        else
        {
            MessageBoxA( NULL, "NOT FOUND GPUUtilities.dll", "GPUUtils...", MB_ICONERROR );
        }
        #else
        foundCount = listGpuDeviceNames(devNames, count);
        #endif
    }

    return foundCount;
}

/**
*******************************************************************************
* @fn listCpuDeviceNames
* @brief returns list of installed CPU devices
*
* @param[out] devNames    : Points to empty string array to return device names
* @param[in] count		  : length of devNames

* @return INT number of strings written to devNames array. (<= count)
*
*******************************************************************************
*/
int listCpuDeviceNamesWrapper(char *devNames[], unsigned int count) {

    int foundCount = 0;
    AMF_RESULT res = g_AMFFactory.Init();   // initialize AMF
    if (AMF_OK == res)
    {
        // Create default CPU AMF context.
        amf::AMFContextPtr pContextAMF = NULL;
        res = g_AMFFactory.GetFactory()->CreateContext(&pContextAMF);
        amf::AMFVariantStruct value;

        //Hack To Do: define AMFContext property enum and use that instead of passing actual clDevice type
        // e.g. pContextAMF->SetProperty(L"AMF_CONTEXT_DEVICETYPE", AMF_CONTEXT_DEVICETYPE_GPU);

        pContextAMF->SetProperty(AMF_CONTEXT_DEVICE_TYPE, AMF_CONTEXT_DEVICE_TYPE_CPU);

        if (AMF_OK == res)
        {
            amf::AMFComputeFactoryPtr pOCLFactory = NULL;
            res = pContextAMF->GetOpenCLComputeFactory(&pOCLFactory);
            if (AMF_OK == res)
            {
                amf_int32 deviceCount = pOCLFactory->GetDeviceCount();
                for (amf_int32 i = 0; i < deviceCount; i++)
                {
                    amf::AMFComputeDevicePtr         pDeviceAMF;
                    res = pOCLFactory->GetDeviceAt(i, &pDeviceAMF);
                    {
                        amf::AMFVariant pName;
                        res = pDeviceAMF->GetProperty(AMF_DEVICE_NAME, &pName);
                        if (AMF_OK == res)
                        {
                            std::strcpy(devNames[i], pName.stringValue);
                            foundCount++;
                        }
                    }
                }
            }
            pOCLFactory.Release();
        }
        pContextAMF.Release();
        g_AMFFactory.Terminate();
    } else
    {
        #ifdef _WIN32
        HMODULE GPUUtilitiesDll = NULL;
        typedef int(__cdecl *listCpuDeviceNamesType)(char *devNames[], unsigned int count);
        listCpuDeviceNamesType listCpuDeviceNames = nullptr;

        GPUUtilitiesDll = LoadLibraryA("GPUUtilities.dll");
        if (NULL != GPUUtilitiesDll)
        {
            listCpuDeviceNames = (listCpuDeviceNamesType)GetProcAddress(GPUUtilitiesDll, "listCpuDeviceNames");
            if (NULL != listCpuDeviceNames)
            {
                foundCount = listCpuDeviceNames(devNames, count);
            }
            else
            {
                MessageBoxA(NULL, "NOT FOUND listCpuDeviceNames", "GPUUtils...", MB_ICONERROR);
            }
        }
        else
        {
            MessageBoxA(NULL, "NOT FOUND GPUUtilities.dll", "GPUUtils...", MB_ICONERROR);
        }
        #else
        foundCount = listCpuDeviceNames(devNames, count);
        #endif
    }

    return foundCount;
}


AMF_RESULT CreateCommandQueuesVIAamf(int deviceIndex, int32_t flag1, cl_command_queue* pcmdQueue1, int32_t flag2, cl_command_queue* pcmdQueue2, int amfDeviceType = AMF_CONTEXT_DEVICE_TYPE_GPU)
{
    bool AllIsOK = true;

	if (pcmdQueue1 == NULL || pcmdQueue2 == NULL) {
		return AMF_INVALID_ARG;
	}

    if (NULL != *pcmdQueue1)
    {
        printf("Queue release %llX\r\n", *pcmdQueue1);
		DBG_CLRELEASE(*pcmdQueue1,"*pcmdQueue1");
       *pcmdQueue1 = NULL;
    }
    if (NULL != *pcmdQueue2)
    {
        printf("Queue release %llX\r\n", *pcmdQueue2);
		DBG_CLRELEASE(*pcmdQueue2,"*pcmdQueue2");
		*pcmdQueue2 = NULL;
    }

    AMF_RESULT res = g_AMFFactory.Init();   // initialize AMF
    if (AMF_OK == res)
    {
        // Create default CPU AMF context.
        amf::AMFContextPtr pContextAMF = NULL;
        res = g_AMFFactory.GetFactory()->CreateContext(&pContextAMF);

        pContextAMF->SetProperty(AMF_CONTEXT_DEVICE_TYPE, amfDeviceType);
        if (AMF_OK == res)
        {
            amf::AMFComputeFactoryPtr pOCLFactory = NULL;
            res = pContextAMF->GetOpenCLComputeFactory(&pOCLFactory);
            if (AMF_OK == res)
            {
                amf_int32 deviceCount = pOCLFactory->GetDeviceCount();
                if (deviceIndex < deviceCount)
                {
                    amf::AMFComputeDevicePtr pDeviceAMF;
                    res = pOCLFactory->GetDeviceAt(deviceIndex, &pDeviceAMF);
                    if (nullptr != pDeviceAMF)
                    {
                        pContextAMF->InitOpenCLEx(pDeviceAMF);
                        cl_context clContext  = static_cast<cl_context>(pDeviceAMF->GetNativeContext());
                        cl_device_id clDevice = static_cast<cl_device_id>(pDeviceAMF->GetNativeDeviceID());
#ifdef RTQ_ENABLED
	#define QUEUE_MEDIUM_PRIORITY                   0x00010000
	#define QUEUE_REAL_TIME_COMPUTE_UNITS           0x00020000
#endif
                        if (NULL != pcmdQueue1)
                        {//user requested one queue
                            int ComputeFlag = 0;
                            amf_int64 Param = flag1 & 0x0FFFF;
#ifdef RTQ_ENABLED
                            if (QUEUE_MEDIUM_PRIORITY == (flag1 & QUEUE_MEDIUM_PRIORITY))
                            {
                                ComputeFlag = 2;
                            }
                            if (QUEUE_REAL_TIME_COMPUTE_UNITS == (flag1 & QUEUE_REAL_TIME_COMPUTE_UNITS))
                            {
                                ComputeFlag = 1;
                            }
#endif
                            cl_command_queue tempQueue = NULL;
                            pDeviceAMF->SetProperty(AMFQUEPROPERTY, Param);
                            amf::AMFComputePtr AMFDevice;
                            pDeviceAMF->CreateCompute(&ComputeFlag, &AMFDevice);
                            if (nullptr != AMFDevice)
                            {
                                tempQueue = static_cast<cl_command_queue>(AMFDevice->GetNativeCommandQueue());
                            }
                            if (NULL == tempQueue)
                            {
                                fprintf(stdout, "createQueue failed to create cmdQueue1 \n");
                                AllIsOK = false;
                            }
                            clRetainCommandQueue(tempQueue);
							CLQUEUE_REFCOUNT(tempQueue);
                            *pcmdQueue1 = tempQueue;
                        }

                        if (NULL != pcmdQueue2)
                        {//user requested second queue
                            int ComputeFlag = 0;
                            amf_int64 Param = flag2 & 0x0FFFF;
#ifdef RTQ_ENABLED
                            if (QUEUE_MEDIUM_PRIORITY == (flag2 & QUEUE_MEDIUM_PRIORITY))
                            {
                                ComputeFlag = 2;
                            }
                            if (QUEUE_REAL_TIME_COMPUTE_UNITS == (flag2 & QUEUE_REAL_TIME_COMPUTE_UNITS))
                            {
                                ComputeFlag = 1;
                            }
#endif
                            cl_command_queue tempQueue = NULL;
                            pDeviceAMF->SetProperty(AMFQUEPROPERTY, Param);
                            amf::AMFComputePtr AMFDevice;
                            pDeviceAMF->CreateCompute(&ComputeFlag, &AMFDevice);
                            if (nullptr != AMFDevice)
                            {
                                tempQueue = static_cast<cl_command_queue>(AMFDevice->GetNativeCommandQueue());
                            }
                            if (NULL == tempQueue)
                            {
                                fprintf(stdout, "createQueue failed to create cmdQueue2 \n");
                                AllIsOK = false;
                            }
                            clRetainCommandQueue(tempQueue);
							CLQUEUE_REFCOUNT(tempQueue);
                            *pcmdQueue2 = tempQueue;
                        }

                    }
                }
                else
                {
                    res = AMF_INVALID_ARG;
                }
            }
            pOCLFactory.Release();
        }
        pContextAMF.Release();
        g_AMFFactory.Terminate();
    }
    else
    {
        return AMF_NOT_INITIALIZED;
    }

    if (false == AllIsOK)
    {
        if (NULL != pcmdQueue1)
        {
            if (NULL != *pcmdQueue1)
            {
                printf("Queue release %llX\r\n", pcmdQueue1);
				DBG_CLRELEASE(*pcmdQueue1,"*pcmdQueue1");
                *pcmdQueue1 = NULL;
            }
        }
        if (NULL != pcmdQueue2)
        {
            if (NULL != *pcmdQueue2)
            {
                printf("Queue release %llX\r\n", pcmdQueue2);
				DBG_CLRELEASE(*pcmdQueue2,"*pcmdQueue2");
				*pcmdQueue2 = NULL;
            }
        }
    }

	CLQUEUE_REFCOUNT(*pcmdQueue1);
	CLQUEUE_REFCOUNT(*pcmdQueue2);
	return res;
}

//bool GetDeviceFromIndex(int deviceIndex, cl_device_id *device, cl_device_type clDeviceType);

bool GetDeviceFromIndex(int deviceIndex, cl_device_id *device, cl_context *context, cl_device_type clDeviceType){

    #ifdef _WIN32
    HMODULE GPUUtilitiesDll = NULL;
    GPUUtilitiesDll = LoadLibraryA("GPUUtilities.dll");
    if (NULL == GPUUtilitiesDll)
        return false;

    typedef int  (WINAPI *getDeviceAndContextType)(int devIdx, cl_context *pContext, cl_device_id *pDevice, cl_device_type clDeviceType);
    getDeviceAndContextType getDeviceAndContext = nullptr;
    getDeviceAndContext = (getDeviceAndContextType)GetProcAddress(GPUUtilitiesDll, "getDeviceAndContext");
    if (NULL == getDeviceAndContext)
        return false;
    #endif

    cl_context clContext = NULL;
    cl_device_id clDevice = NULL;
    //get context of open device via index 0,1,2,...
    getDeviceAndContext(deviceIndex, &clContext, &clDevice, clDeviceType);
    if (NULL == clContext)
        return false;
    if (NULL == clDevice)
        return false;

    *device = clDevice;
    *context = clContext;
    return true;
}

bool CreateCommandQueuesWithCUcount(int deviceIndex, cl_command_queue* pcmdQueue1, cl_command_queue* pcmdQueue2, int Q1CUcount, int Q2CUcount)
{
    cl_int err = 0;
    cl_device_id device = NULL;
    cl_context context = NULL;
    cl_device_partition_property props[] = { CL_DEVICE_PARTITION_BY_COUNTS,
        Q2CUcount, Q1CUcount, CL_DEVICE_PARTITION_BY_COUNTS_LIST_END, 0 }; // count order seems reversed! 

    GetDeviceFromIndex(deviceIndex, &device, &context, CL_DEVICE_TYPE_CPU); // only implemented for CPU

    cl_device_id outdevices[2] = { NULL , NULL };

    err = clCreateSubDevices(device,
        props,
        2,
        outdevices,
        NULL);

    *pcmdQueue1 = clCreateCommandQueue(context, outdevices[0], NULL, &err);
    printf("\r\nOpenCL queue created: 0x%llX, error code: %d\r\n", *pcmdQueue1, err);

    *pcmdQueue2 = clCreateCommandQueue(context, outdevices[1], NULL, &err);
    printf("\r\nOpenCL queue created: 0x%llX, error code: %d\r\n", *pcmdQueue2, err);

    return err;
}


bool CreateCommandQueuesVIAocl(int deviceIndex, int32_t flag1, cl_command_queue* pcmdQueue1, int32_t flag2, cl_command_queue* pcmdQueue2, cl_device_type clDeviceType)
{
    bool AllIsOK = true;

    #ifdef _WIN32
    HMODULE GPUUtilitiesDll = NULL;
    GPUUtilitiesDll = LoadLibraryA("GPUUtilities.dll");
    if (NULL == GPUUtilitiesDll)
        return false;

    typedef int  (WINAPI *getDeviceAndContextType)(int devIdx, cl_context *pContext, cl_device_id *pDevice, cl_device_type clDeviceType);
    getDeviceAndContextType getDeviceAndContext = nullptr;
    getDeviceAndContext = (getDeviceAndContextType)GetProcAddress(GPUUtilitiesDll, "getDeviceAndContext");
    if (NULL == getDeviceAndContext)
        return false;

    typedef cl_command_queue(WINAPI *createQueueType)(cl_context context, cl_device_id device, int, int);
    createQueueType createQueue = nullptr;
    createQueue = (createQueueType)GetProcAddress(GPUUtilitiesDll, "createQueue");
    if (NULL == createQueue)
        return false;
    #endif

    cl_context clContext = NULL;
    cl_device_id clDevice = NULL;
    //get context of open device via index 0,1,2,...
    getDeviceAndContext(deviceIndex, &clContext, &clDevice, clDeviceType);
    if (NULL == clContext)
        return false;
    if (NULL == clDevice)
        return false;
    clRetainDevice(clDevice);
    clRetainContext(clContext);

    if (NULL != pcmdQueue1)
    {//user requested one queue
        cl_command_queue tempQueue = createQueue(clContext, clDevice, 0, 0);
        if (NULL == tempQueue)
        {
            fprintf(stdout, "createQueue failed to create cmdQueue1 \n");
            AllIsOK = false;
        }
        *pcmdQueue1 = tempQueue;
    }

    if (NULL != pcmdQueue2)
    {//user requested second queue
        cl_command_queue tempQueue = createQueue(clContext, clDevice, 0, 0);
        if (NULL == tempQueue)
        {
            fprintf(stdout, "createQueue failed to create cmdQueue2 \n");
            AllIsOK = false;
        }
        *pcmdQueue2 = tempQueue;
    }

    if (false == AllIsOK)
    {
        if (NULL != pcmdQueue1)
        {
            if (NULL != *pcmdQueue1)
            {
                printf("Queue release %llX\r\n", pcmdQueue1);
				DBG_CLRELEASE(*pcmdQueue1,"*pcmdQueue1");
                *pcmdQueue1 = NULL;
            }
        }
        if (NULL != pcmdQueue2)
        {
            if (NULL != *pcmdQueue2)
            {
                printf("Queue release %llX\r\n", pcmdQueue2);
				DBG_CLRELEASE(*pcmdQueue2,"*pcmdQueue2");
				*pcmdQueue2 = NULL;
            }
        }
    }

    if (NULL != clContext)
    {
        clReleaseContext(clContext);
        clContext = NULL;
    }
    if (NULL != clDevice)
    {
        clReleaseDevice(clDevice);
        clDevice = NULL;
    }
    return AllIsOK;
}

bool CreateGpuCommandQueues(int deviceIndex, int32_t flag1, cl_command_queue* pcmdQueue1, int32_t flag2, cl_command_queue* pcmdQueue2)
{
    bool bResult = false;
    switch (CreateCommandQueuesVIAamf(deviceIndex, flag1, pcmdQueue1, flag2, pcmdQueue2, AMF_CONTEXT_DEVICE_TYPE_GPU))
    {
    case AMF_NOT_INITIALIZED:
        bResult = CreateCommandQueuesVIAocl(deviceIndex, 0, pcmdQueue1, 0, pcmdQueue2, CL_DEVICE_TYPE_GPU);
        break;
    default:
        ;
    }
    return bResult;
}

bool CreateCpuCommandQueues(int deviceIndex, int32_t flag1, cl_command_queue* pcmdQueue1, int32_t flag2, cl_command_queue* pcmdQueue2)
{
    bool bResult = false;
    switch (CreateCommandQueuesVIAamf(deviceIndex, flag1, pcmdQueue1, flag2, pcmdQueue2, AMF_CONTEXT_DEVICE_TYPE_CPU))
    {
    case AMF_NOT_INITIALIZED:
        bResult = CreateCommandQueuesVIAocl(deviceIndex, 0, pcmdQueue1, 0, pcmdQueue2, CL_DEVICE_TYPE_CPU);
        break;
    default:
        ;
    }
    return bResult;

    return bResult;
}

/*
int create1QueueOnDevice(cl_command_queue *queue, int devIdx = 0)
{
    cl_context context;
    cl_device_id device;
    getDeviceAndContext(devIdx, &context, &device);

    *queue = createQueue(context, device);

    clReleaseContext(context);
    clReleaseDevice(device);

    return 0;
}
int create2QueuesOnDevice(cl_command_queue *queue1, cl_command_queue *queue2, int devIdx = 0)
{
    cl_context context;
    cl_device_id device;
    getDeviceAndContext(devIdx, &context, &device);

    *queue1 = createQueue(context, device);
    *queue2 = createQueue(context, device);

    clReleaseContext(context);
    clReleaseDevice(device);


    return 0;
}
*/