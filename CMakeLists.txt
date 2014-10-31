cmake_minimum_required(VERSION 2.8.4)

project(rapidjson)

set(BUILD_CXX11 ON CACHE BOOL "Choose whether to build for C++11")

if (BUILD_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif(BUILD_CXX11)

add_subdirectory(thirdparty/gtest)

set(HEADERS
    include/rapidjson/error/en.h
    include/rapidjson/error/error.h
    include/rapidjson/internal/dtoa.h
    include/rapidjson/internal/itoa.h
    include/rapidjson/internal/meta.h
    include/rapidjson/internal/pow10.h
    include/rapidjson/internal/stack.h
    include/rapidjson/internal/strfunc.h
    include/rapidjson/msinttypes/inttypes.h
    include/rapidjson/msinttypes/stdint.h
    include/rapidjson/allocators.h
    include/rapidjson/document.h
    include/rapidjson/encodedstream.h
    include/rapidjson/encodings.h
    include/rapidjson/filereadstream.h
    include/rapidjson/filestream.h
    include/rapidjson/filewritestream.h
    include/rapidjson/memorybuffer.h
    include/rapidjson/memorystream.h
    include/rapidjson/prettywriter.h
    include/rapidjson/rapidjson.h
    include/rapidjson/reader.h
    include/rapidjson/stringbuffer.h
    include/rapidjson/writer.h)

set(PERF_TEST_SOURCE_FILES
    ${HEADERS}
    test/perftest/misctest.cpp
    test/perftest/perftest.cpp
    test/perftest/perftest.h
    test/perftest/platformtest.cpp
    test/perftest/rapidjsontest.cpp)

set(UNIT_TEST_SOURCE_FILES
    ${HEADERS}
    test/unittest/documenttest.cpp
    test/unittest/encodedstreamtest.cpp
    test/unittest/encodingstest.cpp
    test/unittest/filestreamtest.cpp
    test/unittest/jsoncheckertest.cpp
    test/unittest/readertest.cpp
    test/unittest/unittest.cpp
    test/unittest/unittest.h
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
