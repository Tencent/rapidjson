#ifndef RAPIDJSON_READER_H_
#define RAPIDJSON_READER_H_

// Copyright (c) 2011 Milo Yip (miloyip@gmail.com)
// Version 0.1

#include "rapidjson.h"
#include "encodings.h"
#include "internal/pow10.h"
#include "internal/stack.h"
#include <csetjmp>

#ifdef RAPIDJSON_SSE42
#include <nmmintrin.h>
#elif defined(RAPIDJSON_SSE2)
#include <emmintrin.h>
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4127) // conditional expression is constant
#endif

#ifndef RAPIDJSON_PARSE_ERROR
#define RAPIDJSON_PARSE_ERROR(msg, offset) \
	RAPIDJSON_MULTILINEMACRO_BEGIN \
	parseError_ = msg; \
	errorOffset_ = offset; \
	longjmp(jmpbuf_, 1); \
	RAPIDJSON_MULTILINEMACRO_END
#endif

namespace rapidjson {

///////////////////////////////////////////////////////////////////////////////
// ParseFlag

//! Combination of parseFlags
enum ParseFlag {
	kParseDefaultFlags = 0,			//!< Default parse flags. Non-destructive parsing. Text strings are decoded into allocated buffer.
	kParseInsituFlag = 1,			//!< In-situ(destructive) parsing.
	kParseValidateEncodingFlag = 2,	//!< Validate encoding of JSON strings.
};

///////////////////////////////////////////////////////////////////////////////
// Handler

/*!	\class rapidjson::Handler
	\brief Concept for receiving events from GenericReader upon parsing.
\code
concept Handler {
	typename Ch;

	void Null();
	void Bool(bool b);
	void Int(int i);
	void Uint(unsigned i);
	void Int64(int64_t i);
	void Uint64(uint64_t i);
	void Double(double d);
	void String(const Ch* str, SizeType length, bool copy);
	void StartObject();
	void EndObject(SizeType memberCount);
	void StartArray();
	void EndArray(SizeType elementCount);
};
\endcode
*/
///////////////////////////////////////////////////////////////////////////////
// BaseReaderHandler

//! Default implementation of Handler.
/*! This can be used as base class of any reader handler.
	\implements Handler
*/
template<typename Encoding = UTF8<> >
struct BaseReaderHandler {
	typedef typename Encoding::Ch Ch;

	void Default() {}
	void Null() { Default(); }
	void Bool(bool) { Default(); }
	void Int(int) { Default(); }
	void Uint(unsigned) { Default(); }
	void Int64(int64_t) { Default(); }
	void Uint64(uint64_t) { Default(); }
	void Double(double) { Default(); }
	void String(const Ch*, SizeType, bool) { Default(); }
	void StartObject() { Default(); }
	void EndObject(SizeType) { Default(); }
	void StartArray() { Default(); }
	void EndArray(SizeType) { Default(); }
};

///////////////////////////////////////////////////////////////////////////////
// SkipWhitespace

//! Skip the JSON white spaces in a stream.
/*! \param stream A input stream for skipping white spaces.
	\note This function has SSE2/SSE4.2 specialization.
*/
template<typename InputStream>
void SkipWhitespace(InputStream& is) {
	InputStream s = is;	// Use a local copy for optimization
	while (s.Peek() == ' ' || s.Peek() == '\n' || s.Peek() == '\r' || s.Peek() == '\t')
		s.Take();
	is = s;
}

#ifdef RAPIDJSON_SSE42
//! Skip whitespace with SSE 4.2 pcmpistrm instruction, testing 16 8-byte characters at once.
inline const char *SkipWhitespace_SIMD(const char* p) {
	static const char whitespace[16] = " \n\r\t";
	__m128i w = _mm_loadu_si128((const __m128i *)&whitespace[0]);

	for (;;) {
		__m128i s = _mm_loadu_si128((const __m128i *)p);
		unsigned r = _mm_cvtsi128_si32(_mm_cmpistrm(w, s, _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_ANY | _SIDD_BIT_MASK | _SIDD_NEGATIVE_POLARITY));
		if (r == 0)	// all 16 characters are whitespace
			p += 16;
		else {		// some of characters may be non-whitespace
#ifdef _MSC_VER		// Find the index of first non-whitespace
			unsigned long offset;
			if (_BitScanForward(&offset, r))
				return p + offset;
#else
			if (r != 0)
				return p + __builtin_ffs(r) - 1;
#endif
		}
	}
}

#elif defined(RAPIDJSON_SSE2)

//! Skip whitespace with SSE2 instructions, testing 16 8-byte characters at once.
inline const char *SkipWhitespace_SIMD(const char* p) {
	static const char whitespaces[4][17] = {
		"                ",
		"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",
		"\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r",
		"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"};

	__m128i w0 = _mm_loadu_si128((const __m128i *)&whitespaces[0][0]);
	__m128i w1 = _mm_loadu_si128((const __m128i *)&whitespaces[1][0]);
	__m128i w2 = _mm_loadu_si128((const __m128i *)&whitespaces[2][0]);
	__m128i w3 = _mm_loadu_si128((const __m128i *)&whitespaces[3][0]);

	for (;;) {
		__m128i s = _mm_loadu_si128((const __m128i *)p);
		__m128i x = _mm_cmpeq_epi8(s, w0);
		x = _mm_or_si128(x, _mm_cmpeq_epi8(s, w1));
		x = _mm_or_si128(x, _mm_cmpeq_epi8(s, w2));
		x = _mm_or_si128(x, _mm_cmpeq_epi8(s, w3));
		unsigned short r = ~_mm_movemask_epi8(x);
		if (r == 0)	// all 16 characters are whitespace
			p += 16;
		else {		// some of characters may be non-whitespace
#ifdef _MSC_VER		// Find the index of first non-whitespace
			unsigned long offset;
			if (_BitScanForward(&offset, r))
				return p + offset;
#else
			if (r != 0)
				return p + __builtin_ffs(r) - 1;
#endif
		}
	}
}

#endif // RAPIDJSON_SSE2

#ifdef RAPIDJSON_SIMD
//! Template function specialization for InsituStringStream
template<> inline void SkipWhitespace(InsituStringStream& is) { 
	is.src_ = const_cast<char*>(SkipWhitespace_SIMD(is.src_));
}

//! Template function specialization for StringStream
template<> inline void SkipWhitespace(StringStream& is) {
	is.src_ = SkipWhitespace_SIMD(is.src_);
}
#endif // RAPIDJSON_SIMD

///////////////////////////////////////////////////////////////////////////////
// GenericReader

//! SAX-style JSON parser. Use Reader for UTF8 encoding and default allocator.
/*! GenericReader parses JSON text from a stream, and send events synchronously to an 
    object implementing Handler concept.

    It needs to allocate a stack for storing a single decoded string during 
    non-destructive parsing.

    For in-situ parsing, the decoded string is directly written to the source 
    text string, no temporary buffer is required.

    A GenericReader object can be reused for parsing multiple JSON text.
    
    \tparam SourceEncoding Encoding of the input stream.
	\tparam TargetEncoding Encoding of the parse output.
    \tparam Allocator Allocator type for stack.
*/
template <typename SourceEncoding, typename TargetEncoding, typename Allocator = MemoryPoolAllocator<> >
class GenericReader {
public:
	typedef typename SourceEncoding::Ch Ch;

	//! Constructor.
	/*! \param allocator Optional allocator for allocating stack memory. (Only use for non-destructive parsing)
		\param stackCapacity stack capacity in bytes for storing a single decoded string.  (Only use for non-destructive parsing)
	*/
	GenericReader(Allocator* allocator = 0, size_t stackCapacity = kDefaultStackCapacity) : stack_(allocator, stackCapacity), parseError_(0), errorOffset_(0) {}

	//! Parse JSON text.
	/*! \tparam parseFlags Combination of ParseFlag. 
		 \tparam InputStream Type of input stream.
		 \tparam Handler Type of handler which must implement Handler concept.
		 \param stream Input stream to be parsed.
		 \param handler The handler to receive events.
		 \return Whether the parsing is successful.
	*/
	template <unsigned parseFlags, typename InputStream, typename Handler>
	bool Parse(InputStream& is, Handler& handler) {
		parseError_ = 0;
		errorOffset_ = 0;

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4611) // interaction between '_setjmp' and C++ object destruction is non-portable
#endif
		if (setjmp(jmpbuf_)) {
#ifdef _MSC_VER
#pragma warning(pop)
#endif
			stack_.Clear();
			return false;
		}

		SkipWhitespace(is);

		if (is.Peek() == '\0')
			RAPIDJSON_PARSE_ERROR("Text only contains white space(s)", is.Tell());
		else {
			switch (is.Peek()) {
				case '{': ParseObject<parseFlags>(is, handler); break;
				case '[': ParseArray<parseFlags>(is, handler); break;
				default: RAPIDJSON_PARSE_ERROR("Expect either an object or array at root", is.Tell());
			}
			SkipWhitespace(is);

			if (is.Peek() != '\0')
				RAPIDJSON_PARSE_ERROR("Nothing should follow the root object or array.", is.Tell());
		}

		return true;
	}

	bool HasParseError() const { return parseError_ != 0; }
	const char* GetParseError() const { return parseError_; }
	size_t GetErrorOffset() const { return errorOffset_; }

private:
	// Parse object: { string : value, ... }
	template<unsigned parseFlags, typename InputStream, typename Handler>
	void ParseObject(InputStream& is, Handler& handler) {
		RAPIDJSON_ASSERT(is.Peek() == '{');
		is.Take();	// Skip '{'
		handler.StartObject();
		SkipWhitespace(is);

		if (is.Peek() == '}') {
			is.Take();
			handler.EndObject(0);	// empty object
			return;
		}

		for (SizeType memberCount = 0;;) {
			if (is.Peek() != '"')
				RAPIDJSON_PARSE_ERROR("Name of an object member must be a string", is.Tell());

			ParseString<parseFlags>(is, handler);
			SkipWhitespace(is);

			if (is.Take() != ':')
				RAPIDJSON_PARSE_ERROR("There must be a colon after the name of object member", is.Tell());

			SkipWhitespace(is);

			ParseValue<parseFlags>(is, handler);
			SkipWhitespace(is);

			++memberCount;

			switch(is.Take()) {
				case ',': SkipWhitespace(is); break;
				case '}': handler.EndObject(memberCount); return;
				default:  RAPIDJSON_PARSE_ERROR("Must be a comma or '}' after an object member", is.Tell());
			}
		}
	}

	// Parse array: [ value, ... ]
	template<unsigned parseFlags, typename InputStream, typename Handler>
	void ParseArray(InputStream& is, Handler& handler) {
		RAPIDJSON_ASSERT(is.Peek() == '[');
		is.Take();	// Skip '['
		handler.StartArray();
		SkipWhitespace(is);

		if (is.Peek() == ']') {
			is.Take();
			handler.EndArray(0); // empty array
			return;
		}

		for (SizeType elementCount = 0;;) {
			ParseValue<parseFlags>(is, handler);
			++elementCount;
			SkipWhitespace(is);

			switch (is.Take()) {
				case ',': SkipWhitespace(is); break;
				case ']': handler.EndArray(elementCount); return;
				default:  RAPIDJSON_PARSE_ERROR("Must be a comma or ']' after an array element.", is.Tell());
			}
		}
	}

	template<unsigned parseFlags, typename InputStream, typename Handler>
	void ParseNull(InputStream& is, Handler& handler) {
		RAPIDJSON_ASSERT(is.Peek() == 'n');
		is.Take();

		if (is.Take() == 'u' && is.Take() == 'l' && is.Take() == 'l')
			handler.Null();
		else
			RAPIDJSON_PARSE_ERROR("Invalid value", is.Tell() - 1);
	}

	template<unsigned parseFlags, typename InputStream, typename Handler>
	void ParseTrue(InputStream& is, Handler& handler) {
		RAPIDJSON_ASSERT(is.Peek() == 't');
		is.Take();

		if (is.Take() == 'r' && is.Take() == 'u' && is.Take() == 'e')
			handler.Bool(true);
		else
			RAPIDJSON_PARSE_ERROR("Invalid value", is.Tell());
	}

	template<unsigned parseFlags, typename InputStream, typename Handler>
	void ParseFalse(InputStream& is, Handler& handler) {
		RAPIDJSON_ASSERT(is.Peek() == 'f');
		is.Take();

		if (is.Take() == 'a' && is.Take() == 'l' && is.Take() == 's' && is.Take() == 'e')
			handler.Bool(false);
		else
			RAPIDJSON_PARSE_ERROR("Invalid value", is.Tell() - 1);
	}

	// Helper function to parse four hexidecimal digits in \uXXXX in ParseString().
	template<typename InputStream>
	unsigned ParseHex4(InputStream& is) {
		InputStream s = is;	// Use a local copy for optimization
		unsigned codepoint = 0;
		for (int i = 0; i < 4; i++) {
			Ch c = s.Take();
			codepoint <<= 4;
			codepoint += c;
			if (c >= '0' && c <= '9')
				codepoint -= '0';
			else if (c >= 'A' && c <= 'F')
				codepoint -= 'A' - 10;
			else if (c >= 'a' && c <= 'f')
				codepoint -= 'a' - 10;
			else
				RAPIDJSON_PARSE_ERROR("Incorrect hex digit after \\u escape", s.Tell() - 1);
		}
		is = s; // Restore is
		return codepoint;
	}

	class StackStream {
	public:
		typedef typename TargetEncoding::Ch Ch;

		StackStream(internal::Stack<Allocator>& stack) : stack_(stack), length_(0) {}
		void Put(Ch c) {
			*stack_.template Push<Ch>() = c;
			++length_;
		}
		internal::Stack<Allocator>& stack_;
		SizeType length_;

	private:
		// Prohibit assignment for VC C4512 warning
		StackStream& operator=(const StackStream&);
	};

	// Parse string and generate String event. Different code paths for kParseInsituFlag.
	template<unsigned parseFlags, typename InputStream, typename Handler>
	void ParseString(InputStream& is, Handler& handler) {
		InputStream s = is;	// Local copy for optimization
		if (parseFlags & kParseInsituFlag) {
			Ch *head = s.PutBegin();
			ParseStringToStream<parseFlags, SourceEncoding, SourceEncoding>(s, s);
			size_t length = s.PutEnd(head) - 1;
			RAPIDJSON_ASSERT(length <= 0xFFFFFFFF);
			handler.String((typename TargetEncoding::Ch*)head, SizeType(length), false);
		}
		else {
			StackStream stackStream(stack_);
			ParseStringToStream<parseFlags, SourceEncoding, TargetEncoding>(s, stackStream);
			handler.String(stack_.template Pop<typename TargetEncoding::Ch>(stackStream.length_), stackStream.length_ - 1, true);
		}
		is = s;		// Restore is
	}

	// Parse string to an output is
	// This function handles the prefix/suffix double quotes, escaping, and optional encoding validation.
	template<unsigned parseFlags, typename SEncoding, typename TEncoding, typename InputStream, typename OutputStream>
	RAPIDJSON_FORCEINLINE void ParseStringToStream(InputStream& is, OutputStream& os) {
#define Z16 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
		static const char escape[256] = {
			Z16, Z16, 0, 0,'\"', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'/', 
			Z16, Z16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'\\', 0, 0, 0, 
			0, 0,'\b', 0, 0, 0,'\f', 0, 0, 0, 0, 0, 0, 0,'\n', 0, 
			0, 0,'\r', 0,'\t', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16
		};
#undef Z16

		RAPIDJSON_ASSERT(is.Peek() == '\"');
		is.Take();	// Skip '\"'

		for (;;) {
			Ch c = is.Peek();
			if (c == '\\') {	// Escape
				is.Take();
				Ch e = is.Take();
				if ((sizeof(Ch) == 1 || unsigned(e) < 256) && escape[(unsigned char)e])
					os.Put(escape[(unsigned char)e]);
				else if (e == 'u') {	// Unicode
					unsigned codepoint = ParseHex4(is);
					if (codepoint >= 0xD800 && codepoint <= 0xDBFF) {
						// Handle UTF-16 surrogate pair
						if (is.Take() != '\\' || is.Take() != 'u')
							RAPIDJSON_PARSE_ERROR("Missing the second \\u in surrogate pair", is.Tell() - 2);
						unsigned codepoint2 = ParseHex4(is);
						if (codepoint2 < 0xDC00 || codepoint2 > 0xDFFF)
							RAPIDJSON_PARSE_ERROR("The second \\u in surrogate pair is invalid", is.Tell() - 2);
						codepoint = (((codepoint - 0xD800) << 10) | (codepoint2 - 0xDC00)) + 0x10000;
					}
					TEncoding::Encode(os, codepoint);
				}
				else
					RAPIDJSON_PARSE_ERROR("Unknown escape character", is.Tell() - 1);
			}
			else if (c == '"') {	// Closing double quote
				is.Take();
				os.Put('\0');	// null-terminate the string
				return;
			}
			else if (c == '\0')
				RAPIDJSON_PARSE_ERROR("lacks ending quotation before the end of string", is.Tell() - 1);
			else if ((unsigned)c < 0x20) // RFC 4627: unescaped = %x20-21 / %x23-5B / %x5D-10FFFF
				RAPIDJSON_PARSE_ERROR("Incorrect unescaped character in string", is.Tell() - 1);
			else {
				if (parseFlags & kParseValidateEncodingFlag ? 
					!Transcoder<SEncoding, TEncoding>::Validate(is, os) : 
					!Transcoder<SEncoding, TEncoding>::Transcode(is, os))
					RAPIDJSON_PARSE_ERROR("Invalid encoding", is.Tell());
			}
		}
	}

	template<unsigned parseFlags, typename InputStream, typename Handler>
	void ParseNumber(InputStream& is, Handler& handler) {
		InputStream s = is; // Local copy for optimization
		// Parse minus
		bool minus = false;
		if (s.Peek() == '-') {
			minus = true;
			s.Take();
		}

		// Parse int: zero / ( digit1-9 *DIGIT )
		unsigned i;
		bool try64bit = false;
		if (s.Peek() == '0') {
			i = 0;
			s.Take();
		}
		else if (s.Peek() >= '1' && s.Peek() <= '9') {
			i = s.Take() - '0';

			if (minus)
				while (s.Peek() >= '0' && s.Peek() <= '9') {
					if (i >= 214748364) { // 2^31 = 2147483648
						if (i != 214748364 || s.Peek() > '8') {
							try64bit = true;
							break;
						}
					}
					i = i * 10 + (s.Take() - '0');
				}
			else
				while (s.Peek() >= '0' && s.Peek() <= '9') {
					if (i >= 429496729) { // 2^32 - 1 = 4294967295
						if (i != 429496729 || s.Peek() > '5') {
							try64bit = true;
							break;
						}
					}
					i = i * 10 + (s.Take() - '0');
				}
		}
		else
			RAPIDJSON_PARSE_ERROR("Expect a value here.", is.Tell());

		// Parse 64bit int
		uint64_t i64 = 0;
		bool useDouble = false;
		if (try64bit) {
			i64 = i;
			if (minus) 
				while (s.Peek() >= '0' && s.Peek() <= '9') {					
					if (i64 >= 922337203685477580uLL) // 2^63 = 9223372036854775808
						if (i64 != 922337203685477580uLL || s.Peek() > '8') {
							useDouble = true;
							break;
						}
					i64 = i64 * 10 + (s.Take() - '0');
				}
			else
				while (s.Peek() >= '0' && s.Peek() <= '9') {					
					if (i64 >= 1844674407370955161uLL) // 2^64 - 1 = 18446744073709551615
						if (i64 != 1844674407370955161uLL || s.Peek() > '5') {
							useDouble = true;
							break;
						}
					i64 = i64 * 10 + (s.Take() - '0');
				}
		}

		// Force double for big integer
		double d = 0.0;
		if (useDouble) {
			d = (double)i64;
			while (s.Peek() >= '0' && s.Peek() <= '9') {
				if (d >= 1E307)
					RAPIDJSON_PARSE_ERROR("Number too big to store in double", is.Tell());
				d = d * 10 + (s.Take() - '0');
			}
		}

		// Parse frac = decimal-point 1*DIGIT
		int expFrac = 0;
		if (s.Peek() == '.') {
			if (!useDouble) {
				d = try64bit ? (double)i64 : (double)i;
				useDouble = true;
			}
			s.Take();

			if (s.Peek() >= '0' && s.Peek() <= '9') {
				d = d * 10 + (s.Take() - '0');
				--expFrac;
			}
			else
				RAPIDJSON_PARSE_ERROR("At least one digit in fraction part", is.Tell());

			while (s.Peek() >= '0' && s.Peek() <= '9') {
				if (expFrac > -16) {
					d = d * 10 + (s.Peek() - '0');
					--expFrac;
				}
				s.Take();
			}
		}

		// Parse exp = e [ minus / plus ] 1*DIGIT
		int exp = 0;
		if (s.Peek() == 'e' || s.Peek() == 'E') {
			if (!useDouble) {
				d = try64bit ? (double)i64 : (double)i;
				useDouble = true;
			}
			s.Take();

			bool expMinus = false;
			if (s.Peek() == '+')
				s.Take();
			else if (s.Peek() == '-') {
				s.Take();
				expMinus = true;
			}

			if (s.Peek() >= '0' && s.Peek() <= '9') {
				exp = s.Take() - '0';
				while (s.Peek() >= '0' && s.Peek() <= '9') {
					exp = exp * 10 + (s.Take() - '0');
					if (exp > 308)
						RAPIDJSON_PARSE_ERROR("Number too big to store in double", is.Tell());
				}
			}
			else
				RAPIDJSON_PARSE_ERROR("At least one digit in exponent", s.Tell());

			if (expMinus)
				exp = -exp;
		}

		// Finish parsing, call event according to the type of number.
		if (useDouble) {
			d *= internal::Pow10(exp + expFrac);
			handler.Double(minus ? -d : d);
		}
		else {
			if (try64bit) {
				if (minus)
					handler.Int64(-(int64_t)i64);
				else
					handler.Uint64(i64);
			}
			else {
				if (minus)
					handler.Int(-(int)i);
				else
					handler.Uint(i);
			}
		}

		is = s; // restore is
	}

	// Parse any JSON value
	template<unsigned parseFlags, typename InputStream, typename Handler>
	void ParseValue(InputStream& is, Handler& handler) {
		switch (is.Peek()) {
			case 'n': ParseNull  <parseFlags>(is, handler); break;
			case 't': ParseTrue  <parseFlags>(is, handler); break;
			case 'f': ParseFalse <parseFlags>(is, handler); break;
			case '"': ParseString<parseFlags>(is, handler); break;
			case '{': ParseObject<parseFlags>(is, handler); break;
			case '[': ParseArray <parseFlags>(is, handler); break;
			default : ParseNumber<parseFlags>(is, handler);
		}
	}

	static const size_t kDefaultStackCapacity = 256;	//!< Default stack capacity in bytes for storing a single decoded string. 
	internal::Stack<Allocator> stack_;	//!< A stack for storing decoded string temporarily during non-destructive parsing.
	jmp_buf jmpbuf_;					//!< setjmp buffer for fast exit from nested parsing function calls.
	const char* parseError_;
	size_t errorOffset_;
}; // class GenericReader

//! Reader with UTF8 encoding and default allocator.
typedef GenericReader<UTF8<>, UTF8<> > Reader;

} // namespace rapidjson

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // RAPIDJSON_READER_H_
