find_path(RapidJSON_INCLUDE_DIRS
		  NAMES rapidjson/rapidjson.h
		  PATHS ${RAPIDJSON_INCLUDEDIR}
		  		${CMAKE_SOURCE_DIR}/include
		  		ENV RAPIDJSON_INCLUDEDIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(RapidJSON DEFAULT_MSG
								  RapidJSON_INCLUDE_DIRS)

mark_as_advanced(RapidJSON_INCLUDE_DIRS)
