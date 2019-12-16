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

/// \file lnx.h
/// \brief Contains all Linux-related functions exposed by ADL for \LNX platforms.
///
/// This file contains all Linux-related ADL function exposed by ADL for Linux platforms.
/// All functions in this file can be used as a reference to ensure the
/// specific function pointers can be used by the appropriate runtime
/// dynamic library loaders.

#ifndef __LNX_H__
#define __LNX_H__


#include "adl_structures.h"
//#include "amd_only/amd_lnx.h"

#ifndef ADL_EXTERNC
#ifdef __cplusplus
#define ADL_EXTERNC extern "C"
#else
#define ADL_EXTERNC
#endif
#endif

/// \addtogroup LINUX
// @{

///
///\brief Function to get the Desktop Configuration.
/// 
/// This function retreives current the desktop configuration information.
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpDesktopConfig The pointer to the retrieved desktop configuration information.\n\ref define_desktop_config
/// \platform 
/// \LNX
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \deprecated This API has been deprecated because it was only used for RandR 1.1 (Red Hat 5.x) distributions which is now not supported.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_DesktopConfig_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int* lpDesktopConfig);

///
///\brief Function to get the Desktop Configuration.
/// 
/// This function retreives current the desktop configuration information.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpDesktopConfig The pointer to the retrieved desktop configuration information.\n\ref define_desktop_config
/// \platform 
/// \LNX
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \deprecated This API has been deprecated because it was only used for RandR 1.1 (Red Hat 5.x) distributions which is now not supported.
ADL_EXTERNC int EXPOSED ADL_DesktopConfig_Get(int iAdapterIndex, int* lpDesktopConfig);

///
///\brief Function to set the Desktop Configuration.
/// 
/// This function sets the current desktop configuration information.
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDesktopConfig The desired Desktop Configuration information.\n\ref define_desktop_config
/// \platform 
/// \LNX
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \deprecated This API has been deprecated because it was only used for RandR 1.1 (Red Hat 5.x) distributions which is now not supported.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_DesktopConfig_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDesktopConfig);

///
///\brief Function to set the Desktop Configuration.
/// 
/// This function sets the current desktop configuration information.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDesktopConfig The desired Desktop Configuration information.\n\ref define_desktop_config
/// \platform 
/// \LNX
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \deprecated This API has been deprecated because it was only used for RandR 1.1 (Red Hat 5.x) distributions which is now not supported.
ADL_EXTERNC int EXPOSED ADL_DesktopConfig_Set(int iAdapterIndex, int iDesktopConfig);

///
/// \brief  Function to retrieve the number of enabled displays.
/// 
/// This function retrieves the current number of enabled displays.
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpNumberOfDisplays The pointer to the number of enabled displays.
/// \platform
/// \LNX
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \deprecated This API has been deprecated because it was only used for RandR 1.1 (Red Hat 5.x) distributions which is now not supported.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_NumberOfDisplayEnable_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int *lpNumberOfDisplays);

///
/// \brief  Function to retrieve the number of enabled displays.
/// 
/// This function retrieves the current number of enabled displays.
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpNumberOfDisplays The pointer to the number of enabled displays.
/// \platform
/// \LNX
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \deprecated This API has been deprecated because it was only used for RandR 1.1 (Red Hat 5.x) distributions which is now not supported.
ADL_EXTERNC int EXPOSED ADL_NumberOfDisplayEnable_Get(int iAdapterIndex, int *lpNumberOfDisplays);

///
/// \brief Function to dynamically enable displays on a GPU.
/// 
/// This function dynamically enables displays on a GPU.  Supported on randr 1.1 distributions. 
/// This function is not supported on randr 1.2 and newer distributions. 
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  lpDisplayIndexList The pointer to the list of display indexes that will be enabled. All others will be disabled.
/// \param[in]	iDisplayListSize The size of the list.
/// \param[in]  bPersistOnly Non-zero value means that settings will not take effect until system restart
/// \platform 
/// \LNX
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \deprecated This API has been deprecated because it was only used for RandR 1.1 (Red Hat 5.x) distributions which is now not supported.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_DisplayEnable_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int *lpDisplayIndexList, int iDisplayListSize, int bPersistOnly);

///
/// \brief Function to dynamically enable displays on a GPU.
/// 
/// This function dynamically enables displays on a GPU.  Supported on randr 1.1 distributions. 
/// This function is not supported on randr 1.2 and newer distributions. 
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  lpDisplayIndexList The pointer to the list of display indexes that will be enabled. All others will be disabled.
/// \param[in]	iDisplayListSize The size of the list.
/// \param[in]  bPersistOnly Non-zero value means that settings will not take effect until system restart
/// \platform 
/// \LNX
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \deprecated This API has been deprecated because it was only used for RandR 1.1 (Red Hat 5.x) distributions which is now not supported.
ADL_EXTERNC int EXPOSED ADL_DisplayEnable_Set(int iAdapterIndex, int *lpDisplayIndexList, int iDisplayListSize, int bPersistOnly);

///
///\brief Function to set the desktop configuration.
/// 
/// This function sets the desktop configuration.
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index.
/// \param[in]	iDisplayControllerIndex The controller index. A field from ADLDisplayInfo structure
/// \param[in]  iShow The instruction to show the display number. Enter 0 to show and 1 to hide.
/// \param[in]  iDisplayNum The number to display on screen.
/// \param[in]	iPosX The X position on screen to display index in % of screen.
/// \param[in]	iPosY The Y position on screen to display index in % of screen.
/// \platform 
/// \LNX
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_IdentifyDisplay(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, int iDisplayControllerIndex, int iShow, int iDisplayNum, int iPosX, int iPosY);

///
///\brief Function to set the desktop configuration.
/// 
/// This function sets the desktop configuration.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index.
/// \param[in]	iDisplayControllerIndex The controller index. A field from ADLDisplayInfo structure
/// \param[in]  iShow The instruction to show the display number. Enter 0 to show and 1 to hide.
/// \param[in]  iDisplayNum The number to display on screen.
/// \param[in]	iPosX The X position on screen to display index in % of screen.
/// \param[in]	iPosY The Y position on screen to display index in % of screen.
/// \platform 
/// \LNX
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Display_IdentifyDisplay(int iAdapterIndex, int iDisplayIndex, int iDisplayControllerIndex, int iShow, int iDisplayNum, int iPosX, int iPosY);

///
/// \brief Function to set the current gamma value for a LUT (controller).
/// 
/// This function sets the gamma value of a specified display index (controller)
/// Note:  This replaces ADL_Controller_Color_Set
/// \platform
/// \LNX
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index.
/// \param[in]  adlGamma The Gamma information structure.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code.   \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_LUTColor_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, ADLGamma adlGamma);

///
/// \brief Function to set the current gamma value for a LUT (controller).
/// 
/// This function sets the gamma value of a specified display index (controller)
/// Note:  This replaces ADL_Controller_Color_Set
/// \platform
/// \LNX
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index.
/// \param[in]  adlGamma The Gamma information structure.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code.   \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_LUTColor_Set(int iAdapterIndex, int iDisplayIndex, ADLGamma adlGamma);

///
/// \brief Function to get the current value of gamma for a LUT (controller).
/// 
/// This function retrieves the current gamma value of the specified display index.
/// Note:  This replaces ADL_Controller_Color_Get
/// \platform
/// \LNX
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index.
/// \param[out]  lpGammaCurrent The pointer to the retrieved current gamma information structure.
/// \param[out]  lpGammaDefault The pointer to the retrieved default gamma information structure.
/// \param[out]  lpGammaMin The pointer to the retrieved minimum gamma information structure.
/// \param[out]  lpGammaMax The pointer to the retrieved maximum gamma information structure.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code.   \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_LUTColor_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, ADLGamma *lpGammaCurrent, ADLGamma *lpGammaDefault, ADLGamma *lpGammaMin, ADLGamma *lpGammaMax);

///
/// \brief Function to get the current value of gamma for a LUT (controller).
/// 
/// This function retrieves the current gamma value of the specified display index.
/// Note:  This replaces ADL_Controller_Color_Get
/// \platform
/// \LNX
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index.
/// \param[out]  lpGammaCurrent The pointer to the retrieved current gamma information structure.
/// \param[out]  lpGammaDefault The pointer to the retrieved default gamma information structure.
/// \param[out]  lpGammaMin The pointer to the retrieved minimum gamma information structure.
/// \param[out]  lpGammaMax The pointer to the retrieved maximum gamma information structure.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code.   \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_LUTColor_Get(int iAdapterIndex, int iDisplayIndex, ADLGamma *lpGammaCurrent, ADLGamma *lpGammaDefault, ADLGamma *lpGammaMin, ADLGamma *lpGammaMax);

///
/// \brief Function to retrieve all X Screen information for all OS-known adapters.
/// 
/// This function retrieves the X Screen information of all OS-known adapters on a Linux system. OS-known adapters can include adapters that currently exist in the system (logical adapters)    
/// as well as ones that no longer exist in the system but are still recognized by the OS.
/// \platform
/// \LNX
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iInputSize The size of lpInfo buffer.
/// \param[out] lpXScreenInfo The pointer to the buffer storing the retrieved X Screen information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results  
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_XScreenInfo_Get(ADL_CONTEXT_HANDLE context,LPXScreenInfo lpXScreenInfo, int iInputSize);

///
/// \brief Function to retrieve all X Screen information for all OS-known adapters.
/// 
/// This function retrieves the X Screen information of all OS-known adapters on a Linux system. OS-known adapters can include adapters that currently exist in the system (logical adapters)    
/// as well as ones that no longer exist in the system but are still recognized by the OS.
/// \platform
/// \LNX
/// \param[in]  iInputSize The size of lpInfo buffer.
/// \param[out] lpXScreenInfo The pointer to the buffer storing the retrieved X Screen information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results  
ADL_EXTERNC int EXPOSED ADL_Adapter_XScreenInfo_Get(LPXScreenInfo lpXScreenInfo, int iInputSize);

///
/// \brief Function to retrieve the name of the Xrandr display.
/// 
/// This function retrieves the Xrandr display name based on a specified adapter and display.
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The ADL index handle of the desired display.
/// \param[out]  lpXrandrDisplayName The pointer to the buffer storing the retrieved Xrandr display name.
/// \param[in]  iBuffSize The size of the lpXrandrDisplayName buffer.
/// \platform 
/// \LNX
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_XrandrDisplayName_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDisplayIndex, char* lpXrandrDisplayName, int iBuffSize);

///
/// \brief Function to retrieve the name of the Xrandr display.
/// 
/// This function retrieves the Xrandr display name based on a specified adapter and display.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The ADL index handle of the desired display.
/// \param[out]  lpXrandrDisplayName The pointer to the buffer storing the retrieved Xrandr display name.
/// \param[in]  iBuffSize The size of the lpXrandrDisplayName buffer.
/// \platform 
/// \LNX
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Display_XrandrDisplayName_Get(int iAdapterIndex, int iDisplayIndex, char* lpXrandrDisplayName, int iBuffSize);



///
/// \brief Set the requested tear free desktop setting.
///
/// The tear free desktop setting is applied to all GPUs on the system.
///
/// Tear free desktop reduces tearing in everything shown on the screen except full
/// screen OGL applications. It double buffers the graphics adapter output ensuring that the
/// image being drawn is not changed while the display is updating.
///
/// \platform
/// \LNX
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapter Ignored. May be used in the future.
/// \param[in] iRequested The new requested tear free state. \ref ADL_TRUE to request it on and \ref ADL_FALSE to request it off.
/// \param[out] pStatus The current status of tear free \ref define_adapter_tear_free_status
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_Tear_Free_Set(ADL_CONTEXT_HANDLE context,int iAdapter, int iRequested, int* pStatus);

///
/// \brief Set the requested tear free desktop setting.
///
/// The tear free desktop setting is applied to all GPUs on the system.
///
/// Tear free desktop reduces tearing in everything shown on the screen except full
/// screen OGL applications. It double buffers the graphics adapter output ensuring that the
/// image being drawn is not changed while the display is updating.
///
/// \platform
/// \LNX
/// \param[in] iAdapter Ignored. May be used in the future.
/// \param[in] iRequested The new requested tear free state. \ref ADL_TRUE to request it on and \ref ADL_FALSE to request it off.
/// \param[out] pStatus The current status of tear free \ref define_adapter_tear_free_status
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Adapter_Tear_Free_Set(int iAdapter, int iRequested, int* pStatus);

///
/// \brief Get the requested tear free desktop setting and current status.
///
/// Gets the requested setting and current status of all the GPUs on the system.
///
/// Tear free desktop reduces tearing in everything shown on the screen except full
/// screen OGL applications. It double buffers the graphics adapter output ensuring that the
/// image being drawn is not changed while a display is updating.
///
/// \platform
/// \LNX
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapter Ignored. May be used in the future.
/// \param[out] pDefault The default requested value or NULL.
/// \param[out] pRequested The requested tear free state. \ref ADL_TRUE if it is requested on or \ref ADL_FALSE if it requested off.
/// \param[out] pStatus The current status of tear free \ref define_adapter_tear_free_status
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_Tear_Free_Get(ADL_CONTEXT_HANDLE context,int iAdapter, int * pDefault, int * pRequested, int * pStatus);

///
/// \brief Get the requested tear free desktop setting and current status.
///
/// Gets the requested setting and current status of all the GPUs on the system.
///
/// Tear free desktop reduces tearing in everything shown on the screen except full
/// screen OGL applications. It double buffers the graphics adapter output ensuring that the
/// image being drawn is not changed while a display is updating.
///
/// \platform
/// \LNX
/// \param[in] iAdapter Ignored. May be used in the future.
/// \param[out] pDefault The default requested value or NULL.
/// \param[out] pRequested The requested tear free state. \ref ADL_TRUE if it is requested on or \ref ADL_FALSE if it requested off.
/// \param[out] pStatus The current status of tear free \ref define_adapter_tear_free_status
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Adapter_Tear_Free_Get(int iAdapter, int * pDefault, int * pRequested, int * pStatus);

///
/// \brief Functions to retreive Tear Free setting capabilities of the system.
///
/// This function determines whether the current system is capable of applying the tear free desktop settings.
///
/// Tear free desktop reduces tearing in everything shown on the screen except full
/// screen OGL applications. It double buffers the graphics adapter output ensuring that the
/// image being drawn is not changed while a display is updating.
///
/// \platform
/// \LNX
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapterIndex Ignored. May be used in the future.
/// \param[out] lpTFCap Is either ADL_TRUE or ADL_FALSE to represent if the system is capable of tear free setting or not.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL2_Adapter_Tear_Free_Cap(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int * lpTFCap);
// @}

#endif /*__LNX_H__ */
