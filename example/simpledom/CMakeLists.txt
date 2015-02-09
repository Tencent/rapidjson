project(simpledom CXX)

add_executable(simpledom simpledom.cpp)

set_target_properties(simpledom PROPERTIES FOLDER "example")

find_package(RapidJSON)

target_include_directories(simpledom PRIVATE ${RapidJSON_INCLUDE_DIRS})

target_compile_definitions(simpledom PRIVATE ${RapidJSON_DEFINITIONS})

warningsetting(WARNING_SETTING)
target_compile_options(simpledom PRIVATE ${WARNING_SETTING})

install(TARGETS simpledom DESTINATION bin)
