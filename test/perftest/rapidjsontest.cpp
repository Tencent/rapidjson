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

#include "perftest.h"

#if TEST_RAPIDJSON

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filestream.h"
#include "rapidjson/filereadstream.h"

#ifdef RAPIDJSON_SSE2
#define SIMD_SUFFIX(name) name##_SSE2
#elif defined(RAPIDJSON_SSE42)
#define SIMD_SUFFIX(name) name##_SSE42
#else
#define SIMD_SUFFIX(name) name
#endif

using namespace rapidjson;

class RapidJson : public PerfTest {
public:
    RapidJson() : temp_(), doc_() {}

    virtual void SetUp() {
        PerfTest::SetUp();

        // temp buffer for insitu parsing.
        temp_ = (char *)malloc(length_ + 1);

        // Parse as a document
        EXPECT_FALSE(doc_.Parse(json_).IsNull());
    }

    virtual void TearDown() {
        PerfTest::TearDown();
        free(temp_);
    }

private:
    RapidJson(const RapidJson&);
    RapidJson& operator=(const RapidJson&);

protected:
    char *temp_;
    Document doc_;
};

TEST_F(RapidJson, SIMD_SUFFIX(ReaderParseInsitu_DummyHandler)) {
    for (size_t i = 0; i < kTrialCount; i++) {
        memcpy(temp_, json_, length_ + 1);
        InsituStringStream s(temp_);
        BaseReaderHandler<> h;
        Reader reader;
        EXPECT_TRUE(reader.Parse<kParseInsituFlag>(s, h));
    }
}

TEST_F(RapidJson, SIMD_SUFFIX(ReaderParseInsitu_DummyHandler_ValidateEncoding)) {
    for (size_t i = 0; i < kTrialCount; i++) {
        memcpy(temp_, json_, length_ + 1);
        InsituStringStream s(temp_);
        BaseReaderHandler<> h;
        Reader reader;
        EXPECT_TRUE(reader.Parse<kParseInsituFlag | kParseValidateEncodingFlag>(s, h));
    }
}

TEST_F(RapidJson, SIMD_SUFFIX(ReaderParse_DummyHandler)) {
    for (size_t i = 0; i < kTrialCount; i++) {
        StringStream s(json_);
        BaseReaderHandler<> h;
        Reader reader;
        EXPECT_TRUE(reader.Parse(s, h));
    }
}

TEST_F(RapidJson, SIMD_SUFFIX(ReaderParseIterative_DummyHandler)) {
    for (size_t i = 0; i < kTrialCount; i++) {
        StringStream s(json_);
        BaseReaderHandler<> h;
        Reader reader;
        EXPECT_TRUE(reader.Parse<kParseIterativeFlag>(s, h));
    }
}

TEST_F(RapidJson, SIMD_SUFFIX(ReaderParseIterativeInsitu_DummyHandler)) {
    for (size_t i = 0; i < kTrialCount; i++) {
        memcpy(temp_, json_, length_ + 1);
        InsituStringStream s(temp_);
        BaseReaderHandler<> h;
        Reader reader;
        EXPECT_TRUE(reader.Parse<kParseIterativeFlag|kParseInsituFlag>(s, h));
    }
}

TEST_F(RapidJson, SIMD_SUFFIX(ReaderParse_DummyHandler_ValidateEncoding)) {
    for (size_t i = 0; i < kTrialCount; i++) {
        StringStream s(json_);
        BaseReaderHandler<> h;
        Reader reader;
        EXPECT_TRUE(reader.Parse<kParseValidateEncodingFlag>(s, h));
    }
}

TEST_F(RapidJson, SIMD_SUFFIX(DocumentParseInsitu_MemoryPoolAllocator)) {
    for (size_t i = 0; i < kTrialCount; i++) {
        memcpy(temp_, json_, length_ + 1);
        Document doc;
        doc.ParseInsitu(temp_);
        ASSERT_TRUE(doc.IsObject());
    }
}

TEST_F(RapidJson, SIMD_SUFFIX(DocumentParseIterativeInsitu_MemoryPoolAllocator)) {
    for (size_t i = 0; i < kTrialCount; i++) {
        memcpy(temp_, json_, length_ + 1);
        Document doc;
        doc.ParseInsitu<kParseIterativeFlag>(temp_);
        ASSERT_TRUE(doc.IsObject());
    }
}

TEST_F(RapidJson, SIMD_SUFFIX(DocumentParse_MemoryPoolAllocator)) {
    for (size_t i = 0; i < kTrialCount; i++) {
        Document doc;
        doc.Parse(json_);
        ASSERT_TRUE(doc.IsObject());
    }
}

TEST_F(RapidJson, SIMD_SUFFIX(DocumentParseIterative_MemoryPoolAllocator)) {
    for (size_t i = 0; i < kTrialCount; i++) {
        Document doc;
        doc.Parse<kParseIterativeFlag>(json_);
        ASSERT_TRUE(doc.IsObject());
    }
}

TEST_F(RapidJson, SIMD_SUFFIX(DocumentParse_CrtAllocator)) {
    for (size_t i = 0; i < kTrialCount; i++) {
        memcpy(temp_, json_, length_ + 1);
        GenericDocument<UTF8<>, CrtAllocator> doc;
        doc.Parse(temp_);
        ASSERT_TRUE(doc.IsObject());
    }
}

template<typename T>
size_t Traverse(const T& value) {
    size_t count = 1;
    switch(value.GetType()) {
        case kObjectType:
            for (typename T::ConstMemberIterator itr = value.MemberBegin(); itr != value.MemberEnd(); ++itr) {
                count++;    // name
                count += Traverse(itr->value);
            }
            break;

        case kArrayType:
            for (typename T::ConstValueIterator itr = value.Begin(); itr != value.End(); ++itr)
                count += Traverse(*itr);
            break;

        default:
            // Do nothing.
            break;
    }
    return count;
}

TEST_F(RapidJson, DocumentTraverse) {
    for (size_t i = 0; i < kTrialCount; i++) {
        size_t count = Traverse(doc_);
        EXPECT_EQ(4339u, count);
        //if (i == 0)
        //  std::cout << count << std::endl;
    }
}

#ifdef __GNUC__
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(effc++)
#endif

struct ValueCounter : public BaseReaderHandler<> {
    ValueCounter() : count_(1) {}   // root

    bool EndObject(SizeType memberCount) { count_ += memberCount * 2; return true; }
    bool EndArray(SizeType elementCount) { count_ += elementCount; return true; }

    SizeType count_;
};

#ifdef __GNUC__
RAPIDJSON_DIAG_POP
#endif

TEST_F(RapidJson, DocumentAccept) {
    for (size_t i = 0; i < kTrialCount; i++) {
        ValueCounter counter;
        doc_.Accept(counter);
        EXPECT_EQ(4339u, counter.count_);
    }
}

struct NullStream {
    NullStream() /*: length_(0)*/ {}
    void Put(char) { /*++length_;*/ }
    void Flush() {}
    //size_t length_;
};

TEST_F(RapidJson, Writer_NullStream) {
    for (size_t i = 0; i < kTrialCount; i++) {
        NullStream s;
        Writer<NullStream> writer(s);
        doc_.Accept(writer);
        //if (i == 0)
        //  std::cout << s.length_ << std::endl;
    }
}

TEST_F(RapidJson, Writer_StringBuffer) {
    for (size_t i = 0; i < kTrialCount; i++) {
        StringBuffer s(0, 1024 * 1024);
        Writer<StringBuffer> writer(s);
        doc_.Accept(writer);
        const char* str = s.GetString();
        (void)str;
        //if (i == 0)
        //  std::cout << strlen(str) << std::endl;
    }
}

TEST_F(RapidJson, PrettyWriter_StringBuffer) {
    for (size_t i = 0; i < kTrialCount; i++) {
        StringBuffer s(0, 2048 * 1024);
        PrettyWriter<StringBuffer> writer(s);
        writer.SetIndent(' ', 1);
        doc_.Accept(writer);
        const char* str = s.GetString();
        (void)str;
        //if (i == 0)
        //  std::cout << strlen(str) << std::endl;
    }
}

TEST_F(RapidJson, internal_Pow10) {
    double sum = 0;
    for (size_t i = 0; i < kTrialCount * kTrialCount; i++)
        sum += internal::Pow10(int(i & 255));
    EXPECT_GT(sum, 0.0);
}

TEST_F(RapidJson, SIMD_SUFFIX(Whitespace)) {
    for (size_t i = 0; i < kTrialCount; i++) {
        Document doc;
        ASSERT_TRUE(doc.Parse(whitespace_).IsArray());
    }       
}

TEST_F(RapidJson, UTF8_Validate) {
    NullStream os;

    for (size_t i = 0; i < kTrialCount; i++) {
        StringStream is(json_);
        bool result = true;
        while (is.Peek() != '\0')
            result &= UTF8<>::Validate(is, os);
        EXPECT_TRUE(result);
    }
}

// Depreciated.
//TEST_F(RapidJson, FileStream_Read) {
//  for (size_t i = 0; i < kTrialCount; i++) {
//      FILE *fp = fopen(filename_, "rb");
//      FileStream s(fp);
//      while (s.Take() != '\0')
//          ;
//      fclose(fp);
//  }
//}

TEST_F(RapidJson, FileReadStream) {
    for (size_t i = 0; i < kTrialCount; i++) {
        FILE *fp = fopen(filename_, "rb");
        char buffer[65536];
        FileReadStream s(fp, buffer, sizeof(buffer));
        while (s.Take() != '\0')
            ;
        fclose(fp);
    }
}

TEST_F(RapidJson, SIMD_SUFFIX(ReaderParse_DummyHandler_FileReadStream)) {
    for (size_t i = 0; i < kTrialCount; i++) {
        FILE *fp = fopen(filename_, "rb");
        char buffer[65536];
        FileReadStream s(fp, buffer, sizeof(buffer));
        BaseReaderHandler<> h;
        Reader reader;
        reader.Parse(s, h);
        fclose(fp);
    }
}

#endif // TEST_RAPIDJSON
