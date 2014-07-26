#ifndef RAPIDJSON_DOCUMENT_H_
#define RAPIDJSON_DOCUMENT_H_

#include "reader.h"
#include "internal/strfunc.h"
#include <new>		// placement new

#ifdef _MSC_VER
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(4127) // conditional expression is constant
#elif defined(__GNUC__)
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(effc++)
#endif

#ifndef RAPIDJSON_NOMEMBERITERATORCLASS
#include "internal/meta.h"
#include <iterator> // std::iterator, std::random_access_iterator_tag
#endif

namespace rapidjson {

// Forward declaration.
template <typename Encoding, typename Allocator>
class GenericValue;

//! Name-value pair in a JSON object value.
/*!
	This class was internal to GenericValue. It used to be a inner struct.
	But a compiler (IBM XL C/C++ for AIX) have reported to have problem with that so it moved as a namespace scope struct.
	https://code.google.com/p/rapidjson/issues/detail?id=64
*/
template <typename Encoding, typename Allocator> 
struct GenericMember { 
	GenericValue<Encoding, Allocator> name;		//!< name of member (must be a string)
	GenericValue<Encoding, Allocator> value;	//!< value of member.
};

#ifndef RAPIDJSON_NOMEMBERITERATORCLASS

//! (Constant) member iterator for a JSON object value
/*!
	\tparam Const Is this a constant iterator?
	\tparam Encoding	Encoding of the value. (Even non-string values need to have the same encoding in a document)
	\tparam Allocator	Allocator type for allocating memory of object, array and string.

	This class implements a Random Access Iterator for GenericMember elements
	of a GenericValue, see ISO/IEC 14882:2003(E) C++ standard, 24.1 [lib.iterator.requirements].

	\note This iterator implementation is mainly intended to avoid implicit
		conversions from iterator values to \c NULL,
		e.g. from GenericValue::FindMember.

	\note Define \c RAPIDJSON_NOMEMBERITERATORCLASS to fall back to a
		pointer-based implementation, if your platform doesn't provide
		the C++ <iterator> header.

	\see GenericMember, GenericValue::MemberIterator, GenericValue::ConstMemberIterator
 */
template <bool Const, typename Encoding, typename Allocator>
class GenericMemberIterator
	: public std::iterator<std::random_access_iterator_tag
		, typename internal::MaybeAddConst<Const,GenericMember<Encoding,Allocator> >::Type> {

	friend class GenericValue<Encoding,Allocator>;
	template <bool, typename, typename> friend class GenericMemberIterator;

	typedef GenericMember<Encoding,Allocator> PlainType;
	typedef typename internal::MaybeAddConst<Const,PlainType>::Type ValueType;
	typedef std::iterator<std::random_access_iterator_tag,ValueType> BaseType;

public:
	//! Iterator type itself
	typedef GenericMemberIterator Iterator;
	//! Constant iterator type
	typedef GenericMemberIterator<true,Encoding,Allocator>  ConstType;
	//! Non-constant iterator type
	typedef GenericMemberIterator<false,Encoding,Allocator> NonConstType;

	//! Pointer to (const) GenericMember
	typedef typename BaseType::pointer         Pointer;
	//! Reference to (const) GenericMember
	typedef typename BaseType::reference       Reference;
	//! Signed integer type (e.g. \c ptrdiff_t)
	typedef typename BaseType::difference_type DifferenceType;

	//! Default constructor (singular value)
	/*! Creates an iterator pointing to no element.
		\note All operations, except for comparisons, are undefined on such values.
	 */
	GenericMemberIterator() : ptr_() {}

	//! Iterator conversions to more const
	/*!
		\param it (Non-const) iterator to copy from

		Allows the creation of an iterator from another GenericMemberIterator
		that is "less const".  Especially, creating a non-constant iterator
		from a constant iterator are disabled:
		\li const -> non-const (not ok)
		\li const -> const (ok)
		\li non-const -> const (ok)
		\li non-const -> non-const (ok)

		\note If the \c Const template parameter is already \c false, this
			constructor effectively defines a regular copy-constructor.
			Otherwise, the copy constructor is implicitly defined.
	*/
	GenericMemberIterator(const NonConstType & it) : ptr_( it.ptr_ ) {}

	//! @name stepping
	//@{
	Iterator& operator++(){ ++ptr_; return *this; }
	Iterator& operator--(){ --ptr_; return *this; }
	Iterator  operator++(int){ Iterator old(*this); ++ptr_; return old; }
	Iterator  operator--(int){ Iterator old(*this); --ptr_; return old; }
	//@}

	//! @name increment/decrement
	//@{
	Iterator operator+(DifferenceType n) const { return Iterator(ptr_+n); }
	Iterator operator-(DifferenceType n) const { return Iterator(ptr_-n); }

	Iterator& operator+=(DifferenceType n) { ptr_+=n; return *this; }
	Iterator& operator-=(DifferenceType n) { ptr_-=n; return *this; }
	//@}

	//! @name relations
	//@{
	bool operator==(Iterator that) const { return ptr_ == that.ptr_; }
	bool operator!=(Iterator that) const { return ptr_ != that.ptr_; }
	bool operator<=(Iterator that) const { return ptr_ <= that.ptr_; }
	bool operator>=(Iterator that) const { return ptr_ >= that.ptr_; }
	bool operator< (Iterator that) const { return ptr_ < that.ptr_; }
	bool operator> (Iterator that) const { return ptr_ > that.ptr_; }
	//@}

	//! @name dereference
	//@{
	Reference operator*() const { return *ptr_; }
	Pointer   operator->() const { return ptr_; }
	Reference operator[](DifferenceType n) const { return ptr_[n]; }
	//@}

	//! Distance
	DifferenceType operator-(Iterator that) const { return ptr_-that.ptr_; }

private:
	//! Internal constructor from plain pointer
	explicit GenericMemberIterator(Pointer p) : ptr_(p) {}

	Pointer ptr_; //!< raw pointer
};

#else // RAPIDJSON_NOMEMBERITERATORCLASS

// class-based member iterator implementation disabled, use plain pointers

template <bool Const, typename Encoding, typename Allocator>
struct GenericMemberIterator;

//! non-const GenericMemberIterator
template <typename Encoding, typename Allocator>
struct GenericMemberIterator<false,Encoding,Allocator> {
	//! use plain pointer as iterator type
	typedef GenericMember<Encoding,Allocator>* Iterator;
};
//! const GenericMemberIterator
template <typename Encoding, typename Allocator>
struct GenericMemberIterator<true,Encoding,Allocator> {
	//! use plain const pointer as iterator type
	typedef const GenericMember<Encoding,Allocator>* Iterator;
};

#endif // RAPIDJSON_NOMEMBERITERATORCLASS

///////////////////////////////////////////////////////////////////////////////
// GenericStringRef

//! Reference to a constant string (not taking a copy)
/*!
	\tparam CharType character type of the string

	This helper class is used to automatically infer constant string
	references for string literals, especially from \c const \b (!)
	character arrays.

	The main use is for creating JSON string values without copying the
	source string via an \ref Allocator.  This requires that the referenced
	string pointers have a sufficient lifetime, which exceeds the lifetime
	of the associated GenericValue.

	\b Example
	\code
	Value v("foo");   // ok, no need to copy & calculate length
	const char foo[] = "foo";
	v.SetString(foo); // ok

	const char* bar = foo;
	// Value x(bar); // not ok, can't rely on bar's lifetime
	Value x(StringRef(bar)); // lifetime explicitly guaranteed by user
	Value y(StringRef(bar, 3));  // ok, explicitly pass length
	\endcode

	\see StringRef, GenericValue::SetString
*/
template<typename CharType>
struct GenericStringRef {
	typedef CharType Ch; //!< character type of the string

	//! Create string reference from \c const character array
	/*!
		This constructor implicitly creates a constant string reference from
		a \c const character array.  It has better performance than
		\ref StringRef(const CharType*) by inferring the string \ref length
		from the array length, and also supports strings containing null
		characters.

		\tparam N length of the string, automatically inferred

		\param str Constant character array, lifetime assumed to be longer
			than the use of the string in e.g. a GenericValue

		\post \ref s == str

		\note Constant complexity.
		\note There is a hidden, private overload to disallow references to
			non-const character arrays to be created via this constructor.
			By this, e.g. function-scope arrays used to be filled via
			\c snprintf are excluded from consideration.
			In such cases, the referenced string should be \b copied to the
			GenericValue instead.
	 */
	template<SizeType N>
	GenericStringRef(const CharType (&str)[N])
		: s(str), length(N-1) {}

	//! Explicitly create string reference from \c const character pointer
	/*!
		This constructor can be used to \b explicitly  create a reference to
		a constant string pointer.

		\see StringRef(const CharType*)

		\param str Constant character pointer, lifetime assumed to be longer
			than the use of the string in e.g. a GenericValue

		\post \ref s == str

		\note There is a hidden, private overload to disallow references to
			non-const character arrays to be created via this constructor.
			By this, e.g. function-scope arrays used to be filled via
			\c snprintf are excluded from consideration.
			In such cases, the referenced string should be \b copied to the
			GenericValue instead.
	 */
	explicit GenericStringRef(const CharType* str)
		: s(str), length(internal::StrLen(str)){}

	//! Create constant string reference from pointer and length
	/*! \param str constant string, lifetime assumed to be longer than the use of the string in e.g. a GenericValue
		\param len length of the string, excluding the trailing NULL terminator

		\post \ref s == str && \ref length == len
		\note Constant complexity.
	 */
	GenericStringRef(const CharType* str, SizeType len)
		: s(str), length(len) { RAPIDJSON_ASSERT(s != NULL); }

	//! implicit conversion to plain CharType pointer
	operator const Ch *() const { return s; }

	const Ch* const s; //!< plain CharType pointer
	const SizeType length; //!< length of the string (excluding the trailing NULL terminator)

private:
	//! Disallow copy-assignment
	GenericStringRef operator=(const GenericStringRef&);
	//! Disallow construction from non-const array
	template<SizeType N>
	GenericStringRef(CharType (&str)[N]) /* = delete */;
};

//! Mark a character pointer as constant string
/*! Mark a plain character pointer as a "string literal".  This function
	can be used to avoid copying a character string to be referenced as a
	value in a JSON GenericValue object, if the string's lifetime is known
	to be valid long enough.
	\tparam CharType Character type of the string
	\param str Constant string, lifetime assumed to be longer than the use of the string in e.g. a GenericValue
	\return GenericStringRef string reference object
	\relatesalso GenericStringRef

	\see GenericValue::GenericValue(StringRefType), GenericValue::operator=(StringRefType), GenericValue::SetString(StringRefType), GenericValue::PushBack(StringRefType, Allocator&), GenericValue::AddMember
*/
template<typename CharType>
inline GenericStringRef<CharType> StringRef(const CharType* str) {
	return GenericStringRef<CharType>(str, internal::StrLen(str));
}

//! Mark a character pointer as constant string
/*! Mark a plain character pointer as a "string literal".  This function
	can be used to avoid copying a character string to be referenced as a
	value in a JSON GenericValue object, if the string's lifetime is known
	to be valid long enough.

	This version has better performance with supplied length, and also
	supports string containing null characters.

	\tparam CharType character type of the string
	\param str Constant string, lifetime assumed to be longer than the use of the string in e.g. a GenericValue
	\param length The length of source string.
	\return GenericStringRef string reference object
	\relatesalso GenericStringRef
*/
template<typename CharType>
inline GenericStringRef<CharType> StringRef(const CharType* str, size_t length) {
	return GenericStringRef<CharType>(str, SizeType(length));
}

///////////////////////////////////////////////////////////////////////////////
// GenericValue

//! Represents a JSON value. Use Value for UTF8 encoding and default allocator.
/*!
	A JSON value can be one of 7 types. This class is a variant type supporting
	these types.

	Use the Value if UTF8 and default allocator

	\tparam Encoding	Encoding of the value. (Even non-string values need to have the same encoding in a document)
	\tparam Allocator	Allocator type for allocating memory of object, array and string.
*/
#pragma pack (push, 4)
template <typename Encoding, typename Allocator = MemoryPoolAllocator<> > 
class GenericValue {
public:
	//! Name-value pair in an object.
	typedef GenericMember<Encoding, Allocator> Member;
	typedef Encoding EncodingType;					//!< Encoding type from template parameter.
	typedef Allocator AllocatorType;				//!< Allocator type from template parameter.
	typedef typename Encoding::Ch Ch;				//!< Character type derived from Encoding.
	typedef GenericStringRef<Ch> StringRefType;		//!< Reference to a constant string
	typedef typename GenericMemberIterator<false,Encoding,Allocator>::Iterator MemberIterator;	//!< Member iterator for iterating in object.
	typedef typename GenericMemberIterator<true,Encoding,Allocator>::Iterator ConstMemberIterator;	//!< Constant member iterator for iterating in object.
	typedef GenericValue* ValueIterator;			//!< Value iterator for iterating in array.
	typedef const GenericValue* ConstValueIterator;	//!< Constant value iterator for iterating in array.

	//!@name Constructors and destructor.
	//@{

	//! Default constructor creates a null value.
	GenericValue() : data_(), flags_(kNullFlag) {}

private:
	//! Copy constructor is not permitted.
	GenericValue(const GenericValue& rhs);

public:

	//! Constructor with JSON value type.
	/*! This creates a Value of specified type with default content.
		\param type	Type of the value.
		\note Default content for number is zero.
	*/
	GenericValue(Type type) : data_(), flags_() {
		static const unsigned defaultFlags[7] = {
			kNullFlag, kFalseFlag, kTrueFlag, kObjectFlag, kArrayFlag, kConstStringFlag,
			kNumberAnyFlag
		};
		RAPIDJSON_ASSERT(type <= kNumberType);
		flags_ = defaultFlags[type];
	}

	//! Explicit copy constructor (with allocator)
	/*! Creates a copy of a Value by using the given Allocator
		\tparam SourceAllocator allocator of \c rhs
		\param rhs Value to copy from (read-only)
		\param allocator Allocator for allocating copied elements and buffers. Commonly use GenericDocument::GetAllocator().
		\see CopyFrom()
	*/
	template< typename SourceAllocator >
	GenericValue(const GenericValue<Encoding,SourceAllocator>& rhs, Allocator & allocator);

	//! Constructor for boolean value.
	/*! \param b Boolean value
		\note This constructor is limited to \em real boolean values and rejects
			implicitly converted types like arbitrary pointers.  Use an explicit cast
			to \c bool, if you want to construct a boolean JSON value in such cases.
	 */
#ifndef RAPIDJSON_DOXYGEN_RUNNING // hide SFINAE from Doxygen
	template <typename T>
	explicit GenericValue(T b, RAPIDJSON_ENABLEIF((internal::IsSame<T,bool>)))
#else
	explicit GenericValue(bool b)
#endif
		: data_(), flags_(b ? kTrueFlag : kFalseFlag) {}

	//! Constructor for int value.
	explicit GenericValue(int i) : data_(), flags_(kNumberIntFlag) {
		data_.n.i64 = i;
		if (i >= 0)
			flags_ |= kUintFlag | kUint64Flag;
	}

	//! Constructor for unsigned value.
	explicit GenericValue(unsigned u) : data_(), flags_(kNumberUintFlag) {
		data_.n.u64 = u; 
		if (!(u & 0x80000000))
			flags_ |= kIntFlag | kInt64Flag;
	}

	//! Constructor for int64_t value.
	explicit GenericValue(int64_t i64) : data_(), flags_(kNumberInt64Flag) {
		data_.n.i64 = i64;
		if (i64 >= 0) {
			flags_ |= kNumberUint64Flag;
			if (!(static_cast<uint64_t>(i64) & UINT64_C(0xFFFFFFFF00000000)))
				flags_ |= kUintFlag;
			if (!(static_cast<uint64_t>(i64) & UINT64_C(0xFFFFFFFF80000000)))
				flags_ |= kIntFlag;
		}
		else if (i64 >= INT64_C(-2147483648))
			flags_ |= kIntFlag;
	}

	//! Constructor for uint64_t value.
	explicit GenericValue(uint64_t u64) : data_(), flags_(kNumberUint64Flag) {
		data_.n.u64 = u64;
		if (!(u64 & UINT64_C(0x8000000000000000)))
			flags_ |= kInt64Flag;
		if (!(u64 & UINT64_C(0xFFFFFFFF00000000)))
			flags_ |= kUintFlag;
		if (!(u64 & UINT64_C(0xFFFFFFFF80000000)))
			flags_ |= kIntFlag;
	}

	//! Constructor for double value.
	explicit GenericValue(double d) : data_(), flags_(kNumberDoubleFlag) { data_.n.d = d; }

	//! Constructor for constant string (i.e. do not make a copy of string)
	GenericValue(const Ch* s, SizeType length) : data_(), flags_() { SetStringRaw(StringRef(s, length)); }

	//! Constructor for constant string (i.e. do not make a copy of string)
	explicit GenericValue(StringRefType s) : data_(), flags_() { SetStringRaw(s); }

	//! Constructor for copy-string (i.e. do make a copy of string)
	GenericValue(const Ch* s, SizeType length, Allocator& allocator) : data_(), flags_() { SetStringRaw(StringRef(s, length), allocator); }

	//! Constructor for copy-string (i.e. do make a copy of string)
	GenericValue(const Ch*s, Allocator& allocator) : data_(), flags_() { SetStringRaw(StringRef(s), allocator); }

	//! Destructor.
	/*! Need to destruct elements of array, members of object, or copy-string.
	*/
	~GenericValue() {
		if (Allocator::kNeedFree) {	// Shortcut by Allocator's trait
			switch(flags_) {
			case kArrayFlag:
				for (GenericValue* v = data_.a.elements; v != data_.a.elements + data_.a.size; ++v)
					v->~GenericValue();
				Allocator::Free(data_.a.elements);
				break;

			case kObjectFlag:
				for (MemberIterator m = MemberBegin(); m != MemberEnd(); ++m) {
					m->name.~GenericValue();
					m->value.~GenericValue();
				}
				Allocator::Free(data_.o.members);
				break;

			case kCopyStringFlag:
				Allocator::Free(const_cast<Ch*>(data_.s.str));
				break;

			default:
				break;	// Do nothing for other types.
			}
		}
	}

	//@}

	//!@name Assignment operators
	//@{

	//! Assignment with move semantics.
	/*! \param rhs Source of the assignment. It will become a null value after assignment.
	*/
	GenericValue& operator=(GenericValue& rhs) {
		RAPIDJSON_ASSERT(this != &rhs);
		this->~GenericValue();
		RawAssign(rhs);
		return *this;
	}

	//! Assignment of constant string reference (no copy)
	/*! \param str Constant string reference to be assigned
		\note This overload is needed to avoid clashes with the generic primitive type assignment overload below.
		\see GenericStringRef, operator=(T)
	*/
	GenericValue& operator=(StringRefType str) {
		GenericValue s(str);
		return *this = s;
	}

	//! Assignment with primitive types.
	/*! \tparam T Either \ref Type, \c int, \c unsigned, \c int64_t, \c uint64_t
		\param value The value to be assigned.

		\note The source type \c T explicitly disallows all pointer types,
			especially (\c const) \ref Ch*.  This helps avoiding implicitly
			referencing character strings with insufficient lifetime, use
			\ref SetString(const Ch*, Allocator&) (for copying) or
			\ref StringRef() (to explicitly mark the pointer as constant) instead.
			All other pointer types would implicitly convert to \c bool,
			use \ref SetBool() instead.
	*/
	template <typename T>
	RAPIDJSON_DISABLEIF_RETURN(internal::IsPointer<T>,GenericValue&)
	operator=(T value) {
		GenericValue v(value);
		return *this = v;
	}

	//! Deep-copy assignment from Value
	/*! Assigns a \b copy of the Value to the current Value object
		\tparam SourceAllocator Allocator type of \c rhs
		\param rhs Value to copy from (read-only)
		\param allocator Allocator to use for copying
	 */
	template <typename SourceAllocator>
	GenericValue& CopyFrom(const GenericValue<Encoding,SourceAllocator>& rhs, Allocator& allocator) {
		RAPIDJSON_ASSERT((void*)this != (void const*)&rhs);
		this->~GenericValue();
		new (this) GenericValue(rhs,allocator);
		return *this;
	}

	//! Exchange the contents of this value with those of other.
	/*!
		\param other Another value.
		\note Constant complexity.
	*/
	GenericValue& Swap(GenericValue& other) {
		GenericValue temp;
		temp.RawAssign(*this);
		RawAssign(other);
		other.RawAssign(temp);
		return *this;
	}

	//! Prepare Value for move semantics
	/*! \return *this */
	GenericValue& Move() { return *this; }
	//@}

	//!@name Type
	//@{

	Type GetType()	const { return static_cast<Type>(flags_ & kTypeMask); }
	bool IsNull()	const { return flags_ == kNullFlag; }
	bool IsFalse()	const { return flags_ == kFalseFlag; }
	bool IsTrue()	const { return flags_ == kTrueFlag; }
	bool IsBool()	const { return (flags_ & kBoolFlag) != 0; }
	bool IsObject()	const { return flags_ == kObjectFlag; }
	bool IsArray()	const { return flags_ == kArrayFlag; }
	bool IsNumber() const { return (flags_ & kNumberFlag) != 0; }
	bool IsInt()	const { return (flags_ & kIntFlag) != 0; }
	bool IsUint()	const { return (flags_ & kUintFlag) != 0; }
	bool IsInt64()	const { return (flags_ & kInt64Flag) != 0; }
	bool IsUint64()	const { return (flags_ & kUint64Flag) != 0; }
	bool IsDouble() const { return (flags_ & kDoubleFlag) != 0; }
	bool IsString() const { return (flags_ & kStringFlag) != 0; }

	//@}

	//!@name Null
	//@{

	GenericValue& SetNull() { this->~GenericValue(); new (this) GenericValue(); return *this; }

	//@}

	//!@name Bool
	//@{

	bool GetBool() const { RAPIDJSON_ASSERT(IsBool()); return flags_ == kTrueFlag; }
	//!< Set boolean value
	/*! \post IsBool() == true */
	GenericValue& SetBool(bool b) { this->~GenericValue(); new (this) GenericValue(b); return *this; }

	//@}

	//!@name Object
	//@{

	//! Set this value as an empty object.
	/*! \post IsObject() == true */
	GenericValue& SetObject() { this->~GenericValue(); new (this) GenericValue(kObjectType); return *this; }

	//! Get the value associated with the name.
	/*!
		\note In version 0.1x, if the member is not found, this function returns a null value. This makes issue 7.
		Since 0.2, if the name is not correct, it will assert.
		If user is unsure whether a member exists, user should use HasMember() first.
		A better approach is to use the now public FindMember().
	*/
	GenericValue& operator[](const Ch* name) {
		GenericValue n(StringRef(name));
		return (*this)[n];
	}
	const GenericValue& operator[](const Ch* name) const { return const_cast<GenericValue&>(*this)[name]; }

	// This version is faster because it does not need a StrLen(). 
	// It can also handle string with null character.
	GenericValue& operator[](const GenericValue& name) {
		MemberIterator member = FindMember(name);
		if (member != MemberEnd())
			return member->value;
		else {
			RAPIDJSON_ASSERT(false);	// see above note
			static GenericValue NullValue;
			return NullValue;
		}
	}
	const GenericValue& operator[](const GenericValue& name) const { return const_cast<GenericValue&>(*this)[name]; }

	//! Const member iterator
	/*! \pre IsObject() == true */
	ConstMemberIterator MemberBegin() const	{ RAPIDJSON_ASSERT(IsObject()); return ConstMemberIterator(data_.o.members); }
	//! Const \em past-the-end member iterator
	/*! \pre IsObject() == true */
	ConstMemberIterator MemberEnd()	const	{ RAPIDJSON_ASSERT(IsObject()); return ConstMemberIterator(data_.o.members + data_.o.size); }
	//! Member iterator
	/*! \pre IsObject() == true */
	MemberIterator MemberBegin()			{ RAPIDJSON_ASSERT(IsObject()); return MemberIterator(data_.o.members); }
	//! \em Past-the-end member iterator
	/*! \pre IsObject() == true */
	MemberIterator MemberEnd()				{ RAPIDJSON_ASSERT(IsObject()); return MemberIterator(data_.o.members + data_.o.size); }

	//! Check whether a member exists in the object.
	/*!
		\note It is better to use FindMember() directly if you need the obtain the value as well.
	*/
	bool HasMember(const Ch* name) const { return FindMember(name) != MemberEnd(); }

	// This version is faster because it does not need a StrLen(). 
	// It can also handle string with null character.
	bool HasMember(const GenericValue& name) const { return FindMember(name) != MemberEnd(); }

	//! Find member by name.
	/*!
		\pre IsObject() == true
		\return Iterator to member, if it exists.
			Otherwise returns \ref MemberEnd().

		\note Earlier versions of Rapidjson returned a \c NULL pointer, in case
			the requested member doesn't exist. For consistency with e.g.
			\c std::map, this has been changed to MemberEnd() now.
	*/
	MemberIterator FindMember(const Ch* name) {
		GenericValue n(StringRef(name));
		return FindMember(n);
	}

	ConstMemberIterator FindMember(const Ch* name) const { return const_cast<GenericValue&>(*this).FindMember(name); }

	// This version is faster because it does not need a StrLen(). 
	// It can also handle string with null character.
	MemberIterator FindMember(const GenericValue& name) {
		RAPIDJSON_ASSERT(IsObject());
		RAPIDJSON_ASSERT(name.IsString());
		SizeType len = name.data_.s.length;
		MemberIterator member = MemberBegin();
		for ( ; member != MemberEnd(); ++member)
			if (member->name.data_.s.length == len && memcmp(member->name.data_.s.str, name.data_.s.str, len * sizeof(Ch)) == 0)
				break;
		return member;
	}
	ConstMemberIterator FindMember(const GenericValue& name) const { return const_cast<GenericValue&>(*this).FindMember(name); }

	//! Add a member (name-value pair) to the object.
	/*! \param name A string value as name of member.
		\param value Value of any type.
		\param allocator	Allocator for reallocating memory. It must be the same one as used before. Commonly use GenericDocument::GetAllocator().
		\return The value itself for fluent API.
		\note The ownership of \c name and \c value will be transferred to this object on success.
		\pre  IsObject() && name.IsString()
		\post name.IsNull() && value.IsNull()
	*/
	GenericValue& AddMember(GenericValue& name, GenericValue& value, Allocator& allocator) {
		RAPIDJSON_ASSERT(IsObject());
		RAPIDJSON_ASSERT(name.IsString());

		Object& o = data_.o;
		if (o.size >= o.capacity) {
			if (o.capacity == 0) {
				o.capacity = kDefaultObjectCapacity;
				o.members = reinterpret_cast<Member*>(allocator.Malloc(o.capacity * sizeof(Member)));
			}
			else {
				SizeType oldCapacity = o.capacity;
				o.capacity *= 2;
				o.members = reinterpret_cast<Member*>(allocator.Realloc(o.members, oldCapacity * sizeof(Member), o.capacity * sizeof(Member)));
			}
		}
		o.members[o.size].name.RawAssign(name);
		o.members[o.size].value.RawAssign(value);
		o.size++;
		return *this;
	}

	//! Add a member (name-value pair) to the object.
	/*! \param name A constant string reference as name of member.
		\param value Value of any type.
		\param allocator	Allocator for reallocating memory. It must be the same one as used before. Commonly use GenericDocument::GetAllocator().
		\return The value itself for fluent API.
		\note The ownership of \c value will be transferred to this object on success.
		\pre  IsObject()
		\post value.IsNull()
	*/
	GenericValue& AddMember(StringRefType name, GenericValue& value, Allocator& allocator) {
		GenericValue n(name);
		return AddMember(n, value, allocator);
	}

	//! Add a constant string value as member (name-value pair) to the object.
	/*! \param name A constant string reference as name of member.
		\param value constant string reference as value of member.
		\param allocator	Allocator for reallocating memory. It must be the same one as used before. Commonly use GenericDocument::GetAllocator().
		\return The value itself for fluent API.
		\pre  IsObject()
		\note This overload is needed to avoid clashes with the generic primitive type AddMember(StringRefType,T,Allocator&) overload below.
	*/
	GenericValue& AddMember(StringRefType name, StringRefType value, Allocator& allocator) {
		GenericValue v(value);
		return AddMember(name, v, allocator);
	}

	//! Add any primitive value as member (name-value pair) to the object.
	/*! \tparam T Either \ref Type, \c int, \c unsigned, \c int64_t, \c uint64_t
		\param name A constant string reference as name of member.
		\param value Value of primitive type \c T as value of member
		\param allocator Allocator for reallocating memory. Commonly use GenericDocument::GetAllocator().
		\return The value itself for fluent API.
		\pre  IsObject()

		\note The source type \c T explicitly disallows all pointer types,
			especially (\c const) \ref Ch*.  This helps avoiding implicitly
			referencing character strings with insufficient lifetime, use
			\ref AddMember(StringRefType, GenericValue&, Allocator&) or \ref
			AddMember(StringRefType, StringRefType, Allocator&).
			All other pointer types would implicitly convert to \c bool,
			use an explicit cast instead, if needed.
	*/
	template <typename T>
	RAPIDJSON_DISABLEIF_RETURN(internal::IsPointer<T>,GenericValue&)
	AddMember(StringRefType name, T value, Allocator& allocator) {
		GenericValue n(name);
		GenericValue v(value);
		return AddMember(n, v, allocator);
	}

	//! Remove a member in object by its name.
	/*! \param name Name of member to be removed.
	    \return Whether the member existed.
	    \note Removing member is implemented by moving the last member. So the ordering of members is changed.
	*/
	bool RemoveMember(const Ch* name) {
		GenericValue n(StringRef(name));
		return RemoveMember(n);
	}

	bool RemoveMember(const GenericValue& name) {
		MemberIterator m = FindMember(name);
		if (m != MemberEnd()) {
			RemoveMember(m);
			return true;
		}
		else
			return false;
	}

	//! Remove a member in object by iterator.
	/*! \param m member iterator (obtained by FindMember() or MemberBegin()).
		\return the new iterator after removal.
		\note Removing member is implemented by moving the last member. So the ordering of members is changed.
	*/
	MemberIterator RemoveMember(MemberIterator m) {
		RAPIDJSON_ASSERT(IsObject());
		RAPIDJSON_ASSERT(data_.o.size > 0);
		RAPIDJSON_ASSERT(data_.o.members != 0);
		RAPIDJSON_ASSERT(m >= MemberBegin() && m < MemberEnd());

		MemberIterator last(data_.o.members + (data_.o.size - 1));
		if (data_.o.size > 1 && m != last) {
			// Move the last one to this place
			m->name = last->name;
			m->value = last->value;
		}
		else {
			// Only one left, just destroy
			m->name.~GenericValue();
			m->value.~GenericValue();
		}
		--data_.o.size;
		return m;
	}

	//@}

	//!@name Array
	//@{

	//! Set this value as an empty array.
	/*! \post IsArray == true */
	GenericValue& SetArray() {	this->~GenericValue(); new (this) GenericValue(kArrayType); return *this; }

	//! Get the number of elements in array.
	SizeType Size() const { RAPIDJSON_ASSERT(IsArray()); return data_.a.size; }

	//! Get the capacity of array.
	SizeType Capacity() const { RAPIDJSON_ASSERT(IsArray()); return data_.a.capacity; }

	//! Check whether the array is empty.
	bool Empty() const { RAPIDJSON_ASSERT(IsArray()); return data_.a.size == 0; }

	//! Remove all elements in the array.
	/*! This function do not deallocate memory in the array, i.e. the capacity is unchanged.
	*/
	void Clear() {
		RAPIDJSON_ASSERT(IsArray()); 
		for (SizeType i = 0; i < data_.a.size; ++i)
			data_.a.elements[i].~GenericValue();
		data_.a.size = 0;
	}

	//! Get an element from array by index.
	/*! \param index Zero-based index of element.
\code
Value a(kArrayType);
a.PushBack(123);
int x = a[0].GetInt();				// Error: operator[ is ambiguous, as 0 also mean a null pointer of const char* type.
int y = a[SizeType(0)].GetInt();	// Cast to SizeType will work.
int z = a[0u].GetInt();				// This works too.
\endcode
	*/
	GenericValue& operator[](SizeType index) {
		RAPIDJSON_ASSERT(IsArray());
		RAPIDJSON_ASSERT(index < data_.a.size);
		return data_.a.elements[index];
	}
	const GenericValue& operator[](SizeType index) const { return const_cast<GenericValue&>(*this)[index]; }

	//! Element iterator
	ValueIterator Begin() { RAPIDJSON_ASSERT(IsArray()); return data_.a.elements; }
	ValueIterator End() { RAPIDJSON_ASSERT(IsArray()); return data_.a.elements + data_.a.size; }
	ConstValueIterator Begin() const { return const_cast<GenericValue&>(*this).Begin(); }
	ConstValueIterator End() const { return const_cast<GenericValue&>(*this).End(); }

	//! Request the array to have enough capacity to store elements.
	/*! \param newCapacity	The capacity that the array at least need to have.
		\param allocator	Allocator for reallocating memory. It must be the same one as used before. Commonly use GenericDocument::GetAllocator().
		\return The value itself for fluent API.
	*/
	GenericValue& Reserve(SizeType newCapacity, Allocator &allocator) {
		RAPIDJSON_ASSERT(IsArray());
		if (newCapacity > data_.a.capacity) {
			data_.a.elements = (GenericValue*)allocator.Realloc(data_.a.elements, data_.a.capacity * sizeof(GenericValue), newCapacity * sizeof(GenericValue));
			data_.a.capacity = newCapacity;
		}
		return *this;
	}

	//! Append a GenericValue at the end of the array.
	/*! \param value		Value to be appended.
		\param allocator	Allocator for reallocating memory. It must be the same one as used before. Commonly use GenericDocument::GetAllocator().
		\pre IsArray() == true
		\post value.IsNull() == true
		\return The value itself for fluent API.
		\note The ownership of \c value will be transferred to this array on success.
		\note If the number of elements to be appended is known, calls Reserve() once first may be more efficient.
	*/
	GenericValue& PushBack(GenericValue& value, Allocator& allocator) {
		RAPIDJSON_ASSERT(IsArray());
		if (data_.a.size >= data_.a.capacity)
			Reserve(data_.a.capacity == 0 ? kDefaultArrayCapacity : data_.a.capacity * 2, allocator);
		data_.a.elements[data_.a.size++].RawAssign(value);
		return *this;
	}

	//! Append a constant string reference at the end of the array.
	/*! \param value		Constant string reference to be appended.
		\param allocator	Allocator for reallocating memory. It must be the same one used previously. Commonly use GenericDocument::GetAllocator().
		\pre IsArray() == true
		\return The value itself for fluent API.
		\note If the number of elements to be appended is known, calls Reserve() once first may be more efficient.
		\see GenericStringRef
	*/
	GenericValue& PushBack(StringRefType value, Allocator& allocator) {
		return (*this).template PushBack<StringRefType>(value, allocator);
	}

	//! Append a primitive value at the end of the array(.)
	/*! \tparam T Either \ref Type, \c int, \c unsigned, \c int64_t, \c uint64_t
		\param value Value of primitive type T to be appended.
		\param allocator	Allocator for reallocating memory. It must be the same one as used before. Commonly use GenericDocument::GetAllocator().
		\pre IsArray() == true
		\return The value itself for fluent API.
		\note If the number of elements to be appended is known, calls Reserve() once first may be more efficient.

		\note The source type \c T explicitly disallows all pointer types,
			especially (\c const) \ref Ch*.  This helps avoiding implicitly
			referencing character strings with insufficient lifetime, use
			\ref PushBack(GenericValue&, Allocator&) or \ref
			PushBack(StringRefType, Allocator&).
			All other pointer types would implicitly convert to \c bool,
			use an explicit cast instead, if needed.
	*/
	template <typename T>
	RAPIDJSON_DISABLEIF_RETURN(internal::IsPointer<T>,GenericValue&)
	PushBack(T value, Allocator& allocator) {
		GenericValue v(value);
		return PushBack(v, allocator);
	}

	//! Remove the last element in the array.
	GenericValue& PopBack() {
		RAPIDJSON_ASSERT(IsArray());
		RAPIDJSON_ASSERT(!Empty());
		data_.a.elements[--data_.a.size].~GenericValue();
		return *this;
	}
	//@}

	//!@name Number
	//@{

	int GetInt() const			{ RAPIDJSON_ASSERT(flags_ & kIntFlag);   return data_.n.i.i;   }
	unsigned GetUint() const	{ RAPIDJSON_ASSERT(flags_ & kUintFlag);  return data_.n.u.u;   }
	int64_t GetInt64() const	{ RAPIDJSON_ASSERT(flags_ & kInt64Flag); return data_.n.i64; }
	uint64_t GetUint64() const	{ RAPIDJSON_ASSERT(flags_ & kUint64Flag); return data_.n.u64; }

	double GetDouble() const {
		RAPIDJSON_ASSERT(IsNumber());
		if ((flags_ & kDoubleFlag) != 0)				return data_.n.d;	// exact type, no conversion.
		if ((flags_ & kIntFlag) != 0)					return data_.n.i.i;	// int -> double
		if ((flags_ & kUintFlag) != 0)					return data_.n.u.u;	// unsigned -> double
		if ((flags_ & kInt64Flag) != 0)					return (double)data_.n.i64; // int64_t -> double (may lose precision)
		RAPIDJSON_ASSERT((flags_ & kUint64Flag) != 0);	return (double)data_.n.u64;	// uint64_t -> double (may lose precision)
	}

	GenericValue& SetInt(int i)				{ this->~GenericValue(); new (this) GenericValue(i);	return *this; }
	GenericValue& SetUint(unsigned u)		{ this->~GenericValue(); new (this) GenericValue(u);	return *this; }
	GenericValue& SetInt64(int64_t i64)		{ this->~GenericValue(); new (this) GenericValue(i64);	return *this; }
	GenericValue& SetUint64(uint64_t u64)	{ this->~GenericValue(); new (this) GenericValue(u64);	return *this; }
	GenericValue& SetDouble(double d)		{ this->~GenericValue(); new (this) GenericValue(d);	return *this; }

	//@}

	//!@name String
	//@{

	const Ch* GetString() const { RAPIDJSON_ASSERT(IsString()); return data_.s.str; }

	//! Get the length of string.
	/*! Since rapidjson permits "\\u0000" in the json string, strlen(v.GetString()) may not equal to v.GetStringLength().
	*/
	SizeType GetStringLength() const { RAPIDJSON_ASSERT(IsString()); return data_.s.length; }

	//! Set this value as a string without copying source string.
	/*! This version has better performance with supplied length, and also support string containing null character.
		\param s source string pointer. 
		\param length The length of source string, excluding the trailing null terminator.
		\return The value itself for fluent API.
		\post IsString() == true && GetString() == s && GetStringLength() == length
		\see SetString(StringRefType)
	*/
	GenericValue& SetString(const Ch* s, SizeType length) { return SetString(StringRef(s, length)); }

	//! Set this value as a string without copying source string.
	/*! \param s source string reference
		\return The value itself for fluent API.
		\post IsString() == true && GetString() == s && GetStringLength() == s.length
	*/
	GenericValue& SetString(StringRefType s) { this->~GenericValue(); SetStringRaw(s); return *this; }

	//! Set this value as a string by copying from source string.
	/*! This version has better performance with supplied length, and also support string containing null character.
		\param s source string. 
		\param length The length of source string, excluding the trailing null terminator.
		\param allocator Allocator for allocating copied buffer. Commonly use GenericDocument::GetAllocator().
		\return The value itself for fluent API.
		\post IsString() == true && GetString() != s && strcmp(GetString(),s) == 0 && GetStringLength() == length
	*/
	GenericValue& SetString(const Ch* s, SizeType length, Allocator& allocator) { this->~GenericValue(); SetStringRaw(StringRef(s, length), allocator); return *this; }

	//! Set this value as a string by copying from source string.
	/*!	\param s source string. 
		\param allocator Allocator for allocating copied buffer. Commonly use GenericDocument::GetAllocator().
		\return The value itself for fluent API.
		\post IsString() == true && GetString() != s && strcmp(GetString(),s) == 0 && GetStringLength() == length
	*/
	GenericValue& SetString(const Ch* s, Allocator& allocator) { return SetString(s, internal::StrLen(s), allocator); }

	//@}

	//! Generate events of this value to a Handler.
	/*! This function adopts the GoF visitor pattern.
		Typical usage is to output this JSON value as JSON text via Writer, which is a Handler.
		It can also be used to deep clone this value via GenericDocument, which is also a Handler.
		\tparam Handler type of handler.
		\param handler An object implementing concept Handler.
	*/
	template <typename Handler>
	bool Accept(Handler& handler) const {
		switch(GetType()) {
		case kNullType:		return handler.Null();
		case kFalseType:	return handler.Bool(false);
		case kTrueType:		return handler.Bool(true);

		case kObjectType:
			if (!handler.StartObject())
				return false;
			for (ConstMemberIterator m = MemberBegin(); m != MemberEnd(); ++m) {
				if (!handler.String(m->name.data_.s.str, m->name.data_.s.length, (m->name.flags_ & kCopyFlag) != 0))
					return false;
				if (!m->value.Accept(handler))
					return false;
			}
			return handler.EndObject(data_.o.size);

		case kArrayType:
			if (!handler.StartArray())
				return false;
			for (GenericValue* v = data_.a.elements; v != data_.a.elements + data_.a.size; ++v)
				if (!v->Accept(handler))
					return false;
			return handler.EndArray(data_.a.size);
	
		case kStringType:
			return handler.String(data_.s.str, data_.s.length, (flags_ & kCopyFlag) != 0);
	
		case kNumberType:
			if (IsInt())			return handler.Int(data_.n.i.i);
			else if (IsUint())		return handler.Uint(data_.n.u.u);
			else if (IsInt64())		return handler.Int64(data_.n.i64);
			else if (IsUint64())	return handler.Uint64(data_.n.u64);
			else					return handler.Double(data_.n.d);
	
		default:
			RAPIDJSON_ASSERT(false);
		}
		return false;
	}

private:
	template <typename, typename>
	friend class GenericDocument;

	enum {
		kBoolFlag = 0x100,
		kNumberFlag = 0x200,
		kIntFlag = 0x400,
		kUintFlag = 0x800,
		kInt64Flag = 0x1000,
		kUint64Flag = 0x2000,
		kDoubleFlag = 0x4000,
		kStringFlag = 0x100000,
		kCopyFlag = 0x200000,

		// Initial flags of different types.
		kNullFlag = kNullType,
		kTrueFlag = kTrueType | kBoolFlag,
		kFalseFlag = kFalseType | kBoolFlag,
		kNumberIntFlag = kNumberType | kNumberFlag | kIntFlag | kInt64Flag,
		kNumberUintFlag = kNumberType | kNumberFlag | kUintFlag | kUint64Flag | kInt64Flag,
		kNumberInt64Flag = kNumberType | kNumberFlag | kInt64Flag,
		kNumberUint64Flag = kNumberType | kNumberFlag | kUint64Flag,
		kNumberDoubleFlag = kNumberType | kNumberFlag | kDoubleFlag,
		kNumberAnyFlag = kNumberType | kNumberFlag | kIntFlag | kInt64Flag | kUintFlag | kUint64Flag | kDoubleFlag,
		kConstStringFlag = kStringType | kStringFlag,
		kCopyStringFlag = kStringType | kStringFlag | kCopyFlag,
		kObjectFlag = kObjectType,
		kArrayFlag = kArrayType,

		kTypeMask = 0xFF	// bitwise-and with mask of 0xFF can be optimized by compiler
	};

	static const SizeType kDefaultArrayCapacity = 16;
	static const SizeType kDefaultObjectCapacity = 16;

	struct String {
		const Ch* str;
		SizeType length;
		unsigned hashcode;	//!< reserved
	};	// 12 bytes in 32-bit mode, 16 bytes in 64-bit mode

	// By using proper binary layout, retrieval of different integer types do not need conversions.
	union Number {
#if RAPIDJSON_ENDIAN == RAPIDJSON_LITTLEENDIAN
		struct I {
			int i;
			char padding[4];
		}i;
		struct U {
			unsigned u;
			char padding2[4];
		}u;
#else
		struct I {
			char padding[4];
			int i;
		}i;
		struct U {
			char padding2[4];
			unsigned u;
		}u;
#endif
		int64_t i64;
		uint64_t u64;
		double d;
	};	// 8 bytes

	struct Object {
		Member* members;
		SizeType size;
		SizeType capacity;
	};	// 12 bytes in 32-bit mode, 16 bytes in 64-bit mode

	struct Array {
		GenericValue* elements;
		SizeType size;
		SizeType capacity;
	};	// 12 bytes in 32-bit mode, 16 bytes in 64-bit mode

	union Data {
		String s;
		Number n;
		Object o;
		Array a;
	};	// 12 bytes in 32-bit mode, 16 bytes in 64-bit mode

	// Initialize this value as array with initial data, without calling destructor.
	void SetArrayRaw(GenericValue* values, SizeType count, Allocator& allocator) {
		flags_ = kArrayFlag;
		data_.a.elements = (GenericValue*)allocator.Malloc(count * sizeof(GenericValue));
		memcpy(data_.a.elements, values, count * sizeof(GenericValue));
		data_.a.size = data_.a.capacity = count;
	}

	//! Initialize this value as object with initial data, without calling destructor.
	void SetObjectRaw(Member* members, SizeType count, Allocator& allocator) {
		flags_ = kObjectFlag;
		data_.o.members = (Member*)allocator.Malloc(count * sizeof(Member));
		memcpy(data_.o.members, members, count * sizeof(Member));
		data_.o.size = data_.o.capacity = count;
	}

	//! Initialize this value as constant string, without calling destructor.
	void SetStringRaw(StringRefType s) {
		flags_ = kConstStringFlag;
		data_.s.str = s;
		data_.s.length = s.length;
	}

	//! Initialize this value as copy string with initial data, without calling destructor.
	void SetStringRaw(StringRefType s, Allocator& allocator) {
		flags_ = kCopyStringFlag;
		data_.s.str = (Ch *)allocator.Malloc((s.length + 1) * sizeof(Ch));
		data_.s.length = s.length;
		memcpy(const_cast<Ch*>(data_.s.str), s, s.length * sizeof(Ch));
		const_cast<Ch*>(data_.s.str)[s.length] = '\0';
	}

	//! Assignment without calling destructor
	void RawAssign(GenericValue& rhs) {
		data_ = rhs.data_;
		flags_ = rhs.flags_;
		rhs.flags_ = kNullFlag;
	}

	Data data_;
	unsigned flags_;
};
#pragma pack (pop)

//! GenericValue with UTF8 encoding
typedef GenericValue<UTF8<> > Value;

///////////////////////////////////////////////////////////////////////////////
// GenericDocument 

//! A document for parsing JSON text as DOM.
/*!
	\note implements Handler concept
	\tparam Encoding encoding for both parsing and string storage.
	\tparam Allocator allocator for allocating memory for the DOM, and the stack during parsing.
	\warning Although GenericDocument inherits from GenericValue, the API does \b not provide any virtual functions, especially no virtual destructors.  To avoid memory leaks, do not \c delete a GenericDocument object via a pointer to a GenericValue.
*/
template <typename Encoding, typename Allocator = MemoryPoolAllocator<> >
class GenericDocument : public GenericValue<Encoding, Allocator> {
public:
	typedef typename Encoding::Ch Ch;						//!< Character type derived from Encoding.
	typedef GenericValue<Encoding, Allocator> ValueType;	//!< Value type of the document.
	typedef Allocator AllocatorType;						//!< Allocator type from template parameter.

	//! Constructor
	/*! \param allocator		Optional allocator for allocating stack memory.
		\param stackCapacity	Initial capacity of stack in bytes.
	*/
	GenericDocument(Allocator* allocator = 0, size_t stackCapacity = kDefaultStackCapacity) : stack_(allocator, stackCapacity), parseResult_() {}

	//!@name Parse from stream
	//!@{

	//! Parse JSON text from an input stream (with Encoding conversion)
	/*! \tparam parseFlags Combination of \ref ParseFlag.
		\tparam SourceEncoding Encoding of input stream
		\tparam InputStream Type of input stream, implementing Stream concept
		\param is Input stream to be parsed.
		\return The document itself for fluent API.
	*/
	template <unsigned parseFlags, typename SourceEncoding, typename InputStream>
	GenericDocument& ParseStream(InputStream& is) {
		ValueType::SetNull(); // Remove existing root if exist
		GenericReader<SourceEncoding, Encoding, Allocator> reader(&GetAllocator());
		ClearStackOnExit scope(*this);
		parseResult_ = reader.template Parse<parseFlags>(is, *this);
		if (parseResult_) {
			RAPIDJSON_ASSERT(stack_.GetSize() == sizeof(ValueType)); // Got one and only one root object
			this->RawAssign(*stack_.template Pop<ValueType>(1));	// Add this-> to prevent issue 13.
		}
		return *this;
	}

	//! Parse JSON text from an input stream
	/*! \tparam parseFlags Combination of \ref ParseFlag.
		\tparam InputStream Type of input stream, implementing Stream concept
		\param is Input stream to be parsed.
		\return The document itself for fluent API.
	*/
	template <unsigned parseFlags, typename InputStream>
	GenericDocument& ParseStream(InputStream& is) {
		return ParseStream<parseFlags,Encoding,InputStream>(is);
	}

	//! Parse JSON text from an input stream (with \ref kParseDefaultFlags)
	/*! \tparam InputStream Type of input stream, implementing Stream concept
		\param is Input stream to be parsed.
		\return The document itself for fluent API.
	*/
	template <typename InputStream>
	GenericDocument& ParseStream(InputStream& is) {
		return ParseStream<kParseDefaultFlags, Encoding, InputStream>(is);
	}
	//!@}

	//!@name Parse in-place from mutable string
	//!@{

	//! Parse JSON text from a mutable string (with Encoding conversion)
	/*! \tparam parseFlags Combination of \ref ParseFlag.
		\tparam SourceEncoding Transcoding from input Encoding
		\param str Mutable zero-terminated string to be parsed.
		\return The document itself for fluent API.
	*/
	template <unsigned parseFlags, typename SourceEncoding>
	GenericDocument& ParseInsitu(Ch* str) {
		GenericInsituStringStream<Encoding> s(str);
		return ParseStream<parseFlags | kParseInsituFlag, SourceEncoding>(s);
	}

	//! Parse JSON text from a mutable string
	/*! \tparam parseFlags Combination of \ref ParseFlag.
		\param str Mutable zero-terminated string to be parsed.
		\return The document itself for fluent API.
	*/
	template <unsigned parseFlags>
	GenericDocument& ParseInsitu(Ch* str) {
		return ParseInsitu<parseFlags, Encoding>(str);
	}

	//! Parse JSON text from a mutable string (with \ref kParseDefaultFlags)
	/*! \param str Mutable zero-terminated string to be parsed.
		\return The document itself for fluent API.
	*/
	GenericDocument& ParseInsitu(Ch* str) {
		return ParseInsitu<kParseDefaultFlags, Encoding>(str);
	}
	//!@}

	//!@name Parse from read-only string
	//!@{

	//! Parse JSON text from a read-only string (with Encoding conversion)
	/*! \tparam parseFlags Combination of \ref ParseFlag (must not contain \ref kParseInsituFlag).
		\tparam SourceEncoding Transcoding from input Encoding
		\param str Read-only zero-terminated string to be parsed.
	*/
	template <unsigned parseFlags, typename SourceEncoding>
	GenericDocument& Parse(const Ch* str) {
		RAPIDJSON_ASSERT(!(parseFlags & kParseInsituFlag));
		GenericStringStream<SourceEncoding> s(str);
		return ParseStream<parseFlags, SourceEncoding>(s);
	}

	//! Parse JSON text from a read-only string
	/*! \tparam parseFlags Combination of \ref ParseFlag (must not contain \ref kParseInsituFlag).
		\param str Read-only zero-terminated string to be parsed.
	*/
	template <unsigned parseFlags>
	GenericDocument& Parse(const Ch* str) {
		return Parse<parseFlags, Encoding>(str);
	}

	//! Parse JSON text from a read-only string (with \ref kParseDefaultFlags)
	/*! \param str Read-only zero-terminated string to be parsed.
	*/
	GenericDocument& Parse(const Ch* str) {
		return Parse<kParseDefaultFlags>(str);
	}
	//!@}

	//!@name Handling parse errors
	//!@{

	//! Whether a parse error has occured in the last parsing.
	bool HasParseError() const { return parseResult_.IsError(); }

	//! Get the \ref ParseErrorCode of last parsing.
	ParseErrorCode GetParseError() const { return parseResult_.Code(); }

	//! Get the position of last parsing error in input, 0 otherwise.
	size_t GetErrorOffset() const { return parseResult_.Offset(); }

	//!@}

	//! Get the allocator of this document.
	Allocator& GetAllocator() {	return stack_.GetAllocator(); }

	//! Get the capacity of stack in bytes.
	size_t GetStackCapacity() const { return stack_.GetCapacity(); }

private:
	// clear stack on any exit from ParseStream, e.g. due to exception
	struct ClearStackOnExit {
		explicit ClearStackOnExit(GenericDocument& d) : d_(d) {}
		~ClearStackOnExit() { d_.ClearStack(); }
	private:
		ClearStackOnExit(const ClearStackOnExit&);
		ClearStackOnExit& operator=(const ClearStackOnExit&);
		GenericDocument& d_;
	};

	// callers of the following private Handler functions
	template <typename,typename,typename> friend class GenericReader; // for parsing
	friend class GenericValue<Encoding,Allocator>; // for deep copying

	// Implementation of Handler
	bool Null()	{ new (stack_.template Push<ValueType>()) ValueType(); return true; }
	bool Bool(bool b) { new (stack_.template Push<ValueType>()) ValueType(b); return true; }
	bool Int(int i) { new (stack_.template Push<ValueType>()) ValueType(i); return true; }
	bool Uint(unsigned i) { new (stack_.template Push<ValueType>()) ValueType(i); return true; }
	bool Int64(int64_t i) { new (stack_.template Push<ValueType>()) ValueType(i); return true; }
	bool Uint64(uint64_t i) { new (stack_.template Push<ValueType>()) ValueType(i); return true; }
	bool Double(double d) { new (stack_.template Push<ValueType>()) ValueType(d); return true; }

	bool String(const Ch* str, SizeType length, bool copy) { 
		if (copy) 
			new (stack_.template Push<ValueType>()) ValueType(str, length, GetAllocator());
		else
			new (stack_.template Push<ValueType>()) ValueType(str, length);
		return true;
	}

	bool StartObject() { new (stack_.template Push<ValueType>()) ValueType(kObjectType); return true; }
	
	bool EndObject(SizeType memberCount) {
		typename ValueType::Member* members = stack_.template Pop<typename ValueType::Member>(memberCount);
		stack_.template Top<ValueType>()->SetObjectRaw(members, (SizeType)memberCount, GetAllocator());
		return true;
	}

	bool StartArray() { new (stack_.template Push<ValueType>()) ValueType(kArrayType); return true; }
	
	bool EndArray(SizeType elementCount) {
		ValueType* elements = stack_.template Pop<ValueType>(elementCount);
		stack_.template Top<ValueType>()->SetArrayRaw(elements, elementCount, GetAllocator());
		return true;
	}

private:
	//! Prohibit assignment
	GenericDocument& operator=(const GenericDocument&);

	void ClearStack() {
		if (Allocator::kNeedFree)
			while (stack_.GetSize() > 0)	// Here assumes all elements in stack array are GenericValue (Member is actually 2 GenericValue objects)
				(stack_.template Pop<ValueType>(1))->~ValueType();
		else
			stack_.Clear();
	}

	static const size_t kDefaultStackCapacity = 1024;
	internal::Stack<Allocator> stack_;
	ParseResult parseResult_;
};

//! GenericDocument with UTF8 encoding
typedef GenericDocument<UTF8<> > Document;

// defined here due to the dependency on GenericDocument
template <typename Encoding, typename Allocator>
template <typename SourceAllocator>
inline
GenericValue<Encoding,Allocator>::GenericValue(const GenericValue<Encoding,SourceAllocator>& rhs, Allocator& allocator)
{
	GenericDocument<Encoding,Allocator> d(&allocator);
	rhs.Accept(d);
	RawAssign(*d.stack_.template Pop<GenericValue>(1));
}

} // namespace rapidjson

#if defined(_MSC_VER) || defined(__GNUC__)
RAPIDJSON_DIAG_POP
#endif

#endif // RAPIDJSON_DOCUMENT_H_
