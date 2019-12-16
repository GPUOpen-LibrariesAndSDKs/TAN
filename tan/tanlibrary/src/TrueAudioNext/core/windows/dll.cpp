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

#include "tanlibrary/include/TrueAudioNext.h"   //TAN
#include "public/include/core/Platform.h"       //AMF
#include "public/common/AMFFactory.h"           //AMF

HINSTANCE           g_hTANModule = NULL;

TAN_SDK_LINK BOOL WINAPI DllMain(
  _In_ HINSTANCE hinstDLL,
  _In_ DWORD     fdwReason,
  _In_ LPVOID    lpvReserved
);


BOOL WINAPI DllMain(
  _In_ HINSTANCE hinstDLL,
  _In_ DWORD     fdwReason,
  _In_ LPVOID    lpvReserved
)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        g_hTANModule = hinstDLL;
        DisableThreadLibraryCalls(hinstDLL);
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
        // terminate AMF
        AMF_RESULT res = g_AMFFactory.Terminate();
        if (res != AMF_OK)
        {
            wprintf(L"AMF Factory Failed to terminate");
        }
    }
    return TRUE;
}

/*

enum AMF_COMPUTE_QUEUE_TYPE
{
    AMF_COMPUTE_QUEUE_NORMAL = 0,
    AMF_COMPUTE_QUEUE_MEDIUM_PRIORITY = 1
    AMF_COMPUTE_QUEUE_RESERVED_CU = 2
};
struct AMFRealTimeQueue
{
    AMF_COMPUTE_QUEUE_TYPE  queueType;
    int                     CU_COUNT;   //for 
};

*/
