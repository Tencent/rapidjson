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
		TakeBOM(is);
		Read();
	}

	Ch Peek() const { return current_; }
	Ch Take() { Ch c = current_; Read(); return c; }
	size_t Tell() const { is_.Tell(); }

	// Not implemented
	void Put(Ch c) { RAPIDJSON_ASSERT(false); }
	void Flush() { RAPIDJSON_ASSERT(false); } 
	Ch* PutBegin() { RAPIDJSON_ASSERT(false); return 0; }
	size_t PutEnd(Ch*) { RAPIDJSON_ASSERT(false); return 0; }

private:
	friend struct AutoUTF<Ch>;

	void TakeBOM(InputStream& is) {
#define ASSUME(x) if ((unsigned char)is.Peek() != x) break; is.Take()
		switch ((unsigned char)is.Peek()) {
		case 0x00: is.Take(); ASSUME(0x00); ASSUME(0xFE); ASSUME(0xFF); type_ = kUTF32BE; break;
		case 0xEF: is.Take(); ASSUME(0xBB); ASSUME(0xBF); type_ = kUTF8; break;
		case 0xFE: is.Take(); ASSUME(0xFF); type_ = kUTF16BE; break;
		case 0xFF: is.Take(); ASSUME(0xFE); 
			if (is.Peek() == 0x00) {
				is.Take(); ASSUME(0x00); type_ = kUTF32LE; break;
			}
			type_ = kUTF16LE;
		}
#undef ASSUME
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

	void Read() {
		typedef Ch (*TakeFunc)(InputStream& is);
		static const TakeFunc f[] = { ENCODINGS_FUNC(Take) };
		current_ = f[type_](is_);
	}

	InputStream& is_;
	UTFType type_;
	Ch current_;
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

		if (putBOM)
			PutBOM();
	}

	void Put(Ch c) { 
		typedef void (*PutFunc)(OutputStream&, Ch);
		static const PutFunc f[] = { ENCODINGS_FUNC(Put) };
		f[type_](os_, c);
	}

	void Flush() { os_.Flush(); } 

	// Not implemented
	Ch Peek() const { RAPIDJSON_ASSERT(false); }
	Ch Take() { RAPIDJSON_ASSERT(false); }
	size_t Tell() const { RAPIDJSON_ASSERT(false); }
	Ch* PutBegin() { RAPIDJSON_ASSERT(false); return 0; }
	size_t PutEnd(Ch*) { RAPIDJSON_ASSERT(false); return 0; }

private:
	friend struct AutoUTF<Ch>;

	void PutBOM() { 
		typedef void (*PutBOMFunc)(OutputStream&);
		static const PutBOMFunc f[] = { ENCODINGS_FUNC(PutBOM) };
		f[type_](os_);
	}


	OutputStream& os_;
	UTFType type_;
};

#undef ENCODINGS_FUNC

} // namespace rapidjson

#endif // RAPIDJSON_FILESTREAM_H_
