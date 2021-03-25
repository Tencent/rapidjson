
set(GTEST_SEARCH_PATH
    "${GTEST_SOURCE_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}/../thirdparty/gtest/googletest")

if(UNIX)
    if(RAPIDJSON_BUILD_THIRDPARTY_GTEST)
        list(APPEND GTEST_SEARCH_PATH "/usr/src/gtest")
    else()
        list(INSERT GTEST_SEARCH_PATH 1 "/usr/src/gtest")
    endif()
endif()

find_path(GTEST_SOURCE_DIR
    NAMES CMakeLists.txt src/gtest_main.cc
    PATHS ${GTEST_SEARCH_PATH})


# Debian installs gtest include directory in /usr/include, thus need to look
# for include directory separately from source directory.
find_path(GTEST_INCLUDE_DIR
    NAMES gtest/gtest.h
    PATH_SUFFIXES include
    HINTS ${GTEST_SOURCE_DIR}
    PATHS ${GTEST_SEARCH_PATH})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GTestSrc DEFAULT_MSG
    GTEST_SOURCE_DIR
    GTEST_INCLUDE_DIR)
