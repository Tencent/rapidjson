#ifndef RAPIDJSON_WRITER_H_
#define RAPIDJSON_WRITER_H_

#include "rapidjson.h"
#include "internal/stack.h"
#include "internal/strfunc.h"
#include "internal/itoa.h"
#include "stringbuffer.h"
#include <cstdio>	// snprintf() or _sprintf_s()
#include <new>		// placement new

#ifdef _MSC_VER
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(4127) // conditional expression is constant
#endif

namespace rapidjson {

//! JSON writer
/*! Writer implements the concept Handler.
	It generates JSON text by events to an output os.

	User may programmatically calls the functions of a writer to generate JSON text.

	On the other side, a writer can also be passed to objects that generates events, 

	for example Reader::Parse() and Document::Accept().

	\tparam OutputStream Type of output stream.
	\tparam SourceEncoding Encoding of source string.
	\tparam TargetEncoding Encoding of output stream.
	\tparam Allocator Type of allocator for allocating memory of stack.
	\note implements Handler concept
*/
template<typename OutputStream, typename SourceEncoding = UTF8<>, typename TargetEncoding = UTF8<>, typename Allocator = MemoryPoolAllocator<> >
class Writer {
public:
	typedef typename SourceEncoding::Ch Ch;

	//! Constructor
	/*! \param os Output stream.
		\param allocator User supplied allocator. If it is null, it will create a private one.
		\param levelDepth Initial capacity of stack.
	*/
	Writer(OutputStream& os, Allocator* allocator = 0, size_t levelDepth = kDefaultLevelDepth) : 
		os_(&os), level_stack_(allocator, levelDepth * sizeof(Level)),
		doublePrecision_(kDefaultDoublePrecision), hasRoot_(false) {}

	Writer(Allocator* allocator = 0, size_t levelDepth = kDefaultLevelDepth) :
		os_(0), level_stack_(allocator, levelDepth * sizeof(Level)),
		doublePrecision_(kDefaultDoublePrecision), hasRoot_(false) {}

	//! Reset the writer with a new stream.
	/*!
		This function reset the writer with a new stream and default settings,
		in order to make a Writer object reusable for output multiple JSONs.

		\param os New output stream.
		\code
		Writer<OutputStream> writer(os1);
		writer.StartObject();
		// ...
		writer.EndObject();

		writer.Reset(os2);
		writer.StartObject();
		// ...
		writer.EndObject();
		\endcode
	*/
	void Reset(OutputStream& os) {
		os_ = &os;
		doublePrecision_ = kDefaultDoublePrecision;
		hasRoot_ = false;
		level_stack_.Clear();
	}

	//! Checks whether the output is a complete JSON.
	/*!
		A complete JSON has a complete root object or array.
	*/
	bool IsComplete() const {
		return hasRoot_ && level_stack_.Empty();
	}

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

	/*!@name Implementation of Handler
		\see Handler
	*/
	//@{

	bool Null()					{ Prefix(kNullType);   return WriteNull(); }
	bool Bool(bool b)			{ Prefix(b ? kTrueType : kFalseType); return WriteBool(b); }
	bool Int(int i)				{ Prefix(kNumberType); return WriteInt(i); }
	bool Uint(unsigned u)		{ Prefix(kNumberType); return WriteUint(u); }
	bool Int64(int64_t i64)		{ Prefix(kNumberType); return WriteInt64(i64); }
	bool Uint64(uint64_t u64)	{ Prefix(kNumberType); return WriteUint64(u64); }

	//! Writes the given \c double value to the stream
	/*!
		The number of significant digits (the precision) to be written
		can be set by \ref SetDoublePrecision() for the Writer:
		\code
		Writer<...> writer(...);
		writer.SetDoublePrecision(12).Double(M_PI);
		\endcode
		\param d The value to be written.
		\return Whether it is succeed.
	*/
	bool Double(double d)		{ Prefix(kNumberType); return WriteDouble(d); }

	bool String(const Ch* str, SizeType length, bool copy = false) {
		(void)copy;
		Prefix(kStringType);
		return WriteString(str, length);
	}

	bool StartObject() {
		Prefix(kObjectType);
		new (level_stack_.template Push<Level>()) Level(false);
		return WriteStartObject();
	}

	bool EndObject(SizeType memberCount = 0) {
		(void)memberCount;
		RAPIDJSON_ASSERT(level_stack_.GetSize() >= sizeof(Level));
		RAPIDJSON_ASSERT(!level_stack_.template Top<Level>()->inArray);
		level_stack_.template Pop<Level>(1);
		bool ret = WriteEndObject();
		if (level_stack_.Empty())	// end of json text
			os_->Flush();
		return ret;
	}

	bool StartArray() {
		Prefix(kArrayType);
		new (level_stack_.template Push<Level>()) Level(true);
		return WriteStartArray();
	}

	bool EndArray(SizeType elementCount = 0) {
		(void)elementCount;
		RAPIDJSON_ASSERT(level_stack_.GetSize() >= sizeof(Level));
		RAPIDJSON_ASSERT(level_stack_.template Top<Level>()->inArray);
		level_stack_.template Pop<Level>(1);
		bool ret = WriteEndArray();
		if (level_stack_.Empty())	// end of json text
			os_->Flush();
		return ret;
	}
	//@}

	/*! @name Convenience extensions */
	//@{

	//! Writes the given \c double value to the stream (explicit precision)
	/*!
		The currently set double precision is ignored in favor of the explicitly
		given precision for this value.
		\see Double(), SetDoublePrecision(), GetDoublePrecision()
		\param d The value to be written
		\param precision The number of significant digits for this value
		\return Whether it is succeeded.
	*/
	bool Double(double d, int precision) {
		int oldPrecision = GetDoublePrecision();
		SetDoublePrecision(precision);
		bool ret = Double(d);
		SetDoublePrecision(oldPrecision);
		return ret;
	}

	//! Simpler but slower overload.
	bool String(const Ch* str) { return String(str, internal::StrLen(str)); }

	//@}

protected:
	//! Information for each nested level
	struct Level {
		Level(bool inArray_) : valueCount(0), inArray(inArray_) {}
		size_t valueCount;	//!< number of values in this level
		bool inArray;		//!< true if in array, otherwise in object
	};

	static const size_t kDefaultLevelDepth = 32;

	bool WriteNull()  {
		os_->Put('n'); os_->Put('u'); os_->Put('l'); os_->Put('l'); return true;
	}

	bool WriteBool(bool b)  {
		if (b) {
			os_->Put('t'); os_->Put('r'); os_->Put('u'); os_->Put('e');
		}
		else {
			os_->Put('f'); os_->Put('a'); os_->Put('l'); os_->Put('s'); os_->Put('e');
		}
		return true;
	}

	bool WriteInt(int i) {
		char buffer[11];
		const char* end = internal::i32toa(i, buffer);
		for (const char* p = buffer; p != end; ++p)
			os_->Put(*p);
		return true;
	}

	bool WriteUint(unsigned u) {
		char buffer[10];
		const char* end = internal::u32toa(u, buffer);
		for (const char* p = buffer; p != end; ++p)
			os_->Put(*p);
		return true;
	}

	bool WriteInt64(int64_t i64) {
		char buffer[21];
		const char* end = internal::i64toa(i64, buffer);
		for (const char* p = buffer; p != end; ++p)
			os_->Put(*p);
		return true;
	}

	bool WriteUint64(uint64_t u64) {
		char buffer[20];
		const char* end = internal::u64toa(u64, buffer);
		for (const char* p = buffer; p != end; ++p)
			os_->Put(*p);
		return true;
	}

#ifdef _MSC_VER
#define RAPIDJSON_SNPRINTF sprintf_s
#else
#define RAPIDJSON_SNPRINTF snprintf
#endif

	//! \todo Optimization with custom double-to-string converter.
	bool WriteDouble(double d) {
		char buffer[100];
		int ret = RAPIDJSON_SNPRINTF(buffer, sizeof(buffer), "%.*g", doublePrecision_, d);
		RAPIDJSON_ASSERT(ret >= 1);
		for (int i = 0; i < ret; i++)
			os_->Put(buffer[i]);
		return true;
	}
#undef RAPIDJSON_SNPRINTF

	bool WriteString(const Ch* str, SizeType length)  {
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

		os_->Put('\"');
		GenericStringStream<SourceEncoding> is(str);
		while (is.Tell() < length) {
			const Ch c = is.Peek();
			if (!TargetEncoding::supportUnicode && (unsigned)c >= 0x80) {
				// Unicode escaping
				unsigned codepoint;
				if (!SourceEncoding::Decode(is, &codepoint))
					return false;
				os_->Put('\\');
				os_->Put('u');
				if (codepoint <= 0xD7FF || (codepoint >= 0xE000 && codepoint <= 0xFFFF)) {
					os_->Put(hexDigits[(codepoint >> 12) & 15]);
					os_->Put(hexDigits[(codepoint >>  8) & 15]);
					os_->Put(hexDigits[(codepoint >>  4) & 15]);
					os_->Put(hexDigits[(codepoint      ) & 15]);
				}
				else if (codepoint >= 0x010000 && codepoint <= 0x10FFFF)	{
					// Surrogate pair
					unsigned s = codepoint - 0x010000;
					unsigned lead = (s >> 10) + 0xD800;
					unsigned trail = (s & 0x3FF) + 0xDC00;
					os_->Put(hexDigits[(lead >> 12) & 15]);
					os_->Put(hexDigits[(lead >>  8) & 15]);
					os_->Put(hexDigits[(lead >>  4) & 15]);
					os_->Put(hexDigits[(lead      ) & 15]);
					os_->Put('\\');
					os_->Put('u');
					os_->Put(hexDigits[(trail >> 12) & 15]);
					os_->Put(hexDigits[(trail >>  8) & 15]);
					os_->Put(hexDigits[(trail >>  4) & 15]);
					os_->Put(hexDigits[(trail      ) & 15]);					
				}
				else
					return false;	// invalid code point
			}
			else if ((sizeof(Ch) == 1 || (unsigned)c < 256) && escape[(unsigned char)c])  {
				is.Take();
				os_->Put('\\');
				os_->Put(escape[(unsigned char)c]);
				if (escape[(unsigned char)c] == 'u') {
					os_->Put('0');
					os_->Put('0');
					os_->Put(hexDigits[(unsigned char)c >> 4]);
					os_->Put(hexDigits[(unsigned char)c & 0xF]);
				}
			}
			else
				Transcoder<SourceEncoding, TargetEncoding>::Transcode(is, *os_);
		}
		os_->Put('\"');
		return true;
	}

	bool WriteStartObject()	{ os_->Put('{'); return true; }
	bool WriteEndObject()	{ os_->Put('}'); return true; }
	bool WriteStartArray()	{ os_->Put('['); return true; }
	bool WriteEndArray()	{ os_->Put(']'); return true; }

	void Prefix(Type type) {
		(void)type;
		if (level_stack_.GetSize() != 0) { // this value is not at root
			Level* level = level_stack_.template Top<Level>();
			if (level->valueCount > 0) {
				if (level->inArray) 
					os_->Put(','); // add comma if it is not the first element in array
				else  // in object
					os_->Put((level->valueCount % 2 == 0) ? ',' : ':');
			}
			if (!level->inArray && level->valueCount % 2 == 0)
				RAPIDJSON_ASSERT(type == kStringType);  // if it's in object, then even number should be a name
			level->valueCount++;
		}
		else {
			RAPIDJSON_ASSERT(type == kObjectType || type == kArrayType);
			RAPIDJSON_ASSERT(!hasRoot_);	// Should only has one and only one root.
			hasRoot_ = true;
		}
	}

	OutputStream* os_;
	internal::Stack<Allocator> level_stack_;
	int doublePrecision_;
	bool hasRoot_;

	static const int kDefaultDoublePrecision = 6;

private:
	// Prohibit copy constructor & assignment operator.
	Writer(const Writer&);
	Writer& operator=(const Writer&);
};

// Full specialization for StringStream to prevent memory copying

template<>
inline bool Writer<StringBuffer>::WriteInt(int i) {
	char *buffer = os_->Push(11);
	const char* end = internal::i32toa(i, buffer);
	os_->Pop(11 - (end - buffer));
	return true;
}

template<>
inline bool Writer<StringBuffer>::WriteUint(unsigned u) {
	char *buffer = os_->Push(10);
	const char* end = internal::u32toa(u, buffer);
	os_->Pop(10 - (end - buffer));
	return true;
}

template<>
inline bool Writer<StringBuffer>::WriteInt64(int64_t i64) {
	char *buffer = os_->Push(21);
	const char* end = internal::i64toa(i64, buffer);
	os_->Pop(21 - (end - buffer));
	return true;
}

template<>
inline bool Writer<StringBuffer>::WriteUint64(uint64_t u) {
	char *buffer = os_->Push(20);
	const char* end = internal::u64toa(u, buffer);
	os_->Pop(20 - (end - buffer));
	return true;
}

} // namespace rapidjson

#ifdef _MSC_VER
RAPIDJSON_DIAG_POP
#endif

#endif // RAPIDJSON_RAPIDJSON_H_
