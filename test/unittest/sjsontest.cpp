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
#include "rapidjson/writer.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/encodedstream.h"
#include "rapidjson/stringbuffer.h"
#include <sstream>
#include <algorithm>

#ifdef __clang__
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(c++98-compat)
RAPIDJSON_DIAG_OFF(missing-variable-declarations)
#endif

using namespace rapidjson;

TEST(Sjson, Parse) {
    const char* sjson = "hello = \"world\"  t = true  f = false, n = null i = 123, pi = 3.1416 a = [1 2 3 4]";
    GenericDocument<UTF8<>>  doc;
    doc.Parse<kParseSJSONDefaultFlags>(sjson);
    EXPECT_FALSE(doc.HasParseError());
}

TEST(Sjson, ReadSjsonWritejson) {
    // Test writing sjson -> json.
    StringStream s("hello = \"world\"  t = true  f = false, /* I am a comment! */ n = null i = 123, pi = 3.1416 a = [1 2 3]");
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    buffer.ShrinkToFit();
    Reader reader;
    reader.Parse<kParseSJSONDefaultFlags>(s, writer);
    EXPECT_STREQ("{\"hello\":\"world\",\"t\":true,\"f\":false,\"n\":null,\"i\":123,\"pi\":3.1416,\"a\":[1,2,3]}", buffer.GetString());
    EXPECT_EQ(77u, buffer.GetSize());
    EXPECT_TRUE(writer.IsComplete());
}

TEST(Sjson, ReadSjsonWriteSjson) {
    // Test writing sjson -> json.
    StringStream s("hello = \"world\"  t = true  f = false, /* I am a comment! */ n = null i = 123, pi = 3.1416 a = [1 2 3]");
    StringBuffer buffer;
    Writer<StringBuffer, UTF8<>, UTF8<>, CrtAllocator, kWriteDefaultSJSONFlags> writer(buffer);
    buffer.ShrinkToFit();
    Reader reader;
    reader.Parse<kParseSJSONDefaultFlags>(s, writer);
    EXPECT_STREQ("hello=\"world\" t=true f=false n=null i=123 pi=3.1416 a=[1 2 3]", buffer.GetString());
    EXPECT_EQ(61u, buffer.GetSize());
    EXPECT_TRUE(writer.IsComplete());
}
