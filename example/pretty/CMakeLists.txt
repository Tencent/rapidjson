project(pretty CXX)

add_executable(pretty pretty.cpp)

set_target_properties(pretty PROPERTIES FOLDER "example")

find_package(RapidJSON)

target_include_directories(pretty PRIVATE ${RapidJSON_INCLUDE_DIRS})

target_compile_definitions(pretty PRIVATE ${RapidJSON_DEFINITIONS})

install(TARGETS pretty DESTINATION bin)
