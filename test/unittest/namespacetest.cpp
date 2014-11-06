// Copyright (C) 2011 Milo Yip
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include "unittest.h"

// test another instantiation of RapidJSON in a different namespace 

#define RAPIDJSON_NAMESPACE my::rapid::json
#define RAPIDJSON_NAMESPACE_BEGIN namespace my { namespace rapid { namespace json {
#define RAPIDJSON_NAMESPACE_END } } }

// include lots of RapidJSON files

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/encodedstream.h"
#include "rapidjson/stringbuffer.h"

static const char json[] = "{\"hello\":\"world\",\"t\":true,\"f\":false,\"n\":null,\"i\":123,\"pi\":3.1416,\"a\":[1,2,3,4]}";

TEST(NamespaceTest,Using) {
    using namespace RAPIDJSON_NAMESPACE;
    typedef GenericDocument<UTF8<>, CrtAllocator> DocumentType;
    DocumentType doc;

    doc.Parse(json);
    EXPECT_TRUE(!doc.HasParseError());
}

TEST(NamespaceTest,Direct) {
    typedef RAPIDJSON_NAMESPACE::Document Document;
    typedef RAPIDJSON_NAMESPACE::Reader Reader;
    typedef RAPIDJSON_NAMESPACE::StringStream StringStream;
    typedef RAPIDJSON_NAMESPACE::StringBuffer StringBuffer;
    typedef RAPIDJSON_NAMESPACE::Writer<StringBuffer> WriterType;

    StringStream s(json);
    StringBuffer buffer;
    WriterType writer(buffer);
    buffer.ShrinkToFit();
    Reader reader;
    reader.Parse(s, writer);

    EXPECT_STREQ(json, buffer.GetString());
    EXPECT_EQ(sizeof(json)-1, buffer.GetSize());
    EXPECT_TRUE(writer.IsComplete());

    Document doc;
    doc.Parse(buffer.GetString());
    EXPECT_TRUE(!doc.HasParseError());

    buffer.Clear();
    writer.Reset(buffer);
    doc.Accept(writer);
    EXPECT_STREQ(json, buffer.GetString());
    EXPECT_TRUE(writer.IsComplete());
}
