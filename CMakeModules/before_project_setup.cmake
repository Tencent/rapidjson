# ===========================================================================
# CMAKE PART: BUILD-OPTIONS: Setup CMAKE Build Config
# ===========================================================================
# USE: INCLUDE-BEFOR project() !!!

if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.10)
    include_guard(DIRECTORY)
endif()

# ---------------------------------------------------------------------------
# use ccache if found
# ---------------------------------------------------------------------------
find_program(CCACHE_EXECUTABLE "ccache")
if(CCACHE_EXECUTABLE)
    message(STATUS "use ccache")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}" CACHE PATH "path to ccache" FORCE)
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}" CACHE PATH "path to ccache" FORCE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
