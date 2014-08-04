project(simplereader CXX)

add_executable(simplereader simplereader.cpp)

find_package(RapidJSON)

target_include_directories(simplereader PUBLIC ${RapidJSON_INCLUDE_DIRS})

target_compile_definitions(simplereader PUBLIC ${RapidJSON_DEFINITIONS})
