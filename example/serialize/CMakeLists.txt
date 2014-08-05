project(serialize CXX)

add_executable(serialize serialize.cpp)

set_target_properties(serialize PROPERTIES FOLDER "example")

find_package(RapidJSON)

target_include_directories(serialize PRIVATE ${RapidJSON_INCLUDE_DIRS})

target_compile_definitions(serialize PRIVATE ${RapidJSON_DEFINITIONS})

install(TARGETS serialize DESTINATION bin)
