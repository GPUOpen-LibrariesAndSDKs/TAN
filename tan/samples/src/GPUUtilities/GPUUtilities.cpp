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
#include <cl/cl.h>
#include "gpuutilities.h"


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

    cl_device_type clDeviceType = CL_DEVICE_TYPE_GPU;
    int status;

    /*
    * Have a look at the available platforms and pick
    * the AMD one.
    */

    cl_uint numPlatforms = 0;
    cl_platform_id platform = NULL;
    status = clGetPlatformIDs(0, NULL, &numPlatforms);
    if (status != CL_SUCCESS) {
        fprintf(stdout, "clGetPlatformIDs returned error: %d\n", status);
        return 0;
    }
    if (0 < numPlatforms)
    {
        cl_platform_id* platforms = new cl_platform_id[numPlatforms];
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

            if (!(strcmp(vendor, "Advanced Micro Devices, Inc.") == 0))
            {
                continue;
            }

            platform = platforms[i];
        }
        delete[] platforms;
    }

    if (platform == NULL) {
        fprintf(stdout, "No suitable platform found!\n");
        return 0;
    }

    // enumerate devices
    char driverVersion[100] = "\0";

    // Retrieve device
    cl_uint numDevices = 0;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &numDevices);
    if (numDevices == 0)
    {
        fprintf(stdout, "Cannot find any GPU devices!\n");
        return 0;
    }
    cl_device_id* devices = new cl_device_id[numDevices];
    for (cl_uint i = 0; i < numDevices; i++){
        devices[i] = NULL;
    }
    status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, numDevices, devices, &numDevices);
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
            devNames[n] = new char[100];
            devNames[n][0] = '\0';
            clGetDeviceInfo(devices[n], CL_DEVICE_NAME, 100, devNames[n], NULL);
            fprintf(stdout, "   GPU device %s\n", devNames[n]);
        }
    }

    for (cl_uint i = 0; i < numDevices; i++){
        clReleaseDevice(devices[i]);
    }
    delete[] devices;
    return numDevices;
}

int getDeviceAndContext(int devIdx, cl_context *pContext, cl_device_id *pDevice)
{

    cl_command_queue cmdQueue = NULL;

    cl_device_type clDeviceType = CL_DEVICE_TYPE_GPU;
    int status;

    fprintf(stdout, "Creating General Queue\n");
    /*
    * Have a look at the available platforms and pick either
    * the AMD one if available or a reasonable default.
    */

    cl_uint numPlatforms = 0;
    cl_platform_id platform = NULL;
    status = clGetPlatformIDs(0, NULL, &numPlatforms);
    if (status != CL_SUCCESS) {
        fprintf(stdout, "clGetPlatformIDs returned error: %d\n", status);
        return NULL;
    }
    if (0 < numPlatforms)
    {
        cl_platform_id* platforms = new cl_platform_id[numPlatforms];
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

            if (!(strcmp(vendor, "Advanced Micro Devices, Inc.") == 0))
            {
                continue;
            }

            platform = platforms[i];
        }
        delete[] platforms;
    }

    if (platform == NULL) {
        fprintf(stdout, "No suitable platform found!\n");
        return NULL;
    }

    cl_context_properties contextProps[3] =
    {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)platform,
        0
    };


    // enumerate devices

    // To Do: handle multi-GPU case, pick appropriate GPU/APU
    char driverVersion[100] = "\0";

    // Retrieve device
    cl_uint numDevices = 0;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &numDevices);
    if (numDevices == 0)
    {
        fprintf(stdout, "Cannot find any GPU devices!\n");
        return NULL;
    }
    if ((unsigned int)devIdx > numDevices - 1){
        fprintf(stdout, "invalid device index %d \n", devIdx);
        return NULL;
    }

    cl_device_id* devices = new cl_device_id[numDevices];
    status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, numDevices, devices, &numDevices);
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
    deviceId = devIdx;

    cl_int error = 0;

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
    for (unsigned int idx = 0; idx < numDevices; idx++){
        clReleaseDevice(devices[idx]);
    }
    delete[] devices;
    return error;
}


cl_command_queue createQueue(cl_context context, cl_device_id device, int flag, int cu_ )
{
    cl_int error = 0;
    cl_command_queue cmdQueue = NULL;

    cl_device_type clDeviceType = CL_DEVICE_TYPE_GPU;

    flag = 0;
    cu_ = 0;

    // Create a real time queue
    const cl_queue_properties cprops[] = {
        CL_QUEUE_PROPERTIES,
        static_cast<cl_queue_properties>
        (0),
        flag,
        cu_,
        0 };


    cmdQueue = clCreateCommandQueueWithProperties(context, device, cprops, &error);


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
