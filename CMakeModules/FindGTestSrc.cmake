SET(GTEST_SEARCH_PATH 
    "${GTEST_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/thirdparty/gtest")

IF(UNIX)
    LIST(INSERT GTEST_SEARCH_PATH 1 "/usr/src/gtest")
ENDIF()

FIND_PATH(GTEST_SOURCE_DIR
    NAMES CMakeLists.txt src/gtest_main.cc
    PATHS ${GTEST_SEARCH_PATH})

# Debian installs gtest include directory in /usr/include, thus need to look
# for include directory separately from source directory.
FIND_PATH(GTEST_INCLUDE_DIR
    NAMES gtest/gtest.h
    PATH_SUFFIXES include
    PATHS ${GTEST_SEARCH_PATH})

find_package_handle_standard_args(GTestSrc DEFAULT_MSG
    GTEST_SOURCE_DIR
    GTEST_INCLUDE_DIR)
