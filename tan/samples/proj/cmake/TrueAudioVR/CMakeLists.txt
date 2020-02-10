cmake_minimum_required(VERSION 3.10)

# The cmake-policies(7) manual explains that the OLD behaviors of all
# policies are deprecated and that a policy should be set to OLD only under
# specific short-term circumstances.  Projects should be ported to the NEW
# behavior and not rely on setting a policy to OLD.

# VERSION not allowed unless CMP0048 is set to NEW
if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_SKIP_RULE_DEPENDENCY TRUE)

enable_language(CXX)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")
if(NOT WIN32)
if(APPLE)
#SET (CMAKE_CXX_COMPILER "/usr/bin/c++")
set(CMAKE_CXX_FLAGS_DEBUG "-Xpreprocessor -fopenmp -lomp")
set(CMAKE_CXX_FLAGS "-Xpreprocessor -fopenmp -lomp")
#set(CMAKE_SHARED_LINKER_FLAGS " -lomp" )
else()
set(CMAKE_CXX_FLAGS_DEBUG "-fopenmp")
set(CMAKE_CXX_FLAGS "-fopenmp")
endif()
endif()

include(../../../../tanlibrary/proj/cmake/utils/OpenCL.cmake)

# cl kernel to hex header
include(../../../../samples/proj/cmake/FileToHeader/FileToHeader.cmake)

# name
project(TrueAudioVR DESCRIPTION "TrueAudioVR")

#
ADD_DEFINITIONS(-DTAN_SDK_EXPORTS)
ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)
ADD_DEFINITIONS(-DTAN_SDK_EXPORTS)

include_directories(../../../../../amf)
include_directories(../../../..)
include_directories(../../../../common)

# sources
set(
  SOURCE_LIB
  ../../../src/TrueAudioVR/stdafx.cpp
  ../../../src/TrueAudioVR/TrueAudioVR.cpp
  ../../../../common/cpucaps.cpp
  )

if(WIN32)
  list(APPEND SOURCE_LIB ../../../src/TrueAudioVR/dllmain.cpp)
else()
endif()

set(
  HEADER_LIB

  ../../../src/TrueAudioVR/Header.h
  ../../../src/TrueAudioVR/resource.h
  ../../../src/TrueAudioVR/resource1.h
  ../../../src/TrueAudioVR/stdafx.h
  ../../../src/TrueAudioVR/targetver.h
  ../../../src/TrueAudioVR/TrueAudioVR.h

  ../../../../common/cpucaps.h
  )

# create library
add_library(
  TrueAudioVR
  SHARED
  ${SOURCE_LIB}
  ${HEADER_LIB}
  )

# cl kernels compilation
set(
  CL_Directories
  "${TAN_ROOT}/tan/samples/src/TrueAudioVR/Kernels"
  "${TAN_ROOT}/tan/samples/src/TrueAudioVR/Kernels"
  )
set(
  CL_Files
  "Fill.cl"
  "GenerateRoomResponse.cl"
  )
set(
  CL_Output
  "Fill"
  "GenerateRoomResponse"
  )

generateCLKernelHeader(
  TrueAudioVR
  CL_Directories
  CL_Files
  CL_Output
  TrueAudioVR
  )

# linking
target_link_libraries(TrueAudioVR ${OpenCL_LIBRARY})
target_link_libraries(TrueAudioVR amf)

if(NOT WIN32)
  target_compile_options(TrueAudioVR PUBLIC -mavx2)
  target_compile_options(TrueAudioVR PUBLIC -mfma)
  target_compile_options(TrueAudioVR PUBLIC -msse4.2)

  if(CMAKE_BUILD_TYPE MATCHES "Debug" OR CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
    target_compile_options(TrueAudioVR PUBLIC -g)
  endif()

  if(NOT APPLE)
    target_compile_options(TrueAudioVR PUBLIC -Wpsabi)
  endif()
endif()