# cmake module for rapidjson

This file is used for adding ```find_package(Rapidjson)``` for cmake.

## Usage

1. Place ```FindRapidjson.cmake``` into your cmake module path or append the base directory of ```FindRapidjson.cmake``` to [CMAKE_MODULE_PATH](https://cmake.org/cmake/help/latest/variable/CMAKE_MODULE_PATH.html)
2. Just use ```find_package(Rapidjson)``` in your project
3. if rapidjson is found, ```Rapidjson_FOUND``` is set to TRUE and ```Rapidjson_INCLUDE_DIRS``` will be the include directory.

You may also set ```RAPIDJSON_ROOT``` or ```Rapidjson_ROOT``` to a rapidjson installation root to tell where to find rapidjson.

## Sample

```cmake
find_package(Rapidjson)
# Try to download latest rapidjson
if(NOT Rapidjson_FOUND)
    set(3RD_PARTY_RAPIDJSON_REPO_DIR "${PROJECT_BINARY_DIR}/rapidjson")
    set(3RD_PARTY_RAPIDJSON_VERSION master)
    if(NOT EXISTS ${3RD_PARTY_RAPIDJSON_REPO_DIR})
        find_package(Git)
        if(NOT GIT_FOUND)
            message(FATAL_ERROR "git not found")
        endif()

        execute_process(COMMAND ${GIT_EXECUTABLE} clone -b ${3RD_PARTY_RAPIDJSON_VERSION} --depth=1 "https://github.com/Tencent/rapidjson.git" ${3RD_PARTY_RAPIDJSON_REPO_DIR}
            WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        )
    endif()

    set(RAPIDJSON_ROOT ${3RD_PARTY_RAPIDJSON_REPO_DIR})
    find_package(Rapidjson)
endif()

if(Rapidjson_FOUND)
    message(STATUS "Dependency: rapidjson found.(${Rapidjson_INCLUDE_DIRS})")
    include_directories(${Rapidjson_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "Dependency: rapidjson not found")
endif()

```
