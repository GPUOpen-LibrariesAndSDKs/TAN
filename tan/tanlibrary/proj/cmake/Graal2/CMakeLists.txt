cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_SKIP_RULE_DEPENDENCY TRUE)

enable_language(CXX)

include(../../../../tanlibrary/proj/cmake/utils/OpenCL.cmake)

# cl kernel preprocessor
include(../../../../samples/proj/cmake/CLKernelPreprocessor/CLKernelPreprocessor.cmake)

find_package(OpenMP)
if(OpenMP_FOUND)
  ADD_DEFINITIONS(-DOMP_ENABLED)
else()
  find_path(OMP_INCLUDE_DIR NAMES omp.h)
  find_library(OpenMP_LIBRARY NAMES omp)

  if(OMP_INCLUDE_DIR AND OpenMP_LIBRARY)
    ADD_DEFINITIONS(-DOMP_ENABLED)
    set(OpenMP_FOUND 1)
    message("OMP_INCLUDE_DIR: ${OMP_INCLUDE_DIR}")
    message("OpenMP_LIBRARY: ${OpenMP_LIBRARY}")
  endif()
endif()

if(OpenMP_FOUND)
  if(NOT WIN32)
    if(APPLE OR XCODE)
      set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Xpreprocessor -fopenmp -lomp")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xpreprocessor -fopenmp -lomp")
    else()
      set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fopenmp")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
    endif()
  endif()
endif()

if(OpenMP_FOUND)
  include_directories(${OMP_INCLUDE_DIR})
endif()

include_directories(../../../../../amf)
include_directories(../../../..)
include_directories(../../../../common)

ADD_DEFINITIONS(-DTAN_SDK_EXPORTS)
ADD_DEFINITIONS(-DCLFFT_EXPORTS)
ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

# library name
project(Graal2)

# sources
set(
  SOURCE_LIB
  ../../../src/common/OCLHelper.cpp
  ../../../src/Graal2/amdFHT.cpp
  ../../../src/Graal2/amdFHT_OCL.cpp
  ../../../src/Graal2/amdFIR_OCL.cpp
  ../../../src/Graal2/DirectConv.cpp
  ../../../src/Graal2/Graal.cpp
  ../../../src/Graal2/GraalWrapper.cpp
  ../../../src/Graal2/OpenCLInit.cpp
  ../../../src/Graal2/Reverberate.cpp
  ../../../src/Graal2/Reverberate2.cpp
)

set(
  HEADER_LIB
  ../../../src/common/OCLHelper.h
  ../../../src/Graal2/amdFHT.h
  ../../../src/Graal2/amdFHT_OCL.h
  ../../../src/Graal2/amdFIR_OCL.h
  ../../../src/Graal2/DirectConv_OCL.h
  ../../../src/Graal2/Graal.h
  ../../../src/Graal2/GraalInternal.h
  ../../../src/Graal2/GraalWrapper.h
  ../../../src/Graal2/IIRLowLat.h
  ../../../src/Graal2/IIRLowLat_OCL.h
  ../../../src/Graal2/Reverberate_OCL.h
  ../../../src/Graal2/Reverberate2_OCL.h
  )

# cl kernels compilation
set(
  Graal2_CL_Directories
  "${TAN_ROOT}/tan/tanlibrary/src/Graal2"
  "${TAN_ROOT}/tan/tanlibrary/src/Graal2"
  "${TAN_ROOT}/tan/tanlibrary/src/Graal2"
  "${TAN_ROOT}/tan/tanlibrary/src/Graal2"
  "${TAN_ROOT}/tan/tanlibrary/src/Graal2"
  "${TAN_ROOT}/tan/tanlibrary/src/Graal2"
  )
set(
  Graal2_CL_Files
  "amdFHT_kernels.cl"
  "amdFHT2_kernels.cl"
  "amdFHTbig_kernels.cl"
  "amdFHTmad_kernels.cl"
  "amdFIR_kernels.cl"
  "Util_kernels.cl"
  )
set(
  Graal2_CL_Output
  "OclKernels/CLKernel_amdFHT_kernels.h"
  "OclKernels/CLKernel_amdFHT2_kernels.h"
  "OclKernels/CLKernel_amdFHTbig_kernels.h"
  "OclKernels/CLKernel_amdFHTmad_kernels.h"
  "OclKernels/CLKernel_amdFIR_kernels.h"
  "OclKernels/CLKernel_Util_kernels.h"
  )

markGenerated(
  Graal2_CL_Directories
  Graal2_CL_Output
  OutHeaders
  Graal2
  )

#append generated headers
list(LENGTH OutHeaders OutHeadersCount)
math(EXPR OutHeaders_MaxIndex ${OutHeadersCount}-1)

foreach(FileIndex RANGE ${OutHeaders_MaxIndex})
  list(GET OutHeaders ${FileIndex} OutFile)
  list(APPEND HEADER_LIB ${OutFile})
endforeach()

# declare static library creation
# that embeds OpenCL kernels
add_library(
  Graal2
  STATIC
  ${SOURCE_LIB}
  ${HEADER_LIB}
  )

generateCLKernel(
  Graal2
  Graal2_CL_Directories
  Graal2_CL_Files
  Graal2_CL_Output
  Graal2
  )

add_dependencies(Graal2 CLKernelPreprocessor)

target_link_libraries(Graal2 amf)

set_property(TARGET Graal2 PROPERTY POSITION_INDEPENDENT_CODE ON)

if(NOT WIN32)
  target_compile_options(Graal2 PUBLIC -mavx2)
  target_compile_options(Graal2 PUBLIC -mfma)
  target_compile_options(Graal2 PUBLIC -msse4.2)
  target_compile_options(Graal2 PUBLIC -g) #todo: to Debug only

  if(NOT APPLE)
    target_compile_options(Graal2 PUBLIC -Wpsabi)
  endif()
endif()