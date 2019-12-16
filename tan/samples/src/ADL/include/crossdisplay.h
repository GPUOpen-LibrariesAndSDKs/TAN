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

/// \file crossdisplay.h
/// \brief Defines CrossDisplay-specific public functions on \ALL platforms.
///
/// This file contains all CrossDisplay-related functions exposed by ADL for public use on \ALL platforms.

/// Functions in this file include ones for retrieving CrossDisplay capabilities, retrieving CrossDisplay status, and setting CrossDisplay configurations.

#ifndef CROSSDISPLAY_H_
#define CROSSDISPLAY_H_

#ifndef ADL_EXTERNC
#ifdef __cplusplus
#define ADL_EXTERNC extern "C"
#else
#define ADL_EXTERNC
#endif
#endif

#include "adl_structures.h"

/// \addtogroup CROSSDISPLAY
// @{
///
/// \brief Function to retrieve CrossDisplay capabilities.
/// 
/// The output parameters of the function include the CrossDisplay supported flag, supported CrossDisplay roles of an adapter, and its partner adapters. If the adapter is capable of playing a display role, 
/// the number of possible rendering adapters (lpNnumPosRenderingAdapters) and possible rendering adapters (lppPosRenderingAdapters) parameters are valid. If the adapter is capable of playing a rendering role, the number of possible display adapters (lpNumPossDisplayAdapters) and the 
/// possible display adapters (lppPossDisplayAdapters) parameters are valid. If the adapter is capable of playing a display-rendering role, all the output parameters are valid.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the adapter to which the CrossDisplay capability information is applied.
/// \param[out]  lpCrossDisplaySupport The pointer to the address of the flag representing CrossDisplay support. If the system supports CrossDisplay 
///             and the input adapter has or will have a role in the CrossDisplay configuration, the content of the pointer is 1, otherwise it is 0.
///             This pointer should be initialized by the user.
/// \param[out] lpAdapterRole The pointer to the supported roles of the input adapter. This pointer should be initialized by the user. Possible output values are 0, 1, 2, and 3.
///             0: The input adapter does not support any CrossDisplay roles.
///            	1: The input adapter supports a rendering role.
///				2: The input adpater supports a display role.
///				3: The input adapter supports both rendering and display roles.
/// \param[out] lpNumPossDisplayAdapters The pointer to the number of possible display adapters. This pointer should be initialized by the user.
/// \param[out] lppPossDisplayAdapters The pointer to the pointer to the first possible display adapter address. This pointer does not need to be initialized by the user.
/// \param[out] lpNnumPosRenderingAdapters The pointer to the number of possible rendering adapters. This pointer should be initialized by the user.
/// \param[out] lppPosRenderingAdapters The pointer to the pointer to the first possible rendering adapter address. This pointer does not need to be initialized by the user.
/// \param[out] lpErrorStatus The pointer to the driver error or warning code. This pointer should be initialized by the user.
/// \return     If the function succeeds, the returned value is \ref ADL_OK. Otherwise, the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_CrossdisplayAdapterRole_Caps(ADL_CONTEXT_HANDLE context,int iAdapterIndex, 
													 int *lpCrossDisplaySupport, 
													 int *lpAdapterRole,
													 int *lpNumPossDisplayAdapters, 
													 int **lppPossDisplayAdapters,
													 int *lpNnumPosRenderingAdapters,
													 int **lppPosRenderingAdapters,	
													 int *lpErrorStatus);

///
/// \brief Function to retrieve CrossDisplay capabilities.
/// 
/// The output parameters of the function include the CrossDisplay supported flag, supported CrossDisplay roles of an adapter, and its partner adapters. If the adapter is capable of playing a display role, 
/// the number of possible rendering adapters (lpNnumPosRenderingAdapters) and possible rendering adapters (lppPosRenderingAdapters) parameters are valid. If the adapter is capable of playing a rendering role, the number of possible display adapters (lpNumPossDisplayAdapters) and the 
/// possible display adapters (lppPossDisplayAdapters) parameters are valid. If the adapter is capable of playing a display-rendering role, all the output parameters are valid.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the adapter to which the CrossDisplay capability information is applied.
/// \param[out]  lpCrossDisplaySupport The pointer to the address of the flag representing CrossDisplay support. If the system supports CrossDisplay 
///             and the input adapter has or will have a role in the CrossDisplay configuration, the content of the pointer is 1, otherwise it is 0.
///             This pointer should be initialized by the user.
/// \param[out] lpAdapterRole The pointer to the supported roles of the input adapter. This pointer should be initialized by the user. Possible output values are 0, 1, 2, and 3.
///             0: The input adapter does not support any CrossDisplay roles.
///            	1: The input adapter supports a rendering role.
///				2: The input adpater supports a display role.
///				3: The input adapter supports both rendering and display roles.
/// \param[out] lpNumPossDisplayAdapters The pointer to the number of possible display adapters. This pointer should be initialized by the user.
/// \param[out] lppPossDisplayAdapters The pointer to the pointer to the first possible display adapter address. This pointer does not need to be initialized by the user.
/// \param[out] lpNnumPosRenderingAdapters The pointer to the number of possible rendering adapters. This pointer should be initialized by the user.
/// \param[out] lppPosRenderingAdapters The pointer to the pointer to the first possible rendering adapter address. This pointer does not need to be initialized by the user.
/// \param[out] lpErrorStatus The pointer to the driver error or warning code. This pointer should be initialized by the user.
/// \return     If the function succeeds, the returned value is \ref ADL_OK. Otherwise, the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Adapter_CrossdisplayAdapterRole_Caps(int iAdapterIndex, 
													 int *lpCrossDisplaySupport, 
													 int *lpAdapterRole,
													 int *lpNumPossDisplayAdapters, 
													 int **lppPossDisplayAdapters,
													 int *lpNnumPosRenderingAdapters,
													 int **lppPosRenderingAdapters,	
													 int *lpErrorStatus);
 
///
/// \brief Function to retrieve CrossDisplay state information.
/// 
/// The output parameters of the function include the adapter role, the CrossDisplay mode and its partner adapters.
/// If the adapter is in CrossDisplay mode and the adapter role is display, the number of rendering adapters (lpNnumPosRenderingAdapters) and rendering adapters (lppPosRenderingAdapters) parameters are valid.
/// If the adapter is in CrossDisplay mode and the adapter role is rendering, the number of display adapters (lpNumPossDisplayAdapters) and display adapters (lppPossDisplayAdapters) parameters are valid.
/// If the adapter is in multi-adapter mode, only the the adapter role (lpAdapterRole) and CrossDisplay mode (lpCrossDisplaySupport) parameters are valid.
///


/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the adapter to which the CrossDisplay state information is applied.
/// \param[out] lpAdapterRole The pointer to the retrieved adapter role. This pointer should be initialized by the user. Possible output values are 0, 1, and 2. 
///             0: The input adapter does not support any CrossDisplay roles.
///            	1: The input adapter supports a rendering role.
///				2: The input adpater supports a display role.
/// \param[out] lpCrossdisplayMode The pointer to the retrieved CrossDisplay mode. This pointer should be initialized by the user. Possible output values are 0 and 1. 
///				0: Multi-adapter mode.
///				1: CrossDisplay mode.		
/// \param[out] lpNumDisplayAdapters The pointer to the number of retrieved display adapters. This pointer should be initialized by the user.
/// \param[out] lppDisplayAdapters The pointer to the pointer to the first display adapter address. This pointer does not need to be initialized by the user.
/// \param[out] lpNumRenderingAdapters The pointer to the number of possible rendering adapters. This pointer should be initialized by the user.
/// \param[out] lppRenderingAdapters The pointer to the pointer to the first rendering adapter address. This pointer does not need to be initialized by the user.
/// \param[out] lpErrorCodeStatus The pointer to the driver error or warning code. This pointer should be initialized by the user.
/// \return If the function succeeds, the returned value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_CrossdisplayInfo_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex,
					int *lpAdapterRole,
					int *lpCrossdisplayMode,
					int *lpNumDisplayAdapters,
					int **lppDisplayAdapters,
					int *lpNumRenderingAdapters,
					int **lppRenderingAdapters,			
					int *lpErrorCodeStatus 
 ) ;

/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the adapter to which the CrossDisplay state information is applied.
/// \param[out] lpAdapterRole The pointer to the retrieved adapter role. This pointer should be initialized by the user. Possible output values are 0, 1, and 2. 
///             0: The input adapter does not support any CrossDisplay roles.
///            	1: The input adapter supports a rendering role.
///				2: The input adpater supports a display role.
/// \param[out] lpCrossdisplayMode The pointer to the retrieved CrossDisplay mode. This pointer should be initialized by the user. Possible output values are 0 and 1. 
///				0: Multi-adapter mode.
///				1: CrossDisplay mode.		
/// \param[out] lpNumDisplayAdapters The pointer to the number of retrieved display adapters. This pointer should be initialized by the user.
/// \param[out] lppDisplayAdapters The pointer to the pointer to the first display adapter address. This pointer does not need to be initialized by the user.
/// \param[out] lpNumRenderingAdapters The pointer to the number of possible rendering adapters. This pointer should be initialized by the user.
/// \param[out] lppRenderingAdapters The pointer to the pointer to the first rendering adapter address. This pointer does not need to be initialized by the user.
/// \param[out] lpErrorCodeStatus The pointer to the driver error or warning code. This pointer should be initialized by the user.
/// \return If the function succeeds, the returned value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Adapter_CrossdisplayInfo_Get(int iAdapterIndex,
					int *lpAdapterRole,
					int *lpCrossdisplayMode,
					int *lpNumDisplayAdapters,
					int **lppDisplayAdapters,
					int *lpNumRenderingAdapters,
					int **lppRenderingAdapters,			
					int *lpErrorCodeStatus 
 ) ;
 

///
/// \brief Function to set the system configuration to CrossDisplay mode or multi-adapter mode.
/// 
/// This public function enables/disables CrossDisplay mode configuration
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the adapter to which the CrossDisplay configuration change is applied.
/// \param[in]  iDisplayAdapterIndex The display adapter index.
/// \param[in]  iRenderingAdapterIndex The rendering adapter index.
/// \param[in]  iCrossdisplayMode The desired configuration. Possible values are 0 and 1. 
///				0: Multi-adapter mode.
///				1: CrossDisplay mode.
/// \param[out] lpErrorCode The pointer to the driver error or warning code. This pointer should be initialized by the user.
/// \return If the function succeeds, the returned value is \ref ADL_OK or \ref ADL_OK_WAIT. Otherwise, the return value is an ADL error code otherwise.  \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_CrossdisplayInfo_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex,
					int iDisplayAdapterIndex,
					int iRenderingAdapterIndex,
 					int iCrossdisplayMode,
					int *lpErrorCode
					) ;

///
/// \brief Function to set the system configuration to CrossDisplay mode or multi-adapter mode.
/// 
/// This public function enables/disables CrossDisplay mode configuration
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the adapter to which the CrossDisplay configuration change is applied.
/// \param[in]  iDisplayAdapterIndex The display adapter index.
/// \param[in]  iRenderingAdapterIndex The rendering adapter index.
/// \param[in]  iCrossdisplayMode The desired configuration. Possible values are 0 and 1. 
///				0: Multi-adapter mode.
///				1: CrossDisplay mode.
/// \param[out] lpErrorCode The pointer to the driver error or warning code. This pointer should be initialized by the user.
/// \return If the function succeeds, the returned value is \ref ADL_OK or \ref ADL_OK_WAIT. Otherwise, the return value is an ADL error code otherwise.  \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Adapter_CrossdisplayInfo_Set(int iAdapterIndex,
					int iDisplayAdapterIndex,
					int iRenderingAdapterIndex,
 					int iCrossdisplayMode,
					int *lpErrorCode
					) ;


///
///\brief Function to get the graphics platform information of of an adapter.
/// 
/// This function retrieves the graphics platform information of a specified adapter.\n
/// \platform
/// \Win7Vista 
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpXDPlatformInfoMask The pointer to the graphics platform information mask. \ref ADL_CROSSDISPLAY_PLATFORM_XXX
/// \param[out]  lpXDPlatformInfoValue The pointer to the graphics platform information value. \ref ADL_CROSSDISPLAY_PLATFORM_XXX.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_CrossDisplayPlatformInfo_Get (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int *lpXDPlatformInfoMask, int *lpXDPlatformInfoValue);

///
///\brief Function to get the graphics platform information of of an adapter.
/// 
/// This function retrieves the graphics platform information of a specified adapter.\n
/// \platform
/// \Win7Vista 
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpXDPlatformInfoMask The pointer to the graphics platform information mask. \ref ADL_CROSSDISPLAY_PLATFORM_XXX
/// \param[out]  lpXDPlatformInfoValue The pointer to the graphics platform information value. \ref ADL_CROSSDISPLAY_PLATFORM_XXX.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Adapter_CrossDisplayPlatformInfo_Get ( int iAdapterIndex, int *lpXDPlatformInfoMask, int *lpXDPlatformInfoValue);

///
/// \brief Function to set the system configuration to CrossDisplay mode or multi-adapter mode.
/// 
/// This public function enables/disables CrossDisplay mode configuration
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the adapter to which the CrossDisplay configuration change is applied.
/// \param[in]  iDisplayAdapterIndex The display adapter index.
/// \param[in]  iRenderingAdapterIndex The rendering adapter index.
/// \param[in]  iCrossdisplayMode The desired configuration. Possible values are 0 and 1. 
///				0: Multi-adapter mode.
///				1: CrossDisplay mode.
/// \param[in]  iCrossdisplayOption The value contains extra option bits used in cross display switching. \ref ADL_CROSSDISPLAY_OPTION_XXX
///				\ref ADL_CROSSDISPLAY_OPTION_NONE: Checking if 3D application is runnning. If yes, not to do switch, return ADL_OK_WAIT; otherwise do switch.
///				\ref ADL_CROSSDISPLAY_OPTION_FORCESWITCH: Force switching without checking for running 3D applications. 
/// \param[out] lpErrorCode The pointer to the driver error or warning code. This pointer should be initialized by the user.
/// \return If the function succeeds, the returned value is \ref ADL_OK or \ref ADL_OK_WAIT. Otherwise, the return value is an ADL error code otherwise.  \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_CrossdisplayInfoX2_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex,
					int iDisplayAdapterIndex,
					int iRenderingAdapterIndex,
 					int iCrossdisplayMode,
					int iCrossdisplayOption,
					int *lpErrorCode
					) ;

///
/// \brief Function to set the system configuration to CrossDisplay mode or multi-adapter mode.
/// 
/// This public function enables/disables CrossDisplay mode configuration
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the adapter to which the CrossDisplay configuration change is applied.
/// \param[in]  iDisplayAdapterIndex The display adapter index.
/// \param[in]  iRenderingAdapterIndex The rendering adapter index.
/// \param[in]  iCrossdisplayMode The desired configuration. Possible values are 0 and 1. 
///				0: Multi-adapter mode.
///				1: CrossDisplay mode.
/// \param[in]  iCrossdisplayOption The value contains extra option bits used in cross display switching. \ref ADL_CROSSDISPLAY_OPTION_XXX
///				\ref ADL_CROSSDISPLAY_OPTION_NONE: Checking if 3D application is runnning. If yes, not to do switch, return ADL_OK_WAIT; otherwise do switch.
///				\ref ADL_CROSSDISPLAY_OPTION_FORCESWITCH: Force switching without checking for running 3D applications. 
/// \param[out] lpErrorCode The pointer to the driver error or warning code. This pointer should be initialized by the user.
/// \return If the function succeeds, the returned value is \ref ADL_OK or \ref ADL_OK_WAIT. Otherwise, the return value is an ADL error code otherwise.  \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Adapter_CrossdisplayInfoX2_Set(int iAdapterIndex,
					int iDisplayAdapterIndex,
					int iRenderingAdapterIndex,
 					int iCrossdisplayMode,
					int iCrossdisplayOption,
					int *lpErrorCode
					) ;



// @}

#endif /* CROSSDISPLAY_H_ */
