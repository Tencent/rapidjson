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

#ifndef RAPIDJSON_MEMORYBUFFER_H_
#define RAPIDJSON_MEMORYBUFFER_H_

#include "stream.h"
#include "internal/stack.h"

RAPIDJSON_NAMESPACE_BEGIN

//! Represents an in-memory output byte stream.
/*!
    This class is mainly for being wrapped by EncodedOutputStream or AutoUTFOutputStream.

    It is similar to FileWriteBuffer but the destination is an in-memory buffer instead of a file.

    Differences between MemoryBuffer and StringBuffer:
    1. StringBuffer has Encoding but MemoryBuffer is only a byte buffer. 
    2. StringBuffer::GetString() returns a null-terminated string. MemoryBuffer::GetBuffer() returns a buffer without terminator.

    \tparam Allocator type for allocating memory buffer.
    \note implements Stream concept
*/
template <typename Allocator = CrtAllocator>
struct GenericMemoryBuffer {
    typedef char Ch; // byte

    GenericMemoryBuffer(Allocator* allocator = 0, size_t capacity = kDefaultCapacity) : stack_(allocator, capacity), overflowed_(false) {}

    void Put(Ch c) { if (Ch* p = Push(1)) *p = c; }
    void Flush() {}

    void Clear() { stack_.Clear(); overflowed_ = false; }
    void ShrinkToFit() { stack_.ShrinkToFit(); }

    //! Append \c count bytes and return them for writing.
    /*! \return NULL if the allocation fails. Overflowed() is then true. */
    Ch* Push(size_t count) {
        Ch* p = RAPIDJSON_LIKELY(!overflowed_) ? stack_.template TryPush<Ch>(count) : 0;
        if (RAPIDJSON_UNLIKELY(!p))
            overflowed_ = true;
        return p;
    }
    void Pop(size_t count) { stack_.template Pop<Ch>(count); }

    //! True if an allocation failed since construction or the last Clear().
    /*! The content is then incomplete: bytes put after the failure were dropped. */
    bool Overflowed() const { return overflowed_; }

    const Ch* GetBuffer() const {
        return stack_.template Bottom<Ch>();
    }

    size_t GetSize() const { return stack_.GetSize(); }

    static const size_t kDefaultCapacity = 256;
    mutable internal::Stack<Allocator> stack_;
    bool overflowed_;
};

typedef GenericMemoryBuffer<> MemoryBuffer;

//! Implement specialized version of PutN() with memset() for better performance.
template<>
inline void PutN(MemoryBuffer& memoryBuffer, char c, size_t n) {
    if (char* p = memoryBuffer.Push(n))
        std::memset(p, c, n * sizeof(c));
}

RAPIDJSON_NAMESPACE_END

#endif // RAPIDJSON_MEMORYBUFFER_H_
