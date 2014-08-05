project(simpledom CXX)

add_executable(simpledom simpledom.cpp)

find_package(RapidJSON)

target_include_directories(simpledom PRIVATE ${RapidJSON_INCLUDE_DIRS})

target_compile_definitions(simpledom PRIVATE ${RapidJSON_DEFINITIONS})

install(TARGETS simpledom DESTINATION bin)
