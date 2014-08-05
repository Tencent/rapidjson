project(simplewriter CXX)

add_executable(simplewriter simplewriter.cpp)

set_target_properties(simplewriter PROPERTIES FOLDER "example")

find_package(RapidJSON)

target_include_directories(simplewriter PRIVATE ${RapidJSON_INCLUDE_DIRS})

target_compile_definitions(simplewriter PRIVATE ${RapidJSON_DEFINITIONS})

warningsetting(WARNING_SETTING)
target_compile_options(simplewriter PRIVATE ${WARNING_SETTING})

install(TARGETS simplewriter DESTINATION bin)
