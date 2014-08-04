project(capitalize CXX)

add_executable(capitalize capitalize.cpp)

find_package(RapidJSON)

target_include_directories(capitalize PUBLIC ${RapidJSON_INCLUDE_DIRS})

target_compile_definitions(capitalize PUBLIC ${RapidJSON_DEFINITIONS})
