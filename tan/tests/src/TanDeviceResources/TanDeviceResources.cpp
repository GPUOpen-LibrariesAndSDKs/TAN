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

// TanDeviceResourcesTest.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <string.h>

#include <CL/cl.h>
#include "../../../samples/src/GPUUtilities/GpuUtilities.h"
//#include "public/include/core/Context.h"
//#include "public/common/AMFFactory.h"

#ifdef _WIN32
  #include <Windows.h>
  #include <io.h>
#endif

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

//using namespace amf;



int main(int argc, char* argv[])
{

    printf("%s, Copyright (c) 2018 Advanced Micro Devices, Inc. All rights reserved.",argv[0]);

    TanDeviceCapabilities *devCapsList = NULL;
    int count = 0;

    #ifdef _WIN32
        HMODULE GPUUtilitiesDll = NULL;
        typedef int(__cdecl *listGpuDeviceNamesType)(char *devNames[], unsigned int count);
        listGpuDeviceNamesType listGpuDeviceNames = nullptr;

        GPUUtilitiesDll = LoadLibraryA("GPUUtilities.dll");
        if (NULL != GPUUtilitiesDll)
        {
            listTanDevicesAndCaps = (listTanDevicesAndCapsType)GetProcAddress(GPUUtilitiesDll, "listTanDevicesAndCaps");
            if (NULL != listTanDevicesAndCaps)
            {
                int result = listTanDevicesAndCaps(&devCapsList, &count);
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
        int result = listTanDevicesAndCaps(&devCapsList, &count);
    #endif

    for (int i = 0; i < count; i++){

       // clCreateCommandQueue()

        puts("");
        printf("Device #%d\n", i);
        printf("                         devId: %0x\n", devCapsList[i].devId);
        printf("                       devType: %s\n", (devCapsList[i].devType == CL_DEVICE_TYPE_GPU) ? "GPU" : "CPU");
        printf("                    deviceName: %s\n", devCapsList[i].deviceName);
        printf("                  deviceVendor: %s\n", devCapsList[i].deviceVendor);
        printf("                 driverVersion: %s\n", devCapsList[i].driverVersion);
        printf("                   supportsTAN: %d\n", devCapsList[i].supportsTAN);
        printf("            hasAttachedDisplay: %d\n", devCapsList[i].hasAttachedDisplay);
        printf("             maxClockFrequency: %d MHz\n", devCapsList[i].maxClockFrequency);
        printf("         ComputeUnitPerfFactor: %f\n", devCapsList[i].ComputeUnitPerfFactor);
        printf("             totalComputeUnits: %d\n", devCapsList[i].totalComputeUnits);
        printf("     maxReservableComputeUnits: %d\n", devCapsList[i].maxReservableComputeUnits);
        printf("reserveComputeUnitsGranularity: %d\n", devCapsList[i].reserveComputeUnitsGranularity);
        printf("                     maxMemory: %d MB\n", devCapsList[i].maxMemory /(1024*1024));
        printf("               MemoryBandwidth: %d MB/s\n", devCapsList[i].memoryBandwidth);
        printf("                   localMemory: %d KB\n", devCapsList[i].localMemSize/1024 );


    }

    // Usage: 
    // 1) add code to select device based on reported properties.
    // 2) call clRetainDevice(devId) for selected device.       
    // 3) delete devCapsList

    delete[] devCapsList;

    // 4) call clCreateContext with the selected device.


	return 0;
}

