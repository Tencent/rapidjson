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

#include "rapidjson/internal/strtod.h"

using namespace rapidjson::internal;

#define BIGINTEGER_LITERAL(s) BigInteger(s, sizeof(s) - 1)

static const BigInteger kZero(0);
static const BigInteger kOne(1);
static const BigInteger kUint64Max = BIGINTEGER_LITERAL("18446744073709551615");
static const BigInteger kTwo64 = BIGINTEGER_LITERAL("18446744073709551616");

TEST(Strtod, BigInteger_Constructor) {
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

TEST(Strtod, BigInteger_AddUint64) {
    const BigInteger kZero(0);
    const BigInteger kOne(1);

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

TEST(Strtod, BigInteger_MultiplyUint64) {
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

TEST(Strtod, BigInteger_MultiplyUint32) {
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

TEST(Strtod, BigInteger_LeftShift) {
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

TEST(Strtod, BigInteger_Compare) {
    EXPECT_EQ(0, kZero.Compare(kZero));
    EXPECT_EQ(1, kOne.Compare(kZero));
    EXPECT_EQ(-1, kZero.Compare(kOne));
    EXPECT_EQ(0, kUint64Max.Compare(kUint64Max));
    EXPECT_EQ(0, kTwo64.Compare(kTwo64));
    EXPECT_EQ(-1, kUint64Max.Compare(kTwo64));
    EXPECT_EQ(1, kTwo64.Compare(kUint64Max));
}

TEST(Strtod, CheckApproximationCase) {
    static const int kSignificandSize = 52;
    static const int kExponentBias = 0x3FF;
    static const uint64_t kExponentMask = RAPIDJSON_UINT64_C2(0x7FF00000, 0x00000000);
    static const uint64_t kSignificandMask = RAPIDJSON_UINT64_C2(0x000FFFFF, 0xFFFFFFFF);
    static const uint64_t kHiddenBit = RAPIDJSON_UINT64_C2(0x00100000, 0x00000000);

    // http://www.exploringbinary.com/using-integers-to-check-a-floating-point-approximation/
    // Let b = 0x1.465a72e467d88p-149
    //       = 5741268244528520 x 2^-201
    union {
        double d;
        uint64_t u;
    }u;
    u.u = 0x465a72e467d88 | ((static_cast<uint64_t>(-149 + kExponentBias)) << kSignificandSize);
    const double b = u.d;
    const uint64_t bInt = (u.u & kSignificandMask) | kHiddenBit;
    const int bExp = ((u.u & kExponentMask) >> kSignificandSize) - kExponentBias - kSignificandSize;
    EXPECT_DOUBLE_EQ(1.7864e-45, b);
    EXPECT_EQ(5741268244528520, bInt);
    EXPECT_EQ(-201, bExp);

    // Let d = 17864 x 10-49
    const char dInt[] = "17864";
    const int dExp = -49;

    // Let h = 2^(bExp-1)
    const int hExp = bExp - 1;
    EXPECT_EQ(-202, hExp);

    int dS_Exp2 = 0;
    int dS_Exp5 = 0;
    int bS_Exp2 = 0;
    int bS_Exp5 = 0;
    int hS_Exp2 = 0;
    int hS_Exp5 = 0;

    // Adjust for decimal exponent
    if (dExp >= 0) {
        dS_Exp2 += dExp;
        dS_Exp5 += dExp;
    }
    else {
        bS_Exp2 -= dExp;
        bS_Exp5 -= dExp;
        hS_Exp2 -= dExp;
        hS_Exp5 -= dExp;
    }

    // Adjust for binary exponent
    if (bExp >= 0)
        bS_Exp2 += bExp;
    else {
        dS_Exp2 -= bExp;
        hS_Exp2 -= bExp;
    }

    // Adjust for half ulp exponent
    if (hExp >= 0)
        hS_Exp2 += hExp;
    else {
        dS_Exp2 -= hExp;
        bS_Exp2 -= hExp;
    }

    // Remove common power of two factor from all three scaled values
    int common_Exp2 = std::min(dS_Exp2, std::min(bS_Exp2, hS_Exp2));
    dS_Exp2 -= common_Exp2;
    bS_Exp2 -= common_Exp2;
    hS_Exp2 -= common_Exp2;

    EXPECT_EQ(153, dS_Exp2);
    EXPECT_EQ(0, dS_Exp5);
    EXPECT_EQ(1, bS_Exp2);
    EXPECT_EQ(49, bS_Exp5);
    EXPECT_EQ(0, hS_Exp2);
    EXPECT_EQ(49, hS_Exp5);

    BigInteger dS = BIGINTEGER_LITERAL(dInt);
    dS.MultiplyPow5(dS_Exp5) <<= dS_Exp2;

    BigInteger bS(bInt);
    bS.MultiplyPow5(bS_Exp5) <<= bS_Exp2;

    BigInteger hS(1);
    hS.MultiplyPow5(hS_Exp5) <<= hS_Exp2;

    EXPECT_TRUE(BIGINTEGER_LITERAL("203970822259994138521801764465966248930731085529088") == dS);
    EXPECT_TRUE(BIGINTEGER_LITERAL("203970822259994122305215569213032722473144531250000") == bS);
    EXPECT_TRUE(BIGINTEGER_LITERAL("17763568394002504646778106689453125") == hS);

    EXPECT_EQ(1, dS.Compare(bS));
    
    BigInteger delta(0);
    EXPECT_FALSE(dS.Difference(bS, &delta));
    EXPECT_TRUE(BIGINTEGER_LITERAL("16216586195252933526457586554279088") == delta);
    EXPECT_TRUE(bS.Difference(dS, &delta));
    EXPECT_TRUE(BIGINTEGER_LITERAL("16216586195252933526457586554279088") == delta);

    EXPECT_EQ(-1, delta.Compare(hS));
}
