# ===========================================================================
# CMAKE PART: BUILD-OPTIONS: Setup CMAKE Build Config
# ===========================================================================
# USE: INCLUDE-BEFOR project() !!!

include_guard(DIRECTORY)

# ---------------------------------------------------------------------------
# use ccache if found
# ---------------------------------------------------------------------------
find_program(CCACHE_EXECUTABLE "ccache")
if(CCACHE_EXECUTABLE)
    message(STATUS "use ccache")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}" CACHE PATH "ccache" FORCE)
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}" CACHE PATH "ccache" FORCE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
