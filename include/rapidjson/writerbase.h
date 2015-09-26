// Tencent is pleased to support the open source community by making RapidJSON available.
//
// Copyright (C) 2015 THL A29 Limited, a Tencent company, and Milo Yip. All rights reserved.
//
// Additional modifcations, Copyright (C) 2015, Bruno Nicoletti. All rights reserved.
//
// Licensed under the MIT License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// http://opensource.org/licenses/MIT
//
// Unless required by applicable law or agreed to in writing, software distributed
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

#ifndef RAPIDJSON_WRITERBASE_H_
#define RAPIDJSON_WRITERBASE_H_

#include "rapidjson.h"
#include "internal/stack.h"
#include "internal/strfunc.h"
#include "internal/dtoa.h"
#include "internal/itoa.h"
#include "stringbuffer.h"
#include <new>      // placement new

#if RAPIDJSON_HAS_STDSTRING
#include <string>
#endif

#ifdef _MSC_VER
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(4127) // conditional expression is constant
#endif

RAPIDJSON_NAMESPACE_BEGIN

//! JSON writer
/*! Writer implements the concept Handler.
    It generates JSON text by events to an output os.

    User may programmatically calls the functions of a writer to generate JSON text.

    On the other side, a writer can also be passed to objects that generates events,

    for example Reader::Parse() and Document::Accept().

    \tparam OutputStream Type of output stream.
    \tparam SourceEncoding Encoding of source string.
    \tparam TargetEncoding Encoding of output stream.
    \tparam StackAllocator Type of allocator for allocating memory of stack.
    \note implements Handler concept
*/
template<class DERIVED,
         typename OUTPUTSTREAM,
         typename SOURCEENCODING = UTF8<>,
         typename TARGETENCODING = UTF8<>,
         typename STACKALLOCATOR = CrtAllocator>
class WriterBase {
public:
    typedef DERIVED Derived;
    typedef OUTPUTSTREAM OutputStream;
    typedef SOURCEENCODING SourceEncoding;
    typedef TARGETENCODING TargetEncoding;
    typedef STACKALLOCATOR StackAllocator;
    typedef typename SourceEncoding::Ch Ch;

    //! Constructor
    /*! \param os Output stream.
        \param stackAllocator User supplied allocator. If it is null, it will create a private one.
        \param levelDepth Initial capacity of stack.
    */
    explicit
    WriterBase(OutputStream& os, StackAllocator* stackAllocator = 0, size_t levelDepth = kDefaultLevelDepth)
      : os_(&os)
      , level_stack_(stackAllocator, levelDepth * sizeof(Level))
      , hasRoot_(false)
#ifdef RAPIDJSON_UNIT_TEST_STRING_LITERALS
      , lastStringCallWasToStringLiteralVersion_(false)
#endif
  {}

    explicit
    WriterBase(StackAllocator* allocator = 0, size_t levelDepth = kDefaultLevelDepth)
      : os_(0)
      , level_stack_(allocator, levelDepth * sizeof(Level))
      , hasRoot_(false)
#ifdef RAPIDJSON_UNIT_TEST_STRING_LITERALS
      , lastStringCallWasToStringLiteralVersion_(false)
#endif
  {}

    //! Reset the writer with a new stream.
    /*!
        This function reset the writer with a new stream and default settings,
        in order to make a WriterBase object reusable for output multiple JSONs.

        \param os New output stream.
        \code
        WriterBase<OutputStream> writer(os1);
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

    /// \brief Output a 'null' to the JSON stream.
    ///
    ///  \return Whether it is succeed.
    bool Null()
    {
       DerivedPrefix(kNullType);
       return WriteNull();
    }

    /// \brief Output the given bool value to the JSON stream.
    bool Bool(bool b)
    {
       DerivedPrefix(b ? kTrueType : kFalseType);
       return WriteBool(b);
    }

    /// \brief Output the given integer value to the JSON stream.
    ///
    ///  \return Whether it is succeed.
    bool Int(int i)
    {
      DerivedPrefix(kNumberType);
      return WriteInt(i);
    }

    /// \brief Output the given unsigned integer value to the JSON stream.
    ///
    ///  \return Whether it is succeed.
    bool Uint(unsigned u)
    {
      DerivedPrefix(kNumberType);
      return WriteUint(u);
    }

    /// \brief Output the given signed 64 bit integer value to the JSON stream.
    ///
    ///  \return Whether it is succeed.
    bool Int64(int64_t i64)
    {
      DerivedPrefix(kNumberType);
      return WriteInt64(i64);
    }

    /// \brief Output the given unsigned 64 bit integer value to the JSON stream.
    ///
    ///  \return Whether it is succeed.
    bool Uint64(uint64_t u64)
    {
      DerivedPrefix(kNumberType);
      return WriteUint64(u64);
    }

    /// \brief Output the given \c double value to the JSON stream.
    ///
    ///  \return Whether it is succeed.
    bool Double(double d)
    {
      DerivedPrefix(kNumberType);
      return WriteDouble(d);
    }

    /// \brief Output the given value string of the given length value to the JSON stream.
    ///
    ///  \return Whether it is succeed.
    inline bool String(const Ch* str, SizeType length, bool copy = false)
    {
      (void)copy;
      DerivedPrefix(kStringType);
      return WriteString(str, length);
    }

    /// \brief Output the given string literal as a key to the JSON stream.
    ///
    /// This overload will capture string literals, which have thier length known at
    /// compile time.
    ///
    ///  \return Whether it is succeed.
    template<typename CHAR, int N>
    inline bool String(const CHAR(&str)[N])
    {
#ifdef RAPIDJSON_UNIT_TEST_STRING_LITERALS
      lastStringCallWasToStringLiteralVersion_ = true;
#endif
      return String(str, N-1);
    }

    /// \brief Output the given string as a key to the JSON stream. The length is determined by strlen.
    ///
    /// The template magic is needed to differentiate it from the string literal version.
    ///
    ///  \return Whether it is succeed.
    template<typename CHAR>
    inline bool String(const CHAR* const& str)
    {
#ifdef RAPIDJSON_UNIT_TEST_STRING_LITERALS
      lastStringCallWasToStringLiteralVersion_ = false;
#endif
      return String(str, internal::StrLen(str));
    }

#if RAPIDJSON_HAS_STDSTRING
    /// \brief Output the given string to the JSON stream. The length is determined by strlen.
    ///
    ///  \return Whether it is succeed.
    inline bool String(const std::string &str)
    {
#ifdef RAPIDJSON_UNIT_TEST_STRING_LITERALS
      lastStringCallWasToStringLiteralVersion_ = false;
#endif
      return String(str.data(), str.size());
    }
#endif

    /// \brief Output the given key of the given length value to the JSON stream.
    ///
    ///  \return Whether it is succeed.
    bool Key(const Ch* str, SizeType length, bool copy = false)
    {
      return String(str, length, copy);
    }

    /// \brief Output the given string literal as a key to the JSON stream.
    ///
    /// This overload will capture string literals, which have thier length known at
    /// compile time.
    ///
    ///  \return Whether it is succeed.
    template<typename T, int N>
    bool Key(const T(&str)[N])
    {
      return String(str, N-1);
    }

    /// \brief Output the given string as a key to the JSON stream. The length is determined by strlen.
    ///
    /// The template magic is needed to differentiate it from the string literal version.
    ///
    ///  \return Whether it is succeed.
    template<typename T>
    bool Key(const T* const& str)
    {
      return Key(str, internal::StrLen(str));
    }

#if RAPIDJSON_HAS_STDSTRING
    /// \brief Output the given string as a key to the JSON stream. The length is determined by strlen.
    ///
    ///  \return Whether it is succeed.
    bool Key(const std::string &str)
    {
      return String(str.data(), str.size());
    }
#endif

    /// \brief Start a JSON compound object on the stream.
    ///
    ///  \return Whether it is succeed.
    bool StartObject()
    {
      DerivedPrefix(kObjectType);
      new (level_stack_.template Push<Level>()) Level(false);
      return WriteStartObject();
    }

    /// \brief End the currently open JSON compound object on the stream.
    ///
    ///  \return Whether it is succeed.
    bool EndObject(SizeType memberCount = 0)
    {
        (void)memberCount;
        RAPIDJSON_ASSERT(level_stack_.GetSize() >= sizeof(Level));
        RAPIDJSON_ASSERT(!level_stack_.template Top<Level>()->inArray);
        DerivedCloseBlock();
        bool ret = WriteEndObject();
        if (level_stack_.Empty())   // end of json text
            os_->Flush();
        return ret;
    }

    /// \brief Start a JSON array on the stream.
    ///
    ///  \return Whether it is succeed.
    bool StartArray() {
        DerivedPrefix(kArrayType);
        new (level_stack_.template Push<Level>()) Level(true);
        return WriteStartArray();
    }

    /// \brief End the currently open JSON array object on the stream.
    ///
    ///  \return Whether it is succeed.
    bool EndArray(SizeType elementCount = 0)
    {
        (void)elementCount;
        RAPIDJSON_ASSERT(level_stack_.GetSize() >= sizeof(Level));
        RAPIDJSON_ASSERT(level_stack_.template Top<Level>()->inArray);
        DerivedCloseBlock();
        bool ret = WriteEndArray();
        if (level_stack_.Empty())   // end of json text
            os_->Flush();
        return ret;
    }

    /// Functor to output the given key with a bool value
    template<typename STRING>
    bool operator()(STRING &key, bool b)
    {
      return Key(key) and Bool(b);
    }

    /// Functor to output the given key with an int value
    template<typename STRING>
    bool operator()(STRING &key, int i)
    {
      return Key(key) and Int(i);
    }

    /// Functor to output the given key with an unsigned int value
    template<typename STRING>
    bool operator()(STRING &key, unsigned u)
    {
      return Key(key) and Uint(u);
    }

    /// Functor to output the given key with an 64 bit int value
    template<typename STRING>
    bool operator()(STRING &key, int64_t i64)
    {
      return Key(key) and Int64(i64);
    }

    /// Functor to output the given key with an 64 bit int value.
    template<typename STRING>
    bool operator()(STRING &key, uint64_t u64)
    {
      return Key(key) and Uint64(u64);
    }

    /// Functor to output the given key with an double value.
    template<typename STRING>
    bool operator()(STRING &key, double d)
    {
      return Key(key) and Double(d);
    }

    /// Functor to output the given key with a string.
    template<typename STRING1, typename STRING2>
    bool operator()(STRING1 &key, STRING2 &s)
    {
      return Key(key) and String(s);
    }

#ifdef RAPIDJSON_UNIT_TEST_STRING_LITERALS
    bool lastStringCallWasToStringLiteralVersion() const
    {
      return lastStringCallWasToStringLiteralVersion_;
    }
#endif

protected:
    //! Information for each nested level
    struct Level {
        Level(bool inArray_) : valueCount(0), inArray(inArray_) {}
        size_t valueCount;  //!< number of values in this level
        bool inArray;       //!< true if in array, otherwise in object
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
        char* end = internal::u64toa(u64, buffer);
        for (char* p = buffer; p != end; ++p)
            os_->Put(*p);
        return true;
    }

    bool WriteDouble(double d) {
        char buffer[25];
        char* end = internal::dtoa(d, buffer);
        for (char* p = buffer; p != end; ++p)
            os_->Put(*p);
        return true;
    }

    bool WriteString(const Ch* str, SizeType length)  {
        static const char hexDigits[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
        static const char escape[256] = {
#define Z16 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
            //0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
            'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'b', 't', 'n', 'u', 'f', 'r', 'u', 'u', // 00
            'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', // 10
              0,   0, '"',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 20
            Z16, Z16,                                                                       // 30~4F
              0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,'\\',   0,   0,   0, // 50
            Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16                                // 60~FF
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
                else {
                    RAPIDJSON_ASSERT(codepoint >= 0x010000 && codepoint <= 0x10FFFF);
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
                if (!Transcoder<SourceEncoding, TargetEncoding>::Transcode(is, *os_))
                    return false;
        }
        os_->Put('\"');
        return true;
    }

    bool WriteStartObject() { os_->Put('{'); return true; }
    bool WriteEndObject()   { os_->Put('}'); return true; }
    bool WriteStartArray()  { os_->Put('['); return true; }
    bool WriteEndArray()    { os_->Put(']'); return true; }

    // Calls the Prefix function in derived version of this.
    void DerivedPrefix(Type type)
    {
      static_cast<Derived *>(this)->Prefix(type);
    }

    // Calls the Prefix function in derived version of this.
    void DerivedCloseBlock()
    {
      static_cast<Derived *>(this)->CloseBlock();
    }

    OutputStream* os_;
    internal::Stack<StackAllocator> level_stack_;
    bool hasRoot_;

#ifdef RAPIDJSON_UNIT_TEST_STRING_LITERALS
    bool lastStringCallWasToStringLiteralVersion_;
#endif

private:
    // Prohibit copy constructor & assignment operator.
    WriterBase(const WriterBase&);
    WriterBase& operator=(const WriterBase&);
};

RAPIDJSON_NAMESPACE_END

#ifdef _MSC_VER
RAPIDJSON_DIAG_POP
#endif

#endif // RAPIDJSON_RAPIDJSON_H_
