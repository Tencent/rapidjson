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
#include "rapidjson/reader.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;

static const char kJson[] = "{\"hello\":\"world\",\"t\":true,\"f\":false,\"n\":null,\"i\":123,\"pi\":3.1416,\"a\":[1,2,3,-1],\"u64\":1234567890123456789,\"i64\":-1234567890123456789}";

TEST(PrettyWriter, Basic) {
    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    Reader reader;
    StringStream s(kJson);
    reader.Parse(s, writer);
    EXPECT_STREQ(
        "{\n"
        "    \"hello\": \"world\",\n"
        "    \"t\": true,\n"
        "    \"f\": false,\n"
        "    \"n\": null,\n"
        "    \"i\": 123,\n"
        "    \"pi\": 3.1416,\n"
        "    \"a\": [\n"
        "        1,\n"
        "        2,\n"
        "        3,\n"
        "        -1\n"
        "    ],\n"
        "    \"u64\": 1234567890123456789,\n"
        "    \"i64\": -1234567890123456789\n"
        "}",
        buffer.GetString());
}

TEST(PrettyWriter, SetIndent) {
    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    writer.SetIndent('\t', 1);
    Reader reader;
    StringStream s(kJson);
    reader.Parse(s, writer);
    EXPECT_STREQ(
        "{\n"
        "\t\"hello\": \"world\",\n"
        "\t\"t\": true,\n"
        "\t\"f\": false,\n"
        "\t\"n\": null,\n"
        "\t\"i\": 123,\n"
        "\t\"pi\": 3.1416,\n"
        "\t\"a\": [\n"
        "\t\t1,\n"
        "\t\t2,\n"
        "\t\t3,\n"
        "\t\t-1\n"
        "\t],\n"
        "\t\"u64\": 1234567890123456789,\n"
        "\t\"i64\": -1234567890123456789\n"
        "}",
        buffer.GetString());
}

TEST(PrettyWriter, String) {
    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    EXPECT_TRUE(writer.StartArray());
    EXPECT_TRUE(writer.String("Hello\n"));
    EXPECT_TRUE(writer.EndArray());
    EXPECT_STREQ("[\n    \"Hello\\n\"\n]", buffer.GetString());
}

#if RAPIDJSON_HAS_STDSTRING
TEST(PrettyWriter, String_STDSTRING) {
    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    EXPECT_TRUE(writer.StartArray());
    EXPECT_TRUE(writer.String(std::string("Hello\n")));
    EXPECT_TRUE(writer.EndArray());
    EXPECT_STREQ("[\n    \"Hello\\n\"\n]", buffer.GetString());
}
#endif

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

// For covering PutN() generic version
TEST(PrettyWriter, OStreamWrapper) {
    StringStream s(kJson);
    
    std::stringstream ss;
    OStreamWrapper os(ss);
    
    PrettyWriter<OStreamWrapper> writer(os);

    Reader reader;
    reader.Parse(s, writer);
    
    std::string actual = ss.str();
    EXPECT_STREQ(
        "{\n"
        "    \"hello\": \"world\",\n"
        "    \"t\": true,\n"
        "    \"f\": false,\n"
        "    \"n\": null,\n"
        "    \"i\": 123,\n"
        "    \"pi\": 3.1416,\n"
        "    \"a\": [\n"
        "        1,\n"
        "        2,\n"
        "        3,\n"
        "        -1\n"
        "    ],\n"
        "    \"u64\": 1234567890123456789,\n"
        "    \"i64\": -1234567890123456789\n"
        "}",
        actual.c_str());
}
