find_package(GTest REQUIRED)
if(GTEST_FOUND)
    enable_testing()

    include_directories(SYSTEM ${GTEST_INCLUDE_DIR})

    set(TEST_LIBRARIES ${GTEST_LIBRARY} ${GTEST_MAIN_LIBRARY})

    add_custom_target(tests ALL)
    add_subdirectory(perftest)
    add_subdirectory(unittest)

endif()
