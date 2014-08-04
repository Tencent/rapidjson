project(unittest CXX)

set(TEST_SRC
	documenttest.cpp
	encodedstreamtest.cpp
	filestreamtest.cpp
	jsoncheckertest.cpp
	readertest.cpp
	unittest.cpp
	valuetest.cpp
	writertest.cpp)

add_executable(unittest ${TEST_SRC})

find_package(RapidJSON)

target_include_directories(unittest PUBLIC ${RapidJSON_INCLUDE_DIRS})
target_link_libraries(unittest gtest)

set(GTEST_ROOT ${CMAKE_SOURCE_DIR}/thirdparty/gtest)
find_package(GTest REQUIRED)
