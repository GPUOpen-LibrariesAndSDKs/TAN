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

#ifndef __IIRLOWLAT__
#define __IIRLOWLAT__


#include <assert.h>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<string.h>

//#include <atlstr.h>
//#include <atlconv.h>
#include <omp.h>
#ifdef WIN32
#include <io.h>
#include <windows.h>
#include <BaseTsd.h>
//#include "wintime.h"

#ifdef USE_RESTRICT
#else
#define restrict
#endif

#define snprintf _snprintf 
#define vsnprintf _vsnprintf 
#define strcasecmp _stricmp 
#define strncasecmp _strnicmp
typedef unsigned int uint;

static
double mach_absolute_time()
{
double ret = 0;
__int64 frec;
__int64 clocks;
    QueryPerformanceFrequency((LARGE_INTEGER *)&frec);
    QueryPerformanceCounter((LARGE_INTEGER *)&clocks);
    ret = (double)clocks * 1000. / (double) frec; 
	return(ret);
}



#else  // WIN32


#include<strings.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/resource.h>
typedef  long long int __int64;


#endif


#include<math.h>
#include<omp.h>
#include <CL/cl.h>
#include <CL/cl_ext.h>

#include "GraalInternal.h"

#endif
