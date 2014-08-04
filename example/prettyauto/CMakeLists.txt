project(prettyauto CXX)

add_executable(prettyauto prettyauto.cpp)

find_package(RapidJSON)

target_include_directories(prettyauto PUBLIC ${RapidJSON_INCLUDE_DIRS})

target_compile_definitions(prettyauto PUBLIC ${RapidJSON_DEFINITIONS})
