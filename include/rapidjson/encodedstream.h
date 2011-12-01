#ifndef RAPIDJSON_ENCODEDSTREAM_H_
#define RAPIDJSON_ENCODEDSTREAM_H_

#include "rapidjson.h"

namespace rapidjson {

//! Adapts an input byte stream with an specified encoding.
template <typename Encoding, typename InputStream>
class EncodedInputStream {
public:
	typedef typename Encoding::Ch Ch;

	EncodedInputStream(InputStream& is) : is_(is) { 
		current_ = Encoding::TakeBOM(is_);
	}

	Ch Peek() const { return current_; }
	Ch Take() { Ch c = current_; current_ = Encoding::Take(is_); return c; }
	size_t Tell() const { is_.Tell(); }

	// Not implemented
	void Put(Ch c) { RAPIDJSON_ASSERT(false); }
	void Flush() { RAPIDJSON_ASSERT(false); } 
	Ch* PutBegin() { RAPIDJSON_ASSERT(false); return 0; }
	size_t PutEnd(Ch*) { RAPIDJSON_ASSERT(false); return 0; }

private:
	InputStream& is_;
	Ch current_;
};

//! Adapts an output byte stream with an specified encoding.
template <typename Encoding, typename OutputStream>
class EncodedOutputStream {
public:
	typedef typename Encoding::Ch Ch;

	EncodedOutputStream(OutputStream& os, bool putBOM = true) : os_(os) { 
		if (putBOM)
			Encoding::PutBOM(os_);
	}

	void Put(Ch c) { Encoding::Put(os_, c);  }
	void Flush() { os_.Flush(); }

	// Not implemented
	Ch Peek() const { RAPIDJSON_ASSERT(false); }
	Ch Take() { RAPIDJSON_ASSERT(false);  }
	size_t Tell() const { RAPIDJSON_ASSERT(false);  }
	Ch* PutBegin() { RAPIDJSON_ASSERT(false); return 0; }
	size_t PutEnd(Ch*) { RAPIDJSON_ASSERT(false); return 0; }

private:
	OutputStream& os_;
};

#define ENCODINGS_FUNC(x) UTF8<Ch>::x, UTF16LE<Ch>::x, UTF16BE<Ch>::x, UTF32LE<Ch>::x, UTF32BE<Ch>::x

template <typename CharType, typename InputStream>
class AutoUTFInputStream {
public:
	typedef CharType Ch;

	AutoUTFInputStream(InputStream& is, UTFType type = kUTF8) : is_(is), type_(type) {
		DetectType(is);
		static const TakeFunc f[] = { ENCODINGS_FUNC(Take) };
		takeFunc_ = f[type_];
		current_ = takeFunc_(is_);
	}

	UTFType GetType() const { return type_; }

	Ch Peek() const { return current_; }
	Ch Take() { Ch c = current_; current_ = takeFunc_(is_); return c; }
	size_t Tell() const { is_.Tell(); }

	// Not implemented
	void Put(Ch c) { RAPIDJSON_ASSERT(false); }
	void Flush() { RAPIDJSON_ASSERT(false); } 
	Ch* PutBegin() { RAPIDJSON_ASSERT(false); return 0; }
	size_t PutEnd(Ch*) { RAPIDJSON_ASSERT(false); return 0; }

private:
	// Detect encoding type with BOM or RFC 4627
	void DetectType(InputStream& is) {
		// BOM (Byte Order Mark):
		// 00 00 FE FF  UTF-32BE
		// FF FE 00 00  UTF-32LE
		// FE FF		UTF-16BE
		// FF FE		UTF-16LE
		// EF BB BF		UTF-8

		const unsigned char* c = (const unsigned char *)is.Peek4();
		if (!c)
			return;

		unsigned bom = c[0] | (c[1] << 8) | (c[2] << 16) | (c[3] << 24);
		if (bom == 0xFFFE0000)					{ type_ = kUTF32BE; is.Take(); is.Take(); is.Take(); is.Take(); goto sizecheck; }
		else if (bom == 0x0000FEFF)				{ type_ = kUTF32LE;	is.Take(); is.Take(); is.Take(); is.Take();	goto sizecheck;	}
		else if ((bom & 0xFFFF) == 0xFFFE)		{ type_ = kUTF16BE; is.Take(); is.Take();						goto sizecheck; }
		else if ((bom & 0xFFFF) == 0xFEFF)		{ type_ = kUTF16LE; is.Take(); is.Take();						goto sizecheck; }
		else if ((bom & 0xFFFFFF) == 0xBFBBEF)	{ type_ = kUTF8;	is.Take(); is.Take(); is.Take();			goto sizecheck; }

		// RFC 4627: Section 3
		// "Since the first two characters of a JSON text will always be ASCII
		// characters [RFC0020], it is possible to determine whether an octet
		// stream is UTF-8, UTF-16 (BE or LE), or UTF-32 (BE or LE) by looking
		// at the pattern of nulls in the first four octets."
		// 00 00 00 xx  UTF-32BE
		// 00 xx 00 xx  UTF-16BE
		// xx 00 00 00  UTF-32LE
		// xx 00 xx 00  UTF-16LE
		// xx xx xx xx  UTF-8

		unsigned pattern = (c[0] ? 1 : 0) | (c[1] ? 2 : 0) | (c[2] ? 4 : 0) | (c[3] ? 8 : 0);
		switch (pattern) {
		case 0x08: type_ = kUTF32BE; break;
		case 0x0A: type_ = kUTF16BE; break;
		case 0x01: type_ = kUTF32LE; break;
		case 0x05: type_ = kUTF16LE; break;
		case 0x0F: type_ = kUTF8;    break;
		}

	sizecheck:
		// RUntime check whether the size of character type is sufficient. It only perform checks with assertion.
		switch (type_) {
		case kUTF16LE:
		case kUTF16BE:
			RAPIDJSON_ASSERT(sizeof(Ch) >= 2);
			break;
		case kUTF32LE:
		case kUTF32BE:
			RAPIDJSON_ASSERT(sizeof(Ch) >= 4);
			break;
		}
	}

	typedef Ch (*TakeFunc)(InputStream& is);
	InputStream& is_;
	UTFType type_;
	Ch current_;
	TakeFunc takeFunc_;
};

template <typename CharType, typename OutputStream>
class AutoUTFOutputStream {
public:
	typedef CharType Ch;

	AutoUTFOutputStream(OutputStream& os, UTFType type, bool putBOM) : os_(os), type_(type) {
		// RUntime check whether the size of character type is sufficient. It only perform checks with assertion.
		switch (type_) {
		case kUTF16LE:
		case kUTF16BE:
			RAPIDJSON_ASSERT(sizeof(Ch) >= 2);
			break;
		case kUTF32LE:
		case kUTF32BE:
			RAPIDJSON_ASSERT(sizeof(Ch) >= 4);
			break;
		}

		static const PutFunc f[] = { ENCODINGS_FUNC(Put) };
		putFunc_ = f[type_];

		if (putBOM)
			PutBOM();
	}

	UTFType GetType() const { return type_; }

	void Put(Ch c) { 
		putFunc_(os_, c);
	}

	void Flush() { os_.Flush(); } 

	// Not implemented
	Ch Peek() const { RAPIDJSON_ASSERT(false); }
	Ch Take() { RAPIDJSON_ASSERT(false); }
	size_t Tell() const { RAPIDJSON_ASSERT(false); }
	Ch* PutBegin() { RAPIDJSON_ASSERT(false); return 0; }
	size_t PutEnd(Ch*) { RAPIDJSON_ASSERT(false); return 0; }

private:
	void PutBOM() { 
		typedef void (*PutBOMFunc)(OutputStream&);
		static const PutBOMFunc f[] = { ENCODINGS_FUNC(PutBOM) };
		f[type_](os_);
	}

	typedef void (*PutFunc)(OutputStream&, Ch);

	OutputStream& os_;
	UTFType type_;
	PutFunc putFunc_;
};

#undef ENCODINGS_FUNC

} // namespace rapidjson

#endif // RAPIDJSON_FILESTREAM_H_
