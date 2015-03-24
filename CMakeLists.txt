CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMakeModules)

PROJECT(RapidJSON CXX)

set(LIB_MAJOR_VERSION "0")
set(LIB_MINOR_VERSION "12")
set(LIB_PATCH_VERSION "0")
set(LIB_VERSION_STRING "${LIB_MAJOR_VERSION}.${LIB_MINOR_VERSION}.${LIB_PATCH_VERSION}")

# compile in release with debug info mode by default
SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build Type")

# Build all binaries in a separate directory
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(RAPIDJSON_BUILD_DOC "Build rapidjson documentation." ON)
option(RAPIDJSON_BUILD_EXAMPLES "Build rapidjson examples." ON)
option(RAPIDJSON_BUILD_TESTS "Build rapidjson perftests and unittests." ON)

option(RAPIDJSON_HAS_STDSTRING "" OFF)
if(RAPIDJSON_HAS_STDSTRING)
    add_definitions(-DRAPIDJSON_HAS_STDSTRING)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -Wall -Wextra")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -Wall -Wextra")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS=1)
endif()

#add extra search paths for libraries and includes
SET(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH "The directory the headers are installed in")
SET(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE STRING "Directory where lib will install")
SET(DOC_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/doc/${PROJECT_NAME}" CACHE PATH "Path to the documentation")

IF(UNIX OR CYGWIN)
    SET(_CMAKE_INSTALL_DIR "${LIB_INSTALL_DIR}/cmake/${PROJECT_NAME}")
ELSEIF(WIN32)
    SET(_CMAKE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/cmake")
ENDIF()
SET(CMAKE_INSTALL_DIR "${_CMAKE_INSTALL_DIR}" CACHE PATH "The directory cmake fiels are installed in")

include_directories(${CMAKE_SOURCE_DIR}/include)

if(RAPIDJSON_BUILD_DOC)
    add_subdirectory(doc)
endif()

add_custom_target(travis_doc)
add_custom_command(TARGET travis_doc
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/travis-doxygen.sh)

if(RAPIDJSON_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()

if(RAPIDJSON_BUILD_TESTS)
    if(MSVC11)
        # required for VS2012 due to missing support for variadic templates
        add_definitions(-D_VARIADIC_MAX=10)
    endif(MSVC11)
    add_subdirectory(test)
    include(CTest)
endif()

# pkg-config
IF (UNIX OR CYGWIN)
  CONFIGURE_FILE (${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.pc.in
                  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc
                  @ONLY)
  INSTALL (FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc
      DESTINATION "${LIB_INSTALL_DIR}/pkgconfig"
      COMPONENT pkgconfig)
ENDIF()

install(FILES readme.md
        DESTINATION "${DOC_INSTALL_DIR}"
        COMPONENT doc)

install(DIRECTORY include/rapidjson
    DESTINATION "${INCLUDE_INSTALL_DIR}"
    COMPONENT dev)

install(DIRECTORY example/
    DESTINATION "${DOC_INSTALL_DIR}/examples"
    COMPONENT examples
    # Following patterns are for excluding the intermediate/object files
    # from an install of in-source CMake build.
    PATTERN "CMakeFiles" EXCLUDE
    PATTERN "Makefile" EXCLUDE
    PATTERN "cmake_install.cmake" EXCLUDE)

# Provide config and version files to be used by other applications
# ===============================

export(PACKAGE ${PROJECT_NAME})

# cmake-modules
CONFIGURE_FILE(${PROJECT_NAME}Config.cmake.in
    ${PROJECT_NAME}Config.cmake
    @ONLY)
CONFIGURE_FILE(${PROJECT_NAME}ConfigVersion.cmake.in
    ${PROJECT_NAME}ConfigVersion.cmake
    @ONLY)
INSTALL(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    DESTINATION "${CMAKE_INSTALL_DIR}"
    COMPONENT dev)
