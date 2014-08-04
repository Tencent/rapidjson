project(serialize CXX)

add_executable(serialize serialize.cpp)

find_package(RapidJSON)

target_include_directories(serialize PUBLIC ${RapidJSON_INCLUDE_DIRS})

target_compile_definitions(serialize PUBLIC ${RapidJSON_DEFINITIONS})
