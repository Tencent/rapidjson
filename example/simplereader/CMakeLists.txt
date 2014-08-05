project(simplereader CXX)

add_executable(simplereader simplereader.cpp)

find_package(RapidJSON)

target_include_directories(simplereader PRIVATE ${RapidJSON_INCLUDE_DIRS})

target_compile_definitions(simplereader PRIVATE ${RapidJSON_DEFINITIONS})

install(TARGETS simplereader DESTINATION bin)
