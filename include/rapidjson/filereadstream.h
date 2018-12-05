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

#ifndef RAPIDJSON_FILEREADSTREAM_H_
#define RAPIDJSON_FILEREADSTREAM_H_

#include "stream.h"
#include <cstdio>
#include <cstring>

#ifdef __clang__
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(padded)
RAPIDJSON_DIAG_OFF(unreachable-code)
RAPIDJSON_DIAG_OFF(missing-noreturn)
#endif

RAPIDJSON_NAMESPACE_BEGIN

//! File byte stream for input using fread().
/*!
    \note implements Stream concept
*/
class FileReadStream {
public:
    typedef char Ch;    //!< Character type (byte).

    //! Constructor.
    /*!
        \param fp File pointer opened for read.
    */
    FileReadStream(std::FILE* fp) : fp_(fp), buffer_(peekBuffer_), size_(sizeof(peekBuffer_) / sizeof(Ch)), pos_(), len_(), count_()
    {
        RAPIDJSON_ASSERT(fp_ != 0);
    }

    //! Constructor.
    /*!
        \param fp File pointer opened for read.
        \param buffer user-supplied buffer.
        \param bufferSize size of buffer in bytes. Must >=4 bytes.
    */
    FileReadStream(std::FILE* fp, Ch *buffer, size_t size) : fp_(fp), buffer_(buffer), size_(size), pos_(), len_(), count_() {
        RAPIDJSON_ASSERT(fp_ != 0 && buffer_ != 0 && size_ > 0);
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

    size_t Tell() const { return count_ + pos_; }

    // Not implemented
    void Put(Ch) { RAPIDJSON_ASSERT(false); }
    void Flush() { RAPIDJSON_ASSERT(false); } 
    Ch* PutBegin() { RAPIDJSON_ASSERT(false); return 0; }
    size_t PutEnd(Ch*) { RAPIDJSON_ASSERT(false); return 0; }

    // For encoding detection only.
    const Ch* Peek4() const {
        if (len_ - pos_ < 4) {
            if (pos_) {
                len_ -= pos_;
                std::memmove(buffer_, buffer_ + pos_, len_);
                count_ += pos_;
                pos_ = 0;
            }
            len_ += std::fread(buffer_ + len_, sizeof(Ch), size_ - len_, fp_);
            if (len_ < 4)
                return 0;
        }
        return &buffer_[pos_];
    }

private:
    FileReadStream();
    FileReadStream(const FileReadStream&);
    FileReadStream& operator=(const FileReadStream&);

    size_t Read() const {
        count_ += pos_;
        pos_ = 0;
        len_ = std::fread(buffer_, sizeof(Ch), size_, fp_);
        return len_;
    }

    std::FILE* fp_;
    Ch peekBuffer_[4], *buffer_;
    size_t size_;
    mutable size_t pos_, len_, count_;
};

RAPIDJSON_NAMESPACE_END

#ifdef __clang__
RAPIDJSON_DIAG_POP
#endif

#endif // RAPIDJSON_FILESTREAM_H_
