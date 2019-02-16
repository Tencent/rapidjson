cmake_minimum_required(VERSION 2.8)
if(POLICY CMP0025)
  # detect Apple's Clang
  cmake_policy(SET CMP0025 NEW)
endif()
if(POLICY CMP0054)
  cmake_policy(SET CMP0054 NEW)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules)

project(RapidJSON CXX)

set(LIB_MAJOR_VERSION "1")
set(LIB_MINOR_VERSION "1")
set(LIB_PATCH_VERSION "0")
set(LIB_VERSION_STRING "${LIB_MAJOR_VERSION}.${LIB_MINOR_VERSION}.${LIB_PATCH_VERSION}")

# compile in release with debug info mode by default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

# Build all binaries in a separate directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(RAPIDJSON_BUILD_DOC "Build rapidjson documentation." ON)
option(RAPIDJSON_BUILD_EXAMPLES "Build rapidjson examples." ON)
option(RAPIDJSON_BUILD_TESTS "Build rapidjson perftests and unittests." ON)
option(RAPIDJSON_BUILD_THIRDPARTY_GTEST
    "Use gtest installation in `thirdparty/gtest` by default if available" OFF)

option(RAPIDJSON_BUILD_CXX11 "Build rapidjson with C++11 (gcc/clang)" ON)
if(RAPIDJSON_BUILD_CXX11)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
endif()

option(RAPIDJSON_BUILD_ASAN "Build rapidjson with address sanitizer (gcc/clang)" OFF)
option(RAPIDJSON_BUILD_UBSAN "Build rapidjson with undefined behavior sanitizer (gcc/clang)" OFF)

option(RAPIDJSON_ENABLE_INSTRUMENTATION_OPT "Build rapidjson with -march or -mcpu options" ON)

option(RAPIDJSON_HAS_STDSTRING "" OFF)
if(RAPIDJSON_HAS_STDSTRING)
    add_definitions(-DRAPIDJSON_HAS_STDSTRING)
endif()

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Qunused-arguments -fcolor-diagnostics")
    endif()
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(RAPIDJSON_ENABLE_INSTRUMENTATION_OPT AND NOT CMAKE_CROSSCOMPILING)
        if(CMAKE_SYSTEM_PROCESSOR STREQUAL "powerpc" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "ppc64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "ppc64le")
          set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcpu=native")
        else()
          #FIXME: x86 is -march=native, but doesn't mean every arch is this option. To keep original project's compatibility, I leave this except POWER.
          set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
        endif()
    endif()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
    set(EXTRA_CXX_FLAGS -Weffc++ -Wswitch-default -Wfloat-equal -Wconversion -Wsign-conversion)
    if(RAPIDJSON_BUILD_CXX11 AND CMAKE_VERSION VERSION_LESS 3.1)
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.7.0")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
        else()
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
        endif()
    endif()
    if(RAPIDJSON_BUILD_ASAN)
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.8.0")
            message(FATAL_ERROR "GCC < 4.8 doesn't support the address sanitizer")
        else()
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
        endif()
    endif()
    if(RAPIDJSON_BUILD_UBSAN)
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.9.0")
            message(FATAL_ERROR "GCC < 4.9 doesn't support the undefined behavior sanitizer")
        else()
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
        endif()
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(NOT CMAKE_CROSSCOMPILING)
        if(CMAKE_SYSTEM_PROCESSOR STREQUAL "powerpc" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "ppc64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "ppc64le")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcpu=native")
        else()
            #FIXME: x86 is -march=native, but doesn't mean every arch is this option. To keep original project's compatibility, I leave this except POWER.
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
        endif()
    endif()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wno-missing-field-initializers")
    set(EXTRA_CXX_FLAGS -Weffc++ -Wswitch-default -Wfloat-equal -Wconversion -Wimplicit-fallthrough)
    if(RAPIDJSON_BUILD_CXX11 AND CMAKE_VERSION VERSION_LESS 3.1)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    endif()
    if(RAPIDJSON_BUILD_ASAN)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    endif()
    if(RAPIDJSON_BUILD_UBSAN)
        if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined-trap -fsanitize-undefined-trap-on-error")
        else()
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
        endif()
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS=1)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "XL")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -qarch=auto")
endif()

#add extra search paths for libraries and includes
set(INCLUDE_INSTALL_DIR "include" CACHE PATH "The directory the headers are installed in")
set(LIB_INSTALL_DIR "lib" CACHE STRING "Directory where lib will install")
set(DOC_INSTALL_DIR "share/doc/${PROJECT_NAME}" CACHE PATH "Path to the documentation")

if(UNIX OR CYGWIN)
    set(_CMAKE_INSTALL_DIR "${LIB_INSTALL_DIR}/cmake/${PROJECT_NAME}")
elseif(WIN32)
    set(_CMAKE_INSTALL_DIR "cmake")
endif()
set(CMAKE_INSTALL_DIR "${_CMAKE_INSTALL_DIR}" CACHE PATH "The directory cmake files are installed in")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

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
    endif()
    add_subdirectory(test)
    include(CTest)
endif()

# pkg-config
if(UNIX OR CYGWIN)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.pc.in
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc
        DESTINATION "${LIB_INSTALL_DIR}/pkgconfig"
        COMPONENT pkgconfig)
endif()

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

################################################################################
# Export package for use from the build tree
export(PACKAGE ${PROJECT_NAME})

# Create the RapidJSONConfig.cmake file for other cmake projects.
include(CMakePackageConfigHelpers)

# ... for the build tree
set(CONFIG_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CONFIG_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(${PROJECT_NAME}_INCLUDE_DIR ${CONFIG_SOURCE_DIR}/include)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
    PATH_VARS ${PROJECT_NAME}_INCLUDE_DIR
    NO_CHECK_REQUIRED_COMPONENTS_MACRO)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}ConfigVersion.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake @ONLY)

# ... for the install tree
set(${PROJECT_NAME}_INCLUDE_DIR ${INCLUDE_INSTALL_DIR})
set(CONFIG_SOURCE_DIR)
set(CONFIG_DIR)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_DIR}
    PATH_VARS ${PROJECT_NAME}_INCLUDE_DIR
    NO_CHECK_REQUIRED_COMPONENTS_MACRO)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    DESTINATION "${CMAKE_INSTALL_DIR}"
    COMPONENT dev)
