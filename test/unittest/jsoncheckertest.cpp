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

using namespace rapidjson;

static char* ReadFile(const char* filename, size_t& length) {
    const char *paths[] = {
        "jsonchecker",
        "bin/jsonchecker",
        "../bin/jsonchecker",
        "../../bin/jsonchecker",
        "../../../bin/jsonchecker"
    };
    char buffer[1024];
    FILE *fp = 0;
    for (size_t i = 0; i < sizeof(paths) / sizeof(paths[0]); i++) {
        sprintf(buffer, "%s/%s", paths[i], filename);
        fp = fopen(buffer, "rb");
        if (fp)
            break;
    }

    if (!fp)
        return 0;

    fseek(fp, 0, SEEK_END);
    length = static_cast<size_t>(ftell(fp));
    fseek(fp, 0, SEEK_SET);
    char* json = static_cast<char*>(malloc(length + 1));
    size_t readLength = fread(json, 1, length, fp);
    json[readLength] = '\0';
    fclose(fp);
    return json;
}

TEST(JsonChecker, Reader) {
    char filename[256];

    // jsonchecker/failXX.json
    for (int i = 1; i <= 33; i++) {
        if (i == 1) // fail1.json is valid in rapidjson, which has no limitation on type of root element (RFC 7159).
            continue;
        if (i == 18)    // fail18.json is valid in rapidjson, which has no limitation on depth of nesting.
            continue;

        sprintf(filename, "fail%d.json", i);
        size_t length;
        char* json = ReadFile(filename, length);
        if (!json) {
            printf("jsonchecker file %s not found", filename);
            ADD_FAILURE();
            continue;
        }

        GenericDocument<UTF8<>, CrtAllocator> document; // Use Crt allocator to check exception-safety (no memory leak)
        document.Parse(json);
        EXPECT_TRUE(document.HasParseError());

        document.Parse<kParseIterativeFlag>(json);
        EXPECT_TRUE(document.HasParseError());

        free(json);
    }

    // passX.json
    for (int i = 1; i <= 3; i++) {
        sprintf(filename, "pass%d.json", i);
        size_t length;
        char* json = ReadFile(filename, length);
        if (!json) {
            printf("jsonchecker file %s not found", filename);
            continue;
        }

        GenericDocument<UTF8<>, CrtAllocator> document; // Use Crt allocator to check exception-safety (no memory leak)
        document.Parse(json);
        EXPECT_FALSE(document.HasParseError());

        document.Parse<kParseIterativeFlag>(json);
        EXPECT_FALSE(document.HasParseError());

        free(json);
    }
}
