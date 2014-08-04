project(pretty CXX)

add_executable(pretty pretty.cpp)

find_package(RapidJSON)

target_include_directories(pretty PUBLIC ${RapidJSON_INCLUDE_DIRS})

target_compile_definitions(pretty PUBLIC ${RapidJSON_DEFINITIONS})
