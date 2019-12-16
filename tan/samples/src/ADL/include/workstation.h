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

/// \file workstation.h
/// \brief Contains all Workstation exposed functions by ADL for \ALL platforms.
///
/// This file contains all Workstation exposed ADL function for \ALL platforms.
/// All functions found in this file can be used as a reference to ensure
/// the appropriate function pointers to be used by the appropriate runtime
/// dynamic library loaders.

#ifndef WORKSTATION_H_
#define WORKSTATION_H_

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

/// \addtogroup WORKSTATION
// @{

///
/// \brief Function to retrieve current workstation capabilities.
/// 
/// This function retrieves information regarding the capabilities of a specified workstation card.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpValidBits The pointer to a bit-oriented value. If a bit is set to 1, then the corresponding bit in lpCaps is valid. If a bit is set to 0, then the corresponding bit in lpCaps should be ignored.
/// \param[out] lpCaps The pointer to a bit-oriented value representing a specific workstation capability.  \ref define_ws_caps
/// \return If the function is successful, \ref ADL_OK is returned. If lpValidBits and/or lpCaps is NULL, \ref ADL_ERR_NULL_POINTER is returned.\n
/// If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned. If the function is not supported, \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Workstation_Caps(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int *lpValidBits, int *lpCaps);

///
/// \brief Function to retrieve current workstation capabilities.
/// 
/// This function retrieves information regarding the capabilities of a specified workstation card.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpValidBits The pointer to a bit-oriented value. If a bit is set to 1, then the corresponding bit in lpCaps is valid. If a bit is set to 0, then the corresponding bit in lpCaps should be ignored.
/// \param[out] lpCaps The pointer to a bit-oriented value representing a specific workstation capability.  \ref define_ws_caps
/// \return If the function is successful, \ref ADL_OK is returned. If lpValidBits and/or lpCaps is NULL, \ref ADL_ERR_NULL_POINTER is returned.\n
/// If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned. If the function is not supported, \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
ADL_EXTERNC int EXPOSED ADL_Workstation_Caps(int iAdapterIndex, int *lpValidBits, int *lpCaps);

///
/// \brief Function to retreive the worstation stereo mode.
/// 
/// This function retrieves the current and default workstation stereo mode.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpDefState The pointer to the default stereo mode. \ref define_ws_caps
/// \param[out] lpCurState The pointer to the current stereo mode. \ref define_ws_caps
/// \return If the function is successful, \ref ADL_OK is returned. If lpDefState and/or lpCurState are NULL, \ref ADL_ERR_NULL_POINTER is returned.\n
/// If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned. If the function is not supported, \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Workstation_Stereo_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int *lpDefState, int *lpCurState);

///
/// \brief Function to retreive the worstation stereo mode.
/// 
/// This function retrieves the current and default workstation stereo mode.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpDefState The pointer to the default stereo mode. \ref define_ws_caps
/// \param[out] lpCurState The pointer to the current stereo mode. \ref define_ws_caps
/// \return If the function is successful, \ref ADL_OK is returned. If lpDefState and/or lpCurState are NULL, \ref ADL_ERR_NULL_POINTER is returned.\n
/// If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned. If the function is not supported, \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
ADL_EXTERNC int EXPOSED ADL_Workstation_Stereo_Get( int iAdapterIndex, int *lpDefState, int *lpCurState);

///
/// \brief Function to set the workstation stereo mode.
/// 
/// This function sets the current workstation stereo mode.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in] iCurState The desired workstation stereo mode. \ref define_ws_caps
/// \return If the function is successful, \ref ADL_OK is returned. If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned.\n
/// If the function is not supported, \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Workstation_Stereo_Set(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iCurState);

///
/// \brief Function to set the workstation stereo mode.
/// 
/// This function sets the current workstation stereo mode.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in] iCurState The desired workstation stereo mode. \ref define_ws_caps
/// \return If the function is successful, \ref ADL_OK is returned. If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned.\n
/// If the function is not supported, \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
ADL_EXTERNC int EXPOSED ADL_Workstation_Stereo_Set( int iAdapterIndex, int iCurState);



///
/// \brief Function to retrieve the number of GL-Sync connectors on an adapter.
/// 
/// This function retrieves the number of GL-Sync connectors that are available on a specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpNumOfGLSyncConnectors The pointer to the retrieved number of GL-Sync connectors on the specified adapter.
/// \return If the function is successful, \ref ADL_OK is returned. If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned.\n
/// If the function is not supported, \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Workstation_AdapterNumOfGLSyncConnectors_Get (ADL_CONTEXT_HANDLE context,int iAdapterIndex,
															  int* lpNumOfGLSyncConnectors);

///
/// \brief Function to retrieve the number of GL-Sync connectors on an adapter.
/// 
/// This function retrieves the number of GL-Sync connectors that are available on a specified adapter.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpNumOfGLSyncConnectors The pointer to the retrieved number of GL-Sync connectors on the specified adapter.
/// \return If the function is successful, \ref ADL_OK is returned. If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned.\n
/// If the function is not supported, \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
ADL_EXTERNC int EXPOSED ADL_Workstation_AdapterNumOfGLSyncConnectors_Get (int iAdapterIndex,
															  int* lpNumOfGLSyncConnectors);


///
/// \brief Function to determine whether or not a display is capable of Genlock functionality.
/// 
/// This function determines whether a specified display can be used for Framelock/Genlock.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index, which can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpCanGenlock The pointer to the retrieved Genlock capabilities. 1: the display is Genlock-capable; 0: the display is not Genlock-capable.
/// \return If the function is successful, \ref ADL_OK is returned. If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned.\n
/// If the function is not supported, \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Workstation_DisplayGenlockCapable_Get (ADL_CONTEXT_HANDLE context,int iAdapterIndex,
													   int iDisplayIndex,
													   int* lpCanGenlock);

///
/// \brief Function to determine whether or not a display is capable of Genlock functionality.
/// 
/// This function determines whether a specified display can be used for Framelock/Genlock.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index, which can be retrieved from the ADLDisplayInfo data structure.
/// \param[out] lpCanGenlock The pointer to the retrieved Genlock capabilities. 1: the display is Genlock-capable; 0: the display is not Genlock-capable.
/// \return If the function is successful, \ref ADL_OK is returned. If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned.\n
/// If the function is not supported, \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
ADL_EXTERNC int EXPOSED ADL_Workstation_DisplayGenlockCapable_Get (int iAdapterIndex,
													   int iDisplayIndex,
													   int* lpCanGenlock);


///
/// \brief Function to detect the GL-Sync module.
/// 
/// This function detects the GL-Sync module attached to a specified adapter and GL-Sync connector.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iGlSyncConnector The index of the desired GL-Sync connector. The index can range from 0 to NumOfGLSyncConnectors - 1.
/// \param[out] lpGlSyncModuleID The pointer to the retrieved GL-Sync module information. If no GL-Sync module is attached, lpGlSyncModuleID->ulModuleID is set to 0.
/// \return If the function is successful, \ref ADL_OK is returned. If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned.\n
/// If the function is not supported, \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Workstation_GLSyncModuleDetect_Get (ADL_CONTEXT_HANDLE context,int iAdapterIndex,
												   int iGlSyncConnector,
												   ADLGLSyncModuleID* lpGlSyncModuleID);

///
/// \brief Function to detect the GL-Sync module.
/// 
/// This function detects the GL-Sync module attached to a specified adapter and GL-Sync connector.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iGlSyncConnector The index of the desired GL-Sync connector. The index can range from 0 to NumOfGLSyncConnectors - 1.
/// \param[out] lpGlSyncModuleID The pointer to the retrieved GL-Sync module information. If no GL-Sync module is attached, lpGlSyncModuleID->ulModuleID is set to 0.
/// \return If the function is successful, \ref ADL_OK is returned. If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned.\n
/// If the function is not supported, \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
ADL_EXTERNC int EXPOSED ADL_Workstation_GLSyncModuleDetect_Get (int iAdapterIndex,
												   int iGlSyncConnector,
												   ADLGLSyncModuleID* lpGlSyncModuleID);

///
/// \brief Function to describe the GL-Sync module caps.
/// 
/// This function describes the hardware capabilities of the GL-Sync module attached to a specific adapter and GL-Sync connector.\n
/// This API uses the Caller's Memory Allocation Callback Function to allocate memory pointed by ppGlSyncPorts. Caller is responsible to de-alocate the memory.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iGlSyncConnector The index of the desired GL-Sync connector. The index can range from 0 to NumOfGLSyncConnectors - 1.
/// \param[out] lpNumGLSyncGPUPorts The pointer to the number of GPU ports on the specified GL-Sync module.
/// \param[out] lpNumGlSyncPorts The pointer to the number of external ports (RJ45/BNC) on the specified GL-Sync module.
/// \param[out] lpMaxSyncDelay The pointer to the maximum value of the sync delay Genlock setting.
/// \param[out] lpMaxSampleRate The pointer to the maximum value of the sample rate Genlock setting.
/// \param[out] ppGlSyncPorts The pointer to the pointer to the retrieved information about the external ports on the specified GL-Sync connector.\n
/// The user passes a memory allocation callback to allocate memory for this and to free the memory once done with it.
/// \return If the function is successful, \ref ADL_OK is returned. If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned.\n
/// If the function is not supported, \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Workstation_GLSyncModuleInfo_Get (ADL_CONTEXT_HANDLE context,int iAdapterIndex,
												 int iGlSyncConnector,
												 int* lpNumGLSyncGPUPorts,
												 int* lpNumGlSyncPorts,
												 int* lpMaxSyncDelay,
												 int* lpMaxSampleRate,
												 ADLGLSyncPortCaps** ppGlSyncPorts);

///
/// \brief Function to describe the GL-Sync module caps.
/// 
/// This function describes the hardware capabilities of the GL-Sync module attached to a specific adapter and GL-Sync connector.\n
/// This API uses the Caller's Memory Allocation Callback Function to allocate memory pointed by ppGlSyncPorts. Caller is responsible to de-alocate the memory.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iGlSyncConnector The index of the desired GL-Sync connector. The index can range from 0 to NumOfGLSyncConnectors - 1.
/// \param[out] lpNumGLSyncGPUPorts The pointer to the number of GPU ports on the specified GL-Sync module.
/// \param[out] lpNumGlSyncPorts The pointer to the number of external ports (RJ45/BNC) on the specified GL-Sync module.
/// \param[out] lpMaxSyncDelay The pointer to the maximum value of the sync delay Genlock setting.
/// \param[out] lpMaxSampleRate The pointer to the maximum value of the sample rate Genlock setting.
/// \param[out] ppGlSyncPorts The pointer to the pointer to the retrieved information about the external ports on the specified GL-Sync connector.\n
/// The user passes a memory allocation callback to allocate memory for this and to free the memory once done with it.
/// \return If the function is successful, \ref ADL_OK is returned. If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned.\n
/// If the function is not supported, \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
ADL_EXTERNC int EXPOSED ADL_Workstation_GLSyncModuleInfo_Get (int iAdapterIndex,
												 int iGlSyncConnector,
												 int* lpNumGLSyncGPUPorts,
												 int* lpNumGlSyncPorts,
												 int* lpMaxSyncDelay,
												 int* lpMaxSampleRate,
												 ADLGLSyncPortCaps** ppGlSyncPorts);

///
/// \brief Function to retrieve the GL-Sync Genlock configuration settings.
/// 
/// This function retrieves the Genlock configuration settings of a specific GPU connector on a GL-Sync module attached to a specified adapter on a GL-Sync connector.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iGlSyncConnector The index of the desired GL-Sync connector. The index can range from 0 to NumOfGLSyncConnectors - 1.
/// \param[in]  iGlValidMask The mask indicating the Genlock settings to be retrieved (bit field of ADL_GLSYNC_CONFIGMASK_*). \ref define_glsync
/// \param[out] lpGlSyncGenlockConfig The pointer to the ADLGLSyncGenlockConfig structure with the retrieved Genlock settings.
/// \return If the function is successful, \ref ADL_OK is returned. If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned.\n
/// If the function is not supported, \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Workstation_GLSyncGenlockConfiguration_Get (ADL_CONTEXT_HANDLE context,int iAdapterIndex, 
														   int iGlSyncConnector,
														   int iGlValidMask,
														   ADLGLSyncGenlockConfig* lpGlSyncGenlockConfig);

///
/// \brief Function to retrieve the GL-Sync Genlock configuration settings.
/// 
/// This function retrieves the Genlock configuration settings of a specific GPU connector on a GL-Sync module attached to a specified adapter on a GL-Sync connector.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iGlSyncConnector The index of the desired GL-Sync connector. The index can range from 0 to NumOfGLSyncConnectors - 1.
/// \param[in]  iGlValidMask The mask indicating the Genlock settings to be retrieved (bit field of ADL_GLSYNC_CONFIGMASK_*). \ref define_glsync
/// \param[out] lpGlSyncGenlockConfig The pointer to the ADLGLSyncGenlockConfig structure with the retrieved Genlock settings.
/// \return If the function is successful, \ref ADL_OK is returned. If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned.\n
/// If the function is not supported, \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
ADL_EXTERNC int EXPOSED ADL_Workstation_GLSyncGenlockConfiguration_Get (int iAdapterIndex, 
														   int iGlSyncConnector,
														   int iGlValidMask,
														   ADLGLSyncGenlockConfig* lpGlSyncGenlockConfig);

///
/// \brief Function to set the GL-Sync Genlock configuration settings.
/// 
/// This function sets the Genlock configuration settings of a specific GPU connector on a GL-Sync module attached to a specified adapter on a GL-Sync connector.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iGlSyncConnector The index of the desired GL-Sync connector. The index can range from 0 to NumOfGLSyncConnectors - 1.
/// \param[in]  glSyncGenlockConfig The Genlock settings to be set in ADLGLSyncGenlockConfig structure
/// \return If the function is successful, \ref ADL_OK is returned. If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned.\n
/// If the function is not supported, \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Workstation_GLSyncGenlockConfiguration_Set (ADL_CONTEXT_HANDLE context,int iAdapterIndex, 
														   int iGlSyncConnector,
														   ADLGLSyncGenlockConfig glSyncGenlockConfig);

///
/// \brief Function to set the GL-Sync Genlock configuration settings.
/// 
/// This function sets the Genlock configuration settings of a specific GPU connector on a GL-Sync module attached to a specified adapter on a GL-Sync connector.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iGlSyncConnector The index of the desired GL-Sync connector. The index can range from 0 to NumOfGLSyncConnectors - 1.
/// \param[in]  glSyncGenlockConfig The Genlock settings to be set in ADLGLSyncGenlockConfig structure
/// \return If the function is successful, \ref ADL_OK is returned. If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned.\n
/// If the function is not supported, \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
ADL_EXTERNC int EXPOSED ADL_Workstation_GLSyncGenlockConfiguration_Set (int iAdapterIndex, 
														   int iGlSyncConnector,
														   ADLGLSyncGenlockConfig glSyncGenlockConfig);

///
/// \brief Function to retrieve GL-Sync port information.
/// 
/// This function implements the DI call to retrieve the information about a GL-Sync port of a specified GL-Sync module and adapter.
/// This API uses the Caller's Memory Allocation Callback Function to allocate memory pointed by ppGlSyncLEDs. Caller is responsible to de-alocate the memory.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iGlSyncConnector The index of the desired GL-Sync connector. The index can range from 0 to NumOfGLSyncConnectors - 1.
/// \param[in]  iGlSyncPortType The desired GL-Sync port (ADL_GLSYNC_PORT_*). \ref define_glsync
/// \param[in]  iNumLEDs The number of LEDs for the specified port. If the number of LEDs is unknown, specify a value of 0.
/// \param[out] lpGlSyncPortInfo The pointer to the retrieved port information in ADLGlSyncPortInfo structure
/// \param[out] ppGlSyncLEDs The pointer to the pointer to the retrieved information about the LEDs associated to the specified GL-Sync connector.\n
/// The user must pass a memory allocation callback to allocate memory for this and to free the memory once done with it.
/// \return If the function is successful, \ref ADL_OK is returned. If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned.\n
/// If the function is not supported, \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Workstation_GLSyncPortState_Get (ADL_CONTEXT_HANDLE context,int iAdapterIndex, 
												int iGlSyncConnector,
												int iGlSyncPortType,
												int iNumLEDs,
												ADLGlSyncPortInfo* lpGlSyncPortInfo,
												int** ppGlSyncLEDs);

///
/// \brief Function to retrieve GL-Sync port information.
/// 
/// This function implements the DI call to retrieve the information about a GL-Sync port of a specified GL-Sync module and adapter.
/// This API uses the Caller's Memory Allocation Callback Function to allocate memory pointed by ppGlSyncLEDs. Caller is responsible to de-alocate the memory.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iGlSyncConnector The index of the desired GL-Sync connector. The index can range from 0 to NumOfGLSyncConnectors - 1.
/// \param[in]  iGlSyncPortType The desired GL-Sync port (ADL_GLSYNC_PORT_*). \ref define_glsync
/// \param[in]  iNumLEDs The number of LEDs for the specified port. If the number of LEDs is unknown, specify a value of 0.
/// \param[out] lpGlSyncPortInfo The pointer to the retrieved port information in ADLGlSyncPortInfo structure
/// \param[out] ppGlSyncLEDs The pointer to the pointer to the retrieved information about the LEDs associated to the specified GL-Sync connector.\n
/// The user must pass a memory allocation callback to allocate memory for this and to free the memory once done with it.
/// \return If the function is successful, \ref ADL_OK is returned. If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned.\n
/// If the function is not supported, \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
ADL_EXTERNC int EXPOSED ADL_Workstation_GLSyncPortState_Get (int iAdapterIndex, 
												int iGlSyncConnector,
												int iGlSyncPortType,
												int iNumLEDs,
												ADLGlSyncPortInfo* lpGlSyncPortInfo,
												int** ppGlSyncLEDs);


///
/// \brief Function to perform GL-Sync port control.
/// 
/// This function implements the DI call to enable or disable an output signal on the RJ-45 ports of a specified GL-Sync module and adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iGlSyncConnector The index of the desired GL-Sync connector. The index can range from 0 to NumOfGLSyncConnectors - 1.
/// \param[in]  glSyncPortControl The ADLGlSyncPortControl structure to be set.
/// \return If the function is successful, \ref ADL_OK is returned. If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned.\n
/// If the function is not supported, \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Workstation_GLSyncPortState_Set (ADL_CONTEXT_HANDLE context,int iAdapterIndex,
												 int iGlSyncConnector,
												 ADLGlSyncPortControl glSyncPortControl);

///
/// \brief Function to perform GL-Sync port control.
/// 
/// This function implements the DI call to enable or disable an output signal on the RJ-45 ports of a specified GL-Sync module and adapter.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iGlSyncConnector The index of the desired GL-Sync connector. The index can range from 0 to NumOfGLSyncConnectors - 1.
/// \param[in]  glSyncPortControl The ADLGlSyncPortControl structure to be set.
/// \return If the function is successful, \ref ADL_OK is returned. If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned.\n
/// If the function is not supported, \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
ADL_EXTERNC int EXPOSED ADL_Workstation_GLSyncPortState_Set (int iAdapterIndex,
												 int iGlSyncConnector,
												 ADLGlSyncPortControl glSyncPortControl);

												
///
/// \brief Function to retrieve the GL-Sync mode for a display.
/// 
/// This function implements the DI call to retrieve the GL-Sync mode for a specified display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index.
/// \param[out]  lpGlSyncMode The pointer to the ADLGlSyncMode structure storing the GL-Sync mode of the specified display.
/// \return If the function is successful, \ref ADL_OK is returned. If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned.\n
/// If the function is not supported, \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Workstation_DisplayGLSyncMode_Get (ADL_CONTEXT_HANDLE context,int iAdapterIndex,
													int iDisplayIndex,
													ADLGlSyncMode* lpGlSyncMode);

///
/// \brief Function to retrieve the GL-Sync mode for a display.
/// 
/// This function implements the DI call to retrieve the GL-Sync mode for a specified display.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index.
/// \param[out]  lpGlSyncMode The pointer to the ADLGlSyncMode structure storing the GL-Sync mode of the specified display.
/// \return If the function is successful, \ref ADL_OK is returned. If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned.\n
/// If the function is not supported, \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
ADL_EXTERNC int EXPOSED ADL_Workstation_DisplayGLSyncMode_Get (int iAdapterIndex,
													int iDisplayIndex,
													ADLGlSyncMode* lpGlSyncMode);

///
/// \brief Function to set the GL-Sync mode for a display.
/// 
/// This function implements the DI call to set the GL-Sync mode for a specified display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index.
/// \param[in]  glSyncMode The ADLGlSyncMode structure describing the GL-Sync mode to set for the specified display.
/// \return If the function is successful, \ref ADL_OK is returned. \ref ADL_OK_MODE_CHANGE if success but need mode change,\n
/// If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned. If the function is not supported,
/// \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Workstation_DisplayGLSyncMode_Set (ADL_CONTEXT_HANDLE context,int iAdapterIndex,
													int iDisplayIndex,
													ADLGlSyncMode glSyncMode);

///
/// \brief Function to set the GL-Sync mode for a display.
/// 
/// This function implements the DI call to set the GL-Sync mode for a specified display.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDisplayIndex The desired display index.
/// \param[in]  glSyncMode The ADLGlSyncMode structure describing the GL-Sync mode to set for the specified display.
/// \return If the function is successful, \ref ADL_OK is returned. \ref ADL_OK_MODE_CHANGE if success but need mode change,\n
/// If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned. If the function is not supported,
/// \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
ADL_EXTERNC int EXPOSED ADL_Workstation_DisplayGLSyncMode_Set (int iAdapterIndex,
													int iDisplayIndex,
													ADLGlSyncMode glSyncMode);


///
/// \brief Function to get the supported GL-Sync topology.
/// 
/// This function implements the DI call to set the GL-Sync mode for a specified display.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iNumSyncModes 
/// \param[in]  glSyncModes 
/// \param[out] iNumSugSyncModes
/// \param[out] glSugSyncModes
/// \return If the function is successful, \ref ADL_OK is returned.\n
/// If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned. If the function is not supported,
/// \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Workstation_GLSyncSupportedTopology_Get (ADL_CONTEXT_HANDLE context,int iAdapterIndex,
													int iNumSyncModes,
													ADLGlSyncMode2 *glSyncModes,
													int *iNumSugSyncModes,
													ADLGlSyncMode2 **glSugSyncModes);

///
/// \brief Function to get the supported GL-Sync topology.
/// 
/// This function implements the DI call to set the GL-Sync mode for a specified display.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iNumSyncModes 
/// \param[in]  glSyncModes 
/// \param[out] iNumSugSyncModes
/// \param[out] glSugSyncModes
/// \return If the function is successful, \ref ADL_OK is returned.\n
/// If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned. If the function is not supported,
/// \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
ADL_EXTERNC int EXPOSED ADL_Workstation_GLSyncSupportedTopology_Get (int iAdapterIndex,
													int iNumSyncModes,
													ADLGlSyncMode2 *glSyncModes,
													int *iNumSugSyncModes,
													ADLGlSyncMode2 **glSugSyncModes);


///
/// \brief Function to get the load balancing state of the specified adapter.
/// 
/// This function implements the PCS access to Get the load balancing state for a specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[out]  lpResultMask The int pointer to mask bit of state.
///              It can be combination of \ref ADL_WORKSTATION_LOADBALANCING_DISABLED, ref ADL_WORKSTATION_LOADBALANCING_ENABLED.
/// \param[out]  lpCurResultValue The int pointer to the current state
///              It can be one of \ref ADL_WORKSTATION_LOADBALANCING_DISABLED, ref ADL_WORKSTATION_LOADBALANCING_ENABLED.
/// \param[out]  lpDefResultValue The int pointer to the dafault state
///              It can be one of \ref ADL_WORKSTATION_LOADBALANCING_DISABLED, ref ADL_WORKSTATION_LOADBALANCING_ENABLED.
/// \return If the function is successful, \ref ADL_OK is returned. \ref ADL_OK_MODE_CHANGE if success but need mode change,\n
/// If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned. If the function is not supported,
/// \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Workstation_LoadBalancing_Get(ADL_CONTEXT_HANDLE context,int *lpResultMask, int *lpCurResultValue, int *lpDefResultValue);

///
/// \brief Function to get the load balancing state of the specified adapter.
/// 
/// This function implements the PCS access to Get the load balancing state for a specified adapter.
/// \platform
/// \ALL
/// \param[out]  lpResultMask The int pointer to mask bit of state.
///              It can be combination of \ref ADL_WORKSTATION_LOADBALANCING_DISABLED, ref ADL_WORKSTATION_LOADBALANCING_ENABLED.
/// \param[out]  lpCurResultValue The int pointer to the current state
///              It can be one of \ref ADL_WORKSTATION_LOADBALANCING_DISABLED, ref ADL_WORKSTATION_LOADBALANCING_ENABLED.
/// \param[out]  lpDefResultValue The int pointer to the dafault state
///              It can be one of \ref ADL_WORKSTATION_LOADBALANCING_DISABLED, ref ADL_WORKSTATION_LOADBALANCING_ENABLED.
/// \return If the function is successful, \ref ADL_OK is returned. \ref ADL_OK_MODE_CHANGE if success but need mode change,\n
/// If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned. If the function is not supported,
/// \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
ADL_EXTERNC int EXPOSED ADL_Workstation_LoadBalancing_Get(int *lpResultMask, int *lpCurResultValue, int *lpDefResultValue);

///
/// \brief Function to set the load balancing state of the specified adapter.
/// 
/// This function implements the PCS access to set the load balancing mode for a specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iCurState The current state to set for the specified adapter.
/// \return If the function is successful, \ref ADL_OK is returned. \ref ADL_OK_MODE_CHANGE if success but need mode change,\n
/// If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned. If the function is not supported,
/// \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Workstation_LoadBalancing_Set(ADL_CONTEXT_HANDLE context,int iCurState);

///
/// \brief Function to set the load balancing state of the specified adapter.
/// 
/// This function implements the PCS access to set the load balancing mode for a specified adapter.
/// \platform
/// \ALL
/// \param[in]  iCurState The current state to set for the specified adapter.
/// \return If the function is successful, \ref ADL_OK is returned. \ref ADL_OK_MODE_CHANGE if success but need mode change,\n
/// If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned. If the function is not supported,
/// \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
ADL_EXTERNC int EXPOSED ADL_Workstation_LoadBalancing_Set(int iCurState);

///
/// \brief Function to set the load balancing capabilities for the specified adapter.
/// 
/// This function implements the CI call to Get the load balancing caps for a specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpResultMask The int pointer to the mask bits of load balancing of the specified adapter,
///              It can be combination of \ref ADL_WORKSTATION_LOADBALANCING_SUPPORTED, ref ADL_WORKSTATION_LOADBALANCING_AVAILABLE.
/// \param[out]  lpResultValue The int pointer to the capable load balancing bits of the specified adapter.
///              It can be combination of \ref ADL_WORKSTATION_LOADBALANCING_SUPPORTED, ref ADL_WORKSTATION_LOADBALANCING_AVAILABLE
/// \return If the function is successful, \ref ADL_OK is returned. \ref ADL_OK_MODE_CHANGE if success but need mode change,\n
/// If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned. If the function is not supported,
/// \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Workstation_LoadBalancing_Caps(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int *lpResultMask, int *lpResultValue);

///
/// \brief Function to set the load balancing capabilities for the specified adapter.
/// 
/// This function implements the CI call to Get the load balancing caps for a specified adapter.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpResultMask The int pointer to the mask bits of load balancing of the specified adapter,
///              It can be combination of \ref ADL_WORKSTATION_LOADBALANCING_SUPPORTED, ref ADL_WORKSTATION_LOADBALANCING_AVAILABLE.
/// \param[out]  lpResultValue The int pointer to the capable load balancing bits of the specified adapter.
///              It can be combination of \ref ADL_WORKSTATION_LOADBALANCING_SUPPORTED, ref ADL_WORKSTATION_LOADBALANCING_AVAILABLE
/// \return If the function is successful, \ref ADL_OK is returned. \ref ADL_OK_MODE_CHANGE if success but need mode change,\n
/// If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned. If the function is not supported,
/// \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
ADL_EXTERNC int EXPOSED ADL_Workstation_LoadBalancing_Caps(int iAdapterIndex, int *lpResultMask, int *lpResultValue);

///
///\brief Function to get current requested state of Deep Bit Depth and related settings
/// 
/// This function returns if specified adapter supports Deep Bit Depth, grayscale and gamma bypass 
/// and their defaults.   These settings are applicable to system wide to all currently known adapters.  
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[out]  lpCurDBDState : The requested deep bit depth state. This is the state the user wishes 
/// to put the deep bit depth feature to \ref define_dbd_state
///\param[out]  lpDefDBDState: The default deep bit depth state state of the system.   \ref 
/// define_dbd_state
///\param[out] lpCurGrayscale: Not currently used. It is there for future use.
///\param[out] lpDefGrayscale: Not currently used. It is there for future use.
///\param[out] lpCurBypass: Not currently used. It is there for future use.
///\param[out] lpDefBypass: Not currently used. It is there for future use.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an 
/// ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Workstation_DeepBitDepth_Get (ADL_CONTEXT_HANDLE context, int *lpCurDBDState, int *lpDefDBDState, int *lpCurGrayscale, 
											int *lpDefGrayscale, int *lpCurBypass, int *lpDefBypass);

///
///\brief Function to get current requested state of Deep Bit Depth and related settings
/// 
/// This function returns if specified adapter supports Deep Bit Depth, grayscale and gamma bypass 
/// and their defaults.   These settings are applicable to system wide to all currently known adapters.  
/// \platform
/// \ALL
/// \param[out]  lpCurDBDState : The requested deep bit depth state. This is the state the user wishes 
/// to put the deep bit depth feature to \ref define_dbd_state
///\param[out]  lpDefDBDState: The default deep bit depth state state of the system.   \ref 
/// define_dbd_state
///\param[out] lpCurGrayscale: Not currently used. It is there for future use.
///\param[out] lpDefGrayscale: Not currently used. It is there for future use.
///\param[out] lpCurBypass: Not currently used. It is there for future use.
///\param[out] lpDefBypass: Not currently used. It is there for future use.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an 
/// ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Workstation_DeepBitDepth_Get ( int *lpCurDBDState, int *lpDefDBDState, int *lpCurGrayscale, 
											int *lpDefGrayscale, int *lpCurBypass, int *lpDefBypass);

///
///\brief Function to set requested state of Deep Bit Depth and related settings
/// 
/// This function sets for the specified adapter supports Deep Bit Depth, grayscale and gamma bypass 
/// These settings are applicable to system wide to all currently known adapters.
/// A restart is required to apply settings, ADL_OK_RESTART will be return from function.  
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iDBDState: The requested Deep bit depth state to set the system to.\ref 
/// define_dbd_state
/// \param[in]  iGrayscale: Not currently used. It is there for future use.
/// \param[in]  iBypassGamma: Not currently used. It is there for future use.
/// \return If the function succeeds, the return value is \ref ADL_OK_RESTART. Otherwise the return value is an 
/// ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL2_Workstation_DeepBitDepth_Set(ADL_CONTEXT_HANDLE context, int iDBDState, int iGrayscale, int iBypassGamma);

///
///\brief Function to set requested state of Deep Bit Depth and related settings
/// 
/// This function sets for the specified adapter supports Deep Bit Depth, grayscale and gamma bypass 
/// These settings are applicable to system wide to all currently known adapters.
/// A restart is required to apply settings, ADL_OK_RESTART will be return from function.  
/// \platform
/// \ALL
/// \param[in]  iDBDState: The requested Deep bit depth state to set the system to.\ref 
/// define_dbd_state
/// \param[in]  iGrayscale: Not currently used. It is there for future use.
/// \param[in]  iBypassGamma: Not currently used. It is there for future use.
/// \return If the function succeeds, the return value is \ref ADL_OK_RESTART. Otherwise the return value is an 
/// ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Workstation_DeepBitDepth_Set( int iDBDState, int iGrayscale, int iBypassGamma);

///
///\brief Function to get ECC (Error Correction Code) Capabilities on the specified adapter.
/// This function implements the CI call to get ECC (Error Correction Code) Capabilities on the specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex	- The index of the desired adapter.
/// \param[out]  lpSupported	- returns the bolean capability, whether ECC is supported or not(\ref ADL_TRUE or \ref ADL_FALSE) for the specified adapter
/// \return If the function is successful, \ref ADL_OK is returned,
/// If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned. If the function is not supported,
/// \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Workstation_ECC_Caps (ADL_CONTEXT_HANDLE context,int iAdapterIndex, int *lpSupported);

///
///\brief Function to get ECC (Error Correction Code) Capabilities on the specified adapter.
/// This function implements the CI call to get ECC (Error Correction Code) Capabilities on the specified adapter.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex	- The index of the desired adapter.
/// \param[out]  lpSupported	- returns the bolean capability, whether ECC is supported or not(\ref ADL_TRUE or \ref ADL_FALSE) for the specified adapter
/// \return If the function is successful, \ref ADL_OK is returned,
/// If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned. If the function is not supported,
/// \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
ADL_EXTERNC int EXPOSED ADL_Workstation_ECC_Caps (int iAdapterIndex, int *lpSupported);

///
///\brief Function to get ECC (Error Correction Code) current and desired states on the specified adapter.
/// This function implements the CI call to get ECC (Error Correction Code) current mode(driver applied mode) and the desired mode (user requested mode) on the specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex	- The index of the desired adapter.
/// \param[out]  lpDefaultMode	- Default ECC Mode (\ref ECC_MODE_OFF, \ref ECC_MODE_ON).\ref define_ecc_mode_states.
/// \param[out]  lpCurrentMode	- ECC Mode that is currently stored in the permanent store for driver. (\ref ECC_MODE_OFF, \ref ECC_MODE_ON).\ref define_ecc_mode_states.
/// \param[out]  lpDesiredMode	- Desired ECC mode from the user, once the user reboots the machine DesiredMode will be applied to CurrentMode (\ref ECC_MODE_OFF, \ref ECC_MODE_ON). \ref define_ecc_mode_states.
/// \return If the function is successful, \ref ADL_OK is returned,
/// If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned. If the function is not supported,
/// \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Workstation_ECC_Get (ADL_CONTEXT_HANDLE context,int iAdapterIndex, int *lpDefaultMode, int *lpCurrentMode, int *lpDesiredMode);

///
///\brief Function to get ECC (Error Correction Code) current and desired states on the specified adapter.
/// This function implements the CI call to get ECC (Error Correction Code) current mode(driver applied mode) and the desired mode (user requested mode) on the specified adapter.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex	- The index of the desired adapter.
/// \param[out]  lpDefaultMode	- Default ECC Mode (\ref ECC_MODE_OFF, \ref ECC_MODE_ON).\ref define_ecc_mode_states.
/// \param[out]  lpCurrentMode	- ECC Mode that is currently stored in the permanent store for driver. (\ref ECC_MODE_OFF, \ref ECC_MODE_ON).\ref define_ecc_mode_states.
/// \param[out]  lpDesiredMode	- Desired ECC mode from the user, once the user reboots the machine DesiredMode will be applied to CurrentMode (\ref ECC_MODE_OFF, \ref ECC_MODE_ON). \ref define_ecc_mode_states.
/// \return If the function is successful, \ref ADL_OK is returned,
/// If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned. If the function is not supported,
/// \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
ADL_EXTERNC int EXPOSED ADL_Workstation_ECC_Get (int iAdapterIndex, int *lpDefaultMode, int *lpCurrentMode, int *lpDesiredMode);

///
///\brief Function to get ECC (Error Correction Code) current and desired states on the specified adapter.
/// This function implements the CI call to get ECC (Error Correction Code) current mode(driver applied mode) and the desired mode (user requested mode) on the specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex	- The index of the desired adapter.
/// \param[out]  lpModeType		- Default ECC Mode (\ref ECC_MODE_OFF, \ref ECC_MODE_ON, \ref ECC_MODE_HBM).\ref define_ecc_mode_states.
/// \param[out]  lpDefaultMode	- Default ECC Mode (\ref ECC_MODE_OFF, \ref ECC_MODE_ON, \ref ECC_MODE_HBM).\ref define_ecc_mode_states.
/// \param[out]  lpCurrentMode	- ECC Mode that is currently stored in the permanent store for driver. (\ref ECC_MODE_OFF, \ref ECC_MODE_ON, \ref ECC_MODE_HBM).\ref define_ecc_mode_states.
/// \param[out]  lpDesiredMode	- Desired ECC mode from the user, once the user reboots the machine DesiredMode will be applied to CurrentMode (\ref ECC_MODE_OFF, \ref ECC_MODE_ON, \ref ECC_MODE_HBM ). \ref define_ecc_mode_states.
/// \return If the function is successful, \ref ADL_OK is returned,
/// If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned. If the function is not supported,
/// \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Workstation_ECCX2_Get (ADL_CONTEXT_HANDLE context,int iAdapterIndex,int *lpModeType, int *lpDefaultMode, int *lpCurrentMode, int *lpDesiredMode);

///
///\brief Function to get ECC (Error Correction Code) current and desired states on the specified adapter.
/// This function implements the CI call to get ECC (Error Correction Code) current mode(driver applied mode) and the desired mode (user requested mode) on the specified adapter.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex	- The index of the desired adapter.
/// \param[out]  lpModeType		- Default ECC Mode (\ref ECC_MODE_OFF, \ref ECC_MODE_ON, \ref ECC_MODE_HBM).\ref define_ecc_mode_states.
/// \param[out]  lpDefaultMode	- Default ECC Mode (\ref ECC_MODE_OFF, \ref ECC_MODE_ON, \ref ECC_MODE_HBM).\ref define_ecc_mode_states.
/// \param[out]  lpCurrentMode	- ECC Mode that is currently stored in the permanent store for driver. (\ref ECC_MODE_OFF, \ref ECC_MODE_ON, \ref ECC_MODE_HBM).\ref define_ecc_mode_states.
/// \param[out]  lpDesiredMode	- Desired ECC mode from the user, once the user reboots the machine DesiredMode will be applied to CurrentMode (\ref ECC_MODE_OFF, \ref ECC_MODE_ON, \ref ECC_MODE_HBM). \ref define_ecc_mode_states.
/// \return If the function is successful, \ref ADL_OK is returned,
/// If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned. If the function is not supported,
/// \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
ADL_EXTERNC int EXPOSED ADL_Workstation_ECCX2_Get (int iAdapterIndex,int *lpModeType, int *lpDefaultMode, int *lpCurrentMode, int *lpDesiredMode);


///
///\brief Function to get ECC statistics on the specified adapter. 
/// This function implements the CI call to get SEC(Single Error Correct) and DED(Double Error Detect) Counts on the specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex	- The index of the desired adapter.
/// \param[out]  lpAdlEccData	- is of type ADLECCData. Gets the SEC & DED counts \ref ADLECCData 
/// \return If the function is successful, \ref ADL_OK is returned,
/// If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned. If the function is not supported,
/// \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned. 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Workstation_ECCData_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLECCData *lpAdlEccData);

///
///\brief Function to get ECC statistics on the specified adapter. 
/// This function implements the CI call to get SEC(Single Error Correct) and DED(Double Error Detect) Counts on the specified adapter.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex	- The index of the desired adapter.
/// \param[out]  lpAdlEccData	- is of type ADLECCData. Gets the SEC & DED counts \ref ADLECCData 
/// \return If the function is successful, \ref ADL_OK is returned,
/// If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned. If the function is not supported,
/// \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned. 
ADL_EXTERNC int EXPOSED ADL_Workstation_ECCData_Get(int iAdapterIndex, ADLECCData *lpAdlEccData);

///
///\brief Function to set ECC Mode on the specified adapter
/// This function implements the CI call to set ECC (Error Correction Code) to turn on and off this feature on the specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex	- The index of the desired adapter.
/// \param[in]  iDesiredMode	- ECC Mode currently accepts \ref ECC_MODE_OFF or \ref ECC_MODE_ON. \ref define_ecc_mode_states
/// \return If the function is successful, \ref ADL_OK is returned,
/// If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned. If the function is not supported,
/// \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned. 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Workstation_ECC_Set (ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDesiredMode);

///
///\brief Function to set ECC Mode on the specified adapter
/// This function implements the CI call to set ECC (Error Correction Code) to turn on and off this feature on the specified adapter.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex	- The index of the desired adapter.
/// \param[in]  iDesiredMode	- ECC Mode currently accepts \ref ECC_MODE_OFF or \ref ECC_MODE_ON. \ref define_ecc_mode_states
/// \return If the function is successful, \ref ADL_OK is returned,
/// If iAdapterIndex is an invalid adapter index, \ref ADL_ERR_INVALID_ADL_IDX is returned. If the function is not supported,
/// \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned. 
ADL_EXTERNC int EXPOSED ADL_Workstation_ECC_Set (int iAdapterIndex, int iDesiredMode);

// @}

#endif /* WORKSTATION_H_ */
