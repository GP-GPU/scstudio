# Process this file with cmake to produce a Makefile.
# cmake .
#cmake . -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr/local

# Set path to ANTLR using -DANTLR_ROOT=/home/user/antlr-3.1.1

PROJECT(scstudio C CXX)
#SET(CMAKE_VERBOSE_MAKEFILE ON)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(MEMORYCHECK_COMMAND_OPTIONS "--quiet --trace-children=yes --leak-check=full")
SET(MEMORYCHECK_SUPPRESSIONS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/valgrind-python.supp"
  CACHE FILEPATH "File that contains suppressions for the memory checker")
INCLUDE(Dart)

OPTION(ENABLE_COVERAGE "Enable coverage testing (debug only)" OFF)
IF(ENABLE_COVERAGE)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
ENDIF()

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_INSTALL_PREFIX}/share/scripts)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})

INCLUDE(TestBigEndian)
TEST_BIG_ENDIAN(BIG_ENDIAN)
IF(BIG_ENDIAN) 
   ADD_DEFINITIONS(-DHOST_IS_BIG_ENDIAN)
ENDIF(BIG_ENDIAN)

# Enforce strict ANSI C/C++ compliancy checking
IF(CMAKE_COMPILER_IS_GNUCXX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -ansi -Wall -Wno-long-long")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)
# Disable microsoft security warnings
IF(MSVC)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS")
ENDIF(MSVC)

FIND_PACKAGE(Threads)

IF(WIN32)
# use automatic linking on Win32 platforms
SET(Boost_USE_STATIC_LIBS ON)
ELSE(WIN32)
# use dynamic linking
# static boost may not have -fPIC required for linking to shared modules
SET(Boost_USE_STATIC_LIBS OFF)
ENDIF(WIN32)
SET(Boost_USE_MULTITHREADED ON)
FIND_PACKAGE(Boost COMPONENTS date_time thread REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

FIND_PACKAGE(Java)
# keep antlr an optional prerequisite
FIND_PACKAGE(ANTLR)

INCLUDE_DIRECTORIES(src)
INCLUDE_DIRECTORIES(${CMAKE_INSTALL_PREFIX}/include)

IF (WIN32)
  SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
  SET(LIBRARY_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH})
ENDIF(WIN32)

ADD_SUBDIRECTORY(doc)
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(tests)


# $Id: CMakeLists.txt 624 2010-02-25 18:17:11Z gotthardp $

