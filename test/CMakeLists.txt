find_package(GTestSrc)

find_package(GTest)
if(GTEST_FOUND)

    include_directories(SYSTEM ${GTEST_INCLUDE_DIR})
    set(TEST_LIBRARIES ${GTEST_LIBRARY} ${GTEST_MAIN_LIBRARY})

elseif(GTESTSRC_FOUND)

    if (WIN32 AND (NOT CYGWIN) AND (NOT MINGW))
        set(gtest_disable_pthreads ON)
        set(gtest_force_shared_crt ON)
    endif()

    add_subdirectory(${GTEST_SOURCE_DIR} ${CMAKE_BINARY_DIR}/googletest)
    include_directories(SYSTEM ${GTEST_INCLUDE_DIR})

    set(TEST_LIBRARIES gtest gtest_main)
endif()


if(TEST_LIBRARIES)
    add_custom_target(tests ALL)
    add_subdirectory(perftest)
    add_subdirectory(unittest)
endif()
