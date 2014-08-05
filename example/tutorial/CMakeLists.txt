project(tutorial CXX)

add_executable(tutorial tutorial.cpp)

find_package(RapidJSON)

target_include_directories(tutorial PRIVATE ${RapidJSON_INCLUDE_DIRS})

include(${CMAKE_SOURCE_DIR}/share/rapidjson/cmake/RapidJSONConfig.cmake)

target_compile_definitions(tutorial PRIVATE ${RapidJSON_DEFINITIONS})

install(TARGETS tutorial DESTINATION bin)
