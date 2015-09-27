// Copyright (C) 2015 Bruno Nicoletti
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

#ifndef RAPIDJSON_AUTOBLOCKS_H_
#define RAPIDJSON_AUTOBLOCKS_H_

RAPIDJSON_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
/// Class to start and automatically close a JSON object on the given writer
/// when it goes out of scope.
///
/// \tparam WRITER - a class derived from WriterBase.
template<class WRITER>
class ObjectBlock {
public :
  /// \brief Constructor,
  ///
  /// \param the writer to open the object on and eventually close.
  ObjectBlock(WRITER &writer)
    : writer_(&writer)
  {
    writer_->StartObject();
  }

  /// \brief Constructor,
  ///
  /// \param writer the writer to open the object on and eventually close.
  /// \param key the key to give this object
  template <class T>
  ObjectBlock(WRITER &writer,
              const T &key)
    : writer_(&writer)
  {
    writer.Key(key);
    writer_->StartObject();
  }

#if __cplusplus >= 201103
  /// \brief Move copy constructor, used by helper function in BaseWriter
  ///
  /// This steals the writer from the src ObjectBlock. Allows for
  /// function returns and assignements.
  ObjectBlock(ObjectBlockBlock &&src)
    : writer_(src.writer_)
  {
    src.writer_ = NULL;
  }
#endif

  /// \brief Destructor, calls EndObject on writer
  ~ObjectBlock()
  {
    if(writer_) {
      writer_->EndObject();
    }
  }
protected :
  WRITER *writer_;

private :
  ObjectBlock(const ObjectBlock&);
  ObjectBlock& operator=(const ObjectBlock&);
};

////////////////////////////////////////////////////////////////////////////////
/// Class to start and automatically close a JSON array on the given writer
/// when it goes out of scope.
///
/// \tparam WRITER - a class derived from WriterBase.
template<class WRITER>
class ArrayBlock {
public :
  /// \brief Constructor,
  ///
  /// \param the writer to open the object on and eventually close.
  ArrayBlock(WRITER &writer)
    : writer_(&writer)
  {
    writer_->StartArray();
  }

  /// \brief Constructor,
  ///
  /// \param writer the writer to open the array on and eventually close.
  /// \param key the key to give this object
  template <class T>
  ArrayBlock(WRITER &writer,
             const T &key)
    : writer_(&writer)
  {
    writer.Key(key);
    writer_->StartArray();
  }

#if __cplusplus >= 201103
  /// \brief Move copy constructor, used by helper function in BaseWriter
  ///
  /// This steals the writer from the src ArrayBlock. Allows for
  /// function returns and assignements.
  ArrayBlock(ArrayBlock &&src)
    : writer_(src.writer_)
  {
    src.writer_ = NULL;
  }
#endif

  /// \brief Destructor, calls EndArray on writer
  ~ArrayBlock()
  {
    if(writer_) {
      writer_->EndArray();
    }
  }

protected :
  WRITER *writer_;

private :
  // disable copy constructor and assignment
  ArrayBlock(const ArrayBlock&);
  ArrayBlock& operator=(const ArrayBlock&);
};

RAPIDJSON_NAMESPACE_END

#endif // RAPIDJSON_RAPIDJSON_H_
