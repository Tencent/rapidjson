subdirlist(EXAMPLES ${CMAKE_CURRENT_LIST_DIR})

foreach(subdir ${EXAMPLES})
    add_subdirectory(${subdir})
endforeach()
