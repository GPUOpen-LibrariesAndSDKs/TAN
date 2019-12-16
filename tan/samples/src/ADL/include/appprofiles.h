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

/// \file appprofiles.h
/// \brief Contains all appprofiles-related functions exposed by ADL for \ALL platforms.
///
/// This file contains all appprofiles-related functions exposed by ADL for \ALL platforms.
/// All functions in this file can be used as a reference to ensure
/// the specified function pointers can be used by the appropriate runtime
/// dynamic library loaders.

#ifndef APPPROFILES_H_
#define APPPROFILES_H_

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

/// \addtogroup APPPROFILES
// @{

///
/// \brief Function to Reload System appprofiles.
/// 
/// This function Reloads System appprofiles by reloading system profile and loading user profile.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code.   \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_ApplicationProfiles_System_Reload(ADL_CONTEXT_HANDLE context);

///
/// \brief Function to Reload System appprofiles.
/// 
/// This function Reloads System appprofiles by reloading system profile and loading user profile.
/// \platform
/// \WIN
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code.   \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_ApplicationProfiles_System_Reload();

///
/// \brief Function to Load User appprofiles.
/// 
/// This function Loads User appprofiles by loadding system profile and user profile.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code.   \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_ApplicationProfiles_User_Load(ADL_CONTEXT_HANDLE context);

///
/// \brief Function to Load User appprofiles.
/// 
/// This function Loads User appprofiles by loadding system profile and user profile.
/// \platform
/// \WIN
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code.   \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_ApplicationProfiles_User_Load();

///
/// \brief Function to Unload User appprofiles.
/// 
/// This function Unloads User appprofiles by unloading user profile.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code.   \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_ApplicationProfiles_User_Unload(ADL_CONTEXT_HANDLE context);

///
/// \brief Function to Unload User appprofiles.
/// 
/// This function Unloads User appprofiles by unloading user profile.
/// \platform
/// \WIN
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code.   \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_ApplicationProfiles_User_Unload();



/* TODO++ NOT FINALIZED IN ADL

///
/// \brief Function to Get appprofiles status info.
/// 
/// This function Gets appprofiles status info such as system/user profile loaded, loaded profile valid or not.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code.   \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_ApplicationProfiles_StatusInfo_Get(ADL_CONTEXT_HANDLE context);

///
/// \brief Function to Get appprofiles status info.
/// 
/// This function Gets appprofiles status info such as system/user profile loaded, loaded profile valid or not.
/// \platform
/// \WIN
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code.   \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_ApplicationProfiles_StatusInfo_Get();


ADL_EXTERNC int EXPOSED ADL_ApplicationProfiles_Get();
ADL_EXTERNC int EXPOSED ADL_ApplicationProfiles_Select();

*/

///
/// \brief Function to retrieve the profile of an application defined in driver.
/// 
/// This function retrieves the profile information of an application. 
/// This API uses the Caller's Memory Allocation Callback Function to allocate memory pointed by lppProfile. 
/// Caller is responsible to de-allocate the memory.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]	FileName	Specifies the application file name
/// \param[in]	Path		Specifies the application file path
/// \param[in]	Version		Specifies the application version
/// \param[in]	AppProfileArea		Specifies the area the profile works on (like 'PX', 'D3D', etc. )
/// \param[out] lppProfile	The pointer to the pointer to the retrieved application profile. Set it to NULL prior to call this API.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API will be replaced with a new API that supports unicode. This API uses the memory allocation callback function provided by the user.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_ApplicationProfiles_ProfileOfAnApplication_Search(ADL_CONTEXT_HANDLE context,const char* FileName, const char* Path, 
								const char* Version, const char* AppProfileArea, ADLApplicationProfile** lppProfile);

///
/// \brief Function to retrieve the profile of an application defined in driver.
/// 
/// This function retrieves the profile information of an application. 
/// This API uses the Caller's Memory Allocation Callback Function to allocate memory pointed by lppProfile. 
/// Caller is responsible to de-allocate the memory.
/// \platform
/// \WIN
/// \param[in]	FileName	Specifies the application file name
/// \param[in]	Path		Specifies the application file path
/// \param[in]	Version		Specifies the application version
/// \param[in]	AppProfileArea		Specifies the area the profile works on (like 'PX', 'D3D', etc. )
/// \param[out] lppProfile	The pointer to the pointer to the retrieved application profile. Set it to NULL prior to call this API.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API will be replaced with a new API that supports unicode. This API uses the memory allocation callback function provided by the user.
ADL_EXTERNC int EXPOSED ADL_ApplicationProfiles_ProfileOfAnApplication_Search(const char* FileName, const char* Path, 
								const char* Version, const char* AppProfileArea, ADLApplicationProfile** lppProfile);


///
/// \brief Function to retrieve the recent application list from registry.
/// 
/// This function retrieves the recent application list from registry. 
/// This API uses the Caller's Memory Allocation Callback Function to allocate memory pointed by lppAppList. 
/// Caller is responsible to de-allocate the memory.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]	iListType	Specifies the type of the list to retrieve, defined in \ref ApplicationListType 
/// \param[out] lpNumApps	The pointer to the number of applications in the list. 
/// \param[out] lppAppList	The pointer to the pointer to the retrieved application list from registry. Set it to NULL prior to call this API.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_ApplicationProfiles_HitLists_Get(ADL_CONTEXT_HANDLE context, int iListType, int * lpNumApps, ADLApplicationData ** lppAppList);

///
/// \brief Function to retrieve the recent application list from registry.
/// 
/// This function retrieves the recent application list from registry. 
/// This API uses the Caller's Memory Allocation Callback Function to allocate memory pointed by lppAppList. 
/// Caller is responsible to de-allocate the memory.
/// \platform
/// \WIN
/// \param[in]	iListType	Specifies the type of the list to retrieve, defined in \ref ApplicationListType 
/// \param[out] lpNumApps	The pointer to the number of applications in the list. 
/// \param[out] lppAppList	The pointer to the pointer to the retrieved application list from registry. Set it to NULL prior to call this API.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
ADL_EXTERNC int EXPOSED ADL_ApplicationProfiles_HitLists_Get( int iListType, int * lpNumApps, ADLApplicationData ** lppAppList);


///
/// \brief Function to retrieve the recent application list from registry.
/// 
/// This function retrieves the recent application list from registry. 
/// This API uses the Caller's Memory Allocation Callback Function to allocate memory pointed by lppAppList. 
/// Caller is responsible to de-allocate the memory.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]	iListType	Specifies the type of the list to retrieve, defined in \ref ApplicationListType 
/// \param[out] lpNumApps	The pointer to the number of applications in the list. 
/// \param[out] lppAppList	The pointer to the pointer to the retrieved application list from registry including process id. Set it to NULL prior to call this API.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_ApplicationProfiles_HitListsX3_Get(ADL_CONTEXT_HANDLE context, int iListType, int * lpNumApps, ADLApplicationDataX3 ** lppAppList);

///
/// \brief Function to retrieve the recent application list from registry.
/// 
/// This function retrieves the recent application list from registry. 
/// This API uses the Caller's Memory Allocation Callback Function to allocate memory pointed by lppAppList. 
/// Caller is responsible to de-allocate the memory.
/// \platform
/// \WIN
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]	iListType	Specifies the type of the list to retrieve, defined in \ref ApplicationListType 
/// \param[out] lpNumApps	The pointer to the number of applications in the list. 
/// \param[out] lppAppList	The pointer to the pointer to the retrieved application list from registry. Set it to NULL prior to call this API.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_ApplicationProfiles_HitListsX2_Get(ADL_CONTEXT_HANDLE context, int iListType, int * lpNumApps, ADLApplicationDataX2 ** lppAppList);

///
/// \brief Function to retrieve the recent application list from registry.
/// 
/// This function retrieves the recent application list from registry. 
/// This API uses the Caller's Memory Allocation Callback Function to allocate memory pointed by lppAppList. 
/// Caller is responsible to de-allocate the memory.
/// \platform
/// \WIN
/// \param[in]	iListType	Specifies the type of the list to retrieve, defined in \ref ApplicationListType 
/// \param[out] lpNumApps	The pointer to the number of applications in the list. 
/// \param[out] lppAppList	The pointer to the pointer to the retrieved application list from registry. Set it to NULL prior to call this API.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
///
/// \remarks This API uses the memory allocation callback function provided by the user.
ADL_EXTERNC int EXPOSED ADL_ApplicationProfiles_HitListsX2_Get( int iListType, int * lpNumApps, ADLApplicationDataX2 ** lppAppList);

// @}

#endif /* appprofiles_H_ */
