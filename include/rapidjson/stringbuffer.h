// Tencent is pleased to support the open source community by making RapidJSON available.
// 
// Copyright (C) 2015 THL A29 Limited, a Tencent company, and Milo Yip.
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

#ifndef RAPIDJSON_STRINGBUFFER_H_
#define RAPIDJSON_STRINGBUFFER_H_

#include "stream.h"
#include "internal/stack.h"

#if RAPIDJSON_HAS_CXX11_RVALUE_REFS
#include <utility> // std::move
#endif

#include "internal/stack.h"

#if defined(__clang__)
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(c++98-compat)
#endif

RAPIDJSON_NAMESPACE_BEGIN

//! Represents an in-memory output stream.
/*!
    \tparam Encoding Encoding of the stream.
    \tparam Allocator type for allocating memory buffer.
    \note implements Stream concept
*/
template <typename Encoding, typename Allocator = CrtAllocator>
class GenericStringBuffer {
public:
    typedef typename Encoding::Ch Ch;

    GenericStringBuffer(Allocator* allocator = 0, size_t capacity = kDefaultCapacity) : stack_(allocator, capacity), overflowed_(false) {}

#if RAPIDJSON_HAS_CXX11_RVALUE_REFS
    GenericStringBuffer(GenericStringBuffer&& rhs) : stack_(std::move(rhs.stack_)), overflowed_(rhs.overflowed_) {}
    GenericStringBuffer& operator=(GenericStringBuffer&& rhs) {
        if (&rhs != this) {
            stack_ = std::move(rhs.stack_);
            overflowed_ = rhs.overflowed_;
        }
        return *this;
    }
#endif

    void Put(Ch c) { if (Ch* p = Push(1)) *p = c; }
    void PutUnsafe(Ch c) { if (RAPIDJSON_LIKELY(!overflowed_)) *stack_.template PushUnsafe<Ch>() = c; }
    void Flush() {}

    void Clear() { stack_.Clear(); overflowed_ = false; }
    void ShrinkToFit() {
        // Push and pop a null terminator. This is safe.
        Ch* p = stack_.template TryPush<Ch>();
        if (!p)
            return;
        *p = '\0';
        stack_.ShrinkToFit();
        stack_.template Pop<Ch>(1);
    }

    //! Reserve room for \c count characters, to be written with PutUnsafe() or PushUnsafe().
    /*! If the allocation fails, Overflowed() becomes true. */
    void Reserve(size_t count) {
        // One more for the null terminator of GetString().
        if (RAPIDJSON_UNLIKELY(overflowed_ || !stack_.template TryReserve<Ch>(count + 1)))
            overflowed_ = true;
    }

    //! Append \c count characters and return them for writing.
    /*! \return NULL if the allocation fails. Overflowed() is then true. */
    Ch* Push(size_t count) {
        Reserve(count);
        return RAPIDJSON_LIKELY(!overflowed_) ? stack_.template PushUnsafe<Ch>(count) : 0;
    }

    //! Append \c count characters reserved with Reserve().
    /*! \return NULL if Overflowed(). */
    Ch* PushUnsafe(size_t count) { return RAPIDJSON_LIKELY(!overflowed_) ? stack_.template PushUnsafe<Ch>(count) : 0; }
    void Pop(size_t count) { stack_.template Pop<Ch>(count); }

    const Ch* GetString() const {
        // Push and pop a null terminator. This is safe.
        Ch* p = stack_.template TryPush<Ch>();
        if (!p) { // Allocation failed, only possible if nothing was ever put.
            static const Ch empty = '\0';
            return &empty;
        }
        *p = '\0';
        stack_.template Pop<Ch>(1);

        return stack_.template Bottom<Ch>();
    }

    //! True if an allocation failed since construction or the last Clear().
    /*! The content is then incomplete: characters put after the failure were dropped. */
    bool Overflowed() const { return overflowed_; }

    //! Get the size of string in bytes in the string buffer.
    size_t GetSize() const { return stack_.GetSize(); }

    //! Get the length of string in Ch in the string buffer.
    size_t GetLength() const { return stack_.GetSize() / sizeof(Ch); }

    static const size_t kDefaultCapacity = 256;
    mutable internal::Stack<Allocator> stack_;

private:
    bool overflowed_;

    // Prohibit copy constructor & assignment operator.
    GenericStringBuffer(const GenericStringBuffer&);
    GenericStringBuffer& operator=(const GenericStringBuffer&);
};

//! String buffer with UTF8 encoding
typedef GenericStringBuffer<UTF8<> > StringBuffer;

template<typename Encoding, typename Allocator>
inline void PutReserve(GenericStringBuffer<Encoding, Allocator>& stream, size_t count) {
    stream.Reserve(count);
}

template<typename Encoding, typename Allocator>
inline void PutUnsafe(GenericStringBuffer<Encoding, Allocator>& stream, typename Encoding::Ch c) {
    stream.PutUnsafe(c);
}

//! Implement specialized version of PutN() with memset() for better performance.
template<>
inline void PutN(GenericStringBuffer<UTF8<> >& stream, char c, size_t n) {
    if (char* p = stream.Push(n))
        std::memset(p, c, n * sizeof(c));
}

RAPIDJSON_NAMESPACE_END

#if defined(__clang__)
RAPIDJSON_DIAG_POP
#endif

#endif // RAPIDJSON_STRINGBUFFER_H_
