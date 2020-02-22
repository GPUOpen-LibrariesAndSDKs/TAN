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

# cl kernel preprocessor
include(../../../../samples/proj/cmake/CLKernelPreprocessor/CLKernelPreprocessor.cmake)

# name
project(TrueAudioNext VERSION 1.0.0 DESCRIPTION "TAN")

include_directories(../../../../common)

#
ADD_DEFINITIONS(-DLEGACY_COPYING=1)
ADD_DEFINITIONS(-DCONVOLUTION_EXPORTS)
ADD_DEFINITIONS(-DTRUEAUDIONEXT_EXPORTS)
ADD_DEFINITIONS(-DTAN_SDK_EXPORTS)
#ADD_DEFINITIONS(-D_M_X64) is it needed?
ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)
ADD_DEFINITIONS(-DNMC_VECTORIZE_TARGET=AVX2)

include_directories(../../../../../amf)
include_directories(../../../..)

# sources
set(
  SOURCE_LIB

  ../../../src/common/OCLHelper.cpp

  ../../../../common/StringUtility.cpp
  ../../../../common/cpucaps.cpp

  ../../../src/TrueAudioNext/converter/ConverterImpl.cpp
  ../../../src/TrueAudioNext/convolution/ConvolutionImpl.cpp
  ../../../src/TrueAudioNext/core/TANContextImpl.cpp
  ../../../src/TrueAudioNext/core/TANTraceAndDebug.cpp
  ../../../src/TrueAudioNext/fft/FFTImpl.cpp
  ../../../src/TrueAudioNext/filter/FilterImpl.cpp
  ../../../src/TrueAudioNext/IIRfilter/IIRfilterImpl.cpp
  ../../../src/TrueAudioNext/math/MathImpl.cpp
  ../../../src/TrueAudioNext/mixer/MixerImpl.cpp
  )

if(WIN32)
  list(APPEND SOURCE_LIB ../../../src/TrueAudioNext/core/windows/dll.cpp)
else()
  list(APPEND SOURCE_LIB ../../../src/TrueAudioNext/core/linux/dll.cpp)
endif()

set(
  HEADER_LIB

  ../../../include/TrueAudioNext.h
  ../../../src/common/OCLHelper.h
  ../../../src/TrueAudioNext/converter/ConverterImpl.h
  #../../../src/TrueAudioNext/convolution/CLKernel_ConvolutionTD.h
  ../../../src/TrueAudioNext/convolution/ConvolutionImpl.h
  ../../../src/TrueAudioNext/core/TANContextImpl.h
  ../../../src/TrueAudioNext/core/TANTraceAndDebug.h
  ../../../src/TrueAudioNext/fft/FFTImpl.h
  ../../../src/TrueAudioNext/filter/FilterImpl.h
  ../../../src/TrueAudioNext/IIRfilter/IIRfilterImpl.h
  ../../../src/TrueAudioNext/math/MathImpl.h
  ../../../src/TrueAudioNext/mixer/MixerImpl.h
  ../../../src/TrueAudioNext/resource.h
  )

# cl kernels for compilation
set(
  Tan_CL_Directories
  "${TAN_ROOT}/tan/tanlibrary/src/TrueAudioNext/convolution"
  "${TAN_ROOT}/tan/tanlibrary/src/TrueAudioNext/convolution"
  "${TAN_ROOT}/tan/tanlibrary/src/TrueAudioNext/convolution"
  "${TAN_ROOT}/tan/tanlibrary/src/TrueAudioNext/mixer"
  "${TAN_ROOT}/tan/tanlibrary/src/TrueAudioNext/math"
  "${TAN_ROOT}/tan/tanlibrary/src/TrueAudioNext/math"
  "${TAN_ROOT}/tan/tanlibrary/src/TrueAudioNext/math"
  "${TAN_ROOT}/tan/tanlibrary/src/TrueAudioNext/math"
  "${TAN_ROOT}/tan/tanlibrary/src/TrueAudioNext/converter"
  "${TAN_ROOT}/tan/tanlibrary/src/TrueAudioNext/IIRfilter"
  )
set(
  Tan_CL_Files
  "Crossfading.cl"
  "TimeDomainConvolution.cl"
  "SimpleCopy.cl"
  "Mixer.cl"
  "VectorComplexSum.cl"
  "VectorComplexMultiply.cl"
  "VectorComplexDivision.cl"
  "VectorComplexMultiplyAccumulate.cl"
  "Converter.cl"
  "IIRfilter.cl"
  )
set(
  Tan_CL_Output
  "CLKernel_Crossfading.h"
  "CLKernel_TimeDomainConvolution.h"
  "CLKernel_SimpleCopy.h"
  "CLKernel_Mixer.h"
  "CLKernel_VectorComplexSum.h"
  "CLKernel_VectorComplexMultiply.h"
  "CLKernel_VectorComplexDivision.h"
  "CLKernel_VectorComplexMultiplyAccumulate.h"
  "CLKernel_Converter.h"
  "CLKernel_IIRfilter.h"
  )

markGenerated(
  Tan_CL_Directories
  Tan_CL_Output
  OutHeaders
  )

#append generated headers
list(LENGTH OutHeaders OutHeadersCount)
math(EXPR OutHeaders_MaxIndex ${OutHeadersCount}-1)

foreach(FileIndex RANGE ${OutHeaders_MaxIndex})
  list(GET OutHeaders ${FileIndex} OutFile)
  list(APPEND HEADER_LIB ${OutFile})
endforeach()

# create library
add_library(
  TrueAudioNext
  SHARED
  ${SOURCE_LIB}
  ${HEADER_LIB}
  )

generateCLKernel(
  TrueAudioNext
  Tan_CL_Directories
  Tan_CL_Files
  Tan_CL_Output
  TAN
  )

#add_dependencies(TrueAudioNext Graal)
#add_dependencies(TrueAudioNext Graal2)
#add_dependencies(TrueAudioNext clFFT-master)

# linking
target_link_libraries(TrueAudioNext ${OpenCL_LIBRARY})
target_link_libraries(TrueAudioNext amf)
target_link_libraries(TrueAudioNext Graal)
target_link_libraries(TrueAudioNext Graal2)
target_link_libraries(TrueAudioNext clFFT-master)

# Declare public API of your library.
# This API will be installed for third-party application.
# It is a good practice to isolate it in your project tree (like placing it include/ directory).
# Notice that, private headers should not been installed and it is strongly suggested to place them with sources files.
set_target_properties(
  TrueAudioNext
  PROPERTIES
  PUBLIC_HEADER
  ../../../include/TrueAudioNext.h
  )

if(NOT WIN32)
  target_compile_options(TrueAudioNext PUBLIC -mavx2)
  target_compile_options(TrueAudioNext PUBLIC -mfma)
  target_compile_options(TrueAudioNext PUBLIC -msse4.2)

  if(CMAKE_BUILD_TYPE MATCHES "Debug" OR CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
    target_compile_options(TrueAudioNext PUBLIC -g)
  endif()

  if(NOT APPLE)
    target_compile_options(TrueAudioNext PUBLIC -Wpsabi)
  endif()
endif()