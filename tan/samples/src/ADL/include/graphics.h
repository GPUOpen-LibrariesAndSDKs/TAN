///
///  Copyright (c) 2008 - 2009 Advanced Micro Devices, Inc.
 
///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

/// \file adapter.h
/// \brief Contains all graphics-related functions exposed by ADL for \ALL platforms.
///
/// This file contains all graphics-related functions exposed by ADL for \ALL platforms.
/// All functions found in this file can be used as a reference when creating
/// specific function pointers to be used by the appropriate runtime
/// dynamic library loaders.

#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#ifndef ADL_EXTERNC
#ifdef __cplusplus
#define ADL_EXTERNC extern "C"
#else
#define ADL_EXTERNC
#endif
#endif

/// \addtogroup MAINAPI
// @{

/////
///// \brief Function to retrieve the type of graphics platform the system is currently running.
///// 
///// This function determines whether the current graphics platform is a mobile, desktop.
///// \platform 
///// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
///// \param[out] lpPlatForm The pointer to the retrieved graphics platform for the system. \n
/////  lpPlatForm will be either GRAPHICS_PLATFORM_DESKTOP (for desktop platforms) or GRAPHICS_PLATFORM_MOBILE (for mobile platforms)
///// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Graphics_Platform_Get(ADL_CONTEXT_HANDLE context,int *lpPlatForm);

/////
///// \brief Function to retrieve the type of graphics platform the system is currently running.
///// 
///// This function determines whether the current graphics platform is a mobile, desktop.
///// \platform 
///// \WIN
///// \param[out] lpPlatForm The pointer to the retrieved graphics platform for the system. \n
/////  lpPlatForm will be either GRAPHICS_PLATFORM_DESKTOP (for desktop platforms) or GRAPHICS_PLATFORM_MOBILE (for mobile platforms)
///// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Graphics_Platform_Get(int *lpPlatForm);
// @}


#endif /* GRAPHICS_H_ */

