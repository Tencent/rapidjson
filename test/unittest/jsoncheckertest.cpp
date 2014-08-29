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

using namespace rapidjson;

static char* ReadFile(const char* filename, size_t& length) {
    FILE *fp = fopen(filename, "rb");
    if (!fp)
        fp = fopen(filename, "rb");
    if (!fp)
        return 0;

    fseek(fp, 0, SEEK_END);
    length = (size_t)ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char* json = (char*)malloc(length + 1);
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

        sprintf(filename, "jsonchecker/fail%d.json", i);
        size_t length;
        char* json = ReadFile(filename, length);
        if (!json) {
            sprintf(filename, "../../bin/jsonchecker/fail%d.json", i);
            json = ReadFile(filename, length);
            if (!json) {
                printf("jsonchecker file %s not found", filename);
                ADD_FAILURE();
                continue;
            }
        }

        GenericDocument<UTF8<>, CrtAllocator> document; // Use Crt allocator to check exception-safety (no memory leak)
        document.Parse((const char*)json);
        EXPECT_TRUE(document.HasParseError());

        document.Parse<kParseIterativeFlag>((const char*)json);
        EXPECT_TRUE(document.HasParseError());

        free(json);
    }

    // passX.json
    for (int i = 1; i <= 3; i++) {
        sprintf(filename, "jsonchecker/pass%d.json", i);
        size_t length;
        char* json = ReadFile(filename, length);
        if (!json) {
            sprintf(filename, "../../bin/jsonchecker/pass%d.json", i);
            json = ReadFile(filename, length);
            if (!json) {
                printf("jsonchecker file %s not found", filename);
                continue;
            }
        }

        GenericDocument<UTF8<>, CrtAllocator> document; // Use Crt allocator to check exception-safety (no memory leak)
        document.Parse((const char*)json);
        EXPECT_FALSE(document.HasParseError());

        document.Parse<kParseIterativeFlag>((const char*)json);
        EXPECT_FALSE(document.HasParseError());

        free(json);
    }
}
