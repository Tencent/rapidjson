#ifndef RAPIDJSON_ENCODINGS_H_
#define RAPIDJSON_ENCODINGS_H_

#include "rapidjson.h"

namespace rapidjson {

///////////////////////////////////////////////////////////////////////////////
// Encoding

/*! \class rapidjson::Encoding
	\brief Concept for encoding of Unicode characters.

\code
concept Encoding {
	typename Ch;	//! Type of character.

	//! \brief Encode a Unicode codepoint to a stream.
	//! \param os Output stream.
	//! \param codepoint An unicode codepoint, ranging from 0x0 to 0x10FFFF inclusively.
	template<typename OutputStream>
	static void Encode(OutputStream& os, unsigned codepoint) {

	//! \brief Validate one Unicode codepoint from an encoded stream.
	//! \param is Input stream to obtain codepoint.
	//! \param os Output for copying one codepoint.
	//! \return true if it is valid.
	//! \note This function just validating and copying the codepoint without actually decode it.
	template <typename InputStream, typename OutputStream>
	RAPIDJSON_FORCEINLINE static bool Validate(InputStream& is, OutputStream& os) {
};
\endcode
*/

///////////////////////////////////////////////////////////////////////////////
// UTF8

//! UTF-8 encoding.
/*! http://en.wikipedia.org/wiki/UTF-8
	http://tools.ietf.org/html/rfc3629
	\tparam CharType Type for storing 8-bit UTF-8 data. Default is char.
	\implements Encoding
*/
template<typename CharType = char>
struct UTF8 {
	typedef CharType Ch;

	template<typename OutputStream>
	static void Encode(OutputStream& os, unsigned codepoint) {
		if (codepoint <= 0x7F) 
			os.Put(codepoint & 0xFF);
		else if (codepoint <= 0x7FF) {
			os.Put(0xC0 | ((codepoint >> 6) & 0xFF));
			os.Put(0x80 | ((codepoint & 0x3F)));
		}
		else if (codepoint <= 0xFFFF) {
			os.Put(0xE0 | ((codepoint >> 12) & 0xFF));
			os.Put(0x80 | ((codepoint >> 6) & 0x3F));
			os.Put(0x80 | (codepoint & 0x3F));
		}
		else {
			RAPIDJSON_ASSERT(codepoint <= 0x10FFFF);
			os.Put(0xF0 | ((codepoint >> 18) & 0xFF));
			os.Put(0x80 | ((codepoint >> 12) & 0x3F));
			os.Put(0x80 | ((codepoint >> 6) & 0x3F));
			os.Put(0x80 | (codepoint & 0x3F));
		}
	}

	template <typename InputStream>
	RAPIDJSON_FORCEINLINE static bool Decode(InputStream& is, unsigned* codepoint) {
#define COPY() c = is.Take(); *codepoint = (*codepoint << 6) | ((unsigned char)c & 0x3Fu)
#define TRANS(mask) result &= ((GetType((unsigned char)c) & mask) != 0)
#define TAIL() COPY(); TRANS(0x70)
		Ch c = is.Take();
		if (!(c & 0x80)) {
			*codepoint = (unsigned char)c;
			return true;
		}

		unsigned char type = GetType((unsigned char)c);
		*codepoint = (0xFF >> type) & (unsigned char)c;
		bool result = true;
		switch (type) {
		case 2:	TAIL(); return result;
		case 3:	TAIL(); TAIL(); return result;
		case 4:	COPY(); TRANS(0x50); TAIL(); return result;
		case 5:	COPY(); TRANS(0x10); COPY(); TAIL(); return result;
		case 6: TAIL(); TAIL(); TAIL(); return result;
		case 10: COPY(); TRANS(0x20); TAIL(); return result;
		case 11: COPY(); TRANS(0x60); TAIL(); return result;
		default: return false;
		}
#undef COPY
#undef TRANS
#undef TAIL
	}

	template <typename InputStream, typename OutputStream>
	RAPIDJSON_FORCEINLINE static bool Validate(InputStream& is, OutputStream& os) {
#define COPY() os.Put(c = is.Take())
#define TRANS(mask) result &= ((GetType(c) & mask) != 0)
#define TAIL() COPY(); TRANS(0x70)
		Ch c;
		COPY();
		if (!(c & 0x80))
			return true;

		bool result = true;
		switch (GetType(c)) {
		case 2:	TAIL(); return result;
		case 3:	TAIL(); TAIL(); return result;
		case 4:	COPY(); TRANS(0x50); TAIL(); return result;
		case 5:	COPY(); TRANS(0x10); COPY(); TAIL(); return result;
		case 6: TAIL(); TAIL(); TAIL(); return result;
		case 10: COPY(); TRANS(0x20); TAIL(); return result;
		case 11: COPY(); TRANS(0x60); TAIL(); return result;
		default: return false;
		}
#undef COPY
#undef TRANS
#undef TAIL
	}

	RAPIDJSON_FORCEINLINE static unsigned char GetType(unsigned char c) {
		// Referring to DFA of http://bjoern.hoehrmann.de/utf-8/decoder/dfa/
		// With new mapping 1 -> 0x10, 7 -> 0x20, 9 -> 0x40, such that AND operation can test multiple types.
		static const unsigned char type[] = {
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
			0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
			0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
			0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
			8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
			10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,
		};
		return type[c];
	}

	template <typename InputStream>
	static void TakeBOM(InputStream& is) {
		if ((unsigned char)is.Peek() != 0xEF) return;
		is.Take();
		if ((unsigned char)is.Peek() != 0xBB) return;
		is.Take();
		if ((unsigned char)is.Peek() != 0xBF) return;
		is.Take();
	}

	template <typename InputStream>
	RAPIDJSON_FORCEINLINE static Ch Take(InputStream& is) {
		return is.Take();
	}
};

///////////////////////////////////////////////////////////////////////////////
// UTF16

//! UTF-16 encoding.
/*! http://en.wikipedia.org/wiki/UTF-16
	http://tools.ietf.org/html/rfc2781
	\tparam CharType Type for storing 16-bit UTF-16 data. Default is wchar_t. C++11 may use char16_t instead.
	\implements Encoding
*/
template<typename CharType = wchar_t>
struct UTF16 {
	typedef CharType Ch;

	template<typename OutputStream>
	static void Encode(OutputStream& os, unsigned codepoint) {
		if (codepoint <= 0xFFFF) {
			RAPIDJSON_ASSERT(codepoint < 0xD800 || codepoint > 0xDFFF); // Code point itself cannot be surrogate pair 
			os.Put(codepoint);
		}
		else {
			RAPIDJSON_ASSERT(codepoint <= 0x10FFFF);
			unsigned v = codepoint - 0x10000;
			os.Put((v >> 10) | 0xD800);
			os.Put((v & 0x3FF) | 0xDC00);
		}
	}

	template <typename InputStream>
	RAPIDJSON_FORCEINLINE static bool Decode(InputStream& is, unsigned* codepoint) {
		Ch c = is.Take();
		if (c < 0xD800 || c > 0xDFFF) {
			*codepoint = c;
			return true;
		}
		else if (c < 0xDBFF) {
			*codepoint = (c & 0x3FF) << 10;
			c = is.Take();
			*codepoint |= (c & 0x3FF);
			*codepoint += 0x10000;
			return c >= 0xDC00 && c <= 0xDFFF;
		}
		return false;
	}

	template <typename InputStream, typename OutputStream>
	RAPIDJSON_FORCEINLINE static bool Validate(InputStream& is, OutputStream& os) {
		Ch c;
		os.Put(c = is.Take());
		if (c < 0xD800 || c > 0xDFFF)
			return true;
		else if (c < 0xDBFF) {
			os.Put(c = is.Take());
			return c >= 0xDC00 && c <= 0xDFFF;
		}
		return false;
	}
};

template<typename CharType = wchar_t>
struct UTF16LE : UTF16<CharType> {
	template <typename InputStream>
	static void TakeBOM(InputStream& is) {
		if ((unsigned char)is.Peek() != 0xFF) return;
		is.Take();
		if ((unsigned char)is.Peek() != 0xFE) return;
		is.Take();
	}

	template <typename InputStream>
	RAPIDJSON_FORCEINLINE static CharType Take(InputStream& is) {
		CharType c = (unsigned char)is.Take();
		c |= (unsigned char)is.Take() << 8;
		return c;
	}
};

template<typename CharType = wchar_t>
struct UTF16BE : UTF16<CharType> {
	template <typename InputStream>
	static void TakeBOM(InputStream& is) {
		if ((unsigned char)is.Peek() != 0xFE) return;
		is.Take();
		if ((unsigned char)is.Peek() != 0xFF) return;
		is.Take();
	}

	template <typename InputStream>
	RAPIDJSON_FORCEINLINE static CharType Take(InputStream& is) {
		CharType c = (unsigned char)is.Take() << 8;
		c |= (unsigned char)is.Take();
		return c;
	}
};

///////////////////////////////////////////////////////////////////////////////
// UTF32

//! UTF-32 encoding. 
/*! http://en.wikipedia.org/wiki/UTF-32
	\tparam Ch Type for storing 32-bit UTF-32 data. Default is unsigned. C++11 may use char32_t instead.
	\implements Encoding
*/
template<typename CharType = unsigned>
struct UTF32 {
	typedef CharType Ch;

	template<typename OutputStream>
	static void Encode(OutputStream& os, unsigned codepoint) {
		RAPIDJSON_ASSERT(codepoint <= 0x10FFFF);
		os.Put(codepoint);
	}

	template <typename InputStream>
	RAPIDJSON_FORCEINLINE static bool Decode(InputStream& is, unsigned* codepoint) {
		Ch c = is.Take();
		*codepoint = c;
		return c <= 0x10FFFF;
	}

	template <typename InputStream, typename OutputStream>
	RAPIDJSON_FORCEINLINE static bool Validate(InputStream& is, OutputStream& os) {
		Ch c;
		os.Put(c = is.Take());
		return c <= 0x10FFFF;
	}
};

template<typename CharType = unsigned>
struct UTF32LE : UTF32<CharType> {
	template <typename InputStream>
	static void TakeBOM(InputStream& is) {
		if ((unsigned char)is.Peek() != 0xFF) return;
		is.Take();
		if ((unsigned char)is.Peek() != 0xFE) return;
		is.Take();
		if ((unsigned char)is.Peek() != 0x00) return;
		is.Take();
		if ((unsigned char)is.Peek() != 0x00) return;
		is.Take();
	}

	template <typename InputStream>
	RAPIDJSON_FORCEINLINE static CharType Take(InputStream& is) {
		CharType c = (unsigned char)is.Take();
		c |= (unsigned char)is.Take() << 8;
		c |= (unsigned char)is.Take() << 16;
		c |= (unsigned char)is.Take() << 24;
		return c;
	}
};

template<typename CharType = unsigned>
struct UTF32BE : UTF32<CharType> {
	template <typename InputStream>
	static void TakeBOM(InputStream& is) {
		if ((unsigned char)is.Peek() != 0x00) return;
		is.Take();
		if ((unsigned char)is.Peek() != 0x00) return;
		is.Take();
		if ((unsigned char)is.Peek() != 0xFE) return;
		is.Take();
		if ((unsigned char)is.Peek() != 0xFF) return;
		is.Take();
	}

	template <typename InputStream>
	RAPIDJSON_FORCEINLINE static CharType Take(InputStream& is) {
		CharType c = (unsigned char)is.Take() << 24;
		c |= (unsigned char)is.Take() << 16;
		c |= (unsigned char)is.Take() << 8;
		c |= (unsigned char)is.Take();
		return c;
	}
};

///////////////////////////////////////////////////////////////////////////////
// AutoUTF

enum UTFType {
	kUTF8 = 0,
	kUTF16LE = 1,
	kUTF16BE = 2,
	kUTF32LE = 3,
	kUTF32BE = 4,
};

// Dynamically select encoding according to BOM or user setting.
template<typename CharType>
struct AutoUTF {
	typedef CharType Ch;

	template<typename OutputStream>
	RAPIDJSON_FORCEINLINE static void Encode(OutputStream& os, unsigned codepoint) {
		typedef void (*EncodeFunc)(OutputStream&, unsigned);
		static const EncodeFunc f[] = {
			UTF8<Ch>::Encode,
			UTF16<Ch>::Encode,
			UTF16<Ch>::Encode,
			UTF32<Ch>::Encode,
			UTF32<Ch>::Encode,
		};
		(*f[os.type_])(os, codepoint);
	}

	template <typename InputStream>
	RAPIDJSON_FORCEINLINE static bool Decode(InputStream& is, unsigned* codepoint) {
		typedef bool (*DecodeFunc)(InputStream&, unsigned*);
		static const DecodeFunc f[] = {
			UTF8<Ch>::Decode,
			UTF16<Ch>::Decode,
			UTF16<Ch>::Decode,
			UTF32<Ch>::Decode,
			UTF32<Ch>::Decode,
		};
		return (*f[is.type_])(is, codepoint);
	}

	template <typename InputStream, typename OutputStream>
	RAPIDJSON_FORCEINLINE static bool Validate(InputStream& is, OutputStream& os) {
		typedef bool (*ValidateFunc)(InputStream&, unsigned*);
		static const ValidateFunc f[] = {
			UTF8<Ch>::Decode,
			UTF16<Ch>::Decode,
			UTF16<Ch>::Decode,
			UTF32<Ch>::Decode,
			UTF32<Ch>::Decode,
		};
		return (*f[is.type_])(is, os);
	}
};

///////////////////////////////////////////////////////////////////////////////
// Transcoder

template<typename SourceEncoding, typename TargetEncoding>
struct Transcoder {
	template<typename InputStream, typename OutputStream>
	static bool Transcode(InputStream& is, OutputStream& os) {
		unsigned codepoint;
		if (!SourceEncoding::Decode(is, &codepoint))
			return false;
		TargetEncoding::Encode(os, codepoint);
		return true;
	}

	template<typename InputStream, typename OutputStream>
	static bool Validate(InputStream& is, OutputStream& os) {
		return Transcode(is, os);
	}
};

//! Specialization of Transcoder with same source and target encoding.
template<typename Encoding>
struct Transcoder<Encoding, Encoding> {
	template<typename InputStream, typename OutputStream>
	static bool Transcode(InputStream& is, OutputStream& os) {
		os.Put(is.Take());
		return true;
	}
	
	template<typename InputStream, typename OutputStream>
	static bool Validate(InputStream& is, OutputStream& os) {
		return Encoding::Validate(is, os);
	}
};

} // namespace rapidjson

#endif // RAPIDJSON_ENCODINGS_H_
