project(simpledom CXX)

add_executable(simpledom simpledom.cpp)

find_package(RapidJSON)

target_include_directories(simpledom PUBLIC ${RapidJSON_INCLUDE_DIRS})

target_compile_definitions(simpledom PUBLIC ${RapidJSON_DEFINITIONS})
