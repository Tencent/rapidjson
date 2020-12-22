# ===========================================================================
# CMAKE PART: BUILD-OPTIONS: Setup CMAKE Build Config
# ===========================================================================
# USE: INCLUDE-BEFOR project() !!!

# ---------------------------------------------------------------------------
# use ccache if found
# ---------------------------------------------------------------------------
if(NOT DEFINED CMAKE_CXX_COMPILER_LAUNCHER)
    find_program(CCACHE_EXECUTABLE "ccache")
    if(CCACHE_EXECUTABLE)
        message(STATUS "use ccache")
        set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}" CACHE PATH "path to ccache" FORCE)
    endif()
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
