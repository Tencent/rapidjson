project(condense CXX)

add_executable(condense condense.cpp)

find_package(RapidJSON)

target_include_directories(condense PRIVATE ${RapidJSON_INCLUDE_DIRS})

target_compile_definitions(condense PRIVATE ${RapidJSON_DEFINITIONS})

install(TARGETS condense DESTINATION bin)
