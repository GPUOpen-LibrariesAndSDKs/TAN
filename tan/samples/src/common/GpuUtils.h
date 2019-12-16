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
#include <CL/cl.h>

int listGpuDeviceNamesWrapper(char *devNames[], unsigned int count);
int listCpuDeviceNamesWrapper(char *devNames[], unsigned int count);

bool CreateGpuCommandQueues(int deviceIndex, int32_t flag1, cl_command_queue* pcmdQueue1, int32_t flag2, cl_command_queue* pcmdQueue2);
bool CreateCpuCommandQueues(int deviceIndex, int32_t flag1, cl_command_queue* pcmdQueue1, int32_t flag2, cl_command_queue* pcmdQueue2);

bool CreateCommandQueuesWithCUcount(int deviceIndex,cl_command_queue* pcmdQueue1, cl_command_queue* pcmdQueue2, int Q1CUcount, int Q2CUcount);
