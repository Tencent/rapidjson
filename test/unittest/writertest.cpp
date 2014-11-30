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

#include "rapidjson/document.h"
#include "rapidjson/reader.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;

TEST(Writer, Compact) {
    StringStream s("{ \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3] } ");
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    buffer.ShrinkToFit();
    Reader reader;
    reader.Parse<0>(s, writer);
    EXPECT_STREQ("{\"hello\":\"world\",\"t\":true,\"f\":false,\"n\":null,\"i\":123,\"pi\":3.1416,\"a\":[1,2,3]}", buffer.GetString());
    EXPECT_EQ(77u, buffer.GetSize());
    EXPECT_TRUE(writer.IsComplete());
}

// json -> parse -> writer -> json
#define TEST_ROUNDTRIP(json) \
    { \
        StringStream s(json); \
        StringBuffer buffer; \
        Writer<StringBuffer> writer(buffer); \
        Reader reader; \
        reader.Parse<0>(s, writer); \
        EXPECT_STREQ(json, buffer.GetString()); \
        EXPECT_TRUE(writer.IsComplete()); \
    }

TEST(Writer, Root) {
    TEST_ROUNDTRIP("null");
    TEST_ROUNDTRIP("true");
    TEST_ROUNDTRIP("false");
    TEST_ROUNDTRIP("0");
    TEST_ROUNDTRIP("\"foo\"");
    TEST_ROUNDTRIP("[]");
    TEST_ROUNDTRIP("{}");
}

TEST(Writer, Int) {
    TEST_ROUNDTRIP("[-1]");
    TEST_ROUNDTRIP("[-123]");
    TEST_ROUNDTRIP("[-2147483648]");
}

TEST(Writer, UInt) {
    TEST_ROUNDTRIP("[0]");
    TEST_ROUNDTRIP("[1]");
    TEST_ROUNDTRIP("[123]");
    TEST_ROUNDTRIP("[2147483647]");
    TEST_ROUNDTRIP("[4294967295]");
}

TEST(Writer, Int64) {
    TEST_ROUNDTRIP("[-1234567890123456789]");
    TEST_ROUNDTRIP("[-9223372036854775808]");
}

TEST(Writer, Uint64) {
    TEST_ROUNDTRIP("[1234567890123456789]");
    TEST_ROUNDTRIP("[9223372036854775807]");
}

TEST(Writer, String) {
    TEST_ROUNDTRIP("[\"Hello\"]");
    TEST_ROUNDTRIP("[\"Hello\\u0000World\"]");
    TEST_ROUNDTRIP("[\"\\\"\\\\/\\b\\f\\n\\r\\t\"]");
}

TEST(Writer, Double) {
    TEST_ROUNDTRIP("[1.2345,1.2345678,0.123456789012,1234567.8]");

}

TEST(Writer, Transcode) {
    const char json[] = "{\"hello\":\"world\",\"t\":true,\"f\":false,\"n\":null,\"i\":123,\"pi\":3.1416,\"a\":[1,2,3],\"dollar\":\"\x24\",\"cents\":\"\xC2\xA2\",\"euro\":\"\xE2\x82\xAC\",\"gclef\":\"\xF0\x9D\x84\x9E\"}";

    // UTF8 -> UTF16 -> UTF8
    {
        StringStream s(json);
        StringBuffer buffer;
        Writer<StringBuffer, UTF16<>, UTF8<> > writer(buffer);
        GenericReader<UTF8<>, UTF16<> > reader;
        reader.Parse(s, writer);
        EXPECT_STREQ(json, buffer.GetString());
    }

    // UTF8 -> UTF8 -> ASCII -> UTF8 -> UTF8
    {
        StringStream s(json);
        StringBuffer buffer;
        Writer<StringBuffer, UTF8<>, ASCII<> > writer(buffer);
        Reader reader;
        reader.Parse(s, writer);

        StringBuffer buffer2;
        Writer<StringBuffer> writer2(buffer2);
        GenericReader<ASCII<>, UTF8<> > reader2;
        StringStream s2(buffer.GetString());
        reader2.Parse(s2, writer2);

        EXPECT_STREQ(json, buffer2.GetString());
    }
}

#include <sstream>

class OStreamWrapper {
public:
    typedef char Ch;

    OStreamWrapper(std::ostream& os) : os_(os) {}

    Ch Peek() const { assert(false); return '\0'; }
    Ch Take() { assert(false); return '\0'; }
    size_t Tell() const { return 0; }

    Ch* PutBegin() { assert(false); return 0; }
    void Put(Ch c) { os_.put(c); }
    void Flush() { os_.flush(); }
    size_t PutEnd(Ch*) { assert(false); return 0; }

private:
    OStreamWrapper(const OStreamWrapper&);
    OStreamWrapper& operator=(const OStreamWrapper&);

    std::ostream& os_;
};

TEST(Writer, OStreamWrapper) {
    StringStream s("{ \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3] } ");
    
    std::stringstream ss;
    OStreamWrapper os(ss);
    
    Writer<OStreamWrapper> writer(os);

    Reader reader;
    reader.Parse<0>(s, writer);
    
    std::string actual = ss.str();
    EXPECT_STREQ("{\"hello\":\"world\",\"t\":true,\"f\":false,\"n\":null,\"i\":123,\"pi\":3.1416,\"a\":[1,2,3]}", actual.c_str());
}

TEST(Writer, AssertRootMayBeAnyValue) {
#define T(x)\
    {\
        StringBuffer buffer;\
        Writer<StringBuffer> writer(buffer);\
        EXPECT_TRUE(x);\
    }
    T(writer.Bool(false));
    T(writer.Bool(true));
    T(writer.Null());
    T(writer.Int(0));
    T(writer.Uint(0));
    T(writer.Int64(0));
    T(writer.Uint64(0));
    T(writer.Double(0));
    T(writer.String("foo"));
#undef T
}

TEST(Writer, AssertIncorrectObjectLevel) {
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    writer.StartObject();
    writer.EndObject();
    ASSERT_THROW(writer.EndObject(), AssertException);
}

TEST(Writer, AssertIncorrectArrayLevel) {
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    writer.StartArray();
    writer.EndArray();
    ASSERT_THROW(writer.EndArray(), AssertException);
}

TEST(Writer, AssertIncorrectEndObject) {
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    writer.StartObject();
    ASSERT_THROW(writer.EndArray(), AssertException);
}

TEST(Writer, AssertIncorrectEndArray) {
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    writer.StartObject();
    ASSERT_THROW(writer.EndArray(), AssertException);
}

TEST(Writer, AssertObjectKeyNotString) {
#define T(x)\
    {\
        StringBuffer buffer;\
        Writer<StringBuffer> writer(buffer);\
        writer.StartObject();\
        ASSERT_THROW(x, AssertException); \
    }
    T(writer.Bool(false));
    T(writer.Bool(true));
    T(writer.Null());
    T(writer.Int(0));
    T(writer.Uint(0));
    T(writer.Int64(0));
    T(writer.Uint64(0));
    T(writer.Double(0));
    T(writer.StartObject());
    T(writer.StartArray());
#undef T
}

TEST(Writer, AssertMultipleRoot) {
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);

    writer.StartObject();
    writer.EndObject();
    ASSERT_THROW(writer.StartObject(), AssertException);

    writer.Reset(buffer);
    writer.Null();
    ASSERT_THROW(writer.Int(0), AssertException);

    writer.Reset(buffer);
    writer.String("foo");
    ASSERT_THROW(writer.StartArray(), AssertException);

    writer.Reset(buffer);
    writer.StartArray();
    writer.EndArray();
    //ASSERT_THROW(writer.Double(3.14), AssertException);
}

TEST(Writer, RootObjectIsComplete) {
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    EXPECT_FALSE(writer.IsComplete());
    writer.StartObject();
    EXPECT_FALSE(writer.IsComplete());
    writer.String("foo");
    EXPECT_FALSE(writer.IsComplete());
    writer.Int(1);
    EXPECT_FALSE(writer.IsComplete());
    writer.EndObject();
    EXPECT_TRUE(writer.IsComplete());
}

TEST(Writer, RootArrayIsComplete) {
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    EXPECT_FALSE(writer.IsComplete());
    writer.StartArray();
    EXPECT_FALSE(writer.IsComplete());
    writer.String("foo");
    EXPECT_FALSE(writer.IsComplete());
    writer.Int(1);
    EXPECT_FALSE(writer.IsComplete());
    writer.EndArray();
    EXPECT_TRUE(writer.IsComplete());
}

TEST(Writer, RootValueIsComplete) {
#define T(x)\
    {\
        StringBuffer buffer;\
        Writer<StringBuffer> writer(buffer);\
        EXPECT_FALSE(writer.IsComplete()); \
        x; \
        EXPECT_TRUE(writer.IsComplete()); \
    }
    T(writer.Null());
    T(writer.Bool(true));
    T(writer.Bool(false));
    T(writer.Int(0));
    T(writer.Uint(0));
    T(writer.Int64(0));
    T(writer.Uint64(0));
    T(writer.Double(0));
    T(writer.String(""));
#undef T
}
