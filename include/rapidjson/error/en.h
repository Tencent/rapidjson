#ifndef RAPIDJSON_ERROR_EN_H__
#define RAPIDJSON_ERROR_EN_H__

#include "error.h"

namespace rapidjson {

//! Maps error code of parsing into error message.
/*!
	\param parseErrorCode Error code obtained in parsing.
	\return the error message.
	\note User can make a copy of this function for localization.
		Using switch-case is safer for future modification of error codes.
*/
inline const RAPIDJSON_ERROR_CHARTYPE* GetParseError_En(ParseErrorCode parseErrorCode) {
	switch (parseErrorCode) {
		case kParseErrorNone:							return RAPIDJSON_ERROR_STRING("No error.");

		case kParseErrorDocumentEmpty:					return RAPIDJSON_ERROR_STRING("The document is empty.");
		case kParseErrorDocumentRootNotObjectOrArray:	return RAPIDJSON_ERROR_STRING("The document root must be either object or array.");
		case kParseErrorDocumentRootNotSingular:		return RAPIDJSON_ERROR_STRING("The document root must not follow by other values.");
	
		case kParseErrorValueInvalid:					return RAPIDJSON_ERROR_STRING("Invalid value.");
	
		case kParseErrorObjectMissName:					return RAPIDJSON_ERROR_STRING("Missing a name for object member.");
		case kParseErrorObjectMissColon:				return RAPIDJSON_ERROR_STRING("Missing a colon after a name of object member.");
		case kParseErrorObjectMissCommaOrCurlyBracket:	return RAPIDJSON_ERROR_STRING("Missing a comma or '}' after an object member.");
	
		case kParseErrorArrayMissCommaOrSquareBracket:	return RAPIDJSON_ERROR_STRING("Missing a comma or ']' after an array element.");

		case kParseErrorStringUnicodeEscapeInvalidHex:	return RAPIDJSON_ERROR_STRING("Incorrect hex digit after \\u escape in string.");
		case kParseErrorStringUnicodeSurrogateInvalid:	return RAPIDJSON_ERROR_STRING("The surrogate pair in string is invalid.");
		case kParseErrorStringEscapeInvalid:			return RAPIDJSON_ERROR_STRING("Invalid escape character in string.");
		case kParseErrorStringMissQuotationMark:		return RAPIDJSON_ERROR_STRING("Missing a closing quotation mark in string.");
		case kParseErrorStringInvalidEncoding:			return RAPIDJSON_ERROR_STRING("Invalid encoding in string.");

		case kParseErrorNumberTooBig:					return RAPIDJSON_ERROR_STRING("Number too big to be stored in double.");
		case kParseErrorNumberMissFraction:				return RAPIDJSON_ERROR_STRING("Miss fraction part in number.");
		case kParseErrorNumberMissExponent:				return RAPIDJSON_ERROR_STRING("Miss exponent in number.");

		case kParseErrorTermination:					return RAPIDJSON_ERROR_STRING("Terminate parsing due to Handler error.");
		case kParseErrorUnspecificSyntaxError:			return RAPIDJSON_ERROR_STRING("Unspecific syntax error.");

		default:
			return RAPIDJSON_ERROR_STRING("Unknown error.");
	}
}

} // namespace rapidjson

#endif // RAPIDJSON_ERROR_EN_H__
