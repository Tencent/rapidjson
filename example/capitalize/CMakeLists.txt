project(capitalize CXX)

add_executable(capitalize capitalize.cpp)

find_package(RapidJSON)

target_include_directories(capitalize PRIVATE ${RapidJSON_INCLUDE_DIRS})

target_compile_definitions(capitalize PRIVATE ${RapidJSON_DEFINITIONS})

install(TARGETS capitalize DESTINATION bin)
