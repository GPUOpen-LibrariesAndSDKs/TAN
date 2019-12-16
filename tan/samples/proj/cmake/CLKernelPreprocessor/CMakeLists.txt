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

include(../../../../tanlibrary/proj/cmake/utils/OpenCL.cmake)

# name
project(CLKernelPreprocessor DESCRIPTION "CLKernelPreprocessor")

include_directories(../../../../common)

ADD_DEFINITIONS(-D_CONSOLE)
ADD_DEFINITIONS(-D_LIB)
ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

# sources
set(
  SOURCE_EXE
  ../../../src/CLKernelPreprocessor/CLKernelPreprocessor.cpp
  ../../../src/CLKernelPreprocessor/stdafx.cpp

  ../../../../common/FileUtility.cpp
  ../../../../common/StringUtility.cpp
  )

set(
  HEADER_EXE
  ../../../src/CLKernelPreprocessor/stdafx.h
  ../../../src/CLKernelPreprocessor/targetver.h

  ../../../../common/FileUtility.h
  ../../../../common/StringUtility.h
  )

# create binary
add_executable(
  CLKernelPreprocessor
  ${SOURCE_EXE}
  ${HEADER_EXE}
  )

#if ( MSVC )
#  #etc for the other available configuration types (MinSizeRel, RelWithDebInfo)
#  set_target_properties(CLKernelPreprocessor PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
#  set_target_properties(CLKernelPreprocessor PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR})
#  set_target_properties(CLKernelPreprocessor PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR})
#endif ( MSVC )