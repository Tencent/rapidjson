project(capitalize CXX)

add_executable(capitalize capitalize.cpp)

set_target_properties(capitalize PROPERTIES FOLDER "example")

find_package(RapidJSON)

target_include_directories(capitalize PRIVATE ${RapidJSON_INCLUDE_DIRS})

target_compile_definitions(capitalize PRIVATE ${RapidJSON_DEFINITIONS})

install(TARGETS capitalize DESTINATION bin)
