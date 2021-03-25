find_package(Doxygen)

if(NOT DOXYGEN_FOUND)
    message(STATUS "No Doxygen found. Documentation won't be built")
else()
    file(GLOB SOURCES ${CMAKE_CURRENT_LIST_DIR}/../include/*)
    file(GLOB MARKDOWN_DOC ${CMAKE_CURRENT_LIST_DIR}/../doc/*.md)
    list(APPEND MARKDOWN_DOC ${CMAKE_CURRENT_LIST_DIR}/../readme.md)

    configure_file(Doxyfile.in Doxyfile @ONLY)
    configure_file(Doxyfile.zh-cn.in Doxyfile.zh-cn @ONLY)

    file(GLOB DOXYFILES ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile*)

    add_custom_command(OUTPUT html
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.zh-cn
        COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/html
        DEPENDS ${MARKDOWN_DOC} ${SOURCES} ${DOXYFILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../
        )

    add_custom_target(doc ALL DEPENDS html)

    if(RAPIDJSON_INSTALL)
        install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
            DESTINATION ${CMAKE_INSTALL_DOCDIR}
            COMPONENT doc)
    endif()
endif()
