#ifndef RAPIDJSON_ERROR_ERROR_H__
#define RAPIDJSON_ERROR_ERROR_H__

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

///////////////////////////////////////////////////////////////////////////////
// ParseErrorCode

//! Error code of parsing.
/*! \see GenericReader::Parse, GenericReader::GetParseErrorCode
*/
enum ParseErrorCode {
	kParseErrorNone = 0,						//!< No error.

	kParseErrorDocumentEmpty,					//!< The document is empty.
	kParseErrorDocumentRootNotObjectOrArray,	//!< The document root must be either object or array.
	kParseErrorDocumentRootNotSingular,			//!< The document root must not follow by other values.

	kParseErrorValueInvalid,					//!< Invalid value.

	kParseErrorObjectMissName,					//!< Missing a name for object member.
	kParseErrorObjectMissColon,					//!< Missing a colon after a name of object member.
	kParseErrorObjectMissCommaOrCurlyBracket,	//!< Missing a comma or '}' after an object member.

	kParseErrorArrayMissCommaOrSquareBracket,	//!< Missing a comma or ']' after an array element.

	kParseErrorStringUnicodeEscapeInvalidHex,	//!< Incorrect hex digit after \\u escape in string.
	kParseErrorStringUnicodeSurrogateInvalid,	//!< The surrogate pair in string is invalid.
	kParseErrorStringEscapeInvalid,				//!< Invalid escape character in string.
	kParseErrorStringMissQuotationMark,			//!< Missing a closing quotation mark in string.
	kParseErrorStringInvalidEncoding,			//!< Invalid encoidng in string.

	kParseErrorNumberTooBig,					//!< Number too big to be stored in double.
	kParseErrorNumberMissFraction,				//!< Miss fraction part in number.
	kParseErrorNumberMissExponent				//!< Miss exponent in number.
};

struct ParseResult {

	ParseResult() : code_(kParseErrorNone), offset_(0) {}
	ParseResult(ParseErrorCode code, size_t offset) : code_(code), offset_(offset) {}

	ParseErrorCode Code() const { return code_; }
	size_t Offset() const { return offset_; }

	operator bool() const { return !IsError(); }
	bool IsError() const { return code_ != kParseErrorNone; }

	bool operator==(const ParseResult& that) const { return code_ == that.code_; }
	bool operator==(ParseErrorCode code) const { return code_ == code; }
	friend bool operator==(ParseErrorCode code, const ParseResult & err) { return code == err.code_; }

	void Clear() { Set(kParseErrorNone); }
	void Set(ParseErrorCode code, size_t offset = 0) { code_ = code; offset_ = offset; }

private:
	ParseErrorCode code_;
	size_t offset_;
};

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
