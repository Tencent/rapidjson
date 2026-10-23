find_package(GTestSrc)

IF(GTESTSRC_FOUND)
    enable_testing()

    if (WIN32 AND (NOT CYGWIN) AND (NOT MINGW))
        set(gtest_disable_pthreads ON)
        set(gtest_force_shared_crt ON)
    endif()

    # The bundled googletest still declares cmake_minimum_required(VERSION 2.6.4)
    # and cannot be upgraded without dropping the pre-C++11 configurations.
    set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
    add_subdirectory(${GTEST_SOURCE_DIR} ${CMAKE_BINARY_DIR}/googletest)
    unset(CMAKE_POLICY_VERSION_MINIMUM)
    include_directories(SYSTEM ${GTEST_INCLUDE_DIR})

    set(TEST_LIBRARIES gtest gtest_main)

    add_custom_target(tests ALL)
    add_subdirectory(perftest)
    add_subdirectory(unittest)

ENDIF(GTESTSRC_FOUND)
