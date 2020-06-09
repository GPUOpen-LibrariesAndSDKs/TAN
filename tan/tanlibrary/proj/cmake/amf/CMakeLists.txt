cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_SKIP_RULE_DEPENDENCY TRUE)

enable_language(CXX)

include(../../../../tanlibrary/proj/cmake/utils/OpenCL.cmake)

# library name
project(amf)

include_directories(${AMF_HOME})

if(AMF_LIBRARY_NAME)
  message("AMF library name set to: " ${AMF_LIBRARY_NAME})
  message("this way does not work with some lexems, use environment variable AMF_DLL_NAME instead")

  ADD_DEFINITIONS(-DAMF_LIBRARY_NAME=${AMF_LIBRARY_NAME})
else()
  message("AMF will be built with default shared library name, use environment variable AMF_DLL_NAME to override it")
endif()

# sources
set(
  SOURCE_LIB

  ${AMF_HOME}/public/common/AMFFactory.cpp
  ${AMF_HOME}/public/common/AMFSTL.cpp
  ${AMF_HOME}/public/common/PropertyStorageExImpl.cpp
  ${AMF_HOME}/public/common/Thread.cpp
  ${AMF_HOME}/public/common/TraceAdapter.cpp
  ${AMF_HOME}/public/common/DataStreamFactory.cpp
  ${AMF_HOME}/public/common/DataStreamFile.cpp
  ${AMF_HOME}/public/common/DataStreamMemory.cpp
  ${AMF_HOME}/public/common/IOCapsImpl.cpp
  ${AMF_HOME}/public/common/PropertyStorageExImpl.cpp
)

if(WIN32)
  list(APPEND SOURCE_LIB ${AMF_HOME}/public/common/Windows/ThreadWindows.cpp)
else()
  list(APPEND SOURCE_LIB ${AMF_HOME}/public/common/Linux/ThreadLinux.cpp)
endif()

if(AMF_CORE_STATIC)
  list(APPEND SOURCE_LIB ${AMF_HOME}/public/src/core/FactoryImpl.cpp)
endif()

set(
  HEADER_LIB

  ${AMF_HOME}/public/common/AMFFactory.h
  ${AMF_HOME}/public/common/AMFSTL.h
  ${AMF_HOME}/public/common/ByteArray.h
  ${AMF_HOME}/public/common/DataStream.h
  ${AMF_HOME}/public/common/DataStreamFile.h
  ${AMF_HOME}/public/common/DataStreamMemory.h
  ${AMF_HOME}/public/common/InterfaceImpl.h
  ${AMF_HOME}/public/common/IOCapsImpl.h
  ${AMF_HOME}/public/common/ObservableImpl.h
  ${AMF_HOME}/public/common/PropertyStorageExImpl.h
  ${AMF_HOME}/public/common/PropertyStorageImpl.h
  ${AMF_HOME}/public/common/Thread.h
  ${AMF_HOME}/public/common/TraceAdapter.h

  ${AMF_HOME}/public/include/core/AudioBuffer.h
  ${AMF_HOME}/public/include/core/Buffer.h
  ${AMF_HOME}/public/include/core/Compute.h
  ${AMF_HOME}/public/include/core/ComputeFactory.h
  ${AMF_HOME}/public/include/core/Context.h
  ${AMF_HOME}/public/include/core/Data.h
  ${AMF_HOME}/public/include/core/Debug.h
  ${AMF_HOME}/public/include/core/Factory.h
  ${AMF_HOME}/public/include/core/Interface.h
  ${AMF_HOME}/public/include/core/Plane.h
  ${AMF_HOME}/public/include/core/Platform.h
  ${AMF_HOME}/public/include/core/PropertyStorage.h
  ${AMF_HOME}/public/include/core/PropertyStorageEx.h
  ${AMF_HOME}/public/include/core/Result.h
  ${AMF_HOME}/public/include/core/Surface.h
  ${AMF_HOME}/public/include/core/Trace.h
  ${AMF_HOME}/public/include/core/Variant.h
  ${AMF_HOME}/public/include/core/Version.h
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