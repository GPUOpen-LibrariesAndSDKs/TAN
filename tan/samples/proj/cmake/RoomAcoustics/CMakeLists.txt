cmake_minimum_required(VERSION 3.10)

if(WIN32)
  if(MSVC)

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

    find_package(MFC REQUIRED)
    if(NOT MFC_FOUND)
      message(FATAL_ERROR "Failed to find MFC")
      return()
    endif()

    # name
    project(RoomAcoustics DESCRIPTION "RoomAcoustics")

    include_directories(${AMF_HOME})
    include_directories(../../../../../tan)
    include_directories(../../../src/common)
    include_directories(../../../../common)

    ADD_DEFINITIONS(-DRTQ_ENABLED)
    set(CMAKE_MFC_FLAG 1)

    macro(replace_flags var these those)
      if("${${var}}" MATCHES "${these}")
        string(REGEX REPLACE "${these}" "${those}" ${var} "${${var}}")
        #message(STATUS "info: ${var} changed to '${${var}}'")
      endif()
      message(STATUS "info: ${var}='${${var}}'")
    endmacro()

    macro(msvc_link_to_static_crt)
      if(MSVC)
        set(has_correct_flag 0)
        foreach(lang C CXX)
        foreach(suffix "" _DEBUG _MINSIZEREL _RELEASE _RELWITHDEBINFO)
          replace_flags("CMAKE_${lang}_FLAGS${suffix}" "/MD" "/MT")
          if(CMAKE_${lang}_FLAGS${suffix} MATCHES "/MT")
            set(has_correct_flag 1)
          endif()
        endforeach()
        endforeach()
        if(NOT has_correct_flag)
          message(FATAL_ERROR "no CMAKE_*_FLAGS var contains /MT")
        endif()
      endif()
    endmacro()

    msvc_link_to_static_crt()

    # sources
    set(
      SOURCE_EXE
      #resources
      ../../../src/RoomAcoustics/RoomAcoustics.rc

      ../../../src/common/GpuUtils.cpp
      ../../../src/common/WASAPIPlayer.cpp

      ../../../../common/wav.cpp
      ../../../../common/fifo.cpp

      ../../../src/RoomAcoustics/RoomAcoustics.cpp
      ../../../src/RoomAcoustics/RoomAcousticsDlg.cpp
      ../../../src/RoomAcoustics/RoomWindow.cpp
      ../../../src/RoomAcoustics/AudioVR.cpp

      ../../../src/RoomAcoustics/stdafx.cpp

      #excluded
      #../../../src/RoomAcoustics/AudioVR.cpp
      )

    set(
      HEADER_EXE

      ../../../../tanlibrary/include/TrueAudioNext.h

      ../../../../common/cpucaps.h
      ../../../../common/wav.h
      ../../../../common/fifo.h

      ../../../src/common/GpuUtils.h
      ../../../src/common/maxlimits.h
      ../../../src/common/SimpleVRaudio.h
      ../../../src/common/WASAPIPlayer.h

      ../../../src/RoomAcoustics/Resource.h
      ../../../src/RoomAcoustics/RoomAcoustics.h
      ../../../src/RoomAcoustics/RoomAcousticsDlg.h
      ../../../src/RoomAcoustics/RoomWindow.h
      ../../../src/RoomAcoustics/AudioVR.h

      ../../../src/RoomAcoustics/stdafx.h
      ../../../src/RoomAcoustics/targetver.h

      #excluded
      #../../../src/RoomAcoustics/AudioVR.h
      )

    # create binary
    add_executable(
      RoomAcoustics
      WIN32
      ${SOURCE_EXE}
      ${HEADER_EXE}
      )

    target_link_libraries(RoomAcoustics amf)
    target_link_libraries(RoomAcoustics TrueAudioNext)
    target_link_libraries(RoomAcoustics "version.lib")
    target_link_libraries(RoomAcoustics ${OpenCL_LIBRARY})

    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} nafxcwd.lib LIBCMTD.lib /NODEFAULTLIB:nafxcwd /NODEFAULTLIB:LIBCMTD")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /NODEFAULTLIB:nafxcw /NODEFAULTLIB:LIBCMT")

    target_link_libraries(RoomAcoustics debug "nafxcwd.lib")
    target_link_libraries(RoomAcoustics debug "LIBCMTD.lib")
    target_link_libraries(RoomAcoustics optimized "nafxcw.lib")
    target_link_libraries(RoomAcoustics optimized "LIBCMT.lib")

  endif(MSVC)
endif(WIN32)