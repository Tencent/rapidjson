// Tencent is pleased to support the open source community by making RapidJSON available.
//
// Copyright (C) 2015 THL A29 Limited, a Tencent company, Milo Yip and Bruno Nicoletti. All rights reserved.
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

#ifndef RAPIDJSON_IOSTREAMREADSTREAM_H_
#define RAPIDJSON_IOSTREAMREADSTREAM_H_

#include <istream>
#include "rapidjson.h"

RAPIDJSON_NAMESPACE_BEGIN

//! File byte stream for input using fread().
/*!
    \note implements Stream concept
*/
class IOStreamReadStream {
public:
  typedef char Ch;    //!< Character type (byte).

  //! Constructor.
  /*!
    \param An open istream to read from.
  */
  IOStreamReadStream(std::istream &stream)
    : istream_(stream)
  {
    peekBuffer_[0] = peekBuffer_[1] = peekBuffer_[2] = peekBuffer_[3] = 0;
  }

  //! Look at the next character in the stream without consuming it. Returns '\0' on eof.
  Ch Peek() const
  {
    // given the Reader Concept, we need to explicitly check for eof and return '\0'
    int v = istream_.peek();
    if(v == std::char_traits<Ch>::eof()) return '\0';
    return v;
  }

  //! Extract the next character in the stream.
  Ch Take()
  {
    // given the Reader Concept, we need to explicitly check for eof and return '\0'
    int v = istream_.get();
    if(v == std::char_traits<Ch>::eof()) return '\0';
    return v;
  }

  //! Where are we in the stream.
  size_t Tell() const
  {
    return istream_.tellg();
  }

  // Not implemented
  void Put(Ch) { RAPIDJSON_ASSERT(false); }
  void Flush() { RAPIDJSON_ASSERT(false); }
  Ch* PutBegin() { RAPIDJSON_ASSERT(false); return 0; }
  size_t PutEnd(Ch*) { RAPIDJSON_ASSERT(false); return 0; }

  //! Get a pointer to the next four characters in the stream.
  //!
  //! For encoding detection only, called at the start on openning a stream.
  const Ch* Peek4() const {
    istream_.get(peekBuffer_, 4);
    std::streamsize n = istream_.gcount() - 1;
    for(std::streamsize i = 0; i <= n; ++i) {
      istream_.putback(peekBuffer_[n - i]);
    }
    return peekBuffer_;
  }

private:
  std::istream& istream_;
  mutable Ch peekBuffer_[4]; ///< need for the Peek4 nastyness
};

RAPIDJSON_NAMESPACE_END

#endif // RAPIDJSON_STDSTREAMSTREAM_H_
