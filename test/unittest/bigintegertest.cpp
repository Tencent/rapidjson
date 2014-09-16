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

#include "rapidjson/internal/biginteger.h"

using namespace rapidjson::internal;

#define BIGINTEGER_LITERAL(s) BigInteger(s, sizeof(s) - 1)

static const BigInteger kZero(0);
static const BigInteger kOne(1);
static const BigInteger kUint64Max = BIGINTEGER_LITERAL("18446744073709551615");
static const BigInteger kTwo64 = BIGINTEGER_LITERAL("18446744073709551616");

TEST(BigInteger, Constructor) {
    EXPECT_TRUE(kZero.IsZero());
    EXPECT_TRUE(kZero == kZero);
    EXPECT_TRUE(kZero == BIGINTEGER_LITERAL("0"));
    EXPECT_TRUE(kZero == BIGINTEGER_LITERAL("00"));

    const BigInteger a(123);
    EXPECT_TRUE(a == a);
    EXPECT_TRUE(a == BIGINTEGER_LITERAL("123"));
    EXPECT_TRUE(a == BIGINTEGER_LITERAL("0123"));

    EXPECT_EQ(2u, kTwo64.GetCount());
    EXPECT_EQ(0u, kTwo64.GetDigit(0));
    EXPECT_EQ(1u, kTwo64.GetDigit(1));
}

TEST(BigInteger, AddUint64) {
    BigInteger a = kZero;
    a += 0u;
    EXPECT_TRUE(kZero == a);

    a += 1u;
    EXPECT_TRUE(kOne == a);

    a += 1u;
    EXPECT_TRUE(BigInteger(2) == a);

    EXPECT_TRUE(BigInteger(RAPIDJSON_UINT64_C2(0xFFFFFFFF, 0xFFFFFFFF)) == kUint64Max);
    BigInteger b = kUint64Max;
    b += 1u;
    EXPECT_TRUE(kTwo64 == b);
    b += RAPIDJSON_UINT64_C2(0xFFFFFFFF, 0xFFFFFFFF);
    EXPECT_TRUE(BIGINTEGER_LITERAL("36893488147419103231") == b);
}

TEST(BigInteger, MultiplyUint64) {
    BigInteger a = kZero;
    a *= static_cast <uint64_t>(0);
    EXPECT_TRUE(kZero == a);
    a *= static_cast <uint64_t>(123);
    EXPECT_TRUE(kZero == a);

    BigInteger b = kOne;
    b *= static_cast<uint64_t>(1);
    EXPECT_TRUE(kOne == b);
    b *= static_cast<uint64_t>(0);
    EXPECT_TRUE(kZero == b);

    BigInteger c(123);
    c *= static_cast<uint64_t>(456u);
    EXPECT_TRUE(BigInteger(123u * 456u) == c);
    c *= RAPIDJSON_UINT64_C2(0xFFFFFFFF, 0xFFFFFFFF);
    EXPECT_TRUE(BIGINTEGER_LITERAL("1034640981606221330982120") == c);
    c *= RAPIDJSON_UINT64_C2(0xFFFFFFFF, 0xFFFFFFFF);
    EXPECT_TRUE(BIGINTEGER_LITERAL("19085757395861596536664473018420572782123800") == c);
}

TEST(BigInteger, MultiplyUint32) {
    BigInteger a = kZero;
    a *= static_cast <uint32_t>(0);
    EXPECT_TRUE(kZero == a);
    a *= static_cast <uint32_t>(123);
    EXPECT_TRUE(kZero == a);

    BigInteger b = kOne;
    b *= static_cast<uint32_t>(1);
    EXPECT_TRUE(kOne == b);
    b *= static_cast<uint32_t>(0);
    EXPECT_TRUE(kZero == b);

    BigInteger c(123);
    c *= static_cast<uint32_t>(456u);
    EXPECT_TRUE(BigInteger(123u * 456u) == c);
    c *= 0xFFFFFFFFu;
    EXPECT_TRUE(BIGINTEGER_LITERAL("240896125641960") == c);
    c *= 0xFFFFFFFFu;
    EXPECT_TRUE(BIGINTEGER_LITERAL("1034640981124429079698200") == c);
}

TEST(BigInteger, LeftShift) {
    BigInteger a = kZero;
    a <<= 1;
    EXPECT_TRUE(kZero == a);
    a <<= 64;
    EXPECT_TRUE(kZero == a);

    a = BigInteger(123);
    a <<= 0;
    EXPECT_TRUE(BigInteger(123) == a);
    a <<= 1;
    EXPECT_TRUE(BigInteger(246) == a);
    a <<= 64;
    EXPECT_TRUE(BIGINTEGER_LITERAL("4537899042132549697536") == a);
    a <<= 99;
    EXPECT_TRUE(BIGINTEGER_LITERAL("2876235222267216943024851750785644982682875244576768") == a);
}

TEST(BigInteger, Compare) {
    EXPECT_EQ(0, kZero.Compare(kZero));
    EXPECT_EQ(1, kOne.Compare(kZero));
    EXPECT_EQ(-1, kZero.Compare(kOne));
    EXPECT_EQ(0, kUint64Max.Compare(kUint64Max));
    EXPECT_EQ(0, kTwo64.Compare(kTwo64));
    EXPECT_EQ(-1, kUint64Max.Compare(kTwo64));
    EXPECT_EQ(1, kTwo64.Compare(kUint64Max));
}
