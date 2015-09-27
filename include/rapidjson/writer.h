// Tencent is pleased to support the open source community by making RapidJSON available.
//
// Copyright (C) 2015 THL A29 Limited, a Tencent company, and Milo Yip. All rights reserved.
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

#ifndef RAPIDJSON_WRITER_H_
#define RAPIDJSON_WRITER_H_

#include "rapidjson.h"
#include "writerbase.h"
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

#ifdef __GNUC__
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(effc++)
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
template<typename OUTPUTSTREAM,
         typename SOURCEENCODING = UTF8<>,
         typename TARGETENCODING = UTF8<>,
         typename STACKALLOCATOR = CrtAllocator>
class Writer : public WriterBase<Writer<OUTPUTSTREAM, SOURCEENCODING, TARGETENCODING, STACKALLOCATOR>,
                                 OUTPUTSTREAM,
                                 SOURCEENCODING,
                                 TARGETENCODING,
                                 STACKALLOCATOR >
{
public:
    typedef WriterBase<Writer<OUTPUTSTREAM, SOURCEENCODING, TARGETENCODING, STACKALLOCATOR>,
                       OUTPUTSTREAM,
                       SOURCEENCODING,
                       TARGETENCODING,
                       STACKALLOCATOR> Base;
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
    Writer(OutputStream& os, StackAllocator* stackAllocator = 0, size_t levelDepth = Base::kDefaultLevelDepth)
      : Base(os, stackAllocator, levelDepth)
    {}

    //! Constructor.
    /*! \param stackAllocator User supplied allocator. If it is null, it will create a private one.
        \param levelDepth Initial capacity of stack.
    */
    explicit
    Writer(StackAllocator* allocator = 0, size_t levelDepth = Base::kDefaultLevelDepth) :
      Base(allocator, levelDepth)
    {}

protected:
    friend class WriterBase<Writer<OUTPUTSTREAM, SOURCEENCODING, TARGETENCODING, STACKALLOCATOR>, OUTPUTSTREAM, SOURCEENCODING, TARGETENCODING, STACKALLOCATOR >;

    using Base::level_stack_;
    using Base::os_;
    using Base::hasRoot_;

    // Close our block.
    void CloseBlock()
    {
      level_stack_.template Pop<typename Base::Level>(1);
    }

    // Prefix.
    void Prefix(Type type) {
        (void)type;
        if (level_stack_.GetSize() != 0) { // this value is not at root
            typename Base::Level* level = level_stack_.template Top<typename Base::Level>();
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
            RAPIDJSON_ASSERT(!hasRoot_);    // Should only has one and only one root.
            hasRoot_ = true;
        }
    }

private:
    // Prohibit copy constructor & assignment operator.
    Writer(const Writer&);
    Writer& operator=(const Writer&);
};

// Full specialization for StringStream to prevent memory copying

template<>
inline bool WriterBase<Writer<StringBuffer>, StringBuffer>::WriteInt(int i) {
    char *buffer = os_->Push(11);
    const char* end = internal::i32toa(i, buffer);
    os_->Pop(static_cast<size_t>(11 - (end - buffer)));
    return true;
}

template<>
inline bool WriterBase<Writer<StringBuffer>, StringBuffer>::WriteUint(unsigned u) {
    char *buffer = os_->Push(10);
    const char* end = internal::u32toa(u, buffer);
    os_->Pop(static_cast<size_t>(10 - (end - buffer)));
    return true;
}

template<>
inline bool WriterBase<Writer<StringBuffer>, StringBuffer>::WriteInt64(int64_t i64) {
    char *buffer = os_->Push(21);
    const char* end = internal::i64toa(i64, buffer);
    os_->Pop(static_cast<size_t>(21 - (end - buffer)));
    return true;
}

template<>
inline bool WriterBase<Writer<StringBuffer>, StringBuffer>::WriteUint64(uint64_t u) {
    char *buffer = os_->Push(20);
    const char* end = internal::u64toa(u, buffer);
    os_->Pop(static_cast<size_t>(20 - (end - buffer)));
    return true;
}

template<>
inline bool WriterBase<Writer<StringBuffer>, StringBuffer>::WriteDouble(double d) {
    char *buffer = os_->Push(25);
    char* end = internal::dtoa(d, buffer);
    os_->Pop(static_cast<size_t>(25 - (end - buffer)));
    return true;
}

RAPIDJSON_NAMESPACE_END

#ifdef _MSC_VER
RAPIDJSON_DIAG_POP
#endif

#ifdef __GNUC__
RAPIDJSON_DIAG_POP
#endif

#endif // RAPIDJSON_RAPIDJSON_H_
