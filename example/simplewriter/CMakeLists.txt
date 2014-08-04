project(simplewriter CXX)

add_executable(simplewriter simplewriter.cpp)

find_package(RapidJSON)

target_include_directories(simplewriter PUBLIC ${RapidJSON_INCLUDE_DIRS})

target_compile_definitions(simplewriter PUBLIC ${RapidJSON_DEFINITIONS})
