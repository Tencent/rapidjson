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

// Since Travis CI installs old Valgrind 3.7.0, which fails with some SSE4.2
// The unit tests prefix with SIMD should be skipped by Valgrind test

// __SSE2__ and __SSE4_2__ are recognized by gcc, clang, and the Intel compiler.
// We use -march=native with gmake to enable -msse2 and -msse4.2, if supported.
#if defined(__SSE4_2__)
#  define RAPIDJSON_SSE42
#elif defined(__SSE2__)
#  define RAPIDJSON_SSE2
#endif

#define RAPIDJSON_NAMESPACE rapidjson_simd

#include "unittest.h"

#include "rapidjson/reader.h"

using namespace rapidjson_simd;

#ifdef RAPIDJSON_SSE2
#define SIMD_SUFFIX(name) name##_SSE2
#elif defined(RAPIDJSON_SSE42)
#define SIMD_SUFFIX(name) name##_SSE42
#else
#define SIMD_SUFFIX(name) name
#endif

template <typename StreamType>
void TestSkipWhitespace() {
    for (int step = 1; step < 32; step++) {
        char buffer[1025];
        for (size_t i = 0; i < 1024; i++)
            buffer[i] = " \t\r\n"[i % 4];
        for (size_t i = 0; i < 1024; i += step)
            buffer[i] = 'X';
        buffer[1024] = '\0';

        StreamType s(buffer);
        size_t i = 0;
        for (;;) {
            SkipWhitespace(s);
            if (s.Peek() == '\0')
                break;
            EXPECT_EQ(i, s.Tell());
            EXPECT_EQ('X', s.Take());
            i += step;
        }
    }
}

TEST(SIMD, SIMD_SUFFIX(SkipWhitespace)) {
    TestSkipWhitespace<StringStream>();
    TestSkipWhitespace<InsituStringStream>();
}
