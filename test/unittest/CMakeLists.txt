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

set_target_properties(unittest PROPERTIES FOLDER "test")

find_package(RapidJSON)
add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty/gtest-1.7.0
				 thirdparty/gtest
				 EXCLUDE_FROM_ALL)

set_target_properties(gtest PROPERTIES FOLDER "thirdparty")

target_include_directories(unittest
						   PRIVATE
						   ${RapidJSON_INCLUDE_DIRS}
						   ${gtest_SOURCE_DIR}/include)

target_link_libraries(unittest gtest)

if(MSVC)
	target_compile_definitions(unittest PRIVATE "-DGTEST_HAS_TR1_TUPLE=0")
else()
	target_compile_definitions(unittest PRIVATE "-DGTEST_USE_OWN_TR1_TUPLE=1")
endif()

warningsetting(WARNING_SETTING)
target_compile_options(unittest PRIVATE ${WARNING_SETTING})

install(TARGETS unittest DESTINATION bin)
