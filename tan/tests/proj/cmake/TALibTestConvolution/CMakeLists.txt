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
project(TALibTestConvolution DESCRIPTION "TALibTestConvolution")

include_directories(../../../../common)

ADD_DEFINITIONS(-D_CONSOLE)
ADD_DEFINITIONS(-D_LIB)
ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

include_directories(../../../../../amf)
include_directories(../../../../../tan)

# sources
set(
  SOURCE_EXE
  ../../../../samples/src/common/utilities.c

  ../../../../common/FileUtility.cpp
  ../../../../common/wav.cpp

  ../../../src/TALibTestConvolution/TALibTestConvolution.cpp
  )

set(
  HEADER_EXE

  ../../../../common/FileUtility.h
  ../../../../common/wav.h

  ../../../../samples/src/common/utilities.h
  )

# create binary
add_executable(
  TALibTestConvolution
  ${SOURCE_EXE}
  ${HEADER_EXE}
  )

target_link_libraries(TALibTestConvolution TrueAudioNext)
target_link_libraries(TALibTestConvolution GPUUtilities)