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

include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

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
project(RoomAcousticsQT DESCRIPTION "RoomAcousticsQT")

if(PortAudio_DIR)
  message("PortAudio_DIR: " ${PortAudio_DIR})
  message("PortAudio argument: -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}")

  INCLUDE(ExternalProject)
  ExternalProject_Add(
    thirdparty_portaudio
    #URl                 "http://www.portaudio.com/archives/pa_stable_v190600_20161030.tgz"
    SOURCE_DIR          ${PortAudio_DIR}
    #GIT_REPOSITORY      https://git.assembla.com/portaudio.git
    PREFIX              thirdparty_portaudio
    #PREFIX              lib/external_portaudio
    UPDATE_COMMAND      ""
    #CONFIGURE_COMMAND   ${PortAudio_DIR}/configure
    CONFIGURE_COMMAND   "" #cmake ${PortAudio_DIR} -G "Unix Makefiles"
    BUILD_IN_SOURCE     0
    #BUILD_COMMAND       make
    BUILD_COMMAND       ""
    INSTALL_COMMAND     "" #sudo make install
    CMAKE_ARGS          -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    )

  add_subdirectory(${PortAudio_DIR} cmake-portaudio-bin EXCLUDE_FROM_ALL)

  ADD_DEFINITIONS(-DENABLE_PORTAUDIO)
  include_directories(${PortAudio_DIR}/include)
endif()

include_directories(${Qt5Widgets_INCLUDE_DIRS})
link_directories(${Qt5Widgets_LIBRARY})

if(IPP_DIR AND IS_DIRECTORY ${IPP_DIR})
  # enable IPP
  link_directories(${IPP_DIR}/lib/intel64_win)
endif()

include_directories(../../../../../amf)
include_directories(../../../../../tan)
include_directories(../../../src/common)
include_directories(../../../../common)

include_directories(../../../src/RoomAcousticQT/QTObject)

if(WIN32)
  ADD_DEFINITIONS(-DRTQ_ENABLED)
endif()

# sources
set(
  SOURCE_EXE
  ../../../src/RoomAcousticQT/Resources/Resource.rc

  ../../../src/common/GpuUtils.cpp
  ../../../src/common/SimpleVRaudio.cpp
  ../../../src/common/utilities.c

  ../../../../common/FileUtility.cpp
  ../../../../common/StringUtility.cpp
  ../../../../common/RegisterBrowser.cpp
  ../../../../common/wav.cpp
  ../../../../common/fifo.cpp
  ../../../../common/cpucaps.cpp

  ../../../src/RoomAcousticQT/main.cpp
  ../../../src/RoomAcousticQT/QTObject/AdvanceGraphicView.cpp
  ../../../src/RoomAcousticQT/QTObject/QTExportResponse.cpp
  ../../../src/RoomAcousticQT/QTObject/QTRoomAcousticConfig.cpp
  ../../../src/RoomAcousticQT/QTObject/QTRoomAcousticGraphic.cpp
  ../../../src/RoomAcousticQT/RoomAcousticQT.cpp
  )

if(WIN32)
  list(APPEND SOURCE_EXE ../../../src/common/WASAPIPlayer.cpp)
else()
if(NOT APPLE)
  list(APPEND SOURCE_EXE ../../../src/common/AlsaPlayer.cpp)
endif()

endif()

set(
  HEADER_EXE

  ../../../../common/fifo.h

  ../../../src/common/GpuUtils.h
  ../../../src/common/maxlimits.h
  ../../../src/common/SimpleVRaudio.h
  ../../../src/common/utilities.h
  ../../../src/common/WasAPI.h
  ../../../src/common/IWavPlayer.h

  ../../../../common/FileUtility.h
  ../../../../common/StringUtility.h
  ../../../../common/RegisterBrowser.h
  ../../../../common/wav.h
  ../../../../common/cpucaps.h

  ../../../src/RoomAcousticQT/QTObject/AdvanceGraphicView.h
  ../../../src/RoomAcousticQT/Resources/resource.h
  ../../../src/RoomAcousticQT/RoomAcousticQT.h
)

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
  ../../../src/RoomAcousticQT/UI/RoomAcousticConfig.ui
 # ../../../src/RoomAcousticQT/UI/RoomAcousticConfig_NoRTQ.ui
  ../../../src/RoomAcousticQT/UI/ExportResponse.ui
  )

set(
  MOC_SOURCE_FILES
  ../../../src/RoomAcousticQT/QTObject/QTRoomAcousticConfig.h
  ../../../src/RoomAcousticQT/QTObject/QTRoomAcousticGraphic.h
  ../../../src/RoomAcousticQT/QTObject/QTExportResponse.h
  )

set(
  QRC_FILES
  ../../../src/RoomAcousticQT/roomaccousticnew.qrc
  )

# preprocess QT files
qt5_wrap_ui(UI_HEADERS ${UI_FILES})
qt5_wrap_cpp(MOC_HEADERS ${MOC_SOURCE_FILES})
qt5_add_resources(RCC_SOURCES ${QRC_FILES})

# create binary
add_executable(
  RoomAcousticsQT
  ${SOURCE_EXE}
  ${HEADER_EXE}
  ${UI_HEADERS}
  ${MOC_HEADERS}
  ${RCC_SOURCES}
  )

target_link_libraries(RoomAcousticsQT amf)
target_link_libraries(RoomAcousticsQT TrueAudioNext)
target_link_libraries(RoomAcousticsQT TrueAudioVR)
target_link_libraries(RoomAcousticsQT GPUUtilities)
target_link_libraries(RoomAcousticsQT ${OpenCL_LIBRARY})
target_link_libraries(RoomAcousticsQT Qt5::Widgets)

if(WIN32)
  target_link_libraries(RoomAcousticsQT "version.lib")
  target_link_libraries(RoomAcousticsQT "shlwapi.lib")
else()
  find_package(Threads REQUIRED)
  target_link_libraries(RoomAcousticsQT ${CMAKE_THREAD_LIBS_INIT})

  target_link_libraries(RoomAcousticsQT "dl")

  if(NOT APPLE)
    target_link_libraries(RoomAcousticsQT "asound")
  endif()
endif()

if(PortAudio_DIR)
  target_link_libraries(RoomAcousticsQT portaudio)
  #target_link_libraries(RoomAcousticsQT portaudio-static)
endif()

if(IPP_DIR AND IS_DIRECTORY ${IPP_DIR})
  # enable IPP
  target_link_libraries(RoomAcousticsQT ippimt)
  target_link_libraries(RoomAcousticsQT ippsmt)
  target_link_libraries(RoomAcousticsQT ippvmmt)
  target_link_libraries(RoomAcousticsQT ippcoremt)
endif()

if(NOT WIN32)
  target_compile_options(RoomAcousticsQT PUBLIC -mavx2)
  target_compile_options(RoomAcousticsQT PUBLIC -mfma)
  target_compile_options(RoomAcousticsQT PUBLIC -msse4.2)

  if(CMAKE_BUILD_TYPE MATCHES "Debug" OR CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
    target_compile_options(RoomAcousticsQT PUBLIC -g)
  endif()

  if(NOT APPLE)
    target_compile_options(RoomAcousticsQT PUBLIC -Wpsabi)
  endif()
endif()

#copy runtime dll to the binary dir
if(WIN32)
  add_custom_command(
    TARGET RoomAcousticsQT
    POST_BUILD
    
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${TAN_ROOT}/tan/tanlibrary/src/fftw-3.3.5-dll64/libfftw3f-3.dll $<TARGET_FILE_DIR:RoomAcousticsQT>
    
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Qt5::QWindowsIntegrationPlugin> "$<TARGET_FILE_DIR:RoomAcousticsQT>/platforms/qwindows.dll"    
    )
endif()
  
add_custom_command(
  TARGET RoomAcousticsQT
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:TrueAudioNext>" $<TARGET_FILE_DIR:RoomAcousticsQT>
  COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:TrueAudioVR>" $<TARGET_FILE_DIR:RoomAcousticsQT>
  COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:GPUUtilities>" $<TARGET_FILE_DIR:RoomAcousticsQT>

  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Qt5::Core> $<TARGET_FILE_DIR:RoomAcousticsQT>
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Qt5::Gui> $<TARGET_FILE_DIR:RoomAcousticsQT>
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Qt5::Widgets> $<TARGET_FILE_DIR:RoomAcousticsQT>
  )

if(PortAudio_DIR)
  add_custom_command(
    TARGET RoomAcousticsQT
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:portaudio>" $<TARGET_FILE_DIR:RoomAcousticsQT>
    #COMMAND ${CMAKE_COMMAND} -E copy_directory "${RoomAcousticsQT_BINARY_DIR}/cmake-portaudio-bin/$<CONFIGURATION>" $<TARGET_FILE_DIR:RoomAcousticsQT>
    )
endif()