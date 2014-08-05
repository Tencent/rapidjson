cmake_minimum_required(VERSION 3.0)

macro(subdirlist result curdir)
    file(GLOB children RELATIVE ${curdir} ${curdir}/*)
    set(dirlist "")
    foreach(child ${children})
        if(IS_DIRECTORY ${curdir}/${child})
            list(APPEND dirlist ${child})
        endif()
    endforeach()
    set(${result} ${dirlist})
endmacro()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                      "${CMAKE_SOURCE_DIR}/share/rapidjson/cmake/")

project(RapidJSON CXX)

option(RapidJSON_BUILD_EXAMPLES "Build Examples" ON)
option(RapidJSON_BUILD_TESTS "Build Tests" ON)

if(RapidJSON_BUILD_EXAMPLES STREQUAL "ON")
	add_subdirectory(example)
endif()

if(RapidJSON_BUILD_TESTS STREQUAL "ON")
	add_subdirectory(test)
endif()

install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY share/ DESTINATION share)
install(DIRECTORY bin/ DESTINATION bin)
