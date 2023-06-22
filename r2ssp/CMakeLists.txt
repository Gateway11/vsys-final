cmake_minimum_required(VERSION 3.0)
project(r2ssp C CXX ASM)
set(VERSION 0.1)

# Configuration fails on < 3.9 if only C++ is enabled (TestBigEndian macro)
if(NOT CMAKE_VERSION VERSION_LESS 3.9.0)
    set(LANG CXX)
endif()

# CMake policies: enable MACOSX_RPATH by default
if(POLICY CMP0042)
    cmake_policy(SET CMP0042 NEW)
endif()

# Don't treat imported targets with :: as files
if(POLICY CMP0028)
    cmake_policy(SET CMP0028 NEW)
endif()

#############################################################################
# Turning on this flag tells cmake to emit a compile_commands.json file.
# This file can be used to load compilation flags into YCM. See here for more
# details: http://clang.llvm.org/docs/JSONCompilationDatabase.html
set( CMAKE_EXPORT_COMPILE_COMMANDS 1 )

#############################################################################

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" )
  set(COMPILER_IS_CLANG true)

  # The Travis CI build machines don't have libc++ installed
  if (NOT DEFINED ENV{TRAVIS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++" )
  endif()

  # Ninja will by default prevent Clang from outputting diagnostics in color
  if (CMAKE_GENERATOR STREQUAL "Ninja")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics" )
  endif()
endif()

#option(BUILD_DEPRECATED "Include deprecated API in the build" ON)

# Use folders for nice tree in Visual Studio and XCode
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(BUILD_STATIC "Build static libraries (default are shared)" OFF)

include(CMakeDependentOption)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(CheckSymbolExists)
include(CMakePushCheckState)
include(GNUInstallDirs)

if(NOT CMAKE_INSTALL_RPATH AND CMAKE_INSTALL_PREFIX)
	set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_FULL_LIBDIR})
	message(STATUS "Setting install rpath to ${CMAKE_INSTALL_RPATH}")
endif()

enable_language(ASM)
set(COMMON_CFLAGS "-std=c++11 -fPIC -fpermissive")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_CFLAGS}")
SET(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} ${CFLAGS} -x assembler-with-cpp")

find_package(Blis REQUIRED)
#find_package(SpeexDsp REQUIRED)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/fe)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/speexdsp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/speexdsp/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/openmax_dl)
include_directories(${BLIS_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/webrtc/base)
#include_directories(${SPEEXDSP_INCLUDE_DIRS})
#include_directories(${WEBRTC_AUDIO_PROCESSING_INCLUDE_DIRS})

set(LOCAL_SRC_FILES)

include (TargetArch)
target_architecture(DETECTED_ARCH)

if (DETECTED_ARCH MATCHES "armv8")
        include(${CMAKE_CURRENT_SOURCE_DIR}/src_arm64.cmake)
elseif (DETECTED_ARCH MATCHES "arm.*")
        include(${CMAKE_CURRENT_SOURCE_DIR}/src_arm.cmake)
endif()

add_library(r2ssp SHARED ${LOCAL_SRC_FILES})
target_link_libraries(r2ssp ${BLIS_LIBRARIES} fftw3f)

