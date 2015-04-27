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

#ifndef PERFTEST_H_
#define PERFTEST_H_

#define TEST_RAPIDJSON  1
#define TEST_PLATFORM   0
#define TEST_MISC       0

#define TEST_VERSION_CODE(x,y,z) \
  (((x)*100000) + ((y)*100) + (z))

// __SSE2__ and __SSE4_2__ are recognized by gcc, clang, and the Intel compiler.
// We use -march=native with gmake to enable -msse2 and -msse4.2, if supported.
#if defined(__SSE4_2__)
#  define RAPIDJSON_SSE42
#elif defined(__SSE2__)
#  define RAPIDJSON_SSE2
#endif

////////////////////////////////////////////////////////////////////////////////
// Google Test

#ifdef __cplusplus

// gtest indirectly included inttypes.h, without __STDC_CONSTANT_MACROS.
#ifndef __STDC_CONSTANT_MACROS
#  define __STDC_CONSTANT_MACROS 1 // required by C++ standard
#endif

#if defined(__clang__) || defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2))
#if defined(__clang__) || (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
#pragma GCC diagnostic push
#endif
#pragma GCC diagnostic ignored "-Weffc++"
#endif

#include "gtest/gtest.h"

#if defined(__clang__) || defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
#pragma GCC diagnostic pop
#endif

#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#pragma warning(disable : 4996) // 'function': was declared deprecated
#endif

//! Base class for all performance tests
class PerfTest : public ::testing::Test {
public:
    PerfTest() : filename_(), json_(), length_(), whitespace_(), whitespace_length_() {}

    virtual void SetUp() {

        const char *paths[] = {
            "data/sample.json",
            "bin/data/sample.json",
            "../bin/data/sample.json",
            "../../bin/data/sample.json",
            "../../../bin/data/sample.json"
        };
        FILE *fp = 0;
        for (size_t i = 0; i < sizeof(paths) / sizeof(paths[0]); i++) {
            fp = fopen(filename_ = paths[i], "rb");
            if (fp)
                break;
        }
        ASSERT_TRUE(fp != 0);

        fseek(fp, 0, SEEK_END);
        length_ = (size_t)ftell(fp);
        fseek(fp, 0, SEEK_SET);
        json_ = (char*)malloc(length_ + 1);
        ASSERT_EQ(length_, fread(json_, 1, length_, fp));
        json_[length_] = '\0';
        fclose(fp);

        // whitespace test
        whitespace_length_ = 1024 * 1024;
        whitespace_ = (char *)malloc(whitespace_length_  + 4);
        char *p = whitespace_;
        for (size_t i = 0; i < whitespace_length_; i += 4) {
            *p++ = ' ';
            *p++ = '\n';
            *p++ = '\r';
            *p++ = '\t';
        }
        *p++ = '[';
        *p++ = '0';
        *p++ = ']';
        *p++ = '\0';
    }

    virtual void TearDown() {
        free(json_);
        free(whitespace_);
        json_ = 0;
        whitespace_ = 0;
    }

private:
    PerfTest(const PerfTest&);
    PerfTest& operator=(const PerfTest&);

protected:
    const char* filename_;
    char *json_;
    size_t length_;
    char *whitespace_;
    size_t whitespace_length_;

    static const size_t kTrialCount = 1000;
};

#endif // __cplusplus

#endif // PERFTEST_H_
