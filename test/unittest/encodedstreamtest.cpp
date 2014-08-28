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
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/encodedstream.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/memorystream.h"
#include "rapidjson/memorybuffer.h"

using namespace rapidjson;

class EncodedStreamTest : public ::testing::Test {
public:
    EncodedStreamTest() : json_(), length_() {}

    virtual void SetUp() {
        json_ = ReadFile("utf8.json", true, &length_);
    }

    virtual void TearDown() {
        free(json_);
        json_ = 0;
    }

private:
    EncodedStreamTest(const EncodedStreamTest&);
    EncodedStreamTest& operator=(const EncodedStreamTest&);
    
protected:
    static FILE* Open(const char* filename) {
        char buffer[1024];
        sprintf(buffer, "encodings/%s", filename);
        FILE *fp = fopen(buffer, "rb");
        if (!fp) {
            sprintf(buffer, "../../bin/encodings/%s", filename);
            fp = fopen(buffer, "rb");
        }
        return fp;
    }

    static char *ReadFile(const char* filename, bool appendPath, size_t* outLength) {
        FILE *fp = appendPath ? Open(filename) : fopen(filename, "rb");

        if (!fp) {
            *outLength = 0;
            return 0;
        }

        fseek(fp, 0, SEEK_END);
        *outLength = (size_t)ftell(fp);
        fseek(fp, 0, SEEK_SET);
        char* buffer = (char*)malloc(*outLength + 1);
        size_t readLength = fread(buffer, 1, *outLength, fp);
        buffer[readLength] = '\0';
        fclose(fp);
        return buffer;
    }

    template <typename FileEncoding, typename MemoryEncoding>
    void TestEncodedInputStream(const char* filename) {
        // Test FileReadStream
        {
            char buffer[16];
            FILE *fp = Open(filename);
            ASSERT_TRUE(fp != 0);
            FileReadStream fs(fp, buffer, sizeof(buffer));
            EncodedInputStream<FileEncoding, FileReadStream> eis(fs);
            StringStream s(json_);

            while (eis.Peek() != '\0') {
                unsigned expected, actual;
                EXPECT_TRUE(UTF8<>::Decode(s, &expected));
                EXPECT_TRUE(MemoryEncoding::Decode(eis, &actual));
                EXPECT_EQ(expected, actual);
            }
            EXPECT_EQ('\0', s.Peek());
            fclose(fp);
        }

        // Test MemoryStream
        {
            size_t size;
            char* data = ReadFile(filename, true, &size);
            MemoryStream ms(data, size);
            EncodedInputStream<FileEncoding, MemoryStream> eis(ms);
            StringStream s(json_);

            while (eis.Peek() != '\0') {
                unsigned expected, actual;
                EXPECT_TRUE(UTF8<>::Decode(s, &expected));
                EXPECT_TRUE(MemoryEncoding::Decode(eis, &actual));
                EXPECT_EQ(expected, actual);
            }
            EXPECT_EQ('\0', s.Peek());
            free(data);
        }
    }

    void TestAutoUTFInputStream(const char *filename) {
        // Test FileReadStream
        {
            char buffer[16];
            FILE *fp = Open(filename);
            ASSERT_TRUE(fp != 0);
            FileReadStream fs(fp, buffer, sizeof(buffer));
            AutoUTFInputStream<unsigned, FileReadStream> eis(fs);
            StringStream s(json_);
            while (eis.Peek() != '\0') {
                unsigned expected, actual;
                EXPECT_TRUE(UTF8<>::Decode(s, &expected));
                EXPECT_TRUE(AutoUTF<unsigned>::Decode(eis, &actual));
                EXPECT_EQ(expected, actual);
            }
            EXPECT_EQ('\0', s.Peek());
            fclose(fp);
        }

        // Test MemoryStream
        {
            size_t size;
            char* data = ReadFile(filename, true, &size);
            MemoryStream ms(data, size);
            AutoUTFInputStream<unsigned, MemoryStream> eis(ms);
            StringStream s(json_);

            while (eis.Peek() != '\0') {
                unsigned expected, actual;
                EXPECT_TRUE(UTF8<>::Decode(s, &expected));
                EXPECT_TRUE(AutoUTF<unsigned>::Decode(eis, &actual));
                EXPECT_EQ(expected, actual);
            }
            EXPECT_EQ('\0', s.Peek());
            free(data);
        }
    }

    template <typename FileEncoding, typename MemoryEncoding>
    void TestEncodedOutputStream(const char* expectedFilename, bool putBOM) {
        // Test FileWriteStream
        {
            char filename[L_tmpnam];
            FILE* fp = TempFile(filename);
            char buffer[16];
            FileWriteStream os(fp, buffer, sizeof(buffer));
            EncodedOutputStream<FileEncoding, FileWriteStream> eos(os, putBOM);
            StringStream s(json_);
            while (s.Peek() != '\0') {
                bool success = Transcoder<UTF8<>, MemoryEncoding>::Transcode(s, eos);
                EXPECT_TRUE(success);
            }
            eos.Flush();
            fclose(fp);
            EXPECT_TRUE(CompareFile(filename, expectedFilename));
            remove(filename);
        }

        // Test MemoryBuffer
        {
            MemoryBuffer mb;
            EncodedOutputStream<FileEncoding, MemoryBuffer> eos(mb, putBOM);
            StringStream s(json_);
            while (s.Peek() != '\0') {
                bool success = Transcoder<UTF8<>, MemoryEncoding>::Transcode(s, eos);
                EXPECT_TRUE(success);
            }
            eos.Flush();
            EXPECT_TRUE(CompareBufferFile(mb.GetBuffer(), mb.GetSize(), expectedFilename));
        }
    }

    void TestAutoUTFOutputStream(UTFType type, bool putBOM, const char *expectedFilename) {
        // Test FileWriteStream
        {
            char filename[L_tmpnam];
            FILE* fp = TempFile(filename);

            char buffer[16];
            FileWriteStream os(fp, buffer, sizeof(buffer));
            AutoUTFOutputStream<unsigned, FileWriteStream> eos(os, type, putBOM);
            StringStream s(json_);
            while (s.Peek() != '\0') {
                bool success = Transcoder<UTF8<>, AutoUTF<unsigned> >::Transcode(s, eos);
                EXPECT_TRUE(success);
            }
            eos.Flush();
            fclose(fp);
            EXPECT_TRUE(CompareFile(filename, expectedFilename));
            remove(filename);
        }

        // Test MemoryBuffer
        {
            MemoryBuffer mb;
            AutoUTFOutputStream<unsigned, MemoryBuffer> eos(mb, type, putBOM);
            StringStream s(json_);
            while (s.Peek() != '\0') {
                bool success = Transcoder<UTF8<>, AutoUTF<unsigned> >::Transcode(s, eos);
                EXPECT_TRUE(success);
            }
            eos.Flush();
            EXPECT_TRUE(CompareBufferFile(mb.GetBuffer(), mb.GetSize(), expectedFilename));
        }
    }

    bool CompareFile(const char* filename, const char* expectedFilename) {
        size_t actualLength, expectedLength;
        char* actualBuffer = ReadFile(filename, false, &actualLength);
        char* expectedBuffer = ReadFile(expectedFilename, true, &expectedLength);
        bool ret = (expectedLength == actualLength) && memcmp(expectedBuffer, actualBuffer, actualLength) == 0;
        free(actualBuffer);
        free(expectedBuffer);
        return ret;
    }

    bool CompareBufferFile(const char* actualBuffer, size_t actualLength, const char* expectedFilename) {
        size_t expectedLength;
        char* expectedBuffer = ReadFile(expectedFilename, true, &expectedLength);
        bool ret = (expectedLength == actualLength) && memcmp(expectedBuffer, actualBuffer, actualLength) == 0;
        free(expectedBuffer);
        return ret;
    }

    char *json_;
    size_t length_;
};

TEST_F(EncodedStreamTest, EncodedInputStream) {
    TestEncodedInputStream<UTF8<>,    UTF8<>  >("utf8.json");
    TestEncodedInputStream<UTF8<>,    UTF8<>  >("utf8bom.json");
    TestEncodedInputStream<UTF16LE<>, UTF16<> >("utf16le.json");
    TestEncodedInputStream<UTF16LE<>, UTF16<> >("utf16lebom.json");
    TestEncodedInputStream<UTF16BE<>, UTF16<> >("utf16be.json");
    TestEncodedInputStream<UTF16BE<>, UTF16<> >("utf16bebom.json");
    TestEncodedInputStream<UTF32LE<>, UTF32<> >("utf32le.json");
    TestEncodedInputStream<UTF32LE<>, UTF32<> >("utf32lebom.json");
    TestEncodedInputStream<UTF32BE<>, UTF32<> >("utf32be.json");
    TestEncodedInputStream<UTF32BE<>, UTF32<> >("utf32bebom.json");
}

TEST_F(EncodedStreamTest, AutoUTFInputStream) {
    TestAutoUTFInputStream("utf8.json");
    TestAutoUTFInputStream("utf8bom.json");
    TestAutoUTFInputStream("utf16le.json");
    TestAutoUTFInputStream("utf16lebom.json");
    TestAutoUTFInputStream("utf16be.json");
    TestAutoUTFInputStream("utf16bebom.json");
    TestAutoUTFInputStream("utf32le.json");
    TestAutoUTFInputStream("utf32lebom.json");
    TestAutoUTFInputStream("utf32be.json");
    TestAutoUTFInputStream("utf32bebom.json");
}

TEST_F(EncodedStreamTest, EncodedOutputStream) {
    TestEncodedOutputStream<UTF8<>,     UTF8<>  >("utf8.json",      false);
    TestEncodedOutputStream<UTF8<>,     UTF8<>  >("utf8bom.json",   true);
    TestEncodedOutputStream<UTF16LE<>,  UTF16<> >("utf16le.json",   false);
    TestEncodedOutputStream<UTF16LE<>,  UTF16<> >("utf16lebom.json",true);
    TestEncodedOutputStream<UTF16BE<>,  UTF16<> >("utf16be.json",   false);
    TestEncodedOutputStream<UTF16BE<>,  UTF16<> >("utf16bebom.json",true);
    TestEncodedOutputStream<UTF32LE<>,  UTF32<> >("utf32le.json",   false);
    TestEncodedOutputStream<UTF32LE<>,  UTF32<> >("utf32lebom.json",true);
    TestEncodedOutputStream<UTF32BE<>,  UTF32<> >("utf32be.json",   false);
    TestEncodedOutputStream<UTF32BE<>,  UTF32<> >("utf32bebom.json",true);
}

TEST_F(EncodedStreamTest, AutoUTFOutputStream) {
    TestAutoUTFOutputStream(kUTF8,      false,  "utf8.json");
    TestAutoUTFOutputStream(kUTF8,      true,   "utf8bom.json");
    TestAutoUTFOutputStream(kUTF16LE,   false,  "utf16le.json");
    TestAutoUTFOutputStream(kUTF16LE,   true,   "utf16lebom.json");
    TestAutoUTFOutputStream(kUTF16BE,   false,  "utf16be.json");
    TestAutoUTFOutputStream(kUTF16BE,   true,   "utf16bebom.json");
    TestAutoUTFOutputStream(kUTF32LE,   false,  "utf32le.json");
    TestAutoUTFOutputStream(kUTF32LE,   true,   "utf32lebom.json");
    TestAutoUTFOutputStream(kUTF32BE,   false,  "utf32be.json");
    TestAutoUTFOutputStream(kUTF32BE,   true,   "utf32bebom.json");
}
