cmake_minimum_required(VERSION 3.5)
project(RapidJSONRepeatedFindPackage LANGUAGES CXX)

# Configure this consumer with -DRapidJSON_DIR=<installed package config directory>.
# The package must not replace CMake's discovery variable with its build directory.
set(expected_package_dir "${RapidJSON_DIR}")
find_package(RapidJSON CONFIG REQUIRED)
if(NOT RapidJSON_DIR STREQUAL expected_package_dir)
    message(FATAL_ERROR "RapidJSONConfig.cmake changed RapidJSON_DIR")
endif()

set(expected_package_config "${RapidJSON_CONFIG}")
find_package(RapidJSON CONFIG REQUIRED)
if(NOT RapidJSON_CONFIG STREQUAL expected_package_config)
    message(FATAL_ERROR "Repeated discovery selected a different package configuration")
endif()

add_executable(repeated_find_package main.cpp)
target_link_libraries(repeated_find_package PRIVATE RapidJSON)
