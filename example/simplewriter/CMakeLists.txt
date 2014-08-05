project(simplewriter CXX)

add_executable(simplewriter simplewriter.cpp)

find_package(RapidJSON)

target_include_directories(simplewriter PRIVATE ${RapidJSON_INCLUDE_DIRS})

target_compile_definitions(simplewriter PRIVATE ${RapidJSON_DEFINITIONS})

install(TARGETS simplewriter DESTINATION bin)
