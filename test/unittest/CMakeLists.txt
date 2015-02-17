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

add_library(namespacetest STATIC namespacetest.cpp)

add_executable(unittest ${UNITTEST_SOURCES})
target_link_libraries(unittest ${TEST_LIBRARIES} namespacetest)

add_dependencies(tests unittest)

add_test(NAME unittest
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unittest
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

if(NOT MSVC)
    add_test(NAME valgrind_unittest
        COMMAND valgrind --leak-check=full --error-exitcode=1 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unittest
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_test(NAME symbol_check
        COMMAND sh -c "objdump -t -C libnamespacetest.a | grep rapidjson ; test $? -ne 0"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    endif(CMAKE_BUILD_TYPE STREQUAL "Debug")

endif(NOT MSVC)
