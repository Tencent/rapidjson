set(UNITTEST_SOURCES
    documenttest.cpp
    encodedstreamtest.cpp
    encodingstest.cpp
    filestreamtest.cpp
    jsoncheckertest.cpp
    readertest.cpp
    unittest.cpp
    unittest.h
    valuetest.cpp
    writertest.cpp)

add_executable(unittest ${UNITTEST_SOURCES})
target_link_libraries(unittest ${TEST_LIBRARIES})
add_test(NAME unittest
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unittest
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
