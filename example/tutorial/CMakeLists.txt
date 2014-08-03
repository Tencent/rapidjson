project(tutorial CXX)

add_executable(tutorial tutorial.cpp)

find_package(RapidJSON)

target_include_directories(tutorial PUBLIC ${RapidJSON_INCLUDE_DIRS})
