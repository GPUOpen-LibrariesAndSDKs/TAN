///
///  Copyright (c) 2016 Advanced Micro Devices, Inc.
 
///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

/// \file overdriveN.h
/// \brief Contains all OverdriveN-related functions exposed by ADL for \ALL platforms.
///
/// This file contains all OverdriveN-related functions exposed by ADL for \ALL platforms.
/// All functions found in this file can be used as a reference to ensure
/// the specified function pointers can be used by the appropriate runtime
/// dynamic library loaders.

#ifndef OVERDRIVEN_H_
#define OVERDRIVEN_H_

#include "..\adl_structures.h"

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

/// \addtogroup OVERDRIVEN
// @{
///
/// \brief Function to retrieve the OverdriveN capabilities.
/// 
/// This function retrieves the OverdriveN capabilities for a specified adapter.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpODCapabilities The pointer to the \ref ADLODNCapabilities structure storing the retrieved Overdrive parameters. 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
/// \deprecated This API has been deprecated.
ADL_EXTERNC int EXPOSED ADL2_OverdriveN_Capabilities_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLODNCapabilities *lpODCapabilities);

///
/// \brief Function to retrieve the OverdriveN capabilities.
/// 
/// This function retrieves the OverdriveN capabilities for a specified adapter.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpODCapabilities The pointer to the \ref ADLODNCapabilitiesX2 structure storing the retrieved Overdrive parameters. 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_OverdriveN_CapabilitiesX2_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLODNCapabilitiesX2 *lpODCapabilities);

///
/// \brief Function to retrieve the current GPU clocks settings
/// 
/// This function retrieves the current GPU clocks for a specified adapter.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpODPerformanceLevels The pointer to the \ref ADLODNPerformanceLevels structure storing the retrieved data. 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
/// \deprecated This API has been deprecated.
ADL_EXTERNC int EXPOSED ADL2_OverdriveN_SystemClocks_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLODNPerformanceLevels *lpODPerformanceLevels);

///
/// \brief Function to sets the current GPU clocks settings
/// 
/// This function applies the current GPU clocks for a specified adapter.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in] lpODPerformanceLevels The pointer to the \ref ADLODNPerformanceLevels structure to set the data to driver. 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
/// \deprecated This API has been deprecated.
ADL_EXTERNC int EXPOSED ADL2_OverdriveN_SystemClocks_Set(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLODNPerformanceLevels *lpODPerformanceLevels);


///
/// \brief Function to retrieve the current GPU clocks settings
/// 
/// This function retrieves the current GPU clocks for a specified adapter.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpODPerformanceLevels The pointer to the \ref ADLODNPerformanceLevels structure storing the retrieved data. 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_OverdriveN_SystemClocksX2_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLODNPerformanceLevelsX2 *lpODPerformanceLevels);

///
/// \brief Function to sets the current GPU clocks settings
/// 
/// This function applies the current GPU clocks for a specified adapter.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in] lpODPerformanceLevels The pointer to the \ref ADLODNPerformanceLevels structure to set the data to driver. 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_OverdriveN_SystemClocksX2_Set(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLODNPerformanceLevelsX2 *lpODPerformanceLevels);

///
/// \brief Function to retrieve the current memory clocks settings
/// 
/// This function retrieves the current memory clocks for a specified adapter.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpODPerformanceLevels The pointer to the \ref ADLODNPerformanceLevels structure storing the retrieved Overdrive parameters. 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_OverdriveN_MemoryClocksX2_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLODNPerformanceLevelsX2 *lpODPerformanceLevels);

///
/// \brief Function to sets the current memory clocks settings
/// 
/// This function applies the current memory clocks for a specified adapter.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in] lpODPerformanceLevels The pointer to the \ref ADLODNPerformanceLevels structure to set the data to driver. 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_OverdriveN_MemoryClocksX2_Set(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLODNPerformanceLevelsX2 *lpODPerformanceLevels);

///
/// \brief Function to retrieve the current memory clocks settings
/// 
/// This function retrieves the current memory clocks for a specified adapter.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpODPerformanceLevels The pointer to the \ref ADLODNPerformanceLevels structure storing the retrieved Overdrive parameters. 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
/// \deprecated This API has been deprecated.
ADL_EXTERNC int EXPOSED ADL2_OverdriveN_MemoryClocks_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLODNPerformanceLevels *lpODPerformanceLevels);

///
/// \brief Function to sets the current memory clocks settings
/// 
/// This function applies the current memory clocks for a specified adapter.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in] iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in] lpODPerformanceLevels The pointer to the \ref ADLODNPerformanceLevels structure to set the data to driver. 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
/// \deprecated This API has been deprecated.
ADL_EXTERNC int EXPOSED ADL2_OverdriveN_MemoryClocks_Set(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLODNPerformanceLevels *lpODPerformanceLevels);

///
/// \brief Function to retrieve the current Fan control settings
/// 
/// This function retrieves the current fan for a specified adapter.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpOdFanSpeed The pointer to the \ref ADLODNFanControl structure storing the retrieved Overdrive parameters. 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_OverdriveN_FanControl_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLODNFanControl *lpODFanSpeed);

///
/// \brief Function to set the current Fan controls settings
/// 
/// This function applies the current Fan control for a specified adapter.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in] lpOdFanSpeed The pointer to the \ref ADLODNFanControl structure storing the retrieved Overdrive parameters. 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_OverdriveN_FanControl_Set(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLODNFanControl *lpODFanControl);

///
/// \brief Function to retrieve the current power limit settings
/// 
/// This function retrieves the current power limit for a specified adapter.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpOdPowerLimit The pointer to the \ref ADLODPowerLimitSetting structure storing the retrieved Overdrive parameters. 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_OverdriveN_PowerLimit_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLODNPowerLimitSetting *lpODPowerLimit);

///
/// \brief Function to sets the current power limit settings
/// 
/// This function retrieves the current power limit for a specified adapter.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in] lpOdPowerLimit The pointer to the \ref ADLODPowerLimitSetting structure storing the retrieved Overdrive parameters. 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_OverdriveN_PowerLimit_Set(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLODNPowerLimitSetting *lpODPowerLimit);

///
/// \brief Function to retrieve the current temperture
/// 
/// This function retrieves the current temperature for a specified adapter.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in] iTemperatureType, temperature type. \ref ADLODNTemperatureType
/// \param[out] iTemperature The integer pointer, storing the retrieved Overdrive parameters. 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_OverdriveN_Temperature_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iTemperatureType, int *iTemperature);

///
/// \brief Function to retrieve the current OD performance status
/// 
/// This function retrieves the current OD performance for a specified adapter.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpOdPerformanceStatus The pointer to the \ref ADLODNPerformanceStatus structure storing the retrieved Overdrive parameters. 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_OverdriveN_PerformanceStatus_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLODNPerformanceStatus *lpODPerformanceStatus);

// @}

#endif /* OVERDRIVEN_H_ */
