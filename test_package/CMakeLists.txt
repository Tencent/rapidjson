cmake_minimum_required(VERSION 3.2)

project(RapidJSONPackageTestProject)

# Configure environment
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Find external dependencies
find_package(rapidjson)

# Configure compilation options
add_compile_options(-DRAPIDJSON_HAS_STDSTRING=1)

# Configure test package project
set(RAPIDJSON_PACKAGE_TEST_PROJECT RapidJSONPackageTest)
add_executable(${RAPIDJSON_PACKAGE_TEST_PROJECT} rapidjsonTest.cpp)
target_link_libraries(${RAPIDJSON_PACKAGE_TEST_PROJECT} rapidjson::rapidjson)

# Register tests
enable_testing()
add_test(NAME RapidJSONPackageTest COMMAND ${RAPIDJSON_PACKAGE_TEST_PROJECT})
