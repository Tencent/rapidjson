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
		Encoding::TakeBOM(is_);
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
	void Read() {
		current_ = Encoding::Take(is_);
	}
	InputStream& is_;
	Ch current_;
};

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
#define TAKE() is.Take()
#define PEEK(x) if ((unsigned char)is.Peek() != x) break
		switch ((unsigned char)is.Peek()) {
		case 0x00: TAKE(); PEEK(0x00); TAKE(); PEEK(0xFE); TAKE(); PEEK(0xFF); type_ = kUTF32BE; return;
		case 0xEF: TAKE(); PEEK(0xBB); TAKE(); PEEK(0xBF); TAKE(); type_ = kUTF8; return;
		case 0xFE: TAKE(); PEEK(0xFF); TAKE(); type_ = kUTF16BE; return;
		case 0xFF: TAKE(); PEEK(0xFE); TAKE();
			if (is.Peek() == 0x00) {
				TAKE(); PEEK(0x00); TAKE(); type_ = kUTF32LE; return;
			}
			type_ = kUTF16LE;
			return;
		}
#undef TAKE
#undef PEEK
	}

	void Read() {
		typedef Ch (*TakeFunc)(InputStream& is);
		static const TakeFunc f[] = {
			UTF8<Ch>::Take,
			UTF16LE<Ch>::Take,
			UTF16BE<Ch>::Take,
			UTF32LE<Ch>::Take,
			UTF32BE<Ch>::Take,
		};

		current_ = f[type_](is_);
	}

	InputStream& is_;
	UTFType type_;
	Ch current_;
};

} // namespace rapidjson

#endif // RAPIDJSON_FILESTREAM_H_
