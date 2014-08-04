project(condense CXX)

add_executable(condense condense.cpp)

find_package(RapidJSON)

target_include_directories(condense PUBLIC ${RapidJSON_INCLUDE_DIRS})

target_compile_definitions(condense PUBLIC ${RapidJSON_DEFINITIONS})
