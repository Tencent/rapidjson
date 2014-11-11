set(PERFTEST_SOURCES
    misctest.cpp
    perftest.cpp
    platformtest.cpp
    rapidjsontest.cpp)

add_executable(perftest ${PERFTEST_SOURCES})
target_link_libraries(perftest ${TEST_LIBRARIES})
add_test(NAME perftest
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/perftest
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
