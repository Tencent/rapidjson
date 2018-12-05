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

#ifndef RAPIDJSON_ISTREAMWRAPPER_H_
#define RAPIDJSON_ISTREAMWRAPPER_H_

#include "stream.h"
#include <iosfwd>
#include <cstring>

#ifdef __clang__
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(padded)
#elif defined(_MSC_VER)
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(4351) // new behavior: elements of array 'array' will be default initialized
#endif

RAPIDJSON_NAMESPACE_BEGIN

//! Wrapper of \c std::basic_istream into RapidJSON's Stream concept.
/*!
    The classes can be wrapped including but not limited to:

    - \c std::istringstream
    - \c std::stringstream
    - \c std::wistringstream
    - \c std::wstringstream
    - \c std::ifstream
    - \c std::fstream
    - \c std::wifstream
    - \c std::wfstream

    \tparam StreamType Class derived from \c std::basic_istream.
*/
   
template <typename StreamType>
class BasicIStreamWrapper {
public:
    typedef typename StreamType::char_type Ch;

    BasicIStreamWrapper(StreamType& stream) : stream_(stream), buffer_(peekBuffer_), size_(sizeof(peekBuffer_) / sizeof(Ch)), pos_(), len_(), count_() {}

    BasicIStreamWrapper(StreamType& stream, Ch *buffer, size_t size) : stream_(stream), buffer_(buffer), size_(size), pos_(), len_(), count_() {
        RAPIDJSON_ASSERT(buffer_ != 0 && static_cast<std::streamsize>(size_) > 0);
        if (RAPIDJSON_UNLIKELY(size_ < sizeof(peekBuffer_) / sizeof(Ch))) {
            size_ = sizeof(peekBuffer_) / sizeof(Ch);
            buffer_ = peekBuffer_;
        }
    }

    Ch Peek() const {
        if (RAPIDJSON_UNLIKELY(pos_ == len_) && !Read())
            return static_cast<Ch>('\0');
        return buffer_[pos_];
    }

    Ch Take() {
        if (RAPIDJSON_UNLIKELY(pos_ == len_) && !Read())
            return static_cast<Ch>('\0');
        return buffer_[pos_++];
    }

    // tellg() may return -1 when failed. So we count by ourself.
    size_t Tell() const { return count_ + pos_; }

    // Not implemented
    Ch* PutBegin() { RAPIDJSON_ASSERT(false); return 0; }
    void Put(Ch) { RAPIDJSON_ASSERT(false); }
    void Flush() { RAPIDJSON_ASSERT(false); }
    size_t PutEnd(Ch*) { RAPIDJSON_ASSERT(false); return 0; }

    // For encoding detection only.
    const Ch* Peek4() const {
        RAPIDJSON_ASSERT(sizeof(Ch) == 1); // Only usable for byte stream.
        if (len_ - pos_ < 4) {
            if (pos_) {
                len_ -= pos_;
                std::memmove(buffer_, buffer_ + pos_, len_);
                count_ += pos_;
                pos_ = 0;
            }
            if (!stream_.read(buffer_ + len_, static_cast<std::streamsize>(size_ - len_))) {
                len_ += static_cast<size_t>(stream_.gcount());
                if (len_ < 4)
                    return 0;
            }
            else
                len_ = size_;
        }
        return &buffer_[pos_];
    }

private:
    BasicIStreamWrapper(const BasicIStreamWrapper&);
    BasicIStreamWrapper& operator=(const BasicIStreamWrapper&);

    size_t Read() const {
        count_ += pos_;
        pos_ = 0;
        if (!stream_.read(buffer_, static_cast<std::streamsize>(size_)))
            len_ = static_cast<size_t>(stream_.gcount());
        else
            len_ = size_;
        return len_;
    }

    StreamType& stream_;
    Ch peekBuffer_[4], *buffer_;
    size_t size_;
    mutable size_t pos_, len_, count_;
};

typedef BasicIStreamWrapper<std::istream> IStreamWrapper;
typedef BasicIStreamWrapper<std::wistream> WIStreamWrapper;

#if defined(__clang__) || defined(_MSC_VER)
RAPIDJSON_DIAG_POP
#endif

RAPIDJSON_NAMESPACE_END

#endif // RAPIDJSON_ISTREAMWRAPPER_H_
