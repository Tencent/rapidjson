project(messagereader CXX)

add_executable(messagereader messagereader.cpp)

find_package(RapidJSON)

target_include_directories(messagereader PUBLIC ${RapidJSON_INCLUDE_DIRS})

target_compile_definitions(messagereader PUBLIC ${RapidJSON_DEFINITIONS})
