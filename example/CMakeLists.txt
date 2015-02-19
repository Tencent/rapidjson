# Copyright (c) 2011 Milo Yip (miloyip@gmail.com)
# Copyright (c) 2013 Rafal Jeczalik (rjeczalik@gmail.com)
# Distributed under the MIT License (see license.txt file)

set(EXAMPLES
    capitalize
    condense
    messagereader
    pretty
    prettyauto
    serialize
    simpledom
    simplereader
    simplewriter
    tutorial)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra -Weffc++ -Wswitch-default")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra -Weffc++ -Wswitch-default")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS=1)
endif()

foreach (example ${EXAMPLES})
    add_executable(${example} ${example}/${example}.cpp)
endforeach()

add_custom_target(examples ALL DEPENDS ${EXAMPLES})
