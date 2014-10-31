cmake_minimum_required(VERSION 2.8.4)

project(rapidjson)

set(BUILD_CXX11 ON CACHE BOOL "Whether to build using C++11")

if (BUILD_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif(BUILD_CXX11)

add_subdirectory(thirdparty/gtest)

set(PERF_TEST_SOURCE_FILES
    test/perftest/misctest.cpp
    test/perftest/perftest.cpp
    test/perftest/platformtest.cpp
    test/perftest/rapidjsontest.cpp)

set(UNIT_TEST_SOURCE_FILES
    test/unittest/documenttest.cpp
    test/unittest/encodedstreamtest.cpp
    test/unittest/encodingstest.cpp
    test/unittest/filestreamtest.cpp
    test/unittest/jsoncheckertest.cpp
    test/unittest/readertest.cpp
    test/unittest/unittest.cpp
    test/unittest/valuetest.cpp
    test/unittest/writertest.cpp)

include_directories(include)

add_executable(example_capitalize    ${HEADERS} example/capitalize/capitalize.cpp)
add_executable(example_condense      ${HEADERS} example/condense/condense.cpp)
add_executable(example_messagereader ${HEADERS} example/messagereader/messagereader.cpp)
add_executable(example_pretty        ${HEADERS} example/pretty/pretty.cpp)
add_executable(example_prettyauto    ${HEADERS} example/prettyauto/prettyauto.cpp)
add_executable(example_serialize     ${HEADERS} example/serialize/serialize.cpp)
add_executable(example_simpledom     ${HEADERS} example/simpledom/simpledom.cpp)
add_executable(example_simplereader  ${HEADERS} example/simplereader/simplereader.cpp)
add_executable(example_simplewriter  ${HEADERS} example/simplewriter/simplewriter.cpp)

include_directories(SYSTEM ${gtest_SOURCE_DIR}/include)

add_executable(unittests ${UNIT_TEST_SOURCE_FILES})
add_executable(perftests ${PERF_TEST_SOURCE_FILES})

set_target_properties(unittests PROPERTIES COMPILE_FLAGS "-march=native -Wall -Wextra -Werror -Weffc++ -Wswitch-default")
set_target_properties(perftests PROPERTIES COMPILE_FLAGS "-march=native -Wall -Wextra -Werror -Weffc++ -Wswitch-default")

target_link_libraries(unittests pthread gtest)
target_link_libraries(perftests pthread gtest)

add_custom_target(runtests ALL ./unittests DEPENDS unittests)
