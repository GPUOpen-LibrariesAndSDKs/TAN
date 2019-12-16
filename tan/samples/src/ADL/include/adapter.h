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

#ifndef ADAPTER_H_
#define ADAPTER_H_

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


///\addtogroup ADAPTER
// @{
///
/// \brief Function to determine if the adapter is active or not.
/// 
/// The function is used to check if the adapter associated with iAdapterIndex is active. Logic is different for Windows and Linux!
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpStatus The pointer to the retrieved status. \ref ADL_TRUE : Active; \ref ADL_FALSE : Disabled.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC  int EXPOSED ADL2_Adapter_Active_Get (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* lpStatus);

///
/// \brief Function to determine if the adapter is active or not.
/// 
/// The function is used to check if the adapter associated with iAdapterIndex is active. Logic is different for Windows and Linux!
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpStatus The pointer to the retrieved status. \ref ADL_TRUE : Active; \ref ADL_FALSE : Disabled.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC  int EXPOSED ADL_Adapter_Active_Get ( int iAdapterIndex, int* lpStatus);


/// \brief ADL local interface. Function to retrieve the supported aspects list.
/// 
/// This function retrieves the list of supported aspects for a specified adapter.
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpAspects The pointer to the buffer storing the list of aspects supported for the specified adapter.
/// \param[in] iSize The size of the lpAspects buffer.
/// \platform 
/// \ALL
/// \return If the function valid, the return value is 1. Otherwise it is 0. 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_Aspects_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, char* lpAspects, int iSize );

/// \brief ADL local interface. Function to retrieve the supported aspects list.
/// 
/// This function retrieves the list of supported aspects for a specified adapter.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpAspects The pointer to the buffer storing the list of aspects supported for the specified adapter.
/// \param[in] iSize The size of the lpAspects buffer.
/// \platform 
/// \ALL
/// \return If the function valid, the return value is 1. Otherwise it is 0. 
ADL_EXTERNC int EXPOSED ADL_Adapter_Aspects_Get( int iAdapterIndex, char* lpAspects, int iSize );

///
/// \brief Function to retrieve the number of OS-known adapters.
/// 
/// This function retrieves the number of graphics adapters recognized by the OS (OS-known adapters). OS-known adapters can include adapters that are physically present in the system (logical adapters)    
/// as well as ones that no longer present in the system but are still recognized by the OS.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[out] lpNumAdapters The pointer to the number of OS-known adapters.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_NumberOfAdapters_Get (ADL_CONTEXT_HANDLE context,  int* lpNumAdapters );

///
/// \brief Function to retrieve the number of OS-known adapters.
/// 
/// This function retrieves the number of graphics adapters recognized by the OS (OS-known adapters). OS-known adapters can include adapters that are physically present in the system (logical adapters)    
/// as well as ones that no longer present in the system but are still recognized by the OS.
/// \platform
/// \ALL
/// \param[out] lpNumAdapters The pointer to the number of OS-known adapters.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Adapter_NumberOfAdapters_Get (  int* lpNumAdapters );

///
/// \brief Function to save driver data
/// 
/// This function saves all required data from driver to persist updated settings
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The adapter index to be flushed.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Flush_Driver_Data(ADL_CONTEXT_HANDLE context,int iAdapterIndex);

///
/// \brief Function to save driver data
/// 
/// This function saves all required data from driver to persist updated settings
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The adapter index to be flushed.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Flush_Driver_Data(int iAdapterIndex);

///
/// \brief Retrieves all OS-known adapter information.
/// 
/// This function retrieves the adapter information of all OS-known adapters in the system. OS-known adapters can include adapters that are physically present in the system (logical adapters)    
/// as well as ones that are no longer present in the system but are still recognized by the OS.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iInputSize The size of the lpInfo buffer.
/// \param[out] lpInfo The pointer to the buffer containing the retrieved adapter information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results  
///
/// \remarks This API take a fixed-size output array. For dynamic-size output, use \ref ADL_Adapter_AdapterInfoX2_Get function.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_AdapterInfo_Get (ADL_CONTEXT_HANDLE context,LPAdapterInfo lpInfo, int iInputSize);

///
/// \brief Retrieves all OS-known adapter information.
/// 
/// This function retrieves the adapter information of all OS-known adapters in the system. OS-known adapters can include adapters that are physically present in the system (logical adapters)    
/// as well as ones that are no longer present in the system but are still recognized by the OS.
/// \platform
/// \ALL
/// \param[in]  iInputSize The size of the lpInfo buffer.
/// \param[out] lpInfo The pointer to the buffer containing the retrieved adapter information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results  
///
/// \remarks This API take a fixed-size output array. For dynamic-size output, use \ref ADL_Adapter_AdapterInfoX2_Get function.
ADL_EXTERNC int EXPOSED ADL_Adapter_AdapterInfo_Get (LPAdapterInfo lpInfo, int iInputSize);

///
/// \brief Retrieves all OS-known adapter information.
/// 
/// This function retrieves the adapter information of all OS-known adapters in the system. OS-known adapters can include adapters that are physically present in the system (logical adapters)    
/// as well as ones that are no longer present in the system but are still recognized by the OS.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[out] lppAdapterInfo is pointer to the pointer of AdapterInfo array. Initialize to NULL before calling this API. ADL will allocate the necessary memory, using the user provided callback function.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks This API uses the memory allocation callback function provided by the user.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_AdapterInfoX2_Get (ADL_CONTEXT_HANDLE context,AdapterInfo **lppAdapterInfo);

///
/// \brief Retrieves all OS-known adapter information.
/// 
/// This function retrieves the adapter information of all OS-known adapters in the system. OS-known adapters can include adapters that are physically present in the system (logical adapters)    
/// as well as ones that are no longer present in the system but are still recognized by the OS.
/// \platform
/// \ALL
/// \param[out] lppAdapterInfo is pointer to the pointer of AdapterInfo array. Initialize to NULL before calling this API. ADL will allocate the necessary memory, using the user provided callback function.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// 
/// \remarks This API uses the memory allocation callback function provided by the user.
ADL_EXTERNC int EXPOSED ADL_Adapter_AdapterInfoX2_Get (AdapterInfo **lppAdapterInfo);

///
///\brief Function to get the ASICFamilyType from the adapter. 
/// 
/// This function retrieves the ASIC family types for a specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpAsicTypes The pointer to the \ref define_Asic_type information retrieved from the driver.
/// \param[out] lpValids The pointer to the bit vector indicating which bit is valid on the lpAsicTypes returned.
/// \return If the function succeeds, the return value is \ref ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_ASICFamilyType_Get (ADL_CONTEXT_HANDLE context,int iAdapterIndex, int* lpAsicTypes, int* lpValids);

///
///\brief Function to get the ASICFamilyType from the adapter. 
/// 
/// This function retrieves the ASIC family types for a specified adapter.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpAsicTypes The pointer to the \ref define_Asic_type information retrieved from the driver.
/// \param[out] lpValids The pointer to the bit vector indicating which bit is valid on the lpAsicTypes returned.
/// \return If the function succeeds, the return value is \ref ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Adapter_ASICFamilyType_Get (int iAdapterIndex, int* lpAsicTypes, int* lpValids);

///
/// \brief Function to get the current Force3DClock setting from the adapter. 
///
/// This function retrieves the adapter speed information for a specified adapter.  Return valud can be \ref ADL_ADAPTER_SPEEDCAPS_SUPPORTED or “0”
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpCaps The caps of adapter speed settings
/// \param[out] lpValid The valid bits of adapter speed settings
/// \return If the function succeeds, the return value is \ref ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_Speed_Caps (ADL_CONTEXT_HANDLE context,int iAdapterIndex, int* lpCaps, int* lpValid);

///
/// \brief Function to get the current Force3DClock setting from the adapter. 
///
/// This function retrieves the adapter speed information for a specified adapter.  Return valud can be \ref ADL_ADAPTER_SPEEDCAPS_SUPPORTED or “0”
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpCaps The caps of adapter speed settings
/// \param[out] lpValid The valid bits of adapter speed settings
/// \return If the function succeeds, the return value is \ref ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Adapter_Speed_Caps (int iAdapterIndex, int* lpCaps, int* lpValid);

///
/// \brief Function to get the current Speed setting from the adapter.
///
/// This function retrieves the adapter speed information for a specified adapter.  Return value is either \ref ADL_CONTEXT_SPEED_UNFORCED or \ref ADL_CONTEXT_SPEED_FORCEHIGH
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpCurrent The current speed setting
/// \param[out] lpDefault The default speed setting
/// \return If the function succeeds, the return value is \ref ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_Speed_Get (ADL_CONTEXT_HANDLE context,int iAdapterIndex, int *lpCurrent, int *lpDefault);

///
/// \brief Function to get the current Speed setting from the adapter.
///
/// This function retrieves the adapter speed information for a specified adapter.  Return value is either \ref ADL_CONTEXT_SPEED_UNFORCED or \ref ADL_CONTEXT_SPEED_FORCEHIGH
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpCurrent The current speed setting
/// \param[out] lpDefault The default speed setting
/// \return If the function succeeds, the return value is \ref ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Adapter_Speed_Get (int iAdapterIndex, int *lpCurrent, int *lpDefault);

///
/// \brief Function to set the current Speed setting from the adapter.
///
/// This function set the adapter speed information for a specified adapter.  Input value is either \ref ADL_CONTEXT_SPEED_UNFORCED or \ref ADL_CONTEXT_SPEED_FORCEHIGH
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in] iSpeed The speed to set on specified adapter
/// \return If the function succeeds, the return value is \ref ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_Speed_Set (ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iSpeed);

///
/// \brief Function to set the current Speed setting from the adapter.
///
/// This function set the adapter speed information for a specified adapter.  Input value is either \ref ADL_CONTEXT_SPEED_UNFORCED or \ref ADL_CONTEXT_SPEED_FORCEHIGH
/// \platform
/// \ALL
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in] iSpeed The speed to set on specified adapter
/// \return If the function succeeds, the return value is \ref ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Adapter_Speed_Set (int iAdapterIndex, int iSpeed);

///
/// \brief Function to check if the GPU is accessible or not at the time of this call.
/// 
/// Some GPUs enter into various power savings mode when not in use, and during this time, direct access to the GPU can result in failures.
/// Access through proper channels (such as via ADL) always works because the software accounts for the GPU's accessibility.
/// However, direct access outside of authorised mechanisms may fail.
/// Use this API to find out if the GPU is currently accessible or not.
///  If API returns GPU accessible, this status will be maintained for 2 seconds.
///
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpAccessibility The pointer to an int  value which indicates if the adapter is accessible or not (\ref ADL_TRUE : Accessible, \ref ADL_FALSE : Not accessible)
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_Accessibility_Get (ADL_CONTEXT_HANDLE context,int iAdapterIndex, int  *lpAccessibility); 

///
/// \brief Function to check if the GPU is accessible or not at the time of this call.
/// 
/// Some GPUs enter into various power savings mode when not in use, and during this time, direct access to the GPU can result in failures.
/// Access through proper channels (such as via ADL) always works because the software accounts for the GPU's accessibility.
/// However, direct access outside of authorised mechanisms may fail.
/// Use this API to find out if the GPU is currently accessible or not.
///  If API returns GPU accessible, this status will be maintained for 2 seconds.
///
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpAccessibility The pointer to an int  value which indicates if the adapter is accessible or not (\ref ADL_TRUE : Accessible, \ref ADL_FALSE : Not accessible)
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Adapter_Accessibility_Get (int iAdapterIndex, int  *lpAccessibility); 

///
/// \brief ADL local interface. Function to retrieve BIOS information.
/// 
/// This function retrieves the BIOS information for a specified adapter.
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpBiosInfo The pointer to the structure storing the retrieved BIOs information.
/// \platform 
/// \LNX
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_VideoBiosInfo_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLBiosInfo* lpBiosInfo );

///
/// \brief ADL local interface. Function to retrieve BIOS information.
/// 
/// This function retrieves the BIOS information for a specified adapter.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpBiosInfo The pointer to the structure storing the retrieved BIOs information.
/// \platform 
/// \LNX
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Adapter_VideoBiosInfo_Get( int iAdapterIndex, ADLBiosInfo* lpBiosInfo );

///
///\brief Function to get the unique identifier of an adapter.
/// 
/// This function retrieves the unique identifier of a specified adapter.\n
/// The adapter ID is a unique value and will be used to determine what other controllers share the same adapter.\n
/// The desktop will use this to find which HDCs are associated with an adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpAdapterID The pointer to the adapter identifier. Zero means: The adapter is not AMD.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API is a duplicate to ADL_Display_AdapterID_Get()
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
 ADL_EXTERNC int EXPOSED ADL2_Adapter_ID_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int* lpAdapterID);

///
///\brief Function to get the unique identifier of an adapter.
/// 
/// This function retrieves the unique identifier of a specified adapter.\n
/// The adapter ID is a unique value and will be used to determine what other controllers share the same adapter.\n
/// The desktop will use this to find which HDCs are associated with an adapter.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpAdapterID The pointer to the adapter identifier. Zero means: The adapter is not AMD.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API is a duplicate to ADL_Display_AdapterID_Get()
 ADL_EXTERNC int EXPOSED ADL_Adapter_ID_Get(int iAdapterIndex, int* lpAdapterID);

///
/// \brief Function to retrieve adapter caps information.
/// 
/// This function implements a DI call to retrieve adapter capability information .
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] adapterCaps The pointer to the structure storing the retrieved adapter capability information.
/// \return If the function succeeds, the return value is \ref ADL_OK, or an ADL error code otherwise. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_AdapterX2_Caps(ADL_CONTEXT_HANDLE context,int iAdapterIndex,  ADLAdapterCapsX2 *adapterCaps);

///
/// \brief Function to retrieve adapter caps information.
/// 
/// This function implements a DI call to retrieve adapter capability information .
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] adapterCaps The pointer to the structure storing the retrieved adapter capability information.
/// \return If the function succeeds, the return value is \ref ADL_OK, or an ADL error code otherwise. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_AdapterX2_Caps(int iAdapterIndex,  ADLAdapterCapsX2 *adapterCaps);
// @}

///\addtogroup CROSSFIRE
// @{
///
/// \brief Function to retrieve CrossfireX capabilities of the system.
///
/// This function is used to determine if the system is capable of doing CrossfireX, regardless if the system is setup properly.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]	iAdapterIndex The ADL index handle of the desired adapter to query CrossfireX capabilities
/// \param[out]	lpPreferred The index of the preferred CrossfireX chain. The index is a zero based index reference in the ADLCrossfireComb list.
/// \param[out]	lpNumComb The integer value indicating the number of ADLCrossfireComb structures in the lpCrossfireComb buffer
/// \param[out]	ppCrossfireComb The Pointer to an empty memory buffer(NULL).  This function will use the callback to allocate the appropriate buffer size.  This buffer will contain a list of supported CrossfireX combinations if the adapter supports CrossfireX.  If the adapter does not support CrossfireX, this memory buffer will remain empty, or NULL.
/// \return If the function succeeds, the return value is \ref ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_Crossfire_Caps (ADL_CONTEXT_HANDLE context,int iAdapterIndex, int* lpPreferred, int* lpNumComb, ADLCrossfireComb **ppCrossfireComb);

///
/// \brief Function to retrieve CrossfireX capabilities of the system.
///
/// This function is used to determine if the system is capable of doing CrossfireX, regardless if the system is setup properly.
/// \platform
/// \ALL
/// \param[in]	iAdapterIndex The ADL index handle of the desired adapter to query CrossfireX capabilities
/// \param[out]	lpPreferred The index of the preferred CrossfireX chain. The index is a zero based index reference in the ADLCrossfireComb list.
/// \param[out]	lpNumComb The integer value indicating the number of ADLCrossfireComb structures in the lpCrossfireComb buffer
/// \param[out]	ppCrossfireComb The Pointer to an empty memory buffer(NULL).  This function will use the callback to allocate the appropriate buffer size.  This buffer will contain a list of supported CrossfireX combinations if the adapter supports CrossfireX.  If the adapter does not support CrossfireX, this memory buffer will remain empty, or NULL.
/// \return If the function succeeds, the return value is \ref ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
ADL_EXTERNC int EXPOSED ADL_Adapter_Crossfire_Caps (int iAdapterIndex, int* lpPreferred, int* lpNumComb, ADLCrossfireComb **ppCrossfireComb);

///
/// \brief Function to get current CrossfireX combination settings.
///
/// This function is used to retrieve the current information about a particular CrossfireX combination.  A CrossfireX combination includes
/// the CrossfireX capable adapter and its CrossfireX chain (a list of other adapters that will work with the CrossfireX capable adapter).  The possible
/// CrossfireX chain combinations for an adapter is retrieved from the ADL_Adapter_Crossfire_Caps function.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]	iAdapterIndex The ADL index of the CrossfireX capable adapter to query the CrossfireX state and current information
/// \param[in]	lpCrossfireComb The CrossfireX chain to get information about.
/// \param[out]	lpCrossfireInfo State and error information about the CrossfireX combination.
/// \return If the function succeeds, the return value is \ref ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_Crossfire_Get (ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLCrossfireComb *lpCrossfireComb, ADLCrossfireInfo *lpCrossfireInfo);

///
/// \brief Function to get current CrossfireX combination settings.
///
/// This function is used to retrieve the current information about a particular CrossfireX combination.  A CrossfireX combination includes
/// the CrossfireX capable adapter and its CrossfireX chain (a list of other adapters that will work with the CrossfireX capable adapter).  The possible
/// CrossfireX chain combinations for an adapter is retrieved from the ADL_Adapter_Crossfire_Caps function.
/// \platform
/// \ALL
/// \param[in]	iAdapterIndex The ADL index of the CrossfireX capable adapter to query the CrossfireX state and current information
/// \param[in]	lpCrossfireComb The CrossfireX chain to get information about.
/// \param[out]	lpCrossfireInfo State and error information about the CrossfireX combination.
/// \return If the function succeeds, the return value is \ref ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Adapter_Crossfire_Get (int iAdapterIndex, ADLCrossfireComb *lpCrossfireComb, ADLCrossfireInfo *lpCrossfireInfo);

///
/// \brief Function to set CrossfireX combination settings.
///
/// This function is used to enable or disable CrossfireX on a particular adapter (pointed to by iAdapterIndex), along with the indicated CrossfireX chain.
/// The CrossfireX chain combination for the adapter must be one returned by ADL_Adapter_Crossfire_Caps().
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]	iAdapterIndex The adapter in which to enable or disable CrossfireX
/// \param[in]	lpCrossfireComb the CrossfireX chain combination to enable or disable for the master adapter.  If this parameter is not NULL, it will
/// enable CrossfireX with the indicated combination.  If this parameter is NULL, it will disable CrossfireX on that particular adapter.
/// \return If the function succeeds, the return value is \ref ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_Crossfire_Set (ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLCrossfireComb *lpCrossfireComb);

///
/// \brief Function to set CrossfireX combination settings.
///
/// This function is used to enable or disable CrossfireX on a particular adapter (pointed to by iAdapterIndex), along with the indicated CrossfireX chain.
/// The CrossfireX chain combination for the adapter must be one returned by ADL_Adapter_Crossfire_Caps().
/// \platform
/// \ALL
/// \param[in]	iAdapterIndex The adapter in which to enable or disable CrossfireX
/// \param[in]	lpCrossfireComb the CrossfireX chain combination to enable or disable for the master adapter.  If this parameter is not NULL, it will
/// enable CrossfireX with the indicated combination.  If this parameter is NULL, it will disable CrossfireX on that particular adapter.
/// \return If the function succeeds, the return value is \ref ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Adapter_Crossfire_Set (int iAdapterIndex, ADLCrossfireComb *lpCrossfireComb);

///
/// \brief Function to set CrossfireX status.
///
/// This function is used to enable or disable CrossfireX on a particular adapter (pointed to by iAdapterIndex).
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]	iAdapterIndex The adapter in which to enable or disable CrossfireX
/// \param[in]	iState the requested CrossfireX state to be set
/// \return If the function succeeds, the return value is \ref ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_MVPU_Set (ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iState);

// @}


/// \addtogroup LINUX
// @{

///
/// \brief Function to retrieve memory information from the adapter.
/// 
/// This function retrieves the memory information for a specified graphics adapter.
/// \platform
/// \LNX
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpMemoryInfo The pointer to the structure storing the retrieved memory information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_MemoryInfo_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLMemoryInfo *lpMemoryInfo);

///
/// \brief Function to retrieve memory information from the adapter.
/// 
/// This function retrieves the memory information for a specified graphics adapter.
/// \platform
/// \LNX
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpMemoryInfo The pointer to the structure storing the retrieved memory information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Adapter_MemoryInfo_Get(int iAdapterIndex, ADLMemoryInfo *lpMemoryInfo);

///
///\brief Function to get the memory configuration of an adapter
///
/// This function retrieves the memory configuration of a specified adapter.\n
/// \platform
/// \LNX
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   iScreenWidth	The pixel width of the X screen associated with the adapter.
/// \param[in]   iScreenHeight	The pixel height of the X screen associated with the adapter.
/// \param[in]   displayFeatureMask	The valid \ref define_adl_visiblememoryfeatures features
/// \param[in]   numDisplays	 Number of displays associated with the XScreen.
/// \param[in]   displayFeatureList	List of display features associated with each display found in the XScreen. Number of entries is numDisplays.
/// \param[in]   iNumMemTypes	Size of  lppMemRequired List
/// \param[out]  lppMemRequired List of visible memory size required in bytes of specified type
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks This API uses the memory allocation callback function provided by the user. If this API is called in Windows the return value is always \ref ADL_ERR_NOT_SUPPORTED
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_ConfigMemory_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iScreenWidth, int iScreenHeight, int displayFeatureMask, int numDisplays, ADLMemoryDisplayFeatures * displayFeatureList, int *iNumMemTypes, ADLMemoryRequired **lppMemRequired);

///
///\brief Function to get the memory configuration of an adapter
///
/// This function retrieves the memory configuration of a specified adapter.\n
/// \platform
/// \LNX
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   iScreenWidth	The pixel width of the X screen associated with the adapter.
/// \param[in]   iScreenHeight	The pixel height of the X screen associated with the adapter.
/// \param[in]   displayFeatureMask	The valid \ref define_adl_visiblememoryfeatures features
/// \param[in]   numDisplays	 Number of displays associated with the XScreen.
/// \param[in]   displayFeatureList	List of display features associated with each display found in the XScreen. Number of entries is numDisplays.
/// \param[in]   iNumMemTypes	Size of  lppMemRequired List
/// \param[out]  lppMemRequired List of visible memory size required in bytes of specified type
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks This API uses the memory allocation callback function provided by the user. If this API is called in Windows the return value is always \ref ADL_ERR_NOT_SUPPORTED
ADL_EXTERNC int EXPOSED ADL_Adapter_ConfigMemory_Get(int iAdapterIndex, int iScreenWidth, int iScreenHeight, int displayFeatureMask, int numDisplays, ADLMemoryDisplayFeatures * displayFeatureList, int *iNumMemTypes, ADLMemoryRequired **lppMemRequired);

///
///\brief Function to get the core and memory clock info of an adapter.  This is the clock displayed on CCC information center.
///         Specific logic is used to select appropriate clock for display in current configuration.
///
/// This function retrieves the core and memory clock of an adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpCoreClock The pointer to the core clock retrieved from the driver in Mhz.
/// \param[out] lpMemoryClock The pointer to the memory clock retrieved from the driver in Mhz.
/// \return If the function succeeds, the return value is \ref ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_ObservedClockInfo_Get (ADL_CONTEXT_HANDLE context,int iAdapterIndex, int* lpCoreClock, int* lpMemoryClock);

///
///\brief Function to get the core and memory clock info of an adapter.  This is the clock displayed on CCC information center.
///         Specific logic is used to select appropriate clock for display in current configuration.
///
/// This function retrieves the core and memory clock of an adapter.
/// \platform
/// \ALL
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpCoreClock The pointer to the core clock retrieved from the driver in Mhz.
/// \param[out] lpMemoryClock The pointer to the memory clock retrieved from the driver in Mhz.
/// \return If the function succeeds, the return value is \ref ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Adapter_ObservedClockInfo_Get (int iAdapterIndex, int* lpCoreClock, int* lpMemoryClock);

// @}

/// \addtogroup EDIDAPI
// @{
///
///\brief Function to get the board layout information
/// 
/// This function retrieves the ASIC layout information of a specified adapter.\n
/// layout information includes number of slots, size and number of connectors and properties of connectors
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpValidFlags The integer pointer, specifies the valid bits of the slot & connector information.
/// \param[out]  lpNumberSlots The integer pointer, specifies the number of slots
/// \param[out]  lppBracketSlot The pointer to the structure, storing the bracket slot information.
/// \param[out]  lpNumberConnector The integer pointer, specifies the number of connectors in GPU
/// \param[out]  lppConnector The pointer to the structure, storing the connectors information
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
 ADL_EXTERNC int EXPOSED ADL2_Adapter_BoardLayout_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int* lpValidFlags, int *lpNumberSlots, ADLBracketSlotInfo** lppBracketSlot, int* lpNumberConnector, ADLConnectorInfo** lppConnector);

///
///\brief Function to get the board layout information
/// 
/// This function retrieves the ASIC layout information of a specified adapter.\n
/// layout information includes number of slots, size and number of connectors and properties of connectors
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpValidFlags The integer pointer, specifies the valid bits of the slot & connector information.
/// \param[out]  lpNumberSlots The integer pointer, specifies the number of slots
/// \param[out]  lppBracketSlot The pointer to the structure, storing the bracket slot information.
/// \param[out]  lpNumberConnector The integer pointer, specifies the number of connectors in GPU
/// \param[out]  lppConnector The pointer to the structure, storing the connectors information
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
 ADL_EXTERNC int EXPOSED ADL_Adapter_BoardLayout_Get(int iAdapterIndex, int* lpValidFlags, int *lpNumberSlots, ADLBracketSlotInfo** lppBracketSlot, int* lpNumberConnector, ADLConnectorInfo** lppConnector);


///
///\brief Function to get the supported connection types of given connector
/// 
/// This function retrieves the supported connection types and its properties
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   devicePort specifies connector index and relative MST address.
/// \param[out]  lpADLSupportedConnections The pointer to the structure storing the retrieved information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
 ADL_EXTERNC int EXPOSED ADL2_Adapter_SupportedConnections_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLDevicePort devicePort, ADLSupportedConnections* lpADLSupportedConnections);

///
///\brief Function to get the supported connection types of given connector
/// 
/// This function retrieves the supported connection types and its properties
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   devicePort specifies connector index and relative MST address.
/// \param[out]  lpADLSupportedConnections The pointer to the structure storing the retrieved information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
 ADL_EXTERNC int EXPOSED ADL_Adapter_SupportedConnections_Get(int iAdapterIndex, ADLDevicePort devicePort, ADLSupportedConnections* lpADLSupportedConnections);

///
///\brief Function to get the current emulation state of a given connector
/// 
/// This function retrieves the current emulation state
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   devicePort specifies connector index and relative MST address.
/// \param[out]  lpADLConnectionState The pointer to the structure storing the retrieved  information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
 ADL_EXTERNC int EXPOSED ADL2_Adapter_ConnectionState_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLDevicePort devicePort, ADLConnectionState* lpADLConnectionState);

///
///\brief Function to get the current emulation state of a given connector
/// 
/// This function retrieves the current emulation state
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   devicePort specifies connector index and relative MST address.
/// \param[out]  lpADLConnectionState The pointer to the structure storing the retrieved  information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
 ADL_EXTERNC int EXPOSED ADL_Adapter_ConnectionState_Get(int iAdapterIndex, ADLDevicePort devicePort, ADLConnectionState* lpADLConnectionState);

///
///\brief Function to sets the emulation mode of given connector
/// 
/// This function set the emulation mode to the driver
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   devicePort specifies connector index and relative MST address.
/// \param[in]	 iEmulationMode selected Emulated mode. \ref define_emulation_mode
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
 ADL_EXTERNC int EXPOSED ADL2_Adapter_EmulationMode_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLDevicePort devicePort, int iEmulationMode);

///
///\brief Function to sets the emulation mode of given connector
/// 
/// This function set the emulation mode to the driver
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   devicePort specifies connector index and relative MST address.
/// \param[in]	 iEmulationMode selected Emulated mode. \ref define_emulation_mode
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
 ADL_EXTERNC int EXPOSED ADL_Adapter_EmulationMode_Set(int iAdapterIndex, ADLDevicePort devicePort, int iEmulationMode);

///
///\brief Function to set the emulation data to on specified connector
/// 
/// This function sets the emulation data 
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   devicePort specifies connector index and relative MST address.
/// \param[in]   ConnectionData contains connection data including the EDID data
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
 ADL_EXTERNC int EXPOSED ADL2_Adapter_ConnectionData_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex,  ADLDevicePort devicePort, ADLConnectionData ConnectionData);

///
///\brief Function to set the emulation data to on specified connector
/// 
/// This function sets the emulation data 
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   devicePort specifies connector index and relative MST address.
/// \param[in]   ConnectionData contains connection data including the EDID data
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
 ADL_EXTERNC int EXPOSED ADL_Adapter_ConnectionData_Set(int iAdapterIndex,  ADLDevicePort devicePort, ADLConnectionData ConnectionData);

///
///\brief Function to gets the emulation data on specified connector
/// 
/// This function to gets the emulation data
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   devicePort specifies connector index and relative MST address.
/// \param[in]   iQueryType The Specifies type of query. \ref define_emulation_query
/// \param[out]  lpConnectionData The pointer to the structure storing the retrieved information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
 ADL_EXTERNC int EXPOSED ADL2_Adapter_ConnectionData_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex,  ADLDevicePort devicePort, int iQueryType, ADLConnectionData* lpConnectionData);

///
///\brief Function to gets the emulation data on specified connector
/// 
/// This function to gets the emulation data
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   devicePort specifies connector index and relative MST address.
/// \param[in]   iQueryType The Specifies type of query. \ref define_emulation_query
/// \param[out]  lpConnectionData The pointer to the structure storing the retrieved information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
 ADL_EXTERNC int EXPOSED ADL_Adapter_ConnectionData_Get(int iAdapterIndex,  ADLDevicePort devicePort, int iQueryType, ADLConnectionData* lpConnectionData);

///
///\brief Function to remove emulation on specified connector
/// 
/// This function removes the emulation which are emulated already
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   devicePort specifies connector index and relative mst address.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
 ADL_EXTERNC int EXPOSED ADL2_Adapter_ConnectionData_Remove(ADL_CONTEXT_HANDLE context,int iAdapterIndex,  ADLDevicePort devicePort);

///
///\brief Function to remove emulation on specified connector
/// 
/// This function removes the emulation which are emulated already
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]   devicePort specifies connector index and relative mst address.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
 ADL_EXTERNC int EXPOSED ADL_Adapter_ConnectionData_Remove(int iAdapterIndex,  ADLDevicePort devicePort);

///
///\brief Function to retrieve EDID management feature support.
/// 
/// This function retrieves EDID Management supprot for specific adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpSupported If the specified adapter supports EDID Management then returns \ref ADL_TRUE else \ref ADL_FALSE
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_EDIDManagement_Caps(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int *lpSupported);

///
///\brief Function to retrieve EDID management feature support.
/// 
/// This function retrieves EDID Management supprot for specific adapter.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpSupported If the specified adapter supports EDID Management then returns \ref ADL_TRUE else \ref ADL_FALSE
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Adapter_EDIDManagement_Caps(int iAdapterIndex, int *lpSupported);

///
/// \brief Function to get the EDID Persistence state of the system.
/// 
/// This function implements the functionality to get the EDID Persistence state of the system.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[out]  lpCurResultValue The int pointer to the current state
///              It can be one of \ref define_persistence_state
/// \param[out]  lpDefResultValue The int pointer to the default state
///              It can be one of \ref define_persistence_state
/// \return If the function is successful, \ref ADL_OK is returned. \ref ADL_OK_MODE_CHANGE if success but need mode change,\n
/// If the function is not supported, \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Workstation_GlobalEDIDPersistence_Get(ADL_CONTEXT_HANDLE context,int *lpCurResultValue, int *lpDefResultValue);

///
/// \brief Function to get the EDID Persistence state of the system.
/// 
/// This function implements the functionality to get the EDID Persistence state of the system.
/// \platform
/// \ALL
/// \param[out]  lpCurResultValue The int pointer to the current state
///              It can be one of \ref define_persistence_state
/// \param[out]  lpDefResultValue The int pointer to the default state
///              It can be one of \ref define_persistence_state
/// \return If the function is successful, \ref ADL_OK is returned. \ref ADL_OK_MODE_CHANGE if success but need mode change,\n
/// If the function is not supported, \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
ADL_EXTERNC int EXPOSED ADL_Workstation_GlobalEDIDPersistence_Get(int *lpCurResultValue, int *lpDefResultValue);

///
/// \brief Function to set the EDID Persistence  state of the system.
/// 
/// This function implements the functionality to set the EDID Persistence mode for a system.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iCurState The current state to set for the specified adapter. \ref define_persistence_state
/// \return If the function is successful, \ref ADL_OK is returned. \ref ADL_OK_MODE_CHANGE if success but need mode change,\n
/// If the function is not supported, \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Workstation_GlobalEDIDPersistence_Set(ADL_CONTEXT_HANDLE context,int iCurState);

///
/// \brief Function to set the EDID Persistence  state of the system.
/// 
/// This function implements the functionality to set the EDID Persistence mode for a system.
/// \platform
/// \ALL
/// \param[in]  iCurState The current state to set for the specified adapter. \ref define_persistence_state 
/// \return If the function is successful, \ref ADL_OK is returned. \ref ADL_OK_MODE_CHANGE if success but need mode change,\n
/// If the function is not supported, \ref ADL_ERR_NOT_SUPPORTED is returned. For all other errors, \ref ADL_ERR is returned.
ADL_EXTERNC int EXPOSED ADL_Workstation_GlobalEDIDPersistence_Set(int iCurState);

// @}

/// \addtogroup FPS
///
// @{
#if defined (_WIN32) || defined(_WIN64)
/// 
/// \brief Function to retrieve FPS Global Setting Capability.
///
/// This function retrieves the Global Setting Capability.
/// \platform
/// \WIN
/// \param[in] context			: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapterIndex    : The ADL index handle of the desired adapter.
/// \param[out] lpSupported		: Integer indicating capability of the FPS Feature Support. 
/// \param[out] lpVersion	    : Integer indicating Feature Version.
ADL_EXTERNC int EXPOSED ADL2_FPS_Caps (ADL_CONTEXT_HANDLE context,int iAdapterIndex, int *lpSupported, int *lpVersion);

/// 
/// \brief Function to retrieve FPS Global Settings.
///
/// This function retrieves FPS Global Settings.
/// \platform
/// \WIN
/// \param[in] context			: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapterIndex    : The ADL index handle of the desired adapter.
/// \param[out] lpFPSSettings	: Pointer to ADLFPSSettingsOutput struct which will load the Global FPS Settings 
ADL_EXTERNC int EXPOSED ADL2_FPS_Settings_Get (ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLFPSSettingsOutput *lpFPSSettings);

/// 
/// \brief Function to update FPS Global Settings.
///
/// This function update FPS Global Settings.
/// \platform
/// \WIN
/// \param[in] context			: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapterIndex    : The ADL index handle of the desired adapter.
/// \param[in] lpFPSSettings	: ADLFPSSettingsInput struct which will update the Global FPS Settings 
ADL_EXTERNC int EXPOSED ADL2_FPS_Settings_Set (ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLFPSSettingsInput lpFPSSettings);

/// 
/// \brief Function to reset FPS Global Settings.
///
/// This function reset FPS Global Settings.
/// \platform
/// \WIN
/// \param[in] context			: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapterIndex    : The ADL index handle of the desired adapter.
ADL_EXTERNC int EXPOSED ADL2_FPS_Settings_Reset (ADL_CONTEXT_HANDLE context,int iAdapterIndex);

#endif /*(_WIN32) || (_WIN64)*/
// @}

// Deprecated APIs

///
/// \brief Function to retrieve clock information for an adapter.
/// 
/// This function retrieves the clock information for a specified graphics adapter.
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpClockInfo The pointer to the structure storing the retrieved core and memory clock information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results \n
/// \deprecated This API has been deprecated because it does not provide accurate clocks when the ASIC is over-clocked. Use the OD5 set of APIs, when OverDrive5 is supported.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Adapter_ClockInfo_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLClockInfo* lpClockInfo);

///
/// \brief Function to retrieve clock information for an adapter.
/// 
/// This function retrieves the clock information for a specified graphics adapter.
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpClockInfo The pointer to the structure storing the retrieved core and memory clock information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results \n
/// \deprecated This API has been deprecated because it does not provide accurate clocks when the ASIC is over-clocked. Use the OD5 set of APIs, when OverDrive5 is supported.
ADL_EXTERNC int EXPOSED ADL_Adapter_ClockInfo_Get(int iAdapterIndex, ADLClockInfo* lpClockInfo);

///
///\brief Function to get the unique identifier of an adapter. Will be removed! Use ADL_Adapter_ID_Get()
/// 
/// This function retrieves the unique identifier of a specified adapter.\n
/// The adapter ID is a unique value and will be used to determine what other controllers share the same adapter.\n
/// The desktop will use this to find which HDCs are associated with an adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpAdapterID The pointer to the adapter identifier. Zero means: The adapter is not AMD.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \deprecated This API will be removed. Use the duplicate API ADL_Adapter_ID_Get() 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
 ADL_EXTERNC int EXPOSED ADL2_Display_AdapterID_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int* lpAdapterID);

///
///\brief Function to get the unique identifier of an adapter. Will be removed! Use ADL_Adapter_ID_Get()
/// 
/// This function retrieves the unique identifier of a specified adapter.\n
/// The adapter ID is a unique value and will be used to determine what other controllers share the same adapter.\n
/// The desktop will use this to find which HDCs are associated with an adapter.
/// \platform
/// \ALL
/// \param[in]   iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpAdapterID The pointer to the adapter identifier. Zero means: The adapter is not AMD.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \deprecated This API will be removed. Use the duplicate API ADL_Adapter_ID_Get() 
 ADL_EXTERNC int EXPOSED ADL_Display_AdapterID_Get(int iAdapterIndex, int* lpAdapterID);

/// \brief Function to retrieve Gfx EDC Error Log.
/// 
 /// This function implements a call to retrieve Gfx EDC Error Logs.
 /// \platform
 /// \VISTAWIN7
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] pErrorrecordCount The pointer to the number of error records
/// \param[out] errorRecords The pointer to error records
/// \return If the function succeeds, the return value is \ref ADL_OK, or an ADL error code otherwise. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL2_Adapter_EDC_ErrorRecords_Get (ADL_CONTEXT_HANDLE context,int iAdapterIndex, int* pErrorrecordCount, ADLErrorRecord* errorRecords); 

/// \brief Function to inject Gfx EDC Error .
/// 
 /// This function implements a call to inject Gfx EDC Error.
 /// \platform
 /// \VISTAWIN7
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in] errorInjection The pointer to error injection
/// \return If the function succeeds, the return value is \ref ADL_OK, or an ADL error code otherwise. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL2_Adapter_EDC_ErrorInjection_Set (ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLErrorInjection* errorInjection);

/// \brief Function to retrieve Graphic Core Info.
/// 
/// This function implements a call to retrieve Graphic Core Info.
/// \platform
/// \VISTAWIN7
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] pGraphicCoreInfo The pointer to Graphic Core Info
/// \return If the function succeeds, the return value is \ref ADL_OK, or an ADL error code otherwise. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL2_Adapter_Graphic_Core_Info_Get (ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLGraphicCoreInfo* pGraphicCoreInfo);

#endif /* ADAPTER_H_ */

