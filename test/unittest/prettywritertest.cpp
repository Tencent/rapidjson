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
#include "rapidjson/reader.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filewritestream.h"

using namespace rapidjson;

static const char kJson[] = "{\"hello\":\"world\",\"t\":true,\"f\":false,\"n\":null,\"i\":123,\"pi\":3.1416,\"a\":[1,2,3,-1],\"u64\":1234567890123456789,\"i64\":-1234567890123456789}";
static const char kPrettyJson[] =
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
"}";

TEST(PrettyWriter, Basic) {
    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    Reader reader;
    StringStream s(kJson);
    reader.Parse(s, writer);
    EXPECT_STREQ(kPrettyJson, buffer.GetString());
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
    EXPECT_STREQ(kPrettyJson, actual.c_str());
}

// For covering FileWriteStream::PutN()
TEST(PrettyWriter, FileWriteStream) {
    char filename[L_tmpnam];
    FILE* fp = TempFile(filename);
    char buffer[16];
    FileWriteStream os(fp, buffer, sizeof(buffer));
    PrettyWriter<FileWriteStream> writer(os);
    Reader reader;
    StringStream s(kJson);
    reader.Parse(s, writer);
    fclose(fp);

    fp = fopen(filename, "rb");
    fseek(fp, 0, SEEK_END);
    size_t size = (size_t)ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char* json = (char*)malloc(size + 1);
    size_t readLength = fread(json, 1, size, fp);
    json[readLength] = '\0';
    fclose(fp);
    remove(filename);
    EXPECT_STREQ(kPrettyJson, json);
    free(json);
}