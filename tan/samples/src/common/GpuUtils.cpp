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

#include <stdio.h>
#include <string.h>
#include <io.h>
#include <cl/cl.h>

#include "public/include/core/Context.h"        //AMF
#include "public/include/core/ComputeFactory.h" //AMF
#include "public/common/AMFFactory.h"           //AMF

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
int listGpuDeviceNames(char *devNames[], unsigned int count) {

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
                            lstrcpy(devNames[i], pName.stringValue);
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
    }

    return foundCount;
}

AMF_RESULT CreateCommandQueuesVIAamf(int deviceIndex, int32_t flag1, cl_command_queue* pcmdQueue1, int32_t flag2, cl_command_queue* pcmdQueue2)
{
    bool AllIsOK = true;
    if (NULL != pcmdQueue1)
    {
        clReleaseCommandQueue(*pcmdQueue1);
        *pcmdQueue1 = NULL;
    }
    if (NULL != pcmdQueue2)
    {
        clReleaseCommandQueue(*pcmdQueue2);
        *pcmdQueue2 = NULL;
    }

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
                if (deviceIndex < deviceCount)
                {
                    amf::AMFComputeDevicePtr pDeviceAMF;
                    res = pOCLFactory->GetDeviceAt(deviceIndex, &pDeviceAMF);
                    if (nullptr != pDeviceAMF)
                    {
                        pContextAMF->InitOpenCLEx(pDeviceAMF);
                        cl_context clContext  = static_cast<cl_context>(pDeviceAMF->GetNativeContext());
                        cl_device_id clDevice = static_cast<cl_device_id>(pDeviceAMF->GetNativeDeviceID());

#define QUEUE_MEDIUM_PRIORITY                   0x00010000
#define QUEUE_REAL_TIME_COMPUTE_UNITS           0x00020000
                        if (NULL != pcmdQueue1)
                        {//user requested one queue
                            int ComputeFlag = 0;
                            amf_int64 Param = flag1 & 0x0FFFF;
                            if (QUEUE_MEDIUM_PRIORITY == (flag1 & QUEUE_MEDIUM_PRIORITY))
                            {
                                ComputeFlag = 2;
                            }
                            if (QUEUE_REAL_TIME_COMPUTE_UNITS == (flag1 & QUEUE_REAL_TIME_COMPUTE_UNITS))
                            {
                                ComputeFlag = 1;
                            }

                            cl_command_queue tempQueue = NULL;
                            pDeviceAMF->SetProperty(L"MaxRealTimeComputeUnits", Param);
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
                            *pcmdQueue1 = tempQueue;
                        }

                        if (NULL != pcmdQueue2)
                        {//user requested second queue
                            int ComputeFlag = 0;
                            amf_int64 Param = flag2 & 0x0FFFF;
                            if (QUEUE_MEDIUM_PRIORITY == (flag2 & QUEUE_MEDIUM_PRIORITY))
                            {
                                ComputeFlag = 2;
                            }
                            if (QUEUE_REAL_TIME_COMPUTE_UNITS == (flag2 & QUEUE_REAL_TIME_COMPUTE_UNITS))
                            {
                                ComputeFlag = 1;
                            }
                            cl_command_queue tempQueue = NULL;
                            pDeviceAMF->SetProperty(L"MaxRealTimeComputeUnits", Param);
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
                clReleaseCommandQueue(*pcmdQueue1);
                *pcmdQueue1 = NULL;
            }
        }
        if (NULL != pcmdQueue2)
        {
            if (NULL != *pcmdQueue2)
            {
                clReleaseCommandQueue(*pcmdQueue2);
                *pcmdQueue2 = NULL;
            }
        }
    }

    return res;
}

bool CreateCommandQueuesVIAocl(int deviceIndex, int32_t flag1, cl_command_queue* pcmdQueue1, int32_t flag2, cl_command_queue* pcmdQueue2)
{
    bool AllIsOK = true;

    HMODULE GPUUtilitiesDll = NULL;
    GPUUtilitiesDll = LoadLibraryA("GPUUtilities.dll");
    if (NULL == GPUUtilitiesDll)
        return false;

    typedef int  (WINAPI *getDeviceAndContextType)(int devIdx, cl_context *pContext, cl_device_id *pDevice);
    getDeviceAndContextType getDeviceAndContext = nullptr;
    getDeviceAndContext = (getDeviceAndContextType)GetProcAddress(GPUUtilitiesDll, "getDeviceAndContext");
    if (NULL == getDeviceAndContext)
        return false;

    typedef cl_command_queue(WINAPI *createQueueType)(cl_context context, cl_device_id device, int, int);
    createQueueType createQueue = nullptr;
    createQueue = (createQueueType)GetProcAddress(GPUUtilitiesDll, "createQueue");
    if (NULL == createQueue)
        return false;

    cl_context clContext = NULL;
    cl_device_id clDevice = NULL;
    //get context of open device via index 0,1,2,...
    getDeviceAndContext(deviceIndex, &clContext, &clDevice);
    if (NULL == clContext)
        return false;

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
                clReleaseCommandQueue(*pcmdQueue1);
                *pcmdQueue1 = NULL;
            }
        }
        if (NULL != pcmdQueue2)
        {
            if (NULL != *pcmdQueue2)
            {
                clReleaseCommandQueue(*pcmdQueue2);
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

bool CreateCommandQueues(int deviceIndex, int32_t flag1, cl_command_queue* pcmdQueue1, int32_t flag2, cl_command_queue* pcmdQueue2)
{
    bool bResult = false;
    switch (CreateCommandQueuesVIAamf(deviceIndex, flag1, pcmdQueue1, flag2, pcmdQueue2))
    {
    case AMF_NOT_INITIALIZED:
        bResult = CreateCommandQueuesVIAocl(deviceIndex, 0, pcmdQueue1, 0, pcmdQueue2);
        break;
    default:
        ;
    }
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