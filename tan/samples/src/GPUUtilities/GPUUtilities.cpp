// GPUUtilities.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

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

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <io.h>
//#include <gl/gl.h>
//#include <gl/GLU.h>

#include <cl/cl.h>
#include <cl/cl_ext.h>
#include <cl/cl_gl.h>

#include "gpuutilities.h"
#include "../../../../amf/public/include/core/context.h"
#include "../../../../amf/public/common/AMFFactory.h"
#include "../ADL/ADLQuery.h"


/* To Do: Check for GPU device not used by display....

// Create CL context properties, add GLX context & handle to DC
cl_context_properties properties[] = {
 CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(), // GLX Context
 CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDisplay(), // GLX Display
 CL_CONTEXT_PLATFORM, (cl_context_properties)platform, // OpenCL platform
 0
};

// Find CL capable devices in the current GL context
cl_device_id devices[32]; 
size_t size;
clGetGLContextInfoKHR(properties, CL_DEVICES_FOR_GL_CONTEXT_KHR, 32 * sizeof(cl_device_id), devices, &size);

// We want to create context on device NOT in above list, to avoid conflict with graphics....

*/

/*
// partition CUs into two devices equally:
cl_uint max_CUs = 0;
clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &max_CUs, NULL);
fprintf(stdout, "   max compute units: %d\n", max_CUs);

cl_device_partition_property  props{ CL_DEVICE_PARTITION_EQUALLY, max_CUs/2, 0 }
cl_device_id devices[2];
clCreateSubDevices(device, props, 2, devices, NULL);
*/

/**
*******************************************************************************
* @fn listDeviceNames
* @brief returns list of installed  devices
*
* @param[out] devNames    : Points to empty string array to return device names
* @param[in] count		  : length of devNames
* @param[in] clDeviceType : CL_DEVICE_TYPE_GPU or CL_DEVICE_TYPE_CPU

* @return INT number of strings written to devNames array. (<= count)
*
*******************************************************************************
*/
int listOClDeviceNames(char *devNames[], unsigned int count, cl_device_type clDeviceType) {

    int status;

    /*
    * Have a look at the available platforms and pick
    * the AMD one.
    */

    cl_uint numPlatforms = 0;
    cl_platform_id platform = NULL;
    cl_platform_id* platforms = NULL;
    status = clGetPlatformIDs(0, NULL, &numPlatforms);
    if (status != CL_SUCCESS) {
        fprintf(stdout, "clGetPlatformIDs returned error: %d\n", status);
        return 0;
    }
    if (0 < numPlatforms)
    {
        platforms = new cl_platform_id[numPlatforms];
        status = clGetPlatformIDs(numPlatforms, platforms, NULL);
        if (status != CL_SUCCESS) {
            fprintf(stdout, "clGetPlatformIDs returned error: %d\n", status);
            delete[] platforms;
            return 0;
        }

        for (unsigned i = 0; i < numPlatforms; ++i)
        {
            char vendor[100];
            // char name[100];
            status = clGetPlatformInfo(platforms[i],
                CL_PLATFORM_VENDOR,
                sizeof(vendor),
                vendor,
                NULL);

            if (status != CL_SUCCESS) {
                fprintf(stdout, "clGetPlatformInfo returned error: %d\n", status);
                continue;
            }
        }
    }

    /*if (platform == NULL) {
        fprintf(stdout, "No suitable platform found!\n");
        return 0;
    }*/

    // enumerate devices
    char driverVersion[100] = "\0";

    // Retrieve device
    int totalDevices = 0;
    for (unsigned int nPlatform = 0; nPlatform < numPlatforms; nPlatform++) {
        platform = platforms[nPlatform];

        cl_uint numDevices = 0;
        clGetDeviceIDs(platform, clDeviceType, 0, NULL, &numDevices);
        if (numDevices == 0)
        {
            continue;
        }
        cl_device_id* devices = new cl_device_id[numDevices];
        for (cl_uint i = 0; i < numDevices; i++){
            devices[i] = NULL;
        }
        status = clGetDeviceIDs(platform, clDeviceType, numDevices, devices, &numDevices);
        if (status != CL_SUCCESS) {
            fprintf(stdout, "clGetDeviceIDs returned error: %d\n", status);
        }
        status = clGetDeviceInfo(devices[0], CL_DRIVER_VERSION, sizeof(driverVersion), driverVersion, NULL);
        if (status != CL_SUCCESS) {
            fprintf(stdout, "clGetDeviceInfo returned error: %d\n", status);
        }
        else {

            fprintf(stdout, "Driver version: %s\n", driverVersion);
            fprintf(stdout, "%d devices found:\n", numDevices);

            for (unsigned int n = 0; n < numDevices && n < count; n++) {
                int k = totalDevices + n;
                devNames[k] = new char[100];
                devNames[k][0] = '\0';
                clGetDeviceInfo(devices[n], CL_DEVICE_NAME, 100, devNames[k], NULL);
                fprintf(stdout, "   GPU device %s\n", devNames[k]);
                cl_device_topology_amd pciBusInfo;
                status = clGetDeviceInfo(devices[n], CL_DEVICE_TOPOLOGY_AMD, sizeof(cl_device_topology_amd), &pciBusInfo, NULL);
                if (status == CL_SUCCESS){
                    fprintf(stdout, "   PCI bus: %d device: %d function: %d\n", pciBusInfo.pcie.bus, pciBusInfo.pcie.device, pciBusInfo.pcie.function);
                }
                cl_uint max_CUs = 0;
                clGetDeviceInfo(devices[n], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &max_CUs, NULL);
                fprintf(stdout, "   max compute units: %d\n", max_CUs);
            }
        }

        for (cl_uint i = 0; i < numDevices; i++){
            clReleaseDevice(devices[i]);
        }
        totalDevices += numDevices;
        delete[] devices;
        devices = NULL;
    }

    if (platforms)    
        delete[] platforms;

    return totalDevices;
}


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
    int devIdx;
    devIdx = listOClDeviceNames(devNames, count, CL_DEVICE_TYPE_GPU);
    return devIdx;
}

int listCpuDeviceNames(char *devNames[], unsigned int count) {
    int devIdx;
    devIdx = listOClDeviceNames(devNames, count, CL_DEVICE_TYPE_CPU);
    return devIdx;
}


int getDeviceAndContext(int devIdx, cl_context *pContext, cl_device_id *pDevice, cl_device_type clDeviceType)
{
    cl_int error = CL_DEVICE_NOT_FOUND;

    cl_command_queue cmdQueue = NULL;

    int status;

    fprintf(stdout, "Creating General Queue\n");
    /*
    * Have a look at the available platforms and pick either
    * the AMD one if available or a reasonable default.
    */

    cl_uint numPlatforms = 0;
    cl_platform_id platform = NULL;
    cl_platform_id* platforms = NULL;
    status = clGetPlatformIDs(0, NULL, &numPlatforms);
    if (status != CL_SUCCESS) {
        fprintf(stdout, "clGetPlatformIDs returned error: %d\n", status);
        return NULL;
    }
    if (0 < numPlatforms)
    {
        platforms = new cl_platform_id[numPlatforms];
        status = clGetPlatformIDs(numPlatforms, platforms, NULL);
        if (status != CL_SUCCESS) {
            fprintf(stdout, "clGetPlatformIDs returned error: %d\n", status);
            delete[] platforms;
            return NULL;
        }

        // ToDo: we need to extend this logic to read CL_PLATFORM_NAME, etc.
        // and add logic to decide which of several AMD GPUs to use.
        for (unsigned i = 0; i < numPlatforms; ++i)
        {
            char vendor[100];
            //char name[100];
            status = clGetPlatformInfo(platforms[i],
                CL_PLATFORM_VENDOR,
                sizeof(vendor),
                vendor,
                NULL);

            if (status != CL_SUCCESS) {
                fprintf(stdout, "clGetPlatformInfo returned error: %d\n", status);
                continue;
            }
        }
    }


    // enumerate devices

    // To Do: handle multi-GPU case, pick appropriate GPU/APU
    char driverVersion[100] = "\0";
    

    // Retrieve device
    int totalDevices = 0;
    for (unsigned int nPlatform = 0; nPlatform < numPlatforms; nPlatform++) {
        platform = platforms[nPlatform];

        cl_context_properties contextProps[3] =
        {
            CL_CONTEXT_PLATFORM,
            (cl_context_properties)platform,
            0
        };

        // Retrieve device
        cl_uint numDevices = 0;
        clGetDeviceIDs(platform, clDeviceType, 0, NULL, &numDevices);
        if (numDevices == 0)
        {
            continue;
        }

        cl_device_id* devices = new cl_device_id[numDevices];
        status = clGetDeviceIDs(platform, clDeviceType, numDevices, devices, &numDevices);
        if (status != CL_SUCCESS) {
            fprintf(stdout, "clGetDeviceIDs returned error: %d\n", status);
            delete[] devices;
            return NULL;
        }
        clGetDeviceInfo(devices[0], CL_DRIVER_VERSION, sizeof(driverVersion), driverVersion, NULL);

        // log all devices found:
        fprintf(stdout, "Driver version: %s\n", driverVersion);
        fprintf(stdout, "%d devices found:\n", numDevices);
        for (unsigned int n = 0; n < numDevices; n++) {
            char deviceName[100] = "\0";
            clGetDeviceInfo(devices[n], CL_DEVICE_NAME, sizeof(deviceName), deviceName, NULL);
            fprintf(stdout, "   GPU device %s\n", deviceName);
        }

        int deviceId = 0;
        if (devIdx >= totalDevices && devIdx < totalDevices + (int)numDevices){

            deviceId = devIdx - totalDevices;

            *pDevice = devices[deviceId];
            clRetainDevice(*pDevice);
            *pContext = clCreateContext(contextProps, 1, pDevice, NULL, NULL, &error);
            // log chosen device:
            if (error == CL_SUCCESS){
                char deviceName[100] = "\0";
                clGetDeviceInfo(devices[deviceId], CL_DEVICE_NAME, sizeof(deviceName), deviceName, NULL);
                fprintf(stdout, " Using GPU device %s\n", deviceName);
            }
            else {
                fprintf(stdout, "clCreateContext failed: %d \n", error);
            }
        }

        for (unsigned int idx = 0; idx < numDevices; idx++){
            clReleaseDevice(devices[idx]);
        }
        delete[] devices;
        devices = NULL;

        totalDevices += numDevices;
    }

    if (platforms)
        delete[] platforms;

    return error;
}


cl_command_queue createQueue(cl_context context, cl_device_id device, int flag, int var )
{
    cl_int error = 0;
    cl_command_queue cmdQueue = NULL;

    // Create a command queue

    if (flag != 0) {
        // use clCreateCommandQueueWithProperties to pass custom queue properties to driver: 
        const cl_queue_properties cprops[] = {
            CL_QUEUE_PROPERTIES,
            0,
            static_cast<cl_queue_properties>
            (unsigned long long(flag)),
            static_cast<cl_queue_properties>
            (unsigned long long(var)),
            static_cast<cl_queue_properties>
            (unsigned long long(0)) };
        // OpenCL 2.0
        cmdQueue = clCreateCommandQueueWithProperties(context, device, cprops, &error);
    }
    else {
        // OpenCL 1.2
        cmdQueue = clCreateCommandQueue(context, device, NULL, &error);
    }

    return cmdQueue;
}


/* */
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



bool checkOpenCL2_XCompatibility(cl_device_id device_id)
{

    char deviceVersion[256];
    memset(deviceVersion, 0, 256);
    clGetDeviceInfo(device_id, CL_DEVICE_VERSION, 256, deviceVersion, NULL);
    cl_device_type clDeviceType = CL_DEVICE_TYPE_GPU;
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



bool getAMFdeviceProperties(cl_command_queue queue, int *maxReservedComputeUnits){
    AMF_RESULT res = AMF_OK;
    amf::AMFContextPtr amfContext = NULL;
    int maxCus = 0;

    g_AMFFactory.Init();
    // Create default CPU AMF context.
    if (g_AMFFactory.GetFactory() != NULL) {
        res = g_AMFFactory.GetFactory()->CreateContext(&amfContext);
    }
    if (res == AMF_OK && amfContext != 0){
        if (queue != NULL)
        {
            res = amfContext->InitOpenCL(queue);
            if (res == AMF_OK) {
                amf::AMFComputeFactoryPtr pOCLFactory;
                res = amfContext->GetOpenCLComputeFactory(&pOCLFactory);
                if (res == AMF_OK){
                    amf_int32 deviceCount = pOCLFactory->GetDeviceCount();
                    for (amf_int32 i = 0; i < deviceCount; i++)
                    {
                        amf::AMFComputeDevicePtr         pDeviceAMF;
                        res = pOCLFactory->GetDeviceAt(i, &pDeviceAMF);
                        if (amfContext->GetOpenCLDeviceID() == pDeviceAMF->GetNativeDeviceID())
                        {

                            pDeviceAMF->GetProperty(L"MaxRealTimeComputeUnits", &maxCus);
                            break; //TODO:AA
                        }
                    }
                }
            }
        }



        //amf_int64 tmp = 0;
        //amfContext->GetProperty(L"MaxRealTimeComputeUnits", &tmp);
        //*maxReservedComputeUnits = int(tmp);
        //amfContext->Terminate();
        *maxReservedComputeUnits = maxCus;
        return true;
    }
    else {
        return false;
    }
}

// 

int listTanDevicesAndCaps(TanDeviceCapabilities **deviceListPtr, int *listLength)
{
    int status = 0;

    ADLAdapterInfo ADLInfo[100];
    int nADLAdapters = ADLQueryAdapterInfo(ADLInfo, 100);

    TanDeviceCapabilities *deviceList = new TanDeviceCapabilities[100];

    /*
    * Have a look at the available platforms 
    */

    cl_uint numPlatforms = 0;
    cl_platform_id platform = NULL;
    cl_platform_id* platforms = NULL;
    status = clGetPlatformIDs(0, NULL, &numPlatforms);
    if (status != CL_SUCCESS) {
        fprintf(stdout, "clGetPlatformIDs returned error: %d\n", status);
        return 0;
    }
    if (0 < numPlatforms)
    {
        platforms = new cl_platform_id[numPlatforms];
        status = clGetPlatformIDs(numPlatforms, platforms, NULL);
        if (status != CL_SUCCESS) {
            fprintf(stdout, "clGetPlatformIDs returned error: %d\n", status);
            delete[] platforms;
            return 0;
        }
    }


    // enumerate devices
    char driverVersion[100] = "\0";
    char *extensions[2048];

    // Retrieve device
    int totalDevices = 0;

    cl_device_type clDeviceTypes[] = { CL_DEVICE_TYPE_GPU, CL_DEVICE_TYPE_CPU };

    for (int dt = 0; dt < sizeof(clDeviceTypes) / sizeof(cl_device_type); dt++) {


        for (unsigned int nPlatform = 0; nPlatform < numPlatforms; nPlatform++) {
            platform = platforms[nPlatform];

            cl_uint numDevices = 0;
            clGetDeviceIDs(platform, clDeviceTypes[dt], 0, NULL, &numDevices);
            if (numDevices == 0)
            {
                continue;
            }
            cl_device_id* devices = new cl_device_id[numDevices];
            for (cl_uint i = 0; i < numDevices; i++){
                devices[i] = NULL;
            }
            status = clGetDeviceIDs(platform, clDeviceTypes[dt], numDevices, devices, &numDevices);
            if (status != CL_SUCCESS) {
                fprintf(stdout, "clGetDeviceIDs returned error: %d\n", status);
            }
            status = clGetDeviceInfo(devices[0], CL_DRIVER_VERSION, sizeof(driverVersion), driverVersion, NULL);
            if (status != CL_SUCCESS) {
                fprintf(stdout, "clGetDeviceInfo returned error: %d\n", status);
            }
            else {
                /*
                cl_device_id devId;
                cl_device_type devType;
                char *deviceName;
                char *deviceVendor;
                char *driverVersion;
                bool supportsTAN;
                bool hasAttachedDisplay;
                int maxClockFrequency;
                float ComputeUnitPerfFactor;
                int totalComputeUnits;
                int maxReservableComputeUnits;
                int reserveComputeUnitsGranularity;
                int maxMemory;
                */

                //fprintf(stdout, "Driver version: %s\n", driverVersion);
                //fprintf(stdout, "%d devices found:\n", numDevices);

                for (unsigned int n = 0; n < numDevices; n++) {
                    int k = totalDevices + n;
                    //device id:
                    deviceList[k].devId = devices[n];
                    // device type:
                    clGetDeviceInfo(devices[n], CL_DEVICE_TYPE, 100, &deviceList[k].devType, NULL);
                    //device name:
                    deviceList[k].deviceName = new char[100];
                    deviceList[k].deviceName[0] = '\0';
                    clGetDeviceInfo(devices[n], CL_DEVICE_NAME, 100, deviceList[k].deviceName, NULL);
                    //device vendor:
                    deviceList[k].deviceVendor = new char[100];
                    deviceList[k].deviceVendor[0] = '\0';
                    clGetDeviceInfo(devices[n], CL_DEVICE_VENDOR, 100, deviceList[k].deviceVendor, NULL);

                    //driver version:
                    deviceList[k].driverVersion = new char[100];
                    deviceList[k].driverVersion[0] = '\0';
                    clGetDeviceInfo(devices[n], CL_DRIVER_VERSION, 100, deviceList[k].driverVersion, NULL);

                    //supports TAN:
                    deviceList[k].supportsTAN = checkOpenCL2_XCompatibility(devices[n]);

                    //maxClockFrequency:
                    clGetDeviceInfo(devices[n], CL_DEVICE_MAX_CLOCK_FREQUENCY, 100, &deviceList[k].maxClockFrequency, NULL);
                    //ComputeUnitPerfFactor:
                    deviceList[k].ComputeUnitPerfFactor = 1.0;
                    //totalComputeUnits:
                    clGetDeviceInfo(devices[n], CL_DEVICE_MAX_COMPUTE_UNITS, 100, &deviceList[k].totalComputeUnits, NULL);

                    //maxReservableComputeUnits:
                    //deviceList[k].maxReservableComputeUnits = deviceList[k].totalComputeUnits / 5; // hack TODO get from AMF
                    cl_context_properties contextProps[3] =
                    {
                        CL_CONTEXT_PLATFORM,
                        (cl_context_properties)platform,
                        0
                    };

                    cl_int error = 0;
                    deviceList[k].maxReservableComputeUnits = 0;
                    cl_context context = clCreateContext(contextProps, 1, &deviceList[k].devId, NULL, NULL, &error);
                    cl_command_queue queue = clCreateCommandQueue(context, deviceList[k].devId, NULL, &error);
                    getAMFdeviceProperties(queue, &deviceList[k].maxReservableComputeUnits);


                    //reserveComputeUnitsGranularity:
                    deviceList[k].reserveComputeUnitsGranularity = 4; // hack TODO get from AMF
                    if (deviceList[k].reserveComputeUnitsGranularity > deviceList[k].maxReservableComputeUnits) {
                        deviceList[k].reserveComputeUnitsGranularity = deviceList[k].maxReservableComputeUnits;
                    }

                    //maxMemory:
                    //clGetDeviceInfo(devices[n], CL_DEVICE_MAX_MEM_ALLOC_SIZE, 100, &deviceList[k].maxMemory, NULL);
                    clGetDeviceInfo(devices[n], CL_DEVICE_GLOBAL_MEM_SIZE, 100, &deviceList[k].maxMemory, NULL);
                    deviceList[k].localMemSize = 0;
                    clGetDeviceInfo(devices[n], CL_DEVICE_LOCAL_MEM_SIZE, 100, &deviceList[k].localMemSize, NULL);
                    
                    

                    //hack extra stuff
                    cl_device_topology_amd pciBusInfo;
                    memset(&pciBusInfo, 0, sizeof(pciBusInfo));

                    status = clGetDeviceInfo(devices[n], CL_DEVICE_TOPOLOGY_AMD, sizeof(cl_device_topology_amd), &pciBusInfo, NULL);
                    //if (status == CL_SUCCESS){
                    //    fprintf(stdout, "   PCI bus: %d device: %d function: %d\n", pciBusInfo.pcie.bus, pciBusInfo.pcie.device, pciBusInfo.pcie.function);
                    //}
                    status = clGetDeviceInfo(devices[n], CL_DEVICE_EXTENSIONS, 2047, extensions, NULL);

                    //hasAttachedDisplay:
                    deviceList[k].hasAttachedDisplay = false;
                    for (int l = 0; l < nADLAdapters; l++){
                        if (ADLInfo[l].busNumber == pciBusInfo.pcie.bus &&
                            ADLInfo[l].deviceNumber == pciBusInfo.pcie.device &&
                            ADLInfo[l].functionNumber == pciBusInfo.pcie.function)
                        {
                                if(ADLInfo[l].active){
                                    deviceList[k].hasAttachedDisplay = true;
                                }
                                deviceList[k].memoryBandwidth = ADLInfo[l].memoryBandwidth;
                                //hack
                                //deviceList[k].totalComputeUnits = ADLInfo[l].numCUs;

                        }
                    }

                    
                    //{
                    ////    // hack wglGetCurrentContext needs OpenGL32.lib
                    //    cl_device_id devices[32];
                    //    memset(devices, 0, sizeof(devices));
                    //    size_t size = 0;
                    //   cl_context_properties properties[] = {
                    // //       CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(), //glXGetCurrentContext(), // GLX Context
                    ////        CL_GLX_DISPLAY_KHR, (cl_context_properties)0, //glXGetCurrentDisplay(), // GLX Display
                    //        CL_CONTEXT_PLATFORM, (cl_context_properties)platform, // OpenCL platform
                    ////        0
                    //    };

                    //    typedef int(__cdecl *clGetGLContextInfoKHRType)(const cl_context_properties * /* properties */,
                    //        cl_gl_context_info            /* param_name */,
                    //        size_t                        /* param_value_size */,
                    //        void *                        /* param_value */,
                    //        size_t *                      /* param_value_size_ret */);
                    //    clGetGLContextInfoKHRType clGetGLContextInfoKHR = nullptr;

                    //    clGetGLContextInfoKHR = (clGetGLContextInfoKHRType)clGetExtensionFunctionAddress("clGetGLContextInfoKHR");

                    //    clGetGLContextInfoKHR(properties, CL_DEVICES_FOR_GL_CONTEXT_KHR, 32 * sizeof(cl_device_id), devices, &size);

                    //    printf("%d", size);
                    //}
                    

                }
            }

            for (cl_uint i = 0; i < numDevices; i++){
                clReleaseDevice(devices[i]);
            }
            totalDevices += numDevices;
            delete[] devices;
            devices = NULL;
        }
    }

    if (platforms)
        delete[] platforms;


    *listLength = totalDevices;
    *deviceListPtr = deviceList;

    return 0;
}
