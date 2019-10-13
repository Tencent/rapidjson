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
#include "rapidjson/bufferedostreamwrapper.h"
#include "rapidjson/memorybuffer.h"

#include <sstream>
#include <fstream>

#ifdef __clang__
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(c++98-compat)
#endif

using namespace rapidjson;

typedef BufferedOStreamWrapper<> StreamWrapper;

TEST(BufferedOStreamWrapper, UseInWriter) {
    StringStream s
            ("{ \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3] } ");
    std::stringstream ss;
    {
        StreamWrapper buffer(ss);
        Writer<StreamWrapper> writer(buffer);

        Reader reader;
        reader.Parse<0>(s, writer);
    }
    EXPECT_STREQ(
            "{\"hello\":\"world\",\"t\":true,\"f\":false,\"n\":null,\"i\":123,\"pi\":3.1416,\"a\":[1,2,3]}",
            ss.str().c_str());
}

TEST(BufferedOStreamWrapper, InitialSize) {
    std::stringstream ss;
    {
        StreamWrapper buffer(ss);
        EXPECT_EQ("", ss.str());
    }
    EXPECT_EQ("", ss.str());
}

TEST(BufferedOStreamWrapper, Put) {
    std::stringstream ss;
    {
        StreamWrapper buffer(ss);
        buffer.Put('A');
    }

    EXPECT_EQ("A", ss.str());
}

static std::string GenerateLongString(size_t len) {
    static const std::string
            PATTER = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string res;
    for (size_t i = 0; i < len / PATTER.size(); ++i) {
        res += PATTER;
    }

    const size_t left_size = len - res.size();
    res += PATTER.substr(0, left_size);

    return res;
}

TEST(BufferedOStreamWrapper, RepeatedPut) {
    const std::string source = GenerateLongString(65000);

    std::stringstream ss;
    {
        StreamWrapper buffer(ss);

        for (size_t i = 0, len = source.size(); i < len; ++i) {
            buffer.Put(source[i]);
        }
    }

    EXPECT_EQ(source, ss.str());
}

TEST(BufferedOStreamWrapper, Push5) {
    std::stringstream ss;
    {
        StreamWrapper buffer(ss);
        char *buf = buffer.Push(5);
        memset(buf, ' ', 5);
    }

    EXPECT_EQ(std::string(5, ' '), ss.str());
}

TEST(BufferedOStreamWrapper, Push65536) {
    std::stringstream ss;
    {
        StreamWrapper buffer(ss);
        memset(buffer.Push(65536u), '_', 65536u);
    }

    EXPECT_EQ(std::string(65536u, '_'), ss.str());
}

TEST(BufferedOStreamWrapper, Push5Then65536) {
    std::stringstream ss;
    {
        StreamWrapper buffer(ss);
        memset(buffer.Push(5), ' ', 5);
        memset(buffer.Push(65536u), '_', 65536u);
    }

    EXPECT_EQ(std::string(5, ' ') + std::string(65536u, '_'), ss.str());
}

TEST(BufferedOStreamWrapper, RepeatedPush) {
    const size_t chunk_size = 53;
    const std::string source = GenerateLongString(chunk_size * 1000);

    std::stringstream ss;
    {
        StreamWrapper buffer(ss);

        for (size_t i = 0, len = source.size(); i < len; i += chunk_size) {
            memcpy(buffer.Push(chunk_size), source.c_str() + i, chunk_size);
        }
    }

    EXPECT_EQ(source, ss.str());
}

TEST(BufferedOStreamWrapper, Pop) {
    std::stringstream ss;
    {
        StreamWrapper buffer(ss);
        memcpy(buffer.Push(5), "ABCDE", 5);
        buffer.Pop(3);
    }

    EXPECT_EQ("AB", ss.str());
}

TEST(BufferedOStreamWrapper, Flush) {
    std::stringstream ss;
    StreamWrapper buffer(ss);
    memcpy(buffer.Push(6), "abcdef", 6);
    buffer.Flush();

    EXPECT_EQ("abcdef", ss.str());
}

template <typename StringStreamType>
static void TestStringStream() {
    typedef typename StringStreamType::char_type Ch;

    Ch s[] = { 'A', 'B', 'C', '\0' };
    StringStreamType oss(s);
    BufferedOStreamWrapper<StringStreamType> os(oss);
    for (size_t i = 0; i < 3; i++)
        os.Put(s[i]);
    os.Flush();
    for (size_t i = 0; i < 3; i++)
        EXPECT_EQ(s[i], oss.str()[i]);
}

TEST(BufferedOStreamWrapper, ostringstream) {
    TestStringStream<std::ostringstream>();
}

TEST(BufferedOStreamWrapper, stringstream) {
    TestStringStream<std::stringstream>();
}

TEST(BufferedOStreamWrapper, wostringstream) {
    TestStringStream<std::wostringstream>();
}

TEST(BufferedOStreamWrapper, wstringstream) {
    TestStringStream<std::wstringstream>();
}

TEST(BufferedOStreamWrapper, cout) {
    BufferedOStreamWrapper<> os(std::cout);
    const char* s = "Hello World!\n";
    while (*s)
        os.Put(*s++);
    os.Flush();
}

template <typename FileStreamType>
static void TestFileStream() {
    char filename[L_tmpnam];
    FILE* fp = TempFile(filename);
    fclose(fp);

    const char* s = "Hello World!\n";
    {
        FileStreamType ofs(filename, std::ios::out | std::ios::binary);
        BufferedOStreamWrapper<FileStreamType> osw(ofs);
        for (const char* p = s; *p; p++)
            osw.Put(*p);
        osw.Flush();
    }

    fp = fopen(filename, "r");
    ASSERT_TRUE( fp != NULL );
    for (const char* p = s; *p; p++)
        EXPECT_EQ(*p, static_cast<char>(fgetc(fp)));
    fclose(fp);
}

TEST(BufferedOStreamWrapper, ofstream) {
    TestFileStream<std::ofstream>();
}

TEST(BufferedOStreamWrapper, fstream) {
    TestFileStream<std::fstream>();
}


#ifdef __clang__
RAPIDJSON_DIAG_POP
#endif
