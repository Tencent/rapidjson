#ifndef RAPIDJSON_ERROR_ERROR_H__
#define RAPIDJSON_ERROR_ERROR_H__

#include "../reader.h"	// ParseErrorCode

///////////////////////////////////////////////////////////////////////////////
// RAPIDJSON_ERROR_CHARTYPE

//! Character type of error messages.
/*! The default charater type is char.
	On Windows, user can define this macro as TCHAR for supporting both
	unicode/non-unicode settings.
*/
#ifndef RAPIDJSON_ERROR_CHARTYPE
#define RAPIDJSON_ERROR_CHARTYPE char
#endif

///////////////////////////////////////////////////////////////////////////////
// RAPIDJSON_ERROR_STRING

//! Macro for converting string literial to RAPIDJSON_ERROR_CHARTYPE[].
/*! By default this conversion macro does nothing.
	On Windows, user can define this macro as _T(x) for supporting both 
	unicode/non-unicode settings.
*/
#ifndef RAPIDJSON_ERROR_STRING
#define RAPIDJSON_ERROR_STRING(x) x
#endif

namespace rapidjson {

//! Function pointer type of GetParseError().
/*! This is the prototype for GetParseError_X(), where X is a locale.
	User can dynamically change locale in runtime, e.g.:

\code
	GetParseErrorFunc GetParseError = GetParseError_En; // or whatever
	const RAPIDJSON_ERROR_CHARTYPE* s = GetParseError(document.GetParseErrorCode());
\endcode
*/

typedef const RAPIDJSON_ERROR_CHARTYPE* (*GetParseErrorFunc)(ParseErrorCode);

} // namespace rapidjson

#endif // RAPIDJSON_ERROR_ERROR_H__
