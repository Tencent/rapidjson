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

#ifndef UNITTEST_H_
#define UNITTEST_H_


// gtest indirectly included inttypes.h, without __STDC_CONSTANT_MACROS.
#ifndef __STDC_CONSTANT_MACROS
#  define __STDC_CONSTANT_MACROS 1 // required by C++ standard
#endif

#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#pragma warning(disable : 4996) // 'function': was declared deprecated
#endif

#if defined(__clang__) || defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2))
#if defined(__clang__) || (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
#pragma GCC diagnostic push
#endif
#pragma GCC diagnostic ignored "-Weffc++"
#endif

#include "gtest/gtest.h"
#include <stdexcept>

#if defined(__clang__) || defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
#pragma GCC diagnostic pop
#endif

template <typename Ch>
inline unsigned StrLen(const Ch* s) {
    const Ch* p = s;
    while (*p) p++;
    return unsigned(p - s);
}

template<typename Ch>
inline int StrCmp(const Ch* s1, const Ch* s2) {
    while(*s1 && (*s1 == *s2)) { s1++; s2++; }
    return (unsigned)*s1 < (unsigned)*s2 ? -1 : (unsigned)*s1 > (unsigned)*s2;
}

template <typename Ch>
inline Ch* StrDup(const Ch* str) {
    size_t bufferSize = sizeof(Ch) * (StrLen(str) + 1);
    Ch* buffer = (Ch*)malloc(bufferSize);
    memcpy(buffer, str, bufferSize);
    return buffer;
}

inline FILE* TempFile(char *filename) {
#if _MSC_VER
    filename = tmpnam(filename);

    // For Visual Studio, tmpnam() adds a backslash in front. Remove it.
    if (filename[0] == '\\')
        for (int i = 0; filename[i] != '\0'; i++)
            filename[i] = filename[i + 1];
        
    return fopen(filename, "wb");
#else
    strcpy(filename, "/tmp/fileXXXXXX");
    int fd = mkstemp(filename);
    return fdopen(fd, "w");
#endif
}

// Use exception for catching assert
#if _MSC_VER
#pragma warning(disable : 4127)
#endif

class AssertException : public std::logic_error {
public:
    AssertException(const char* w) : std::logic_error(w) {}
};

#define RAPIDJSON_ASSERT(x) if (!(x)) throw AssertException(RAPIDJSON_STRINGIFY(x))

class Random {
public:
    Random(unsigned seed = 0) : mSeed(seed) {}

    unsigned operator()() {
        mSeed = 214013 * mSeed + 2531011;
        return mSeed;
    }

private:
    unsigned mSeed;
};

#endif // UNITTEST_H_
