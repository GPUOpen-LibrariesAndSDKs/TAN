///
///  Copyright (c) 2012 Advanced Micro Devices, Inc.
 
///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

/// \file overdrive6.h
/// \brief Contains all Overdrive6-related functions exposed by ADL for \ALL platforms.
///
/// This file contains all Overdrive6-related functions exposed by ADL for \ALL platforms.
/// All functions found in this file can be used as a reference to ensure
/// the specified function pointers can be used by the appropriate runtime
/// dynamic library loaders.

#ifndef OVERDRIVE6_H_
#define OVERDRIVE6_H_

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

/// \addtogroup OVERDRIVE6
// @{

///
/// \brief Function to retrieve the current Overdrive capabilities.
/// 
/// This function retrieves the current Overdrive capabilities for a specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in,out] lpODCapabilities The pointer to the \ref ADLOD6Capabilities structure storing the retrieved Overdrive parameters. 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive6_Capabilities_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLOD6Capabilities *lpODCapabilities);

///
/// \brief Function to retrieve the current Overdrive capabilities.
/// 
/// This function retrieves the current Overdrive capabilities for a specified adapter.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in,out] lpODCapabilities The pointer to the \ref ADLOD6Capabilities structure storing the retrieved Overdrive parameters. 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 

ADL_EXTERNC int EXPOSED ADL_Overdrive6_Capabilities_Get(int iAdapterIndex, ADLOD6Capabilities *lpODCapabilities);

///
/// \brief Function to retrieve the current or default Overdrive clock ranges.
/// 
/// This function retrieves the current or default Overdrive clock ranges for a specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iStateType Specifies if default or current clock ranges for the power state will be retrieved.  Currently only performance state is supported.  Possible Values: \ref ADL_OD6_GETSTATEINFO_DEFAULT_PERFORMANCE, \ref ADL_OD6_GETSTATEINFO_CUSTOM_PERFORMANCE, \ref ADL_OD6_GETSTATEINFO_CURRENT
/// \param[in,out] lpStateInfo Pointer to the retrieved clock range info structure \ref ADLOD6StateInfo.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive6_StateInfo_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iStateType, ADLOD6StateInfo *lpStateInfo);

///
/// \brief Function to retrieve the current or default Overdrive clock ranges.
/// 
/// This function retrieves the current or default Overdrive clock ranges for a specified adapter.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iStateType Specifies if default or current clock ranges for the power state will be retrieved.  Currently only performance state is supported.  Possible Values: \ref ADL_OD6_GETSTATEINFO_DEFAULT_PERFORMANCE, \ref ADL_OD6_GETSTATEINFO_CUSTOM_PERFORMANCE, \ref ADL_OD6_GETSTATEINFO_CURRENT
/// \param[in,out] lpStateInfo Pointer to the retrieved clock range info structure \ref ADLOD6StateInfo.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 

ADL_EXTERNC int EXPOSED ADL_Overdrive6_StateInfo_Get(int iAdapterIndex, int iStateType, ADLOD6StateInfo *lpStateInfo);

///
/// \brief Function to set the current Overdrive clock ranges.
/// 
/// This function sets the current Overdrive clock ranges for a specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iStateType The power state which will have clock ranges changed.  Currently only performance state is supported. Possible Values: \ref ADL_OD6_SETSTATE_PERFORMANCE
/// \param[in]  lpStateInfo Pointer to the structure \ref ADLOD6StateInfo containing clock ranges to be set.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive6_State_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iStateType, ADLOD6StateInfo *lpStateInfo);

///
/// \brief Function to set the current Overdrive clock ranges.
/// 
/// This function sets the current Overdrive clock ranges for a specified adapter.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iStateType The power state which will have clock ranges changed.  Currently only performance state is supported. Possible Values: \ref ADL_OD6_SETSTATE_PERFORMANCE
/// \param[in]  lpStateInfo Pointer to the structure \ref ADLOD6StateInfo containing clock ranges to be set.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 

ADL_EXTERNC int EXPOSED ADL_Overdrive6_State_Set(int iAdapterIndex, int iStateType, ADLOD6StateInfo *lpStateInfo);

///
/// \brief Function to reset the Overdrive clock ranges to default.
/// 
/// This function resets the Overdrive clock ranges to default for a specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iStateType The power state which will be reset to default clock ranges.  Currently only performance state is supported. Possible Values: \ref ADL_OD6_SETSTATE_PERFORMANCE
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive6_State_Reset(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iStateType);

///
/// \brief Function to reset the Overdrive clock ranges to default.
/// 
/// This function resets the Overdrive clock ranges to default for a specified adapter.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iStateType The power state which will be reset to default clock ranges.  Currently only performance state is supported. Possible Values: \ref ADL_OD6_SETSTATE_PERFORMANCE
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 

ADL_EXTERNC int EXPOSED ADL_Overdrive6_State_Reset(int iAdapterIndex, int iStateType);

///
/// \brief Function to retrieve current Overdrive and performance-related activity.
/// 
/// This function retrieves current Overdrive and performance-related activity for a specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in,out] lpCurrentStatus The pointer to the \ref ADLOD6CurrentStatus structure storing the retrieved activity information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive6_CurrentStatus_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLOD6CurrentStatus *lpCurrentStatus);

///
/// \brief Function to retrieve current Overdrive and performance-related activity.
/// 
/// This function retrieves current Overdrive and performance-related activity for a specified adapter.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in,out] lpCurrentStatus The pointer to the \ref ADLOD6CurrentStatus structure storing the retrieved activity information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 

ADL_EXTERNC int EXPOSED ADL_Overdrive6_CurrentStatus_Get(int iAdapterIndex, ADLOD6CurrentStatus *lpCurrentStatus);

///
/// \brief Function to retrieve capabilities of the GPU thermal controller.
/// 
/// This function retrieves GPU thermal controller capabilities for a specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in,out] lpThermalControllerInfo The pointer to the \ref ADLThermalControllerInfo structure storing the retrieved GPU thermal controller information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive6_ThermalController_Caps(ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLOD6ThermalControllerCaps *lpThermalControllerCaps);

///
/// \brief Function to retrieve capabilities of the GPU thermal controller.
/// 
/// This function retrieves GPU thermal controller capabilities for a specified adapter.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in,out] lpThermalControllerInfo The pointer to the \ref ADLThermalControllerInfo structure storing the retrieved GPU thermal controller information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 

ADL_EXTERNC int EXPOSED ADL_Overdrive6_ThermalController_Caps(int iAdapterIndex, ADLOD6ThermalControllerCaps *lpThermalControllerCaps);

///
/// \brief Function to retrieve GPU temperature from the thermal controller.
/// 
/// This function retrieves GPU temperature from the thermal controller for a specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in,out] lpTemperature The pointer to the retrieved temperature in milli-degrees Celsius.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive6_Temperature_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int *lpTemperature);

///
/// \brief Function to retrieve GPU temperature from the thermal controller.
/// 
/// This function retrieves GPU temperature from the thermal controller for a specified adapter.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in,out] lpTemperature The pointer to the retrieved temperature.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 

ADL_EXTERNC int EXPOSED ADL_Overdrive6_Temperature_Get(int iAdapterIndex, int *lpTemperature);

///
/// \brief Function to retrieve the fan speed reported by the thermal controller.
/// 
/// This function retrieves the reported fan speed from a specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in,out] lpFanSpeedInfo The pointer to the \ref ADLOD6FanSpeedInfo structure storing the retrieved fan speed.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive6_FanSpeed_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLOD6FanSpeedInfo *lpFanSpeedInfo);

///
/// \brief Function to retrieve the fan speed reported by the thermal controller.
/// 
/// This function retrieves the reported fan speed from a specified adapter.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in,out] lpFanSpeedInfo The pointer to the \ref ADLOD6FanSpeedInfo structure storing the retrieved fan speed.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 

ADL_EXTERNC int EXPOSED ADL_Overdrive6_FanSpeed_Get(int iAdapterIndex, ADLOD6FanSpeedInfo *lpFanSpeedInfo);

///
/// \brief Function to set the fan speed.
/// 
/// This function sets the fan speed for a specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  lpFanSpeedValue The pointer to the \ref ADLOD6FanSpeedValue structure containing the new fan speed value to be set.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive6_FanSpeed_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLOD6FanSpeedValue *lpFanSpeedValue);

///
/// \brief Function to set the fan speed.
/// 
/// This function sets the fan speed for a specified adapter.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  lpFanSpeedValue The pointer to the \ref ADLOD6FanSpeedValue structure containing the new fan speed value to be set.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 

ADL_EXTERNC int EXPOSED ADL_Overdrive6_FanSpeed_Set(int iAdapterIndex, ADLOD6FanSpeedValue *lpFanSpeedValue);

///
/// \brief Function to reset the fan speed to the default.
/// 
/// This function resets the fan speed for a specified adapter to the default.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive6_FanSpeed_Reset(ADL_CONTEXT_HANDLE context,int iAdapterIndex);

///
/// \brief Function to reset the fan speed to the default.
/// 
/// This function resets the fan speed for a specified adapter to the default.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 

ADL_EXTERNC int EXPOSED ADL_Overdrive6_FanSpeed_Reset(int iAdapterIndex);

/// \brief Function to check for PowerControl capabilities.
///
/// This function returns the PowerControl capability of the specified adapter.  PowerControl is the feature which allows the 
/// performance characteristics of the GPU to be adjusted by changing the PowerTune power limits.  Increasing the PowerControl value
/// can increase maximum performance, but may cause more power to be consumed.  Decreasing the PowerControl value can lower power consumption,
/// but may cause lower performance.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpSupported The pointer to a value that receives the PowerControl capability of the adapter.  Non-zero if PowerControl is supported.
/// \return If the function succeeds, the return value is \ref ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive6_PowerControl_Caps (ADL_CONTEXT_HANDLE context,int iAdapterIndex, int *lpSupported);

/// \brief Function to check for PowerControl capabilities.
///
/// This function returns the PowerControl capability of the specified adapter.  PowerControl is the feature which allows the 
/// performance characteristics of the GPU to be adjusted by changing the PowerTune power limits.  Increasing the PowerControl value
/// can increase maximum performance, but may cause more power to be consumed.  Decreasing the PowerControl value can lower power consumption,
/// but may cause lower performance.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpSupported The pointer to a value that receives the PowerControl capability of the adapter.  Non-zero if PowerControl is supported.
/// \return If the function succeeds, the return value is \ref ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results 

ADL_EXTERNC int EXPOSED ADL_Overdrive6_PowerControl_Caps (int iAdapterIndex, int *lpSupported);

/// \brief Function to get the PowerControl adjustment range.
/// 
/// This function returns the PowerControl adjustment range (in percentage) for the specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle  of the desired adapter.
/// \param[in,out]  lpPowerControlInfo The pointer to the \ref ADLOD6PowerControlInfo returning the PowerControl adjustment range. 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive6_PowerControlInfo_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLOD6PowerControlInfo *lpPowerControlInfo);

/// \brief Function to get the PowerControl adjustment range.
/// 
/// This function returns the PowerControl adjustment range (in percentage) for the specified adapter.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle  of the desired adapter.
/// \param[in,out]  lpPowerControlInfo The pointer to the \ref ADLOD6PowerControlInfo returning the PowerControl adjustment range. 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. 

ADL_EXTERNC int EXPOSED ADL_Overdrive6_PowerControlInfo_Get(int iAdapterIndex, ADLOD6PowerControlInfo *lpPowerControlInfo);

///  \brief Function to get the current and default PowerControl adjustment values 
///
/// This function returns the current and default PowerControl adjustment values for the specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle  of the desired adapter. 
/// \param[out]  lpCurrentValue pointer to the current PowerControl adjustment value. 
/// \param[out]  lpDefaultValue pointer to the default PowerControl adjustment value. 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error  code. 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive6_PowerControl_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int *lpCurrentValue, int *lpDefaultValue);

///  \brief Function to get the current and default PowerControl adjustment values 
///
/// This function returns the current and default PowerControl adjustment values for the specified adapter.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle  of the desired adapter. 
/// \param[out]  lpCurrentValue pointer to the current PowerControl adjustment value. 
/// \param[out]  lpDefaultValue pointer to the default PowerControl adjustment value. 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error  code. 

ADL_EXTERNC int EXPOSED ADL_Overdrive6_PowerControl_Get(int iAdapterIndex, int *lpCurrentValue, int *lpDefaultValue);

///  \brief Function to set the current PowerControl adjustment value.
///
/// This function sets the current PowerControl adjustment value.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter. 
/// \param[in]  iValue The new PowerControl adjustment value to be set.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error  code. 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive6_PowerControl_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iValue);

///  \brief Function to set the current PowerControl adjustment value.
///
/// This function sets the current PowerControl adjustment value.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter. 
/// \param[in]  iValue The new PowerControl adjustment value to be set.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error  code. 
ADL_EXTERNC int EXPOSED ADL_Overdrive6_PowerControl_Set(int iAdapterIndex, int iValue);


/// \brief Function to get the VoltageControl adjustment range.
///
/// This function returns the VoltageControl adjustment range for the specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle  of the desired adapter.
/// \param[in,out]  lpVoltageControlInfo The pointer to the \ref ADLOD6VoltageControlInfo returning the VoltageControl adjustment range.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive6_VoltageControlInfo_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLOD6VoltageControlInfo *lpVoltageControlInfo);


/// \brief Function to get the VoltageControl adjustment range.
///
/// This function returns the VoltageControl adjustment range for the specified adapter.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle  of the desired adapter.
/// \param[in,out]  lpVoltageControlInfo The pointer to the \ref ADLOD6VoltageControlInfo returning the VoltageControl adjustment range.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code.

ADL_EXTERNC int EXPOSED ADL_Overdrive6_VoltageControlInfo_Get(int iAdapterIndex, ADLOD6VoltageControlInfo *lpVoltageControlInfo);

///  \brief Function to get the current and default VoltageControl adjustment values
///
/// This function returns the current and default VoltageControl adjustment values for the specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle  of the desired adapter.
/// \param[out]  lpCurrentValue pointer to the current VoltageControl adjustment value.
/// \param[out]  lpDefaultValue pointer to the default VoltageControl adjustment value.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error  code.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive6_VoltageControl_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int *lpCurrentValue, int *lpDefaultValue);


///  \brief Function to get the current and default VoltageControl adjustment values
///
/// This function returns the current and default VoltageControl adjustment values for the specified adapter.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle  of the desired adapter.
/// \param[out]  lpCurrentValue pointer to the current VoltageControl adjustment value.
/// \param[out]  lpDefaultValue pointer to the default VoltageControl adjustment value.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error  code.
ADL_EXTERNC int EXPOSED ADL_Overdrive6_VoltageControl_Get(int iAdapterIndex, int *lpCurrentValue, int *lpDefaultValue);

///  \brief Function to set the current VoltageControl adjustment value.
///
/// This function sets the current VoltageControl adjustment value.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iValue The new VoltageControl adjustment value to be set.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error  code.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive6_VoltageControl_Set(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iValue);


///  \brief Function to set the current VoltageControl adjustment value.
///
/// This function sets the current VoltageControl adjustment value.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iValue The new VoltageControl adjustment value to be set.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error  code.

ADL_EXTERNC int EXPOSED ADL_Overdrive6_VoltageControl_Set(int iAdapterIndex, int iValue);

///
/// \brief Function to retrieve the current Overdrive extension capabilities .
/// 
/// This function retrieves the current Overdrive extension capabilities for a specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in,out] lpODCapabilities The pointer to the \ref ADLOD6CapabilitiesEx structure storing the retrieved Overdrive parameters. 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive6_CapabilitiesEx_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLOD6CapabilitiesEx *lpODCapabilities);

///
/// \brief Function to retrieve the current Overdrive extension capabilities.
/// 
/// This function retrieves the current Overdrive extension capabilities for a specified adapter.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in,out] lpODCapabilities The pointer to the \ref ADLOD6CapabilitiesEx structure storing the retrieved Overdrive parameters. 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Overdrive6_CapabilitiesEx_Get(int iAdapterIndex, ADLOD6CapabilitiesEx *lpODCapabilities);

///
/// \brief Function to retrieve the current or default Overdrive extension clock ranges.
/// 
/// This function retrieves the current or default Overdrive extension clock ranges for a specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iStateType Specifies if default or current clock ranges for the power state will be retrieved.  Currently only performance state is supported.  Possible Values: \ref ADL_OD6_GETSTATEINFO_DEFAULT_PERFORMANCE, \ref ADL_OD6_GETSTATEINFO_CUSTOM_PERFORMANCE, \ref ADL_OD6_GETSTATEINFO_CURRENT
/// \param[in,out] lpStateInfo Pointer to the retrieved clock range info structure \ref ADLOD6StateEx.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive6_StateEx_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iStateType, ADLOD6StateEx *lpODState);

///
/// \brief Function to retrieve the current or default Overdrive extension clock ranges.
/// 
/// This function retrieves the current or default Overdrive extension clock ranges for a specified adapter.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iStateType Specifies if default or current clock ranges for the power state will be retrieved.  Currently only performance state is supported.  Possible Values: \ref ADL_OD6_GETSTATEINFO_DEFAULT_PERFORMANCE, \ref ADL_OD6_GETSTATEINFO_CUSTOM_PERFORMANCE, \ref ADL_OD6_GETSTATEINFO_CURRENT
/// \param[in,out] lpStateInfo Pointer to the retrieved clock range info structure \ref ADLOD6StateEx.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Overdrive6_StateEx_Get(int iAdapterIndex, int iStateType, ADLOD6StateEx *lpODState);

///
/// \brief Function to set the current Overdrive extension clock ranges.
/// 
/// This function sets the current Overdrive extension clock ranges for a specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iStateType The power state which will have clock ranges changed.  Currently only performance state is supported. Possible Values: \ref ADL_OD6_SETSTATE_PERFORMANCE
/// \param[in]  lpStateInfo Pointer to the structure \ref ADLOD6StateEx containing clock ranges to be set.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive6_StateEx_Set(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iStateType, ADLOD6StateEx *lpODState);

///
/// \brief Function to set the current Overdrive extension clock ranges.
/// 
/// This function sets the current Overdrive extension clock ranges for a specified adapter.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iStateType The power state which will have clock ranges changed.  Currently only performance state is supported. Possible Values: \ref ADL_OD6_SETSTATE_PERFORMANCE
/// \param[in]  lpStateInfo Pointer to the structure \ref ADLOD6StateEx containing clock ranges to be set.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Overdrive6_StateEx_Set(int iAdapterIndex, int iStateType, ADLOD6StateEx *lpODState);

///  \brief Function to enable the current Thermal Limit Unlock feature.
///
/// This function enable the current Thermal Limit Unlock feature.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]    iStateType The power state which will have clock ranges changed.  Currently only performance state is supported. Possible Values: \ref ADL_OD6_SETSTATE_PERFORMANCE
/// \param[in]  iEnable:  1 - Enable TLU, 0 - Disable TLU.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Overdrive6_ThermalLimitUnlock_Set(int iAdapterIndex, int iStateType, int iEnable);

///  \brief Function to enable the current Thermal Limit Unlock feature.
///
/// This function enable the current Thermal Limit Unlock feature.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]    iStateType The power state which will have clock ranges changed.  Currently only performance state is supported. Possible Values: \ref ADL_OD6_SETSTATE_PERFORMANCE
/// \param[in]  iEnable:  1 - Enable TLU, 0 - Disable TLU.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error  code.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive6_ThermalLimitUnlock_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iStateType, int iEnable);

///  \brief Function to retrieve the current Thermal Limit Unlock feature status.
///
/// This function to retrieve the current Thermal Limit Unlock feature status.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]    iStateType The power state which will have clock ranges changed.  Currently only performance state is supported. Possible Values: \ref ADL_OD6_SETSTATE_PERFORMANCE
/// \param[out]  pEnable: a pointer to an int containing the current TLU status. Possible Values: 1 - Enable TLU, 0 - Disable TLU.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Overdrive6_ThermalLimitUnlock_Get(int iAdapterIndex, int iStateType, int* pEnabled);

///  \brief Function to retrieve the current Thermal Limit Unlock feature status.
///
/// This function to retrieve the current Thermal Limit Unlock feature status.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]    iStateType The power state which will have clock ranges changed.  Currently only performance state is supported. Possible Values: \ref ADL_OD6_SETSTATE_PERFORMANCE
/// \param[out]  pEnable: a pointer to an int containing the current TLU status. Possible Values: 1 - Enable TLU, 0 - Disable TLU.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error  code.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive6_ThermalLimitUnlock_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iStateType, int* pEnabled);

///  \brief Function returns the advanced fan control capability of the specified adapter.
/// Advanced fan control is the feature which makes the fan speed is always kept to a minimum within current settings so the acoustics are also minimized.
/// The end users are able to select the target ASIC temperature and fan PWM% via the CCC or other applications.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpSupported: a pointer to an int containing the current advanced fan control(AFC) feature status. Possible Values: 1 - AFC Enabled, 0 - AFC Disabled.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error  code.
ADL_EXTERNC int EXPOSED ADL_Overdrive6_AdvancedFan_Caps (int iAdapterIndex, int *lpSupported);

///  \brief Function returns the advanced fan control capability of the specified adapter.
/// Advanced fan control is the feature which makes the fan speed is always kept to a minimum so the acoustics are also minimized.
/// The end user will be able to select the target ASIC temperature and fan PWM% via the CCC or other applications.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpSupported: a pointer to an int containing the current advanced fan control(AFC) feature status. Possible Values: 1 - AFC Enabled, 0 - AFC Disabled.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error  code.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process and to assure that ADL API are thread-safe. Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive6_AdvancedFan_Caps (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int *lpSupported);

///  \brief Function returns the target temperature range of the specified adapter .
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpTargetTemperatureInfo: a pointer to an ADLOD6ParameterRange structure containing acceptable temperature range to work with.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error  code.
ADL_EXTERNC int EXPOSED ADL_Overdrive6_TargetTemperatureRangeInfo_Get(int iAdapterIndex, ADLOD6ParameterRange *lpTargetTemperatureInfo);

///  \brief Function returns the target temperature range of the specified adapter .
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpTargetTemperatureInfo: a pointer to an ADLOD6ParameterRange structure containing acceptable temperature range to work with.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error  code.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process and to assure that ADL API are thread-safe. Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive6_TargetTemperatureRangeInfo_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLOD6ParameterRange *lpTargetTemperatureInfo);

///  \brief Function returns default target temperature and current temp. value of the specified adapter.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpDefaultValue: a pointer to an int that will contain default temperature value for specified adapter.
/// \param[out]  lpCurrentValue: a pointer to an int that will contain current temperature value for specified adapter.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error  code.
ADL_EXTERNC int EXPOSED ADL_Overdrive6_TargetTemperatureData_Get(int iAdapterIndex, int *lpCurrentValue, int *lpDefaultValue);

///  \brief Function returns default target temperature and current temp. value of the specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpDefaultValue: a pointer to an int that will contain default temperature value for specified adapter.
/// \param[out]  lpCurrentValue: a pointer to an int that will contain current temperature value for specified adapter.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error  code.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process and to assure that ADL API are thread-safe. Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive6_TargetTemperatureData_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int *lpCurrentValue, int *lpDefaultValue);

///  \brief Function changes the target temperature current value of the specified adapter .
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  iCurrentValue: int value containing new temperature value to apply for specified adapter.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error  code.
ADL_EXTERNC int EXPOSED ADL_Overdrive6_TargetTemperatureData_Set(int iAdapterIndex, int iCurrentValue);

///  \brief Function changes the target temperature current value of the specified adapter .
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  iCurrentValue:  int value containing new temperature value to apply for specified adapter.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error  code.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process and to assure that ADL API are thread-safe. Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive6_TargetTemperatureData_Set(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iCurrentValue);

///  \brief Function returns the target Fan PWM range of the specified adapter .
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpFanPWMLimitInfo: a pointer to an ADLOD6ParameterRange structure containing acceptable Fan PWM range to work with.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error  code.
ADL_EXTERNC int EXPOSED ADL_Overdrive6_FanPWMLimitRangeInfo_Get(int iAdapterIndex, ADLOD6ParameterRange *lpFanPWMLimitInfo);

///  \brief Function returns the target Fan PWM range of the specified adapter .
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpFanPWMLimitInfo: a pointer to an ADLOD6ParameterRange structure containing acceptable Fan PWM range to work with.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error  code.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process and to assure that ADL API are thread-safe. Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive6_FanPWMLimitRangeInfo_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLOD6ParameterRange *lpFanPWMLimitInfo);

///  \brief Function returns default target Fan PWM and current Fan PWM value of the specified adapter.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpDefaultValue: a pointer to an int that will contain default Fan PWM value for specified adapter.
/// \param[out]  lpCurrentValue: a pointer to an int that will contain current Fan PWM value for specified adapter.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error  code.
ADL_EXTERNC int EXPOSED ADL_Overdrive6_FanPWMLimitData_Get(int iAdapterIndex, int *lpCurrentValue, int *lpDefaultValue);

///  \brief Function returns default target Fan PWM and current Fan PWM value of the specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  lpDefaultValue: a pointer to an int that will contain default Fan PWM value for specified adapter.
/// \param[out]  lpCurrentValue: a pointer to an int that will contain current Fan PWM value for specified adapter.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error  code.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process and to assure that ADL API are thread-safe. Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive6_FanPWMLimitData_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int *lpCurrentValue, int *lpDefaultValue);

///  \brief Function changes the target Fan PWM current value of the specified adapter .
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out]  iCurrentValue: int value containing new Fan PWM value to apply for specified adapter.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error  code.
ADL_EXTERNC int EXPOSED ADL_Overdrive6_FanPWMLimitData_Set(int iAdapterIndex, int iCurrentValue);

///  \brief Function changes the target Fan PWM current value of the specified adapter .
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] iCurrentValue: int value containing new Fan PWM value to apply for specified adapter.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error  code.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process and to assure that ADL API are thread-safe. Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive6_FanPWMLimitData_Set(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iCurrentValue);

///  \brief Function returns the current power of the specified adapter .
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  ePowerType The int to specify the power type.  0 Is for ASIC Total Power.
/// \param[out]  lpCurrentValue The int pointer containing the current power in Watts with 8 fractional bits.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error  code.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process and to assure that ADL API are thread-safe. Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive6_CurrentPower_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iPowerType, int *lpCurrentValue);

// @}

#endif /* OVERDRIVE6_H_ */
