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

foreach (example ${EXAMPLES})
    add_executable(${example} ${example}/${example}.cpp)
endforeach()

add_custom_target(examples ALL DEPENDS ${EXAMPLES})
