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

FIND_PACKAGE(nlohmann_json 3.2.0 REQUIRED)

FIND_LIBRARY(BIOSCIENCE_LIB
        NAME libbioscience-types.dylib libbioscience-types.so
        PATHS ${CMAKE_INSTALL_PREFIX}/lib/opencog
        NO_DEFAULT_PATH)
FIND_LIBRARY(PERSIST_FILE
        NAME libload_scm.dylib libload_scm.so
        PATHS ${CMAKE_INSTALL_PREFIX}/lib/opencog
        NO_DEFAULT_PATH)

#uSocket
FIND_LIBRARY(uSOCKET_LIB
        NAME uSockets.a
        PATHS ${CMAKE_INSTALL_PREFIX}/lib NO_DEFAULT_PATH)


# Set default include paths.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
        ${COGUTIL_INCLUDE_DIR}
        ${ATOMSPACE_INCLUDE_DIR}
        ${ZLIB_INCLUDE_DIRS}
        ${LIBUV_INCLUDE_DIRS})


TARGET_LINK_LIBRARIES(${BINARY}_lib
        ${COGUTIL_LIBRARIES}
        ${ATOMSPACE_atomtypes_LIBRARY}
        ${ATOMSPACE_atombase_LIBRARY}
        ${ATOMSPACE_atomcore_LIBRARY}
        ${ATOMSPACE_atomspace_LIBRARY}
        ${ATOMSPACE_value_LIBRARY}
        ${ATOMSPACE_truthvalue_LIBRARY}
        ${ATOMSPACE_persist_file_LIBRARY}
        ${Boost_LIBRARIES}
        ${PERSIST_FILE}
        ${ZLIB_LIBRARIES}
        ${LIBUV_LIBRARIES}
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
        ${ATOMSPACE_persist_file_LIBRARY}
        ${Boost_LIBRARIES}
        ${PERSIST_FILE}
        ${ZLIB_LIBRARIES}
        ${LIBUV_LIBRARIES}
        ${uSOCKET_LIB}
        nlohmann_json::nlohmann_json)

IF(BIOSCIENCE_LIB)
    MESSAGE(STATUS "AGI-Bio found.")
    TARGET_LINK_LIBRARIES(${BINARY}_lib ${BIOSCIENCE_LIB})
    TARGET_LINK_LIBRARIES(atom_server ${BIOSCIENCE_LIB})
ENDIF()

INSTALL(TARGETS atom_server DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)