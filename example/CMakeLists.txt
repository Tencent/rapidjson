
set(EXAMPLES
    capitalize
    condense
    filterkey
    filterkeydom
    jsonx
    lookaheadparser
    messagereader
    parsebyparts
    pretty
    prettyauto
    schemavalidator
    serialize
    simpledom
    simplereader
    simplepullreader
    simplewriter
    sortkeys
    tutorial)

include_directories("../include/")

add_definitions(-D__STDC_FORMAT_MACROS)
set_property(DIRECTORY PROPERTY COMPILE_OPTIONS ${EXTRA_CXX_FLAGS})

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

add_executable(archivertest archiver/archiver.cpp archiver/archivertest.cpp)

foreach(example ${EXAMPLES})
    add_executable(${example} ${example}/${example}.cpp)
endforeach()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_link_libraries(parsebyparts pthread)
endif()

add_custom_target(examples ALL DEPENDS ${EXAMPLES})
