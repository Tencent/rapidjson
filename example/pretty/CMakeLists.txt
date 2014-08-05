project(pretty CXX)

add_executable(pretty pretty.cpp)

find_package(RapidJSON)

target_include_directories(pretty PRIVATE ${RapidJSON_INCLUDE_DIRS})

target_compile_definitions(pretty PRIVATE ${RapidJSON_DEFINITIONS})

install(TARGETS pretty DESTINATION bin)
