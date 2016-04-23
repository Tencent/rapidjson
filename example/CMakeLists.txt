cmake_minimum_required(VERSION 2.8)

if(POLICY CMP0054)
  cmake_policy(SET CMP0054 NEW)
endif()

set(EXAMPLES
    capitalize
    condense
    filterkey
    filterkeydom
    jsonx
    messagereader
    parsebyparts
    pretty
    prettyauto
    schemavalidator
    serialize
    simpledom
    simplereader
    simplewriter
    tutorial)
    
include_directories("../include/")

add_definitions(-D__STDC_FORMAT_MACROS)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Werror -Wall -Wextra -Weffc++ -Wswitch-default")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra -Weffc++ -Wswitch-default -Wfloat-equal -Wimplicit-fallthrough -Weverything")
endif()

foreach (example ${EXAMPLES})
    add_executable(${example} ${example}/${example}.cpp)
endforeach()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_link_libraries(parsebyparts pthread)
endif()

add_custom_target(examples ALL DEPENDS ${EXAMPLES})
