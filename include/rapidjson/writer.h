#ifndef RAPIDJSON_WRITER_H_
#define RAPIDJSON_WRITER_H_

#include "rapidjson.h"
#include "internal/stack.h"
#include "internal/strfunc.h"
#include <cstdio>	// snprintf() or _sprintf_s()
#include <new>		// placement new

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4127) // conditional expression is constant
#endif

namespace rapidjson {

//! JSON writer
/*! Writer implements the concept Handler.
	It generates JSON text by events to an output os.

	User may programmatically calls the functions of a writer to generate JSON text.

	On the other side, a writer can also be passed to objects that generates events, 

	for example Reader::Parse() and Document::Accept().

	\tparam OutputStream Type of output stream.
	\tparam SourceEncoding Encoding of both source strings.
	\tparam TargetEncoding Encoding of and output stream.
	\note implements Handler concept
*/
template<typename OutputStream, typename SourceEncoding = UTF8<>, typename TargetEncoding = UTF8<>, typename Allocator = MemoryPoolAllocator<> >
class Writer {
public:
	typedef typename SourceEncoding::Ch Ch;

	Writer(OutputStream& os, Allocator* allocator = 0, size_t levelDepth = kDefaultLevelDepth) : 
		os_(os), level_stack_(allocator, levelDepth * sizeof(Level)),
		doublePrecision_(kDefaultDoublePrecision) {}

	//! Set the number of significant digits for \c double values
	/*! When writing a \c double value to the \c OutputStream, the number
		of significant digits is limited to 6 by default.
		\param p maximum number of significant digits (default: 6)
		\return The Writer itself for fluent API.
	*/
	Writer& SetDoublePrecision(int p = kDefaultDoublePrecision) {
		if (p < 0) p = kDefaultDoublePrecision; // negative precision is ignored
		doublePrecision_ = p;
		return *this;
	}

	//! \see SetDoublePrecision()
	int GetDoublePrecision() const { return doublePrecision_; }

	//@name Implementation of Handler
	//@{
	Writer& Null()					{ Prefix(kNullType);   WriteNull();			return *this; }
	Writer& Bool(bool b)			{ Prefix(b ? kTrueType : kFalseType); WriteBool(b); return *this; }
	Writer& Int(int i)				{ Prefix(kNumberType); WriteInt(i);			return *this; }
	Writer& Uint(unsigned u)		{ Prefix(kNumberType); WriteUint(u);		return *this; }
	Writer& Int64(int64_t i64)		{ Prefix(kNumberType); WriteInt64(i64);		return *this; }
	Writer& Uint64(uint64_t u64)	{ Prefix(kNumberType); WriteUint64(u64);	return *this; }

	//! Writes the given \c double value to the stream
	/*!
		The number of significant digits (the precision) to be written
		can be set by \ref SetDoublePrecision() for the Writer:
		\code
		Writer<...> writer(...);
		writer.SetDoublePrecision(12).Double(M_PI);
		\endcode
		\param d The value to be written.
		\return The Writer itself for fluent API.
	*/
	Writer& Double(double d)		{ Prefix(kNumberType); WriteDouble(d);		return *this; }

	Writer& String(const Ch* str, SizeType length, bool copy = false) {
		(void)copy;
		Prefix(kStringType);
		WriteString(str, length);
		return *this;
	}

	Writer& StartObject() {
		Prefix(kObjectType);
		new (level_stack_.template Push<Level>()) Level(false);
		WriteStartObject();
		return *this;
	}

	Writer& EndObject(SizeType memberCount = 0) {
		(void)memberCount;
		RAPIDJSON_ASSERT(level_stack_.GetSize() >= sizeof(Level));
		RAPIDJSON_ASSERT(!level_stack_.template Top<Level>()->inArray);
		level_stack_.template Pop<Level>(1);
		WriteEndObject();
		if (level_stack_.Empty())	// end of json text
			os_.Flush();
		return *this;
	}

	Writer& StartArray() {
		Prefix(kArrayType);
		new (level_stack_.template Push<Level>()) Level(true);
		WriteStartArray();
		return *this;
	}

	Writer& EndArray(SizeType elementCount = 0) {
		(void)elementCount;
		RAPIDJSON_ASSERT(level_stack_.GetSize() >= sizeof(Level));
		RAPIDJSON_ASSERT(level_stack_.template Top<Level>()->inArray);
		level_stack_.template Pop<Level>(1);
		WriteEndArray();
		if (level_stack_.Empty())	// end of json text
			os_.Flush();
		return *this;
	}
	//@}

	//! Simpler but slower overload.
	Writer& String(const Ch* str) { return String(str, internal::StrLen(str)); }

protected:
	//! Information for each nested level
	struct Level {
		Level(bool inArray_) : valueCount(0), inArray(inArray_) {}
		size_t valueCount;	//!< number of values in this level
		bool inArray;		//!< true if in array, otherwise in object
	};

	static const size_t kDefaultLevelDepth = 32;

	void WriteNull()  {
		os_.Put('n'); os_.Put('u'); os_.Put('l'); os_.Put('l');
	}

	void WriteBool(bool b)  {
		if (b) {
			os_.Put('t'); os_.Put('r'); os_.Put('u'); os_.Put('e');
		}
		else {
			os_.Put('f'); os_.Put('a'); os_.Put('l'); os_.Put('s'); os_.Put('e');
		}
	}

	void WriteInt(int i) {
		if (i < 0) {
			os_.Put('-');
			i = -i;
		}
		WriteUint((unsigned)i);
	}

	void WriteUint(unsigned u) {
		char buffer[10];
		char *p = buffer;
		do {
			*p++ = (u % 10) + '0';
			u /= 10;
		} while (u > 0);

		do {
			--p;
			os_.Put(*p);
		} while (p != buffer);
	}

	void WriteInt64(int64_t i64) {
		if (i64 < 0) {
			os_.Put('-');
			i64 = -i64;
		}
		WriteUint64((uint64_t)i64);
	}

	void WriteUint64(uint64_t u64) {
		char buffer[20];
		char *p = buffer;
		do {
			*p++ = char(u64 % 10) + '0';
			u64 /= 10;
		} while (u64 > 0);

		do {
			--p;
			os_.Put(*p);
		} while (p != buffer);
	}

#ifdef _MSC_VER
#define RAPIDJSON_SNPRINTF sprintf_s
#else
#define RAPIDJSON_SNPRINTF snprintf
#endif

	//! \todo Optimization with custom double-to-string converter.
	void WriteDouble(double d) {
		char buffer[100];
		int ret = RAPIDJSON_SNPRINTF(buffer, sizeof(buffer), "%.*g", doublePrecision_, d);
		RAPIDJSON_ASSERT(ret >= 1);
		for (int i = 0; i < ret; i++)
			os_.Put(buffer[i]);
	}
#undef RAPIDJSON_SNPRINTF

	void WriteString(const Ch* str, SizeType length)  {
		static const char hexDigits[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
		static const char escape[256] = {
#define Z16 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
			//0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
			'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'b', 't', 'n', 'u', 'f', 'r', 'u', 'u', // 00
			'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', // 10
			  0,   0, '"',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 20
			Z16, Z16,																		// 30~4F
			  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,'\\',   0,   0,   0, // 50
			Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16								// 60~FF
#undef Z16
		};

		os_.Put('\"');
		GenericStringStream<SourceEncoding> is(str);
		while (is.Tell() < length) {
			const Ch c = is.Peek();
			if ((sizeof(Ch) == 1 || (unsigned)c < 256) && escape[(unsigned char)c])  {
				is.Take();
				os_.Put('\\');
				os_.Put(escape[(unsigned char)c]);
				if (escape[(unsigned char)c] == 'u') {
					os_.Put('0');
					os_.Put('0');
					os_.Put(hexDigits[(unsigned char)c >> 4]);
					os_.Put(hexDigits[(unsigned char)c & 0xF]);
				}
			}
			else
				Transcoder<SourceEncoding, TargetEncoding>::Transcode(is, os_);
		}
		os_.Put('\"');
	}

	void WriteStartObject()	{ os_.Put('{'); }
	void WriteEndObject()	{ os_.Put('}'); }
	void WriteStartArray()	{ os_.Put('['); }
	void WriteEndArray()	{ os_.Put(']'); }

	void Prefix(Type type) {
		(void)type;
		if (level_stack_.GetSize() != 0) { // this value is not at root
			Level* level = level_stack_.template Top<Level>();
			if (level->valueCount > 0) {
				if (level->inArray) 
					os_.Put(','); // add comma if it is not the first element in array
				else  // in object
					os_.Put((level->valueCount % 2 == 0) ? ',' : ':');
			}
			if (!level->inArray && level->valueCount % 2 == 0)
				RAPIDJSON_ASSERT(type == kStringType);  // if it's in object, then even number should be a name
			level->valueCount++;
		}
		else
			RAPIDJSON_ASSERT(type == kObjectType || type == kArrayType);
	}

	OutputStream& os_;
	internal::Stack<Allocator> level_stack_;
	int doublePrecision_;

	static const int kDefaultDoublePrecision = 6;

private:
	// Prohibit assignment for VC C4512 warning
	Writer& operator=(const Writer& w);
};

} // namespace rapidjson

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // RAPIDJSON_RAPIDJSON_H_
