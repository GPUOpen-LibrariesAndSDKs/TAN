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

/// \file adl.h
/// \brief Contains the main functions exposed by ADL for \ALL platforms.
///
/// This file contains the main functions exposed by ADL for \ALL platforms.
/// All functions in this file can be used as a reference when creating
/// specific function pointers to be used by the appropriate runtime
/// dynamic library loaders.
/// \mainpage
// @{
/// This document describes the APIs (application programming interfaces) that can be used to access
/// AMD-specific display driver functionality, such as those related to specific display devices. \n\n
/// Included in this document are definitions as well as the data structures used by each API. \n
/// Public functions are prefixed with "ADL_". 
/// The majority of the APIs are supported on 32-bit and 64-bit versions of Windows (XP and Vista) and Linux platforms. \n\n
/// Working code samples and snippets of the API source code files are also provided for reference purposes.\n\n
///\note Although a number of header files are referenced in this document, only three are required and provoided with this SDK:\n
///\li \b adl_sdk.h : The ony header file to be included in a C/C++ project \n
///\li \b adl_structures.h : Definitions of all structures for public use. Included in \b adl_sdk.h \n
///\li \b adl_defines.h : Definitions of all Constants and Enumerations.  Included in \b adl_structures.h \n
///
/// The three files should be placed in the same folder.
// @}

#ifndef ADL_H_
#define ADL_H_

// Define the API groups here so they can appear in this particular order.
// Use Doxygen command "ingroup" to add APIs to these groups from other files.

/// \example Adl2/Adl2_Sample.cpp
/// A C/C++ Sample Application for \WIN \n
/// Visual Studio 2012 solution file <b> "Adl2.sln" </b> is included in ADL SDK

/// \example ColorCaps/main.cpp
/// A C/C++ Sample Application for \ALL \n
/// Visual Studio 2015 solution file <b> "ADL Sample.sln" </b> is included in ADL SDK

/// \example DDCBlockAccess/DDCBlockAccessDlg.cpp
/// A C/C++ Sample Application for \WIN \n
/// Visual Studio 2005 solution file <b> "DDCBlockAccess.sln" </b> is included in ADL SDK

/// \example EDIDSampleTool/main.cpp
/// A C/C++ Sample Application for \WIN \n
/// Visual Studio 2012 solution file <b> "EDIDSampleTool.sln" </b> is included in ADL SDK

/// \example Eyefinity/main.cpp
/// A C/C++ Sample Application for \WIN \n
/// Visual Studio 2015 solution file <b> "Eyefinity.sln" </b> is included in ADL SDK

/// \example Eyefinity2/main.cpp
/// A C/C++ Sample Application for \WIN \n
/// Visual Studio 2010 solution file <b> "Eyefinity.sln" </b> is included in ADL SDK

/// \example Overdrive/Overdrive_Sample.cpp
/// A C/C++ Sample Application for \WIN \n
/// Visual Studio 2015 solution file <b> "Overdrive_Sample.sln" </b> is included in ADL SDK

/// \example OverdriveN/OverdriveN.cpp
/// A C/C++ Sample Application for \WIN \n
/// Visual Studio 2012 solution file <b> "OverdriveN.sln" </b> is included in ADL SDK

/// \example PowerXPress/PowerXPress.cpp
/// A C/C++ Sample Application for \WIN \n
/// Visual Studio 2015 solution file <b> "PowerXPress.sln" </b> is included in ADL SDK

/// \example Sample-Managed/program.cs
/// A C# Sample Application for \WIN \n
/// Visual Studio 2005 solution file <b> "Sample-Managed.sln" </b> is included in ADL SDK

/// \example Sample-Managed/ADL.cs
/// A C# class encapsulating some of the ADL APIs. Used by the C# Sample Application

// Adding the Supported Hardware and OS page (Add another slash to the lines below to enable...)
/// \page LOGOS Compatibility
/// \htmlinclude Public-Documents/logo.htm

// Adding the tutorial as a separate page...
/// \page WHATISADL What is ADL and how to use it?
/// \htmlinclude Public-Documents/WhatIsADL.html

// Enable the top "API Groups" this by adding one more slash line at the beginning of line below...
//\defgroup ADL_API API Groups



///\ingroup ADL_API
///\defgroup MAINAPI Main Group of APIs
/// This group describes the APIs for initializing, refreshing and destroying ADL

///\ingroup ADL_API
///\defgroup ADAPTERAPI Adapter Related APIs
/// This group outlines all adapter related APIs.

///\ingroup ADAPTERAPI
///\defgroup ADAPTER General Adapter APIs
/// This group describes general Adapter  APIs.

///\ingroup ADAPTERAPI
///\defgroup CROSSDISPLAY CrossDisplay APIs
/// This group describes the CrossDisplay APIs.

///\ingroup ADAPTERAPI
///\defgroup CROSSFIRE CrossFire APIs
/// This group describes the CrossFire APIs.

///\ingroup ADL_API
///\defgroup DISPLAYAPI Display Related APIs
/// This group outlines all display related APIs.

///\ingroup DISPLAYAPI
///\defgroup DISPLAY Miscellaneous Display APIs
/// This group outlines the Miscellaneous Display related APIs.

///\ingroup DISPLAYAPI
///\defgroup DISPLAYMNGR Eyefinity APIs
/// This group outlines the Displays Manager related APIs.

/// \ingroup DISPLAYAPI
/// \defgroup COLOR Color related APIs
/// This group describes the Color related APIs

///\ingroup DISPLAYAPI
/// \defgroup T_OVERRIDE Timing Override APIs
/// This group describes the Timing Overide related APIs

/// \ingroup DISPLAYAPI
/// \defgroup CUSTOMIZED Customized Modes APIs
/// This group describes the APIs related to Customized modes

/// \ingroup DISPLAYAPI
/// \defgroup OVERSCAN Overscan/Underscan APIs
/// This group outlines Overscan and Underscan related APIs.

/// \ingroup DISPLAYAPI
///\defgroup OVERLAY Overlay APIs
/// This group outlines all Overlay related APIs.

/// \ingroup DISPLAYAPI
///\defgroup POWERX PowerXpress APIs
/// This group outlines PowerXpress Related APIs.

/// \ingroup DISPLAYAPI
///\defgroup VIEWPORT ViewPort APIs
/// This group outlines ViewPort Related APIs.

/// \ingroup ADL_API
///\defgroup SPECIALDISP DFP, TV, Component Video APIs
/// This group outlines DFP, TV and Component Video related APIs.

/// \ingroup SPECIALDISP
///\defgroup DFP Digital Flat Panel APIs
/// This group outlines all digital flat panel related APIs.

/// \ingroup SPECIALDISP
///\defgroup TV TV APIs
/// This group outlines all television related APIs.

/// \ingroup SPECIALDISP
///\defgroup CV Component Video APIs
/// This group outlines all component video related APIs.

/// \ingroup ADL_API
///\defgroup OVERDRIVE OverDrive APIs
/// This group outlines all OverDrive related APIs

/// \ingroup OVERDRIVE
///\defgroup OVERDRIVE5GROUP OverDrive5 APIs
/// This group outlines all OverDrive5 related APIs

/// \ingroup OVERDRIVE5GROUP
/// \page OD5_APIS OverDrive5 APIs Notes
/// \htmlinclude Public-Documents/OD5_APIs.html

/// \ingroup OVERDRIVE5GROUP
///\defgroup OVERDRIVE5 OverDrive5 APIs
/// This group outlines all OverDrive5 related APIs

/// \ingroup OVERDRIVE
///\defgroup OVERDRIVE6 OverDrive6 APIs
/// This group outlines all OverDrive6 related APIs

/// \ingroup OVERDRIVE
///\defgroup OVERDRIVEN OverDrive Next APIs
/// This group outlines all OverDrive Next related APIs

/// \ingroup ADL_API
///\defgroup I2CDDCEDID I2C, DDC and EDID APIs
/// This group outlines I2C, DDC and EDID related APIs.

/// \ingroup ADL_API
///\defgroup WORKSTATION Workstation APIs
/// This group outlines all workstation related APIs.

/// \ingroup ADL_API
///\defgroup APPPROFILES Application Profiles APIs
/// This group outlines all Application Profiles APIs.

/// \ingroup ADL_API
///\defgroup LINUX Linux Specific APIs
/// This group outlines Linux specific APIs.

///\ingroup WORKSTATION
///\defgroup EDIDAPI EDID Emulation Related APIs
/// This group outlines the EDID Emulation related APIs.

#include "adl_sdk.h"
#include "amd_only/amd_structures.h"

#include "adapter.h"
#include "display.h"
#include "workstation.h"
#include "displaysmanager.h"

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


///\addtogroup MAINAPI
// @{

///
/// \brief Function to initialize the ADL interface. This function should be called first.
/// 
/// This function initializes the ADL library. Allows the client to specify desired threading behavior of ADL APIs. It also initializes global pointers and, at the same time, calls the ADL_Main_Control_Refresh function.
/// \platform 
/// \ALL
/// \param[in] callback: The memory allocation function for memory buffer allocation. This must be provided by the user. This must be provided by the user. ADL internally uses this callback to allocate memory for the output parameters returned to the user and user is responsible to free the memory once used for these parameters. ADL internal takes care of allocating and de allocating the memory for its local variables.
/// \param[in] iEnumConnectedAdapters: Specify a value of 0 to retrieve adapter information for all adapters that have ever been present in the system. Specify a value of 1 to retrieve adapter information only for adapters that are physically present and enabled in the system.
/// \param[in] threadingModel: Specify ADL threading behavior.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Main_ControlX2_Create ( ADL_MAIN_MALLOC_CALLBACK callback, int iEnumConnectedAdapters, ADLThreadingModel threadingModel );

///
/// \brief Function to initialize the ADL2 interface and issue client's context handle. 
/// 
/// Clients can use ADL2 versions of ADL APIs to assure that there is no interference with other ADL clients that are running in the same process. 
/// Such clients have to call ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
/// ADL2_Main_ControlX2_Create allows the client to specify desired threading behavior of ADL APIs.
/// \platform 
/// \ALL
/// \param[in] callback: The memory allocation function for memory buffer allocation. This must be provided by the user. This must be provided by the user. ADL internally uses this callback to allocate memory for the output parameters returned to the user and user is responsible to free the memory once used for these parameters. ADL internal takes care of allocating and de allocating the memory for its local variables.
/// \param[in] iEnumConnectedAdapters: Specify a value of 0 to retrieve adapter information for all adapters that have ever been present in the system. Specify a value of 1 to retrieve adapter information only for adapters that are physically present and enabled in the system.
/// \param[out] context: \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
/// \param[in] threadingModel: Specify ADL threading behavior.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL2_Main_ControlX2_Create ( ADL_MAIN_MALLOC_CALLBACK callback, int iEnumConnectedAdapters, ADL_CONTEXT_HANDLE* context, ADLThreadingModel threadingModel);


///
/// \brief Function to initialize the ADL interface. This function should be called first.
/// 
/// This function initializes the ADL library. ADL initialized using ADL_Main_Control_Create will not enforce serialization of ADL API executions by multiple threads.  Multiple threads will be allowed to enter to ADL at the same time. 
/// Note that ADL library is not guaranteed to be thread-safe. Client that calls ADL_Main_Control_Create have to provide its own mechanism for ADL calls serialization.
/// It also initializes global pointers and, at the same time, calls the ADL_Main_Control_Refresh function.
/// \platform 
/// \ALL
/// \param[in] callback: The memory allocation function for memory buffer allocation. This must be provided by the user. ADL internally uses this callback to allocate memory for the output parameters returned to the user and user is responsible to free the memory once used for these parameters. ADL internal takes care of allocating and de allocating the memory for its local variables.
/// \param[in] iEnumConnectedAdapters: Specify a value of 0 to retrieve adapter information for all adapters that have ever been present in the system. Specify a value of 1 to retrieve adapter information only for adapters that are physically present and enabled in the system.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Main_Control_Create ( ADL_MAIN_MALLOC_CALLBACK callback, int iEnumConnectedAdapters );

///
/// \brief Function to initialize the ADL2 interface and to obtain client's context handle. 
/// 
/// Clients can use ADL2 versions of ADL APIs to assure that there is no interference with other ADL clients that are running in the same process.  Such clients have to call ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
/// ADL initialized using ADL2_Main_Control_Create will not enforce serialization of ADL API executions by multiple threads.  Multiple threads will be allowed to enter to ADL at the same time. 
/// Note that ADL library is not guaranteed to be thread-safe. Client that calls ADL2_Main_Control_Create have to provide its own mechanism for ADL calls serialization.
/// \platform 
/// \ALL
/// \param[in] callback: The memory allocation function for memory buffer allocation. This must be provided by the user. ADL internally uses this callback to allocate memory for the output parameters returned to the user and user is responsible to free the memory once used for these parameters. ADL internal takes care of allocating and de allocating the memory for its local variables.
/// \param[in] iEnumConnectedAdapters: Specify a value of 0 to retrieve adapter information for all adapters that have ever been present in the system. Specify a value of 1 to retrieve adapter information only for adapters that are physically present and enabled in the system.
/// \param[out] context: \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL2_Main_Control_Create ( ADL_MAIN_MALLOC_CALLBACK callback, int iEnumConnectedAdapters, ADL_CONTEXT_HANDLE* context);

///
/// \brief Function to refresh adapter information. This function generates an adapter index value for all logical adapters that have ever been present in the system.
/// 
/// This function updates the adapter information based on the logical adapters currently in the system. The adapter index and UDID mappings remain unchanged for each refresh call. 
/// \platform 
/// \ALL
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Main_Control_Refresh (ADL_CONTEXT_HANDLE context);

///
/// \brief Function to refresh adapter information. This function generates an adapter index value for all logical adapters that have ever been present in the system.
/// 
/// This function updates the adapter information based on the logical adapters currently in the system. The adapter index and UDID mappings remain unchanged for each refresh call. 
/// \platform 
/// \ALL
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Main_Control_Refresh ();

///
/// \brief Function to destroy ADL global pointers. This function should be called last.
/// 
/// All ADL global buffers and resources are released after this function is called.
/// \platform 
/// \ALL
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL_Main_Control_Destroy ();

///
/// \brief Destroy client's ADL context.
/// 
/// Clients can use ADL2 versions of ADL APIs to assure that there is no interference with other ADL clients that are running in the same process and to assure that ADL APIs are thread-safe. 
/// Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using ADL2_Main_Control_Destroy.
/// \platform 
/// \ALL
/// \param[in] context: \ref ADL_CONTEXT_HANDLE instance to destroy.
/// \return If the function succeeds, the return value is \ref ADL_OK. Otherwise the return value is an ADL error code. \ref define_adl_results
ADL_EXTERNC int EXPOSED ADL2_Main_Control_Destroy (ADL_CONTEXT_HANDLE context);

///
/// \brief ADL local interface. Function to determine the validity of and retrieve the function pointer (similar to the GetProcAdress API) for a specified function.
/// 
/// This function returns the function pointer of a specified function if it is valid (defined and exposed in a DLL module). Call this function to ensure that a function is valid before calling it.
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[in]  lpModule The pointer to the desired handle of the DLL.
/// \param[in]  lpProcName The pointer to the desired function name.
/// \platform 
/// \ALL
/// \return If the specified function is valid, the return value is a function pointer. Otherwise it is NULL.
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC EXPOSED void* ADL2_Main_Control_GetProcAddress (ADL_CONTEXT_HANDLE context, void* lpModule, char* lpProcName );

///
/// \brief ADL local interface. Function to determine the validity of and retrieve the function pointer (similar to the GetProcAdress API) for a specified function.
/// 
/// This function returns the function pointer of a specified function if it is valid (defined and exposed in a DLL module). Call this function to ensure that a function is valid before calling it.
/// \param[in]  lpModule The pointer to the desired handle of the DLL.
/// \param[in]  lpProcName The pointer to the desired function name.
/// \platform 
/// \ALL
/// \return If the specified function is valid, the return value is a function pointer. Otherwise it is NULL.
ADL_EXTERNC EXPOSED void* ADL_Main_Control_GetProcAddress ( void* lpModule, char* lpProcName );


///
/// \brief Function to retrieve version information.
/// 
/// This function retrieves software version information and a web link to an XML file with information about the latest web-posted Catalyst drivers
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[out]  lpVersionsInfo The pointer to ADLVersionsInfo structure with the retrieved version information.
/// \platform 
/// \ALL
/// \return The function returns \ref ADL_OK or \ref ADL_OK_WARNING (if any of the version strings could not be retrieved from the system, for example "Catalyst Version"). If ADL is not initialized properly the function will return \ref ADL_ERR_DISABLED_ADAPTER
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Graphics_Versions_Get(ADL_CONTEXT_HANDLE context, ADLVersionsInfo * lpVersionsInfo );

///
/// \brief Function to retrieve version information.
/// 
/// This function retrieves software version information and a web link to an XML file with information about the latest web-posted Catalyst drivers
/// \param[out]  lpVersionsInfo The pointer to ADLVersionsInfo structure with the retrieved version information.
/// \platform 
/// \ALL
/// \return The function returns \ref ADL_OK or \ref ADL_OK_WARNING (if any of the version strings could not be retrieved from the system, for example "Catalyst Version"). If ADL is not initialized properly the function will return \ref ADL_ERR_DISABLED_ADAPTER
ADL_EXTERNC int EXPOSED ADL_Graphics_Versions_Get( ADLVersionsInfo * lpVersionsInfo );

///
/// \brief Function to retrieve s version information.
/// 
/// This function retrieves software version information and a web link to an XML file with information about the latest web-posted Catalyst drivers
/// \param[in] context: Client's ADL context handle \ref ADL_CONTEXT_HANDLE obtained from \ref ADL2_Main_Control_Create.
/// \param[out]  lpVersionsInfo The pointer to ADLVersionsInfoX2 structure with the retrieved version information.
/// \platform 
/// \WIN
/// \return The function returns \ref ADL_OK or \ref ADL_OK_WARNING (if any of the version strings could not be retrieved from the system, for example "Catalyst Version"). If ADL is not initialized properly the function will return \ref ADL_ERR_DISABLED_ADAPTER
/// \remarks Clients can use ADL2 version of the API to assure that there is no interference with other ADL clients that are running in the same process . Such clients have to call \ref ADL2_Main_Control_Create first to obtain \ref ADL_CONTEXT_HANDLE instance that has to be passed to each subsequent ADL2 call and finally destroyed using \ref ADL2_Main_Control_Destroy.
ADL_EXTERNC int EXPOSED ADL2_Graphics_VersionsX2_Get(ADL_CONTEXT_HANDLE context, ADLVersionsInfoX2 * lpVersionsInfo );
// @}

#endif /* ADL_H_ */
