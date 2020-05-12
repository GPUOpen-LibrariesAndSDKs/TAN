cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_SKIP_RULE_DEPENDENCY TRUE)

enable_language(CXX)

find_package(OpenMP)
message("OpenMP_VERSION: ${OpenMP_VERSION}")
message("OpenMP_CXX_FOUND: ${OpenMP_CXX_FOUND}")
message("OpenMP_CXX_FLAGS: ${OpenMP_CXX_FLAGS}")
message("OpenMP_CXX_LIB_NAMES: ${OpenMP_CXX_LIB_NAMES}")
message("OpenMP_CXX_LIBRARY: ${OpenMP_CXX_LIBRARY}")
message("OpenMP_CXX_LIBRARIES: ${OpenMP_CXX_LIBRARIES}")

if(OpenMP_FOUND AND OpenMP_CXX_LIBRARIES)
  ADD_DEFINITIONS(-DOMP_ENABLED)
else()
  #find_path(OMP_INCLUDE_DIR NAMES omp.h)
  find_library(OpenMP_CXX_LIBRARIES NAMES omp)

  if(OpenMP_CXX_LIBRARIES)
    set(OpenMP_FOUND 1)
  endif()
endif()

if(OpenMP_FOUND)
  #message("OMP_INCLUDE_DIR: ${OMP_INCLUDE_DIR}")
  message("OpenMP_CXX_LIBRARIES: ${OpenMP_CXX_LIBRARIES}")

  if((NOT OpenMP_CXX_FLAGS) OR (NOT OpenMP_C_FLAGS))
    set(OpenMP_C_FLAGS "-fopenmp")
    set(OpenMP_CXX_FLAGS "-fopenmp")

    if(APPLE OR XCODE)
      set(OpenMP_C_FLAGS "-Xpreprocessor ${OpenMP_C_FLAGS}")
      set(OpenMP_CXX_FLAGS "-Xpreprocessor ${OpenMP_CXX_FLAGS}")
    endif()
  endif()

  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${OpenMP_CXX_FLAGS}")
else()
  message(FATAL_ERROR "Error: OpenMP not found!")
endif()

include(../../../../tanlibrary/proj/cmake/utils/OpenCL.cmake)

# cl kernel preprocessor
include(../../../../samples/proj/cmake/CLKernelPreprocessor/CLKernelPreprocessor.cmake)

include_directories(../../../../../amf)
include_directories(../../../..)
include_directories(../../../../common)

if(OpenMP_FOUND)
  #include_directories(${OMP_INCLUDE_DIR})
endif()

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

set_property(TARGET Graal2 PROPERTY POSITION_INDEPENDENT_CODE ON)

if(NOT WIN32)
  target_compile_options(Graal2 PUBLIC -mavx2)
  target_compile_options(Graal2 PUBLIC -mfma)
  target_compile_options(Graal2 PUBLIC -msse4.2)

  if(CMAKE_BUILD_TYPE MATCHES "Debug" OR CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
    target_compile_options(Graal2 PUBLIC -g)
  endif()

  if(NOT APPLE)
    target_compile_options(Graal2 PUBLIC -Wpsabi)
  endif()
endif()