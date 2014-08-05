project(simplereader CXX)

add_executable(simplereader simplereader.cpp)

set_target_properties(simplereader PROPERTIES FOLDER "example")

find_package(RapidJSON)

target_include_directories(simplereader PRIVATE ${RapidJSON_INCLUDE_DIRS})

target_compile_definitions(simplereader PRIVATE ${RapidJSON_DEFINITIONS})

warningsetting(WARNING_SETTING)
target_compile_options(simplereader PRIVATE ${WARNING_SETTING})

install(TARGETS simplereader DESTINATION bin)
