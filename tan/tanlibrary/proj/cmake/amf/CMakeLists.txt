cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_SKIP_RULE_DEPENDENCY TRUE)

enable_language(CXX)

# library name
project(amf)

include_directories(../../../../../amf)

if(AMF_LIBRARY_NAME)
  message("AMF library name set to: " ${AMF_LIBRARY_NAME})
  message("this way does not work with some lexems, use environment variable AMF_DLL_NAME instead")

  ADD_DEFINITIONS(-DAMF_LIBRARY_NAME=${AMF_LIBRARY_NAME})
else()
  message("AMF will be built with default shared library name, use environment variable AMF_DLL_NAME to override it")
endif()

#ADD_DEFINITIONS(-DAMF_CORE_STATIC)

# sources
set(
  SOURCE_LIB

  ../../../../../amf/public/common/AMFFactory.cpp
  ../../../../../amf/public/common/AMFSTL.cpp
  ../../../../../amf/public/common/PropertyStorageExImpl.cpp
  ../../../../../amf/public/common/Thread.cpp
  ../../../../../amf/public/common/TraceAdapter.cpp
  ../../../../../amf/public/common/DataStreamFactory.cpp
  ../../../../../amf/public/common/DataStreamFile.cpp
  ../../../../../amf/public/common/DataStreamMemory.cpp
  ../../../../../amf/public/common/IOCapsImpl.cpp
  ../../../../../amf/public/common/PropertyStorageExImpl.cpp
)

if(WIN32)
  list(APPEND SOURCE_LIB ../../../../../amf/public/common/Windows/ThreadWindows.cpp)
else()
  list(APPEND SOURCE_LIB ../../../../../amf/public/common/Linux/ThreadLinux.cpp)
endif()

set(
  HEADER_LIB

  ../../../../../amf/public/common/AMFFactory.h
  ../../../../../amf/public/common/AMFSTL.h
  ../../../../../amf/public/common/ByteArray.h
  ../../../../../amf/public/common/DataStream.h
  ../../../../../amf/public/common/DataStreamFile.h
  ../../../../../amf/public/common/DataStreamMemory.h
  ../../../../../amf/public/common/InterfaceImpl.h
  ../../../../../amf/public/common/IOCapsImpl.h
  ../../../../../amf/public/common/ObservableImpl.h
  ../../../../../amf/public/common/PropertyStorageExImpl.h
  ../../../../../amf/public/common/PropertyStorageImpl.h
  ../../../../../amf/public/common/Thread.h
  ../../../../../amf/public/common/TraceAdapter.h

  ../../../../../amf/public/include/core/AudioBuffer.h
  ../../../../../amf/public/include/core/Buffer.h
  ../../../../../amf/public/include/core/Compute.h
  ../../../../../amf/public/include/core/ComputeFactory.h
  ../../../../../amf/public/include/core/Context.h
  ../../../../../amf/public/include/core/Data.h
  ../../../../../amf/public/include/core/Debug.h
  ../../../../../amf/public/include/core/Factory.h
  ../../../../../amf/public/include/core/Interface.h
  ../../../../../amf/public/include/core/Plane.h
  ../../../../../amf/public/include/core/Platform.h
  ../../../../../amf/public/include/core/PropertyStorage.h
  ../../../../../amf/public/include/core/PropertyStorageEx.h
  ../../../../../amf/public/include/core/Result.h
  ../../../../../amf/public/include/core/Surface.h
  ../../../../../amf/public/include/core/Trace.h
  ../../../../../amf/public/include/core/Variant.h
  ../../../../../amf/public/include/core/Version.h
  )


# declare static library creation
add_library(
  amf
  STATIC
  ${SOURCE_LIB}
  ${HEADER_LIB}
  )

set_property(TARGET amf PROPERTY POSITION_INDEPENDENT_CODE ON)

if(NOT WIN32)
  find_package(Threads REQUIRED)
  target_link_libraries(amf ${CMAKE_THREAD_LIBS_INIT})
  target_link_libraries(amf "dl")

  target_compile_options(amf PUBLIC -mavx2)
  target_compile_options(amf PUBLIC -mfma)
  target_compile_options(amf PUBLIC -msse4.2)

  if(CMAKE_BUILD_TYPE MATCHES "Debug" OR CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
    target_compile_options(amf PUBLIC -g)
  endif()

  if(NOT APPLE)
    target_compile_options(amf PUBLIC -Wpsabi)
  endif()
endif()