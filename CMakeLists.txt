cmake_minimum_required(VERSION 3.9)
project(atomspace_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

IF (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -std=libc++17")
ENDIF()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_MACOSX_RPATH 1)
endif()
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(BOOST_USE_STATIC_RUNTIME OFF)

find_package(Boost 1.46 COMPONENTS system filesystem program_options)

IF(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
ELSE(Boost_FOUND)
    MESSAGE(FATAL_ERROR "Boost libraries not found. Please install them")
ENDIF(Boost_FOUND)

#cogutil
FIND_PACKAGE(CogUtil 2.0.1 CONFIG REQUIRED)
IF (COGUTIL_FOUND)
    MESSAGE(STATUS "CogUtil version ${COGUTIL_VERSION} found.")
    ADD_DEFINITIONS(-DHAVE_COGUTIL)
    SET(HAVE_COGUTIL 1)
ELSE (COGUTIL_FOUND)
    MESSAGE(FATAL_ERROR "CogUtil missing: it is needed!")
ENDIF (COGUTIL_FOUND)

#atomspace
FIND_PACKAGE(AtomSpace 5.0.3 CONFIG REQUIRED)
IF(ATOMSPACE_FOUND)
    MESSAGE(STATUS "AtomSpace found.")
    ADD_DEFINITIONS(-DHAVE_ATOMSPACE)
    SET(HAVE_ATOMSPACE 1)
ELSE (ATOMSPACE_FOUND)
    MESSAGE(FATAL_ERROR "AtomSpace missing: it is needed!")
ENDIF (ATOMSPACE_FOUND)

# add the 'cmake' directory from cogutil to search path
list(APPEND CMAKE_MODULE_PATH  ${COGUTIL_DATA_DIR}/cmake)

# Only list install files that have actually changed.
SET(CMAKE_INSTALL_MESSAGE "LAZY")

ADD_SUBDIRECTORY(src)

# Needed for unit tests.

FIND_PACKAGE(Cxxtest)
IF (CXXTEST_FOUND)
    MESSAGE(STATUS "CxxTest found.")
    ADD_CUSTOM_TARGET(tests)
    ADD_SUBDIRECTORY(tests EXCLUDE_FROM_ALL)
ELSE (CXXTEST_FOUND)
    MESSAGE(STATUS "CxxTest missing: needed for unit tests.")
ENDIF (CXXTEST_FOUND)
