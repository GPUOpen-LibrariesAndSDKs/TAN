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

/// \file displaysmanager.h
/// \brief Contains all functions related to Display Manager that are exposed by ADL for \ALL platforms.
///
/// This file contains all Displays Manager functions exposed by ADL for \ALL platforms.
/// All functions found in this file can be used as a reference to ensure
/// the specified function pointers can be used by the appropriate runtime
/// dynamic library loaders.

#ifndef DISPLAYSMANAGER_H_
#define DISPLAYSMANAGER_H_

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

/// \addtogroup DISPLAYMNGR
// @{


///
/// \brief Function to set the current extended desktop mode status for a display.
/// 
/// This function enables or disables extended desktop mode for a specified display. 
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in] iStatus The desired display extended desktop mode status. \ref ADL_TRUE : extended desktop mode; \ref ADL_FALSE : otherwise.
/// \param[out] lpNewlyActivate The pointer to the retrieved adapter activation information. \ref ADL_TRUE : the adapter is activated for the first time; \ref ADL_FALSE : otherwise.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_Active_Set (ADL_CONTEXT_HANDLE context,int iAdapterIndex, 
                                                 int iStatus, 
                                                 int* lpNewlyActivate);

///
/// \brief Function to set the current extended desktop mode status for a display.
/// 
/// This function enables or disables extended desktop mode for a specified display. 
/// \platform
/// \WIN
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in] iStatus The desired display extended desktop mode status. \ref ADL_TRUE : extended desktop mode; \ref ADL_FALSE : otherwise.
/// \param[out] lpNewlyActivate The pointer to the retrieved adapter activation information. \ref ADL_TRUE : the adapter is activated for the first time; \ref ADL_FALSE : otherwise.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Adapter_Active_Set (int iAdapterIndex, 
                                                int iStatus, 
                                                int* lpNewlyActivate);

///
/// \brief Function to set the current extended desktop mode status for the display.
/// 
/// This function enables or disables the input adapter for a specified display. 
/// In addition, this function allows the user to enable the input adapter and pass in the preferred targets.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in] iStatus The desired display extended desktop mode status. \ref ADL_TRUE : extended desktop mode; \ref ADL_FALSE : otherwise.
/// \param[in] iNumPreferTarget If iStatus is TRUE, the user can pass the size of preferred Display Targets array.
/// \param[in] lpPreferTarget If iStatus is TRUE, the user can pass the preferred Display Targets array to be enabled (with targets cloned).
/// \param[out] lpNewlyActivate The pointer to the retrieved adapter activation information. \ref ADL_TRUE : the adapter is activated for the first time; \ref ADL_FALSE : otherwise.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_Active_SetPrefer (ADL_CONTEXT_HANDLE context,int iAdapterIndex, 
                                                       int iStatus, 
                                                       int iNumPreferTarget,
                                                       ADLDisplayTarget* lpPreferTarget,
                                                       int* lpNewlyActivate);

///
/// \brief Function to set the current extended desktop mode status for the display.
/// 
/// This function enables or disables the input adapter for a specified display. 
/// In addition, this function allows the user to enable the input adapter and pass in the preferred targets.
/// \platform
/// \WIN
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in] iStatus The desired display extended desktop mode status. \ref ADL_TRUE : extended desktop mode; \ref ADL_FALSE : otherwise.
/// \param[in] iNumPreferTarget If iStatus is TRUE, the user can pass the size of preferred Display Targets array.
/// \param[in] lpPreferTarget If iStatus is TRUE, the user can pass the preferred Display Targets array to be enabled (with targets cloned).
/// \param[out] lpNewlyActivate The pointer to the retrieved adapter activation information. \ref ADL_TRUE : the adapter is activated for the first time; \ref ADL_FALSE : otherwise.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Adapter_Active_SetPrefer (int iAdapterIndex, 
                                                      int iStatus, 
                                                      int iNumPreferTarget,
                                                      ADLDisplayTarget* lpPreferTarget,
                                                      int* lpNewlyActivate);

///
/// \brief Function to retrieve the primary display adapter index.
/// 
/// This function retrieves the adapter index for the primary display adapter. 
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[out] lpPrimaryAdapterIndex The pointer to the ADL index handle of the primary adapter. 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_Primary_Get (ADL_CONTEXT_HANDLE context,int* lpPrimaryAdapterIndex);

///
/// \brief Function to retrieve the primary display adapter index.
/// 
/// This function retrieves the adapter index for the primary display adapter. 
/// \platform
/// \WIN
/// \param[out] lpPrimaryAdapterIndex The pointer to the ADL index handle of the primary adapter. 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Adapter_Primary_Get (int* lpPrimaryAdapterIndex);

///
/// \brief Function to set the primary display adapter index.
/// 
/// This function sets the adapter index for a specified primary display adapter. 
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_Primary_Set (ADL_CONTEXT_HANDLE context,int iAdapterIndex);

///
/// \brief Function to set the primary display adapter index.
/// 
/// This function sets the adapter index for a specified primary display adapter. 
/// \platform
/// \WIN
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Adapter_Primary_Set (int iAdapterIndex);

///
/// \brief Function to perform a mode switch for an adapter.
/// 
/// This function performs a mode switch for a specified adapter.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_ModeSwitch (ADL_CONTEXT_HANDLE context, int iAdapterIndex);

///
/// \brief Function to perform a mode switch for an adapter.
/// 
/// This function performs a mode switch for a specified adapter.
/// \platform
/// \WIN
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Adapter_ModeSwitch ( int iAdapterIndex);

///
/// \brief Function to retrieve the display mode information.
/// 
/// This function retrieves the current display mode information. 
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter. A value of -1 retrieves all modes for the system across multiple GPUs.
/// \param[in] iDisplayIndex The desired display index. If the index is -1, this field is ignored.
/// \param[out] lpNumModes The pointer to the number of modes retrieved.
/// \param[out] lppModes The pointer to the pointer to the retrieved display modes. Refer to the Display ADLMode structure for more information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
///	\remarks On Linux platform iXPos,iYPos,iColourDepth,iOrientation & iModeFlag are not supported. iXPos and iYPos can be retrieved from xrandr.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_Modes_Get (ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int* lpNumModes, ADLMode** lppModes);

///
/// \brief Function to retrieve the display mode information.
/// 
/// This function retrieves the current display mode information. 
/// \platform
/// \ALL
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter. A value of -1 retrieves all modes for the system across multiple GPUs.
/// \param[in] iDisplayIndex The desired display index. If the index is -1, this field is ignored.
/// \param[out] lpNumModes The pointer to the number of modes retrieved.
/// \param[out] lppModes The pointer to the pointer to the retrieved display modes. Refer to the Display ADLMode structure for more information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
///	\remarks On Linux platform iXPos,iYPos,iColourDepth,iOrientation & iModeFlag are not supported. iXPos and iYPos can be retrieved from xrandr.
ADL_EXTERNC int EXPOSED ADL_Display_Modes_Get (int iAdapterIndex, int iDisplayIndex, int* lpNumModes, ADLMode** lppModes);

///
/// \brief Function to set display mode information.
/// 
/// This function sets the current display mode information. 
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter. A value of -1 sets the modes across multiple GPUs.
/// \param[in] iDisplayIndex The desired display index. If the index is -1, this field is ignored.
/// \param[in] iNumModes The number of modes to be set.
/// \param[in] lpModes The pointer to the display mode information to be set. Refer to the ADLMode structure for more information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_Modes_Set (ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int iNumModes, ADLMode* lpModes);

///
/// \brief Function to set display mode information.
/// 
/// This function sets the current display mode information. 
/// \platform
/// \WIN
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter. A value of -1 sets the modes across multiple GPUs.
/// \param[in] iDisplayIndex The desired display index. If the index is -1, this field is ignored.
/// \param[in] iNumModes The number of modes to be set.
/// \param[in] lpModes The pointer to the display mode information to be set. Refer to the ADLMode structure for more information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
ADL_EXTERNC int EXPOSED ADL_Display_Modes_Set (int iAdapterIndex, int iDisplayIndex, int iNumModes, ADLMode* lpModes);

///
/// \brief Function to retrieve the OS possible modes list for an adapter (all OS platforms).
/// 
/// This function retrieves the OS possible modes list for a specified input adapter.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter. 
/// \param[out] lpNumModes The pointer to the number of display modes from the lppModes array. 
/// \param[out] lppModes The pointer to the pointer to the retrieved possible display modes buffer. Refer to the ADLMode structure for more information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_PossibleMode_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int* lpNumModes, ADLMode** lppModes);

///
/// \brief Function to retrieve the OS possible modes list for an adapter (all OS platforms).
/// 
/// This function retrieves the OS possible modes list for a specified input adapter.
/// \platform
/// \WIN
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter. 
/// \param[out] lpNumModes The pointer to the number of display modes from the lppModes array. 
/// \param[out] lppModes The pointer to the pointer to the retrieved possible display modes buffer. Refer to the ADLMode structure for more information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
ADL_EXTERNC int EXPOSED ADL_Display_PossibleMode_Get(int iAdapterIndex, int* lpNumModes, ADLMode** lppModes);

///
/// \brief Function to retrieve the forcible connected status of a display.
/// 
/// This function determines whether or not a display (i.e. CV) is set to be forcibly connected. 
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in] iDisplayIndex The desired display index.
/// \param[out] lpStatus The forcible connected status of the display. \ref ADL_TRUE : forced connected; \ref ADL_FALSE : otherwise.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks A display can be \b forcibly \b connected without being actually connected or not recognized as connected. For example,  some old 50 ohm TVs are not detected properly and need to be forcibly connected in order to be turned on.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_ForcibleDisplay_Get (ADL_CONTEXT_HANDLE context,int iAdapterIndex, 
                                                          int iDisplayIndex, 
                                                          int* lpStatus);

///
/// \brief Function to retrieve the forcible connected status of a display.
/// 
/// This function determines whether or not a display (i.e. CV) is set to be forcibly connected. 
/// \platform
/// \ALL
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in] iDisplayIndex The desired display index.
/// \param[out] lpStatus The forcible connected status of the display. \ref ADL_TRUE : forced connected; \ref ADL_FALSE : otherwise.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks A display can be \b forcibly \b connected without being actually connected or not recognized as connected. For example,  some old 50 ohm TVs are not detected properly and need to be forcibly connected in order to be turned on.
ADL_EXTERNC int EXPOSED ADL_Display_ForcibleDisplay_Get (int iAdapterIndex, 
                                                         int iDisplayIndex, 
                                                         int* lpStatus);

///
/// \brief Function to set the forcible connected status of a display.
/// 
/// This function sets a display (i.e. TV) to be either forcibly connected or not forcibly connected. 
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in] iDisplayIndex The desired display index.
/// \param[in] iStatus The desired connection status. \ref ADL_TRUE : forced connected; \ref ADL_FALSE : otherwise.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks A display can be \b forcibly \b connected without being actually connected or not recognized as connected. For example,  some old 50 ohm TVs are not detected properly and need to be forcibly connected in order to be turned on.
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_ForcibleDisplay_Set (ADL_CONTEXT_HANDLE context,int iAdapterIndex, 
                                                          int iDisplayIndex, 
                                                          int iStatus);

///
/// \brief Function to set the forcible connected status of a display.
/// 
/// This function sets a display (i.e. TV) to be either forcibly connected or not forcibly connected. 
/// \platform
/// \ALL
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in] iDisplayIndex The desired display index.
/// \param[in] iStatus The desired connection status. \ref ADL_TRUE : forced connected; \ref ADL_FALSE : otherwise.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks A display can be \b forcibly \b connected without being actually connected or not recognized as connected. For example,  some old 50 ohm TVs are not detected properly and need to be forcibly connected in order to be turned on.
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
ADL_EXTERNC int EXPOSED ADL_Display_ForcibleDisplay_Set (int iAdapterIndex, 
                                                         int iDisplayIndex, 
                                                         int iStatus);

///
/// \brief Function to retrieve the number of Activatable sources based on ADL Index.
/// 
/// This function retrieves the activatable sources list for a specified input adapter. \b ActivatableSources is the number of  displays that can be turned on simultaneously for the same GPU. For example, for AMD Radeon HD 4xxx Series and earlier this number is 2 while for ATI Radeon HD 5xxx Series the number could be 6 or 4.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter. 
/// \param[out] lpNumSources The pointer to the number of sources that can be activated. 
/// \param[out] lppSources The pointer to the pointer to the retrieved activatable sources buffer. Refer to the ADLActivatableSource structure for more information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_NumberOfActivatableSources_Get (ADL_CONTEXT_HANDLE context,int iAdapterIndex, int* lpNumSources, ADLActivatableSource** lppSources );

///
/// \brief Function to retrieve the number of Activatable sources based on ADL Index.
/// 
/// This function retrieves the activatable sources list for a specified input adapter. \b ActivatableSources is the number of  displays that can be turned on simultaneously for the same GPU. For example, for AMD Radeon HD 4xxx Series and earlier this number is 2 while for ATI Radeon HD 5xxx Series the number could be 6 or 4.
/// \platform
/// \ALL
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter. 
/// \param[out] lpNumSources The pointer to the number of sources that can be activated. 
/// \param[out] lppSources The pointer to the pointer to the retrieved activatable sources buffer. Refer to the ADLActivatableSource structure for more information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
ADL_EXTERNC int EXPOSED ADL_Adapter_NumberOfActivatableSources_Get (int iAdapterIndex, int* lpNumSources, ADLActivatableSource** lppSources );

///
/// \brief Function to retrieve the adapter display manner capabilities based on ADL index.
/// 
/// This function retrieves the adapter display manner capabilities (e.g., single, clone, extended desktop, and stretch mode).
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter. A value of -1 returns all adapter manner information for the system across all GPUs.
/// \param[out] lpNumDisplayCaps The Number of display manner capabilities.
/// \param[out] lppAdapterDisplayCaps The lists of display manner capabilities across all GPUs. Refer to the ADLAdapterDisplayCap structure for more information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_Display_Caps (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* lpNumDisplayCaps, ADLAdapterDisplayCap** lppAdapterDisplayCaps);

///
/// \brief Function to retrieve the adapter display manner capabilities based on ADL index.
/// 
/// This function retrieves the adapter display manner capabilities (e.g., single, clone, extended desktop, and stretch mode).
/// \platform
/// \WIN
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter. A value of -1 returns all adapter manner information for the system across all GPUs.
/// \param[out] lpNumDisplayCaps The Number of display manner capabilities.
/// \param[out] lppAdapterDisplayCaps The lists of display manner capabilities across all GPUs. Refer to the ADLAdapterDisplayCap structure for more information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
ADL_EXTERNC int EXPOSED ADL_Adapter_Display_Caps ( int iAdapterIndex, int* lpNumDisplayCaps, ADLAdapterDisplayCap** lppAdapterDisplayCaps);

/// 
/// \brief Function to retrieve current display map configurations.
///
/// This function retrieves the current display map configurations, including the controllers and adapters mapped to each display.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter. A value of -1 returns all display configurations for the system across multiple GPUs.
/// \param[out] lpNumDisplayMap The pointer to the number of retrieved display maps.
/// \param[out] lppDisplayMap The pointer to the pointer to the display manner information. Refer to the ADLDisplayMap structure for more information. 
/// \param[out] lpNumDisplayTarget The pointer to the display target sets retrieved.
/// \param[out] lppDisplayTarget The pointer to the pointer to the display target buffer. Refer to the ADLDisplayTarget structure for more information.
/// \param[in] iOptions The function option. \ref ADL_DISPLAY_DISPLAYMAP_OPTION_GPUINFO. 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_DisplayMapConfig_Get (ADL_CONTEXT_HANDLE context,int iAdapterIndex, 
                                                           int* lpNumDisplayMap, 
                                                           ADLDisplayMap** lppDisplayMap, 
                                                           int* lpNumDisplayTarget, 
                                                           ADLDisplayTarget** lppDisplayTarget,
                                                           int iOptions);

/// 
/// \brief Function to retrieve current display map configurations.
///
/// This function retrieves the current display map configurations, including the controllers and adapters mapped to each display.
/// \platform
/// \WIN
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter. A value of -1 returns all display configurations for the system across multiple GPUs.
/// \param[out] lpNumDisplayMap The pointer to the number of retrieved display maps.
/// \param[out] lppDisplayMap The pointer to the pointer to the display manner information. Refer to the ADLDisplayMap structure for more information. 
/// \param[out] lpNumDisplayTarget The pointer to the display target sets retrieved.
/// \param[out] lppDisplayTarget The pointer to the pointer to the display target buffer. Refer to the ADLDisplayTarget structure for more information.
/// \param[in] iOptions The function option. \ref ADL_DISPLAY_DISPLAYMAP_OPTION_GPUINFO. 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
ADL_EXTERNC int EXPOSED ADL_Display_DisplayMapConfig_Get (int iAdapterIndex, 
                                                          int* lpNumDisplayMap, 
                                                          ADLDisplayMap** lppDisplayMap, 
                                                          int* lpNumDisplayTarget, 
                                                          ADLDisplayTarget** lppDisplayTarget,
                                                          int iOptions);


///
/// \brief Function to set the current display configuration.
/// 
/// This function sets the current display configurations for each display, including the controller and adapter mapped to each display. 
/// Possible display configurations are single, clone, extended desktop, and stretch mode. 
/// If clone mode is desired and the current display configuration is extended desktop mode, the function disables extended desktop mode in order to enable clone mode. 
/// If extended display mode is desired and the current display configuration is single mode, this function enables extended desktop.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter. A value of -1 sets the displays in the system across multiple GPUs.
/// \param[in] iNumDisplayMap The number of display maps to be set.
/// \param[in] lpDisplayMap The pointer to the retrieved display map information. Refer to the ADLDisplayMap structure for more information.
/// \param[in] iNumDisplayTarget The number of display target sets to be set.
/// \param[in] lpDisplayTarget The pointer to the display target information. Refer to the ADLDisplayTarget structure for more information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_DisplayMapConfig_Set  (ADL_CONTEXT_HANDLE context,int iAdapterIndex, 
                                                            int iNumDisplayMap, 
                                                            ADLDisplayMap* lpDisplayMap, 
                                                            int iNumDisplayTarget,  
                                                            ADLDisplayTarget* lpDisplayTarget);

///
/// \brief Function to set the current display configuration.
/// 
/// This function sets the current display configurations for each display, including the controller and adapter mapped to each display. 
/// Possible display configurations are single, clone, extended desktop, and stretch mode. 
/// If clone mode is desired and the current display configuration is extended desktop mode, the function disables extended desktop mode in order to enable clone mode. 
/// If extended display mode is desired and the current display configuration is single mode, this function enables extended desktop.
/// \platform
/// \WIN
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter. A value of -1 sets the displays in the system across multiple GPUs.
/// \param[in] iNumDisplayMap The number of display maps to be set.
/// \param[in] lpDisplayMap The pointer to the retrieved display map information. Refer to the ADLDisplayMap structure for more information.
/// \param[in] iNumDisplayTarget The number of display target sets to be set.
/// \param[in] lpDisplayTarget The pointer to the display target information. Refer to the ADLDisplayTarget structure for more information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
ADL_EXTERNC int EXPOSED ADL_Display_DisplayMapConfig_Set  (int iAdapterIndex, 
                                                           int iNumDisplayMap, 
                                                           ADLDisplayMap* lpDisplayMap, 
                                                           int iNumDisplayTarget,  
                                                           ADLDisplayTarget* lpDisplayTarget);

///
/// \brief Function to retrieve the possible display mappings.
/// 
/// This function retrieves possible display mappings for an adapter. 
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in] iNumberOfPresetMapping The current number of preset mappings from the lpPresetMappings array.
/// \param[in] lpPresetMappings The pointer to the buffer storing the current preset mappings. Refer to the ADLPossibleMapping structure for more information.
/// \param[in] iEnquiryControllerIndex The desired controller index for mapping.
/// \param[out] lpNumberOfEnquiryPossibleMappings The pointer to the number of possible mappings from the lppEnquiryPossibleMappings array.
/// \param[out] lppEnquiryPossibleMappings The pointer to the pointer to the retrieved possible mapping buffer. Refer to the ADLPossibleMapping structure for more information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_PossibleMapping_Get (ADL_CONTEXT_HANDLE context,int iAdapterIndex, 
                                                          int iNumberOfPresetMapping, 
                                                          ADLPossibleMapping* lpPresetMappings, 
                                                          int iEnquiryControllerIndex, 
                                                          int* lpNumberOfEnquiryPossibleMappings, 
                                                          ADLPossibleMapping** lppEnquiryPossibleMappings);

///
/// \brief Function to retrieve the possible display mappings.
/// 
/// This function retrieves possible display mappings for an adapter. 
/// \platform
/// \WIN
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in] iNumberOfPresetMapping The current number of preset mappings from the lpPresetMappings array.
/// \param[in] lpPresetMappings The pointer to the buffer storing the current preset mappings. Refer to the ADLPossibleMapping structure for more information.
/// \param[in] iEnquiryControllerIndex The desired controller index for mapping.
/// \param[out] lpNumberOfEnquiryPossibleMappings The pointer to the number of possible mappings from the lppEnquiryPossibleMappings array.
/// \param[out] lppEnquiryPossibleMappings The pointer to the pointer to the retrieved possible mapping buffer. Refer to the ADLPossibleMapping structure for more information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
ADL_EXTERNC int EXPOSED ADL_Display_PossibleMapping_Get (int iAdapterIndex, 
                                                         int iNumberOfPresetMapping, 
                                                         ADLPossibleMapping* lpPresetMappings, 
                                                         int iEnquiryControllerIndex, 
                                                         int* lpNumberOfEnquiryPossibleMappings, 
                                                         ADLPossibleMapping** lppEnquiryPossibleMappings);


///
/// \brief Function to validate the list of the display configurations based on ADL Index.
/// 
/// This function validate the list of the display configurations for a specified input adapter. This function is applicable to all OS platforms.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter. 
/// \param[in] iNumPossibleMap The number of possible maps to be validated. 
/// \param[in] lpPossibleMaps The list of possible maps to be validated. Refer to the ADLPossibleMap structure for more information.
/// \param[out] lpNumPossibleMapResult The pointer to the number of validate result list. 
/// \param[out] lppPossibleMapResult The pointer to the pointer to validation result list. Refer to the ADLPossibleMapResult structure for more information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_DisplayMapConfig_Validate(ADL_CONTEXT_HANDLE context,int iAdapterIndex,
                                                               int iNumPossibleMap,
                                                               ADLPossibleMap* lpPossibleMaps,
                                                               int* lpNumPossibleMapResult,
                                                               ADLPossibleMapResult** lppPossibleMapResult);

///
/// \brief Function to validate the list of the display configurations based on ADL Index.
/// 
/// This function validate the list of the display configurations for a specified input adapter. This function is applicable to all OS platforms.
/// \platform
/// \WIN
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter. 
/// \param[in] iNumPossibleMap The number of possible maps to be validated. 
/// \param[in] lpPossibleMaps The list of possible maps to be validated. Refer to the ADLPossibleMap structure for more information.
/// \param[out] lpNumPossibleMapResult The pointer to the number of validate result list. 
/// \param[out] lppPossibleMapResult The pointer to the pointer to validation result list. Refer to the ADLPossibleMapResult structure for more information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
ADL_EXTERNC int EXPOSED ADL_Display_DisplayMapConfig_Validate(int iAdapterIndex,
                                                              int iNumPossibleMap,
                                                              ADLPossibleMap* lpPossibleMaps,
                                                              int* lpNumPossibleMapResult,
                                                              ADLPossibleMapResult** lppPossibleMapResult);


///
/// \brief Function to validate a list of display configurations.
/// 
/// This function allows the user to input a potential displays map and its targets. The function can also be used to obtain a list of display targets that can be added to
/// this given topology and a list of display targets that can be removed from this given topology.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex		The ADL index handle of the desired adapter. This function does not support the -1 value to test topologies across multiple GPUs.
/// \param[in]  iNumDisplayMap		The number of display maps in this configuration.
/// \param[in]  lpDisplayMap		The pointer to the display map information. Refer to the \ref ADLDisplayMap structure for more information.
/// \param[in]  iNumDisplayTarget	The number of display target sets in this configuration.
/// \param[in]  lpDisplayTarget		The pointer to the display target information. Refer to the \ref ADLDisplayTarget structure for more information.
/// \param[out] lpNumPossibleAddTarget		The pointer to the number of items in the array of lppPossibleAddTarget.
/// \param[out] lppPossibleAddTarget		The pointer to the array of the display targets that can be added in the given topology. Refer to the \ref ADLDisplayTarget structure for more information. The user is responsible for freeing the memory for this array when it is no longer needed.
/// \param[out] lpNumPossibleRemoveTarget	The pointer to the number of items in the array of lppPossibleRemoveTarget.
/// \param[out] lppPossibleRemoveTarget		The pointer to the array of the display targets that can be removed from the given topology. Refer to the \ref ADLDisplayTarget structure for more information. The user is responsible for freeing the memory for this array when it is no longer needed.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_DisplayMapConfig_PossibleAddAndRemove (ADL_CONTEXT_HANDLE context,int iAdapterIndex,
                                                                            int iNumDisplayMap, 
                                                                            ADLDisplayMap* lpDisplayMap, 
                                                                            int iNumDisplayTarget,  
                                                                            ADLDisplayTarget* lpDisplayTarget,
                                                                            int* lpNumPossibleAddTarget,
                                                                            ADLDisplayTarget** lppPossibleAddTarget,
                                                                            int* lpNumPossibleRemoveTarget,
                                                                            ADLDisplayTarget** lppPossibleRemoveTarget);

///
/// \brief Function to validate a list of display configurations.
/// 
/// This function allows the user to input a potential displays map and its targets. The function can also be used to obtain a list of display targets that can be added to
/// this given topology and a list of display targets that can be removed from this given topology.
/// \platform
/// \WIN
/// \param[in]  iAdapterIndex		The ADL index handle of the desired adapter. This function does not support the -1 value to test topologies across multiple GPUs.
/// \param[in]  iNumDisplayMap		The number of display maps in this configuration.
/// \param[in]  lpDisplayMap		The pointer to the display map information. Refer to the \ref ADLDisplayMap structure for more information.
/// \param[in]  iNumDisplayTarget	The number of display target sets in this configuration.
/// \param[in]  lpDisplayTarget		The pointer to the display target information. Refer to the \ref ADLDisplayTarget structure for more information.
/// \param[out] lpNumPossibleAddTarget		The pointer to the number of items in the array of lppPossibleAddTarget.
/// \param[out] lppPossibleAddTarget		The pointer to the array of the display targets that can be added in the given topology. Refer to the \ref ADLDisplayTarget structure for more information. The user is responsible for freeing the memory for this array when it is no longer needed.
/// \param[out] lpNumPossibleRemoveTarget	The pointer to the number of items in the array of lppPossibleRemoveTarget.
/// \param[out] lppPossibleRemoveTarget		The pointer to the array of the display targets that can be removed from the given topology. Refer to the \ref ADLDisplayTarget structure for more information. The user is responsible for freeing the memory for this array when it is no longer needed.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
ADL_EXTERNC int EXPOSED ADL_Display_DisplayMapConfig_PossibleAddAndRemove (int iAdapterIndex,
                                                                           int iNumDisplayMap, 
                                                                           ADLDisplayMap* lpDisplayMap, 
                                                                           int iNumDisplayTarget,  
                                                                           ADLDisplayTarget* lpDisplayTarget,
                                                                           int* lpNumPossibleAddTarget,
                                                                           ADLDisplayTarget** lppPossibleAddTarget,
                                                                           int* lpNumPossibleRemoveTarget,
                                                                           ADLDisplayTarget** lppPossibleRemoveTarget);



///
/// \brief Function to get the current supported SLS grid patterns (MxN) for a GPU. 
/// 
/// This function gets a list of supported SLS grids for a specified input adapter based on display devices currently connected to the GPU.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex  The ADL index of the desired adapter. This function does not support -1.
/// \param[out] lpNumSLSGrid   Pointer to a variable that will receive the number of elements in a supported SLS grid list. 
/// \param[out] lppSLSGrid     Pointer of a pointer to a variable with type \ref ADLSLSGrid, which contains supported SLS grids the adapter supports. Application does not need to allocate memory but it should free the pointer since the memory is allocated by the ADL call back function.
/// \param[in]  iOption        Bit vector, specifies the layout type of SLS grid data and portrait flag. There are two types of SLS layouts: relative to landscape (ref \ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_LANDSCAPE) and relative to current angle (ref \ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_CURRENTANGLE). If the current desktop associated with the input adapter is rotated to 90 or 270 degree, set bit ref \ADL_DISPLAY_SLSGRID_PORTAIT_MODE to 1.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_SLSGrid_Caps (ADL_CONTEXT_HANDLE context,int iAdapterIndex, 
                                                   int* lpNumSLSGrid, 
                                                   ADLSLSGrid** lppSLSGrid,
                                                   int iOption);

///
/// \brief Function to get the current supported SLS grid patterns (MxN) for a GPU. 
/// 
/// This function gets a list of supported SLS grids for a specified input adapter based on display devices currently connected to the GPU.
/// \platform
/// \WIN
/// \param[in]  iAdapterIndex  The ADL index of the desired adapter. This function does not support -1.
/// \param[out] lpNumSLSGrid   Pointer to a variable that will receive the number of elements in a supported SLS grid list. 
/// \param[out] lppSLSGrid     Pointer of a pointer to a variable with type \ref ADLSLSGrid, which contains supported SLS grids the adapter supports. Application does not need to allocate memory but it should free the pointer since the memory is allocated by the ADL call back function.
/// \param[in]  iOption        Bit vector, specifies the layout type of SLS grid data and portrait flag. There are two types of SLS layouts: relative to landscape (ref \ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_LANDSCAPE) and relative to current angle (ref \ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_CURRENTANGLE). If the current desktop associated with the input adapter is rotated to 90 or 270 degree, set bit ref \ADL_DISPLAY_SLSGRID_PORTAIT_MODE to 1.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
ADL_EXTERNC int EXPOSED ADL_Display_SLSGrid_Caps (int iAdapterIndex, 
                                                  int* lpNumSLSGrid, 
                                                  ADLSLSGrid** lppSLSGrid,
                                                  int iOption);


///
/// \brief Function to get the active SLS map index list for a given GPU. 
/// 
/// This function retrieves a list of active SLS map indexes for a specified input GPU.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapterIndex         The ADL index of the desired adapter. This function does not support -1.
/// \param[out] lpNumSLSMapIndexList Pointer to a variable that will receive the number of elements in active SLS maps index list. 
/// \param[out] lppSLSMapIndexList   Pointer of a pointer to a variable with type of integer array. The integer array stores all active SLS map indexes. Application does not need to allocate memory, but it should free the memory since the memory is allocated by ADL call back function. 
/// \param[in]  iOptions             Bit vector, specifies the type of SLS map. Currently only \ref ADL_DISPLAY_SLSMAPINDEXLIST_OPTION_ACTIVE is supported.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks	The driver does not support the return of all SLS grids. It only supports all active SLS grids. Therefore 0x00000001 is the only valid value for iOption.
///
/// \remarks This API uses the memory allocation callback function provided by the user.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_SLSMapIndexList_Get (ADL_CONTEXT_HANDLE context,int iAdapterIndex, int* lpNumSLSMapIndexList,int** lppSLSMapIndexList,int iOptions );

///
/// \brief Function to get the active SLS map index list for a given GPU. 
/// 
/// This function retrieves a list of active SLS map indexes for a specified input GPU.
/// \platform
/// \WIN
/// \param[in] iAdapterIndex         The ADL index of the desired adapter. This function does not support -1.
/// \param[out] lpNumSLSMapIndexList Pointer to a variable that will receive the number of elements in active SLS maps index list. 
/// \param[out] lppSLSMapIndexList   Pointer of a pointer to a variable with type of integer array. The integer array stores all active SLS map indexes. Application does not need to allocate memory, but it should free the memory since the memory is allocated by ADL call back function. 
/// \param[in]  iOptions             Bit vector, specifies the type of SLS map. Currently only \ref ADL_DISPLAY_SLSMAPINDEXLIST_OPTION_ACTIVE is supported.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks	The driver does not support the return of all SLS grids. It only supports all active SLS grids. Therefore 0x00000001 is the only valid value for iOption.
///
/// \remarks This API uses the memory allocation callback function provided by the user.
ADL_EXTERNC int EXPOSED ADL_Display_SLSMapIndexList_Get (int iAdapterIndex, int* lpNumSLSMapIndexList,int** lppSLSMapIndexList,int iOptions );


///
/// \brief Function to get the SLS map index for a given adapter and a given display device. 
/// 
/// This function to get a SLS map index based on a group of displays that are connected in the given adapter. The driver only searches the active SLS grid database.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapterIndex        The ADL index of the desired adapter. This function does not support -1.
/// \param[in] iADLNumDisplayTarget Specifies the number of input displays.
/// \param[in] lpDisplayTarget      Array of displays that are used to do the query. The array type is \ref ADLDisplayTarget.
/// \param[out]lpSLSMapIndex        Pointer to a variable that will receive the SLS map index. If the displays in an SLS map match the input displays, a valid SLS map index will be assigned to this parameter. Otherwise, -1 will be assigned to it.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_SLSMapIndex_Get (ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iADLNumDisplayTarget,ADLDisplayTarget* lpDisplayTarget,int* lpSLSMapIndex);

///
/// \brief Function to get the SLS map index for a given adapter and a given display device. 
/// 
/// This function to get a SLS map index based on a group of displays that are connected in the given adapter. The driver only searches the active SLS grid database.
/// \platform
/// \WIN
/// \param[in] iAdapterIndex        The ADL index of the desired adapter. This function does not support -1.
/// \param[in] iADLNumDisplayTarget Specifies the number of input displays.
/// \param[in] lpDisplayTarget      Array of displays that are used to do the query. The array type is \ref ADLDisplayTarget.
/// \param[out]lpSLSMapIndex        Pointer to a variable that will receive the SLS map index. If the displays in an SLS map match the input displays, a valid SLS map index will be assigned to this parameter. Otherwise, -1 will be assigned to it.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_SLSMapIndex_Get (int iAdapterIndex, int iADLNumDisplayTarget,ADLDisplayTarget* lpDisplayTarget,int* lpSLSMapIndex);



/// 
/// \brief Function to retrieve an SLS configuration.
///
/// This function retrieves an SLS configuration, which includes the, SLS map, SLS targets, SLS standard modes, bezel modes or a transient mode, and offsets. 
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex          Specifies the adapter to be queried. This function does not support -1.
/// \param[in]  iSLSMapIndex           Specifies the SLS map index to be queried.
/// \param[out] lpSLSMap               Pointer to a variable that contains the SLS map data. The data type is \ref ADLSLSMap. 
/// \param[out] lpNumSLSTarget         Pointer to variable that will receive the number of targets in the SLS map.
/// \param[out] lppSLSTarget           Pointer of a pointer to a variable that conations targets in the SLS map. the data type is \ref ADLDisplayTarget Application does not need to allocate memory but it should free the pointer since the memory is allocated by ADL call back function.
/// \param[out] lpNumSLSNativeMode     Pointer to a variable that will receive the number of native SLS modes supported by the SLS configuration.
/// \param[out] lppSLSNativeMode       Pointer of a pointer to a variable that contains the native SLS modes. the data type is \ref ADLSLSMode. Application does not need to allocate memory but it should free the pointer since the memory is allocated by ADL call back function.
/// \param[out] lpNumNativeModeOffsets Pointer to a variable that will receive the number of native SLS mode offsets supported by the SLS configuration.
/// \param[out] lppNativeModeOffsets   Pointer of a pointer to a variable that contains the native SLS mode offsets. the data type is \ref ADLSLSOffset. Application does not need to allocate memory but it should free the pointer since the memory is allocated by ADL call back function.
/// \param[out] lpNumSLSBezelMode      Pointer to a variable that will receive the number of the bezel modes supported by the SLS configuration.
/// \param[out] lppSLSbezelMode        Pointer of a pointer to a variable that contains the bezel SLS modes. the data type is \ref ADLSLSMode. Application does not need to allocate memory but it should free the pointer since the memory is allocated by ADL call back function.
/// \param[out] lpNumSLSTransientMode  Pointer to a variable that will receive the number of the transient modes supported by the SLS configuration.
/// \param[out] lppTransientMode       Pointer of a pointer to a variable that contains the transient SLS modes. the data type is \ref ADLSLSMode. Application does not need to allocate memory but it should free the pointer since the memory is allocated by ADL call back function.
/// \param[out] lpNumSLSOffset         Pointer to a variable that will receive the number of the SLS offset supported by the SLS configuration.
/// \param[out] lppSLSOffset           Pointer of a pointer to a variable that contains the SLS offsets. the data type is \ref ADLSLSOffset. Application does not need to allocate memory but it should free the pointer since the memory is allocated by ADL call back function.
/// \param[in]  iOption                Specifies the layout type of SLS grid data. It is bit vector. There are two types of SLS layout:s, relative to landscape (ref \ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_LANDSCAPE) and relative to current angle (ref \ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_CURRENTANGLE).
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_SLSMapConfigX2_Get (ADL_CONTEXT_HANDLE context,int iAdapterIndex, 
                                                         int iSLSMapIndex, 
                                                         ADLSLSMap* lpSLSMap, 
                                                         int* lpNumSLSTarget, 
                                                         ADLSLSTarget** lppSLSTarget,
                                                         int* lpNumNativeMode,
                                                         ADLSLSMode** lppNativeMode,
                                                         int* lpNumNativeModeOffsets,
                                                         ADLSLSOffset** lppNativeModeOffsets,
                                                         int* lpNumBezelMode,
                                                         ADLBezelTransientMode** lppBezelMode,
                                                         int* lpNumTransientMode,
                                                         ADLBezelTransientMode** lppTransientMode,
                                                         int* lpNumSLSOffset,
                                                         ADLSLSOffset** lppSLSOffset,
                                                         int iOption);

/// 
/// \brief Function to retrieve an SLS configuration.
///
/// This function retrieves an SLS configuration, which includes the, SLS map, SLS targets, SLS standard modes, bezel modes or a transient mode, and offsets. 
/// \platform
/// \WIN
/// \param[in]  iAdapterIndex          Specifies the adapter to be queried. This function does not support -1.
/// \param[in]  iSLSMapIndex           Specifies the SLS map index to be queried.
/// \param[out] lpSLSMap               Pointer to a variable that contains the SLS map data. The data type is \ref ADLSLSMap. 
/// \param[out] lpNumSLSTarget         Pointer to variable that will receive the number of targets in the SLS map.
/// \param[out] lppSLSTarget           Pointer of a pointer to a variable that conations targets in the SLS map. the data type is \ref ADLDisplayTarget Application does not need to allocate memory but it should free the pointer since the memory is allocated by ADL call back function.
/// \param[out] lpNumSLSNativeMode     Pointer to a variable that will receive the number of native SLS modes supported by the SLS configuration.
/// \param[out] lppSLSNativeMode       Pointer of a pointer to a variable that contains the native SLS modes. the data type is \ref ADLSLSMode. Application does not need to allocate memory but it should free the pointer since the memory is allocated by ADL call back function.
/// \param[out] lpNumNativeModeOffsets Pointer to a variable that will receive the number of native SLS mode offsets supported by the SLS configuration.
/// \param[out] lppNativeModeOffsets   Pointer of a pointer to a variable that contains the native SLS mode offsets. the data type is \ref ADLSLSOffset. Application does not need to allocate memory but it should free the pointer since the memory is allocated by ADL call back function.
/// \param[out] lpNumSLSBezelMode      Pointer to a variable that will receive the number of the bezel modes supported by the SLS configuration.
/// \param[out] lppSLSbezelMode        Pointer of a pointer to a variable that contains the bezel SLS modes. the data type is \ref ADLSLSMode. Application does not need to allocate memory but it should free the pointer since the memory is allocated by ADL call back function.
/// \param[out] lpNumSLSTransientMode  Pointer to a variable that will receive the number of the transient modes supported by the SLS configuration.
/// \param[out] lppTransientMode       Pointer of a pointer to a variable that contains the transient SLS modes. the data type is \ref ADLSLSMode. Application does not need to allocate memory but it should free the pointer since the memory is allocated by ADL call back function.
/// \param[out] lpNumSLSOffset         Pointer to a variable that will receive the number of the SLS offset supported by the SLS configuration.
/// \param[out] lppSLSOffset           Pointer of a pointer to a variable that contains the SLS offsets. the data type is \ref ADLSLSOffset. Application does not need to allocate memory but it should free the pointer since the memory is allocated by ADL call back function.
/// \param[in]  iOption                Specifies the layout type of SLS grid data. It is bit vector. There are two types of SLS layout:s, relative to landscape (ref \ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_LANDSCAPE) and relative to current angle (ref \ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_CURRENTANGLE).
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
ADL_EXTERNC int EXPOSED ADL_Display_SLSMapConfigX2_Get (int iAdapterIndex, 
                                                        int iSLSMapIndex, 
                                                        ADLSLSMap* lpSLSMap, 
                                                        int* lpNumSLSTarget, 
                                                        ADLSLSTarget** lppSLSTarget,
                                                        int* lpNumNativeMode,
                                                        ADLSLSMode** lppNativeMode,
                                                        int* lpNumNativeModeOffsets,
                                                        ADLSLSOffset** lppNativeModeOffsets,
                                                        int* lpNumBezelMode,
                                                        ADLBezelTransientMode** lppBezelMode,
                                                        int* lpNumTransientMode,
                                                        ADLBezelTransientMode** lppTransientMode,
                                                        int* lpNumSLSOffset,
                                                        ADLSLSOffset** lppSLSOffset,
                                                        int iOption);

/// 
/// \brief Function to retrieve an SLS configuration.
///
/// This function retrieves an SLS configuration, which includes the, SLS map, SLS targets, SLS standard modes, bezel modes or a transient mode, and offsets. 
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex          Specifies the adapter to be queried. This function does not support -1.
/// \param[in]  iSLSMapIndex           Specifies the SLS map index to be queried.
/// \param[out] lpSLSMap               Pointer to a variable that contains the SLS map data. The data type is \ref ADLSLSMap. 
/// \param[out] lpNumSLSTarget         Pointer to variable that will receive the number of targets in the SLS map.
/// \param[out] lppSLSTarget           Pointer of a pointer to a variable that conations targets in the SLS map. the data type is \ref ADLDisplayTarget Application does not need to allocate memory but it should free the pointer since the memory is allocated by ADL call back function.
/// \param[out] lpNumSLSNativeMode     Pointer to a variable that will receive the number of native SLS modes supported by the SLS configuration.
/// \param[out] lppSLSNativeMode       Pointer of a pointer to a variable that contains the native SLS modes. the data type is \ref ADLSLSMode. Application does not need to allocate memory but it should free the pointer since the memory is allocated by ADL call back function.
/// \param[out] lpNumSLSBezelMode      Pointer to a variable that will receive the number of the bezel modes supported by the SLS configuration.
/// \param[out] lppSLSbezelMode        Pointer of a pointer to a variable that contains the bezel SLS modes. the data type is \ref ADLSLSMode. Application does not need to allocate memory but it should free the pointer since the memory is allocated by ADL call back function.
/// \param[out] lpNumSLSTransientMode  Pointer to a variable that will receive the number of the transient modes supported by the SLS configuration.
/// \param[out] lppTransientMode       Pointer of a pointer to a variable that contains the transient SLS modes. the data type is \ref ADLSLSMode. Application does not need to allocate memory but it should free the pointer since the memory is allocated by ADL call back function.
/// \param[out] lpNumSLSOffset         Pointer to a variable that will receive the number of the SLS offset supported by the SLS configuration.
/// \param[out] lppSLSOffset           Pointer of a pointer to a variable that contains the SLS offsets. the data type is \ref ADLSLSOffset. Application does not need to allocate memory but it should free the pointer since the memory is allocated by ADL call back function.
/// \param[in]  iOption                Specifies the layout type of SLS grid data. It is bit vector. There are two types of SLS layout:s, relative to landscape (ref \ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_LANDSCAPE) and relative to current angle (ref \ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_CURRENTANGLE).
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
/// \remarks Clients other than CCC are expected to set "ADL_4KWORKAROUND_CANCEL" environment variable before calling this function.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_SLSMapConfig_Get (ADL_CONTEXT_HANDLE context,int iAdapterIndex, 
                                                       int iSLSMapIndex, 
                                                       ADLSLSMap* lpSLSMap, 
                                                       int* lpNumSLSTarget, 
                                                       ADLSLSTarget** lppSLSTarget,
                                                       int* lpNumNativeMode,
                                                       ADLSLSMode** lppNativeMode,
                                                       int* lpNumBezelMode,
                                                       ADLBezelTransientMode** lppBezelMode,
                                                       int* lpNumTransientMode,
                                                       ADLBezelTransientMode** lppTransientMode,
                                                       int* lpNumSLSOffset,
                                                       ADLSLSOffset** lppSLSOffset,
                                                       int iOption);

/// 
/// \brief Function to retrieve an SLS configuration.
///
/// This function retrieves an SLS configuration, which includes the, SLS map, SLS targets, SLS standard modes, bezel modes or a transient mode, and offsets. 
/// \platform
/// \WIN
/// \param[in]  iAdapterIndex          Specifies the adapter to be queried. This function does not support -1.
/// \param[in]  iSLSMapIndex           Specifies the SLS map index to be queried.
/// \param[out] lpSLSMap               Pointer to a variable that contains the SLS map data. The data type is \ref ADLSLSMap. 
/// \param[out] lpNumSLSTarget         Pointer to variable that will receive the number of targets in the SLS map.
/// \param[out] lppSLSTarget           Pointer of a pointer to a variable that conations targets in the SLS map. the data type is \ref ADLDisplayTarget Application does not need to allocate memory but it should free the pointer since the memory is allocated by ADL call back function.
/// \param[out] lpNumSLSNativeMode     Pointer to a variable that will receive the number of native SLS modes supported by the SLS configuration.
/// \param[out] lppSLSNativeMode       Pointer of a pointer to a variable that contains the native SLS modes. the data type is \ref ADLSLSMode. Application does not need to allocate memory but it should free the pointer since the memory is allocated by ADL call back function.
/// \param[out] lpNumSLSBezelMode      Pointer to a variable that will receive the number of the bezel modes supported by the SLS configuration.
/// \param[out] lppSLSbezelMode        Pointer of a pointer to a variable that contains the bezel SLS modes. the data type is \ref ADLSLSMode. Application does not need to allocate memory but it should free the pointer since the memory is allocated by ADL call back function.
/// \param[out] lpNumSLSTransientMode  Pointer to a variable that will receive the number of the transient modes supported by the SLS configuration.
/// \param[out] lppTransientMode       Pointer of a pointer to a variable that contains the transient SLS modes. the data type is \ref ADLSLSMode. Application does not need to allocate memory but it should free the pointer since the memory is allocated by ADL call back function.
/// \param[out] lpNumSLSOffset         Pointer to a variable that will receive the number of the SLS offset supported by the SLS configuration.
/// \param[out] lppSLSOffset           Pointer of a pointer to a variable that contains the SLS offsets. the data type is \ref ADLSLSOffset. Application does not need to allocate memory but it should free the pointer since the memory is allocated by ADL call back function.
/// \param[in]  iOption                Specifies the layout type of SLS grid data. It is bit vector. There are two types of SLS layout:s, relative to landscape (ref \ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_LANDSCAPE) and relative to current angle (ref \ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_CURRENTANGLE).
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
/// \remarks Clients other than CCC are expected to set "ADL_4KWORKAROUND_CANCEL" environment variable before calling this function.
ADL_EXTERNC int EXPOSED ADL_Display_SLSMapConfig_Get (int iAdapterIndex, 
                                                      int iSLSMapIndex, 
                                                      ADLSLSMap* lpSLSMap, 
                                                      int* lpNumSLSTarget, 
                                                      ADLSLSTarget** lppSLSTarget,
                                                      int* lpNumNativeMode,
                                                      ADLSLSMode** lppNativeMode,
                                                      int* lpNumBezelMode,
                                                      ADLBezelTransientMode** lppBezelMode,
                                                      int* lpNumTransientMode,
                                                      ADLBezelTransientMode** lppTransientMode,
                                                      int* lpNumSLSOffset,
                                                      ADLSLSOffset** lppSLSOffset,
                                                      int iOption);



///
/// \brief Function to create an SLS configuration.
/// 
/// This function creates an SLS configuration with a given grid, targets, and bezel mode percent. It will output an SLS map index if the SLS map is successfully created.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapterIndex      The ADL index of the desired adapter. This function does not support -1.
/// \param[in]  SLSMap            Specifies the display SLS map data that contains SLS grid and orientation information. Its type is \ref ADLSLSMap
/// \param[in]  iNumTarget        Specifies the number of displays to be used to create an SLS.
/// \param[in]  lpSLSTarget       Array of displays that are used to create an SLS. The data type is \ref ADLSLSTarget.
/// \param[in]  iBezelModePercent Specifies the bezel mode percent. If these displays do not support bezel, set it to 0.
/// \param[out] lpSLSMapIndex     Pointer to a variable that will receive the new created SLS map index. If an SLS grid failed to create, its value is -1, other wise, it is non-negative integer.
/// \param[in]  iOption           Specifies the layout type of SLS grid data. It is bit vector. There are two types of SLS layouts,: relative to landscape (ref \ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_LANDSCAPE) and relative to current angle (ref \ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_CURRENTANGLE).
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients other than CCC are expected to set SLSMap->iSLSMapValue to \ref ADL_DISPLAY_SLSMAP_SLSLAYOUTMODE_FILL 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_SLSMapConfig_Create (ADL_CONTEXT_HANDLE context,int iAdapterIndex, 
                                                          ADLSLSMap SLSMap, 
                                                          int iNumTarget, 
                                                          ADLSLSTarget* lpSLSTarget,
                                                          int iBezelModePercent,
                                                          int *lpSLSMapIndex	,
                                                          int iOption
                                                          );

///
/// \brief Function to create an SLS configuration.
/// 
/// This function creates an SLS configuration with a given grid, targets, and bezel mode percent. It will output an SLS map index if the SLS map is successfully created.
/// \platform
/// \WIN
/// \param[in] iAdapterIndex      The ADL index of the desired adapter. This function does not support -1.
/// \param[in]  SLSMap            Specifies the display SLS map data that contains SLS grid and orientation information. Its type is \ref ADLSLSMap
/// \param[in]  iNumTargetTarget  Specifies the number of displays to be used to create an SLS.
/// \param[in]  lpSLSTarget       Array of displays that are used to create an SLS. The data type is \ref ADLSLSTarget.
/// \param[in]  iBezelModePercent Specifies the bezel mode percent. If these displays do not support bezel, set it to 0.
/// \param[out] lpSLSMapIndex     Pointer to a variable that will receive the new created SLS map index. If an SLS grid failed to create, its value is -1, other wise, it is non-negative integer.
/// \param[in]  iOption           Specifies the layout type of SLS grid data. It is bit vector. There are two types of SLS layouts,: relative to landscape (ref \ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_LANDSCAPE) and relative to current angle (ref \ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_CURRENTANGLE).
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients other than CCC are expected to set SLSMap->iSLSMapValue to \ref ADL_DISPLAY_SLSMAP_SLSLAYOUTMODE_FILL 
ADL_EXTERNC int EXPOSED ADL_Display_SLSMapConfig_Create (int iAdapterIndex, 
                                                         ADLSLSMap SLSMap, 
                                                         int iNumTargetTarget, 
                                                         ADLSLSTarget* lpSLSTarget,
                                                         int iBezelModePercent,
                                                         int *lpSLSMapIndex	,
                                                         int iOption
                                                         );

///
/// \brief Function to delete an SLS map from the driver database.
/// 
/// This function deletes an SLS map from the driver database based on the input SLS map index.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapterIndex  The ADL index of the desired adapter. This function does not support -1.
/// \param[in] iSLSMapIndex   Specifies the SLS map index to be deleted.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_SLSMapConfig_Delete (ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iSLSMapIndex);

///
/// \brief Function to delete an SLS map from the driver database.
/// 
/// This function deletes an SLS map from the driver database based on the input SLS map index.
/// \platform
/// \WIN
/// \param[in] iAdapterIndex  The ADL index of the desired adapter. This function does not support -1.
/// \param[in] iSLSMapIndex   Specifies the SLS map index to be deleted.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
ADL_EXTERNC int EXPOSED ADL_Display_SLSMapConfig_Delete (int iAdapterIndex, int iSLSMapIndex);

///
/// \brief Function to delete an list of SLS map indexes from the driver database.
/// 
/// This function deletes all SLS maps in a list from the driver database based on the input SLS map index.
/// \platform
/// \WIN
/// \param[in] iAdapterIndex  The ADL index of the desired adapter. This function does not support -1.
/// \param[in] iNumSLSMapINdex Specifies number of SLS Map Indexes for input
/// \param[in] lpSLSMapIndexes, array of SLSMapIndexes to be deleted
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
ADL_EXTERNC int EXPOSED ADL2_Display_SLSMapConfigX2_Delete (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iNumSLSMapIndex, int *lpSLSMapIndexes);


///
/// \brief Function to enable/disable SLS bind.
/// 
/// This function enables or disables an SLS bind
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapterIndex  The ADL index of the desired adapter. This function does not support -1.
/// \param[in] iSLSMapIndex   Specifies the SLS map index to be deleted.
/// \param[in] iState         Specifies the action type. To enable SLS map, sets it to \ref ADL_TRUE. Otherwise set it to \ref ADL_FASLE.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_SLSMapConfig_SetState (ADL_CONTEXT_HANDLE context,int iAdapterIndex, 
                                                            int iSLSMapIndex,
                                                            int iState);

///
/// \brief Function to enable/disable SLS bind.
/// 
/// This function enables or disables an SLS bind
/// \platform
/// \WIN
/// \param[in] iAdapterIndex  The ADL index of the desired adapter. This function does not support -1.
/// \param[in] iSLSMapIndex   Specifies the SLS map index to be deleted.
/// \param[in] iState         Specifies the action type. To enable SLS map, sets it to \ref ADL_TRUE. Otherwise set it to \ref ADL_FASLE.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
ADL_EXTERNC int EXPOSED ADL_Display_SLSMapConfig_SetState (int iAdapterIndex, 
                                                           int iSLSMapIndex,
                                                           int iState);

///
/// \brief Function to rearrange display orders in an SLS map.
/// 
/// This function changes the display orders in an SLS map in order to make the image show on the displays correctly.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapterIndex  Specifies the adapter to be operated on. This function does not support -1.
/// \param[in] iSLSMapIndex   Specifies the SLS map index that will need to do rearrangement
/// \param[in] iNumSLSTarget  Specifies the number of displays in the SLS map.
/// \param[in] lpSLSTarget    Array of displays that are in the SLS map.
/// \param[in] slsMap         Specifies the SLS map info. Its type is \ref ADLSLSMap. The only needed data is the map orientation. All others are ignored by the function.  
/// \param[in] iOption        Specifies the layout type of SLS grid data. It is bit vector. There are two types of SLS layouts, relative to landscape (ref \ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_LANDSCAPE) and relative to current angle (ref \ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_CURRENTANGLE).
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_SLSMapConfig_Rearrange (ADL_CONTEXT_HANDLE context,int iAdapterIndex, 
                                                             int iSLSMapIndex,
                                                             int iNumDisplayTarget,
                                                             ADLSLSTarget* lpSLSTarget,
                                                             ADLSLSMap slsMap,
                                                             int iOption);

///
/// \brief Function to rearrange display orders in an SLS map.
/// 
/// This function changes the display orders in an SLS map in order to make the image show on the displays correctly.
/// \platform
/// \WIN
/// \param[in] iAdapterIndex  Specifies the adapter to be operated on. This function does not support -1.
/// \param[in] iSLSMapIndex   Specifies the SLS map index that will need to do rearrangement
/// \param[in] iNumSLSTarget  Specifies the number of displays in the SLS map.
/// \param[in] lpSLSTarget    Array of displays that are in the SLS map.
/// \param[in] slsMap         Specifies the SLS map info. Its type is \ref ADLSLSMap. The only needed data is the map orientation. All others are ignored by the function.  
/// \param[in] iOption        Specifies the layout type of SLS grid data. It is bit vector. There are two types of SLS layouts, relative to landscape (ref \ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_LANDSCAPE) and relative to current angle (ref \ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_CURRENTANGLE).
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_SLSMapConfig_Rearrange (int iAdapterIndex, 
                                                            int iSLSMapIndex,
                                                            int iNumDisplayTarget,
                                                            ADLSLSTarget* lpSLSTarget,
                                                            ADLSLSMap slsMap,
                                                            int iOption);






#if defined (_XP)
///
/// \brief Function to retrieve the ATI possible modes list for an adapter and preset mapping (Windows XP).
/// 
/// This function retrieves the ATI possible modes list based on a specified input adapter and preset mapping. This function is applicable only on Windows XP platforms.
/// \platform
/// \XP
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in] iNumDisplayTargets The number of display target from the lpDisplayTargets array. 
/// \param[in] lpDisplayTargets The pointer to the retrieved display target configurations. Refer to the ADLDisplayTarget structure for more information. 
/// E.g. If a CRT and TV are on as extended mode, lpMapConfigs will contain two ADLDisplayTarget structures: 
/// one CRT and one TV, both in extended manner.
/// \param[in] iLargeDesktopSupportedType The large desktop type. Refer to the ADLLARGEDESKTOPTYPE enum for more information.
/// \param[in] iDevicePanningControl The device panning control mode. Refer to the ADLPanningMode enum for more information.
/// \param[out] lpNumModes The pointer to the number of modes from the lppModes array. 
/// \param[out] lppModes The pointer to the pointer to the possible display modes buffer. Refer to the ADLMode structure for more information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_PossibleMode_WinXP_Get (ADL_CONTEXT_HANDLE context,int iAdapterIndex, 
                                                             int iNumDisplayTargets, 
                                                             ADLDisplayTarget* lpDisplayTargets,
                                                             int iLargeDesktopSupportedType,
                                                             int iDevicePanningControl,
                                                             int* lpNumModes, 
                                                             ADLMode** lppModes);

///
/// \brief Function to retrieve the ATI possible modes list for an adapter and preset mapping (Windows XP).
/// 
/// This function retrieves the ATI possible modes list based on a specified input adapter and preset mapping. This function is applicable only on Windows XP platforms.
/// \platform
/// \XP
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in] iNumDisplayTargets The number of display target from the lpDisplayTargets array. 
/// \param[in] lpDisplayTargets The pointer to the retrieved display target configurations. Refer to the ADLDisplayTarget structure for more information. 
/// E.g. If a CRT and TV are on as extended mode, lpMapConfigs will contain two ADLDisplayTarget structures: 
/// one CRT and one TV, both in extended manner.
/// \param[in] iLargeDesktopSupportedType The large desktop type. Refer to the ADLLARGEDESKTOPTYPE enum for more information.
/// \param[in] iDevicePanningControl The device panning control mode. Refer to the ADLPanningMode enum for more information.
/// \param[out] lpNumModes The pointer to the number of modes from the lppModes array. 
/// \param[out] lppModes The pointer to the pointer to the possible display modes buffer. Refer to the ADLMode structure for more information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
ADL_EXTERNC int EXPOSED ADL_Display_PossibleMode_WinXP_Get (int iAdapterIndex, 
                                                            int iNumDisplayTargets, 
                                                            ADLDisplayTarget* lpDisplayTargets,
                                                            int iLargeDesktopSupportedType,
                                                            int iDevicePanningControl,
                                                            int* lpNumModes, 
                                                            ADLMode** lppModes);
#endif


///
/// \brief Get bezel offset stepping size for the input adapter
/// 
///
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex                  The ADL index of the desired adapter. This function does not support -1.
/// \param[out] iNumBezelOffsetSteppingSize    The number of Bezel offset stepping size per adapter.
/// \param[out] lppBezelOffsetSteppingSizeint  Bezel offset stepping size information. Refer to \ref ADLBezelOffsetSteppingSize struct.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_BezelOffsetSteppingSize_Get (ADL_CONTEXT_HANDLE context,int iAdapterIndex, 
                                                                  int* lpNumBezelOffsetSteppingSize, 
                                                                  ADLBezelOffsetSteppingSize** lppBezelOffsetSteppingSize);

///
/// \brief Get bezel offset stepping size for the input adapter
/// 
///
/// \platform
/// \WIN
/// \param[in]  iAdapterIndex                  The ADL index of the desired adapter. This function does not support -1.
/// \param[out] iNumBezelOffsetSteppingSize    The number of Bezel offset stepping size per adapter.
/// \param[out] lppBezelOffsetSteppingSizeint  Bezel offset stepping size information. Refer to \ref ADLBezelOffsetSteppingSize struct.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_BezelOffsetSteppingSize_Get (int iAdapterIndex, 
                                                                 int* lpNumBezelOffsetSteppingSize, 
                                                                 ADLBezelOffsetSteppingSize** lppBezelOffsetSteppingSize);

///
/// \brief Set SLS bezel offsets for each display index.
/// 
///
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapterIndex        The ADL index of the desired adapter. This function does not support -1.
/// \param[in] iSLSMapIndex         The SLS map index to apply bezel offset.
/// \param[in] iNumBezelOffset      input size of the SLS offset per display per mode array.
/// \param[in] lpBezelOffset        List of SLS offset information, refer to \ref ADLBezelOffset struct.
/// \param[in] SLSMap               The display SLS map information. Refer to the \ref ADLSLSMap structure for more information.
/// \param[in] iOption              Option to set bezel offset. Default: \ref ADL_DISPLAY_BEZELOFFSET_COMMIT
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients other than CCC are expected to set SLSMap->iSLSMapMask to 0.
/// \remarks Clients other than CCC are expected to set SLSMap->iSLSMapValue to \ref ADL_DISPLAY_SLSMAP_SLSLAYOUTMODE_FILL
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_BezelOffset_Set (ADL_CONTEXT_HANDLE context,int iAdapterIndex, 
                                                      int iSLSMapIndex, 
                                                      int iNumBezelOffset, 
                                                      LPADLSLSOffset lpBezelOffset, 
                                                      ADLSLSMap SLSMap, 
                                                      int iOption);

///
/// \brief Set SLS bezel offsets for each display index.
/// 
///
/// \platform
/// \WIN
/// \param[in] iAdapterIndex        The ADL index of the desired adapter. This function does not support -1.
/// \param[in] iSLSMapIndex         The SLS map index to apply bezel offset.
/// \param[in] iNumBezelOffset      input size of the SLS offset per display per mode array.
/// \param[in] lpBezelOffset        List of SLS offset information, refer to \ref ADLBezelOffset struct.
/// \param[in] SLSMap               The display SLS map information. Refer to the \ref ADLSLSMap structure for more information.
/// \param[in] iOption              Option to set bezel offset. Default: \ref ADL_DISPLAY_BEZELOFFSET_COMMIT
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients other than CCC are expected to set SLSMap->iSLSMapMask to 0.
/// \remarks Clients other than CCC are expected to set SLSMap->iSLSMapValue to \ref ADL_DISPLAY_SLSMAP_SLSLAYOUTMODE_FILL
ADL_EXTERNC int EXPOSED ADL_Display_BezelOffset_Set (int iAdapterIndex, 
                                                     int iSLSMapIndex, 
                                                     int iNumBezelOffset, 
                                                     LPADLSLSOffset lpBezelOffset, 
                                                     ADLSLSMap SLSMap, 
                                                     int iOption);

///
/// \brief Validate the list of the SLS display configurations to determine if bezel is supported or not.
/// 
///
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex              The ADL index of the desired adapter. This function does not support -1.
/// \param[in]  iNumPossibleSLSMap         Input size of SLS map to be validated.
/// \param[in]  lpPossibleSLSMaps          List of possible SLS maps to be validated.
/// \param[out] lpNumPossibleSLSMapResult  Size of the valid array.
/// \param[out] lppPossibleMapResult valid result list.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_BezelSupported_Validate (ADL_CONTEXT_HANDLE context,int iAdapterIndex, 
                                                              int iNumPossibleSLSMap, 
                                                              LPADLPossibleSLSMap lpPossibleSLSMaps, 
                                                              int* lpNumPossibleSLSMapResult, 
                                                              LPADLPossibleMapResult* lppPossibleMapResult);

///
/// \brief Validate the list of the SLS display configurations to determine if bezel is supported or not.
/// 
///
/// \platform
/// \WIN
/// \param[in]  iAdapterIndex              The ADL index of the desired adapter. This function does not support -1.
/// \param[in]  iNumPossibleSLSMap         Input size of SLS map to be validated.
/// \param[in]  lpPossibleSLSMaps          List of possible SLS maps to be validated.
/// \param[out] lpNumPossibleSLSMapResult  Size of the valid array.
/// \param[out] lppPossibleMapResult valid result list.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_BezelSupported_Validate (int iAdapterIndex, 
                                                             int iNumPossibleSLSMap, 
                                                             LPADLPossibleSLSMap lpPossibleSLSMaps, 
                                                             int* lpNumPossibleSLSMapResult, 
                                                             LPADLPossibleMapResult* lppPossibleMapResult);

///
/// \brief Set SLS overlap offsets for each display index.
/// 
///
/// \platform
/// \WIN7 and above
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapterIndex                The ADL index of the desired adapter. This function does not support -1.
/// \param[in] iSLSMapIndex                 The SLS map index to apply Overlap offset.
/// \param[in] iNumOverlappedModes          the no. of overlapped modes defined. Each such mode refers to a set of targets in the array of targets
/// \param[in] lpOverlappedModes            the list of overlapped modes defined
/// \param[in] iNumTargetOffsets            the no. of targets in the list of targets
/// \param[in] lpTargetOffsets              the list of target offsets for all the overlap modes defined
/// \param[in] SLSMap                       The display SLS map information. Refer to the \ref ADLSLSMap structure for more information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_Overlap_Set (ADL_CONTEXT_HANDLE context,int iAdapterIndex, 
                                     int iSLSMapIndex, 
                                     int iNumOverlappedModes,
                                     LPADLSLSTargetOverlap lpOverlappedModes,
									 int iNumTargetOffsets,
									 LPADLSLSOffset lpTargetOffsets,
                                     ADLSLSMap SLSMap);

///
/// \brief Set SLS overlap offsets for each display index.
/// 
///
/// \platform
/// \WIN7 and above
/// \param[in] iAdapterIndex                The ADL index of the desired adapter. This function does not support -1.
/// \param[in] iSLSMapIndex                 The SLS map index to apply overlap offset.
/// \param[in] iNumOverlappedModes          the no. of overlapped modes defined. Each such mode refers to a set of targets in the array of targets
/// \param[in] lpOverlappedModes            the list of overlapped modes defined
/// \param[in] iNumTargetOffsets            the no. of targets in the list of targets
/// \param[in] lpTargetOffsets              the list of target offsets for all the overlap modes defined
/// \param[in] SLSMap                       The display SLS map information. Refer to the \ref ADLSLSMap structure for more information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks Call ADL_Flush_Driver_Data() after to persist settings on reboot.
ADL_EXTERNC int EXPOSED ADL_Display_Overlap_Set (int iAdapterIndex, 
                                     int iSLSMapIndex, 
                                     int iNumOverlappedModes,
                                     LPADLSLSTargetOverlap lpOverlappedModes,
									 int iNumTargetOffsets,
									 LPADLSLSOffset lpTargetOffsets,
                                     ADLSLSMap SLSMap);

///
/// \brief Get SLS middle mode for specific adapter.
/// 
///
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex               The ADL index of the desired adapter. This function does not support -1.
/// \param[in]  iSLSMapIndex                The SLS map index of the desired adapter.
/// \param[out] lpMiddleModeSupported		If SLS middle mode is supported or not. 1: supported; otherwise: not supported.
/// \param[out] lpMaxNumMiddleModeToSet		The maximum number of SLS middle modes is allowed to be set. 
/// \param[out] lpNumAvailableSLSMiddlemode	Pointer to a variable that will receive the number of elements in a Available SLS middle mode list.
/// \param[out] lppAvailableSLSMiddlemode	Pointer of a pointer to a variable that contains the Available SLS middle modes. the data type is \ref ADLSLSMode.
/// \param[out] lpNumCurrentSLSMiddleMode	Pointer to a variable that will receive the number of elements in a Current SLS middle mode list.
/// \param[out] lppCurrentSLSMiddleMode		Pointer of a pointer to a variable that contains the Current SLS middle modes. the data type is \ref ADLSLSMode.
/// \param[out] lpNumDefaultSLSMiddleMode	Pointer to a variable that will receive the number of elements in a Default SLS middle mode list.
/// \param[out] lppDefaultSLSMiddleMode		Pointer of a pointer to a variable that contains the Default SLS middle modes. the data type is \ref ADLSLSMode.
/// \param[in]  iOption						Specifies the layout type of SLS grid data. It is bit vector. There are two types of SLS layouts, relative to landscape (ref \ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_LANDSCAPE) and relative to current angle (ref \ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_CURRENTANGLE).
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks 1. Currently only support one current and one default middle mode.
///			 2. This API uses the memory allocation callback function provided by the user.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED	ADL2_Display_SLSMiddleMode_Get (ADL_CONTEXT_HANDLE context,int iAdapterIndex, 
											int iSLSMapIndex,
											int* lpMiddleModeSupported,
											int* lpMaxNumMiddleModeToSet,
											int* lpNumAvailableSLSMiddleMode, 
											ADLSLSMode** lppAvailableSLSMiddleMode,
											int*lpNumCurrentSLSMiddleMode,
											ADLSLSMode ** lppCurrentSLSMiddleMode,
											int*lpNumDefaultSLSMiddleMode,
											ADLSLSMode ** lppDefaultSLSMiddleMode, 
											int iOption);

///
/// \brief Get SLS middle mode for specific adapter.
/// 
///
/// \platform
/// \WIN
/// \param[in]  iAdapterIndex               The ADL index of the desired adapter. This function does not support -1.
/// \param[in]  iSLSMapIndex                The SLS map index of the desired adapter.
/// \param[out] lpMiddleModeSupported		If SLS middle mode is supported or not. 1: supported; otherwise: not supported.
/// \param[out] lpMaxNumMiddleModeToSet		The maximum number of SLS middle modes is allowed to be set. 
/// \param[out] lpNumAvailableSLSMiddlemode	Pointer to a variable that will receive the number of elements in a Available SLS middle mode list.
/// \param[out] lppAvailableSLSMiddlemode	Pointer of a pointer to a variable that contains the Available SLS middle modes. the data type is \ref ADLSLSMode.
/// \param[out] lpNumCurrentSLSMiddleMode	Pointer to a variable that will receive the number of elements in a Current SLS middle mode list.
/// \param[out] lppCurrentSLSMiddleMode		Pointer of a pointer to a variable that contains the Current SLS middle modes. the data type is \ref ADLSLSMode.
/// \param[out] lpNumDefaultSLSMiddleMode	Pointer to a variable that will receive the number of elements in a Default SLS middle mode list.
/// \param[out] lppDefaultSLSMiddleMode		Pointer of a pointer to a variable that contains the Default SLS middle modes. the data type is \ref ADLSLSMode.
/// \param[in]  iOption						Specifies the layout type of SLS grid data. It is bit vector. There are two types of SLS layouts, relative to landscape (ref \ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_LANDSCAPE) and relative to current angle (ref \ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_CURRENTANGLE).
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks 1. Currently only support one current and one default middle mode.
///			 2. This API uses the memory allocation callback function provided by the user.
ADL_EXTERNC int EXPOSED	ADL_Display_SLSMiddleMode_Get (int iAdapterIndex, 
											int iSLSMapIndex,
											int* lpMiddleModeSupported,
											int* lpMaxNumMiddleModeToSet,
											int* lpNumAvailableSLSMiddleMode, 
											ADLSLSMode** lppAvailableSLSMiddleMode,
											int*lpNumCurrentSLSMiddleMode,
											ADLSLSMode ** lppCurrentSLSMiddleMode,
											int*lpNumDefaultSLSMiddleMode,
											ADLSLSMode ** lppDefaultSLSMiddleMode, 
											int iOption);

///
/// \brief Set SLS middle mode for specific adapter.
/// 
///
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapterIndex                The ADL index of the desired adapter. This function does not support -1.
/// \param[in] iSLSMapIndex                 The SLS map index of the desired adapter.
/// \param[in] iNumCurrentSLSMiddleMode		The number of elements in a Current SLS middle mode list to be set.
/// \param[in] lpCurrentSLSMiddleMode		Pointer of a variable that contains the Current SLS middle modes. the data type is \ref ADLSLSMode.
/// \param[in] iOption						Specifies the layout type of SLS grid data. It is bit vector. There are two types of SLS layouts, relative to landscape (ref \ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_LANDSCAPE) and relative to current angle (ref \ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_CURRENTANGLE).
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks	1. Currently only support one current/default middle mode.
///				2. Driver may returns CWDDE_OK_MODE_SET_NEEDED and CWDDE_ERR_BAD_MIDDLE_MODE.
///					- for return code CWDDE_OK_MODE_SET_NEEDED, ADL needs to do Mode Set in order to make newly set middle mode replacing the old one.
///					- for return code CWDDE_ERR_BAD_MIDDLE_MODE, ADL returns ADL_FAIL.
///
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED	ADL2_Display_SLSMiddleMode_Set (ADL_CONTEXT_HANDLE context,int iAdapterIndex, 
											int iSLSMapIndex,
											int iNumCurrentSLSMiddleMode,
											ADLSLSMode * lpCurrentSLSMiddleMode, 
											int iOption);

///
/// \brief Set SLS middle mode for specific adapter.
/// 
///
/// \platform
/// \WIN
/// \param[in] iAdapterIndex                The ADL index of the desired adapter. This function does not support -1.
/// \param[in] iSLSMapIndex                 The SLS map index of the desired adapter.
/// \param[in] iNumCurrentSLSMiddleMode		The number of elements in a Current SLS middle mode list to be set.
/// \param[in] lpCurrentSLSMiddleMode		Pointer of a variable that contains the Current SLS middle modes. the data type is \ref ADLSLSMode.
/// \param[in] iOption						Specifies the layout type of SLS grid data. It is bit vector. There are two types of SLS layouts, relative to landscape (ref \ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_LANDSCAPE) and relative to current angle (ref \ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_CURRENTANGLE).
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks	1. Currently only support one current/default middle mode.
///				2. Driver may returns CWDDE_OK_MODE_SET_NEEDED and CWDDE_ERR_BAD_MIDDLE_MODE.
///					- for return code CWDDE_OK_MODE_SET_NEEDED, ADL needs to do Mode Set in order to make newly set middle mode replacing the old one.
///					- for return code CWDDE_ERR_BAD_MIDDLE_MODE, ADL returns ADL_FAIL.
///
ADL_EXTERNC int EXPOSED	ADL_Display_SLSMiddleMode_Set (int iAdapterIndex, 
											int iSLSMapIndex,
											int iNumCurrentSLSMiddleMode,
											ADLSLSMode * lpCurrentSLSMiddleMode, 
											int iOption);

///
/// \brief Function to set the Registry key "UnsupportedMonitorModesAllowed" when 10bit Pixel format is enabled from workstation aspect.
/// 
/// This function sets the above registry key to 1.
/// \platform
/// \Win7
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \return If the function is successful, \ref ADL_OK is returned. If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned.\n
/// If the function is not supported, \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Workstation_EnableUnsupportedDisplayModes(ADL_CONTEXT_HANDLE context, int iAdapterIndex);

///
/// \brief Function to set the Registry key "UnsupportedMonitorModesAllowed" when 10bit Pixel format is enabled from workstation aspect.
/// 
/// This function sets the above registry key to 1.
/// \platform
/// \Win7
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \return If the function is successful, \ref ADL_OK is returned. If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned.\n
/// If the function is not supported, \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
ADL_EXTERNC int EXPOSED ADL_Workstation_EnableUnsupportedDisplayModes( int iAdapterIndex);

// @} 

///
/// \brief Function to Get an SLS records from the driver database.
/// 
/// This function to Get an SLS records from the driver database.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex  The ADL index of the desired adapter. This function does not support -1.
/// \param[in]   displayID The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpNumOfRecords gets the number of SLS Grids for the given adapter and display
/// \param[out]  lppGridIDList gets the SLS Grid IDs for the given adapter and display
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_SLSRecords_Get (ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLDisplayID displayID, int *lpNumOfRecords, int **lppGridIDList);

///
/// \brief Function to Get an SLS records from the driver database.
/// 
/// This function to Get an SLS records from the driver database.
/// \platform
/// \WIN
/// \param[in]   iAdapterIndex  The ADL index of the desired adapter. This function does not support -1.
/// \param[in]   displayID The desired display index. It can be retrieved from the ADLDisplayInfo data structure.
/// \param[out]  lpNumOfRecords gets the number of SLS Grids for the given adapter and display
/// \param[out]  lppGridIDList gets the SLS Grid IDs for the given adapter and display
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_SLSRecords_Get (int iAdapterIndex, ADLDisplayID displayID, int *lpNumOfRecords, int **lppGridIDList);



/// \brief ADL local interface. Retrieves GPU information of any AMD and non-AMD displays cloned for given GPU  or all OS-known GPUs.
/// 
/// This function retrieves information for passed adapter or -1 for all GPUs.     
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter(one of GPU adapter index) or -1 if all GPUs are desired. For A+I system, please pass -1 only.
/// \param[out] lpHasAMDAndNonAMDDIsplays pointer to an integer.  It is 1 if there is any AMd and non-AMD displays cloned in the GPU or the system
/// \platform 
/// \WIn7 or above
/// \return If the function valid, the return value is 1. Otherwise it is 0. 
/// \remarks This API uses the memory allocation callback function provided by the user.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_AMDAndNonAMDDIsplayClone_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int *lpHasAMDAndNonAMDDIsplays);



/// \brief ADL local interface. Retrieves GPU information of special clone types for given GPU  or all OS-known GPUs.
/// 
/// This function retrieves special clone types information for passed adapter or -1 for all GPUs. Currently, cross-GPU-clone and corss-GPU-clone-AMDWithNonAMD    
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter(one of GPU adapter index) or -1 if all GPUs are desired. For A+I system, please pass -1 only.
/// \param[out] lpCloneTypes pointer to an integer.  It is bit vector.  If the first bit is 1, the clone is cross-GPU-clone. It is may AMD with AMD or AMD with non-AMD.
///             If The second bit is 1, the clone is cross-GPU-clone, but with non-AMD displays. It is available in WIN10 and WIN 8.1 PX A+I system
/// \platform 
/// \WIn7 or above
/// \return If the function valid, the return value is 1. Otherwise it is 0. 
/// \remarks if iAdapterIndex is not -1 and if there is one of cross-GPU clones found in the GPU, then cross-GPU clone is true
/// \remarks if the iAdapertIndex is -1 and if there is one of cross-GPU clones found in the system, then cross-GPU clone is true

ADL_EXTERNC int EXPOSED ADL2_Adapter_CloneTypes_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int *lpCloneTypes);

/// \brief ADL local interface. Disable all cross-GPU clone for given GPU  or all OS-known GPUs.
/// 
/// This function disables the cross-GPU clone for passed adapter(GPU) or -1 for all GPUs.     
/// \param[in]  context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter(one of GPU adapter index) or -1 if all GPUs are desired. For A+I system, please pass -1 only.
/// \param[in]  iOptions  is an integer, which decides the displays from other GPUs will be disabled or changed to extended mode. Currently, the API only supports the "disabled". This filed is just for extension for future useage.
/// \platform 
/// \WIn7 or above
/// \return If the function valid, the return value is 1. Otherwise it is 0. 
/// \remarks iOption support only 0 currently.
ADL_EXTERNC int EXPOSED ADL2_Adapter_CrossGPUClone_Disable(ADL_CONTEXT_HANDLE context, int iAdapterIndex,int iOptions );



#endif /* DISPLAYSMANAGER_H_ */

