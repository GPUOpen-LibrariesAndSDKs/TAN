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

/// \file powerxpress.h
/// \brief Contains all AMD PowerExpress-related functions exposed by ADL for \ALL platforms.
///
/// All functions found in this file can be used as a reference to ensure
/// the specified function pointers can be used by the appropriate runtime
/// dynamic library loaders. This header file is for AMD internal use only.

#ifndef AMD_PX_H_
#define AMD_PX_H_

#ifndef ADL_EXTERNC
#ifdef __cplusplus
#define ADL_EXTERNC extern "C"
#else
#define ADL_EXTERNC
#endif
#endif

/// \addtogroup POWERX
// @{

///
///\brief Function to retrieve the PowerXpress version.
///
/// This function retrieves the PowerXpress version.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpVersion The pointer to the value storing the retrieved version.
/// \return If the system is PowerXpress capable, the function returns ADL_OK and lpVersion is always set to \ref ADL_DL_POWERXPRESS_VERSION
/// \return If the system is not PowerXpress capable, the function returns a value different from \ref ADL_OK
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_PowerXpressVersion_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int *lpVersion);

///
///\brief Function to retrieve the PowerXpress version.
///
/// This function retrieves the PowerXpress version.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpVersion The pointer to the value storing the retrieved version.
/// \return If the system is PowerXpress capable, the function returns ADL_OK and lpVersion is always set to \ref ADL_DL_POWERXPRESS_VERSION
/// \return If the system is not PowerXpress capable, the function returns a value different from \ref ADL_OK
ADL_EXTERNC int EXPOSED ADL_Display_PowerXpressVersion_Get(int iAdapterIndex, int *lpVersion);

///
///\brief Function to get the active GPU in PowerXpress.
///
/// This function retrieves the active GPU in PowerXpress.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpActiveGPU The pointer to the value storing the active GPU in PowerXpress: \ref ADL_DL_POWERXPRESS_GPU_INTEGRATED or \ref ADL_DL_POWERXPRESS_GPU_DISCRETE
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_PowerXpressActiveGPU_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int *lpActiveGPU);

///
///\brief Function to get the active GPU in PowerXpress.
///
/// This function retrieves the active GPU in PowerXpress.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpActiveGPU The pointer to the value storing the active GPU in PowerXpress: \ref ADL_DL_POWERXPRESS_GPU_INTEGRATED or \ref ADL_DL_POWERXPRESS_GPU_DISCRETE
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_PowerXpressActiveGPU_Get(int iAdapterIndex, int *lpActiveGPU);

///
///\brief Function to set the active GPU in PowerXpress.
///
/// This function sets the active GPU in PowerXpress.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   iActiveGPU The value of the desired active GPU in PowerXpress: \ref ADL_DL_POWERXPRESS_GPU_INTEGRATED or \ref ADL_DL_POWERXPRESS_GPU_DISCRETE
/// \param[out]  lpOperationResult The pointer to the retrieved result of the operation.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_PowerXpressActiveGPU_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iActiveGPU, int * lpOperationResult);

///
///\brief Function to set the active GPU in PowerXpress.
///
/// This function sets the active GPU in PowerXpress.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   iActiveGPU The value of the desired active GPU in PowerXpress: \ref ADL_DL_POWERXPRESS_GPU_INTEGRATED or \ref ADL_DL_POWERXPRESS_GPU_DISCRETE
/// \param[out]  lpOperationResult The pointer to the retrieved result of the operation.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_PowerXpressActiveGPU_Set(int iAdapterIndex, int iActiveGPU, int * lpOperationResult);

///
///\brief Function to retrieve the auto switch configuration setting for PowerXpress.  This is not supported when DPPE state switching is configured.
///
/// This function retrieves the auto switch configuration setting for PowerXpress.  This is not supported when DPPE state switching is configured.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpAutoSwitchOnACDCEvent The pointer to the switch configuration for ACDC. The returned value could be 0 or 1
/// \param[out]  lpAutoSwitchOnDCACEvent The pointer to the switch configuration for DCAC. The  returned value could be 0 or 1
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_PowerXpress_AutoSwitchConfig_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int *lpAutoSwitchOnACDCEvent, int *lpAutoSwitchOnDCACEvent);

///
///\brief Function to retrieve the auto switch configuration setting for PowerXpress.  This is not supported when DPPE state switching is configured.
///
/// This function retrieves the auto switch configuration setting for PowerXpress.  This is not supported when DPPE state switching is configured.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpAutoSwitchOnACDCEvent The pointer to the switch configuration for ACDC. The returned value could be 0 or 1
/// \param[out]  lpAutoSwitchOnDCACEvent The pointer to the switch configuration for DCAC. The  returned value could be 0 or 1
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_PowerXpress_AutoSwitchConfig_Get(int iAdapterIndex, int *lpAutoSwitchOnACDCEvent, int *lpAutoSwitchOnDCACEvent);

///
///\brief Function to set the auto switch configuration setting for PowerXpress.  This is not supported when DPPE state switching is configured.
///
/// This function sets the auto switch configuration setting for PowerXpress.  This is not supported when DPPE state switching is configured.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   iAutoSwitchOnACDCEvent The pointer to the value used to set the switch configuration for ACDC. Set value 0 or 1
/// \param[in]   iAutoSwitchOnDCACEvent The pointer to the value used to set the switch configuration for DCAC. Set value 0 or 1
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Display_PowerXpress_AutoSwitchConfig_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iAutoSwitchOnACDCEvent, int iAutoSwitchOnDCACEvent);

///
///\brief Function to set the auto switch configuration setting for PowerXpress.  This is not supported when DPPE state switching is configured.
///
/// This function sets the auto switch configuration setting for PowerXpress.  This is not supported when DPPE state switching is configured.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   iAutoSwitchOnACDCEvent The pointer to the value used to set the switch configuration for ACDC. Set value 0 or 1
/// \param[in]   iAutoSwitchOnDCACEvent The pointer to the value used to set the switch configuration for DCAC. Set value 0 or 1
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Display_PowerXpress_AutoSwitchConfig_Set(int iAdapterIndex, int iAutoSwitchOnACDCEvent, int iAutoSwitchOnDCACEvent);

///
/// \brief This function gets the PowerXpress configuration Caps.
///  
/// This function gets the PowerXpress configuration Caps.
/// \platform 
/// \WIN7
/// \LINUX
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpPXConfigCaps The pointer to the PowerXpress config Caps returned: \ref ADLPXConfigCaps
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_PowerXpress_Config_Caps(ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLPXConfigCaps *lpPXConfigCaps);

///
/// \brief This function gets the PowerXpress configuration Caps.
///  
/// This function gets the PowerXpress configuration Caps.
/// \platform 
/// \WIN7
/// \LINUX
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpPXConfigCaps The pointer to the PowerXpress config Caps returned: \ref ADLPXConfigCaps
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_PowerXpress_Config_Caps(int iAdapterIndex, ADLPXConfigCaps *lpPXConfigCaps);

///
/// \brief This function gets the PowerXpress scheme.
///
/// This function gets the PowerXpress scheme.
/// \platform 
/// \WIN7
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex				The ADL index handle of the desired adapter.
/// \param[out]  lpPXSchemeRange			The pointer to the scheme range to be returned: \ref ADLPXScheme
/// \param[out]  lpPXSchemeCurrentState		The pointer to the current scheme to be returned: \ref ADLPXScheme
/// \param[out]  lpPXSchemeDefaultState		The pointer to the default scheme to be returned: \ref ADLPXScheme
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_PowerXpress_Scheme_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLPXScheme *lpPXSchemeRange, 
									   ADLPXScheme *lpPXSchemeCurrentState, ADLPXScheme *lpPXSchemeDefaultState);

///
/// \brief This function gets the PowerXpress scheme.
///
/// This function gets the PowerXpress scheme.
/// \platform 
/// \WIN7
/// \param[in]   iAdapterIndex				The ADL index handle of the desired adapter.
/// \param[out]  lpPXSchemeRange			The pointer to the scheme range to be returned: \ref ADLPXScheme
/// \param[out]  lpPXSchemeCurrentState		The pointer to the current scheme to be returned: \ref ADLPXScheme
/// \param[out]  lpPXSchemeDefaultState		The pointer to the default scheme to be returned: \ref ADLPXScheme
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_PowerXpress_Scheme_Get(int iAdapterIndex, ADLPXScheme *lpPXSchemeRange, 
									   ADLPXScheme *lpPXSchemeCurrentState, ADLPXScheme *lpPXSchemeDefaultState);

///
/// \brief This function sets the PowerXpress scheme.
///
/// This function sets the PowerXpress scheme.
/// \platform 
/// \WIN7
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   scheme The PX scheme to set: \ref ADLPXScheme
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_PowerXpress_Scheme_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLPXScheme scheme);

///
/// \brief This function sets the PowerXpress scheme.
///
/// This function sets the PowerXpress scheme.
/// \platform 
/// \WIN7
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   scheme The PX scheme to set: \ref ADLPXScheme
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_PowerXpress_Scheme_Set(int iAdapterIndex, ADLPXScheme scheme);


///
/// \brief  This function gets ancillary GPUs
///
/// In Power Express configuration system, the master GPU(Integrated GPU)  can have several linked GPU which can be removed by the driver.
/// These removable GPU are  called ancillary devices. They are usually discrete GPUs in Power Express system
/// \platform 
/// \WIN7
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpNumberOfAncillaryDevices Pointer to a variable that indicates the number of ancillary devices.
/// \param[out]  lppRemovableDevices Pointer of a pointer to a variable with type \ref ADLBdf, which contains a list of ancillary devices  .
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_PowerXpress_AncillaryDevices_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int *lpNumberOfAncillaryDevices, ADLBdf  **lppAncillaryDevices);

///
/// \brief  This function gets ancillary GPUs
///
/// In Power Express configuration system, the master GPU(Integrated GPU)  can have several linked GPU which can be removed by the driver.
/// These removable GPU are  called ancillary devices. They are usually discrete GPUs in Power Express system
/// \platform 
/// \WIN7
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpNumberOfAncillaryDevices Pointer to a variable that indicates the number of ancillary devices.
/// \param[out]  lppRemovableDevices Pointer of a pointer to a variable with type \ref ADLBdf, which contains a list of ancillary devices  .
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_PowerXpress_AncillaryDevices_Get(int iAdapterIndex, int *lpNumberOfAncillaryDevices, ADLBdf  **lppAncillaryDevices);

// @}

#endif /* AMD_PX_H_ */
