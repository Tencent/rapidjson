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
#include "rapidjson/writer.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/encodedstream.h"
#include "rapidjson/stringbuffer.h"
#include <sstream>

using namespace rapidjson;

template <typename Allocator, typename StackAllocator>
void ParseTest() {
    typedef GenericDocument<UTF8<>, Allocator, StackAllocator> DocumentType;
    typedef typename DocumentType::ValueType ValueType;
    DocumentType doc;

    doc.Parse(" { \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3, 4] } ");

    EXPECT_TRUE(doc.IsObject());

    EXPECT_TRUE(doc.HasMember("hello"));
    const ValueType& hello = doc["hello"];
    EXPECT_TRUE(hello.IsString());
    EXPECT_STREQ("world", hello.GetString());

    EXPECT_TRUE(doc.HasMember("t"));
    const ValueType& t = doc["t"];
    EXPECT_TRUE(t.IsTrue());

    EXPECT_TRUE(doc.HasMember("f"));
    const ValueType& f = doc["f"];
    EXPECT_TRUE(f.IsFalse());

    EXPECT_TRUE(doc.HasMember("n"));
    const ValueType& n = doc["n"];
    EXPECT_TRUE(n.IsNull());

    EXPECT_TRUE(doc.HasMember("i"));
    const ValueType& i = doc["i"];
    EXPECT_TRUE(i.IsNumber());
    EXPECT_EQ(123, i.GetInt());

    EXPECT_TRUE(doc.HasMember("pi"));
    const ValueType& pi = doc["pi"];
    EXPECT_TRUE(pi.IsNumber());
    EXPECT_EQ(3.1416, pi.GetDouble());

    EXPECT_TRUE(doc.HasMember("a"));
    const ValueType& a = doc["a"];
    EXPECT_TRUE(a.IsArray());
    EXPECT_EQ(4u, a.Size());
    for (SizeType i = 0; i < 4; i++)
        EXPECT_EQ(i + 1, a[i].GetUint());
}

TEST(Document, Parse) {
    ParseTest<MemoryPoolAllocator<>, CrtAllocator>();
    ParseTest<MemoryPoolAllocator<>, MemoryPoolAllocator<> >();
    ParseTest<CrtAllocator, MemoryPoolAllocator<> >();
    ParseTest<CrtAllocator, CrtAllocator>();
}

static FILE* OpenEncodedFile(const char* filename) {
    char buffer[1024];
    sprintf(buffer, "encodings/%s", filename);
    FILE *fp = fopen(buffer, "rb");
    if (!fp) {
        sprintf(buffer, "../../bin/encodings/%s", filename);
        fp = fopen(buffer, "rb");
    }
    return fp;
}

TEST(Document, ParseStream_EncodedInputStream) {
    // UTF8 -> UTF16
    FILE* fp = OpenEncodedFile("utf8.json");
    char buffer[256];
    FileReadStream bis(fp, buffer, sizeof(buffer));
    EncodedInputStream<UTF8<>, FileReadStream> eis(bis);

    GenericDocument<UTF16<> > d;
    d.ParseStream<0, UTF8<> >(eis);
    EXPECT_FALSE(d.HasParseError());

    fclose(fp);

    wchar_t expected[] = L"I can eat glass and it doesn't hurt me.";
    GenericValue<UTF16<> >& v = d[L"en"];
    EXPECT_TRUE(v.IsString());
    EXPECT_EQ(sizeof(expected) / sizeof(wchar_t) - 1, v.GetStringLength());
    EXPECT_EQ(0, StrCmp(expected, v.GetString()));

    // UTF16 -> UTF8 in memory
    StringBuffer bos;
    typedef EncodedOutputStream<UTF8<>, StringBuffer> OutputStream;
    OutputStream eos(bos, false);   // Not writing BOM
    Writer<OutputStream, UTF16<>, UTF8<> > writer(eos);
    d.Accept(writer);

    {
        // Condense the original file and compare.
        FILE *fp = OpenEncodedFile("utf8.json");
        FileReadStream is(fp, buffer, sizeof(buffer));
        Reader reader;
        StringBuffer bos2;
        Writer<StringBuffer> writer(bos2);
        reader.Parse(is, writer);

        EXPECT_EQ(bos.GetSize(), bos2.GetSize());
        EXPECT_EQ(0, memcmp(bos.GetString(), bos2.GetString(), bos2.GetSize()));
    }
}

TEST(Document, ParseStream_AutoUTFInputStream) {
    // Any -> UTF8
    FILE* fp = OpenEncodedFile("utf32be.json");
    char buffer[256];
    FileReadStream bis(fp, buffer, sizeof(buffer));
    AutoUTFInputStream<unsigned, FileReadStream> eis(bis);

    Document d;
    d.ParseStream<0, AutoUTF<unsigned> >(eis);
    EXPECT_FALSE(d.HasParseError());

    fclose(fp);

    char expected[] = "I can eat glass and it doesn't hurt me.";
    Value& v = d["en"];
    EXPECT_TRUE(v.IsString());
    EXPECT_EQ(sizeof(expected) - 1, v.GetStringLength());
    EXPECT_EQ(0, StrCmp(expected, v.GetString()));

    // UTF8 -> UTF8 in memory
    StringBuffer bos;
    Writer<StringBuffer> writer(bos);
    d.Accept(writer);

    {
        // Condense the original file and compare.
        FILE *fp = OpenEncodedFile("utf8.json");
        FileReadStream is(fp, buffer, sizeof(buffer));
        Reader reader;
        StringBuffer bos2;
        Writer<StringBuffer> writer(bos2);
        reader.Parse(is, writer);

        EXPECT_EQ(bos.GetSize(), bos2.GetSize());
        EXPECT_EQ(0, memcmp(bos.GetString(), bos2.GetString(), bos2.GetSize()));
    }
}

TEST(Document, Swap) {
    Document d1;
    Document::AllocatorType& a = d1.GetAllocator();

    d1.SetArray().PushBack(1, a).PushBack(2, a);

    Value o;
    o.SetObject().AddMember("a", 1, a);

    // Swap between Document and Value
    d1.Swap(o);
    EXPECT_TRUE(d1.IsObject());
    EXPECT_TRUE(o.IsArray());

    // Swap between Document and Document
    Document d2;
    d2.SetArray().PushBack(3, a);
    d1.Swap(d2);
    EXPECT_TRUE(d1.IsArray());
    EXPECT_TRUE(d2.IsObject());
}

// This should be slow due to assignment in inner-loop.
struct OutputStringStream : public std::ostringstream {
    typedef char Ch;

    void Put(char c) {
        put(c);
    }
    void Flush() {}
};

TEST(Document, AcceptWriter) {
    Document doc;
    doc.Parse(" { \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3, 4] } ");

    OutputStringStream os;
    Writer<OutputStringStream> writer(os);
    doc.Accept(writer);

    EXPECT_EQ("{\"hello\":\"world\",\"t\":true,\"f\":false,\"n\":null,\"i\":123,\"pi\":3.1416,\"a\":[1,2,3,4]}", os.str());
}

// Issue 44:    SetStringRaw doesn't work with wchar_t
TEST(Document, UTF16_Document) {
    GenericDocument< UTF16<> > json;
    json.Parse<kParseValidateEncodingFlag>(L"[{\"created_at\":\"Wed Oct 30 17:13:20 +0000 2012\"}]");

    ASSERT_TRUE(json.IsArray());
    GenericValue< UTF16<> >& v = json[0];
    ASSERT_TRUE(v.IsObject());

    GenericValue< UTF16<> >& s = v[L"created_at"];
    ASSERT_TRUE(s.IsString());

    EXPECT_EQ(0, wcscmp(L"Wed Oct 30 17:13:20 +0000 2012", s.GetString()));
}

#if RAPIDJSON_HAS_CXX11_RVALUE_REFS

TEST(Document, MoveConstructor) {
    typedef GenericDocument<UTF8<>, CrtAllocator> Document;
    Document::AllocatorType allocator;

    Document a(&allocator);
    a.Parse("[\"one\", \"two\", \"three\"]");
    EXPECT_FALSE(a.HasParseError());
    EXPECT_TRUE(a.IsArray());
    EXPECT_EQ(3u, a.Size());
    EXPECT_EQ(&a.GetAllocator(), &allocator);

    // Document b(a); // should not compile
    Document b(std::move(a));
    EXPECT_TRUE(a.IsNull());
    EXPECT_TRUE(b.IsArray());
    EXPECT_EQ(3u, b.Size());
    EXPECT_EQ(&a.GetAllocator(), (void*)0);
    EXPECT_EQ(&b.GetAllocator(), &allocator);

    b.Parse("{\"Foo\": \"Bar\", \"Baz\": 42}");
    EXPECT_FALSE(b.HasParseError());
    EXPECT_TRUE(b.IsObject());
    EXPECT_EQ(2u, b.MemberCount());

    // Document c = a; // should not compile
    Document c = std::move(b);
    EXPECT_TRUE(b.IsNull());
    EXPECT_TRUE(c.IsObject());
    EXPECT_EQ(2u, c.MemberCount());
    EXPECT_EQ(&b.GetAllocator(), (void*)0);
    EXPECT_EQ(&c.GetAllocator(), &allocator);
}

TEST(Document, MoveConstructorParseError) {
    typedef GenericDocument<UTF8<>, CrtAllocator> Document;

    ParseResult noError;
    Document a;
    a.Parse("{ 4 = 4]");
    ParseResult error(a.GetParseError(), a.GetErrorOffset());
    EXPECT_TRUE(a.HasParseError());
    EXPECT_NE(error.Code(), noError.Code());
    EXPECT_NE(error.Offset(), noError.Offset());

    Document b(std::move(a));
    EXPECT_FALSE(a.HasParseError());
    EXPECT_TRUE(b.HasParseError());
    EXPECT_EQ(a.GetParseError(), noError.Code());
    EXPECT_EQ(b.GetParseError(), error.Code());
    EXPECT_EQ(a.GetErrorOffset(), noError.Offset());
    EXPECT_EQ(b.GetErrorOffset(), error.Offset());

    Document c(std::move(b));
    EXPECT_FALSE(b.HasParseError());
    EXPECT_TRUE(c.HasParseError());
    EXPECT_EQ(b.GetParseError(), noError.Code());
    EXPECT_EQ(c.GetParseError(), error.Code());
    EXPECT_EQ(b.GetErrorOffset(), noError.Offset());
    EXPECT_EQ(c.GetErrorOffset(), error.Offset());
}

TEST(Document, MoveConstructorStack) {
    typedef UTF8<> Encoding;
    typedef GenericDocument<Encoding, CrtAllocator> Document;

    Document a;
    size_t defaultCapacity = a.GetStackCapacity();

    // Trick Document into getting GetStackCapacity() to return non-zero
    typedef GenericReader<Encoding, Encoding, Document::AllocatorType> Reader;
    Reader reader(&a.GetAllocator());
    GenericStringStream<Encoding> is("[\"one\", \"two\", \"three\"]");
    reader.Parse<kParseDefaultFlags>(is, a);
    size_t capacity = a.GetStackCapacity();
    EXPECT_GT(capacity, 0);

    Document b(std::move(a));
    EXPECT_EQ(a.GetStackCapacity(), defaultCapacity);
    EXPECT_EQ(b.GetStackCapacity(), capacity);

    Document c = std::move(b);
    EXPECT_EQ(b.GetStackCapacity(), defaultCapacity);
    EXPECT_EQ(c.GetStackCapacity(), capacity);
}

TEST(Document, MoveAssignment) {
    typedef GenericDocument<UTF8<>, CrtAllocator> Document;
    Document::AllocatorType allocator;

    Document a(&allocator);
    a.Parse("[\"one\", \"two\", \"three\"]");
    EXPECT_FALSE(a.HasParseError());
    EXPECT_TRUE(a.IsArray());
    EXPECT_EQ(3u, a.Size());
    EXPECT_EQ(&a.GetAllocator(), &allocator);

    // Document b; b = a; // should not compile
    Document b;
    b = std::move(a);
    EXPECT_TRUE(a.IsNull());
    EXPECT_TRUE(b.IsArray());
    EXPECT_EQ(3u, b.Size());
    EXPECT_EQ(&a.GetAllocator(), (void*)0);
    EXPECT_EQ(&b.GetAllocator(), &allocator);

    b.Parse("{\"Foo\": \"Bar\", \"Baz\": 42}");
    EXPECT_FALSE(b.HasParseError());
    EXPECT_TRUE(b.IsObject());
    EXPECT_EQ(2u, b.MemberCount());

    // Document c; c = a; // should not compile
    Document c;
    c = std::move(b);
    EXPECT_TRUE(b.IsNull());
    EXPECT_TRUE(c.IsObject());
    EXPECT_EQ(2u, c.MemberCount());
    EXPECT_EQ(&b.GetAllocator(), (void*)0);
    EXPECT_EQ(&c.GetAllocator(), &allocator);
}

TEST(Document, MoveAssignmentParseError) {
    typedef GenericDocument<UTF8<>, CrtAllocator> Document;

    ParseResult noError;
    Document a;
    a.Parse("{ 4 = 4]");
    ParseResult error(a.GetParseError(), a.GetErrorOffset());
    EXPECT_TRUE(a.HasParseError());
    EXPECT_NE(error.Code(), noError.Code());
    EXPECT_NE(error.Offset(), noError.Offset());

    Document b;
    b = std::move(a);
    EXPECT_FALSE(a.HasParseError());
    EXPECT_TRUE(b.HasParseError());
    EXPECT_EQ(a.GetParseError(), noError.Code());
    EXPECT_EQ(b.GetParseError(), error.Code());
    EXPECT_EQ(a.GetErrorOffset(), noError.Offset());
    EXPECT_EQ(b.GetErrorOffset(), error.Offset());

    Document c;
    c = std::move(b);
    EXPECT_FALSE(b.HasParseError());
    EXPECT_TRUE(c.HasParseError());
    EXPECT_EQ(b.GetParseError(), noError.Code());
    EXPECT_EQ(c.GetParseError(), error.Code());
    EXPECT_EQ(b.GetErrorOffset(), noError.Offset());
    EXPECT_EQ(c.GetErrorOffset(), error.Offset());
}

TEST(Document, MoveAssignmentStack) {
    typedef UTF8<> Encoding;
    typedef GenericDocument<Encoding, CrtAllocator> Document;

    Document a;
    size_t defaultCapacity = a.GetStackCapacity();

    // Trick Document into getting GetStackCapacity() to return non-zero
    typedef GenericReader<Encoding, Encoding, Document::AllocatorType> Reader;
    Reader reader(&a.GetAllocator());
    GenericStringStream<Encoding> is("[\"one\", \"two\", \"three\"]");
    reader.Parse<kParseDefaultFlags>(is, a);
    size_t capacity = a.GetStackCapacity();
    EXPECT_GT(capacity, 0);

    Document b;
    b = std::move(a);
    EXPECT_EQ(a.GetStackCapacity(), defaultCapacity);
    EXPECT_EQ(b.GetStackCapacity(), capacity);

    Document c;
    c = std::move(b);
    EXPECT_EQ(b.GetStackCapacity(), defaultCapacity);
    EXPECT_EQ(c.GetStackCapacity(), capacity);
}

#endif // RAPIDJSON_HAS_CXX11_RVALUE_REFS

// Issue 22: Memory corruption via operator=
// Fixed by making unimplemented assignment operator private.
//TEST(Document, Assignment) {
//  Document d1;
//  Document d2;
//  d1 = d2;
//}
