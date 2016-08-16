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

typedef int(__cdecl *listGpuDeviceNamesType)(char *devNames[], unsigned int count);
typedef int(__cdecl *getDeviceAndContextType)(int devIdx, cl_context *pContext, cl_device_id *pDevice);
typedef cl_command_queue(__cdecl *createQueueType)(cl_context context, cl_device_id device, int flag, int cu_);

// TAN objects creation functions.
extern "C"
{
    __declspec(dllexport) int __cdecl  listGpuDeviceNames(char *devNames[], unsigned int count);
    __declspec(dllexport) int __cdecl getDeviceAndContext(int devIdx, cl_context *pContext, cl_device_id *pDevice);
    __declspec(dllexport) cl_command_queue __cdecl createQueue(cl_context context, cl_device_id device, int flag = 0, int cu_ = 0);
}
