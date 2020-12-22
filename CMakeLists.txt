cmake_minimum_required(VERSION 3.5...3.19)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules)

message(WARNING "CMAKE_VERSION = ${CMAKE_VERSION}")
include(before_project_setup)   # setup ccache if available ...

set(PROJECT_VERSION_MAJOR "1")
set(PROJECT_VERSION_MINOR "1")
set(PROJECT_VERSION_PATCH "0")
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

if(CMAKE_VERSION VERSION_LESS 3.12)
    PROJECT(RapidJSON CXX)
else()
    PROJECT(RapidJSON VERSION ${PROJECT_VERSION} LANGUAGES CXX)
endif()

include(GNUInstallDirs)

#
# Check if this is the master project.
set(MASTER_PROJECT OFF)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(MASTER_PROJECT ON)
endif()

# compile in release with debug info mode by default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

#
# Build all binaries in a separate directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#
# options settings
option(RAPIDJSON_INSTALL "Generate the install target" ${MASTER_PROJECT})
option(RAPIDJSON_BUILD_DOC "Build rapidjson documentation." ${MASTER_PROJECT})
option(RAPIDJSON_BUILD_EXAMPLES "Build rapidjson examples." ${MASTER_PROJECT})
option(RAPIDJSON_BUILD_TESTS "Build rapidjson perftests and unittests." ${MASTER_PROJECT})

option(RAPIDJSON_BUILD_THIRDPARTY_GTEST
    "Use gtest installation in `thirdparty/gtest` by default if available" OFF)

option(RAPIDJSON_BUILD_CXX11 "Build rapidjson with C++11 (gcc/clang)" ON)
if(NOT DEFINED CMAKE_CXX_STANDARD AND RAPIDJSON_BUILD_CXX11)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
endif()

option(RAPIDJSON_BUILD_ASAN "Build rapidjson with address sanitizer (gcc/clang)" ${MASTER_PROJECT})
option(RAPIDJSON_BUILD_UBSAN "Build rapidjson with undefined behavior sanitizer (gcc/clang)" ${MASTER_PROJECT})
option(RAPIDJSON_ENABLE_INSTRUMENTATION_OPT "Build rapidjson with -march or -mcpu options" ${MASTER_PROJECT})
option(RAPIDJSON_HAS_STDSTRING "" OFF)

################################################################################
add_library(rapidjson INTERFACE)
add_library(rapidjson::rapidjson ALIAS rapidjson)
target_include_directories(rapidjson INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
if(RAPIDJSON_BUILD_CXX11)
    if(NOT CMAKE_VERSION VERSION_LESS 3.8)
        target_compile_features(rapidjson INTERFACE cxx_std_11)
    endif()
    target_compile_definitions(rapidjson INTERFACE RAPIDJSON_HAS_STDSTRING)
elseif(RAPIDJSON_HAS_STDSTRING)
    target_compile_definitions(rapidjson INTERFACE RAPIDJSON_HAS_STDSTRING)
endif()
################################################################################

# TODO: use target_compile_options()
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(RAPIDJSON_ENABLE_INSTRUMENTATION_OPT AND NOT CMAKE_CROSSCOMPILING)
        if(CMAKE_SYSTEM_PROCESSOR STREQUAL "powerpc" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "ppc64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "ppc64le")
          set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcpu=native")
        else()
          #FIXME: x86 is -march=native, but doesn't mean every arch is this option. To keep original project's compatibility, I leave this except POWER.
          set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
        endif()
    endif()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    set(EXTRA_CXX_FLAGS -Weffc++ -Wswitch-default -Wfloat-equal -Wconversion -Wsign-conversion)

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

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    set(EXTRA_CXX_FLAGS -Weffc++ -Wswitch-default -Wfloat-equal -Wconversion -Wimplicit-fallthrough)

    # FIXME: do not suppress any warning! CK
    # if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    #     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-missing-field-initializers -Wno-deprecated-copy")
    # endif()

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

#
# generate doc
if(RAPIDJSON_BUILD_DOC)
    add_subdirectory(doc)

    add_custom_target(travis_doc)
    add_custom_command(TARGET travis_doc
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/travis-doxygen.sh)
endif()

#
# build examples
if(RAPIDJSON_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()

#
# build tests
if(RAPIDJSON_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

################################################################################
# Provide config and version files to be used by other applications
################################################################################
if(RAPIDJSON_INSTALL)
    include(CMakePackageConfigHelpers)

    # pkg-config
    if(UNIX OR CYGWIN)
      configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.pc.in
                      ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc
                      @ONLY)
      install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc
          DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
          COMPONENT pkgconfig)
    endif()

    install(FILES readme.md
        DESTINATION "${CMAKE_INSTALL_DOCDIR}"
        COMPONENT doc)

    install(DIRECTORY include/rapidjson
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
        COMPONENT dev)

    install(DIRECTORY example/
        DESTINATION "${CMAKE_INSTALL_DOCDIR}/examples"
        COMPONENT examples
        # TODO: better to disable in-source build! CK
        # Following patterns are for excluding the intermediate/object files
        # from an install of in-source CMake build.
        PATTERN "CMakeLists.txt" EXCLUDE
        PATTERN "CMakeFiles" EXCLUDE
        PATTERN "Makefile" EXCLUDE
        PATTERN "cmake_install.cmake" EXCLUDE)

    # Create the RapidJSONConfig.cmake file for other cmake projects.
    # ... for the build tree
    install(TARGETS rapidjson EXPORT ${PROJECT_NAME}Targets)

    set(configFile ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake)
    set(versionFile ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake)
    set(configInstallDestination ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
        ${configFile}
        INSTALL_DESTINATION ${configInstallDestination}
    )
    write_basic_package_version_file(
        ${versionFile}
        COMPATIBILITY SameMajorVersion
    )

    install(FILES ${configFile} ${versionFile} DESTINATION ${configInstallDestination})
    install(
        EXPORT ${PROJECT_NAME}Targets
        NAMESPACE "rapidjson::"
        DESTINATION ${configInstallDestination}
    )
endif()
