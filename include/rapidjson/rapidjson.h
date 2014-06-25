#ifndef RAPIDJSON_RAPIDJSON_H_
#define RAPIDJSON_RAPIDJSON_H_

// Copyright (c) 2011 Milo Yip (miloyip@gmail.com)
// Version 0.1

#include <cstdlib>	// malloc(), realloc(), free()
#include <cstring>	// memcpy()

///////////////////////////////////////////////////////////////////////////////
// RAPIDJSON_NO_INT64DEFINE

// Here defines int64_t and uint64_t types in global namespace.
// If user have their own definition, can define RAPIDJSON_NO_INT64DEFINE to disable this.
#ifndef RAPIDJSON_NO_INT64DEFINE
#ifdef _MSC_VER
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#define RAPIDJSON_FORCEINLINE __forceinline
#else
#include <inttypes.h>
#define RAPIDJSON_FORCEINLINE
#endif
#endif // RAPIDJSON_NO_INT64TYPEDEF

///////////////////////////////////////////////////////////////////////////////
// RAPIDJSON_ENDIAN
#define RAPIDJSON_LITTLEENDIAN	0	//!< Little endian machine
#define RAPIDJSON_BIGENDIAN		1	//!< Big endian machine

//! Endianness of the machine.
/*!	GCC provided macro for detecting endianness of the target machine. But other
	compilers may not have this. User can define RAPIDJSON_ENDIAN to either
	RAPIDJSON_LITTLEENDIAN or RAPIDJSON_BIGENDIAN.
*/
#ifndef RAPIDJSON_ENDIAN
#ifdef __BYTE_ORDER__
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define RAPIDJSON_ENDIAN RAPIDJSON_LITTLEENDIAN
#else
#define RAPIDJSON_ENDIAN RAPIDJSON_BIGENDIAN
#endif // __BYTE_ORDER__
#else
#define RAPIDJSON_ENDIAN RAPIDJSON_LITTLEENDIAN	// Assumes little endian otherwise.
#endif
#endif // RAPIDJSON_ENDIAN


///////////////////////////////////////////////////////////////////////////////
// RAPIDJSON_ALIGNSIZE

//! Data alignment of the machine.
/*!
	Some machine requires strict data alignment.
	Currently the default uses 4 bytes alignment. User can customize this.
*/
#ifndef RAPIDJSON_ALIGN
#define RAPIDJSON_ALIGN(x) ((x + 3) & ~3)
#endif

///////////////////////////////////////////////////////////////////////////////
// RAPIDJSON_SSE2/RAPIDJSON_SSE42/RAPIDJSON_SIMD

// Enable SSE2 optimization.
//#define RAPIDJSON_SSE2

// Enable SSE4.2 optimization.
//#define RAPIDJSON_SSE42

#if defined(RAPIDJSON_SSE2) || defined(RAPIDJSON_SSE42)
#define RAPIDJSON_SIMD
#endif

///////////////////////////////////////////////////////////////////////////////
// RAPIDJSON_NO_SIZETYPEDEFINE

#ifndef RAPIDJSON_NO_SIZETYPEDEFINE
namespace rapidjson {
//! Use 32-bit array/string indices even for 64-bit platform, instead of using size_t.
/*! User may override the SizeType by defining RAPIDJSON_NO_SIZETYPEDEFINE.
*/
typedef unsigned SizeType;
} // namespace rapidjson
#endif

///////////////////////////////////////////////////////////////////////////////
// RAPIDJSON_ASSERT

//! Assertion.
/*! By default, rapidjson uses C assert() for assertion.
	User can override it by defining RAPIDJSON_ASSERT(x) macro.
*/
#ifndef RAPIDJSON_ASSERT
#include <cassert>
#define RAPIDJSON_ASSERT(x) assert(x)
#endif // RAPIDJSON_ASSERT

///////////////////////////////////////////////////////////////////////////////
// RAPIDJSON_STATIC_ASSERT

// Adopt from boost
#ifndef RAPIDJSON_STATIC_ASSERT
namespace rapidjson {
template <bool x> struct STATIC_ASSERTION_FAILURE;
template <> struct STATIC_ASSERTION_FAILURE<true> { enum { value = 1 }; };
template<int x> struct StaticAssertTest {};
} // namespace rapidjson

#define RAPIDJSON_JOIN(X, Y) RAPIDJSON_DO_JOIN(X, Y)
#define RAPIDJSON_DO_JOIN(X, Y) RAPIDJSON_DO_JOIN2(X, Y)
#define RAPIDJSON_DO_JOIN2(X, Y) X##Y

#define RAPIDJSON_STATIC_ASSERT(x) typedef ::rapidjson::StaticAssertTest<\
	sizeof(::rapidjson::STATIC_ASSERTION_FAILURE<bool(x) >)>\
	RAPIDJSON_JOIN(StaticAssertTypedef, __LINE__)
#endif

///////////////////////////////////////////////////////////////////////////////
// Helpers

#define RAPIDJSON_MULTILINEMACRO_BEGIN do {  
#define RAPIDJSON_MULTILINEMACRO_END \
} while((void)0, 0)

///////////////////////////////////////////////////////////////////////////////
// Allocators and Encodings

#include "allocators.h"
#include "encodings.h"

namespace rapidjson {

///////////////////////////////////////////////////////////////////////////////
//  Stream

/*! \class rapidjson::Stream
	\brief Concept for reading and writing characters.

	For read-only stream, no need to implement PutBegin(), Put(), Flush() and PutEnd().

	For write-only stream, only need to implement Put() and Flush().

\code
concept Stream {
	typename Ch;	//!< Character type of the stream.

	//! Read the current character from stream without moving the read cursor.
	Ch Peek() const;

	//! Read the current character from stream and moving the read cursor to next character.
	Ch Take();

	//! Get the current read cursor.
	//! \return Number of characters read from start.
	size_t Tell();

	//! Begin writing operation at the current read pointer.
	//! \return The begin writer pointer.
	Ch* PutBegin();

	//! Write a character.
	void Put(Ch c);

	//! Flush the buffer.
	void Flush();

	//! End the writing operation.
	//! \param begin The begin write pointer returned by PutBegin().
	//! \return Number of characters written.
	size_t PutEnd(Ch* begin);
}
\endcode
*/

//! Put N copies of a character to a stream.
template<typename Stream, typename Ch>
inline void PutN(Stream& stream, Ch c, size_t n) {
	for (size_t i = 0; i < n; i++)
		stream.Put(c);
}

///////////////////////////////////////////////////////////////////////////////
// StringStream

//! Read-only string stream.
/*! \implements Stream
*/
template <typename Encoding>
struct GenericStringStream {
	typedef typename Encoding::Ch Ch;

	GenericStringStream(const Ch *src) : src_(src), head_(src) {}

	Ch Peek() const { return *src_; }
	Ch Take() { return *src_++; }
	size_t Tell() const { return src_ - head_; }

	Ch* PutBegin() { RAPIDJSON_ASSERT(false); return 0; }
	void Put(Ch) { RAPIDJSON_ASSERT(false); }
	void Flush() { RAPIDJSON_ASSERT(false); }
	size_t PutEnd(Ch*) { RAPIDJSON_ASSERT(false); return 0; }

	const Ch* src_;		//!< Current read position.
	const Ch* head_;	//!< Original head of the string.
};

typedef GenericStringStream<UTF8<> > StringStream;

///////////////////////////////////////////////////////////////////////////////
// InsituStringStream

//! A read-write string stream.
/*! This string stream is particularly designed for in-situ parsing.
	\implements Stream
*/
template <typename Encoding>
struct GenericInsituStringStream {
	typedef typename Encoding::Ch Ch;

	GenericInsituStringStream(Ch *src) : src_(src), dst_(0), head_(src) {}

	// Read
	Ch Peek() { return *src_; }
	Ch Take() { return *src_++; }
	size_t Tell() { return src_ - head_; }

	// Write
	Ch* PutBegin() { return dst_ = src_; }
	void Put(Ch c) { RAPIDJSON_ASSERT(dst_ != 0); *dst_++ = c; }
	void Flush() {}
	size_t PutEnd(Ch* begin) { return dst_ - begin; }

	Ch* src_;
	Ch* dst_;
	Ch* head_;
};

typedef GenericInsituStringStream<UTF8<> > InsituStringStream;

///////////////////////////////////////////////////////////////////////////////
// Type

//! Type of JSON value
enum Type {
	kNullType = 0,		//!< null
	kFalseType = 1,		//!< false
	kTrueType = 2,		//!< true
	kObjectType = 3,	//!< object
	kArrayType = 4,		//!< array 
	kStringType = 5,	//!< string
	kNumberType = 6,	//!< number
};

} // namespace rapidjson

#endif // RAPIDJSON_RAPIDJSON_H_
