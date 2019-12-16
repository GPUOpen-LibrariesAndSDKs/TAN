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

#if defined (LINUX)
#include "include/adl_sdk.h"
#include <dlfcn.h>	//dyopen, dlsym, dlclose
#include <stdlib.h>
#include <string.h>	//memeset

// Linux equivalent of sprintf_s
#define sprintf_s snprintf
// Linux equivalent of fscanf_s
#define fscanf_s fscanf
// Linux equivalent of GetProcAddress
#define GetProcAddress dlsym

#else
#include <windows.h>
#include "include\adl_sdk.h"
#endif

#include <stdio.h>

typedef int(*ADL_MAIN_CONTROL_CREATE)(ADL_MAIN_MALLOC_CALLBACK, int);
typedef int(*ADL_MAIN_CONTROL_DESTROY)();
typedef int(*ADL_ADAPTER_NUMBEROFADAPTERS_GET) (int*);
typedef int(*ADL_ADAPTER_ADAPTERINFO_GET) (LPAdapterInfo, int);
typedef int(*ADL_ADAPTER_ACTIVE_GET) (int, int*);
typedef int(*ADL_ADAPTER_MEMORYINFO_GET)(int iAdapterIndex, ADLMemoryInfo *lpMemoryInfo);
typedef int(*ADL_ADAPTER_GCOREINFO_GET)(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLGraphicCoreInfo *lpGCInfo);


int OpenADL(void);
void CloseADL(void);

void* __stdcall ADL_Main_Memory_Alloc(int iSize)
{
    void* lpBuffer = malloc(iSize);
    return lpBuffer;
}

void __stdcall ADL_Main_Memory_Free(void** lpBuffer)
{
    if (NULL != *lpBuffer)
    {
        free(*lpBuffer);
        *lpBuffer = NULL;
    }
}

enum	COMMAND	{ NONE, GETALL, GETMODE, SETMODE, INFOOVER, SETOVER, GETLIST, GENERATE, BIOSINFO };

#if defined (LINUX)
void *hDLL;		// Handle to .so library
#else
HINSTANCE hDLL;		// Handle to DLL
#endif

LPAdapterInfo     lpAdapterInfo = NULL;
LPADLDisplayInfo  lpAdlDisplayInfo = NULL;
LPADLMode lpADLMode = NULL;
ADLDisplayModeInfo * lpModeInfoList = NULL;

FILE * file = NULL;
FILE * file2 = NULL;
char msg[128];
char err[128];
int sMsg = sizeof(msg);
int sErr = sizeof(err);

// Initialize ADL
int OpenADL()
{
    ADL_MAIN_CONTROL_CREATE          ADL_Main_Control_Create;
    int ADL_Err = ADL_ERR;

#if defined (LINUX)
    char sztemp[256];
    sprintf(sztemp, "libatiadlxx.so");
    hDLL = dlopen(sztemp, RTLD_LAZY | RTLD_GLOBAL);
#else
    hDLL = LoadLibraryA("atiadlxx.dll");
    if (hDLL == NULL)
    {
        // A 32 bit calling application on 64 bit OS will fail to LoadLIbrary.
        // Try to load the 32 bit library (atiadlxy.dll) instead
        hDLL = LoadLibraryA("atiadlxy.dll");
    }
#endif

    if (hDLL != NULL)
    {
        ADL_Main_Control_Create = (ADL_MAIN_CONTROL_CREATE)GetProcAddress(hDLL, "ADL_Main_Control_Create");
        if (NULL != ADL_Main_Control_Create)
            ADL_Err = ADL_Main_Control_Create(ADL_Main_Memory_Alloc, 1);
        // The second parameter is 1, which means:
        // retrieve adapter information only for adapters that are physically present and enabled in the system
    }
    else
    {
        printf("ADL Library not found!\n");
    }

    return ADL_Err;
}

// Destroy ADL
void CloseADL()
{
    ADL_MAIN_CONTROL_DESTROY         ADL_Main_Control_Destroy;

    if (NULL != file)
        fclose(file);
    if (NULL != file2)
        fclose(file2);

    ADL_Main_Memory_Free((void **)&lpAdapterInfo);
    ADL_Main_Memory_Free((void **)&lpAdlDisplayInfo);

    ADL_Main_Control_Destroy = (ADL_MAIN_CONTROL_DESTROY)GetProcAddress(hDLL, "ADL_Main_Control_Destroy");
    if (NULL != ADL_Main_Control_Destroy)
        ADL_Main_Control_Destroy();

#if defined (LINUX)
    dlclose(hDLL);
#else
    FreeLibrary(hDLL);
#endif
}


#include "ADLQuery.h"
//ToDO ADLMemoryInfo bandwidth

//typedef struct _ADLAdapterInfo {
//    int iAdapterIndex;
//    bool active;
//    int busNumber;
//    int deviceNumber;
//    int functionNumber;
//} ADLAdapterInfo;
//ADLMemoryInfo

int ADLQueryAdapterInfo(ADLAdapterInfo *adapterInfo, int listLen){
    ADL_ADAPTER_NUMBEROFADAPTERS_GET ADL_Adapter_NumberOfAdapters_Get;
    ADL_ADAPTER_ADAPTERINFO_GET      ADL_Adapter_AdapterInfo_Get;
    ADL_ADAPTER_ACTIVE_GET           ADL_Adapter_Active_Get;
    ADL_ADAPTER_MEMORYINFO_GET       ADL_Adapter_MemoryInfo_Get;
    ADL_ADAPTER_GCOREINFO_GET        ADL_Adapter_GCoreInfo_Get;
    // ?? ADLGraphicCoreInfo

    int  ADL_Err = 0;

    int iNumberAdapters = 0;
    int iAdapterIndex = 0;
    ADL_Err = OpenADL();
    if (ADL_Err != 0) {
        return 0;
    }
    ADL_Adapter_NumberOfAdapters_Get = (ADL_ADAPTER_NUMBEROFADAPTERS_GET)GetProcAddress(hDLL, "ADL_Adapter_NumberOfAdapters_Get");
    ADL_Adapter_AdapterInfo_Get = (ADL_ADAPTER_ADAPTERINFO_GET)GetProcAddress(hDLL, "ADL_Adapter_AdapterInfo_Get");
    ADL_Adapter_Active_Get = (ADL_ADAPTER_ACTIVE_GET)GetProcAddress(hDLL, "ADL_Adapter_Active_Get");
    ADL_Adapter_MemoryInfo_Get = (ADL_ADAPTER_MEMORYINFO_GET)GetProcAddress(hDLL, "ADL_Adapter_MemoryInfo_Get");
    ADL_Adapter_GCoreInfo_Get = (ADL_ADAPTER_GCOREINFO_GET)GetProcAddress(hDLL, "ADL2_Adapter_Graphic_Core_Info_Get");
    ADLMemoryInfo memInfo;
    ADLGraphicCoreInfo gcInfo;

    // Obtain the number of adapters for the system
    ADL_Adapter_NumberOfAdapters_Get(&iNumberAdapters);
    if (listLen < iNumberAdapters){
        return iNumberAdapters;
    }

    if (0 < iNumberAdapters)
    {
        lpAdapterInfo = (LPAdapterInfo)malloc(sizeof(AdapterInfo) * iNumberAdapters);
        memset(lpAdapterInfo, '\0', sizeof(AdapterInfo) * iNumberAdapters);

        // Get the AdapterInfo structure for all adapters in the system
        ADL_Adapter_AdapterInfo_Get(lpAdapterInfo, sizeof(AdapterInfo) * iNumberAdapters);
    }
    else
    {
        //DisplayErrorAndPause("ERROR: No adapters found in this system!");
        CloseADL();
        return ADL_Err;
    }

    int iActive = 0;
    // Repeat for all available adapters in the system
    for (int i = 0; i < iNumberAdapters; i++)
    {
        iAdapterIndex = lpAdapterInfo[i].iAdapterIndex;
        ADL_Err = ADL_Adapter_Active_Get(iAdapterIndex, &iActive);

#ifdef DEBUG
        printf("iAdapterIndex: %d\n", lpAdapterInfo[i].iAdapterIndex);
        printf("iBusNumber: %d\n", lpAdapterInfo[i].iBusNumber);
        printf("iDeviceNumber: %d\n", lpAdapterInfo[i].iDeviceNumber);
        printf("iFunctionNumber: %d\n", lpAdapterInfo[i].iFunctionNumber);
        printf("iOSDisplayIndex: %d\n", lpAdapterInfo[i].iOSDisplayIndex);
        printf("iVendorID: %d\n", lpAdapterInfo[i].iVendorID);
        printf("iExist: %d\n", lpAdapterInfo[i].iExist);
        printf("iPresent: %d\n", lpAdapterInfo[i].iPresent);
        printf("iSize: %d\n", lpAdapterInfo[i].iSize);
        printf("iActive: %d\n\n", iActive);
#endif


        adapterInfo[i].iAdapterIndex = iAdapterIndex;
        adapterInfo[i].busNumber = lpAdapterInfo[i].iBusNumber;
        adapterInfo[i].deviceNumber = lpAdapterInfo[i].iDeviceNumber;
        adapterInfo[i].functionNumber = lpAdapterInfo[i].iFunctionNumber;
        adapterInfo[i].active = iActive;

        ADL_Adapter_MemoryInfo_Get(i, &memInfo);
        adapterInfo[i].totalMemory = memInfo.iMemorySize;
        adapterInfo[i].memoryBandwidth = memInfo.iMemoryBandwidth;

        memset(&gcInfo, 0, sizeof(gcInfo));
        ADL_Adapter_GCoreInfo_Get(NULL, i, &gcInfo);

        adapterInfo[i].numCUs = gcInfo.iNumCUs;

    }

    CloseADL();
    return iNumberAdapters;
}