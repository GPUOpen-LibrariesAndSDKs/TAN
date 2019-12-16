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

#ifndef __SECURE_CRT_LNX_H__
#define __SECURE_CRT_LNX_H__

#include <stdio.h>
#include <string.h>
#include <wchar.h>


/*
This file contains linux warppers for the secure CRT functions that are available in Windows.

The wrapper implementation will simply wrap secure functions to insecure functions, to allow cross-platform common code to compile.
it will NOT actually implement the security features.

2012 - Wei Liang
*/

#ifndef _ERRNO_T_DEFINED
#define _ERRNO_T_DEFINED
typedef int errno_t;
#endif

errno_t strncpy_s(
   char *strDest,
   size_t numberOfElements,
   const char *strSource, size_t count
);

errno_t strcpy_s(
   char *strDestination,
   size_t numberOfElements,
   const char *strSource 
);

errno_t strcat_s(
   char *strDestination,
   size_t numberOfElements,
   const char *strSource 
);

errno_t wcstombs_s(
   size_t *pReturnValue,
   char *mbstr,
   size_t sizeInBytes,
   const wchar_t *wcstr,
   size_t count 
);

errno_t wcscpy_s(
   wchar_t *strDestination,
   size_t numberOfElements,
   const wchar_t *strSource 
);

errno_t wcsncpy_s(
   wchar_t *strDest,
   size_t numberOfElements,
   const wchar_t *strSource,
   size_t count 
);

errno_t wcscat_s(
   wchar_t *strDestination,
   size_t numberOfElements,
   const wchar_t *strSource 
);

errno_t wctomb_s(
   int *pRetValue,
   char *mbchar,
   size_t sizeInBytes,
   wchar_t wchar 
);

#endif /*__SECURE_CRT_LNX_H__*/
