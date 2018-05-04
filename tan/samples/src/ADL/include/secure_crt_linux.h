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
