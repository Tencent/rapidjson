set(UNITTEST_SOURCES
	allocatorstest.cpp
    bigintegertest.cpp
    documenttest.cpp
    encodedstreamtest.cpp
    encodingstest.cpp
    filestreamtest.cpp
    itoatest.cpp
    jsoncheckertest.cpp
    namespacetest.cpp
    pointertest.cpp
    prettywritertest.cpp
    readertest.cpp
	simdtest.cpp
    stringbuffertest.cpp
    strtodtest.cpp
    unittest.cpp
    valuetest.cpp
    writertest.cpp)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra -Weffc++ -Wswitch-default -Wfloat-equal")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra -Weffc++ -Wswitch-default -Wfloat-equal -Wimplicit-fallthrough")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS=1)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DRAPIDJSON_HAS_STDSTRING=1")

add_library(namespacetest STATIC namespacetest.cpp)

add_executable(unittest ${UNITTEST_SOURCES})
target_link_libraries(unittest ${TEST_LIBRARIES} namespacetest)

add_dependencies(tests unittest)

add_test(NAME unittest
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unittest
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

if(NOT MSVC)
    # Not running SIMD.* unit test cases for Valgrind
    add_test(NAME valgrind_unittest
        COMMAND valgrind --leak-check=full --error-exitcode=1 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unittest --gtest_filter=-SIMD.*
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_test(NAME symbol_check
        COMMAND sh -c "objdump -t -C libnamespacetest.a | grep rapidjson ; test $? -ne 0"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    endif(CMAKE_BUILD_TYPE STREQUAL "Debug")

endif(NOT MSVC)
