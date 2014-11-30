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
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

using namespace rapidjson;

TEST(StringBuffer, InitialSize) {
    StringBuffer buffer;
    EXPECT_EQ(0u, buffer.GetSize());
    EXPECT_STREQ("", buffer.GetString());
}

TEST(StringBuffer, Put) {
    StringBuffer buffer;
    buffer.Put('A');

    EXPECT_EQ(1u, buffer.GetSize());
    EXPECT_STREQ("A", buffer.GetString());
}

TEST(StringBuffer, Clear) {
    StringBuffer buffer;
    buffer.Put('A');
    buffer.Put('B');
    buffer.Put('C');
    buffer.Clear();

    EXPECT_EQ(0u, buffer.GetSize());
    EXPECT_STREQ("", buffer.GetString());
}

TEST(StringBuffer, Push) {
    StringBuffer buffer;
    buffer.Push(5);

    EXPECT_EQ(5u, buffer.GetSize());
}

TEST(StringBuffer, Pop) {
    StringBuffer buffer;
    buffer.Put('A');
    buffer.Put('B');
    buffer.Put('C');
    buffer.Put('D');
    buffer.Put('E');
    buffer.Pop(3);

    EXPECT_EQ(2u, buffer.GetSize());
    EXPECT_STREQ("AB", buffer.GetString());
}

#if RAPIDJSON_HAS_CXX11_RVALUE_REFS

#include <type_traits>

TEST(StringBuffer, Traits) {
    static_assert( std::is_constructible<StringBuffer>::value, "");
    static_assert( std::is_default_constructible<StringBuffer>::value, "");
#ifndef _MSC_VER
    static_assert(!std::is_copy_constructible<StringBuffer>::value, "");
#endif
    static_assert( std::is_move_constructible<StringBuffer>::value, "");

    static_assert(!std::is_nothrow_constructible<StringBuffer>::value, "");
    static_assert(!std::is_nothrow_default_constructible<StringBuffer>::value, "");
    static_assert(!std::is_nothrow_copy_constructible<StringBuffer>::value, "");
    static_assert(!std::is_nothrow_move_constructible<StringBuffer>::value, "");

    static_assert( std::is_assignable<StringBuffer,StringBuffer>::value, "");
#ifndef _MSC_VER
    static_assert(!std::is_copy_assignable<StringBuffer>::value, "");
#endif
    static_assert( std::is_move_assignable<StringBuffer>::value, "");

    static_assert(!std::is_nothrow_assignable<StringBuffer,StringBuffer>::value, "");
    static_assert(!std::is_nothrow_copy_assignable<StringBuffer>::value, "");
    static_assert(!std::is_nothrow_move_assignable<StringBuffer>::value, "");

    static_assert( std::is_destructible<StringBuffer>::value, "");
#ifndef _MSC_VER
    static_assert(std::is_nothrow_destructible<StringBuffer>::value, "");
#endif
}

TEST(StringBuffer, MoveConstructor) {
    StringBuffer x;
    x.Put('A');
    x.Put('B');
    x.Put('C');
    x.Put('D');

    EXPECT_EQ(4u, x.GetSize());
    EXPECT_STREQ("ABCD", x.GetString());

    // StringBuffer y(x); // does not compile (!is_copy_constructible)
    StringBuffer y(std::move(x));
    EXPECT_EQ(0u, x.GetSize());
    EXPECT_EQ(4u, y.GetSize());
    EXPECT_STREQ("ABCD", y.GetString());

    // StringBuffer z = y; // does not compile (!is_copy_assignable)
    StringBuffer z = std::move(y);
    EXPECT_EQ(0u, y.GetSize());
    EXPECT_EQ(4u, z.GetSize());
    EXPECT_STREQ("ABCD", z.GetString());
}

TEST(StringBuffer, MoveAssignment) {
    StringBuffer x;
    x.Put('A');
    x.Put('B');
    x.Put('C');
    x.Put('D');

    EXPECT_EQ(4u, x.GetSize());
    EXPECT_STREQ("ABCD", x.GetString());

    StringBuffer y;
    // y = x; // does not compile (!is_copy_assignable)
    y = std::move(x);
    EXPECT_EQ(0u, x.GetSize());
    EXPECT_EQ(4u, y.GetSize());
    EXPECT_STREQ("ABCD", y.GetString());
}

#endif // RAPIDJSON_HAS_CXX11_RVALUE_REFS
