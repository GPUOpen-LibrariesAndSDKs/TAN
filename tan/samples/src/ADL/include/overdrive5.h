///
///  Copyright (c) 2008 - 2009 Advanced Micro Devices, Inc.
 
///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

/// \file overdrive5.h
/// \brief Contains all Overdrive5-related functions exposed by ADL for \ALL platforms.
///
/// This file contains all Overdrive5-related functions exposed by ADL for \ALL platforms.
/// All functions found in this file can be used as a reference to ensure
/// the specified function pointers can be used by the appropriate runtime
/// dynamic library loaders.

#ifndef OVERDRIVE5_H_
#define OVERDRIVE5_H_

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

/// \addtogroup OVERDRIVE5
// @{

///
/// \brief Function to retrieve current power management-related activity.
/// 
/// This function retrieves current power management-related activity for a specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in,out] lpActivity The pointer to the ADLPMActivity structure storing the retrieved power management activity information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive5_CurrentActivity_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLPMActivity *lpActivity);

///
/// \brief Function to retrieve current power management-related activity.
/// 
/// This function retrieves current power management-related activity for a specified adapter.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in,out] lpActivity The pointer to the ADLPMActivity structure storing the retrieved power management activity information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Overdrive5_CurrentActivity_Get(int iAdapterIndex, ADLPMActivity *lpActivity);

///
/// \brief Function to retrieve thermal devices information.
/// 
/// This function retrieves thermal devices information for a specified adapter and thermal controller.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iThermalControllerIndex The ADL index handle of the desired thermal controller. <b>Set to 0.</b>
/// \param[in,out] lpThermalControllerInfo The pointer to the ADLThermalControllerInfo structure storing the retrieved thermal device information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive5_ThermalDevices_Enum(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iThermalControllerIndex, ADLThermalControllerInfo *lpThermalControllerInfo);

///
/// \brief Function to retrieve thermal devices information.
/// 
/// This function retrieves thermal devices information for a specified adapter and thermal controller.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iThermalControllerIndex The ADL index handle of the desired thermal controller. <b>Set to 0.</b>
/// \param[in,out] lpThermalControllerInfo The pointer to the ADLThermalControllerInfo structure storing the retrieved thermal device information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Overdrive5_ThermalDevices_Enum(int iAdapterIndex, int iThermalControllerIndex, ADLThermalControllerInfo *lpThermalControllerInfo);

///
/// \brief Function to retrieve thermal controller temperatures.
/// 
/// This function retrieves thermal controller temperature information for a specified adapter and thermal controller.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iThermalControllerIndex The ADL index handle of the desired thermal controller. <b>Set to 0.</b>
/// \param[in,out] lpTemperature The pointer to the ADLTemperature structure storing the retrieved temperature information in milli-degrees Celsius.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive5_Temperature_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iThermalControllerIndex, ADLTemperature *lpTemperature);

///
/// \brief Function to retrieve thermal controller temperatures.
/// 
/// This function retrieves thermal controller temperature information for a specified adapter and thermal controller.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iThermalControllerIndex The ADL index handle of the desired thermal controller. <b>Set to 0.</b>
/// \param[in,out] lpTemperature The pointer to the ADLTemperature structure storing the retrieved temperature information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Overdrive5_Temperature_Get(int iAdapterIndex, int iThermalControllerIndex, ADLTemperature *lpTemperature);

///
/// \brief Function to retrieve the fan speed reporting capability for thermal controllers.
/// 
/// This function retrieves the fan speed reporting capability for a specified adapter and thermal controller.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iThermalControllerIndex The ADL index handle of the desired thermal controller. <b>Set to 0.</b>
/// \param[in,out] lpFanSpeedInfo The pointer to the ADLFanSpeedInfo structure storing the retrieved fan speed information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive5_FanSpeedInfo_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iThermalControllerIndex, ADLFanSpeedInfo *lpFanSpeedInfo);

///
/// \brief Function to retrieve the fan speed reporting capability for thermal controllers.
/// 
/// This function retrieves the fan speed reporting capability for a specified adapter and thermal controller.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iThermalControllerIndex The ADL index handle of the desired thermal controller. <b>Set to 0.</b>
/// \param[in,out] lpFanSpeedInfo The pointer to the ADLFanSpeedInfo structure storing the retrieved fan speed information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Overdrive5_FanSpeedInfo_Get(int iAdapterIndex, int iThermalControllerIndex, ADLFanSpeedInfo *lpFanSpeedInfo);

///
/// \brief Function to retrieve the fan speed reported by the thermal controller.
/// 
/// This function retrieves the reported fan speed from a specified thermal controller.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iThermalControllerIndex The ADL index handle of the desired thermal controller. <b>Set to 0.</b>
/// \param[in,out] lpFanSpeedValue The pointer to the ADLFanSpeedValue structure storing the retrieved fan speed.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive5_FanSpeed_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iThermalControllerIndex, ADLFanSpeedValue *lpFanSpeedValue);

///
/// \brief Function to retrieve the fan speed reported by the thermal controller.
/// 
/// This function retrieves the reported fan speed from a specified thermal controller.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iThermalControllerIndex The ADL index handle of the desired thermal controller. <b>Set to 0.</b>
/// \param[in,out] lpFanSpeedValue The pointer to the ADLFanSpeedValue structure storing the retrieved fan speed.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Overdrive5_FanSpeed_Get(int iAdapterIndex, int iThermalControllerIndex, ADLFanSpeedValue *lpFanSpeedValue);

///
/// \brief Function to set the fan speed.
/// 
/// This function sets the fan speed for a specified adapter and thermal controller.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iThermalControllerIndex The ADL index handle of the desired thermal controller. <b>Set to 0.</b>
/// \param[in]  lpFanSpeedValue The pointer to the ADLFanSpeedValue structure storing the desired fan speed.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive5_FanSpeed_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iThermalControllerIndex, ADLFanSpeedValue *lpFanSpeedValue);

///
/// \brief Function to set the fan speed.
/// 
/// This function sets the fan speed for a specified adapter and thermal controller.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iThermalControllerIndex The ADL index handle of the desired thermal controller. <b>Set to 0.</b>
/// \param[in]  lpFanSpeedValue The pointer to the ADLFanSpeedValue structure storing the desired fan speed.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Overdrive5_FanSpeed_Set(int iAdapterIndex, int iThermalControllerIndex, ADLFanSpeedValue *lpFanSpeedValue);

///
/// \brief Function to set the fan speed to the default fan speed value.
/// 
/// This function sets the current fan speed for a specified adapter and thermal controller to the default fan speed.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iThermalControllerIndex The ADL index handle of the desired thermal controller. <b>Set to 0.</b>
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive5_FanSpeedToDefault_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iThermalControllerIndex);

///
/// \brief Function to set the fan speed to the default fan speed value.
/// 
/// This function sets the current fan speed for a specified adapter and thermal controller to the default fan speed.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iThermalControllerIndex The ADL index handle of the desired thermal controller. <b>Set to 0.</b>
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Overdrive5_FanSpeedToDefault_Set(int iAdapterIndex, int iThermalControllerIndex);

///
/// \brief Function to retrieve the current Overdrive parameters.
/// 
/// This function retrieves the current Overdrive parameters for a specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in,out] lpOdParameters The pointer to the ADLODParameters structure storing the retrieved Overdrive parameters.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive5_ODParameters_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLODParameters *lpOdParameters);

///
/// \brief Function to retrieve the current Overdrive parameters.
/// 
/// This function retrieves the current Overdrive parameters for a specified adapter.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in,out] lpOdParameters The pointer to the ADLODParameters structure storing the retrieved Overdrive parameters.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Overdrive5_ODParameters_Get(int iAdapterIndex, ADLODParameters *lpOdParameters);

///
/// \brief Function to retrieve the current or default Overdrive performance levels.
/// 
/// This function retrieves the current or default Overdrive performance levels for a specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDefault The value indicating whether or not to retrieve the default performance levels.
/// \param[in,out] lpOdPerformanceLevels The pointer to the ADLODPerformanceLevels structure storing the retrieved information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive5_ODPerformanceLevels_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iDefault, ADLODPerformanceLevels *lpOdPerformanceLevels);

///
/// \brief Function to retrieve the current or default Overdrive performance levels.
/// 
/// This function retrieves the current or default Overdrive performance levels for a specified adapter.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iDefault The value indicating whether or not to retrieve the default performance levels.
/// \param[in,out] lpOdPerformanceLevels The pointer to the ADLODPerformanceLevels structure storing the retrieved information.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Overdrive5_ODPerformanceLevels_Get(int iAdapterIndex, int iDefault, ADLODPerformanceLevels *lpOdPerformanceLevels);

///
/// \brief Function to set the current Overdrive performance levels.
/// 
/// This function sets the current Overdrive performance levels for a specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  lpOdPerformanceLevels The pointer to the ADLODPerformanceLevels structure storing the desired performance level.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive5_ODPerformanceLevels_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLODPerformanceLevels *lpOdPerformanceLevels);

///
/// \brief Function to set the current Overdrive performance levels.
/// 
/// This function sets the current Overdrive performance levels for a specified adapter.
/// \platform
/// \ALL
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  lpOdPerformanceLevels The pointer to the ADLODPerformanceLevels structure storing the desired performance level.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results 
ADL_EXTERNC int EXPOSED ADL_Overdrive5_ODPerformanceLevels_Set(int iAdapterIndex, ADLODPerformanceLevels *lpOdPerformanceLevels);


///  \brief Function to check for  PowerControl capabilities
///
/// The output parameter of the function lets the caller know about whether or not Power control
/// feature is supported.
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpSupported The pointer to the  caps of adapter settings
/// \return If the function succeeds, the return value is \ref ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive5_PowerControl_Caps(ADL_CONTEXT_HANDLE context,int iAdapterIndex,  int *lpSupported);

///  \brief Function to check for  PowerControl capabilities
///
/// The output parameter of the function lets the caller know about whether or not Power control
/// feature is supported.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[out] lpSupported The pointer to the  caps of adapter settings
/// \return If the function succeeds, the return value is \ref ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results 

ADL_EXTERNC int EXPOSED ADL_Overdrive5_PowerControl_Caps(int iAdapterIndex,  int *lpSupported);

///  \brief Function to get values  of  PowerControl information
/// 
/// The output parameter of the function lets the caller know about Power control adjust limit , step values and default value.
/// parameters.
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle  of the desired adapter 
/// \param[out]  lpPowerControlInfo The pointer to the ADLPowerControlInfo containing value of Power ///control 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive5_PowerControlInfo_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, ADLPowerControlInfo *lpPowerControlInfo);

///  \brief Function to get values  of  PowerControl information
/// 
/// The output parameter of the function lets the caller know about Power control adjust limit , step values and default value.
/// parameters.
/// \param[in]  iAdapterIndex The ADL index handle  of the desired adapter 
/// \param[out]  lpPowerControlInfo The pointer to the ADLPowerControlInfo containing value of Power ///control 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. 

ADL_EXTERNC int EXPOSED ADL_Overdrive5_PowerControlInfo_Get(int iAdapterIndex, ADLPowerControlInfo *lpPowerControlInfo);

///  \brief Function to get values  of  PowerControl
///
/// The parameter of the function lets the caller to get new value of the Power control
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle  of the desired adapter 
/// \param[out]  lpCurrentValue pointer to the current value of Power control 
/// \param[out]  lpDefaultValue pointer to the Default value of Power control 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error  code. 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive5_PowerControl_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int *lpCurrentValue, int *lpDefaultValue);

///  \brief Function to get values  of  PowerControl
///
/// The parameter of the function lets the caller to get new value of the Power control
/// \param[in]  iAdapterIndex The ADL index handle  of the desired adapter 
/// \param[out]  lpCurrentValue pointer to the current value of Power control 
/// \param[out]  lpDefaultValue pointer to the Default value of Power control 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error  code. 

ADL_EXTERNC int EXPOSED ADL_Overdrive5_PowerControl_Get(int iAdapterIndex, int *lpCurrentValue, int *lpDefaultValue);

///  \brief Function to set values  of  PowerControl
///
/// The parameter of the function lets the caller to set new value to the Power control
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle  of the desired adapter 
/// \param[in]  iValue New  value of Power control 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error  code. 
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive5_PowerControl_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iValue);

///  \brief Function to set values  of  PowerControl
///
/// The parameter of the function lets the caller to set new value to the Power control
/// \param[in]  iAdapterIndex The ADL index handle  of the desired adapter 
/// \param[in]  iValue New  value of Power control 
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error  code. 

ADL_EXTERNC int EXPOSED ADL_Overdrive5_PowerControl_Set(int iAdapterIndex, int iValue);

///
/// \brief Function to retrieve current power management capabilities
/// 
/// This function retrieves current power management capabilities for a specified adapter.
/// \platform
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iSupported The pointer to an int variable which indicates if Overdrive feature is supported ( \ref ADL_TRUE : supported, \ref ADL_FALSE : not supported)
/// \param[in]  iEnabled The pointer to an int variable which indicates if Overdrive feature is enabled ( \ref ADL_TRUE : enabled, \ref ADL_FALSE : disabled)
/// \param[in]  iEnabled The pointer to an int variable receiving the Overdrive feature version.
/// \return If the function succeeds, the return value is \ref ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results. If Overdrive feature is not supported the return code is \ref ADL_ERR_NOT_SUPPORTED 
///
/// \remarks Call this API prior to calling any other Overdrive APIs to determine if this feature is supported and enabled.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Overdrive_Caps (ADL_CONTEXT_HANDLE context,int iAdapterIndex, int * iSupported, int * iEnabled, int * iVersion );

///
/// \brief Function to retrieve current power management capabilities
/// 
/// This function retrieves current power management capabilities for a specified adapter.
/// \platform
/// \ALL
/// \param[in]  iSupported The pointer to an int variable which indicates if Overdrive feature is supported ( \ref ADL_TRUE : supported, \ref ADL_FALSE : not supported)
/// \param[in]  iEnabled The pointer to an int variable which indicates if Overdrive feature is enabled ( \ref ADL_TRUE : enabled, \ref ADL_FALSE : disabled)
/// \param[in]  iEnabled The pointer to an int variable receiving the Overdrive feature version.
/// \return If the function succeeds, the return value is \ref ADL_OK (zero). Otherwise the return value is an ADL error code (non-zero). \ref define_adl_results. If Overdrive feature is not supported the return code is \ref ADL_ERR_NOT_SUPPORTED 
///
/// \remarks Call this API prior to calling any other Overdrive APIs to determine if this feature is supported and enabled.
ADL_EXTERNC int EXPOSED ADL_Overdrive_Caps (int iAdapterIndex, int * iSupported, int * iEnabled, int * iVersion );

// @}

#endif /* OVERDRIVE5_H_ */
