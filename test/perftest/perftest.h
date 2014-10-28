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
        FILE *fp = fopen(filename_ = "data/sample.json", "rb");
        if (!fp) 
            fp = fopen(filename_ = "../../bin/data/sample.json", "rb");
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
