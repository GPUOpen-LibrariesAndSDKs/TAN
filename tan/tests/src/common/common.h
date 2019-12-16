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

#include "tanlibrary/include/TrueAudioNext.h"

#define QUEUE_NORMAL_QUEUE 0

#ifdef RTQ_ENABLED
#define QUEUE_MEDIUM_PRIORITY                   0x00010000
#define QUEUE_REAL_TIME_COMPUTE_UNITS           0x00020000
#endif
#define RETURN_IF_FAILED(x) { AMF_RESULT tmp = (x); if (tmp != AMF_OK) { return tmp; } }
#define RETURN_FALSE_IF_FAILED(x) { AMF_RESULT tmp = (x); if (tmp != AMF_OK) { return false; } }
enum eTestMode
{
	eGPU,
	eCPU
};

enum eCommandQueueType
{
#ifdef RTQ_ENABLED
	eMediumPriorityQueue = 2,
	eRealTimeQueue = 1,
#endif
	eNormalQueue = 0
};

#ifdef RTQ_ENABLED
enum eCommandQueueType
{
	eMediumPriorityQueue,
	eRealTimeQueue
};
#endif RTQ_ENABLED


enum eOutputFlag
{
	eInput = 1,
	eReferenceOutput = 2,
	eTestOutput = 4,
	eDiff = 8,
	eResponse = 16
};

amf_uint random(int maxVal);

float random_float(float maxVal);