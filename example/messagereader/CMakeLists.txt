project(messagereader CXX)

add_executable(messagereader messagereader.cpp)

find_package(RapidJSON)

target_include_directories(messagereader PRIVATE ${RapidJSON_INCLUDE_DIRS})

target_compile_definitions(messagereader PRIVATE ${RapidJSON_DEFINITIONS})

install(TARGETS messagereader DESTINATION bin)
