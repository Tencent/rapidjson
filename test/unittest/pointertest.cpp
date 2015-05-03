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
#include "rapidjson/pointer.h"
#include "rapidjson/stringbuffer.h"
#include <sstream>

using namespace rapidjson;

static const char kJson[] = "{\n"
"    \"foo\":[\"bar\", \"baz\"],\n"
"    \"\" : 0,\n"
"    \"a/b\" : 1,\n"
"    \"c%d\" : 2,\n"
"    \"e^f\" : 3,\n"
"    \"g|h\" : 4,\n"
"    \"i\\\\j\" : 5,\n"
"    \"k\\\"l\" : 6,\n"
"    \" \" : 7,\n"
"    \"m~n\" : 8\n"
"}";

TEST(Pointer, Parse) {
    {
        Pointer p("");
        EXPECT_TRUE(p.IsValid());
        EXPECT_EQ(0u, p.GetTokenCount());
    }

    {
        Pointer p("/foo");
        EXPECT_TRUE(p.IsValid());
        EXPECT_EQ(1u, p.GetTokenCount());
        EXPECT_EQ(3u, p.GetTokens()[0].length);
        EXPECT_STREQ("foo", p.GetTokens()[0].name);
    }

    {
        Pointer p("/foo/0");
        EXPECT_TRUE(p.IsValid());
        EXPECT_EQ(2u, p.GetTokenCount());
        EXPECT_EQ(3u, p.GetTokens()[0].length);
        EXPECT_STREQ("foo", p.GetTokens()[0].name);
        EXPECT_EQ(1u, p.GetTokens()[1].length);
        EXPECT_STREQ("0", p.GetTokens()[1].name);
        EXPECT_EQ(0u, p.GetTokens()[1].index);
    }

    {
        // Unescape ~1
        Pointer p("/a~1b");
        EXPECT_TRUE(p.IsValid());
        EXPECT_EQ(1u, p.GetTokenCount());
        EXPECT_EQ(3u, p.GetTokens()[0].length);
        EXPECT_STREQ("a/b", p.GetTokens()[0].name);
    }

    {
        // Unescape ~0
        Pointer p("/m~0n");
        EXPECT_TRUE(p.IsValid());
        EXPECT_EQ(1u, p.GetTokenCount());
        EXPECT_EQ(3u, p.GetTokens()[0].length);
        EXPECT_STREQ("m~n", p.GetTokens()[0].name);
    }

    {
        // empty name
        Pointer p("/");
        EXPECT_TRUE(p.IsValid());
        EXPECT_EQ(1u, p.GetTokenCount());
        EXPECT_EQ(0u, p.GetTokens()[0].length);
        EXPECT_STREQ("", p.GetTokens()[0].name);
    }

    {
        // empty and non-empty name
        Pointer p("//a");
        EXPECT_TRUE(p.IsValid());
        EXPECT_EQ(2u, p.GetTokenCount());
        EXPECT_EQ(0u, p.GetTokens()[0].length);
        EXPECT_STREQ("", p.GetTokens()[0].name);
        EXPECT_EQ(1u, p.GetTokens()[1].length);
        EXPECT_STREQ("a", p.GetTokens()[1].name);
    }

    {
        // Null characters
        Pointer p("/\0\0", 3);
        EXPECT_TRUE(p.IsValid());
        EXPECT_EQ(1u, p.GetTokenCount());
        EXPECT_EQ(2u, p.GetTokens()[0].length);
        EXPECT_EQ('\0', p.GetTokens()[0].name[0]);
        EXPECT_EQ('\0', p.GetTokens()[0].name[1]);
        EXPECT_EQ('\0', p.GetTokens()[0].name[2]);
    }

    {
        // Valid index
        Pointer p("/123");
        EXPECT_TRUE(p.IsValid());
        EXPECT_EQ(1u, p.GetTokenCount());
        EXPECT_STREQ("123", p.GetTokens()[0].name);
        EXPECT_EQ(123u, p.GetTokens()[0].index);
    }

    {
        // Invalid index (with leading zero)
        Pointer p("/01");
        EXPECT_TRUE(p.IsValid());
        EXPECT_EQ(1u, p.GetTokenCount());
        EXPECT_STREQ("01", p.GetTokens()[0].name);
        EXPECT_EQ(kPointerInvalidIndex, p.GetTokens()[0].index);
    }

    if (sizeof(SizeType) == 4) {
        // Invalid index (overflow)
        Pointer p("/4294967296");
        EXPECT_TRUE(p.IsValid());
        EXPECT_EQ(1u, p.GetTokenCount());
        EXPECT_STREQ("4294967296", p.GetTokens()[0].name);
        EXPECT_EQ(kPointerInvalidIndex, p.GetTokens()[0].index);
    }

    {
        // kPointerParseErrorTokenMustBeginWithSolidus
        Pointer p(" ");
        EXPECT_FALSE(p.IsValid());
        EXPECT_EQ(kPointerParseErrorTokenMustBeginWithSolidus, p.GetParseErrorCode());
        EXPECT_EQ(0u, p.GetParseErrorOffset());
    }

    {
        // kPointerParseErrorInvalidEscape
        Pointer p("/~");
        EXPECT_FALSE(p.IsValid());
        EXPECT_EQ(kPointerParseErrorInvalidEscape, p.GetParseErrorCode());
        EXPECT_EQ(2u, p.GetParseErrorOffset());
    }

    {
        // kPointerParseErrorInvalidEscape
        Pointer p("/~2");
        EXPECT_FALSE(p.IsValid());
        EXPECT_EQ(kPointerParseErrorInvalidEscape, p.GetParseErrorCode());
        EXPECT_EQ(2u, p.GetParseErrorOffset());
    }
}

TEST(Pointer, Parse_URIFragment) {
    {
        Pointer p("#");
        EXPECT_TRUE(p.IsValid());
        EXPECT_EQ(0u, p.GetTokenCount());
    }

    {
        Pointer p("#/foo");
        EXPECT_TRUE(p.IsValid());
        EXPECT_EQ(1u, p.GetTokenCount());
        EXPECT_EQ(3u, p.GetTokens()[0].length);
        EXPECT_STREQ("foo", p.GetTokens()[0].name);
    }

    {
        Pointer p("#/foo/0");
        EXPECT_TRUE(p.IsValid());
        EXPECT_EQ(2u, p.GetTokenCount());
        EXPECT_EQ(3u, p.GetTokens()[0].length);
        EXPECT_STREQ("foo", p.GetTokens()[0].name);
        EXPECT_EQ(1u, p.GetTokens()[1].length);
        EXPECT_STREQ("0", p.GetTokens()[1].name);
        EXPECT_EQ(0u, p.GetTokens()[1].index);
    }

    {
        // Unescape ~1
        Pointer p("#/a~1b");
        EXPECT_TRUE(p.IsValid());
        EXPECT_EQ(1u, p.GetTokenCount());
        EXPECT_EQ(3u, p.GetTokens()[0].length);
        EXPECT_STREQ("a/b", p.GetTokens()[0].name);
    }

    {
        // Unescape ~0
        Pointer p("#/m~0n");
        EXPECT_TRUE(p.IsValid());
        EXPECT_EQ(1u, p.GetTokenCount());
        EXPECT_EQ(3u, p.GetTokens()[0].length);
        EXPECT_STREQ("m~n", p.GetTokens()[0].name);
    }

    {
        // empty name
        Pointer p("#/");
        EXPECT_TRUE(p.IsValid());
        EXPECT_EQ(1u, p.GetTokenCount());
        EXPECT_EQ(0u, p.GetTokens()[0].length);
        EXPECT_STREQ("", p.GetTokens()[0].name);
    }

    {
        // empty and non-empty name
        Pointer p("#//a");
        EXPECT_TRUE(p.IsValid());
        EXPECT_EQ(2u, p.GetTokenCount());
        EXPECT_EQ(0u, p.GetTokens()[0].length);
        EXPECT_STREQ("", p.GetTokens()[0].name);
        EXPECT_EQ(1u, p.GetTokens()[1].length);
        EXPECT_STREQ("a", p.GetTokens()[1].name);
    }

    {
        // Null characters
        Pointer p("#/%00%00");
        EXPECT_TRUE(p.IsValid());
        EXPECT_EQ(1u, p.GetTokenCount());
        EXPECT_EQ(2u, p.GetTokens()[0].length);
        EXPECT_EQ('\0', p.GetTokens()[0].name[0]);
        EXPECT_EQ('\0', p.GetTokens()[0].name[1]);
        EXPECT_EQ('\0', p.GetTokens()[0].name[2]);
    }

    {
        // Percentage Escapes
        EXPECT_STREQ("c%d", Pointer("#/c%25d").GetTokens()[0].name);
        EXPECT_STREQ("e^f", Pointer("#/e%5Ef").GetTokens()[0].name);
        EXPECT_STREQ("g|h", Pointer("#/g%7Ch").GetTokens()[0].name);
        EXPECT_STREQ("i\\j", Pointer("#/i%5Cj").GetTokens()[0].name);
        EXPECT_STREQ("k\"l", Pointer("#/k%22l").GetTokens()[0].name);
        EXPECT_STREQ(" ", Pointer("#/%20").GetTokens()[0].name);
    }

    {
        // Valid index
        Pointer p("#/123");
        EXPECT_TRUE(p.IsValid());
        EXPECT_EQ(1u, p.GetTokenCount());
        EXPECT_STREQ("123", p.GetTokens()[0].name);
        EXPECT_EQ(123u, p.GetTokens()[0].index);
    }

    {
        // Invalid index (with leading zero)
        Pointer p("#/01");
        EXPECT_TRUE(p.IsValid());
        EXPECT_EQ(1u, p.GetTokenCount());
        EXPECT_STREQ("01", p.GetTokens()[0].name);
        EXPECT_EQ(kPointerInvalidIndex, p.GetTokens()[0].index);
    }

    if (sizeof(SizeType) == 4) {
        // Invalid index (overflow)
        Pointer p("#/4294967296");
        EXPECT_TRUE(p.IsValid());
        EXPECT_EQ(1u, p.GetTokenCount());
        EXPECT_STREQ("4294967296", p.GetTokens()[0].name);
        EXPECT_EQ(kPointerInvalidIndex, p.GetTokens()[0].index);
    }

    {
        // kPointerParseErrorTokenMustBeginWithSolidus
        Pointer p("# ");
        EXPECT_FALSE(p.IsValid());
        EXPECT_EQ(kPointerParseErrorTokenMustBeginWithSolidus, p.GetParseErrorCode());
        EXPECT_EQ(1u, p.GetParseErrorOffset());
    }

    {
        // kPointerParseErrorInvalidEscape
        Pointer p("#/~");
        EXPECT_FALSE(p.IsValid());
        EXPECT_EQ(kPointerParseErrorInvalidEscape, p.GetParseErrorCode());
        EXPECT_EQ(3u, p.GetParseErrorOffset());
    }

    {
        // kPointerParseErrorInvalidEscape
        Pointer p("#/~2");
        EXPECT_FALSE(p.IsValid());
        EXPECT_EQ(kPointerParseErrorInvalidEscape, p.GetParseErrorCode());
        EXPECT_EQ(3u, p.GetParseErrorOffset());
    }

    {
        // kPointerParseErrorInvalidPercentEncoding
        Pointer p("#/%");
        EXPECT_FALSE(p.IsValid());
        EXPECT_EQ(kPointerParseErrorInvalidPercentEncoding, p.GetParseErrorCode());
        EXPECT_EQ(3u, p.GetParseErrorOffset());
    }

    {
        // kPointerParseErrorInvalidPercentEncoding
        Pointer p("#/%g0");
        EXPECT_FALSE(p.IsValid());
        EXPECT_EQ(kPointerParseErrorInvalidPercentEncoding, p.GetParseErrorCode());
        EXPECT_EQ(3u, p.GetParseErrorOffset());
    }

    {
        // kPointerParseErrorInvalidPercentEncoding
        Pointer p("#/%0g");
        EXPECT_FALSE(p.IsValid());
        EXPECT_EQ(kPointerParseErrorInvalidPercentEncoding, p.GetParseErrorCode());
        EXPECT_EQ(4u, p.GetParseErrorOffset());
    }

    {
        // kPointerParseErrorCharacterMustPercentEncode
        Pointer p("#/ ");
        EXPECT_FALSE(p.IsValid());
        EXPECT_EQ(kPointerParseErrorCharacterMustPercentEncode, p.GetParseErrorCode());
        EXPECT_EQ(2u, p.GetParseErrorOffset());
    }

    {
        // kPointerParseErrorCharacterMustPercentEncode
        Pointer p("#/\\");
        EXPECT_FALSE(p.IsValid());
        EXPECT_EQ(kPointerParseErrorCharacterMustPercentEncode, p.GetParseErrorCode());
        EXPECT_EQ(2u, p.GetParseErrorOffset());
    }

}

TEST(Pointer, Stringify) {
    // Test by roundtrip
    const char* sources[] = {
        "",
        "/foo",
        "/foo/0",
        "/",
        "/a~1b",
        "/c%d",
        "/e^f",
        "/g|h",
        "/i\\j",
        "/k\"l",
        "/ ",
        "/m~0n"
    };

    for (size_t i = 0; i < sizeof(sources) / sizeof(sources[0]); i++) {
        Pointer p(sources[i]);
        StringBuffer s;
        p.Stringify(s);
        EXPECT_STREQ(sources[i], s.GetString());
    }
}

// Construct a Pointer with static tokens, no dynamic allocation involved.
#define NAME(s) { s, sizeof(s) / sizeof(s[0]) - 1, kPointerInvalidIndex }
#define INDEX(i) { #i, sizeof(#i) - 1, i }

static const Pointer::Token kTokens[] = { NAME("foo"), INDEX(0) }; // equivalent to "/foo/0"

#undef NAME
#undef INDEX

TEST(Pointer, ConstructorWithToken) {
    Pointer p(kTokens, sizeof(kTokens) / sizeof(kTokens[0]));
    EXPECT_TRUE(p.IsValid());
    EXPECT_EQ(2u, p.GetTokenCount());
    EXPECT_EQ(3u, p.GetTokens()[0].length);
    EXPECT_STREQ("foo", p.GetTokens()[0].name);
    EXPECT_EQ(1u, p.GetTokens()[1].length);
    EXPECT_STREQ("0", p.GetTokens()[1].name);
    EXPECT_EQ(0u, p.GetTokens()[1].index);
}

TEST(Pointer, CopyConstructor) {
    {
        Pointer p("/foo/0");
        Pointer q(p);
        EXPECT_TRUE(q.IsValid());
        EXPECT_EQ(2u, q.GetTokenCount());
        EXPECT_EQ(3u, q.GetTokens()[0].length);
        EXPECT_STREQ("foo", q.GetTokens()[0].name);
        EXPECT_EQ(1u, q.GetTokens()[1].length);
        EXPECT_STREQ("0", q.GetTokens()[1].name);
        EXPECT_EQ(0u, q.GetTokens()[1].index);
    }

    // Static tokens
    {
        Pointer p(kTokens, sizeof(kTokens) / sizeof(kTokens[0]));
        Pointer q(p);
        EXPECT_TRUE(q.IsValid());
        EXPECT_EQ(2u, q.GetTokenCount());
        EXPECT_EQ(3u, q.GetTokens()[0].length);
        EXPECT_STREQ("foo", q.GetTokens()[0].name);
        EXPECT_EQ(1u, q.GetTokens()[1].length);
        EXPECT_STREQ("0", q.GetTokens()[1].name);
        EXPECT_EQ(0u, q.GetTokens()[1].index);
    }
}

TEST(Pointer, Assignment) {
    {
        Pointer p("/foo/0");
        Pointer q;
        q = p;
        EXPECT_TRUE(q.IsValid());
        EXPECT_EQ(2u, q.GetTokenCount());
        EXPECT_EQ(3u, q.GetTokens()[0].length);
        EXPECT_STREQ("foo", q.GetTokens()[0].name);
        EXPECT_EQ(1u, q.GetTokens()[1].length);
        EXPECT_STREQ("0", q.GetTokens()[1].name);
        EXPECT_EQ(0u, q.GetTokens()[1].index);
    }

    // Static tokens
    {
        Pointer p(kTokens, sizeof(kTokens) / sizeof(kTokens[0]));
        Pointer q;
        q = p;
        EXPECT_TRUE(q.IsValid());
        EXPECT_EQ(2u, q.GetTokenCount());
        EXPECT_EQ(3u, q.GetTokens()[0].length);
        EXPECT_STREQ("foo", q.GetTokens()[0].name);
        EXPECT_EQ(1u, q.GetTokens()[1].length);
        EXPECT_STREQ("0", q.GetTokens()[1].name);
        EXPECT_EQ(0u, q.GetTokens()[1].index);
    }
}

TEST(Pointer, Create) {
    Document d;
    {
        Value* v = &Pointer("").Create(d, d.GetAllocator());
        EXPECT_EQ(&d, v);
    }
    {
        Value* v = &Pointer("/foo").Create(d, d.GetAllocator());
        EXPECT_EQ(&d["foo"], v);
    }
    {
        Value* v = &Pointer("/foo/0").Create(d, d.GetAllocator());
        EXPECT_EQ(&d["foo"][0], v);
    }
    {
        Value* v = &Pointer("/foo/-").Create(d, d.GetAllocator());
        EXPECT_EQ(&d["foo"][1], v);
    }
    {
        Value* v = &Pointer("/foo/-/-").Create(d, d.GetAllocator());
        EXPECT_EQ(&d["foo"][2][0], v);
    }

    {
        // Document with no allocator
        Value* v = &Pointer("/foo/-").Create(d);
        EXPECT_EQ(&d["foo"][3], v);
    }

    {
        // Value (not document) must give allocator
        Value* v = &Pointer("/-").Create(d["foo"], d.GetAllocator());
        EXPECT_EQ(&d["foo"][4], v);
    }
}

TEST(Pointer, Get) {
    Document d;
    d.Parse(kJson);

    EXPECT_EQ(&d, Pointer("").Get(d));
    EXPECT_EQ(&d["foo"], Pointer("/foo").Get(d));
    EXPECT_EQ(&d["foo"][0], Pointer("/foo/0").Get(d));
    EXPECT_EQ(&d[""], Pointer("/").Get(d));
    EXPECT_EQ(&d["a/b"], Pointer("/a~1b").Get(d));
    EXPECT_EQ(&d["c%d"], Pointer("/c%d").Get(d));
    EXPECT_EQ(&d["e^f"], Pointer("/e^f").Get(d));
    EXPECT_EQ(&d["g|h"], Pointer("/g|h").Get(d));
    EXPECT_EQ(&d["i\\j"], Pointer("/i\\j").Get(d));
    EXPECT_EQ(&d["k\"l"], Pointer("/k\"l").Get(d));
    EXPECT_EQ(&d[" "], Pointer("/ ").Get(d));
    EXPECT_EQ(&d["m~n"], Pointer("/m~0n").Get(d));
    EXPECT_TRUE(Pointer("/abc").Get(d) == 0);
}

TEST(Pointer, GetWithDefault) {
    Document d;
    d.Parse(kJson);

    // Value version
    Document::AllocatorType& a = d.GetAllocator();
    const Value v("qux");
    EXPECT_TRUE(Value("bar") == Pointer("/foo/0").GetWithDefault(d, v, a));
    EXPECT_TRUE(Value("baz") == Pointer("/foo/1").GetWithDefault(d, v, a));
    EXPECT_TRUE(Value("qux") == Pointer("/foo/2").GetWithDefault(d, v, a));
    EXPECT_TRUE(Value("last") == Pointer("/foo/-").GetWithDefault(d, Value("last").Move(), a));
    EXPECT_STREQ("last", d["foo"][3].GetString());

    EXPECT_TRUE(Pointer("/foo/null").GetWithDefault(d, Value().Move(), a).IsNull());
    EXPECT_TRUE(Pointer("/foo/null").GetWithDefault(d, "x", a).IsNull());

    // Generic version
    EXPECT_EQ(-1, Pointer("/foo/int").GetWithDefault(d, -1, a).GetInt());
    EXPECT_EQ(-1, Pointer("/foo/int").GetWithDefault(d, -2, a).GetInt());
    EXPECT_EQ(0x87654321, Pointer("/foo/uint").GetWithDefault(d, 0x87654321, a).GetUint());
    EXPECT_EQ(0x87654321, Pointer("/foo/uint").GetWithDefault(d, 0x12345678, a).GetUint());

    const int64_t i64 = static_cast<int64_t>(RAPIDJSON_UINT64_C2(0x80000000, 0));
    EXPECT_EQ(i64, Pointer("/foo/int64").GetWithDefault(d, i64, a).GetInt64());
    EXPECT_EQ(i64, Pointer("/foo/int64").GetWithDefault(d, i64 + 1, a).GetInt64());

    const uint64_t u64 = RAPIDJSON_UINT64_C2(0xFFFFFFFFF, 0xFFFFFFFFF);
    EXPECT_EQ(u64, Pointer("/foo/uint64").GetWithDefault(d, u64, a).GetUint64());
    EXPECT_EQ(u64, Pointer("/foo/uint64").GetWithDefault(d, u64 - 1, a).GetUint64());

    EXPECT_TRUE(Pointer("/foo/true").GetWithDefault(d, true, a).IsTrue());
    EXPECT_TRUE(Pointer("/foo/true").GetWithDefault(d, false, a).IsTrue());

    EXPECT_TRUE(Pointer("/foo/false").GetWithDefault(d, false, a).IsFalse());
    EXPECT_TRUE(Pointer("/foo/false").GetWithDefault(d, true, a).IsFalse());

    // StringRef version
    EXPECT_STREQ("Hello", Pointer("/foo/hello").GetWithDefault(d, "Hello", a).GetString());

    // Copy string version
    {
        char buffer[256];
        strcpy(buffer, "World");
        EXPECT_STREQ("World", Pointer("/foo/world").GetWithDefault(d, buffer, a).GetString());
        memset(buffer, 0, sizeof(buffer));
    }
    EXPECT_STREQ("World", GetValueByPointer(d, "/foo/world")->GetString());
}

TEST(Pointer, GetWithDefault_NoAllocator) {
    Document d;
    d.Parse(kJson);

    // Value version
    const Value v("qux");
    EXPECT_TRUE(Value("bar") == Pointer("/foo/0").GetWithDefault(d, v));
    EXPECT_TRUE(Value("baz") == Pointer("/foo/1").GetWithDefault(d, v));
    EXPECT_TRUE(Value("qux") == Pointer("/foo/2").GetWithDefault(d, v));
    EXPECT_TRUE(Value("last") == Pointer("/foo/-").GetWithDefault(d, Value("last").Move()));
    EXPECT_STREQ("last", d["foo"][3].GetString());

    EXPECT_TRUE(Pointer("/foo/null").GetWithDefault(d, Value().Move()).IsNull());
    EXPECT_TRUE(Pointer("/foo/null").GetWithDefault(d, "x").IsNull());

    // Generic version
    EXPECT_EQ(-1, Pointer("/foo/int").GetWithDefault(d, -1).GetInt());
    EXPECT_EQ(-1, Pointer("/foo/int").GetWithDefault(d, -2).GetInt());
    EXPECT_EQ(0x87654321, Pointer("/foo/uint").GetWithDefault(d, 0x87654321).GetUint());
    EXPECT_EQ(0x87654321, Pointer("/foo/uint").GetWithDefault(d, 0x12345678).GetUint());

    const int64_t i64 = static_cast<int64_t>(RAPIDJSON_UINT64_C2(0x80000000, 0));
    EXPECT_EQ(i64, Pointer("/foo/int64").GetWithDefault(d, i64).GetInt64());
    EXPECT_EQ(i64, Pointer("/foo/int64").GetWithDefault(d, i64 + 1).GetInt64());

    const uint64_t u64 = RAPIDJSON_UINT64_C2(0xFFFFFFFFF, 0xFFFFFFFFF);
    EXPECT_EQ(u64, Pointer("/foo/uint64").GetWithDefault(d, u64).GetUint64());
    EXPECT_EQ(u64, Pointer("/foo/uint64").GetWithDefault(d, u64 - 1).GetUint64());

    EXPECT_TRUE(Pointer("/foo/true").GetWithDefault(d, true).IsTrue());
    EXPECT_TRUE(Pointer("/foo/true").GetWithDefault(d, false).IsTrue());

    EXPECT_TRUE(Pointer("/foo/false").GetWithDefault(d, false).IsFalse());
    EXPECT_TRUE(Pointer("/foo/false").GetWithDefault(d, true).IsFalse());

    // StringRef version
    EXPECT_STREQ("Hello", Pointer("/foo/hello").GetWithDefault(d, "Hello").GetString());

    // Copy string version
    {
        char buffer[256];
        strcpy(buffer, "World");
        EXPECT_STREQ("World", Pointer("/foo/world").GetWithDefault(d, buffer).GetString());
        memset(buffer, 0, sizeof(buffer));
    }
    EXPECT_STREQ("World", GetValueByPointer(d, "/foo/world")->GetString());
}

TEST(Pointer, Set) {
    Document d;
    d.Parse(kJson);
    Document::AllocatorType& a = d.GetAllocator();
    
    // Value version
    Pointer("/foo/0").Set(d, Value(123).Move(), a);
    EXPECT_EQ(123, d["foo"][0].GetInt());

    Pointer("/foo/-").Set(d, Value(456).Move(), a);
    EXPECT_EQ(456, d["foo"][2].GetInt());

    Pointer("/foo/null").Set(d, Value().Move(), a);
    EXPECT_TRUE(GetValueByPointer(d, "/foo/null")->IsNull());

    // Generic version
    Pointer("/foo/int").Set(d, -1, a);
    EXPECT_EQ(-1, GetValueByPointer(d, "/foo/int")->GetInt());

    Pointer("/foo/uint").Set(d, 0x87654321, a);
    EXPECT_EQ(0x87654321, GetValueByPointer(d, "/foo/uint")->GetUint());

    const int64_t i64 = static_cast<int64_t>(RAPIDJSON_UINT64_C2(0x80000000, 0));
    Pointer("/foo/int64").Set(d, i64, a);
    EXPECT_EQ(i64, GetValueByPointer(d, "/foo/int64")->GetInt64());

    const uint64_t u64 = RAPIDJSON_UINT64_C2(0xFFFFFFFFF, 0xFFFFFFFFF);
    Pointer("/foo/uint64").Set(d, u64, a);
    EXPECT_EQ(u64, GetValueByPointer(d, "/foo/uint64")->GetUint64());

    Pointer("/foo/true").Set(d, true, a);
    EXPECT_TRUE(GetValueByPointer(d, "/foo/true")->IsTrue());

    Pointer("/foo/false").Set(d, false, a);
    EXPECT_TRUE(GetValueByPointer(d, "/foo/false")->IsFalse());

    // StringRef version
    Pointer("/foo/hello").Set(d, "Hello", a);
    EXPECT_STREQ("Hello", GetValueByPointer(d, "/foo/hello")->GetString());

    // Copy string version
    {
        char buffer[256];
        strcpy(buffer, "World");
        Pointer("/foo/world").Set(d, buffer, a);
        memset(buffer, 0, sizeof(buffer));
    }
    EXPECT_STREQ("World", GetValueByPointer(d, "/foo/world")->GetString());
}

TEST(Pointer, Set_NoAllocator) {
    Document d;
    d.Parse(kJson);
    
    // Value version
    Pointer("/foo/0").Set(d, Value(123).Move());
    EXPECT_EQ(123, d["foo"][0].GetInt());

    Pointer("/foo/-").Set(d, Value(456).Move());
    EXPECT_EQ(456, d["foo"][2].GetInt());

    Pointer("/foo/null").Set(d, Value().Move());
    EXPECT_TRUE(GetValueByPointer(d, "/foo/null")->IsNull());

    // Generic version
    Pointer("/foo/int").Set(d, -1);
    EXPECT_EQ(-1, GetValueByPointer(d, "/foo/int")->GetInt());

    Pointer("/foo/uint").Set(d, 0x87654321);
    EXPECT_EQ(0x87654321, GetValueByPointer(d, "/foo/uint")->GetUint());

    const int64_t i64 = static_cast<int64_t>(RAPIDJSON_UINT64_C2(0x80000000, 0));
    Pointer("/foo/int64").Set(d, i64);
    EXPECT_EQ(i64, GetValueByPointer(d, "/foo/int64")->GetInt64());

    const uint64_t u64 = RAPIDJSON_UINT64_C2(0xFFFFFFFFF, 0xFFFFFFFFF);
    Pointer("/foo/uint64").Set(d, u64);
    EXPECT_EQ(u64, GetValueByPointer(d, "/foo/uint64")->GetUint64());

    Pointer("/foo/true").Set(d, true);
    EXPECT_TRUE(GetValueByPointer(d, "/foo/true")->IsTrue());

    Pointer("/foo/false").Set(d, false);
    EXPECT_TRUE(GetValueByPointer(d, "/foo/false")->IsFalse());

    // StringRef version
    Pointer("/foo/hello").Set(d, "Hello");
    EXPECT_STREQ("Hello", GetValueByPointer(d, "/foo/hello")->GetString());

    // Copy string version
    {
        char buffer[256];
        strcpy(buffer, "World");
        Pointer("/foo/world").Set(d, buffer);
        memset(buffer, 0, sizeof(buffer));
    }
    EXPECT_STREQ("World", GetValueByPointer(d, "/foo/world")->GetString());
}

TEST(Pointer, Swap) {
    Document d;
    d.Parse(kJson);
    Document::AllocatorType& a = d.GetAllocator();
    Pointer("/foo/0").Swap(d, *Pointer("/foo/1").Get(d), a);
    EXPECT_STREQ("baz", d["foo"][0].GetString());
    EXPECT_STREQ("bar", d["foo"][1].GetString());
}

TEST(Pointer, Swap_NoAllocator) {
    Document d;
    d.Parse(kJson);
    Pointer("/foo/0").Swap(d, *Pointer("/foo/1").Get(d));
    EXPECT_STREQ("baz", d["foo"][0].GetString());
    EXPECT_STREQ("bar", d["foo"][1].GetString());
}

TEST(Pointer, CreateValueByPointer) {
    Document d;
    Document::AllocatorType& a = d.GetAllocator();

    {
        Value& v = CreateValueByPointer(d, Pointer("/foo/0"), a);
        EXPECT_EQ(&d["foo"][0], &v);
    }
    {
        Value& v = CreateValueByPointer(d, "/foo/1", a);
        EXPECT_EQ(&d["foo"][1], &v);
    }
}

TEST(Pointer, CreateValueByPointer_NoAllocator) {
    Document d;

    {
        Value& v = CreateValueByPointer(d, Pointer("/foo/0"));
        EXPECT_EQ(&d["foo"][0], &v);
    }
    {
        Value& v = CreateValueByPointer(d, "/foo/1");
        EXPECT_EQ(&d["foo"][1], &v);
    }
}

TEST(Pointer, GetValueByPointer) {
    Document d;
    d.Parse(kJson);

    EXPECT_EQ(&d["foo"][0], GetValueByPointer(d, Pointer("/foo/0")));
    EXPECT_EQ(&d["foo"][0], GetValueByPointer(d, "/foo/0"));

    // const version
    const Value& v = d;
    EXPECT_EQ(&d["foo"][0], GetValueByPointer(v, Pointer("/foo/0")));
    EXPECT_EQ(&d["foo"][0], GetValueByPointer(v, "/foo/0"));
}

TEST(Pointer, GetValueByPointerWithDefault_Pointer) {
    Document d;
    d.Parse(kJson);

    Document::AllocatorType& a = d.GetAllocator();
    const Value v("qux");
    EXPECT_TRUE(Value("bar") == GetValueByPointerWithDefault(d, Pointer("/foo/0"), v, a));
    EXPECT_TRUE(Value("bar") == GetValueByPointerWithDefault(d, Pointer("/foo/0"), v, a));
    EXPECT_TRUE(Value("baz") == GetValueByPointerWithDefault(d, Pointer("/foo/1"), v, a));
    EXPECT_TRUE(Value("qux") == GetValueByPointerWithDefault(d, Pointer("/foo/2"), v, a));
    EXPECT_TRUE(Value("last") == GetValueByPointerWithDefault(d, Pointer("/foo/-"), Value("last").Move(), a));
    EXPECT_STREQ("last", d["foo"][3].GetString());

    EXPECT_TRUE(GetValueByPointerWithDefault(d, Pointer("/foo/null"), Value().Move(), a).IsNull());
    EXPECT_TRUE(GetValueByPointerWithDefault(d, Pointer("/foo/null"), "x", a).IsNull());

    // Generic version
    EXPECT_EQ(-1, GetValueByPointerWithDefault(d, Pointer("/foo/int"), -1, a).GetInt());
    EXPECT_EQ(-1, GetValueByPointerWithDefault(d, Pointer("/foo/int"), -2, a).GetInt());
    EXPECT_EQ(0x87654321, GetValueByPointerWithDefault(d, Pointer("/foo/uint"), 0x87654321, a).GetUint());
    EXPECT_EQ(0x87654321, GetValueByPointerWithDefault(d, Pointer("/foo/uint"), 0x12345678, a).GetUint());

    const int64_t i64 = static_cast<int64_t>(RAPIDJSON_UINT64_C2(0x80000000, 0));
    EXPECT_EQ(i64, GetValueByPointerWithDefault(d, Pointer("/foo/int64"), i64, a).GetInt64());
    EXPECT_EQ(i64, GetValueByPointerWithDefault(d, Pointer("/foo/int64"), i64 + 1, a).GetInt64());

    const uint64_t u64 = RAPIDJSON_UINT64_C2(0xFFFFFFFFF, 0xFFFFFFFFF);
    EXPECT_EQ(u64, GetValueByPointerWithDefault(d, Pointer("/foo/uint64"), u64, a).GetUint64());
    EXPECT_EQ(u64, GetValueByPointerWithDefault(d, Pointer("/foo/uint64"), u64 - 1, a).GetUint64());

    EXPECT_TRUE(GetValueByPointerWithDefault(d, Pointer("/foo/true"), true, a).IsTrue());
    EXPECT_TRUE(GetValueByPointerWithDefault(d, Pointer("/foo/true"), false, a).IsTrue());

    EXPECT_TRUE(GetValueByPointerWithDefault(d, Pointer("/foo/false"), false, a).IsFalse());
    EXPECT_TRUE(GetValueByPointerWithDefault(d, Pointer("/foo/false"), true, a).IsFalse());

    // StringRef version
    EXPECT_STREQ("Hello", GetValueByPointerWithDefault(d, Pointer("/foo/hello"), "Hello", a).GetString());

    // Copy string version
    {
        char buffer[256];
        strcpy(buffer, "World");
        EXPECT_STREQ("World", GetValueByPointerWithDefault(d, Pointer("/foo/world"), buffer, a).GetString());
        memset(buffer, 0, sizeof(buffer));
    }
    EXPECT_STREQ("World", GetValueByPointer(d, Pointer("/foo/world"))->GetString());
}

TEST(Pointer, GetValueByPointerWithDefault_String) {
    Document d;
    d.Parse(kJson);

    Document::AllocatorType& a = d.GetAllocator();
    const Value v("qux");
    EXPECT_TRUE(Value("bar") == GetValueByPointerWithDefault(d, "/foo/0", v, a));
    EXPECT_TRUE(Value("bar") == GetValueByPointerWithDefault(d, "/foo/0", v, a));
    EXPECT_TRUE(Value("baz") == GetValueByPointerWithDefault(d, "/foo/1", v, a));
    EXPECT_TRUE(Value("qux") == GetValueByPointerWithDefault(d, "/foo/2", v, a));
    EXPECT_TRUE(Value("last") == GetValueByPointerWithDefault(d, "/foo/-", Value("last").Move(), a));
    EXPECT_STREQ("last", d["foo"][3].GetString());

    EXPECT_TRUE(GetValueByPointerWithDefault(d, "/foo/null", Value().Move(), a).IsNull());
    EXPECT_TRUE(GetValueByPointerWithDefault(d, "/foo/null", "x", a).IsNull());

    // Generic version
    EXPECT_EQ(-1, GetValueByPointerWithDefault(d, "/foo/int", -1, a).GetInt());
    EXPECT_EQ(-1, GetValueByPointerWithDefault(d, "/foo/int", -2, a).GetInt());
    EXPECT_EQ(0x87654321, GetValueByPointerWithDefault(d, "/foo/uint", 0x87654321, a).GetUint());
    EXPECT_EQ(0x87654321, GetValueByPointerWithDefault(d, "/foo/uint", 0x12345678, a).GetUint());

    const int64_t i64 = static_cast<int64_t>(RAPIDJSON_UINT64_C2(0x80000000, 0));
    EXPECT_EQ(i64, GetValueByPointerWithDefault(d, "/foo/int64", i64, a).GetInt64());
    EXPECT_EQ(i64, GetValueByPointerWithDefault(d, "/foo/int64", i64 + 1, a).GetInt64());

    const uint64_t u64 = RAPIDJSON_UINT64_C2(0xFFFFFFFFF, 0xFFFFFFFFF);
    EXPECT_EQ(u64, GetValueByPointerWithDefault(d, "/foo/uint64", u64, a).GetUint64());
    EXPECT_EQ(u64, GetValueByPointerWithDefault(d, "/foo/uint64", u64 - 1, a).GetUint64());

    EXPECT_TRUE(GetValueByPointerWithDefault(d, "/foo/true", true, a).IsTrue());
    EXPECT_TRUE(GetValueByPointerWithDefault(d, "/foo/true", false, a).IsTrue());

    EXPECT_TRUE(GetValueByPointerWithDefault(d, "/foo/false", false, a).IsFalse());
    EXPECT_TRUE(GetValueByPointerWithDefault(d, "/foo/false", true, a).IsFalse());

    // StringRef version
    EXPECT_STREQ("Hello", GetValueByPointerWithDefault(d, "/foo/hello", "Hello", a).GetString());

    // Copy string version
    {
        char buffer[256];
        strcpy(buffer, "World");
        EXPECT_STREQ("World", GetValueByPointerWithDefault(d, "/foo/world", buffer, a).GetString());
        memset(buffer, 0, sizeof(buffer));
    }
    EXPECT_STREQ("World", GetValueByPointer(d, "/foo/world")->GetString());
}

TEST(Pointer, GetValueByPointerWithDefault_Pointer_NoAllocator) {
    Document d;
    d.Parse(kJson);

    const Value v("qux");
    EXPECT_TRUE(Value("bar") == GetValueByPointerWithDefault(d, Pointer("/foo/0"), v));
    EXPECT_TRUE(Value("bar") == GetValueByPointerWithDefault(d, Pointer("/foo/0"), v));
    EXPECT_TRUE(Value("baz") == GetValueByPointerWithDefault(d, Pointer("/foo/1"), v));
    EXPECT_TRUE(Value("qux") == GetValueByPointerWithDefault(d, Pointer("/foo/2"), v));
    EXPECT_TRUE(Value("last") == GetValueByPointerWithDefault(d, Pointer("/foo/-"), Value("last").Move()));
    EXPECT_STREQ("last", d["foo"][3].GetString());

    EXPECT_TRUE(GetValueByPointerWithDefault(d, Pointer("/foo/null"), Value().Move()).IsNull());
    EXPECT_TRUE(GetValueByPointerWithDefault(d, Pointer("/foo/null"), "x").IsNull());

    // Generic version
    EXPECT_EQ(-1, GetValueByPointerWithDefault(d, Pointer("/foo/int"), -1).GetInt());
    EXPECT_EQ(-1, GetValueByPointerWithDefault(d, Pointer("/foo/int"), -2).GetInt());
    EXPECT_EQ(0x87654321, GetValueByPointerWithDefault(d, Pointer("/foo/uint"), 0x87654321).GetUint());
    EXPECT_EQ(0x87654321, GetValueByPointerWithDefault(d, Pointer("/foo/uint"), 0x12345678).GetUint());

    const int64_t i64 = static_cast<int64_t>(RAPIDJSON_UINT64_C2(0x80000000, 0));
    EXPECT_EQ(i64, GetValueByPointerWithDefault(d, Pointer("/foo/int64"), i64).GetInt64());
    EXPECT_EQ(i64, GetValueByPointerWithDefault(d, Pointer("/foo/int64"), i64 + 1).GetInt64());

    const uint64_t u64 = RAPIDJSON_UINT64_C2(0xFFFFFFFFF, 0xFFFFFFFFF);
    EXPECT_EQ(u64, GetValueByPointerWithDefault(d, Pointer("/foo/uint64"), u64).GetUint64());
    EXPECT_EQ(u64, GetValueByPointerWithDefault(d, Pointer("/foo/uint64"), u64 - 1).GetUint64());

    EXPECT_TRUE(GetValueByPointerWithDefault(d, Pointer("/foo/true"), true).IsTrue());
    EXPECT_TRUE(GetValueByPointerWithDefault(d, Pointer("/foo/true"), false).IsTrue());

    EXPECT_TRUE(GetValueByPointerWithDefault(d, Pointer("/foo/false"), false).IsFalse());
    EXPECT_TRUE(GetValueByPointerWithDefault(d, Pointer("/foo/false"), true).IsFalse());

    // StringRef version
    EXPECT_STREQ("Hello", GetValueByPointerWithDefault(d, Pointer("/foo/hello"), "Hello").GetString());

    // Copy string version
    {
        char buffer[256];
        strcpy(buffer, "World");
        EXPECT_STREQ("World", GetValueByPointerWithDefault(d, Pointer("/foo/world"), buffer).GetString());
        memset(buffer, 0, sizeof(buffer));
    }
    EXPECT_STREQ("World", GetValueByPointer(d, Pointer("/foo/world"))->GetString());
}

TEST(Pointer, GetValueByPointerWithDefault_String_NoAllocator) {
    Document d;
    d.Parse(kJson);

    const Value v("qux");
    EXPECT_TRUE(Value("bar") == GetValueByPointerWithDefault(d, "/foo/0", v));
    EXPECT_TRUE(Value("bar") == GetValueByPointerWithDefault(d, "/foo/0", v));
    EXPECT_TRUE(Value("baz") == GetValueByPointerWithDefault(d, "/foo/1", v));
    EXPECT_TRUE(Value("qux") == GetValueByPointerWithDefault(d, "/foo/2", v));
    EXPECT_TRUE(Value("last") == GetValueByPointerWithDefault(d, "/foo/-", Value("last").Move()));
    EXPECT_STREQ("last", d["foo"][3].GetString());

    EXPECT_TRUE(GetValueByPointerWithDefault(d, "/foo/null", Value().Move()).IsNull());
    EXPECT_TRUE(GetValueByPointerWithDefault(d, "/foo/null", "x").IsNull());

    // Generic version
    EXPECT_EQ(-1, GetValueByPointerWithDefault(d, "/foo/int", -1).GetInt());
    EXPECT_EQ(-1, GetValueByPointerWithDefault(d, "/foo/int", -2).GetInt());
    EXPECT_EQ(0x87654321, GetValueByPointerWithDefault(d, "/foo/uint", 0x87654321).GetUint());
    EXPECT_EQ(0x87654321, GetValueByPointerWithDefault(d, "/foo/uint", 0x12345678).GetUint());

    const int64_t i64 = static_cast<int64_t>(RAPIDJSON_UINT64_C2(0x80000000, 0));
    EXPECT_EQ(i64, GetValueByPointerWithDefault(d, "/foo/int64", i64).GetInt64());
    EXPECT_EQ(i64, GetValueByPointerWithDefault(d, "/foo/int64", i64 + 1).GetInt64());

    const uint64_t u64 = RAPIDJSON_UINT64_C2(0xFFFFFFFFF, 0xFFFFFFFFF);
    EXPECT_EQ(u64, GetValueByPointerWithDefault(d, "/foo/uint64", u64).GetUint64());
    EXPECT_EQ(u64, GetValueByPointerWithDefault(d, "/foo/uint64", u64 - 1).GetUint64());

    EXPECT_TRUE(GetValueByPointerWithDefault(d, "/foo/true", true).IsTrue());
    EXPECT_TRUE(GetValueByPointerWithDefault(d, "/foo/true", false).IsTrue());

    EXPECT_TRUE(GetValueByPointerWithDefault(d, "/foo/false", false).IsFalse());
    EXPECT_TRUE(GetValueByPointerWithDefault(d, "/foo/false", true).IsFalse());

    // StringRef version
    EXPECT_STREQ("Hello", GetValueByPointerWithDefault(d, "/foo/hello", "Hello").GetString());

    // Copy string version
    {
        char buffer[256];
        strcpy(buffer, "World");
        EXPECT_STREQ("World", GetValueByPointerWithDefault(d, "/foo/world", buffer).GetString());
        memset(buffer, 0, sizeof(buffer));
    }
    EXPECT_STREQ("World", GetValueByPointer(d, "/foo/world")->GetString());
}

TEST(Pointer, SetValueByPointer_Pointer) {
    Document d;
    d.Parse(kJson);
    Document::AllocatorType& a = d.GetAllocator();

    // Value version
    SetValueByPointer(d, Pointer("/foo/0"), Value(123).Move(), a);
    EXPECT_EQ(123, d["foo"][0].GetInt());

    SetValueByPointer(d, Pointer("/foo/null"), Value().Move(), a);
    EXPECT_TRUE(GetValueByPointer(d, "/foo/null")->IsNull());

    // Generic version
    SetValueByPointer(d, Pointer("/foo/int"), -1, a);
    EXPECT_EQ(-1, GetValueByPointer(d, "/foo/int")->GetInt());

    SetValueByPointer(d, Pointer("/foo/uint"), 0x87654321, a);
    EXPECT_EQ(0x87654321, GetValueByPointer(d, "/foo/uint")->GetUint());

    const int64_t i64 = static_cast<int64_t>(RAPIDJSON_UINT64_C2(0x80000000, 0));
    SetValueByPointer(d, Pointer("/foo/int64"), i64, a);
    EXPECT_EQ(i64, GetValueByPointer(d, "/foo/int64")->GetInt64());

    const uint64_t u64 = RAPIDJSON_UINT64_C2(0xFFFFFFFFF, 0xFFFFFFFFF);
    SetValueByPointer(d, Pointer("/foo/uint64"), u64, a);
    EXPECT_EQ(u64, GetValueByPointer(d, "/foo/uint64")->GetUint64());

    SetValueByPointer(d, Pointer("/foo/true"), true, a);
    EXPECT_TRUE(GetValueByPointer(d, "/foo/true")->IsTrue());

    SetValueByPointer(d, Pointer("/foo/false"), false, a);
    EXPECT_TRUE(GetValueByPointer(d, "/foo/false")->IsFalse());

    // StringRef version
    SetValueByPointer(d, Pointer("/foo/hello"), "Hello", a);
    EXPECT_STREQ("Hello", GetValueByPointer(d, "/foo/hello")->GetString());

    // Copy string version
    {
        char buffer[256];
        strcpy(buffer, "World");
        SetValueByPointer(d, Pointer("/foo/world"), buffer, a);
        memset(buffer, 0, sizeof(buffer));
    }
    EXPECT_STREQ("World", GetValueByPointer(d, "/foo/world")->GetString());
}

TEST(Pointer, SetValueByPointer_String) {
    Document d;
    d.Parse(kJson);
    Document::AllocatorType& a = d.GetAllocator();

    // Value version
    SetValueByPointer(d, "/foo/0", Value(123).Move(), a);
    EXPECT_EQ(123, d["foo"][0].GetInt());

    SetValueByPointer(d, "/foo/null", Value().Move(), a);
    EXPECT_TRUE(GetValueByPointer(d, "/foo/null")->IsNull());

    // Generic version
    SetValueByPointer(d, "/foo/int", -1, a);
    EXPECT_EQ(-1, GetValueByPointer(d, "/foo/int")->GetInt());

    SetValueByPointer(d, "/foo/uint", 0x87654321, a);
    EXPECT_EQ(0x87654321, GetValueByPointer(d, "/foo/uint")->GetUint());

    const int64_t i64 = static_cast<int64_t>(RAPIDJSON_UINT64_C2(0x80000000, 0));
    SetValueByPointer(d, "/foo/int64", i64, a);
    EXPECT_EQ(i64, GetValueByPointer(d, "/foo/int64")->GetInt64());

    const uint64_t u64 = RAPIDJSON_UINT64_C2(0xFFFFFFFFF, 0xFFFFFFFFF);
    SetValueByPointer(d, "/foo/uint64", u64, a);
    EXPECT_EQ(u64, GetValueByPointer(d, "/foo/uint64")->GetUint64());

    SetValueByPointer(d, "/foo/true", true, a);
    EXPECT_TRUE(GetValueByPointer(d, "/foo/true")->IsTrue());

    SetValueByPointer(d, "/foo/false", false, a);
    EXPECT_TRUE(GetValueByPointer(d, "/foo/false")->IsFalse());

    // StringRef version
    SetValueByPointer(d, "/foo/hello", "Hello", a);
    EXPECT_STREQ("Hello", GetValueByPointer(d, "/foo/hello")->GetString());

    // Copy string version
    {
        char buffer[256];
        strcpy(buffer, "World");
        SetValueByPointer(d, "/foo/world", buffer, a);
        memset(buffer, 0, sizeof(buffer));
    }
    EXPECT_STREQ("World", GetValueByPointer(d, "/foo/world")->GetString());
}

TEST(Pointer, SetValueByPointer_Pointer_NoAllocator) {
    Document d;
    d.Parse(kJson);

    // Value version
    SetValueByPointer(d, Pointer("/foo/0"), Value(123).Move());
    EXPECT_EQ(123, d["foo"][0].GetInt());

    SetValueByPointer(d, Pointer("/foo/null"), Value().Move());
    EXPECT_TRUE(GetValueByPointer(d, "/foo/null")->IsNull());

    // Generic version
    SetValueByPointer(d, Pointer("/foo/int"), -1);
    EXPECT_EQ(-1, GetValueByPointer(d, "/foo/int")->GetInt());

    SetValueByPointer(d, Pointer("/foo/uint"), 0x87654321);
    EXPECT_EQ(0x87654321, GetValueByPointer(d, "/foo/uint")->GetUint());

    const int64_t i64 = static_cast<int64_t>(RAPIDJSON_UINT64_C2(0x80000000, 0));
    SetValueByPointer(d, Pointer("/foo/int64"), i64);
    EXPECT_EQ(i64, GetValueByPointer(d, "/foo/int64")->GetInt64());

    const uint64_t u64 = RAPIDJSON_UINT64_C2(0xFFFFFFFFF, 0xFFFFFFFFF);
    SetValueByPointer(d, Pointer("/foo/uint64"), u64);
    EXPECT_EQ(u64, GetValueByPointer(d, "/foo/uint64")->GetUint64());

    SetValueByPointer(d, Pointer("/foo/true"), true);
    EXPECT_TRUE(GetValueByPointer(d, "/foo/true")->IsTrue());

    SetValueByPointer(d, Pointer("/foo/false"), false);
    EXPECT_TRUE(GetValueByPointer(d, "/foo/false")->IsFalse());

    // StringRef version
    SetValueByPointer(d, Pointer("/foo/hello"), "Hello");
    EXPECT_STREQ("Hello", GetValueByPointer(d, "/foo/hello")->GetString());

    // Copy string version
    {
        char buffer[256];
        strcpy(buffer, "World");
        SetValueByPointer(d, Pointer("/foo/world"), buffer);
        memset(buffer, 0, sizeof(buffer));
    }
    EXPECT_STREQ("World", GetValueByPointer(d, "/foo/world")->GetString());
}

TEST(Pointer, SetValueByPointer_String_NoAllocator) {
    Document d;
    d.Parse(kJson);

    // Value version
    SetValueByPointer(d, "/foo/0", Value(123).Move());
    EXPECT_EQ(123, d["foo"][0].GetInt());

    SetValueByPointer(d, "/foo/null", Value().Move());
    EXPECT_TRUE(GetValueByPointer(d, "/foo/null")->IsNull());

    // Generic version
    SetValueByPointer(d, "/foo/int", -1);
    EXPECT_EQ(-1, GetValueByPointer(d, "/foo/int")->GetInt());

    SetValueByPointer(d, "/foo/uint", 0x87654321);
    EXPECT_EQ(0x87654321, GetValueByPointer(d, "/foo/uint")->GetUint());

    const int64_t i64 = static_cast<int64_t>(RAPIDJSON_UINT64_C2(0x80000000, 0));
    SetValueByPointer(d, "/foo/int64", i64);
    EXPECT_EQ(i64, GetValueByPointer(d, "/foo/int64")->GetInt64());

    const uint64_t u64 = RAPIDJSON_UINT64_C2(0xFFFFFFFFF, 0xFFFFFFFFF);
    SetValueByPointer(d, "/foo/uint64", u64);
    EXPECT_EQ(u64, GetValueByPointer(d, "/foo/uint64")->GetUint64());

    SetValueByPointer(d, "/foo/true", true);
    EXPECT_TRUE(GetValueByPointer(d, "/foo/true")->IsTrue());

    SetValueByPointer(d, "/foo/false", false);
    EXPECT_TRUE(GetValueByPointer(d, "/foo/false")->IsFalse());

    // StringRef version
    SetValueByPointer(d, "/foo/hello", "Hello");
    EXPECT_STREQ("Hello", GetValueByPointer(d, "/foo/hello")->GetString());

    // Copy string version
    {
        char buffer[256];
        strcpy(buffer, "World");
        SetValueByPointer(d, "/foo/world", buffer);
        memset(buffer, 0, sizeof(buffer));
    }
    EXPECT_STREQ("World", GetValueByPointer(d, "/foo/world")->GetString());
}

TEST(Pointer, SwapValueByPointer) {
    Document d;
    d.Parse(kJson);
    Document::AllocatorType& a = d.GetAllocator();
    SwapValueByPointer(d, Pointer("/foo/0"), *GetValueByPointer(d, "/foo/1"), a);
    EXPECT_STREQ("baz", d["foo"][0].GetString());
    EXPECT_STREQ("bar", d["foo"][1].GetString());

    SwapValueByPointer(d, "/foo/0", *GetValueByPointer(d, "/foo/1"), a);
    EXPECT_STREQ("bar", d["foo"][0].GetString());
    EXPECT_STREQ("baz", d["foo"][1].GetString());
}

TEST(Pointer, SwapValueByPointer_NoAllocator) {
    Document d;
    d.Parse(kJson);
    SwapValueByPointer(d, Pointer("/foo/0"), *GetValueByPointer(d, "/foo/1"));
    EXPECT_STREQ("baz", d["foo"][0].GetString());
    EXPECT_STREQ("bar", d["foo"][1].GetString());

    SwapValueByPointer(d, "/foo/0", *GetValueByPointer(d, "/foo/1"));
    EXPECT_STREQ("bar", d["foo"][0].GetString());
    EXPECT_STREQ("baz", d["foo"][1].GetString());
}