// Copyright (C) 2015 Bruno Nicoletti. All rights reserved.
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
#ifndef RAPIDJSON_IOSTREAMWRITESTREAM_H_
#define RAPIDJSON_IOSTREAMWRITESTREAM_H_

#include <iostream>

#include "rapidjson.h"

RAPIDJSON_NAMESPACE_BEGIN

//! File byte stream for output using a std::ostream.
/*!
  \note implements Stream concept
*/

class IOStreamWriteStream {
public:
  typedef char Ch;    //!< Character type. Only support char.

  //! Constructor, takes a reference to an std::ostream
  IOStreamWriteStream(std::ostream &stream)
    : ostream_(stream)
  {
  }

  //! Put a character out to the ostream.
  void Put(char c) {
    ostream_.put(c);
  }

  //! Flush the ostream.
  void Flush() {
    ostream_.flush();
  }

  // Not implemented
  char Peek() const { RAPIDJSON_ASSERT(false); return 0; }
  char Take() { RAPIDJSON_ASSERT(false); return 0; }
  size_t Tell() const { RAPIDJSON_ASSERT(false); return 0; }
  char* PutBegin() { RAPIDJSON_ASSERT(false); return 0; }
  size_t PutEnd(char*) { RAPIDJSON_ASSERT(false); return 0; }

private:
  // Prohibit copy constructor & assignment operator.
  IOStreamWriteStream(const IOStreamWriteStream&);
  IOStreamWriteStream& operator=(const IOStreamWriteStream&);

  std::ostream &ostream_;
};

RAPIDJSON_NAMESPACE_END
#endif // RAPIDJSON_IOSTREAMWRITESTREAM_H_
