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
    for (size_t step = 1; step < 32; step++) {
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
