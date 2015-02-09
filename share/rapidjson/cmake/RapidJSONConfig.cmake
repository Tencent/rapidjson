set(RapidJSON_SIMD_OPTIMIZATION
    "None"
    CACHE STRING
    "Instruction Set Used For Optimization")

set_property(CACHE
             RapidJSON_SIMD_OPTIMIZATION
             PROPERTY STRINGS
             None SSE2 SSE4.2)

set(RapidJSON_ENDIANNESS
    "Not Set"
    CACHE STRING
    "Endianness Of The Machine")

set_property(CACHE
             RapidJSON_ENDIANNESS
             PROPERTY STRINGS
             "Not Set" Little Big)


if(RapidJSON_SIMD_OPTIMIZATION STREQUAL "SSE2")
    list(APPEND
         RapidJSON_DEFINITIONS
         "-DRAPIDJSON_SSE2")
elseif(RapidJSON_SIMD_OPTIMIZATION STREQUAL "SSE4.2")
    list(APPEND
         RapidJSON_DEFINITIONS
         "-DRAPIDJSON_SSE42")
endif()

if(RapidJSON_ENDIANNESS STREQUAL "Little")
    list(APPEND
         RapidJSON_DEFINITIONS
         "-DRAPIDJSON_ENDIAN=0")
elseif(RapidJSON_ENDIANNESS STREQUAL "Big")
    list(APPEND
         RapidJSON_DEFINITIONS
         "-DRAPIDJSON_ENDIAN=1")
endif()
