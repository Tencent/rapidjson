project(prettyauto CXX)

add_executable(prettyauto prettyauto.cpp)

set_target_properties(prettyauto PROPERTIES FOLDER "example")

find_package(RapidJSON)

target_include_directories(prettyauto PRIVATE ${RapidJSON_INCLUDE_DIRS})

target_compile_definitions(prettyauto PRIVATE ${RapidJSON_DEFINITIONS})

install(TARGETS prettyauto DESTINATION bin)
