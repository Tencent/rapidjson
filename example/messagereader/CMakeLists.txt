project(messagereader CXX)

add_executable(messagereader messagereader.cpp)

set_target_properties(messagereader PROPERTIES FOLDER "example")

find_package(RapidJSON)

target_include_directories(messagereader PRIVATE ${RapidJSON_INCLUDE_DIRS})

target_compile_definitions(messagereader PRIVATE ${RapidJSON_DEFINITIONS})

warningsetting(WARNING_SETTING)
target_compile_options(messagereader PRIVATE ${WARNING_SETTING})

install(TARGETS messagereader DESTINATION bin)
