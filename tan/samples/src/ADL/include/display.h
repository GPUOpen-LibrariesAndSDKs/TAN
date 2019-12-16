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

/// \file display.h
/// \brief Contains all Display functions exposed by ADL for \ALL platforms
///
/// This file contains all display-related functions exposed by ADL for \ALL platforms.
/// All functions in this file can be used as a reference to ensure
/// the appropriate function pointers can be used by the appropriate runtime
/// dynamic library loaders.

#ifndef DISPLAY_H_
#define DISPLAY_H_


#include "adl_structures.h"

#ifndef ADL_EXTERNC
#ifdef __cplusplus
#define ADL_EXTERNC extern "C"
#else
#define ADL_EXTERNC
#endif
#endif

#ifndef EXPOSED
#define EXPOSED
#endif /* EXPOSED */

/// \addtogroup DISPLAY
// @{

///
/// \brief Function to retrieve the adapter display information.
/// 
/// This function retrieves display information for a specified adapter. 
/// Display information includes display index, name, type, and display connection status, etc.\n
/// This API uses the Caller's Memory Allocation Callback Function to allocate memory pointed by lppInfo. Caller is responsible to de-alocate the memory.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter. 
/// \param[out] lpNumDisplays The pointer to the number of displays detected. 
/// \param[out] lppInfo The pointer to the pointer to the retrieved display information array. Initialize to NULL before calling this API. Refer to the ADLDisplayInfo structure for more information.
/// \param[in]  iForceDetect   \b 0: Do not force detection of the adapters in the system; \b 1 : Force detection
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_DisplayInfo_Get (ADL_CONTEXT_HANDLE context,int iAdapterIndex, 
										  int* lpNumDisplays, 
										  ADLDisplayInfo ** lppInfo, 
										  int iForceDetect);

///
/// \brief Function to retrieve the adapter display information.
/// 
/// This function retrieves display information for a specified adapter. 
/// Display information includes display index, name, type, and display connection status, etc.\n
/// This API uses the Caller's Memory Allocation Callback Function to allocate memory pointed by lppInfo. Caller is responsible to de-alocate the memory.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter. 
/// \param[out] lpNumDisplays The pointer to the number of displays detected. 
/// \param[out] lppInfo The pointer to the pointer to the retrieved display information array. Initialize to NULL before calling this API. Refer to the ADLDisplayInfo structure for more information.
/// \param[in]  iForceDetect   \b 0: Do not force detection of the adapters in the system; \b 1 : Force detection
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
ADL_EXTERNC int EXPOSED ADL_Display_DisplayInfo_Get (int iAdapterIndex, 
										  int* lpNumDisplays, 
										  ADLDisplayInfo ** lppInfo, 
										  int iForceDetect);

///
/// \brief Function to retrieve the Display Port MST information.
/// 
/// This function retrieves display port MST information for a specified adapter. 
/// Display port MST information includes display indexes, names, bandwidth, identifiers, relative address and display connection, etc.\n
/// This API uses the Caller's Memory Allocation Callback Function to allocate memory pointed by lppDisplayDPMstInfo. Caller is responsible to de-alocate the memory.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter. A value of -1 returns all displays in the system across multiple GPUs.
/// \param[out] lpNumDisplays The pointer to the number of displays detected. 
/// \param[out] lppDisplayDPMstInfo The pointer to the list of the retrieved display port MST information array. Initialize to NULL before calling this API. Refer to the ADLDisplayDPMSTInfo structure for more information.
/// \param[in]  iForceDetect   ADL_FALSE: Do not force detection of the adapters in the system; ADL_TRUE: Force detection
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_DpMstInfo_Get (ADL_CONTEXT_HANDLE context,int iAdapterIndex, 
                                          int* lpNumDisplays, 
                                          ADLDisplayDPMSTInfo ** lppDisplayDPMstInfo,
										  int iForceDetect);

///
/// \brief Function to retrieve the Display Port MST information.
/// 
/// This function retrieves display port MST information for a specified adapter. 
/// Display port MST information includes display indexes, names, bandwidth, identifiers, relative address and display connection, etc.\n
/// This API uses the Caller's Memory Allocation Callback Function to allocate memory pointed by lppDisplayDPMstInfo. Caller is responsible to de-alocate the memory.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter. A value of -1 returns all displays in the system across multiple GPUs.
/// \param[out] lpNumDisplays The pointer to the number of displays detected. 
/// \param[out] lppDisplayDPMstInfo The pointer to the list of the retrieved display port MST information array. Initialize to NULL before calling this API. Refer to the ADLDisplayDPMSTInfo structure for more information.
/// \param[in]  iForceDetect   ADL_FALSE: Do not force detection of the adapters in the system; ADL_TRUE: Force detection
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
ADL_EXTERNC int EXPOSED ADL_Display_DpMstInfo_Get (int iAdapterIndex, 
                                          int* lpNumDisplays, 
                                          ADLDisplayDPMSTInfo ** lppDisplayDPMstInfo,
										  int iForceDetect);


///
///\brief Function to retrieve the number of displays supported by an adapter. 
/// 
/// This function retrieves the number of displays that is supported by a specified adapter. Supported displays include devices that are connected and
/// disconnected together with those enabled and disabled devices.   
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpNumDisplays The pointer to the number of displays supported by the adapter.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_NumberOfDisplays_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int *lpNumDisplays);

///
///\brief Function to retrieve the number of displays supported by an adapter. 
/// 
/// This function retrieves the number of displays that is supported by a specified adapter. Supported displays include devices that are connected and
/// disconnected together with those enabled and disabled devices.   
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpNumDisplays The pointer to the number of displays supported by the adapter.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Display_NumberOfDisplays_Get(int iAdapterIndex, int *lpNumDisplays);

///
///\brief Function to retrieve the display perserved aspect ratio of an adapter. 
/// 
/// This function retrieves the current, default, and the feature supported values of the display preserved aspect ratio.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpSupport The pointer to the value indicating whether the feature is supported by the driver. 1: feature is supported; 0: feature is not supported.
/// \param[out] lpCurrent The pointer to the current setting of display preserved aspect ratio. Its alternative value is 1 or 0.
/// \param[out] lpDefault The pointer to the default setting of display preserved aspect ratio. Its alternative value is 1 or 0.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_PreservedAspectRatio_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int* lpSupport, int * lpCurrent, int * lpDefault);

///
///\brief Function to retrieve the display perserved aspect ratio of an adapter. 
/// 
/// This function retrieves the current, default, and the feature supported values of the display preserved aspect ratio.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpSupport The pointer to the value indicating whether the feature is supported by the driver. 1: feature is supported; 0: feature is not supported.
/// \param[out] lpCurrent The pointer to the current setting of display preserved aspect ratio. Its alternative value is 1 or 0.
/// \param[out] lpDefault The pointer to the default setting of display preserved aspect ratio. Its alternative value is 1 or 0.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Display_PreservedAspectRatio_Get(int iAdapterIndex, int iDisplayIndex, int* lpSupport, int * lpCurrent, int * lpDefault);


///
///\brief Function to set the display preserved aspect ratio. 
/// 
/// This function sets the current value of display preserved aspect ratio.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in] iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in] iCurrent The desired display preserved aspect ratio setting. Possible values are 1 or 0.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_PreservedAspectRatio_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex,  int iCurrent);

///
///\brief Function to set the display preserved aspect ratio. 
/// 
/// This function sets the current value of display preserved aspect ratio.
/// \platform
/// \ALL
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in] iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in] iCurrent The desired display preserved aspect ratio setting. Possible values are 1 or 0.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
ADL_EXTERNC int EXPOSED ADL_Display_PreservedAspectRatio_Set(int iAdapterIndex, int iDisplayIndex,  int iCurrent);

///
///\brief Function to retrieve the display image expansion setting. 
/// 
/// This function retrieves the current, default, and the feature supported values of the image expansion setting.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpSupport The pointer to the value indicating whether the feature is supported by the driver. 1: feature is supported; 0: feature is not supported.
/// \param[out] lpCurrent The pointer to the current setting of display image expansion. Its alternative value is 1 or 0.
/// \param[out] lpDefault The pointer to the default setting of display image expansion. Its alternative value is 1 or 0.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_ImageExpansion_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int* lpSupport, int * lpCurrent, int * lpDefault);

///
///\brief Function to retrieve the display image expansion setting. 
/// 
/// This function retrieves the current, default, and the feature supported values of the image expansion setting.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpSupport The pointer to the value indicating whether the feature is supported by the driver. 1: feature is supported; 0: feature is not supported.
/// \param[out] lpCurrent The pointer to the current setting of display image expansion. Its alternative value is 1 or 0.
/// \param[out] lpDefault The pointer to the default setting of display image expansion. Its alternative value is 1 or 0.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Display_ImageExpansion_Get(int iAdapterIndex, int iDisplayIndex, int* lpSupport, int * lpCurrent, int * lpDefault);

///
///\brief Function to set the display image expansion setting. 
/// 
/// This function sets the current value of display image expansion setting.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in] iCurrent The desired setting of display image expansion. Possible values are 1 or 0.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_ImageExpansion_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int iCurrent);

///
///\brief Function to set the display image expansion setting. 
/// 
/// This function sets the current value of display image expansion setting.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in] iCurrent The desired setting of display image expansion. Possible values are 1 or 0.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
ADL_EXTERNC int EXPOSED ADL_Display_ImageExpansion_Set(int iAdapterIndex, int iDisplayIndex, int iCurrent);

///
///\brief Function to get Device Display Position. 
/// 
/// This function retrieves the display position parameters for a specified adapter and display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpX The pointer to the current X coordinate display position.
/// \param[out] lpY The pointer to the current Y coordinate display position.
/// \param[out] lpXDefault The pointer to the default X coordinate display position.
/// \param[out] lpYDefault The pointer to the default Y coordinate display position.
/// \param[out] lpMinX The pointer to the minimum X display size.
/// \param[out] lpMinY The pointer to the minimum Y display size.
/// \param[out] lpMaxX The pointer to the maximum X display size.
/// \param[out] lpMaxY The pointer to the maximum Y display size.
/// \param[out] lpStepX The pointer to the step size along the X axis.
/// \param[out] lpStepY The pointer to the step size along the Y axis.
/// \return	If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_Position_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int* lpX, int* lpY, int* lpXDefault, int* lpYDefault, 
									 int* lpMinX, int* lpMinY, int* lpMaxX, int*lpMaxY, int* lpStepX, int* lpStepY);

///
///\brief Function to get Device Display Position. 
/// 
/// This function retrieves the display position parameters for a specified adapter and display.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpX The pointer to the current X coordinate display position.
/// \param[out] lpY The pointer to the current Y coordinate display position.
/// \param[out] lpXDefault The pointer to the default X coordinate display position.
/// \param[out] lpYDefault The pointer to the default Y coordinate display position.
/// \param[out] lpMinX The pointer to the minimum X display size.
/// \param[out] lpMinY The pointer to the minimum Y display size.
/// \param[out] lpMaxX The pointer to the maximum X display size.
/// \param[out] lpMaxY The pointer to the maximum Y display size.
/// \param[out] lpStepX The pointer to the step size along the X axis.
/// \param[out] lpStepY The pointer to the step size along the Y axis.
/// \return	If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Display_Position_Get(int iAdapterIndex, int iDisplayIndex, int* lpX, int* lpY, int* lpXDefault, int* lpYDefault, 
									 int* lpMinX, int* lpMinY, int* lpMaxX, int*lpMaxY, int* lpStepX, int* lpStepY);


///
///\brief Function to set the Device Display Position. 
/// 
/// This function sets the display X and Y position values for a specified adapter and display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iX The desired X coordinate position.
/// \param[in]  iY The desired Y coordinate position.
/// \return	If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_Position_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int iX, int iY);

///
///\brief Function to set the Device Display Position. 
/// 
/// This function sets the display X and Y position values for a specified adapter and display.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iX The desired X coordinate position.
/// \param[in]  iY The desired Y coordinate position.
/// \return	If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
ADL_EXTERNC int EXPOSED ADL_Display_Position_Set(int iAdapterIndex, int iDisplayIndex, int iX, int iY);

///
///\brief Function to get the Device Display Size. 
/// 
/// This function retrieves the display position parameters.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpWidth The pointer to the current display width.
/// \param[out] lpHeight The pointer to the current display height.
/// \param[out] lpDefaultWidth The pointer to the default display width.
/// \param[out] lpDefaultHeight The pointer to the default display height.
/// \param[out] lpMinWidth The pointer to the minimum display width.
/// \param[out] lpMinHeight The pointer to the minimum display height.
/// \param[out] lpMaxWidth The pointer to the maximum display width.
/// \param[out] lpMaxHeight The pointer to the maximum display height.
/// \param[out] lpStepWidth The pointer to the step width.
/// \param[out] lpStepHeight The pointer to the step height.
/// \return	If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_Size_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int* lpWidth, int* lpHeight, int* lpDefaultWidth, int* lpDefaultHeight, 
									 int* lpMinWidth, int* lpMinHeight, int* lpMaxWidth, int*lpMaxHeight, int* lpStepWidth, int* lpStepHeight);

///
///\brief Function to get the Device Display Size. 
/// 
/// This function retrieves the display position parameters.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpWidth The pointer to the current display width.
/// \param[out] lpHeight The pointer to the current display height.
/// \param[out] lpDefaultWidth The pointer to the default display width.
/// \param[out] lpDefaultHeight The pointer to the default display height.
/// \param[out] lpMinWidth The pointer to the minimum display width.
/// \param[out] lpMinHeight The pointer to the minimum display height.
/// \param[out] lpMaxWidth The pointer to the maximum display width.
/// \param[out] lpMaxHeight The pointer to the maximum display height.
/// \param[out] lpStepWidth The pointer to the step width.
/// \param[out] lpStepHeight The pointer to the step height.
/// \return	If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Display_Size_Get(int iAdapterIndex, int iDisplayIndex, int* lpWidth, int* lpHeight, int* lpDefaultWidth, int* lpDefaultHeight, 
									 int* lpMinWidth, int* lpMinHeight, int* lpMaxWidth, int*lpMaxHeight, int* lpStepWidth, int* lpStepHeight);

///
///\brief Function to set the Device Display Size. 
/// 
/// This function sets the display width and height values.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iWidth The desired width of the display.
/// \param[in]  iHeight The desired height of the display.
/// \return	If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_Size_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int iWidth, int iHeight);

///
///\brief Function to set the Device Display Size. 
/// 
/// This function sets the display width and height values.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iWidth The desired width of the display.
/// \param[in]  iHeight The desired height of the display.
/// \return	If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
ADL_EXTERNC int EXPOSED ADL_Display_Size_Set(int iAdapterIndex, int iDisplayIndex, int iWidth, int iHeight);

///
///\brief Function to retrieve the adjustment display information.
/// 
/// This function retrieves the display adjustment information for a specified adapter and display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpInfo The pointer to the adjustment information retrieved from the driver.\n
///	\ref define_adjustment_capabilities
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_AdjustCaps_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int* lpInfo);

///
///\brief Function to retrieve the adjustment display information.
/// 
/// This function retrieves the display adjustment information for a specified adapter and display.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpInfo The pointer to the adjustment information retrieved from the driver.\n
///	\ref define_adjustment_capabilities
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Display_AdjustCaps_Get(int iAdapterIndex, int iDisplayIndex, int* lpInfo);

///
///\brief Function to get the number of displays and controllers supported by an adapter.
/// 
/// This function retrieves the number of displays and controllers that are supported by a specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpNumberOfControlers The pointer to the variable storing the total number of controllers. This variable must be initialized.
/// \param[out]  lpNumberOfDisplays The pointer to the variable storing the total number of displays. This variable must be initialized.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_Capabilities_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int* lpNumberOfControlers, int* lpNumberOfDisplays);

///
///\brief Function to get the number of displays and controllers supported by an adapter.
/// 
/// This function retrieves the number of displays and controllers that are supported by a specified adapter.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpNumberOfControlers The pointer to the variable storing the total number of controllers. This variable must be initialized.
/// \param[out]  lpNumberOfDisplays The pointer to the variable storing the total number of displays. This variable must be initialized.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_Capabilities_Get(int iAdapterIndex, int* lpNumberOfControlers, int* lpNumberOfDisplays);

///
///\brief Function to indicate whether displays are physically connected to an adapter.
/// 
/// This function indicates whether displays are physically connected to a specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpConnections The pointer to the bit field indicating whether the output connectors on the specified adapter have devices physically attached to them.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_ConnectedDisplays_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int * lpConnections);

///
///\brief Function to indicate whether displays are physically connected to an adapter.
/// 
/// This function indicates whether displays are physically connected to a specified adapter.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpConnections The pointer to the bit field indicating whether the output connectors on the specified adapter have devices physically attached to them.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_ConnectedDisplays_Get(int iAdapterIndex, int * lpConnections);

///
///\brief Function to get HDTV capability settings.
/// 
/// This function retrieves HDTV capability settings for a specified display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpDisplayConfig The pointer to the retrieved display configuration.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_DeviceConfig_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, ADLDisplayConfig * lpDisplayConfig);

///
///\brief Function to get HDTV capability settings.
/// 
/// This function retrieves HDTV capability settings for a specified display.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpDisplayConfig The pointer to the retrieved display configuration.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_DeviceConfig_Get(int iAdapterIndex, int iDisplayIndex, ADLDisplayConfig * lpDisplayConfig);

///
///\brief Function to get the current display property value.
/// 
/// This function retrieves the current display property value for a specified display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpDisplayProperty The pointer to the ADLDisplayProperty structure storing the retrieved display property value. lpDisplayProperty->iExpansionMode contains the Expansion Mode value.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_Property_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, ADLDisplayProperty * lpDisplayProperty);

///
///\brief Function to get the current display property value.
/// 
/// This function retrieves the current display property value for a specified display.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpDisplayProperty The pointer to the ADLDisplayProperty structure storing the retrieved display property value. lpDisplayProperty->iExpansionMode contains the Expansion Mode value.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_Property_Get(int iAdapterIndex, int iDisplayIndex, ADLDisplayProperty * lpDisplayProperty);

///
///\brief Function to set current display property value.
/// 
/// This function sets current display property value for a specified display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  lpDisplayProperty The pointer to the desired ADLDisplayProperty structure. Set lpDisplayProperty->iExpansionMode with an Expansion Mode value.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_Property_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, ADLDisplayProperty * lpDisplayProperty);

///
///\brief Function to set current display property value.
/// 
/// This function sets current display property value for a specified display.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  lpDisplayProperty The pointer to the desired ADLDisplayProperty structure. Set lpDisplayProperty->iExpansionMode with an Expansion Mode value.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
ADL_EXTERNC int EXPOSED ADL_Display_Property_Set(int iAdapterIndex, int iDisplayIndex, ADLDisplayProperty * lpDisplayProperty);

///
///\brief Function to retrieve the Display Switching Flag from the registry.
/// 
/// This function retrieves the Display Switching Flag from the registery for a specified adapter.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpResult The pointer to value storing the retrieved flag. 1: Driver will not accept display switching request;  0: User can request display switching.

/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_SwitchingCapability_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int *lpResult);

/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_SwitchingCapability_Get(int iAdapterIndex, int *lpResult);

///
///\brief Function to retrieve the dither state.
/// 
/// This function retrieves the dither state for a specified display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpDitherState The pointer to the value storing the retrieved dither state. See \ref define_dither_states
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_DitherState_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int *lpDitherState);

///
///\brief Function to retrieve the dither state.
/// 
/// This function retrieves the dither state for a specified display.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpDitherState The pointer to the value storing the retrieved dither state. See \ref define_dither_states
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_DitherState_Get(int iAdapterIndex, int iDisplayIndex, int *lpDitherState);

///
///\brief Function to set the dither state.
/// 
/// This function sets the dither state. 
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]   iDitherState The desired dither state. For the list of valid states see \ref define_dither_states
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks The final bit depth after dithering must match the input bit depth of the monitor. The function call will fail if it doesn't.
/// \remarks The settings are immediately persisted, no need to call ADL_Flush_Driver_Data().
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_DitherState_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int iDitherState);

///
///\brief Function to set the dither state.
/// 
/// This function sets the dither state.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]   iDitherState The desired dither state. For the list of valid states see \ref define_dither_states
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks The final bit depth after dithering must match the input bit depth of the monitor. The function call will fail if it doesn't.
/// \remarks The settings are immediately persisted, no need to call ADL_Flush_Driver_Data().
ADL_EXTERNC int EXPOSED ADL_Display_DitherState_Set(int iAdapterIndex, int iDisplayIndex, int iDitherState);

///
///\brief Function to retrieve the supported pixel format.  \b HDMI \b only
/// 
/// This function retrieves the supported pixel format.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]   lpPixelFormat The pointer to the supported pixel format. \ref define_pixel_formats
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_SupportedPixelFormat_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int *lpPixelFormat); 

///
///\brief Function to retrieve the supported pixel format.  \b HDMI \b only
/// 
/// This function retrieves the supported pixel format.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]   lpPixelFormat The pointer to the supported pixel format. \ref define_pixel_formats
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_SupportedPixelFormat_Get(int iAdapterIndex, int iDisplayIndex, int *lpPixelFormat); 

///
///\brief Function to retrieve the current display pixel format.  \b HDMI \b only
/// 
/// This function retrieves the current display pixel format.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpPixelFormat The pointer to the pixel format. \ref define_pixel_formats
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_PixelFormat_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int *lpPixelFormat);

///
///\brief Function to retrieve the current display pixel format.  \b HDMI \b only
/// 
/// This function retrieves the current display pixel format.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpPixelFormat The pointer to the pixel format. \ref define_pixel_formats
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_PixelFormat_Get(int iAdapterIndex, int iDisplayIndex, int *lpPixelFormat);

///
///\brief Function to retrieve the current display pixel format.  \b HDMI \b only
/// 
/// This function retrieves the current display pixel format.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpDefPixelFormat The pointer to the default pixel format. \ref define_pixel_formats
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_PixelFormatDefault_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayIndex, int *lpDefPixelFormat);

///
///\brief Function to set the current display pixel format.  \b HDMI \b only
/// 
/// This function sets the current display pixel format.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]   iPixelFormat The desired pixel format. \ref define_pixel_formats
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_PixelFormat_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int iPixelFormat);

///
///\brief Function to set the current display pixel format.  \b HDMI \b only
/// 
/// This function sets the current display pixel format.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]   iPixelFormat The desired pixel format. \ref define_pixel_formats
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
ADL_EXTERNC int EXPOSED ADL_Display_PixelFormat_Set(int iAdapterIndex, int iDisplayIndex, int iPixelFormat);

///
///\brief Function to retrieve the supported color depth.  \b HDMI and DP\b only
/// 
/// This function retrieves the current display on given adpater’s supported color depths
/// --When supported Color depth is one format only, no need expose the User interface.
/// \platform
/// \ALL
/// \param[in]  context:        Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex:  the ADL index handle of the desired adapter per physical GPU.
/// \param[in]  iDisplayIndex:  The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpColorDepth:   The pointer to the bit vector of all display supported color depths. \ref define_color_depth_bits
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL2_Display_SupportedColorDepth_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayIndex, int *lpColorDepth);

///
///\brief Function to retrieve the supported color depth.  \b HDMI and DP\b only
/// 
/// This function retrieves the current display on given adpater’s supported color depths
/// --When supported Color depth is one format only, no need expose the User interface.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex: the ADL index handle of the desired adapter per physical GPU.
/// \param[in]  iDisplayIndex: The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpColorDepth: The pointer to the bit vector of all display supported color depths. \ref define_color_depth_bits
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_SupportedColorDepth_Get(int iAdapterIndex, int iDisplayIndex, int *lpColorDepth);

///
///\brief Function to get color depth.  \b HDMI and DP\b only
/// 
/// This function retrieves the current display on given adpater’s current color depth.
/// \platform
/// \ALL
/// \param[in]   context:       Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex: The ADL index handle of the desired adapter per physical GPU.
/// \param[in]   iDisplayIndex: The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpColorDepth:  The pointer to the current color depth setting for given display. \ref define_colordepth_constants
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL2_Display_ColorDepth_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayIndex, int *lpColorDepth);

///
///\brief Function to get color depth.  \b HDMI and DP\b only
/// 
/// This function retrieves the current display on given adpater’s current color depth.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex: The ADL index handle of the desired adapter per physical GPU.
/// \param[in]   iDisplayIndex: The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpColorDepth:  The pointer to the current color depth setting for given display. \ref define_colordepth_constants
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_ColorDepth_Get(int iAdapterIndex, int iDisplayIndex, int *lpColorDepth);

///
///\brief Function to set color depth.  \b HDMI and DP\b only
/// 
/// This function sets a user select color depth to current display on given adapter.
/// \platform
/// \ALL
/// \param[in]  context:        Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex:  The ADL index handle of the desired adapter per physical GPU.
/// \param[in]  iDisplayIndex:  The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iColorDepth:    Requested color depth. \ref define_colordepth_constants
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL2_Display_ColorDepth_Set(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayIndex, int iColorDepth);

///
///\brief Function to set color depth.  \b HDMI and DP\b only
/// 
/// This function sets a user select color depth to current display on given adapter.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex:  The ADL index handle of the desired adapter per physical GPU.
/// \param[in]  iDisplayIndex:  The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iColorDepth:    Requested color depth. \ref define_colordepth_constants
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_ColorDepth_Set(int iAdapterIndex, int iDisplayIndex, int iColorDepth);

///
///\brief Function to retrieve the OD clock information.
/// 
/// This function retrieves the OD clock information.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpOdClockInfo The pointer to the structure storing the retrieved clock information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_ODClockInfo_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLAdapterODClockInfo *lpOdClockInfo);

///
///\brief Function to retrieve the OD clock information.
/// 
/// This function retrieves the OD clock information.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpOdClockInfo The pointer to the structure storing the retrieved clock information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_ODClockInfo_Get(int iAdapterIndex, ADLAdapterODClockInfo *lpOdClockInfo);

///
///\brief Function to set the OD clock configuration.
/// 
/// This function sets the OD clock configuration.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   lpOdClockConfig The pointer to the structure used to set the OD clock configuration.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_ODClockConfig_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLAdapterODClockConfig *lpOdClockConfig);

///
///\brief Function to set the OD clock configuration.
/// 
/// This function sets the OD clock configuration.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   lpOdClockConfig The pointer to the structure used to set the OD clock configuration.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
ADL_EXTERNC int EXPOSED ADL_Display_ODClockConfig_Set(int iAdapterIndex, ADLAdapterODClockConfig *lpOdClockConfig);

///
/// \brief Function to retrieve the adjustment coherent setting.
/// 
/// This function retrieves the current adjustment coherent setting\n
/// It is applicable to DFP/DFP2 display types.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpAdjustmentCoherentCurrent The pointer to the retrieved adjustment coherent setting. 1: Coherent; 0: non-coherent
/// \param[out]  lpAdjustmentCoherentDefault The pointer to the retrieved default setting. 1: Coherent; 0: Non-coherent
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code.   \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_AdjustmentCoherent_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int *lpAdjustmentCoherentCurrent, int *lpAdjustmentCoherentDefault);

///
/// \brief Function to retrieve the adjustment coherent setting.
/// 
/// This function retrieves the current adjustment coherent setting\n
/// It is applicable to DFP/DFP2 display types.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpAdjustmentCoherentCurrent The pointer to the retrieved adjustment coherent setting. 1: Coherent; 0: non-coherent
/// \param[out]  lpAdjustmentCoherentDefault The pointer to the retrieved default setting. 1: Coherent; 0: Non-coherent
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code.   \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_AdjustmentCoherent_Get(int iAdapterIndex, int iDisplayIndex, int *lpAdjustmentCoherentCurrent, int *lpAdjustmentCoherentDefault);

///
///\brief Function to set the adjustment coherent setting.
/// 
/// This function sets the current adjustment coherent setting to a specified setting.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iAdjustmentCoherent The desired adjustment coherent setting. 1: Coherent; 0: Non-coherent
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_AdjustmentCoherent_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int iAdjustmentCoherent);

///
///\brief Function to set the adjustment coherent setting.
/// 
/// This function sets the current adjustment coherent setting to a specified setting.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iAdjustmentCoherent The desired adjustment coherent setting. 1: Coherent; 0: Non-coherent
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
ADL_EXTERNC int EXPOSED ADL_Display_AdjustmentCoherent_Set(int iAdapterIndex, int iDisplayIndex, int iAdjustmentCoherent);

///
///\brief Function to retrieve the reduced blanking setting.
/// 
/// This function retrieves the current reduced blanking setting.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpReducedBlankingCurrent The pointer to the current reduced blanking setting.
/// \param[out]  lpReducedBlankingDefault The pointer to the retrieved default setting.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code.   \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_ReducedBlanking_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int *lpReducedBlankingCurrent, int *lpReducedBlankingDefault);

///
///\brief Function to retrieve the reduced blanking setting.
/// 
/// This function retrieves the current reduced blanking setting.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpReducedBlankingCurrent The pointer to the current reduced blanking setting.
/// \param[out]  lpReducedBlankingDefault The pointer to the retrieved default setting.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code.   \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_ReducedBlanking_Get(int iAdapterIndex, int iDisplayIndex, int *lpReducedBlankingCurrent, int *lpReducedBlankingDefault);

///
///\brief Function to set the reduced blanking setting.
/// 
/// This function sets the current reduced blanking setting to a specified setting.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iReducedBlanking The desired reduced blanking setting.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code.   \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_ReducedBlanking_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int iReducedBlanking);

///
///\brief Function to set the reduced blanking setting.
/// 
/// This function sets the current reduced blanking setting to a specified setting.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iReducedBlanking The desired reduced blanking setting.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code.   \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
ADL_EXTERNC int EXPOSED ADL_Display_ReducedBlanking_Set(int iAdapterIndex, int iDisplayIndex, int iReducedBlanking);

///
/// \brief Function to retrieve the available display formats 
///
/// This call retrieves the available display formats
/// \platform 
/// \ALL 
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpSettingsSupported The pointer to the supported settings. \ref define_formats_ovr
/// \param[out]  lpSettingsSupportedEx The pointer to the extended supported settings. \ref define_formats_ovr 
/// \param[out]  lpCurSettings The pointer to the current override settings \ref define_formats_ovr
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_FormatsOverride_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int* lpSettingsSupported, int* lpSettingsSupportedEx, int* lpCurSettings);

///
/// \brief Function to retrieve the available display formats 
///
/// This call retrieves the available display formats
/// \platform 
/// \ALL 
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpSettingsSupported The pointer to the supported settings. \ref define_formats_ovr
/// \param[out]  lpSettingsSupportedEx The pointer to the extended supported settings. \ref define_formats_ovr 
/// \param[out]  lpCurSettings The pointer to the current override settings \ref define_formats_ovr
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_FormatsOverride_Get(int iAdapterIndex, int iDisplayIndex, int* lpSettingsSupported, int* lpSettingsSupportedEx, int* lpCurSettings);

///
/// \brief Function to overide the display formats
///
/// This call overrides the display formats
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iOverrideSettings The new format settings \ref define_formats_ovr
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_FormatsOverride_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int iOverrideSettings);

///
/// \brief Function to overide the display formats
///
/// This call overrides the display formats
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iOverrideSettings The new format settings \ref define_formats_ovr
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
ADL_EXTERNC int EXPOSED ADL_Display_FormatsOverride_Set(int iAdapterIndex, int iDisplayIndex, int iOverrideSettings);

///
///\brief Function to retrieve the information about MultiVPU capabilities.
/// 
/// This function retrieves the information about MultiVPU capabilities.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in,out]  lpMvpuCaps The structure storing the retrieved information about MultiVPU capabilities.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks The maximum number of adapters is \ref ADL_DL_MAX_MVPU_ADAPTERS
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_MVPUCaps_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLMVPUCaps *lpMvpuCaps);

///
///\brief Function to retrieve the information about MultiVPU capabilities.
/// 
/// This function retrieves the information about MultiVPU capabilities.
/// \platform
/// \WIN
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in,out]  lpMvpuCaps The structure storing the retrieved information about MultiVPU capabilities.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks The maximum number of adapters is \ref ADL_DL_MAX_MVPU_ADAPTERS
ADL_EXTERNC int EXPOSED ADL_Display_MVPUCaps_Get(int iAdapterIndex, ADLMVPUCaps *lpMvpuCaps);

///
///\brief Function to retrieve information about MultiVPU status.
/// 
/// This function retrieves information about MultiVPU status.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in,out]  lpMvpuStatus The structure storing the retrieved information about MultiVPU status.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_MVPUStatus_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLMVPUStatus *lpMvpuStatus);

///
///\brief Function to retrieve information about MultiVPU status.
/// 
/// This function retrieves information about MultiVPU status.
/// \platform
/// \WIN
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in,out]  lpMvpuStatus The structure storing the retrieved information about MultiVPU status.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_MVPUStatus_Get(int iAdapterIndex, ADLMVPUStatus *lpMvpuStatus);

// @}

///\addtogroup OVERLAY
// @{

///
///\brief Function to get the minimum, maximum, and default values of an overlay adjustment.
/// 
/// This function retrieves the minimum, maximum, and default values of an overlay adjustment for a specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   lpOverlayInput The pointer to the ADLControllerOverlayInput structure.
/// \param[in,out]  lpCapsInfo The pointer to the ADLControllerOverlayInfo structure storing the retrieved overlay adjustment information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_ControllerOverlayAdjustmentCaps_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLControllerOverlayInput *lpOverlayInput, ADLControllerOverlayInfo *lpCapsInfo);

///
///\brief Function to get the minimum, maximum, and default values of an overlay adjustment.
/// 
/// This function retrieves the minimum, maximum, and default values of an overlay adjustment for a specified adapter.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   lpOverlayInput The pointer to the ADLControllerOverlayInput structure.
/// \param[in,out]  lpCapsInfo The pointer to the ADLControllerOverlayInfo structure storing the retrieved overlay adjustment information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_ControllerOverlayAdjustmentCaps_Get(int iAdapterIndex, ADLControllerOverlayInput *lpOverlayInput, ADLControllerOverlayInfo *lpCapsInfo);

///
///\brief Function to retrieve the current setting of an overlay adjustment.
/// 
/// This function retrieves the current setting of an overlay adjustment.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpOverlay The pointer to the retrieved overlay adjustment ADLControllerOverlayInput structure.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_ControllerOverlayAdjustmentData_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLControllerOverlayInput * lpOverlay);

///
///\brief Function to retrieve the current setting of an overlay adjustment.
/// 
/// This function retrieves the current setting of an overlay adjustment.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpOverlay The pointer to the retrieved overlay adjustment ADLControllerOverlayInput structure.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_ControllerOverlayAdjustmentData_Get(int iAdapterIndex, ADLControllerOverlayInput * lpOverlay);

///
///\brief Function to set the current setting of an overlay adjustment.
/// 
/// This function sets the current setting of an overlay adjustment.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in,out]  lpOverlay The pointer to the overlay adjustment ADLControllerOverlayInput structure.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_ControllerOverlayAdjustmentData_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLControllerOverlayInput * lpOverlay);

///
///\brief Function to set the current setting of an overlay adjustment.
/// 
/// This function sets the current setting of an overlay adjustment.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in,out]  lpOverlay The pointer to the overlay adjustment ADLControllerOverlayInput structure.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
ADL_EXTERNC int EXPOSED ADL_Display_ControllerOverlayAdjustmentData_Set(int iAdapterIndex, ADLControllerOverlayInput * lpOverlay);

// @}

///\addtogroup VIEWPORT
// @{

///
///\brief Function to change the view position, view size or view pan lock of a selected display.
/// 
/// This function used to change the view position, view size or view pan lock of a selected display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]   lpControllerMode. The pointer to the ADLControllerMode structure.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_ViewPort_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, ADLControllerMode * lpControllerMode);

///
///\brief Function to change the view position, view size or view pan lock of a selected display.
/// 
/// This function used to change the view position, view size or view pan lock of a selected display.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]   lpControllerMode. The pointer to the ADLControllerMode structure.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_ViewPort_Set(int iAdapterIndex, int iDisplayIndex, ADLControllerMode * lpControllerMode);

///
///\brief Function to get the view position, view size or view pan lock of a selected display.
/// 
/// This function used to get the view position, view size or view pan lock of a selected display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]      iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]      iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]     lpControllerMode The pointer to the ADLControllerMode structure.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_ViewPort_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, ADLControllerMode * lpControllerMode);

///
///\brief Function to get the view position, view size or view pan lock of a selected display.
/// 
/// This function used to get the view position, view size or view pan lock of a selected display.
/// \platform
/// \ALL
/// \param[in]      iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]      iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]     lpControllerMode The pointer to the ADLControllerMode structure.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_ViewPort_Get(int iAdapterIndex, int iDisplayIndex, ADLControllerMode * lpControllerMode);

///
///\brief Function to check if the selected adapter supports the view port control.
/// 
/// This function used to check if the selected adapter supports the view port control.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]      iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]     lpSupported is if this adapter supports the view port control. ADL_TRUE: the view port control are supported; otherwise: not supported.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_ViewPort_Cap(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int* lpSupported);

///
///\brief Function to check if the selected adapter supports the view port control.
/// 
/// This function used to check if the selected adapter supports the view port control.
/// \platform
/// \ALL
/// \param[in]      iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]     lpSupported is if this adapter supports the view port control. ADL_TRUE: the view port control are supported; otherwise: not supported.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_ViewPort_Cap(int iAdapterIndex, int* lpSupported);

// @}


///\addtogroup I2CDDCEDID
// @{

///
///\brief Function to retrieve the I2C API revision.
/// 
/// This function retrieves the I2C API revision.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpMajor The pointer to the major version.
/// \param[out]  lpMinor The pointer to the minor version.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_WriteAndReadI2CRev_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int *lpMajor, int *lpMinor);

///
///\brief Function to retrieve the I2C API revision.
/// 
/// This function retrieves the I2C API revision.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpMajor The pointer to the major version.
/// \param[out]  lpMinor The pointer to the minor version.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_WriteAndReadI2CRev_Get(int iAdapterIndex, int *lpMajor, int *lpMinor);

///
/// \brief Function to write and read I2C.
/// 
/// This function writes and reads I2C.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in,out]  plI2C A pointer to the ADLI2C structure. 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_WriteAndReadI2C(ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLI2C *plI2C);

///
/// \brief Function to write and read I2C.
/// 
/// This function writes and reads I2C.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in,out]  plI2C A pointer to the ADLI2C structure. 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_WriteAndReadI2C(int iAdapterIndex, ADLI2C *plI2C);

///
///\brief Function to get Display DDC block access.
/// 
/// This function provides means for applications to send/receive data in the DDC information block via the 12C bus.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iOption Combination of \ref ADL_DDC_OPTION_SWITCHDDC2 and \ref ADL_DDC_OPTION_RESTORECOMMAND  
/// \param[in]  iCommandIndex The index of the command to be saved in the registry.
/// This parameter is used only when \ref ADL_DDC_OPTION_RESTORECOMMAND of \b iOption is specified. Otherwize set this parameter to 0.
/// \param[in]  iSendMsgLen The size of the send message buffer.
/// \param[in]  lpucSendMsgBuf The pointer to the send message buffer.
/// \param[out] lpulRecvMsgLen The pointer to the size of the receive message buffer.
/// \param[out] lpucRecvMsgBuf The pointer to the retrieved receive message buffer.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks To send data without receiving anything back, specify non-zero send message size and non-NULL buffer,
/// then set the receive message size to 0 and buffer to NULL.\n
/// To receive data, you have to send a command defined by the I2C protocol to the display, so you will always have to send
/// some sequence of bytes to the display, even if you only want to read from the display.\n
/// To send and receive data at the same time, specify non-zero send and receive message size and non-NULL send and receive
/// message buffers. You will have to worry about sending proper commands to the display, as defined by the I2C protocol.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_DDCBlockAccess_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int iOption, int iCommandIndex,int iSendMsgLen, char *lpucSendMsgBuf, int *lpulRecvMsgLen, char *lpucRecvMsgBuf);

///
///\brief Function to get Display DDC block access.
/// 
/// This function provides means for applications to send/receive data in the DDC information block via the 12C bus.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iOption Combination of \ref ADL_DDC_OPTION_SWITCHDDC2 and \ref ADL_DDC_OPTION_RESTORECOMMAND  
/// \param[in]  iCommandIndex The index of the command to be saved in the registry.
/// This parameter is used only when \ref ADL_DDC_OPTION_RESTORECOMMAND of \b iOption is specified. Otherwize set this parameter to 0.
/// \param[in]  iSendMsgLen The size of the send message buffer.
/// \param[in]  lpucSendMsgBuf The pointer to the send message buffer.
/// \param[out] lpulRecvMsgLen The pointer to the size of the receive message buffer.
/// \param[out] lpucRecvMsgBuf The pointer to the retrieved receive message buffer.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks To send data without receiving anything back, specify non-zero send message size and non-NULL buffer,
/// then set the receive message size to 0 and buffer to NULL.\n
/// To receive data, you have to send a command defined by the I2C protocol to the display, so you will always have to send
/// some sequence of bytes to the display, even if you only want to read from the display.\n
/// To send and receive data at the same time, specify non-zero send and receive message size and non-NULL send and receive
/// message buffers. You will have to worry about sending proper commands to the display, as defined by the I2C protocol.
ADL_EXTERNC int EXPOSED ADL_Display_DDCBlockAccess_Get(int iAdapterIndex, int iDisplayIndex, int iOption, int iCommandIndex,int iSendMsgLen, char *lpucSendMsgBuf, int *lpulRecvMsgLen, char *lpucRecvMsgBuf);

///
///\brief Function to get the DDC info. 
/// 
/// This function retrieves the display device config (DDC) information only if the device has the information.
/// No assumption should be made that this call will cause the driver will do real time detection.
/// The driver determines when DDC detection should take place. If a display is attached and it supports DDC,
/// all available information should be returned and ADLDDCInfo.ulSupportsDDC should be set to 1.
/// Any fields that are not supported by an attached DDC display should be zeroed on return.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpInfo The pointer to the ADLDDCInfo structure storing all DDC retrieved from the driver.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_DDCInfo_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, ADLDDCInfo* lpInfo);

///
///\brief Function to get the DDC info. 
/// 
/// This function retrieves the display device config (DDC) information only if the device has the information.
/// No assumption should be made that this call will cause the driver will do real time detection.
/// The driver determines when DDC detection should take place. If a display is attached and it supports DDC,
/// all available information should be returned and ADLDDCInfo.ulSupportsDDC should be set to 1.
/// Any fields that are not supported by an attached DDC display should be zeroed on return.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpInfo The pointer to the ADLDDCInfo structure storing all DDC retrieved from the driver.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Display_DDCInfo_Get(int iAdapterIndex, int iDisplayIndex, ADLDDCInfo* lpInfo);

///
///\brief Function to get the DDC info. 
/// 
/// This function retrieves the display device config (DDC) information only if the device has the information.
/// No assumption should be made that this call will cause the driver will do real time detection.
/// The driver determines when DDC detection should take place. If a display is attached and it supports DDC,
/// all available information should be returned and ADLDDCInfo.ulSupportsDDC should be set to 1.
/// Any fields that are not supported by an attached DDC display should be zeroed on return.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpInfo The pointer to the ADLDDCInfo structure storing all DDC retrieved from the driver.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_DDCInfo2_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, ADLDDCInfo2* lpInfo);

///
///\brief Function to get the DDC info. 
/// 
/// This function retrieves the display device config (DDC) information only if the device has the information.
/// No assumption should be made that this call will cause the driver will do real time detection.
/// The driver determines when DDC detection should take place. If a display is attached and it supports DDC,
/// all available information should be returned and ADLDDCInfo.ulSupportsDDC should be set to 1.
/// Any fields that are not supported by an attached DDC display should be zeroed on return.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpInfo The pointer to the ADLDDCInfo structure storing all DDC retrieved from the driver.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Display_DDCInfo2_Get(int iAdapterIndex, int iDisplayIndex, ADLDDCInfo2* lpInfo);


///
///\brief Function to get the EDID data.
/// 
/// This function retrieves the EDID data for a specififed display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in,out]  lpEDIDData The pointer to the ADLDisplayEDIDData structure storing the retrieved EDID data.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_EdidData_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, ADLDisplayEDIDData *lpEDIDData);

///
///\brief Function to get the EDID data.
/// 
/// This function retrieves the EDID data for a specififed display.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in,out]  lpEDIDData The pointer to the ADLDisplayEDIDData structure storing the retrieved EDID data.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_EdidData_Get(int iAdapterIndex, int iDisplayIndex, ADLDisplayEDIDData *lpEDIDData);

// @}

///\addtogroup COLOR
// @{

///
///\brief Function to get the Color Caps display information. 
/// 
/// This function retrieves the display Color Caps information for a specified adapter and display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpCaps The pointer to the Color Caps information retrieved from the driver. \ref define_color_type
/// \param[out] lpValids The pointer to the bit vector indicating which bit is valid on the lpCaps returned.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_ColorCaps_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int* lpCaps, int* lpValids);

///
///\brief Function to get the Color Caps display information. 
/// 
/// This function retrieves the display Color Caps information for a specified adapter and display.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpCaps The pointer to the Color Caps information retrieved from the driver. \ref define_color_type
/// \param[out] lpValids The pointer to the bit vector indicating which bit is valid on the lpCaps returned.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Display_ColorCaps_Get(int iAdapterIndex, int iDisplayIndex, int* lpCaps, int* lpValids);

///
///\brief Function to set the current value of a specific color and type.  
/// 
/// This function sets the display color for a specified adapter, display, and color type.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iColorType The desired color type.  It can be any of the defined type. \ref define_color_type
/// \param[in]  iCurrent The desired color value.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_Color_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int iColorType, int iCurrent);

///
///\brief Function to set the current value of a specific color and type.  
/// 
/// This function sets the display color for a specified adapter, display, and color type.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iColorType The desired color type.  It can be any of the defined type. \ref define_color_type
/// \param[in]  iCurrent The desired color value.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
ADL_EXTERNC int EXPOSED ADL_Display_Color_Set(int iAdapterIndex, int iDisplayIndex, int iColorType, int iCurrent);

///
///\brief Function to retrieve the detailed information a specified display color item.
/// 
/// This function retrieves the detailed information for a specified display color.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iColorType The desired color type.  It can be any of the defined type. \ref define_color_type
/// \param[out] lpCurrent The pointer to the current color value.
/// \param[out] lpDefault The pointer to the default color value.
/// \param[out] lpMin The pointer to the minimum color value.
/// \param[out] lpMax The pointer to the maximum color value.
/// \param[out] lpStep The pointer to the increment of the color setting.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_Color_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int iColorType, int* lpCurrent, int* lpDefault, int* lpMin, int* lpMax, int* lpStep);

///
///\brief Function to retrieve the detailed information a specified display color item.
/// 
/// This function retrieves the detailed information for a specified display color.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iColorType The desired color type.  It can be any of the defined type. \ref define_color_type
/// \param[out] lpCurrent The pointer to the current color value.
/// \param[out] lpDefault The pointer to the default color value.
/// \param[out] lpMin The pointer to the minimum color value.
/// \param[out] lpMax The pointer to the maximum color value.
/// \param[out] lpStep The pointer to the increment of the color setting.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Display_Color_Get(int iAdapterIndex, int iDisplayIndex, int iColorType, int* lpCurrent, int* lpDefault, int* lpMin, int* lpMax, int* lpStep);

///
///\brief Function to get color temperature source.
/// 
/// This function retrieves the current color temperature source.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpTempSource The pointer of the retrieved current color temperature source. \ref define_color_type
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code.   \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_ColorTemperatureSource_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int *lpTempSource);

///
///\brief Function to get color temperature source.
/// 
/// This function retrieves the current color temperature source.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpTempSource The pointer of the retrieved current color temperature source. \ref define_color_type
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code.   \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_ColorTemperatureSource_Get(int iAdapterIndex, int iDisplayIndex, int *lpTempSource);

///
///\brief Function to get default color temperature source.
///
/// This function retrieves the default color temperature source.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpTempSourceDefault The pointer of the retrieved default color temperature source. \ref define_color_type
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code.   \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_ColorTemperatureSourceDefault_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int *lpTempSourceDefault);

///
/// \brief Function to set the color temperature source.
/// 
/// This function sets the current color temperature source.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iTempSource The desired color temperature source. \ref define_color_type
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code.   \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_ColorTemperatureSource_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int iTempSource);

///
/// \brief Function to set the color temperature source.
/// 
/// This function sets the current color temperature source.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iTempSource The desired color temperature source. \ref define_color_type
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code.   \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
ADL_EXTERNC int EXPOSED ADL_Display_ColorTemperatureSource_Set(int iAdapterIndex, int iDisplayIndex, int iTempSource);

// @}

/// \addtogroup T_OVERRIDE
// @{

///
///\brief Function to retrieve display mode timing override information.
/// 
/// This function retrieves display mode timing override information for a specified display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]   lpModeIn The pointer to the ADLDisplayMode structure storing the desired display mode.
/// \param[out]  lpModeInfoOut The pointer to the ADLDisplayModeInfo structure storing the retrieved display mode information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
/// \deprecated This API has been deprecated. It has been replaced with API ADL2_Display_ModeTimingOverrideX2_Get
ADL_EXTERNC int EXPOSED ADL2_Display_ModeTimingOverride_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, ADLDisplayMode *lpModeIn, ADLDisplayModeInfo *lpModeInfoOut);

///
///\brief Function to retrieve display mode timing override information.
/// 
/// This function retrieves display mode timing override information for a specified display.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]   lpModeIn The pointer to the ADLDisplayMode structure storing the desired display mode.
/// \param[out]  lpModeInfoOut The pointer to the ADLDisplayModeInfo structure storing the retrieved display mode information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \deprecated This API has been deprecated. It has been replaced with API ADL_Display_ModeTimingOverrideX2_Get
ADL_EXTERNC int EXPOSED ADL_Display_ModeTimingOverride_Get(int iAdapterIndex, int iDisplayIndex, ADLDisplayMode *lpModeIn, ADLDisplayModeInfo *lpModeInfoOut);

///
///\brief Function to set display mode timing override information.
/// 
/// This function sets display mode timing override information for a specified display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]   lpMode The pointer to the ADLDisplayModeInfo structure which sets the display mode timing override information.
/// \param[in]   iForceUpdate The value which forces the update of the timing override information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_ModeTimingOverride_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, ADLDisplayModeInfo *lpMode, int iForceUpdate);

///
///\brief Function to set display mode timing override information.
/// 
/// This function sets display mode timing override information for a specified display.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]   lpMode The pointer to the ADLDisplayModeInfo structure which sets the display mode timing override information.
/// \param[in]   iForceUpdate The value which forces the update of the timing override information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
ADL_EXTERNC int EXPOSED ADL_Display_ModeTimingOverride_Set(int iAdapterIndex, int iDisplayIndex, ADLDisplayModeInfo *lpMode, int iForceUpdate);

///
///\brief Function to get the display mode timing override list.
/// 
/// This function retrieves display mode timing override list.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]   iMaxNumOfOverrides The maximum number of elements in the array pointed by lpModeInfoList. The number specified must be > 0.
/// \param[out]  lpModeInfoList The pointer to the retrieved ADLDisplayModeInfo structure. This pointer must be allocated by the user.
/// \param[out]  lpNumOfOverrides The pointer to the actual number of the retrieved overridden modes.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
/// \deprecated This API has been deprecated. It has been replaced with API ADL2_Display_ModeTimingOverrideListX2_Get
ADL_EXTERNC int EXPOSED ADL2_Display_ModeTimingOverrideList_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int iMaxNumOfOverrides, ADLDisplayModeInfo *lpModeInfoList, int *lpNumOfOverrides);

///
///\brief Function to get the display mode timing override list.
/// 
/// This function retrieves display mode timing override list.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]   iMaxNumOfOverrides The maximum number of elements in the array pointed by lpModeInfoList. The number specified must be > 0.
/// \param[out]  lpModeInfoList The pointer to the retrieved ADLDisplayModeInfo structure. This pointer must be allocated by the user.
/// \param[out]  lpNumOfOverrides The pointer to the actual number of the retrieved overridden modes.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \deprecated This API has been deprecated. It has been replaced with API ADL_Display_ModeTimingOverrideListX2_Get
ADL_EXTERNC int EXPOSED ADL_Display_ModeTimingOverrideList_Get(int iAdapterIndex, int iDisplayIndex, int iMaxNumOfOverrides, ADLDisplayModeInfo *lpModeInfoList, int *lpNumOfOverrides);
// @}

/// \addtogroup CUSTOMIZED
// @{

///
///\brief Function to retrieve the number of customized modes.
/// 
/// This function retrieves the number of customized modes for a specified display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpListNum The pointer to the number of available modes.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_CustomizedModeListNum_Get (ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int* lpListNum);

///
///\brief Function to retrieve the number of customized modes.
/// 
/// This function retrieves the number of customized modes for a specified display.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpListNum The pointer to the number of available modes.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_CustomizedModeListNum_Get (int iAdapterIndex, int iDisplayIndex, int* lpListNum);

///
///\brief Function to retrieve the customized mode list.
/// 
/// This function retrieves the customized mode list for a specified display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpCustomModeList The pointer to the list of the returned ADLCustomMode structures. The user should allocate the memory:  Number of modes * sizeof ( ADLCustomMode )
/// \param[in]  iBuffSize The size of the lpCustomModelist buffer. 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_CustomizedModeList_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex,  ADLCustomMode* lpCustomModeList, int iBuffSize);

///
///\brief Function to retrieve the customized mode list.
/// 
/// This function retrieves the customized mode list for a specified display.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpCustomModeList The pointer to the list of the returned ADLCustomMode structures. The user should allocate the memory:  Number of modes * sizeof ( ADLCustomMode )
/// \param[in]  iBuffSize The size of the lpCustomModelist buffer. 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_CustomizedModeList_Get(int iAdapterIndex, int iDisplayIndex,  ADLCustomMode* lpCustomModeList, int iBuffSize);

///
///\brief Function to add a customized mode.
/// 
/// This function adds a custom mode to the customized mode list.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  customMode The custom mode to be added to the list.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_CustomizedMode_Add (ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, ADLCustomMode customMode);

///
///\brief Function to add a customized mode.
/// 
/// This function adds a custom mode to the customized mode list.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  customMode The custom mode to be added to the list.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
ADL_EXTERNC int EXPOSED ADL_Display_CustomizedMode_Add (int iAdapterIndex, int iDisplayIndex, ADLCustomMode customMode);

///
///\brief Function to delete a customized mode.
/// 
/// This function deletes a custom mode from the customized mode list.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iIndex The index value of the mode to be deleted.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_CustomizedMode_Delete (ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int iIndex);

///
///\brief Function to delete a customized mode.
/// 
/// This function deletes a custom mode from the customized mode list.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iIndex The index value of the mode to be deleted.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
ADL_EXTERNC int EXPOSED ADL_Display_CustomizedMode_Delete (int iAdapterIndex, int iDisplayIndex, int iIndex);

///
///\brief Function to validate a customized mode.
/// 
/// This function validates a custom mode from the customized mode list.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  customMode The custom mode to be validated.
/// \param[out]	lpValid The pointer to the retrieved validation bit.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_CustomizedMode_Validate(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, ADLCustomMode customMode, int *lpValid);

///
///\brief Function to validate a customized mode.
/// 
/// This function validates a custom mode from the customized mode list.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  customMode The custom mode to be validated.
/// \param[out]	lpValid The pointer to the retrieved validation bit.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_CustomizedMode_Validate(int iAdapterIndex, int iDisplayIndex, ADLCustomMode customMode, int *lpValid);
// @}




/// \addtogroup UNDERSCAN
// @{

///
///\brief Function to get the value of under scan support.  
/// 
/// This function gets the display under scan support flag for a specified adapter, display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  *lpSupport pointer to the the under scan support value.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_UnderscanSupport_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int *lpSupport);
 
///
///\brief Function to get the value of under scan enabled.  
/// 
/// This function gets the display under scan enabled flag for a specified adapter, display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  *lpCurrent pointer to the the under scan enabled current value. Enabled :1 ; disabled:0
/// \param[in]  *lpDefault pointer to the the under scan enabled default value. Enabled :1 ; disabled:0
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_UnderscanState_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int *lpCurrent, int *lpDefault);

///
///\brief Function to set the value of under scan enabled.  
/// 
/// This function sets the display under scan enabled flag for a specified adapter, display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iUnderscanEnabled the the under scan enabled value. Enabled :1 ; disabled:0
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_UnderscanState_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int iUnderscanEnabled);

///
///\brief Function to set the current value of underscan.  
/// 
/// This function sets the display underscan for a specified adapter, display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iCurrent The desired underscan value.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_Underscan_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int iCurrent);

///
///\brief Function to set the current value of underscan.  
/// 
/// This function sets the display underscan for a specified adapter, display.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iCurrent The desired underscan value.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
ADL_EXTERNC int EXPOSED ADL_Display_Underscan_Set(int iAdapterIndex, int iDisplayIndex, int iCurrent);

///
///\brief Function to retrieve the detailed information for underscan.
/// 
/// This function retrieves the detailed information for underscan.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpCurrent The pointer to the current underscan value.
/// \param[out] lpDefault The pointer to the default underscan value.
/// \param[out] lpMin The pointer to the minimum underscan value.
/// \param[out] lpMax The pointer to the maximum underscan value.
/// \param[out] lpStep The pointer to the increment of the underscan setting.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_Underscan_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int* lpCurrent, int* lpDefault, int* lpMin, int* lpMax, int* lpStep);
  
///
///\brief Function to retrieve the detailed information for underscan.
/// 
/// This function retrieves the detailed information for underscan.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpCurrent The pointer to the current underscan value.
/// \param[out] lpDefault The pointer to the default underscan value.
/// \param[out] lpMin The pointer to the minimum underscan value.
/// \param[out] lpMax The pointer to the maximum underscan value.
/// \param[out] lpStep The pointer to the increment of the underscan setting.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Display_Underscan_Get(int iAdapterIndex, int iDisplayIndex, int* lpCurrent, int* lpDefault, int* lpMin, int* lpMax, int* lpStep);

///
///\brief Function to set the current value of gamma for each controller.
/// 
/// This function sets the Overscan value for a specified display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iCurrent The desired Overscan value.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_Overscan_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int iCurrent);

///
///\brief Function to set the current value of gamma for each controller.
/// 
/// This function sets the Overscan value for a specified display.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iCurrent The desired Overscan value.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
ADL_EXTERNC int EXPOSED ADL_Display_Overscan_Set(int iAdapterIndex, int iDisplayIndex, int iCurrent);





///
///\brief Function to retrieve the current value of gamma for each controller.
/// 
/// This function retrieves the Overscan value for a specified display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpCurrent The pointer to the current Overscan value.
/// \param[out]  lpDefualt The pointer to the default Overscan value.
/// \param[out]  lpMin The pointer to the minimum Overscan value.
/// \param[out]  lpMax The pointer to the maximum Overscan value.
/// \param[out]  lpStep The pointer to the increment of the Overscan value.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_Overscan_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int *lpCurrent, int *lpDefualt, int *lpMin, int *lpMax, int *lpStep);

///
///\brief Function to retrieve the current value of gamma for each controller.
/// 
/// This function retrieves the Overscan value for a specified display.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpCurrent The pointer to the current Overscan value.
/// \param[out]  lpDefualt The pointer to the default Overscan value.
/// \param[out]  lpMin The pointer to the minimum Overscan value.
/// \param[out]  lpMax The pointer to the maximum Overscan value.
/// \param[out]  lpStep The pointer to the increment of the Overscan value.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. 
ADL_EXTERNC int EXPOSED ADL_Display_Overscan_Get(int iAdapterIndex, int iDisplayIndex, int *lpCurrent, int *lpDefualt, int *lpMin, int *lpMax, int *lpStep);
// @}

/// \addtogroup DFP
// @{

///
///\brief Function to get the display base audio support. 
/// 
/// This function determines whether base display audio support is available.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpSupport The int pointer to the value indicating whether the feature is supported by the driver. 1: feature is supported; 0: feature is not supported.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_DFP_BaseAudioSupport_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex,int* lpSupport);

///
///\brief Function to get the display base audio support. 
/// 
/// This function determines whether base display audio support is available.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpSupport The int pointer to the value indicating whether the feature is supported by the driver. 1: feature is supported; 0: feature is not supported.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_DFP_BaseAudioSupport_Get(int iAdapterIndex, int iDisplayIndex,int* lpSupport);

///
///\brief Function to get the display HDMI support. 
/// 
/// This function determines whether the base display HDMI support is available. 
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpSupport The int pointer indicating whether the feature is supported by the driver. If the feature is supported, a value of 1 is stored. Otherwise, a value of 0 is stored.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_DFP_HDMISupport_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex,int* lpSupport);

///
///\brief Function to get the display HDMI support. 
/// 
/// This function determines whether the base display HDMI support is available. 
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpSupport The int pointer indicating whether the feature is supported by the driver. If the feature is supported, a value of 1 is stored. Otherwise, a value of 0 is stored.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_DFP_HDMISupport_Get(int iAdapterIndex, int iDisplayIndex,int* lpSupport);

///
///\brief Function to get the display MVPU analog support. 
/// 
/// This function determines whether display MVPU analog support is available.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpSupport The int pointer to the value indicating whether the feature is supported by the driver. 1: feature is supported; 0: feature is not supported.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_DFP_MVPUAnalogSupport_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex,int* lpSupport);

///
///\brief Function to get the display MVPU analog support. 
/// 
/// This function determines whether display MVPU analog support is available.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpSupport The int pointer to the value indicating whether the feature is supported by the driver. 1: feature is supported; 0: feature is not supported.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_DFP_MVPUAnalogSupport_Get(int iAdapterIndex, int iDisplayIndex,int* lpSupport);

///
///\brief Function to retrieve PixelFormat caps.
/// 
/// This function retrieves the pixel format caps for a specified display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpValidBits The pointer to the bit field indicating which bits in the lpValidCaps parameter are valid.
/// \param[out]  lpValidCaps The pointer to the valid pixel formats caps. \ref define_pixel_formats
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API is supported only by HDMI.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_DFP_PixelFormat_Caps(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int *lpValidBits, int *lpValidCaps);

///
///\brief Function to retrieve PixelFormat caps.
/// 
/// This function retrieves the pixel format caps for a specified display.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpValidBits The pointer to the bit field indicating which bits in the lpValidCaps parameter are valid.
/// \param[out]  lpValidCaps The pointer to the valid pixel formats caps. \ref define_pixel_formats
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API is supported only by HDMI.
ADL_EXTERNC int EXPOSED ADL_DFP_PixelFormat_Caps(int iAdapterIndex, int iDisplayIndex, int *lpValidBits, int *lpValidCaps);

///
///\brief Function to retrieve current pixel format setting.
/// 
/// This function retrieves the pixel format setting for a specified display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpCurState The pointer to the current pixel format of the display. \ref define_pixel_formats
/// \param[out]  lpDefault The pointer to the default pixel format of the display. \ref define_pixel_formats
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API is supported only by HDMI.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_DFP_PixelFormat_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int *lpCurState, int *lpDefault);

///
///\brief Function to retrieve current pixel format setting.
/// 
/// This function retrieves the pixel format setting for a specified display.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpCurState The pointer to the current pixel format of the display. \ref define_pixel_formats
/// \param[out]  lpDefault The pointer to the default pixel format of the display. \ref define_pixel_formats
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API is supported only by HDMI.
ADL_EXTERNC int EXPOSED ADL_DFP_PixelFormat_Get(int iAdapterIndex, int iDisplayIndex, int *lpCurState, int *lpDefault);

///
///\brief Function to set the current pixel format setting.
/// 
/// This function sets the pixel format setting for a specified display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iState The desired pixel format for the specified display.  \ref define_pixel_formats
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API is supported only by HDMI.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_DFP_PixelFormat_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int iState);

///
///\brief Function to set the current pixel format setting.
/// 
/// This function sets the pixel format setting for a specified display.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iState The desired pixel format for the specified display.  \ref define_pixel_formats
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API is supported only by HDMI.
ADL_EXTERNC int EXPOSED ADL_DFP_PixelFormat_Set(int iAdapterIndex, int iDisplayIndex, int iState);

///
///\brief Function to get the GPUScalingEnable setting.
/// 
/// This function sets the GPU Scaling Enable setting for a specified display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  lpSupport  The pointer to the value indicating whether GPUScalingEnable is supported. 1: supported; 0: not supported.
/// \param[in]  lpCurrent  The pointer to the value indicating whether GPUScalingEnable is currently enabled. 1: enabled; 0: disabled.
/// \param[in]  lpDefault  The pointer to the GPUScalingEnable default setting.
/// \return If the function succeeds, the return value is \ref ADL_OK, ADL_ERR_NULL_POINTER if any of the inputs is NULL, Otherwise the return value is an ADL_ERR.  \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_DFP_GPUScalingEnable_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int *lpSupport, int *lpCurrent, int *lpDefault);

///
///\brief Function to get the GPUScalingEnable setting.
/// 
/// This function sets the GPU Scaling Enable setting for a specified display.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  lpSupport  The pointer to the value indicating whether GPUScalingEnable is supported. 1: supported; 0: not supported.
/// \param[in]  lpCurrent  The pointer to the value indicating whether GPUScalingEnable is currently enabled. 1: enabled; 0: disabled.
/// \param[in]  lpDefault  The pointer to the GPUScalingEnable default setting.
/// \return If the function succeeds, the return value is \ref ADL_OK, ADL_ERR_NULL_POINTER if any of the inputs is NULL, Otherwise the return value is an ADL_ERR.  \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_DFP_GPUScalingEnable_Get(int iAdapterIndex, int iDisplayIndex, int *lpSupport, int *lpCurrent, int *lpDefault);

///
///\brief Function to set the GPUScalingEnable setting.
/// 
/// This function sets the GPU Scaling Enable setting for a specified display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iCurrent The desired GPU Scaling Enable state for the specified display. 1: enabled; 0: disabled..
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_DFP_GPUScalingEnable_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int iCurrent);

///
///\brief Function to set the GPUScalingEnable setting.
/// 
/// This function sets the GPU Scaling Enable setting for a specified display.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iCurrent The desired GPU Scaling Enable state for the specified display. 1: enabled; 0: disabled..
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
ADL_EXTERNC int EXPOSED ADL_DFP_GPUScalingEnable_Set(int iAdapterIndex, int iDisplayIndex, int iCurrent);

///
///\brief Function to get the Allow Only CE Timings setting.
/// 
/// This function sets the Allow Only CE Timings setting for a specified display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  lpSupport  The pointer to the value indicating whether AllowOnlyCETimings is supported. 1: supported; 0: not supported.
/// \param[in]  lpCurrent  The pointer to the value indicating whether AllowOnlyCETimings is currently enabled. 1: enabled; 0: disabled.
/// \param[in]  lpDefault  The pointer to the AllowOnlyCETimings default setting.
/// \return If the function succeeds, the return value is \ref ADL_OK, ADL_ERR_NULL_POINTER if any of the inputs is NULL, Otherwise the return value is an ADL_ERR.  \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_DFP_AllowOnlyCETimings_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int *lpSupport, int *lpCurrent, int *lpDefault);

///
///\brief Function to get the Allow Only CE Timings setting.
/// 
/// This function sets the Allow Only CE Timings setting for a specified display.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  lpSupport  The pointer to the value indicating whether AllowOnlyCETimings is supported. 1: supported; 0: not supported.
/// \param[in]  lpCurrent  The pointer to the value indicating whether AllowOnlyCETimings is currently enabled. 1: enabled; 0: disabled.
/// \param[in]  lpDefault  The pointer to the AllowOnlyCETimings default setting.
/// \return If the function succeeds, the return value is \ref ADL_OK, ADL_ERR_NULL_POINTER if any of the inputs is NULL, Otherwise the return value is an ADL_ERR.  \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_DFP_AllowOnlyCETimings_Get(int iAdapterIndex, int iDisplayIndex, int *lpSupport, int *lpCurrent, int *lpDefault);

///
///\brief Function to set the Allow Only CE Timings setting.
/// 
/// This function sets the Allow Only CE Timings setting for a specified display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iCurrent The desired Allow Only CE Timings state for the specified display. 1: enabled; 0: disabled..
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_DFP_AllowOnlyCETimings_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int iCurrent);

///
///\brief Function to set the Allow Only CE Timings setting.
/// 
/// This function sets the Allow Only CE Timings setting for a specified display.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iCurrent The desired Allow Only CE Timings state for the specified display. 1: enabled; 0: disabled..
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
ADL_EXTERNC int EXPOSED ADL_DFP_AllowOnlyCETimings_Set(int iAdapterIndex, int iDisplayIndex, int iCurrent);

// @}


/// \addtogroup TV
// @{

///
///\brief Function to retrieve the TV Caps display information. 
/// 
/// This function retrieves the display adjustment information.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpcaps The pointer to the TV Caps information retrieved from the driver. \ref define_tv_caps
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \deprecated Dropping support for component, composite, and S-Video connectors.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_TVCaps_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int* lpcaps);

///
///\brief Function to retrieve the TV Caps display information. 
/// 
/// This function retrieves the display adjustment information.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpcaps The pointer to the TV Caps information retrieved from the driver. \ref define_tv_caps
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \deprecated Dropping support for component, composite, and S-Video connectors.
ADL_EXTERNC int EXPOSED ADL_Display_TVCaps_Get(int iAdapterIndex, int iDisplayIndex, int* lpcaps);

///
///\brief Function to set the TV standard.
/// 
/// This function sets the TV Standard for a specified display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iCurrent The desired TV Standard. \ref define_tv_caps
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \deprecated Dropping support for component, composite, and S-Video connectors.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_TV_Standard_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int iCurrent);

///
///\brief Function to set the TV standard.
/// 
/// This function sets the TV Standard for a specified display.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iCurrent The desired TV Standard. \ref define_tv_caps
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \deprecated Dropping support for component, composite, and S-Video connectors.
ADL_EXTERNC int EXPOSED ADL_TV_Standard_Set(int iAdapterIndex, int iDisplayIndex, int iCurrent);

///
///\brief Function to retrieve the TV standard.
/// 
/// This function retrieves the TV standard for a specified display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpCurrent The pointer to the current TV standard. \ref define_tv_caps
/// \param[out] lpDefault The pointer to the default TV standard. \ref define_tv_caps
/// \param[out] lpSupportedStandards The pointer to the bit vector of the support video standard.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \deprecated Dropping support for component, composite, and S-Video connectors.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_TV_Standard_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int *lpCurrent, int *lpDefault, int* lpSupportedStandards);

///
///\brief Function to retrieve the TV standard.
/// 
/// This function retrieves the TV standard for a specified display.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpCurrent The pointer to the current TV standard. \ref define_tv_caps
/// \param[out] lpDefault The pointer to the default TV standard. \ref define_tv_caps
/// \param[out] lpSupportedStandards The pointer to the bit vector of the support video standard.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \deprecated Dropping support for component, composite, and S-Video connectors.
ADL_EXTERNC int EXPOSED ADL_TV_Standard_Get(int iAdapterIndex, int iDisplayIndex, int *lpCurrent, int *lpDefault, int* lpSupportedStandards);

// @}

/// \addtogroup CV
// @{

///
///\brief Function to retrieve the settings of the CV dongle.
/// 
/// This function retrieves the CV dongle settings.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpDongleSetting The pointer to the CV dongle settings.
/// \param[out] lpOverrideSettingsSupported The pointer to the supported override settings.
/// \param[out] lpCurOverrideSettings The pointer to the current CV override settings.\n
/// \ref define_cv_dongle
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \deprecated Dropping support for component, composite, and S-Video connectors.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_CV_DongleSettings_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int* lpDongleSetting, int* lpOverrideSettingsSupported, int* lpCurOverrideSettings);

///
///\brief Function to retrieve the settings of the CV dongle.
/// 
/// This function retrieves the CV dongle settings.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpDongleSetting The pointer to the CV dongle settings.
/// \param[out] lpOverrideSettingsSupported The pointer to the supported override settings.
/// \param[out] lpCurOverrideSettings The pointer to the current CV override settings.\n
/// \ref define_cv_dongle
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \deprecated Dropping support for component, composite, and S-Video connectors.
ADL_EXTERNC int EXPOSED ADL_CV_DongleSettings_Get(int iAdapterIndex, int iDisplayIndex, int* lpDongleSetting, int* lpOverrideSettingsSupported, int* lpCurOverrideSettings);

///
///\brief Function to set the current CV dongle settings.
/// 
/// This function sets the current CV dongle settings to a specified setting.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in] iOverrideSettings The new CV settings.  \ref define_cv_dongle
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \deprecated Dropping support for component, composite, and S-Video connectors.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_CV_DongleSettings_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int iOverrideSettings);

///
///\brief Function to set the current CV dongle settings.
/// 
/// This function sets the current CV dongle settings to a specified setting.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in] iOverrideSettings The new CV settings.  \ref define_cv_dongle
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \deprecated Dropping support for component, composite, and S-Video connectors.
ADL_EXTERNC int EXPOSED ADL_CV_DongleSettings_Set(int iAdapterIndex, int iDisplayIndex, int iOverrideSettings);

///
/// \brief Function to reset the CV settings to its default settings.
/// 
/// This function resets the CV dongle settings.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \deprecated Dropping support for component, composite, and S-Video connectors.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_CV_DongleSettings_Reset(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex);

///
/// \brief Function to reset the CV settings to its default settings.
/// 
/// This function resets the CV dongle settings.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \deprecated Dropping support for component, composite, and S-Video connectors.
ADL_EXTERNC int EXPOSED ADL_CV_DongleSettings_Reset(int iAdapterIndex, int iDisplayIndex);

// @}

///\brief Function to get the current UnderScan Auto setting from the display. 
/// This function retrieves the UnderScan Auto information for a specified display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The index of the desired adapter.
/// \param[in]  iDisplayIndex The index of the desired display.
/// \param[out] lpCurrent The current underscan auto setting
/// \param[out] lpDefault The default underscan auto setting
/// \param[out] lpMin The current underscan auto setting
/// \param[out] lpMax The default underscan auto setting
/// \param[out] lpStep The current underscan auto setting
/// \return If the function succeeds, the return value is ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_UnderScan_Auto_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int *lpCurrent, int *lpDefault, int *lpMin, int *lpMax, int *lpStep);

///\brief Function to get the current UnderScan Auto setting from the display. 
/// This function retrieves the UnderScan Auto information for a specified display.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The index of the desired adapter.
/// \param[in]  iDisplayIndex The index of the desired display.
/// \param[out] lpCurrent The current underscan auto setting
/// \param[out] lpDefault The default underscan auto setting
/// \param[out] lpMin The current underscan auto setting
/// \param[out] lpMax The default underscan auto setting
/// \param[out] lpStep The current underscan auto setting
/// \return If the function succeeds, the return value is ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Display_UnderScan_Auto_Get(int iAdapterIndex, int iDisplayIndex, int *lpCurrent, int *lpDefault, int *lpMin, int *lpMax, int *lpStep);

///\brief Function to set the current UnderScan Auto setting for the display. 
/// This function set the UnderScan Auto setting for a specified display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The index of the desired adapter.
/// \param[in]  iDisplayIndex The index of the desired display.
/// \param[in]  iCurrent The new underscan auto setting
/// \return If the function succeeds, the return value is ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results 
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_UnderScan_Auto_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int iCurrent);

///\brief Function to set the current UnderScan Auto setting for the display. 
/// This function set the UnderScan Auto setting for a specified display.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The index of the desired adapter.
/// \param[in]  iDisplayIndex The index of the desired display.
/// \param[in]  iCurrent The new underscan auto setting
/// \return If the function succeeds, the return value is ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results 
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
ADL_EXTERNC int EXPOSED ADL_Display_UnderScan_Auto_Set(int iAdapterIndex, int iDisplayIndex, int iCurrent);

///\brief Function to get the current Deflicker setting from the display. 
/// This function retrieves the Deflicker information for a specified display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The index of the desired adapter.
/// \param[in]  iDisplayIndex The index of the desired display.
/// \param[out] lpCurrent The current Deflicker setting
/// \param[out] lpDefault The default Deflicker setting
/// \param[out] lpMin The current Deflicker setting
/// \param[out] lpMax The default Deflicker setting
/// \param[out] lpStep The current Deflicker setting 
/// \return If the function succeeds, the return value is ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_Deflicker_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int *lpCurrent, int *lpDefault, int *lpMin, int *lpMax, int *lpStep);

///\brief Function to get the current Deflicker setting from the display. 
/// This function retrieves the Deflicker information for a specified display.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The index of the desired adapter.
/// \param[in]  iDisplayIndex The index of the desired display.
/// \param[out] lpCurrent The current Deflicker setting
/// \param[out] lpDefault The default Deflicker setting
/// \param[out] lpMin The current Deflicker setting
/// \param[out] lpMax The default Deflicker setting
/// \param[out] lpStep The current Deflicker setting 
/// \return If the function succeeds, the return value is ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Display_Deflicker_Get(int iAdapterIndex, int iDisplayIndex, int *lpCurrent, int *lpDefault, int *lpMin, int *lpMax, int *lpStep);

///\brief Function to set the current Deflicker setting for the display. 
/// This function set the Deflicker setting for a specified display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The index of the desired adapter.
/// \param[in]  iDisplayIndex The index of the desired display.
/// \param[in]  iCurrent The new Deflicker setting
/// \return If the function succeeds, the return value is ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results 
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_Deflicker_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayindex, int iCurrent);

///\brief Function to set the current Deflicker setting for the display. 
/// This function set the Deflicker setting for a specified display.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The index of the desired adapter.
/// \param[in]  iDisplayIndex The index of the desired display.
/// \param[in]  iCurrent The new Deflicker setting
/// \return If the function succeeds, the return value is ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results 
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
ADL_EXTERNC int EXPOSED ADL_Display_Deflicker_Set(int iAdapterIndex, int iDisplayindex, int iCurrent);

///\brief Function to get the current FilterSVideo setting from the display. 
/// This function retrieves the S-Video Sharpness Control information for a specified display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The index of the desired adapter.
/// \param[in]  iDisplayIndex The index of the desired display.
/// \param[out] lpCurrent The current FilterSVideo setting
/// \param[out] lpDefault The default FilterSVideo setting
/// \param[out] lpMin The current FilterSVideo setting
/// \param[out] lpMax The default FilterSVideo setting
/// \param[out] lpStep The current FilterSVideo setting
/// \return If the function succeeds, the return value is ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_FilterSVideo_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int *lpCurrent, int *lpDefault, int *lpMin, int *lpMax, int *lpStep);

///\brief Function to get the current FilterSVideo setting from the display. 
/// This function retrieves the S-Video Sharpness Control information for a specified display.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The index of the desired adapter.
/// \param[in]  iDisplayIndex The index of the desired display.
/// \param[out] lpCurrent The current FilterSVideo setting
/// \param[out] lpDefault The default FilterSVideo setting
/// \param[out] lpMin The current FilterSVideo setting
/// \param[out] lpMax The default FilterSVideo setting
/// \param[out] lpStep The current FilterSVideo setting
/// \return If the function succeeds, the return value is ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Display_FilterSVideo_Get(int iAdapterIndex, int iDisplayIndex, int *lpCurrent, int *lpDefault, int *lpMin, int *lpMax, int *lpStep);

///\brief Function to set the current FilterSVideo setting for the display. 
/// This function set the S-Video Sharpness Control setting for a specified display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The index of the desired adapter.
/// \param[in]  iDisplayIndex The index of the desired display.
/// \param[in]  iCurrent The new FilterSVideo setting
/// \return If the function succeeds, the return value is ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results 
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_FilterSVideo_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int iCurrent);

///\brief Function to set the current FilterSVideo setting for the display. 
/// This function set the S-Video Sharpness Control setting for a specified display.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The index of the desired adapter.
/// \param[in]  iDisplayIndex The index of the desired display.
/// \param[in]  iCurrent The new FilterSVideo setting
/// \return If the function succeeds, the return value is ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results 
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
ADL_EXTERNC int EXPOSED ADL_Display_FilterSVideo_Set(int iAdapterIndex, int iDisplayIndex, int iCurrent);

///\brief
/// This function sets the picture setting (Graphics, Photo, Cinema or Gaming) on any HDMI that supports these modes.
/// The application associated with this function is designed such that, the ITC display option must be toggled 'ON' (checked) 
/// before display content options become available for setting.  If the display content is set, but the ITC is toggled 'OFF', 
/// the display content options will disable (gray-out), but still show which display content was last 'set'
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]	iAdapterIndex The index of the desired adapter.
/// \param[in]	iDisplayIndex The index of the desired display.
/// \param[in]	iContent The display content desired to be set; where iContent is one of ADLDisplayContent.iContentType
/// \return If the function succeeds, the return value is ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results 
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_DisplayContent_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int iContent);

///\brief
/// This function sets the picture setting (Graphics, Photo, Cinema or Gaming) on any HDMI that supports these modes.
/// The application associated with this function is designed such that, the ITC display option must be toggled 'ON' (checked) 
/// before display content options become available for setting.  If the display content is set, but the ITC is toggled 'OFF', 
/// the display content options will disable (gray-out), but still show which display content was last 'set'
/// \platform
/// \ALL
/// \param[in]	iAdapterIndex The index of the desired adapter.
/// \param[in]	iDisplayIndex The index of the desired display.
/// \param[in]	iContent The display content desired to be set; where iContent is one of ADLDisplayContent.iContentType
/// \return If the function succeeds, the return value is ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results 
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
ADL_EXTERNC int EXPOSED ADL_Display_DisplayContent_Set(int iAdapterIndex, int iDisplayIndex, int iContent);

///\brief
/// This function gets the picture setting (Graphics, Photo, Cinema or Gaming) on any HDMI that supports these modes.
/// The application associated with this function is designed such that, even if the ITC display option is toggled 'OFF' (unchecked), 
/// this function will still return the last display content mode that was set (or initial value of Graphics)
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]	iAdapterIndex The index of the desired adapter.
/// \param[in]	iDisplayIndex The index of the desired display.
/// \param[in]	piContent The current display content setting; where piContent is one of ADLDisplayContent.iContentType
/// \return If the function succeeds, the return value is ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_DisplayContent_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex,int iDisplayIndex,int* piContent);

///\brief
/// This function gets the picture setting (Graphics, Photo, Cinema or Gaming) on any HDMI that supports these modes.
/// The application associated with this function is designed such that, even if the ITC display option is toggled 'OFF' (unchecked), 
/// this function will still return the last display content mode that was set (or initial value of Graphics)
/// \platform
/// \ALL
/// \param[in]	iAdapterIndex The index of the desired adapter.
/// \param[in]	iDisplayIndex The index of the desired display.
/// \param[in]	piContent The current display content setting; where piContent is one of ADLDisplayContent.iContentType
/// \return If the function succeeds, the return value is ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Display_DisplayContent_Get(int iAdapterIndex,int iDisplayIndex,int* piContent);

///\brief 
/// This function gets the application availability for display content value  and ITC flag.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]	iAdapterIndex The index of the desired adapter.
/// \param[in]	iDisplayIndex The index of the desired display.
/// \param[in]	pCapContent The current cap setting(if display content and ITC data are retrievable); 
/// 				 where pCapContent returns the bitOR value of all supported ADLDisplayContent.iContentType (i.e. according to definitions of ADLDisplayContent.iContentType, 
/// 				 if all content is supported it is expected that pCapContent will return with a value of, Graphics | Photo | Cinema |Game = 15)
/// \return If the function succeeds, the return value is ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results 
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_DisplayContent_Cap(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int* pCapContent);

///\brief 
/// This function gets the application availability for display content value  and ITC flag.
/// \platform
/// \ALL
/// \param[in]	iAdapterIndex The index of the desired adapter.
/// \param[in]	iDisplayIndex The index of the desired display.
/// \param[in]	pCapContent The current cap setting(if display content and ITC data are retrievable); 
/// 				 where pCapContent returns the bitOR value of all supported ADLDisplayContent.iContentType (i.e. according to definitions of ADLDisplayContent.iContentType, 
/// 				 if all content is supported it is expected that pCapContent will return with a value of, Graphics | Photo | Cinema |Game = 15)
/// \return If the function succeeds, the return value is ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results 
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
ADL_EXTERNC int EXPOSED ADL_Display_DisplayContent_Cap(int iAdapterIndex, int iDisplayIndex, int* pCapContent);

///
///\brief Function to retrieve Timing Override support.
/// 
/// This function retrieves Timing Override support for a specified display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpSupported If the specified adapter supports Custom Resolutions then returns ADL_TRUE else ADL_FALSE
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_ModeTimingOverride_Caps(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int *lpSupported);

///
///\brief Function to retrieve Timing Override support.
/// 
/// This function retrieves Timing Override support for a specified display.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpSupported If the specified adapter supports Custom Resolutions then returns ADL_TRUE else ADL_FALSE
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Adapter_ModeTimingOverride_Caps(int iAdapterIndex, int *lpSupported);

///
///\brief Function to retrieve current display mode timing override information.
/// 
/// This function retrieves current display mode timing override information for a specified display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   displayID The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpModeInfoOut The pointer to the ADLDisplayModeInfo structure storing the retrieved display mode information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_TargetTiming_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLDisplayID displayID, ADLDisplayModeInfo *lpModeInfoOut);

///
///\brief Function to retrieve current display mode timing override information.
/// 
/// This function retrieves current display mode timing override information for a specified display.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   displayID The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpModeInfoOut The pointer to the ADLDisplayModeInfo structure storing the retrieved display mode information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_TargetTiming_Get(int iAdapterIndex, ADLDisplayID displayID, ADLDisplayModeInfo *lpModeInfoOut);

///
///\brief Function to retrieve display mode timing override information.
/// 
/// This function retrieves display mode timing override information for a specified display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   displayID The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]   lpModeIn The pointer to the ADLDisplayMode structure storing the desired display mode.
/// \param[out]  lpModeInfoOut The pointer to the ADLDisplayModeInfo structure storing the retrieved display mode information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_ModeTimingOverrideX2_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLDisplayID displayID, ADLDisplayModeX2 *lpModeIn, ADLDisplayModeInfo *lpModeInfoOut);

///
///\brief Function to retrieve display mode timing override information.
/// 
/// This function retrieves display mode timing override information for a specified display.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   displayID The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]   lpModeIn The pointer to the ADLDisplayMode structure storing the desired display mode.
/// \param[out]  lpModeInfoOut The pointer to the ADLDisplayModeInfo structure storing the retrieved display mode information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_ModeTimingOverrideX2_Get(int iAdapterIndex, ADLDisplayID displayID, ADLDisplayModeX2 *lpModeIn, ADLDisplayModeInfo *lpModeInfoOut);

///
///\brief Function to get the display mode timing override list.
/// 
/// This function retrieves display mode timing override list.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   displayID The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpNumOfModes The pointer to the actual number of the retrieved overridden modes.
/// \param[out]  lpModeInfoList The pointer to the retrieved ADLDisplayModeInfo structure. This pointer must be allocated by the user.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_ModeTimingOverrideListX2_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLDisplayID displayID,int *lpNumOfModes, ADLDisplayModeInfo **lpModeInfoList);

///
///\brief Function to get the display mode timing override list.
/// 
/// This function retrieves display mode timing override list.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   displayID The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpNumOfModes The pointer to the actual number of the retrieved overridden modes.
/// \param[out]  lpModeInfoList The pointer to the retrieved ADLDisplayModeInfo structure. This pointer must be allocated by the user.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_ModeTimingOverrideListX2_Get(int iAdapterIndex, ADLDisplayID displayID,int *lpNumOfModes, ADLDisplayModeInfo **lpModeInfoList);

///
///\brief Function to delete display mode timing override information.
/// 
/// This function delete display mode timing override information for a specified display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   displayID The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]   lpModeIn The pointer to the ADLDisplayMode structure storing the desired display mode.
/// \param[in]   iForceUpdate The value which forces the update of the timing override information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_ModeTimingOverride_Delete(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLDisplayID displayID, ADLDisplayModeX2 *lpMode, int iForceUpdate);

///
///\brief Function to delete display mode timing override information.
/// 
/// This function delete display mode timing override information for a specified display.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   displayID The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]   lpModeIn The pointer to the ADLDisplayMode structure storing the desired display mode.
/// \param[in]   iForceUpdate The value which forces the update of the timing override information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_ModeTimingOverride_Delete(int iAdapterIndex, ADLDisplayID displayID, ADLDisplayModeX2 *lpMode, int iForceUpdate);

///
///\brief Function to get the Down-scaling Caps display information. 
/// 
/// This function retrieves the Down-scaling Caps information for a specified adapter and display.
/// \platform
/// \Win7 and above
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayID The desired display ID. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpCaps The pointer to the Down-scaling Caps information retrieved from the driver.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Display_Downscaling_Caps(int iAdapterIndex, int iDisplayID, int* lpCaps);


///
///\brief Function to get the Down-scaling Caps display information. 
/// 
/// This function retrieves the Down-scaling Caps information for a specified adapter and display.
/// \platform
/// \Win7 and above
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayID The desired display ID. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpCaps The pointer to the Down-scaling Caps information retrieved from the driver.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL2_Display_Downscaling_Caps(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayID, int* lpCaps);


///
///\brief Function to get the current state and capability of the FreeSync feature.  
/// 
/// This function gets the current and default FreeSync settings for a specified display along with the supported FreeSync refresh rate range.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpCurrent  The pointer to the FreeSync setting currently applied. It is a bit vector. \ref define_freesync_usecase
/// \param[out] lpDefault  The pointer to the default FreeSync setting. It is a bit vector. \ref define_freesync_usecase
/// \param[out] lpMinRefreshRateInMicroHz  The pointer to the minimum refresh rate in the range supported by FreeSync in microhertz.
/// \param[out] lpMaxRefreshRateInMicroHz  The pointer to the maximum refresh rate in the range supported by FreeSync in microhertz.
/// \return If the function succeeds, the return value is \ref ADL_OK, ADL_ERR_NULL_POINTER if any of the inputs is NULL, Otherwise the return value is an ADL_ERR.  \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL_Display_FreeSyncState_Get(int iAdapterIndex, int iDisplayIndex, int *lpCurrent, int *lpDefault, int *lpMinRefreshRateInMicroHz, int *lpMaxRefreshRateInMicroHz);

///
///\brief Function to get the current state and capability of the FreeSync feature.  
/// 
/// This function gets the current and default FreeSync settings for a specified display along with the supported FreeSync refresh rate range.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpCurrent  The pointer to the FreeSync setting currently applied. It is a bit vector. \ref define_freesync_usecase
/// \param[out] lpDefault  The pointer to the default FreeSync setting. It is a bit vector. \ref define_freesync_usecase
/// \param[out] lpMinRefreshRateInMicroHz  The pointer to the minimum refresh rate in the range supported by FreeSync in microhertz.
/// \param[out] lpMaxRefreshRateInMicroHz  The pointer to the maximum refresh rate in the range supported by FreeSync in microhertz.
/// \return If the function succeeds, the return value is \ref ADL_OK, ADL_ERR_NULL_POINTER if any of the inputs is NULL, Otherwise the return value is an ADL_ERR.  \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_FreeSyncState_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayIndex, int *lpCurrent, int *lpDefault, int *lpMinRefreshRateInMicroHz, int *lpMaxRefreshRateInMicroHz);

///
///\brief Function to set the current state of the FreeSync feature.  
/// 
/// This function sets the FreeSync feature enable state for a specified display. There is a second input parameter that may be used to request a specific refresh rate to be applied.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iSetting The desired FreeSync setting to be applied for the specified display. \ref define_freesync_usecase
/// \param[in]  iRefreshRateInMicroHz If non-zero value is specified, this indicates a request to set a specific refresh rate in microhertz. Otherwise, driver default will decide FreeSync refresh rate dynamically.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL_Display_FreeSyncState_Set(int iAdapterIndex, int iDisplayIndex, int iSetting, int iRefreshRateInMicroHz);

///
///\brief Function to set the current state of the FreeSync feature.  
/// 
/// This function sets the FreeSync feature enable state for a specified display. There is a second input parameter that may be used to request a specific refresh rate to be applied.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[in]  iSetting The desired FreeSync setting to be applied for the specified display. \ref define_freesync_usecase
/// \param[in]  iRefreshRateInMicroHz If non-zero value is specified, this indicates a request to set a specific refresh rate in microhertz. Otherwise, driver default will decide FreeSync refresh rate dynamically.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_FreeSyncState_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int iSetting, int iRefreshRateInMicroHz);

///
///\brief Function to retrieve per display Display Connectivity Experience information. 
/// 
/// This function sets the Display Connectivity Experience settings of a display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpFreeSyncCaps The pointer to the ADLDceSettings structure
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_DCE_Set(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayIndex, ADLDceSettings *lpADLDceSettings);

///
///\brief Function to retrieve per display Display Connectivity Experience information. 
/// 
/// This function sets the Display Connectivity Experience settings of a display.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpFreeSyncCaps The pointer to the ADLDceSettings structure
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Display_DCE_Set(int iAdapterIndex, int iDisplayIndex, ADLDceSettings *lpADLDceSettings);

///
///\brief Function to retrieve per display Display Connectivity Experience information. 
/// 
/// This function retrieves the Display Connectivity Experience settings of a display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpFreeSyncCaps The pointer to the ADLDceSettings structure
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_DCE_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayIndex, ADLDceSettings *lpADLDceSettings);

///
///\brief Function to retrieve per display Display Connectivity Experience information. 
/// 
/// This function retrieves the Display Connectivity Experience settings of a display.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpFreeSyncCaps The pointer to the ADLDceSettings structure
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Display_DCE_Get(int iAdapterIndex, int iDisplayIndex, ADLDceSettings *lpADLDceSettings);


///
///\brief Function to retrieve per display FreeSync capability information. 
/// 
/// This function retrieves the FreeSync capabilities of a display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpFreeSyncCaps The pointer to the ADLFreeSyncCap structure storing the retrieved FreeSync capabilities for the requested display index.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_FreeSync_Cap(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayIndex, ADLFreeSyncCap *lpFreeSyncCaps);

///
///\brief Function to retrieve per display FreeSync capability information. 
/// 
/// This function retrieves the FreeSync capabilities of a display.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpFreeSyncCaps The pointer to the ADLFreeSyncCap structure storing the retrieved FreeSync capabilities for the requested display index.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Display_FreeSync_Cap(int iAdapterIndex, int iDisplayIndex, ADLFreeSyncCap *lpFreeSyncCaps);

///
///\brief Function to retrieve per display Display Connectivity Experience information. 
/// 
/// This function sets the Display Connectivity Experience settings of a display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpFreeSyncCaps The pointer to the ADLDceSettings structure
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_DCE_Set(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayIndex, ADLDceSettings *lpADLDceSettings);

///
///\brief Function to retrieve per display Display Connectivity Experience information. 
/// 
/// This function sets the Display Connectivity Experience settings of a display.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpFreeSyncCaps The pointer to the ADLDceSettings structure
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Display_DCE_Set(int iAdapterIndex, int iDisplayIndex, ADLDceSettings *lpADLDceSettings);

///
///\brief Function to retrieve per display Display Connectivity Experience information. 
/// 
/// This function retrieves the Display Connectivity Experience settings of a display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpFreeSyncCaps The pointer to the ADLDceSettings structure
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_DCE_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayIndex, ADLDceSettings *lpADLDceSettings);

///
///\brief Function to retrieve per display Display Connectivity Experience information. 
/// 
/// This function retrieves the Display Connectivity Experience settings of a display.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpFreeSyncCaps The pointer to the ADLDceSettings structure
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Display_DCE_Get(int iAdapterIndex, int iDisplayIndex, ADLDceSettings *lpADLDceSettings);

///
/// \brief Function to set the current EDS mode enumeration mode.
/// 
/// This function sets the current EDS mode enumeration mode: Safe or Unsafe(raw modes).
/// \platform
/// \ALL
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in] unsafeMode The EDS mode enumeration mode to set (0: Safe mode, 1: Unsafe mode).
/// \return Succeed: \ref ADL_OK. Otherwise: ADL error code \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_CDS_UnsafeMode_Set(int iAdapterIndex, int unsafeMode);

///
/// \brief Function to set the current EDS mode enumeration mode.
/// 
/// This function sets the current EDS mode enumeration mode: Safe or Unsafe(raw modes).
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in] unsafeMode The EDS mode enumeration mode to set (0: Safe mode, 1: Unsafe mode).
/// \return Succeed: \ref ADL_OK. Otherwise: ADL error code \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL2_CDS_UnsafeMode_Set(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int unsafeMode);


#endif /* DISPLAY_H_ */
