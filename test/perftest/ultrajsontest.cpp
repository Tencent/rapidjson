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

#if TEST_ULTRAJSON

#include "ultrajson/ultrajsondec.c"
#include "ultrajson/ultrajsonenc.c"

class UltraJson : public PerfTest {
};

static char dummy = 0;

static void Object_objectAddKey(JSOBJ obj, JSOBJ name, JSOBJ value) {}
static void Object_arrayAddItem(JSOBJ obj, JSOBJ value) {}

static JSOBJ Object_newString(wchar_t *start, wchar_t *end) { return &dummy; }
static JSOBJ Object_newTrue(void)                           { return &dummy; }
static JSOBJ Object_newFalse(void)                          { return &dummy; }
static JSOBJ Object_newNull(void)                           { return &dummy; }
static JSOBJ Object_newObject(void)                         { return &dummy; }
static JSOBJ Object_newArray(void)                          { return &dummy; }
static JSOBJ Object_newInteger(JSINT32 value)               { return &dummy; }
static JSOBJ Object_newLong(JSINT64 value)                  { return &dummy; }
static JSOBJ Object_newDouble(double value)                 { return &dummy; }

static void Object_releaseObject(JSOBJ obj) {}

static JSONObjectDecoder decoder = {
    Object_newString,
    Object_objectAddKey,
    Object_arrayAddItem,
    Object_newTrue,
    Object_newFalse,
    Object_newNull,
    Object_newObject,
    Object_newArray,
    Object_newInteger,
    Object_newLong,
    Object_newDouble,
    Object_releaseObject,
    malloc,
    free,
    realloc
};

TEST_F(UltraJson, Decode) {
    for (int i = 0; i < kTrialCount; i++) {
        decoder.errorStr = NULL;
        decoder.errorOffset = NULL;
        void *ret = JSON_DecodeObject(&decoder, json_, length_);
        ASSERT_TRUE(ret != 0);
    }
}

TEST_F(UltraJson, Whitespace) {
    for (int i = 0; i < kTrialCount; i++) {
        decoder.errorStr = NULL;
        decoder.errorOffset = NULL;
        void *ret = JSON_DecodeObject(&decoder, whitespace_, whitespace_length_);
        ASSERT_TRUE(ret != 0);
    }
}

#endif // TEST_ULTRAJSON
