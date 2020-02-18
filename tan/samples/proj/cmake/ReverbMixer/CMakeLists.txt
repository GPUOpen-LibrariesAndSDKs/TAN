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

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
#set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
#set(CMAKE_AUTOUIC ON)

# Find the QtWidgets library
find_package(Qt5Widgets CONFIG REQUIRED)
if(NOT Qt5Widgets_FOUND)
  message(SEND_ERROR "Failed to find Qt5Widgets")
  return()
endif()

# name
project(ReverbMixer DESCRIPTION "ReverbMixer")

if(PortAudio_DIR)
  ExternalProject_Get_Property(thirdparty_portaudio install_dir)
  message("portaudio install_dir: " ${install_dir})

  #add_subdirectory(${PortAudio_DIR} cmake-portaudio-bin EXCLUDE_FROM_ALL)

  ADD_DEFINITIONS(-DENABLE_PORTAUDIO)
  include_directories(${PortAudio_DIR}/include)
endif()

include_directories(${Qt5Widgets_INCLUDE_DIRS})
link_directories(${Qt5Widgets_LIBRARY})

include_directories(../../../../common)
include_directories(../../../../../amf)
include_directories(../../../../../tan)
include_directories(../../../src/common)

include_directories(../../../src/ReverbMixer)
include_directories(../../../src/ReverbMixer/UI)
include_directories(../../../src/ReverbMixer/QTObject)

ADD_DEFINITIONS(-D_CONSOLE)
ADD_DEFINITIONS(-D_LIB)
ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

# sources
set(
  SOURCE_EXE

  ../../../src/common/GpuUtils.cpp

  ../../../../common/FileUtility.cpp
  ../../../../common/StringUtility.cpp
  ../../../../common/fifo.cpp
  ../../../../common/wav.cpp

  ../../../src/ReverbMixer/ReverbMixer.cpp
  ../../../src/ReverbMixer/ReverbProcessor.cpp

  ../../../src/ReverbMixer/QTObject/QTReverbMixerMainWindow.cpp
  ../../../src/ReverbMixer/QTObject/QTSaveResponse.cpp
  )

set(
  HEADER_EXE

  ../../../../common/FileUtility.h
  ../../../../common/StringUtility.h
  ../../../../common/fifo.h
  ../../../../common/wav.h

  ../../../src/ReverbMixer/ReverbProcessor.h
  #../../../src/ReverbMixer/stdafx.h
  )

if(WIN32)
  list(APPEND SOURCE_EXE ../../../src/common/WASAPIPlayer.cpp)
else()
  if(NOT APPLE)
    list(APPEND SOURCE_EXE ../../../src/common/AlsaPlayer.cpp)
  endif()
endif()

if(WIN32)
  list(APPEND HEADER_EXE ../../../src/common/WASAPIPlayer.h)
else()
  if(NOT APPLE)
    list(APPEND HEADER_EXE ../../../src/common/AlsaPlayer.h)
  endif()
endif()

if(PortAudio_DIR)
  list(APPEND SOURCE_EXE ../../../src/common/PortPlayer.cpp)
  list(APPEND HEADER_EXE ../../../src/common/PortPlayer.h)
  message("PortAudio support injected to project")
endif()

set(
  UI_FILES
  ../../../src/ReverbMixer/UI/ReverbMixer.ui
  ../../../src/ReverbMixer/UI/QD_SaveResponse.ui
  )

set(
  MOC_SOURCE_FILES
  ../../../src/ReverbMixer/QTObject/QTSaveResponse.h
  ../../../src/ReverbMixer/QTObject/QTReverbMixerMainWindow.h
  )

# preprocess QT files
qt5_wrap_ui(UI_HEADERS ${UI_FILES})
qt5_wrap_cpp(MOC_HEADERS ${MOC_SOURCE_FILES})

# create binary
add_executable(
  ReverbMixer
  ${SOURCE_EXE}
  ${HEADER_EXE}
  ${UI_HEADERS}
  ${MOC_HEADERS}
  )

if(PortAudio_DIR)
  target_link_libraries(ReverbMixer portaudio)
  #target_link_libraries(ReverbMixer portaudio-static)
endif()

target_link_libraries(ReverbMixer amf)
target_link_libraries(ReverbMixer TrueAudioNext)
target_link_libraries(ReverbMixer GPUUtilities)
target_link_libraries(ReverbMixer Qt5::Widgets)

if(WIN32)
  target_link_libraries(ReverbMixer "version.lib")
  target_link_libraries(ReverbMixer "shlwapi.lib")
else()
  find_package(Threads REQUIRED)
  target_link_libraries(ReverbMixer ${CMAKE_THREAD_LIBS_INIT})

  target_link_libraries(ReverbMixer "dl")

  if(NOT APPLE)
    target_link_libraries(ReverbMixer "asound")
  endif()
endif()

#copy runtime dll to the binary dir
if(WIN32)
  #if(NOT AMF_CORE_STATIC)
  #  add_custom_command(
  #    TARGET ReverbMixer
  #    POST_BUILD
  #
  #    COMMAND ${CMAKE_COMMAND} -E copy_directory "${${AMF_NAME}_BINARY_DIR}/$<CONFIGURATION>" $<TARGET_FILE_DIR:ReverbMixer>
  #    )
  #endif()

  add_custom_command(
    TARGET ReverbMixer
    POST_BUILD
    #COMMAND ${CMAKE_COMMAND} -E copy_directory "${${AMF_NAME}_BINARY_DIR}/$<CONFIGURATION>" $<TARGET_FILE_DIR:ReverbMixer>
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${TrueAudioNext_BINARY_DIR}/$<CONFIGURATION>" $<TARGET_FILE_DIR:ReverbMixer>
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${GPUUtilities_BINARY_DIR}/$<CONFIGURATION>" $<TARGET_FILE_DIR:ReverbMixer>
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${RoomAcousticsQT_BINARY_DIR}/cmake-portaudio-bin/$<CONFIGURATION>" $<TARGET_FILE_DIR:ReverbMixer>

    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Core> $<TARGET_FILE_DIR:ReverbMixer>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Gui> $<TARGET_FILE_DIR:ReverbMixer>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Widgets> $<TARGET_FILE_DIR:ReverbMixer>
    )
endif()