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

#pragma once
typedef int(__cdecl *listGpuDeviceNamesType)(char *devNames[], unsigned int count);
typedef int(__cdecl *listCpuDeviceNamesType)(char *devNames[], unsigned int count);
typedef int(__cdecl *getDeviceAndContextType)(int devIdx, cl_context *pContext, cl_device_id *pDevice);
typedef cl_command_queue(__cdecl *createQueueType)(cl_context context, cl_device_id device, int flag, int cu_);

typedef struct _TanDeviceCapabilities {
    cl_device_id devId;                     // OpenCL device ID
    cl_device_type devType;                 // device type [ CL_DEVICE_TYPE_CPU  | CL_DEVICE_TYPE_GPU | ...]
    char *deviceName;                       // device name string
    char *deviceVendor;                     // device vendor string
    char *driverVersion;                    // driver version string
    bool supportsTAN;                       // This device supports AMD True Audio Next
    bool hasAttachedDisplay;                // This device has a connected monitor
    int maxClockFrequency;                  // This device's maximum clock in MHz
    float ComputeUnitPerfFactor;            // Device specific performance factor 
    int totalComputeUnits;                  // Total number of compute units in this device
    int maxReservableComputeUnits;          // Maximum reservable compute units <= 20% of Total
    int reserveComputeUnitsGranularity;     // granualarity of Cu reservation: typically 4
    unsigned __int64 maxMemory;             // Maximum memory allocation on device
    unsigned __int64 memoryBandwidth;       // Memory Bandwidth
    // extended
    unsigned __int64 localMemSize;          // Local Memory size

    int reserved[1024 - sizeof(__int64)];
} TanDeviceCapabilities;



// TAN objects creation functions.
extern "C"
{
    __declspec(dllexport) int __cdecl  listGpuDeviceNames(char *devNames[], unsigned int count);
    __declspec(dllexport) int __cdecl  listCpuDeviceNames(char *devNames[], unsigned int count);
    __declspec(dllexport) int __cdecl  listOClDeviceNames(char *devNames[], unsigned int count, cl_device_type clDeviceType);

    __declspec(dllexport) int __cdecl getDeviceAndContext(int devIdx, cl_context *pContext, cl_device_id *pDevice, cl_device_type clDeviceType = CL_DEVICE_TYPE_GPU);

    __declspec(dllexport) cl_command_queue __cdecl createQueue(cl_context context, cl_device_id device, int flag = 0, int cu_ = 0);

    __declspec(dllexport) int __cdecl  listTanDevicesAndCaps(TanDeviceCapabilities **deviceList, int *count);

}
