set(BINARY ${CMAKE_PROJECT_NAME})

file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp)

set(SOURCES ${SOURCES})

add_executable(atom_server ${SOURCES})

add_library(${BINARY}_lib STATIC ${SOURCES})

LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

#zlib
FIND_PACKAGE(ZLIB REQUIRED)

#libuv
FIND_PACKAGE(libuv REQUIRED)

FIND_LIBRARY(BIOSCIENCE_LIB NAME libbioscience-types.dylib PATHS /usr/local/lib/opencog
        NO_DEFAULT_PATH)

FIND_LIBRARY(LOAD_SCM NAME libload_scm.dylib PATHS /usr/local/lib/opencog NO_DEFAULT_PATH)


#uSocket
FIND_LIBRARY(uSOCKET_LIB uSockets.a PATHS /usr/local/lib NO_DEFAULT_PATH)


FIND_LIBRARY(BIOSCIENCE_LIB NAME libbioscience-types.dylib PATHS /usr/local/lib/opencog
        NO_DEFAULT_PATH)

FIND_LIBRARY(LOAD_SCM NAME libload_scm.dylib PATHS /usr/local/lib/opencog NO_DEFAULT_PATH)

FIND_PACKAGE(nlohmann_json 3.2.0 REQUIRED)

# Set default include paths.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
        ${COGUTIL_INCLUDE_DIR}
        ${ATOMSPACE_INCLUDE_DIR}
        ${ZLIB_INCLUDE_DIRS}
        ${LIBUV_LIBRARIES}
        /usr/local/include/uWebSockets)

TARGET_LINK_LIBRARIES(${BINARY}_lib
        ${COGUTIL_LIBRARIES}
        ${ATOMSPACE_atomtypes_LIBRARY}
        ${ATOMSPACE_atombase_LIBRARY}
        ${ATOMSPACE_atomcore_LIBRARY}
        ${ATOMSPACE_atomspace_LIBRARY}
        ${ATOMSPACE_value_LIBRARY}
        ${ATOMSPACE_truthvalue_LIBRARY}
        ${Boost_LIBRARIES}
        ${LOAD_SCM}
        ${BIOSCIENCE_LIB}
        ${ZLIB_LIBRARIES}
        ${LIBUV}
        ${uSOCKET_LIB}
        nlohmann_json::nlohmann_json)

TARGET_LINK_LIBRARIES(atom_server
        ${COGUTIL_LIBRARIES}
        ${ATOMSPACE_atomtypes_LIBRARY}
        ${ATOMSPACE_atombase_LIBRARY}
        ${ATOMSPACE_atomcore_LIBRARY}
        ${ATOMSPACE_atomspace_LIBRARY}
        ${ATOMSPACE_value_LIBRARY}
        ${ATOMSPACE_truthvalue_LIBRARY}
        ${Boost_LIBRARIES}
        ${LOAD_SCM}
        ${BIOSCIENCE_LIB}
        ${ZLIB_LIBRARIES}
        ${LIBUV}
        ${uSOCKET_LIB}
        nlohmann_json::nlohmann_json)