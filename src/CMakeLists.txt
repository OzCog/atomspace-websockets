set(BINARY ${CMAKE_PROJECT_NAME})

file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp)

set(SOURCES ${SOURCES})

add_library(${BINARY}_lib STATIC ${SOURCES})

LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

#zlib
FIND_PACKAGE(ZLIB REQUIRED)

FIND_PACKAGE(nlohmann_json 3.2.0 REQUIRED)

FIND_LIBRARY(BIOSCIENCE_LIB
        NAME libbioscience-types.dylib libbioscience-types.so
        PATHS ${CMAKE_INSTALL_PREFIX}/lib/opencog
        NO_DEFAULT_PATH)
#uSocket
FIND_LIBRARY(uSOCKET_LIB
        NAME uSockets.a
        HINTS /usr/local/lib NO_DEFAULT_PATH)

# Set default include paths.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
        ${COGUTIL_INCLUDE_DIR}
        ${ATOMSPACE_INCLUDE_DIR}
        ${ZLIB_INCLUDE_DIRS})


TARGET_LINK_LIBRARIES(${BINARY}_lib
        ${COGUTIL_LIBRARIES}
        ${ATOMSPACE_LIBRARIES}
        ${Boost_LIBRARIES}
        ${ZLIB_LIBRARIES}
        nlohmann_json::nlohmann_json
        ${uSOCKET_LIB})

IF(BIOSCIENCE_LIB)
    MESSAGE(STATUS "AGI-Bio found.")
    TARGET_LINK_LIBRARIES(${BINARY}_lib ${BIOSCIENCE_LIB})
ENDIF()

ADD_EXECUTABLE(atom_server main.cpp)
TARGET_LINK_LIBRARIES(atom_server ${BINARY}_lib)
ADD_DEPENDENCIES(atom_server ${BINARY}_lib)

INSTALL(TARGETS atom_server DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
