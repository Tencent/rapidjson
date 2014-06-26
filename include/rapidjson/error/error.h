#ifndef RAPIDJSON_ERROR_ERROR_H__
#define RAPIDJSON_ERROR_ERROR_H__

// For example, on Windows, user can define this macro as TCHAR
#ifndef RAPIDJSON_ERROR_CHARTYPE
#define RAPIDJSON_ERROR_CHARTYPE char
#endif

// For example, on Windows, user can define this macro as _T(x)
#ifndef RAPIDJSON_ERROR_STRING
#define RAPIDJSON_ERROR_STRING(x) x
#endif

namespace rapidjson {

// User can dynamically change locale in runtime, e.g.:
//	GetParseErrorFunc GetParseError = GetParseError_En; // or whatever
//	const RAPIDJSON_ERROR_CHARTYPE* s = GetParseError(document.GetParseErrorCode());

typedef const RAPIDJSON_ERROR_CHARTYPE* (*GetParseErrorFunc)(ParseErrorCode);

} // namespace rapidjson

#endif // RAPIDJSON_ERROR_ERROR_H__