cmake_minimum_required(VERSION 3.11...4.0)

include(FetchContent)

set(BUILD_GMOCK OFF)
set(INSTALL_GTEST OFF)
if (WIN32 AND (NOT CYGWIN) AND (NOT MINGW))
    set(gtest_disable_pthreads ON)
    set(gtest_force_shared_crt ON)
endif()
set(fetchcontent_extra_args)
if(CMAKE_VERSION GREATER_EQUAL 3.24)
    list(APPEND fetchcontent_extra_args
        FIND_PACKAGE_ARGS CONFIG
    )
endif()
FetchContent_Declare(GTest
    GIT_REPOSITORY https://github.com/google/googletest
    GIT_TAG v1.17.0
    ${fetchcontent_extra_args}
)
FetchContent_MakeAvailable(GTest)

add_custom_target(tests ALL)
add_subdirectory(perftest)
add_subdirectory(unittest)
