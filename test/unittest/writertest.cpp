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
        reader.Parse<kParseFullPrecisionFlag>(s, writer); \
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

#if RAPIDJSON_HAS_STDSTRING
    {
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        writer.String(std::string("Hello\n"));
        EXPECT_STREQ("\"Hello\\n\"", buffer.GetString());
    }
#endif
}

TEST(Writer, Double) {
    TEST_ROUNDTRIP("[1.2345,1.2345678,0.123456789012,1234567.8]");
    TEST_ROUNDTRIP("0.0");
    TEST_ROUNDTRIP("-0.0"); // Issue #289
    TEST_ROUNDTRIP("1e30");
    TEST_ROUNDTRIP("1.0");
    TEST_ROUNDTRIP("5e-324"); // Min subnormal positive double
    TEST_ROUNDTRIP("2.225073858507201e-308"); // Max subnormal positive double
    TEST_ROUNDTRIP("2.2250738585072014e-308"); // Min normal positive double
    TEST_ROUNDTRIP("1.7976931348623157e308"); // Max double

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
    StringStream s("{ \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3], \"u64\": 1234567890123456789, \"i64\":-1234567890123456789 } ");
    
    std::stringstream ss;
    OStreamWrapper os(ss);
    
    Writer<OStreamWrapper> writer(os);

    Reader reader;
    reader.Parse<0>(s, writer);
    
    std::string actual = ss.str();
    EXPECT_STREQ("{\"hello\":\"world\",\"t\":true,\"f\":false,\"n\":null,\"i\":123,\"pi\":3.1416,\"a\":[1,2,3],\"u64\":1234567890123456789,\"i64\":-1234567890123456789}", actual.c_str());
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

TEST(Writer, InvalidEncoding) {
    // Fail in decoding invalid UTF-8 sequence http://www.cl.cam.ac.uk/~mgk25/ucs/examples/UTF-8-test.txt
    {
        GenericStringBuffer<UTF16<> > buffer;
        Writer<GenericStringBuffer<UTF16<> >, UTF8<>, UTF16<> > writer(buffer);
        writer.StartArray();
        EXPECT_FALSE(writer.String("\xfe"));
        EXPECT_FALSE(writer.String("\xff"));
        EXPECT_FALSE(writer.String("\xfe\xfe\xff\xff"));
        writer.EndArray();
    }

    // Fail in encoding
    {
        StringBuffer buffer;
        Writer<StringBuffer, UTF32<> > writer(buffer);
        static const UTF32<>::Ch s[] = { 0x110000, 0 }; // Out of U+0000 to U+10FFFF
        EXPECT_FALSE(writer.String(s));
    }

    // Fail in unicode escaping in ASCII output
    {
        StringBuffer buffer;
        Writer<StringBuffer, UTF32<>, ASCII<> > writer(buffer);
        static const UTF32<>::Ch s[] = { 0x110000, 0 }; // Out of U+0000 to U+10FFFF
        EXPECT_FALSE(writer.String(s));
    }
}

TEST(Writer, InvalidEventSequence) {
    // {]
    {
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        writer.StartObject();
        EXPECT_THROW(writer.EndArray(), AssertException);
        EXPECT_FALSE(writer.IsComplete());
    }

    // [}
    {
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        writer.StartArray();
        EXPECT_THROW(writer.EndObject(), AssertException);
        EXPECT_FALSE(writer.IsComplete());
    }

    // { 1: 
    {
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        writer.StartObject();
        EXPECT_THROW(writer.Int(1), AssertException);
        EXPECT_FALSE(writer.IsComplete());
    }
}
