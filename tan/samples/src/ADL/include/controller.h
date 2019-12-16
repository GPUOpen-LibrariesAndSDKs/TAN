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

/// \file controller.h
/// \brief Contains all controller-related functions exposed by ADL for \ALL platforms.
///
/// This file contains all controller-related functions exposed by ADL for \ALL platforms.
/// All functions in this file can be used as a reference to ensure
/// the specified function pointers can be used by the appropriate runtime
/// dynamic library loaders.

#ifndef CONTROLLER_H_
#define CONTROLLER_H_

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

///\addtogroup LINUX
// @{

///
/// \brief Function to set the current gamma value for a controller.
/// 
/// This function sets the gamma value of a specified controller.
/// \platform
/// \LNX
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iControllerIndex The desired controller index.
/// \param[in]  adlGamma The Gamma information structure.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code.   \ref define_adl_results
/// \deprecated This API has been deprecated because the controller index is no longer used with DAL2. Replaced by ADL_Display_LUTColor_Set
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Controller_Color_Set(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iControllerIndex, ADLGamma adlGamma);

///
/// \brief Function to set the current gamma value for a controller.
/// 
/// This function sets the gamma value of a specified controller.
/// \platform
/// \LNX
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iControllerIndex The desired controller index.
/// \param[in]  adlGamma The Gamma information structure.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code.   \ref define_adl_results
/// \deprecated This API has been deprecated because the controller index is no longer used with DAL2. Replaced by ADL_Display_LUTColor_Set
ADL_EXTERNC int EXPOSED ADL_Controller_Color_Set(int iAdapterIndex, int iControllerIndex, ADLGamma adlGamma);

///
/// \brief Function to get the current value of gamma for a controller.
/// 
/// This function retrieves the current gamma value of the specified controller.
/// \platform
/// \LNX
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iControllerIndex The desired controller index.
/// \param[out]  lpGammaCurrent The pointer to the retrieved current gamma information structure.
/// \param[out]  lpGammaDefault The pointer to the retrieved default gamma information structure.
/// \param[out]  lpGammaMin The pointer to the retrieved minimum gamma information structure.
/// \param[out]  lpGammaMax The pointer to the retrieved maximum gamma information structure.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code.   \ref define_adl_results
/// \deprecated This API has been deprecated because the controller index is no longer used with DAL2. Replaced by ADL_Display_LUTColor_Get
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Controller_Color_Get(ADL_CONTEXT_HANDLE context,int iAdapterIndex, int iControllerIndex, ADLGamma *lpGammaCurrent, ADLGamma *lpGammaDefault, ADLGamma *lpGammaMin, ADLGamma *lpGammaMax);

///
/// \brief Function to get the current value of gamma for a controller.
/// 
/// This function retrieves the current gamma value of the specified controller.
/// \platform
/// \LNX
/// \param[in]  iAdapterIndex The ADL index handle of the desired adapter.
/// \param[in]  iControllerIndex The desired controller index.
/// \param[out]  lpGammaCurrent The pointer to the retrieved current gamma information structure.
/// \param[out]  lpGammaDefault The pointer to the retrieved default gamma information structure.
/// \param[out]  lpGammaMin The pointer to the retrieved minimum gamma information structure.
/// \param[out]  lpGammaMax The pointer to the retrieved maximum gamma information structure.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code.   \ref define_adl_results
/// \deprecated This API has been deprecated because the controller index is no longer used with DAL2. Replaced by ADL_Display_LUTColor_Get
ADL_EXTERNC int EXPOSED ADL_Controller_Color_Get(int iAdapterIndex, int iControllerIndex, ADLGamma *lpGammaCurrent, ADLGamma *lpGammaDefault, ADLGamma *lpGammaMin, ADLGamma *lpGammaMax);

// @}

#endif /* CONTROLLER_H_ */
