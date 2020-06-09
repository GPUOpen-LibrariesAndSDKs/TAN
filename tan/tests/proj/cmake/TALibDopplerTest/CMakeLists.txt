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
project(TALibDopplerTest DESCRIPTION "TALibDopplerTest")

include_directories(../../../../common)

ADD_DEFINITIONS(-D_CONSOLE)
ADD_DEFINITIONS(-D_LIB)
ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

include_directories(${AMF_HOME})
include_directories(../../../../../tan)

if(IPP_DIR AND IS_DIRECTORY ${IPP_DIR})
  # enable IPP
  link_directories(${IPP_DIR}/lib/intel64_win)
endif()

# sources
set(
  SOURCE_EXE
  ../../../../common/FileUtility.cpp
  ../../../../common/StringUtility.cpp
  ../../../../common/wav.cpp

  ../../../src/TALibDopplerTest/stdafx.cpp
  ../../../src/TALibDopplerTest/TALibDopplerTest.cpp
  )

set(
  HEADER_EXE
  ../../../../common/FileUtility.h
  ../../../../common/StringUtility.h
  ../../../../common/wav.h

  ../../../src/TALibDopplerTest/stdafx.h
  ../../../src/TALibDopplerTest/targetver.h
  )

# create binary
add_executable(
  TALibDopplerTest
  ${SOURCE_EXE}
  ${HEADER_EXE}
  )

target_link_libraries(TALibDopplerTest TrueAudioVR)
target_link_libraries(TALibDopplerTest TrueAudioNext)
target_link_libraries(TALibDopplerTest GPUUtilities)

#copy runtime dll to the binary dir
add_custom_command(
  TARGET TALibDopplerTest
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:TrueAudioNext>" $<TARGET_FILE_DIR:TALibDopplerTest>
  COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:TrueAudioVR>" $<TARGET_FILE_DIR:TALibDopplerTest>
  COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:GPUUtilities>" $<TARGET_FILE_DIR:TALibDopplerTest>
  )