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

#ifndef RAPIDJSON_BUFFEREDOSTREAMWRAPPER_H_
#define RAPIDJSON_BUFFEREDOSTREAMWRAPPER_H_

#include <iosfwd>

#include "writer.h"

#if defined(__clang__)
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(c++98-compat)
#endif

RAPIDJSON_NAMESPACE_BEGIN

template<typename Stream = std::ostream, typename Allocator = CrtAllocator>
class BufferedOStreamWrapper {
public:
    typedef typename Stream::char_type Ch;
    explicit BufferedOStreamWrapper(Stream &stream, Allocator *allocator = nullptr)
            : stream_(stream), allocator_(allocator) {}

    ~BufferedOStreamWrapper() {
        Flush();

        Allocator::Free(top_);
        RAPIDJSON_DELETE(ownAllocator_);
    }

    BufferedOStreamWrapper(const BufferedOStreamWrapper &) = delete;
    BufferedOStreamWrapper &operator=(const BufferedOStreamWrapper &) = delete;

    void Put(Ch c) {
        ReserveMore(1);
        PutUnsafe(c);
    }

    void PutUnsafe(Ch c) {
        *end_ = c;
        ++end_;
    }

    Ch *Push(size_t cnt) {
        ReserveMore(cnt);
        auto *const res = end_;
        end_ += cnt;
        return res;
    }

    // Can pop only as many elements as was pushed the last time
    void Pop(size_t cnt) { end_ -= cnt; }

    void Flush() {
        FlushInternal();
        stream_.flush();
    }

    // Not implemented
    [[nodiscard]] char Peek() const {
        RAPIDJSON_ASSERT(false);
        return 0;
    }

    char Take() {
        RAPIDJSON_ASSERT(false);
        return 0;
    }

    [[nodiscard]] size_t Tell() const {
        RAPIDJSON_ASSERT(false);
        return 0;
    }
    char *PutBegin() {
        RAPIDJSON_ASSERT(false);
        return nullptr;
    }
    size_t PutEnd(char *) {
        RAPIDJSON_ASSERT(false);
        return 0;
    }

    void ReserveMore(size_t extra_cnt) {
        if (RAPIDJSON_LIKELY(GetBufLeftCapacity() >= extra_cnt))
            return;

        const size_t cur_capacity = GetBufCapacity();

        if (cur_capacity >= kMaxBufSize) {
            FlushInternal();
            if (extra_cnt > cur_capacity) {
                Resize(extra_cnt);
            }
        } else if (top_ == nullptr) {
            FirstAllocMemory(extra_cnt);
        } else {
            Resize(extra_cnt);
        }
    }

private:
    static const size_t kMaxBufSize = 1024 * 1;
    static const size_t kInitialBufSize = 64;
    Stream &stream_;

    Ch *top_ = nullptr;
    Ch *end_ = nullptr;
    Ch *capacity_end_ = nullptr;

    Allocator *allocator_ = nullptr;
    Allocator *ownAllocator_ = nullptr;

    void ClearBuf() { end_ = top_; }

    void FirstAllocMemory(size_t cnt) {
        if (allocator_ == nullptr) {
            ownAllocator_ = allocator_ = RAPIDJSON_NEW(Allocator());
        }

        size_t capacity = kInitialBufSize;
        while (capacity < cnt) {
            capacity *= 2;
        }

        end_ = top_ =
                reinterpret_cast<Ch *>(allocator_->Malloc(capacity * sizeof(Ch)));
        capacity_end_ = top_ + capacity;
    }

    void Resize(size_t extra_cnt) {
        const size_t cur_size = GetBufSize();
        const size_t cur_cap = GetBufCapacity();
        size_t new_cap = cur_cap;
        const size_t required_cap = cur_size + extra_cnt;
        do {
            new_cap *= 2;
        } while (required_cap > new_cap);

        top_ = reinterpret_cast<Ch *>(allocator_->Realloc(top_, cur_cap, new_cap));
        end_ = top_ + cur_size;
        capacity_end_ = top_ + new_cap;
    }

    size_t GetBufLeftCapacity() const {
        return static_cast<size_t>(capacity_end_ - end_) / sizeof(Ch);
    }
    size_t GetBufCapacity() const {
        return static_cast<size_t>(capacity_end_ - top_) / sizeof(Ch);
    }
    size_t GetBufSize() const {
        return static_cast<size_t>(end_ - top_) / sizeof(Ch);
    }

    void FlushInternal() {
        stream_.write(top_, static_cast<std::streamsize>(GetBufSize()));
        ClearBuf();
    }
};

template<typename Allocator>
inline void PutReserve(BufferedOStreamWrapper<Allocator> &stream, size_t count) {
    stream.ReserveMore(count);
}

template<typename Allocator>
inline void PutUnsafe(BufferedOStreamWrapper<Allocator> &stream,
                      typename BufferedOStreamWrapper<Allocator>::Ch c) {
    stream.PutUnsafe(c);
}

//! Implement specialized version of PutN() with memset() for better
//! performance.
template<typename Allocator>
inline void PutN(BufferedOStreamWrapper<Allocator> &stream,
                 typename BufferedOStreamWrapper<Allocator>::Ch c, size_t n) {
    std::memset(stream.Push(n), c, n * sizeof(c));
}

// Full specialization for Writer<BufferedOStreamWrapper<>> to prevent memory copying

template<>
inline bool Writer<BufferedOStreamWrapper<>>::WriteInt(int i) {
    return internal::WriteIntToStream(i, os_);
}

template<>
inline bool Writer<BufferedOStreamWrapper<>>::WriteUint(unsigned u) {
    return internal::WriteUintToStream(u, os_);
}

template<>
inline bool Writer<BufferedOStreamWrapper<>>::WriteInt64(int64_t i64) {
    return internal::WriteInt64ToStream(i64, os_);
}

template<>
inline bool Writer<BufferedOStreamWrapper<>>::WriteUint64(uint64_t u) {
    return internal::WriteUint64ToStream(u, os_);
}

template<>
inline bool Writer<BufferedOStreamWrapper<>>::WriteDouble(double d) {
    return internal::WriteDoubleToStream(d, os_, maxDecimalPlaces_);
}

RAPIDJSON_NAMESPACE_END

#if defined(__clang__)
RAPIDJSON_DIAG_POP
#endif

#endif // RAPIDJSON_BUFFEREDOSTREAMWRAPPER_H_
