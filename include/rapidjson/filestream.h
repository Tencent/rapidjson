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

#ifndef RAPIDJSON_FILESTREAM_H_
#define RAPIDJSON_FILESTREAM_H_

#include "rapidjson.h"
#include <cstdio>

RAPIDJSON_NAMESPACE_BEGIN

//! (Deprecated) Wrapper of C file stream for input or output.
/*!
    This simple wrapper does not check the validity of the stream.
    \note implements Stream concept
    \note deprecated: This was only for basic testing in version 0.1, it is found that the performance is very low by using fgetc(). Use FileReadStream instead.
*/
class FileStream {
public:
    typedef char Ch;    //!< Character type. Only support char.

    FileStream(std::FILE* fp) : fp_(fp), current_('\0'), count_(0) { Read(); }
    char Peek() const { return current_; }
    char Take() { char c = current_; Read(); return c; }
    size_t Tell() const { return count_; }
    void Put(char c) { fputc(c, fp_); }
    void Flush() { fflush(fp_); }

    // Not implemented
    char* PutBegin() { return 0; }
    size_t PutEnd(char*) { return 0; }

private:
    // Prohibit copy constructor & assignment operator.
    FileStream(const FileStream&);
    FileStream& operator=(const FileStream&);

    void Read() {
        RAPIDJSON_ASSERT(fp_ != 0);
        int c = fgetc(fp_);
        if (c != EOF) {
            current_ = (char)c;
            count_++;
        }
        else if (current_ != '\0')
            current_ = '\0';
    }

    std::FILE* fp_;
    char current_;
    size_t count_;
};

RAPIDJSON_NAMESPACE_END

#endif // RAPIDJSON_FILESTREAM_H_
