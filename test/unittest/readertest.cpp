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

#include "rapidjson/reader.h"
#include "rapidjson/internal/dtoa.h"
#include "rapidjson/internal/itoa.h"
#include "rapidjson/memorystream.h"

using namespace rapidjson;

#ifdef __GNUC__
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(effc++)
RAPIDJSON_DIAG_OFF(float-equal)
#endif

template<bool expect>
struct ParseBoolHandler : BaseReaderHandler<UTF8<>, ParseBoolHandler<expect> > {
    ParseBoolHandler() : step_(0) {}
    bool Default() { ADD_FAILURE(); return false; }
    // gcc 4.8.x generates warning in EXPECT_EQ(bool, bool) on this gtest version.
    // Workaround with EXPECT_TRUE().
    bool Bool(bool b) { /*EXPECT_EQ(expect, b); */EXPECT_TRUE(expect == b);  ++step_; return true; }

    unsigned step_;
};

TEST(Reader, ParseTrue) {
    StringStream s("true");
    ParseBoolHandler<true> h;
    Reader reader;
    reader.Parse(s, h);
    EXPECT_EQ(1u, h.step_);
}

TEST(Reader, ParseFalse) {
    StringStream s("false");
    ParseBoolHandler<false> h;
    Reader reader;
    reader.Parse(s, h);
    EXPECT_EQ(1u, h.step_);
}

struct ParseIntHandler : BaseReaderHandler<UTF8<>, ParseIntHandler> {
    ParseIntHandler() : step_(0), actual_() {}
    bool Default() { ADD_FAILURE(); return false; }
    bool Int(int i) { actual_ = i; step_++; return true; }

    unsigned step_;
    int actual_;
};

struct ParseUintHandler : BaseReaderHandler<UTF8<>, ParseUintHandler> {
    ParseUintHandler() : step_(0), actual_() {}
    bool Default() { ADD_FAILURE(); return false; }
    bool Uint(unsigned i) { actual_ = i; step_++; return true; }

    unsigned step_;
    unsigned actual_;
};

struct ParseInt64Handler : BaseReaderHandler<UTF8<>, ParseInt64Handler> {
    ParseInt64Handler() : step_(0), actual_() {}
    bool Default() { ADD_FAILURE(); return false; }
    bool Int64(int64_t i) { actual_ = i; step_++; return true; }

    unsigned step_;
    int64_t actual_;
};

struct ParseUint64Handler : BaseReaderHandler<UTF8<>, ParseUint64Handler> {
    ParseUint64Handler() : step_(0), actual_() {}
    bool Default() { ADD_FAILURE(); return false; }
    bool Uint64(uint64_t i) { actual_ = i; step_++; return true; }

    unsigned step_;
    uint64_t actual_;
};

struct ParseDoubleHandler : BaseReaderHandler<UTF8<>, ParseDoubleHandler> {
    ParseDoubleHandler() : step_(0), actual_() {}
    bool Default() { ADD_FAILURE(); return false; }
    bool Double(double d) { actual_ = d; step_++; return true; }

    unsigned step_;
    double actual_;
};

TEST(Reader, ParseNumber_Integer) {
#define TEST_INTEGER(Handler, str, x) \
    { \
        StringStream s(str); \
        Handler h; \
        Reader reader; \
        reader.Parse(s, h); \
        EXPECT_EQ(1u, h.step_); \
        EXPECT_EQ(x, h.actual_); \
    }

    TEST_INTEGER(ParseUintHandler, "0", 0u);
    TEST_INTEGER(ParseUintHandler, "123", 123u);
    TEST_INTEGER(ParseUintHandler, "2147483648", 2147483648u);       // 2^31 - 1 (cannot be stored in int)
    TEST_INTEGER(ParseUintHandler, "4294967295", 4294967295u);

    TEST_INTEGER(ParseIntHandler, "-123", -123);
    TEST_INTEGER(ParseIntHandler, "-2147483648", static_cast<int32_t>(0x80000000));     // -2^31 (min of int)

    TEST_INTEGER(ParseUint64Handler, "4294967296", RAPIDJSON_UINT64_C2(1, 0));   // 2^32 (max of unsigned + 1, force to use uint64_t)
    TEST_INTEGER(ParseUint64Handler, "18446744073709551615", RAPIDJSON_UINT64_C2(0xFFFFFFFF, 0xFFFFFFFF));   // 2^64 - 1 (max of uint64_t)

    TEST_INTEGER(ParseInt64Handler, "-2147483649", static_cast<int64_t>(RAPIDJSON_UINT64_C2(0xFFFFFFFF, 0x7FFFFFFF)));   // -2^31 -1 (min of int - 1, force to use int64_t)
    TEST_INTEGER(ParseInt64Handler, "-9223372036854775808", static_cast<int64_t>(RAPIDJSON_UINT64_C2(0x80000000, 0x00000000)));       // -2^63 (min of int64_t)

    // Random test for uint32_t/int32_t
    {
        union {
            uint32_t u;
            int32_t i;
        }u;
        Random r;

        for (unsigned i = 0; i < 100000; i++) {
            u.u = r();

            char buffer[32];
            *internal::u32toa(u.u, buffer) = '\0';
            TEST_INTEGER(ParseUintHandler, buffer, u.u);

            if (u.i < 0) {
                *internal::i32toa(u.i, buffer) = '\0';
                TEST_INTEGER(ParseIntHandler, buffer, u.i);
            }
        }
    }

    // Random test for uint64_t/int64_t
    {
        union {
            uint64_t u;
            int64_t i;
        }u;
        Random r;

        for (unsigned i = 0; i < 100000; i++) {
            u.u = uint64_t(r()) << 32;
            u.u |= r();

            char buffer[32];
            if (u.u >= 4294967296ULL) {
                *internal::u64toa(u.u, buffer) = '\0';
                TEST_INTEGER(ParseUint64Handler, buffer, u.u);
            }

            if (u.i <= -2147483649LL) {
                *internal::i64toa(u.i, buffer) = '\0';
                TEST_INTEGER(ParseInt64Handler, buffer, u.i);
            }
        }
    }
#undef TEST_INTEGER
}

template<bool fullPrecision>
static void TestParseDouble() {
#define TEST_DOUBLE(fullPrecision, str, x) \
    { \
        StringStream s(str); \
        ParseDoubleHandler h; \
        Reader reader; \
        ASSERT_EQ(kParseErrorNone, reader.Parse<fullPrecision ? kParseFullPrecisionFlag : 0>(s, h).Code()); \
        EXPECT_EQ(1u, h.step_); \
        internal::Double e(x), a(h.actual_); \
        if (fullPrecision) { \
            EXPECT_EQ(e.Uint64Value(), a.Uint64Value()); \
            if (e.Uint64Value() != a.Uint64Value()) \
                printf("  String: %s\n  Actual: %.17g\nExpected: %.17g\n", str, h.actual_, x); \
        } \
        else { \
            EXPECT_EQ(e.Sign(), a.Sign()); /* for 0.0 != -0.0 */ \
            EXPECT_DOUBLE_EQ(x, h.actual_); \
        } \
    }
    
    TEST_DOUBLE(fullPrecision, "0.0", 0.0);
    TEST_DOUBLE(fullPrecision, "-0.0", -0.0); // For checking issue #289
    TEST_DOUBLE(fullPrecision, "1.0", 1.0);
    TEST_DOUBLE(fullPrecision, "-1.0", -1.0);
    TEST_DOUBLE(fullPrecision, "1.5", 1.5);
    TEST_DOUBLE(fullPrecision, "-1.5", -1.5);
    TEST_DOUBLE(fullPrecision, "3.1416", 3.1416);
    TEST_DOUBLE(fullPrecision, "1E10", 1E10);
    TEST_DOUBLE(fullPrecision, "1e10", 1e10);
    TEST_DOUBLE(fullPrecision, "1E+10", 1E+10);
    TEST_DOUBLE(fullPrecision, "1E-10", 1E-10);
    TEST_DOUBLE(fullPrecision, "-1E10", -1E10);
    TEST_DOUBLE(fullPrecision, "-1e10", -1e10);
    TEST_DOUBLE(fullPrecision, "-1E+10", -1E+10);
    TEST_DOUBLE(fullPrecision, "-1E-10", -1E-10);
    TEST_DOUBLE(fullPrecision, "1.234E+10", 1.234E+10);
    TEST_DOUBLE(fullPrecision, "1.234E-10", 1.234E-10);
    TEST_DOUBLE(fullPrecision, "1.79769e+308", 1.79769e+308);
    TEST_DOUBLE(fullPrecision, "2.22507e-308", 2.22507e-308);
    TEST_DOUBLE(fullPrecision, "-1.79769e+308", -1.79769e+308);
    TEST_DOUBLE(fullPrecision, "-2.22507e-308", -2.22507e-308);
    TEST_DOUBLE(fullPrecision, "4.9406564584124654e-324", 4.9406564584124654e-324); // minimum denormal
    TEST_DOUBLE(fullPrecision, "2.2250738585072009e-308", 2.2250738585072009e-308); // Max subnormal double
    TEST_DOUBLE(fullPrecision, "2.2250738585072014e-308", 2.2250738585072014e-308); // Min normal positive double
    TEST_DOUBLE(fullPrecision, "1.7976931348623157e+308", 1.7976931348623157e+308); // Max double
    TEST_DOUBLE(fullPrecision, "1e-10000", 0.0);                                   // must underflow
    TEST_DOUBLE(fullPrecision, "18446744073709551616", 18446744073709551616.0);    // 2^64 (max of uint64_t + 1, force to use double)
    TEST_DOUBLE(fullPrecision, "-9223372036854775809", -9223372036854775809.0);    // -2^63 - 1(min of int64_t + 1, force to use double)
    TEST_DOUBLE(fullPrecision, "0.9868011474609375", 0.9868011474609375);          // https://github.com/miloyip/rapidjson/issues/120
    TEST_DOUBLE(fullPrecision, "123e34", 123e34);                                  // Fast Path Cases In Disguise
    TEST_DOUBLE(fullPrecision, "45913141877270640000.0", 45913141877270640000.0);
    TEST_DOUBLE(fullPrecision, "2.2250738585072011e-308", 2.2250738585072011e-308); // http://www.exploringbinary.com/php-hangs-on-numeric-value-2-2250738585072011e-308/

    // Since
    // abs((2^-1022 - 2^-1074) - 2.2250738585072012e-308) = 3.109754131239141401123495768877590405345064751974375599... �� 10^-324
    // abs((2^-1022) - 2.2250738585072012e-308) = 1.830902327173324040642192159804623318305533274168872044... �� 10 ^ -324
    // So 2.2250738585072012e-308 should round to 2^-1022 = 2.2250738585072014e-308
    TEST_DOUBLE(fullPrecision, "2.2250738585072012e-308", 2.2250738585072014e-308); // http://www.exploringbinary.com/java-hangs-when-converting-2-2250738585072012e-308/

    // More closer to normal/subnormal boundary
    // boundary = 2^-1022 - 2^-1075 = 2.225073858507201136057409796709131975934819546351645648... �� 10^-308
    TEST_DOUBLE(fullPrecision, "2.22507385850720113605740979670913197593481954635164564e-308", 2.2250738585072009e-308);
    TEST_DOUBLE(fullPrecision, "2.22507385850720113605740979670913197593481954635164565e-308", 2.2250738585072014e-308);

    // 1.0 is in (1.0 - 2^-54, 1.0 + 2^-53)
    // 1.0 - 2^-54 = 0.999999999999999944488848768742172978818416595458984375
    TEST_DOUBLE(fullPrecision, "0.999999999999999944488848768742172978818416595458984375", 1.0); // round to even
    TEST_DOUBLE(fullPrecision, "0.999999999999999944488848768742172978818416595458984374", 0.99999999999999989); // previous double
    TEST_DOUBLE(fullPrecision, "0.999999999999999944488848768742172978818416595458984376", 1.0); // next double
    // 1.0 + 2^-53 = 1.00000000000000011102230246251565404236316680908203125
    TEST_DOUBLE(fullPrecision, "1.00000000000000011102230246251565404236316680908203125", 1.0); // round to even
    TEST_DOUBLE(fullPrecision, "1.00000000000000011102230246251565404236316680908203124", 1.0); // previous double
    TEST_DOUBLE(fullPrecision, "1.00000000000000011102230246251565404236316680908203126", 1.00000000000000022); // next double

    // Numbers from https://github.com/floitsch/double-conversion/blob/master/test/cctest/test-strtod.cc

    TEST_DOUBLE(fullPrecision, "72057594037927928.0", 72057594037927928.0);
    TEST_DOUBLE(fullPrecision, "72057594037927936.0", 72057594037927936.0);
    TEST_DOUBLE(fullPrecision, "72057594037927932.0", 72057594037927936.0);
    TEST_DOUBLE(fullPrecision, "7205759403792793199999e-5", 72057594037927928.0);
    TEST_DOUBLE(fullPrecision, "7205759403792793200001e-5", 72057594037927936.0);

    TEST_DOUBLE(fullPrecision, "9223372036854774784.0", 9223372036854774784.0);
    TEST_DOUBLE(fullPrecision, "9223372036854775808.0", 9223372036854775808.0);
    TEST_DOUBLE(fullPrecision, "9223372036854775296.0", 9223372036854775808.0);
    TEST_DOUBLE(fullPrecision, "922337203685477529599999e-5", 9223372036854774784.0);
    TEST_DOUBLE(fullPrecision, "922337203685477529600001e-5", 9223372036854775808.0);

    TEST_DOUBLE(fullPrecision, "10141204801825834086073718800384", 10141204801825834086073718800384.0);
    TEST_DOUBLE(fullPrecision, "10141204801825835211973625643008", 10141204801825835211973625643008.0);
    TEST_DOUBLE(fullPrecision, "10141204801825834649023672221696", 10141204801825835211973625643008.0);
    TEST_DOUBLE(fullPrecision, "1014120480182583464902367222169599999e-5", 10141204801825834086073718800384.0);
    TEST_DOUBLE(fullPrecision, "1014120480182583464902367222169600001e-5", 10141204801825835211973625643008.0);

    TEST_DOUBLE(fullPrecision, "5708990770823838890407843763683279797179383808", 5708990770823838890407843763683279797179383808.0);
    TEST_DOUBLE(fullPrecision, "5708990770823839524233143877797980545530986496", 5708990770823839524233143877797980545530986496.0);
    TEST_DOUBLE(fullPrecision, "5708990770823839207320493820740630171355185152", 5708990770823839524233143877797980545530986496.0);
    TEST_DOUBLE(fullPrecision, "5708990770823839207320493820740630171355185151999e-3", 5708990770823838890407843763683279797179383808.0);
    TEST_DOUBLE(fullPrecision, "5708990770823839207320493820740630171355185152001e-3", 5708990770823839524233143877797980545530986496.0);

    {
        char n1e308[310];   // '1' followed by 308 '0'
        n1e308[0] = '1';
        for (int i = 1; i < 309; i++)
            n1e308[i] = '0';
        n1e308[309] = '\0';
        TEST_DOUBLE(fullPrecision, n1e308, 1E308);
    }

    // Cover trimming
    TEST_DOUBLE(fullPrecision, 
"2.22507385850720113605740979670913197593481954635164564802342610972482222202107694551652952390813508"
"7914149158913039621106870086438694594645527657207407820621743379988141063267329253552286881372149012"
"9811224514518898490572223072852551331557550159143974763979834118019993239625482890171070818506906306"
"6665599493827577257201576306269066333264756530000924588831643303777979186961204949739037782970490505"
"1080609940730262937128958950003583799967207254304360284078895771796150945516748243471030702609144621"
"5722898802581825451803257070188608721131280795122334262883686223215037756666225039825343359745688844"
"2390026549819838548794829220689472168983109969836584681402285424333066033985088644580400103493397042"
"7567186443383770486037861622771738545623065874679014086723327636718751234567890123456789012345678901"
"e-308", 
    2.2250738585072014e-308);

    {
        static const unsigned count = 100; // Tested with 1000000 locally
        Random r;
        Reader reader; // Reusing reader to prevent heap allocation

        // Exhaustively test different exponents with random significant
        for (uint64_t exp = 0; exp < 2047; exp++) {
            ;
            for (unsigned i = 0; i < count; i++) {
                // Need to call r() in two statements for cross-platform coherent sequence.
                uint64_t u = (exp << 52) | uint64_t(r() & 0x000FFFFF) << 32;
                u |= uint64_t(r());
                internal::Double d = internal::Double(u);

                char buffer[32];
                *internal::dtoa(d.Value(), buffer) = '\0';

                StringStream s(buffer);
                ParseDoubleHandler h;
                ASSERT_EQ(kParseErrorNone, reader.Parse<fullPrecision ? kParseFullPrecisionFlag : 0>(s, h).Code());
                EXPECT_EQ(1u, h.step_);
                internal::Double a(h.actual_);
                if (fullPrecision) {
                    EXPECT_EQ(d.Uint64Value(), a.Uint64Value());
                    if (d.Uint64Value() != a.Uint64Value())
                        printf("  String: %sn  Actual: %.17gnExpected: %.17gn", buffer, h.actual_, d.Value());
                }
                else {
                    EXPECT_EQ(d.Sign(), a.Sign()); /* for 0.0 != -0.0 */
                    EXPECT_DOUBLE_EQ(d.Value(), h.actual_);
                }
            }
        }
    }
#undef TEST_DOUBLE
}

TEST(Reader, ParseNumber_NormalPrecisionDouble) {
    TestParseDouble<false>();
}

TEST(Reader, ParseNumber_FullPrecisionDouble) {
    TestParseDouble<true>();
}

TEST(Reader, ParseNumber_NormalPrecisionError) {
    static unsigned count = 1000000;
    Random r;

    double ulpSum = 0.0;
    double ulpMax = 0.0;
    for (unsigned i = 0; i < count; i++) {
        internal::Double e, a;
        do {
            // Need to call r() in two statements for cross-platform coherent sequence.
            uint64_t u = uint64_t(r()) << 32;
            u |= uint64_t(r());
            e = u;
        } while (e.IsNan() || e.IsInf() || !e.IsNormal());

        char buffer[32];
        *internal::dtoa(e.Value(), buffer) = '\0';

        StringStream s(buffer);
        ParseDoubleHandler h;
        Reader reader;
        ASSERT_EQ(kParseErrorNone, reader.Parse(s, h).Code());
        EXPECT_EQ(1u, h.step_);

        a = h.actual_;
        uint64_t bias1 = e.ToBias();
        uint64_t bias2 = a.ToBias();
        double ulp = bias1 >= bias2 ? bias1 - bias2 : bias2 - bias1;
        ulpMax = std::max(ulpMax, ulp);
        ulpSum += ulp;
    }
    printf("ULP Average = %g, Max = %g \n", ulpSum / count, ulpMax);
}

TEST(Reader, ParseNumber_Error) {
#define TEST_NUMBER_ERROR(errorCode, str) \
    { \
        char buffer[1001]; \
        sprintf(buffer, "%s", str); \
        InsituStringStream s(buffer); \
        BaseReaderHandler<> h; \
        Reader reader; \
        EXPECT_FALSE(reader.Parse(s, h)); \
        EXPECT_EQ(errorCode, reader.GetParseErrorCode());\
    }

    // Number too big to be stored in double.
    {
        char n1e309[311];   // '1' followed by 309 '0'
        n1e309[0] = '1';
        for (int i = 1; i < 310; i++)
            n1e309[i] = '0';
        n1e309[310] = '\0';
        TEST_NUMBER_ERROR(kParseErrorNumberTooBig, n1e309);
    }
    TEST_NUMBER_ERROR(kParseErrorNumberTooBig, "1e309");

    // Miss fraction part in number.
    TEST_NUMBER_ERROR(kParseErrorNumberMissFraction, "1.");
    TEST_NUMBER_ERROR(kParseErrorNumberMissFraction, "1.a");

    // Miss exponent in number.
    TEST_NUMBER_ERROR(kParseErrorNumberMissExponent, "1e");
    TEST_NUMBER_ERROR(kParseErrorNumberMissExponent, "1e_");

#undef TEST_NUMBER_ERROR
}

template <typename Encoding>
struct ParseStringHandler : BaseReaderHandler<Encoding, ParseStringHandler<Encoding> > {
    ParseStringHandler() : str_(0), length_(0), copy_() {}
    ~ParseStringHandler() { EXPECT_TRUE(str_ != 0); if (copy_) free(const_cast<typename Encoding::Ch*>(str_)); }
    
    ParseStringHandler(const ParseStringHandler&);
    ParseStringHandler& operator=(const ParseStringHandler&);

    bool Default() { ADD_FAILURE(); return false; }
    bool String(const typename Encoding::Ch* str, size_t length, bool copy) { 
        EXPECT_EQ(0, str_);
        if (copy) {
            str_ = (typename Encoding::Ch*)malloc((length + 1) * sizeof(typename Encoding::Ch));
            memcpy(const_cast<typename Encoding::Ch*>(str_), str, (length + 1) * sizeof(typename Encoding::Ch));
        }
        else
            str_ = str;
        length_ = length; 
        copy_ = copy;
        return true;
    }

    const typename Encoding::Ch* str_;
    size_t length_;
    bool copy_;
};

TEST(Reader, ParseString) {
#define TEST_STRING(Encoding, e, x) \
    { \
        Encoding::Ch* buffer = StrDup(x); \
        GenericInsituStringStream<Encoding> is(buffer); \
        ParseStringHandler<Encoding> h; \
        GenericReader<Encoding, Encoding> reader; \
        reader.Parse<kParseInsituFlag | kParseValidateEncodingFlag>(is, h); \
        EXPECT_EQ(0, StrCmp<Encoding::Ch>(e, h.str_)); \
        EXPECT_EQ(StrLen(e), h.length_); \
        free(buffer); \
        GenericStringStream<Encoding> s(x); \
        ParseStringHandler<Encoding> h2; \
        GenericReader<Encoding, Encoding> reader2; \
        reader2.Parse(s, h2); \
        EXPECT_EQ(0, StrCmp<Encoding::Ch>(e, h2.str_)); \
        EXPECT_EQ(StrLen(e), h2.length_); \
    }

    // String constant L"\xXX" can only specify character code in bytes, which is not endianness-neutral. 
    // And old compiler does not support u"" and U"" string literal. So here specify string literal by array of Ch.
    // In addition, GCC 4.8 generates -Wnarrowing warnings when character code >= 128 are assigned to signed integer types.
    // Therefore, utype is added for declaring unsigned array, and then cast it to Encoding::Ch.
#define ARRAY(...) { __VA_ARGS__ }
#define TEST_STRINGARRAY(Encoding, utype, array, x) \
    { \
        static const utype ue[] = array; \
        static const Encoding::Ch* e = reinterpret_cast<const Encoding::Ch *>(&ue[0]); \
        TEST_STRING(Encoding, e, x); \
    }

#define TEST_STRINGARRAY2(Encoding, utype, earray, xarray) \
    { \
        static const utype ue[] = earray; \
        static const utype xe[] = xarray; \
        static const Encoding::Ch* e = reinterpret_cast<const Encoding::Ch *>(&ue[0]); \
        static const Encoding::Ch* x = reinterpret_cast<const Encoding::Ch *>(&xe[0]); \
        TEST_STRING(Encoding, e, x); \
    }

    TEST_STRING(UTF8<>, "", "\"\"");
    TEST_STRING(UTF8<>, "Hello", "\"Hello\"");
    TEST_STRING(UTF8<>, "Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING(UTF8<>, "\"\\/\b\f\n\r\t", "\"\\\"\\\\/\\b\\f\\n\\r\\t\"");
    TEST_STRING(UTF8<>, "\x24", "\"\\u0024\"");         // Dollar sign U+0024
    TEST_STRING(UTF8<>, "\xC2\xA2", "\"\\u00A2\"");     // Cents sign U+00A2
    TEST_STRING(UTF8<>, "\xE2\x82\xAC", "\"\\u20AC\""); // Euro sign U+20AC
    TEST_STRING(UTF8<>, "\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  // G clef sign U+1D11E

    // UTF16
    TEST_STRING(UTF16<>, L"", L"\"\"");
    TEST_STRING(UTF16<>, L"Hello", L"\"Hello\"");
    TEST_STRING(UTF16<>, L"Hello\nWorld", L"\"Hello\\nWorld\"");
    TEST_STRING(UTF16<>, L"\"\\/\b\f\n\r\t", L"\"\\\"\\\\/\\b\\f\\n\\r\\t\"");
    TEST_STRINGARRAY(UTF16<>, wchar_t, ARRAY(0x0024, 0x0000), L"\"\\u0024\"");
    TEST_STRINGARRAY(UTF16<>, wchar_t, ARRAY(0x00A2, 0x0000), L"\"\\u00A2\"");  // Cents sign U+00A2
    TEST_STRINGARRAY(UTF16<>, wchar_t, ARRAY(0x20AC, 0x0000), L"\"\\u20AC\"");  // Euro sign U+20AC
    TEST_STRINGARRAY(UTF16<>, wchar_t, ARRAY(0xD834, 0xDD1E, 0x0000), L"\"\\uD834\\uDD1E\"");   // G clef sign U+1D11E

    // UTF32
    TEST_STRINGARRAY2(UTF32<>, unsigned, ARRAY('\0'), ARRAY('\"', '\"', '\0'));
    TEST_STRINGARRAY2(UTF32<>, unsigned, ARRAY('H', 'e', 'l', 'l', 'o', '\0'), ARRAY('\"', 'H', 'e', 'l', 'l', 'o', '\"', '\0'));
    TEST_STRINGARRAY2(UTF32<>, unsigned, ARRAY('H', 'e', 'l', 'l', 'o', '\n', 'W', 'o', 'r', 'l', 'd', '\0'), ARRAY('\"', 'H', 'e', 'l', 'l', 'o', '\\', 'n', 'W', 'o', 'r', 'l', 'd', '\"', '\0'));
    TEST_STRINGARRAY2(UTF32<>, unsigned, ARRAY('\"', '\\', '/', '\b', '\f', '\n', '\r', '\t', '\0'), ARRAY('\"', '\\', '\"', '\\', '\\', '/', '\\', 'b', '\\', 'f', '\\', 'n', '\\', 'r', '\\', 't', '\"', '\0'));
    TEST_STRINGARRAY2(UTF32<>, unsigned, ARRAY(0x00024, 0x0000), ARRAY('\"', '\\', 'u', '0', '0', '2', '4', '\"', '\0'));
    TEST_STRINGARRAY2(UTF32<>, unsigned, ARRAY(0x000A2, 0x0000), ARRAY('\"', '\\', 'u', '0', '0', 'A', '2', '\"', '\0'));   // Cents sign U+00A2
    TEST_STRINGARRAY2(UTF32<>, unsigned, ARRAY(0x020AC, 0x0000), ARRAY('\"', '\\', 'u', '2', '0', 'A', 'C', '\"', '\0'));   // Euro sign U+20AC
    TEST_STRINGARRAY2(UTF32<>, unsigned, ARRAY(0x1D11E, 0x0000), ARRAY('\"', '\\', 'u', 'D', '8', '3', '4', '\\', 'u', 'D', 'D', '1', 'E', '\"', '\0'));    // G clef sign U+1D11E

#undef TEST_STRINGARRAY
#undef ARRAY
#undef TEST_STRING

    // Support of null character in string
    {
        StringStream s("\"Hello\\u0000World\"");
        const char e[] = "Hello\0World";
        ParseStringHandler<UTF8<> > h;
        Reader reader;
        reader.Parse(s, h);
        EXPECT_EQ(0, memcmp(e, h.str_, h.length_ + 1));
        EXPECT_EQ(11u, h.length_);
    }
}

TEST(Reader, ParseString_Transcoding) {
    const char* x = "\"Hello\"";
    const wchar_t* e = L"Hello";
    GenericStringStream<UTF8<> > is(x);
    GenericReader<UTF8<>, UTF16<> > reader;
    ParseStringHandler<UTF16<> > h;
    reader.Parse(is, h);
    EXPECT_EQ(0, StrCmp<UTF16<>::Ch>(e, h.str_));
    EXPECT_EQ(StrLen(e), h.length_);
}

TEST(Reader, ParseString_TranscodingWithValidation) {
    const char* x = "\"Hello\"";
    const wchar_t* e = L"Hello";
    GenericStringStream<UTF8<> > is(x);
    GenericReader<UTF8<>, UTF16<> > reader;
    ParseStringHandler<UTF16<> > h;
    reader.Parse<kParseValidateEncodingFlag>(is, h);
    EXPECT_EQ(0, StrCmp<UTF16<>::Ch>(e, h.str_));
    EXPECT_EQ(StrLen(e), h.length_);
}

TEST(Reader, ParseString_NonDestructive) {
    StringStream s("\"Hello\\nWorld\"");
    ParseStringHandler<UTF8<> > h;
    Reader reader;
    reader.Parse(s, h);
    EXPECT_EQ(0, StrCmp("Hello\nWorld", h.str_));
    EXPECT_EQ(11u, h.length_);
}

template <typename Encoding>
ParseErrorCode TestString(const typename Encoding::Ch* str) {
    GenericStringStream<Encoding> s(str);
    BaseReaderHandler<Encoding> h;
    GenericReader<Encoding, Encoding> reader;
    reader.template Parse<kParseValidateEncodingFlag>(s, h);
    return reader.GetParseErrorCode();
}

TEST(Reader, ParseString_Error) {
#define TEST_STRING_ERROR(errorCode, str)\
        EXPECT_EQ(errorCode, TestString<UTF8<> >(str))

#define ARRAY(...) { __VA_ARGS__ }
#define TEST_STRINGENCODING_ERROR(Encoding, TargetEncoding, utype, array) \
    { \
        static const utype ue[] = array; \
        static const Encoding::Ch* e = reinterpret_cast<const Encoding::Ch *>(&ue[0]); \
        EXPECT_EQ(kParseErrorStringInvalidEncoding, TestString<Encoding>(e));\
        /* decode error */\
        GenericStringStream<Encoding> s(e);\
        BaseReaderHandler<TargetEncoding> h;\
        GenericReader<Encoding, TargetEncoding> reader;\
        reader.Parse(s, h);\
        EXPECT_EQ(kParseErrorStringInvalidEncoding, reader.GetParseErrorCode());\
    }

    // Invalid escape character in string.
    TEST_STRING_ERROR(kParseErrorStringEscapeInvalid, "[\"\\a\"]");

    // Incorrect hex digit after \\u escape in string.
    TEST_STRING_ERROR(kParseErrorStringUnicodeEscapeInvalidHex, "[\"\\uABCG\"]");

    // Quotation in \\u escape in string (Issue #288)
    TEST_STRING_ERROR(kParseErrorStringUnicodeEscapeInvalidHex, "[\"\\uaaa\"]");
    TEST_STRING_ERROR(kParseErrorStringUnicodeEscapeInvalidHex, "[\"\\uD800\\uFFF\"]");

    // The surrogate pair in string is invalid.
    TEST_STRING_ERROR(kParseErrorStringUnicodeSurrogateInvalid, "[\"\\uD800X\"]");
    TEST_STRING_ERROR(kParseErrorStringUnicodeSurrogateInvalid, "[\"\\uD800\\uFFFF\"]");

    // Missing a closing quotation mark in string.
    TEST_STRING_ERROR(kParseErrorStringMissQuotationMark, "[\"Test]");

    // http://www.cl.cam.ac.uk/~mgk25/ucs/examples/UTF-8-test.txt

    // 3  Malformed sequences 

    // 3.1 Unexpected continuation bytes
    {
         char e[] = { '[', '\"', 0, '\"', ']', '\0' };
         for (unsigned char c = 0x80u; c <= 0xBFu; c++) {
            e[2] = c;
            ParseErrorCode error = TestString<UTF8<> >(e);
            EXPECT_EQ(kParseErrorStringInvalidEncoding, error);
            if (error != kParseErrorStringInvalidEncoding)
                std::cout << (unsigned)(unsigned char)c << std::endl;
         }
    }

    // 3.2 Lonely start characters, 3.5 Impossible bytes
    {
        char e[] = { '[', '\"', 0, ' ', '\"', ']', '\0' };
        for (unsigned c = 0xC0u; c <= 0xFFu; c++) {
            e[2] = (char)c;
            TEST_STRING_ERROR(kParseErrorStringInvalidEncoding, e);
        }
    }

    // 4  Overlong sequences 

    // 4.1  Examples of an overlong ASCII character
    TEST_STRINGENCODING_ERROR(UTF8<>, UTF16<>, unsigned char, ARRAY('[', '\"', 0xC0u, 0xAFu, '\"', ']', '\0'));
    TEST_STRINGENCODING_ERROR(UTF8<>, UTF16<>, unsigned char, ARRAY('[', '\"', 0xE0u, 0x80u, 0xAFu, '\"', ']', '\0'));
    TEST_STRINGENCODING_ERROR(UTF8<>, UTF16<>, unsigned char, ARRAY('[', '\"', 0xF0u, 0x80u, 0x80u, 0xAFu, '\"', ']', '\0'));

    // 4.2  Maximum overlong sequences 
    TEST_STRINGENCODING_ERROR(UTF8<>, UTF16<>, unsigned char, ARRAY('[', '\"', 0xC1u, 0xBFu, '\"', ']', '\0'));
    TEST_STRINGENCODING_ERROR(UTF8<>, UTF16<>, unsigned char, ARRAY('[', '\"', 0xE0u, 0x9Fu, 0xBFu, '\"', ']', '\0'));
    TEST_STRINGENCODING_ERROR(UTF8<>, UTF16<>, unsigned char, ARRAY('[', '\"', 0xF0u, 0x8Fu, 0xBFu, 0xBFu, '\"', ']', '\0'));

    // 4.3  Overlong representation of the NUL character 
    TEST_STRINGENCODING_ERROR(UTF8<>, UTF16<>, unsigned char, ARRAY('[', '\"', 0xC0u, 0x80u, '\"', ']', '\0'));
    TEST_STRINGENCODING_ERROR(UTF8<>, UTF16<>, unsigned char, ARRAY('[', '\"', 0xE0u, 0x80u, 0x80u, '\"', ']', '\0'));
    TEST_STRINGENCODING_ERROR(UTF8<>, UTF16<>, unsigned char, ARRAY('[', '\"', 0xF0u, 0x80u, 0x80u, 0x80u, '\"', ']', '\0'));

    // 5  Illegal code positions

    // 5.1 Single UTF-16 surrogates
    TEST_STRINGENCODING_ERROR(UTF8<>, UTF16<>, unsigned char, ARRAY('[', '\"', 0xEDu, 0xA0u, 0x80u, '\"', ']', '\0'));
    TEST_STRINGENCODING_ERROR(UTF8<>, UTF16<>, unsigned char, ARRAY('[', '\"', 0xEDu, 0xADu, 0xBFu, '\"', ']', '\0'));
    TEST_STRINGENCODING_ERROR(UTF8<>, UTF16<>, unsigned char, ARRAY('[', '\"', 0xEDu, 0xAEu, 0x80u, '\"', ']', '\0'));
    TEST_STRINGENCODING_ERROR(UTF8<>, UTF16<>, unsigned char, ARRAY('[', '\"', 0xEDu, 0xAFu, 0xBFu, '\"', ']', '\0'));
    TEST_STRINGENCODING_ERROR(UTF8<>, UTF16<>, unsigned char, ARRAY('[', '\"', 0xEDu, 0xB0u, 0x80u, '\"', ']', '\0'));
    TEST_STRINGENCODING_ERROR(UTF8<>, UTF16<>, unsigned char, ARRAY('[', '\"', 0xEDu, 0xBEu, 0x80u, '\"', ']', '\0'));
    TEST_STRINGENCODING_ERROR(UTF8<>, UTF16<>, unsigned char, ARRAY('[', '\"', 0xEDu, 0xBFu, 0xBFu, '\"', ']', '\0'));

    // Malform UTF-16 sequences
    TEST_STRINGENCODING_ERROR(UTF16<>, UTF8<>, wchar_t, ARRAY('[', '\"', 0xDC00, 0xDC00, '\"', ']', '\0'));
    TEST_STRINGENCODING_ERROR(UTF16<>, UTF8<>, wchar_t, ARRAY('[', '\"', 0xD800, 0xD800, '\"', ']', '\0'));

    // Malform UTF-32 sequence
    TEST_STRINGENCODING_ERROR(UTF32<>, UTF8<>, unsigned, ARRAY('[', '\"', 0x110000, '\"', ']', '\0'));

    // Malform ASCII sequence
    TEST_STRINGENCODING_ERROR(ASCII<>, UTF8<>, char, ARRAY('[', '\"', char(0x80), '\"', ']', '\0'));

#undef ARRAY
#undef TEST_STRINGARRAY_ERROR
}

template <unsigned count>
struct ParseArrayHandler : BaseReaderHandler<UTF8<>, ParseArrayHandler<count> > {
    ParseArrayHandler() : step_(0) {}

    bool Default() { ADD_FAILURE(); return false; }
    bool Uint(unsigned i) { EXPECT_EQ(step_, i); step_++; return true; }
    bool StartArray() { EXPECT_EQ(0u, step_); step_++; return true; }
    bool EndArray(SizeType) { step_++; return true; }

    unsigned step_;
};

TEST(Reader, ParseEmptyArray) {
    char *json = StrDup("[ ] ");
    InsituStringStream s(json);
    ParseArrayHandler<0> h;
    Reader reader;
    reader.Parse(s, h);
    EXPECT_EQ(2u, h.step_);
    free(json);
}

TEST(Reader, ParseArray) {
    char *json = StrDup("[1, 2, 3, 4]");
    InsituStringStream s(json);
    ParseArrayHandler<4> h;
    Reader reader;
    reader.Parse(s, h);
    EXPECT_EQ(6u, h.step_);
    free(json);
}

TEST(Reader, ParseArray_Error) {
#define TEST_ARRAY_ERROR(errorCode, str) \
    { \
        char buffer[1001]; \
        strncpy(buffer, str, 1000); \
        InsituStringStream s(buffer); \
        BaseReaderHandler<> h; \
        GenericReader<UTF8<>, UTF8<>, CrtAllocator> reader; \
        EXPECT_FALSE(reader.Parse(s, h)); \
        EXPECT_EQ(errorCode, reader.GetParseErrorCode());\
    }

    // Missing a comma or ']' after an array element.
    TEST_ARRAY_ERROR(kParseErrorArrayMissCommaOrSquareBracket, "[1");
    TEST_ARRAY_ERROR(kParseErrorArrayMissCommaOrSquareBracket, "[1}");
    TEST_ARRAY_ERROR(kParseErrorArrayMissCommaOrSquareBracket, "[1 2]");

#undef TEST_ARRAY_ERROR
}

struct ParseObjectHandler : BaseReaderHandler<UTF8<>, ParseObjectHandler> {
    ParseObjectHandler() : step_(0) {}

    bool Default() { ADD_FAILURE(); return false; }
    bool Null() { EXPECT_EQ(8u, step_); step_++; return true; }
    bool Bool(bool b) { 
        switch(step_) {
            case 4: EXPECT_TRUE(b); step_++; return true;
            case 6: EXPECT_FALSE(b); step_++; return true;
            default: ADD_FAILURE(); return false;
        }
    }
    bool Int(int i) { 
        switch(step_) {
            case 10: EXPECT_EQ(123, i); step_++; return true;
            case 15: EXPECT_EQ(1, i); step_++; return true;
            case 16: EXPECT_EQ(2, i); step_++; return true;
            case 17: EXPECT_EQ(3, i); step_++; return true;
            default: ADD_FAILURE(); return false;
        }
    }
    bool Uint(unsigned i) { return Int(i); }
    bool Double(double d) { EXPECT_EQ(12u, step_); EXPECT_DOUBLE_EQ(3.1416, d); step_++; return true; }
    bool String(const char* str, size_t, bool) { 
        switch(step_) {
            case 1: EXPECT_STREQ("hello", str); step_++; return true;
            case 2: EXPECT_STREQ("world", str); step_++; return true;
            case 3: EXPECT_STREQ("t", str); step_++; return true;
            case 5: EXPECT_STREQ("f", str); step_++; return true;
            case 7: EXPECT_STREQ("n", str); step_++; return true;
            case 9: EXPECT_STREQ("i", str); step_++; return true;
            case 11: EXPECT_STREQ("pi", str); step_++; return true;
            case 13: EXPECT_STREQ("a", str); step_++; return true;
            default: ADD_FAILURE(); return false;
        }
    }
    bool StartObject() { EXPECT_EQ(0u, step_); step_++; return true; }
    bool EndObject(SizeType memberCount) { EXPECT_EQ(19u, step_); EXPECT_EQ(7u, memberCount); step_++; return true; }
    bool StartArray() { EXPECT_EQ(14u, step_); step_++; return true; }
    bool EndArray(SizeType elementCount) { EXPECT_EQ(18u, step_); EXPECT_EQ(3u, elementCount); step_++; return true; }

    unsigned step_;
};

TEST(Reader, ParseObject) {
    const char* json = "{ \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3] } ";

    // Insitu
    {
        char* json2 = StrDup(json);
        InsituStringStream s(json2);
        ParseObjectHandler h;
        Reader reader;
        reader.Parse<kParseInsituFlag>(s, h);
        EXPECT_EQ(20u, h.step_);
        free(json2);
    }

    // Normal
    {
        StringStream s(json);
        ParseObjectHandler h;
        Reader reader;
        reader.Parse(s, h);
        EXPECT_EQ(20u, h.step_);
    }
}

struct ParseEmptyObjectHandler : BaseReaderHandler<UTF8<>, ParseEmptyObjectHandler> {
    ParseEmptyObjectHandler() : step_(0) {}

    bool Default() { ADD_FAILURE(); return false; }
    bool StartObject() { EXPECT_EQ(0u, step_); step_++; return true; }
    bool EndObject(SizeType) { EXPECT_EQ(1u, step_); step_++; return true; }

    unsigned step_;
};

TEST(Reader, Parse_EmptyObject) {
    StringStream s("{ } ");
    ParseEmptyObjectHandler h;
    Reader reader;
    reader.Parse(s, h);
    EXPECT_EQ(2u, h.step_);
}

struct ParseMultipleRootHandler : BaseReaderHandler<UTF8<>, ParseMultipleRootHandler> {
    ParseMultipleRootHandler() : step_(0) {}

    bool Default() { ADD_FAILURE(); return false; }
    bool StartObject() { EXPECT_EQ(0u, step_); step_++; return true; }
    bool EndObject(SizeType) { EXPECT_EQ(1u, step_); step_++; return true; }
    bool StartArray() { EXPECT_EQ(2u, step_); step_++; return true; }
    bool EndArray(SizeType) { EXPECT_EQ(3u, step_); step_++; return true; }

    unsigned step_;
};

template <unsigned parseFlags>
void TestMultipleRoot() {
    StringStream s("{}[] a");
    ParseMultipleRootHandler h;
    Reader reader;
    EXPECT_TRUE(reader.Parse<parseFlags>(s, h));
    EXPECT_EQ(2u, h.step_);
    EXPECT_TRUE(reader.Parse<parseFlags>(s, h));
    EXPECT_EQ(4u, h.step_);
    EXPECT_EQ(' ', s.Take());
    EXPECT_EQ('a', s.Take());
}

TEST(Reader, Parse_MultipleRoot) {
    TestMultipleRoot<kParseStopWhenDoneFlag>();
}

TEST(Reader, ParseIterative_MultipleRoot) {
    TestMultipleRoot<kParseIterativeFlag | kParseStopWhenDoneFlag>();
}

template <unsigned parseFlags>
void TestInsituMultipleRoot() {
    char* buffer = strdup("{}[] a");
    InsituStringStream s(buffer);
    ParseMultipleRootHandler h;
    Reader reader;
    EXPECT_TRUE(reader.Parse<kParseInsituFlag | parseFlags>(s, h));
    EXPECT_EQ(2u, h.step_);
    EXPECT_TRUE(reader.Parse<kParseInsituFlag | parseFlags>(s, h));
    EXPECT_EQ(4u, h.step_);
    EXPECT_EQ(' ', s.Take());
    EXPECT_EQ('a', s.Take());
    free(buffer);
}

TEST(Reader, ParseInsitu_MultipleRoot) {
    TestInsituMultipleRoot<kParseStopWhenDoneFlag>();
}

TEST(Reader, ParseInsituIterative_MultipleRoot) {
    TestInsituMultipleRoot<kParseIterativeFlag | kParseStopWhenDoneFlag>();
}

#define TEST_ERROR(errorCode, str) \
    { \
        char buffer[1001]; \
        strncpy(buffer, str, 1000); \
        InsituStringStream s(buffer); \
        BaseReaderHandler<> h; \
        Reader reader; \
        EXPECT_FALSE(reader.Parse(s, h)); \
        EXPECT_EQ(errorCode, reader.GetParseErrorCode());\
    }

TEST(Reader, ParseDocument_Error) {
    // The document is empty.
    TEST_ERROR(kParseErrorDocumentEmpty, "");
    TEST_ERROR(kParseErrorDocumentEmpty, " ");
    TEST_ERROR(kParseErrorDocumentEmpty, " \n");

    // The document root must not follow by other values.
    TEST_ERROR(kParseErrorDocumentRootNotSingular, "[] 0");
    TEST_ERROR(kParseErrorDocumentRootNotSingular, "{} 0");
    TEST_ERROR(kParseErrorDocumentRootNotSingular, "null []");
    TEST_ERROR(kParseErrorDocumentRootNotSingular, "0 {}");
}

TEST(Reader, ParseValue_Error) {
    // Invalid value.
    TEST_ERROR(kParseErrorValueInvalid, "nulL");
    TEST_ERROR(kParseErrorValueInvalid, "truE");
    TEST_ERROR(kParseErrorValueInvalid, "falsE");
    TEST_ERROR(kParseErrorValueInvalid, "a]");
    TEST_ERROR(kParseErrorValueInvalid, ".1");
}

TEST(Reader, ParseObject_Error) {
    // Missing a name for object member.
    TEST_ERROR(kParseErrorObjectMissName, "{1}");
    TEST_ERROR(kParseErrorObjectMissName, "{:1}");
    TEST_ERROR(kParseErrorObjectMissName, "{null:1}");
    TEST_ERROR(kParseErrorObjectMissName, "{true:1}");
    TEST_ERROR(kParseErrorObjectMissName, "{false:1}");
    TEST_ERROR(kParseErrorObjectMissName, "{1:1}");
    TEST_ERROR(kParseErrorObjectMissName, "{[]:1}");
    TEST_ERROR(kParseErrorObjectMissName, "{{}:1}");
    TEST_ERROR(kParseErrorObjectMissName, "{xyz:1}");

    // Missing a colon after a name of object member.
    TEST_ERROR(kParseErrorObjectMissColon, "{\"a\" 1}");
    TEST_ERROR(kParseErrorObjectMissColon, "{\"a\",1}");

    // Must be a comma or '}' after an object member
    TEST_ERROR(kParseErrorObjectMissCommaOrCurlyBracket, "{\"a\":1]");

    // This tests that MemoryStream is checking the length in Peek().
    {
        MemoryStream ms("{\"a\"", 1);
        BaseReaderHandler<> h;
        Reader reader;
        EXPECT_FALSE(reader.Parse<kParseStopWhenDoneFlag>(ms, h));
        EXPECT_EQ(kParseErrorObjectMissName, reader.GetParseErrorCode());
    }
}

#undef TEST_ERROR

TEST(Reader, SkipWhitespace) {
    StringStream ss(" A \t\tB\n \n\nC\r\r \rD \t\n\r E");
    const char* expected = "ABCDE";
    for (size_t i = 0; i < 5; i++) {
        SkipWhitespace(ss);
        EXPECT_EQ(expected[i], ss.Take());
    }
}

// Test implementing a stream without copy stream optimization.
// Clone from GenericStringStream except that copy constructor is disabled.
template <typename Encoding>
class CustomStringStream {
public:
    typedef typename Encoding::Ch Ch;

    CustomStringStream(const Ch *src) : src_(src), head_(src) {}

    Ch Peek() const { return *src_; }
    Ch Take() { return *src_++; }
    size_t Tell() const { return static_cast<size_t>(src_ - head_); }

    Ch* PutBegin() { RAPIDJSON_ASSERT(false); return 0; }
    void Put(Ch) { RAPIDJSON_ASSERT(false); }
    void Flush() { RAPIDJSON_ASSERT(false); }
    size_t PutEnd(Ch*) { RAPIDJSON_ASSERT(false); return 0; }

private:
    // Prohibit copy constructor & assignment operator.
    CustomStringStream(const CustomStringStream&);
    CustomStringStream& operator=(const CustomStringStream&);

    const Ch* src_;     //!< Current read position.
    const Ch* head_;    //!< Original head of the string.
};

// If the following code is compiled, it should generate compilation error as predicted.
// Because CustomStringStream<> is not copyable via making copy constructor private.
#if 0
namespace rapidjson {

template <typename Encoding>
struct StreamTraits<CustomStringStream<Encoding> > {
    enum { copyOptimization = 1 };
};

} // namespace rapidjson
#endif 

TEST(Reader, CustomStringStream) {
    const char* json = "{ \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3] } ";
    CustomStringStream<UTF8<char> > s(json);
    ParseObjectHandler h;
    Reader reader;
    reader.Parse(s, h);
    EXPECT_EQ(20u, h.step_);
}

#include <sstream>

class IStreamWrapper {
public:
    typedef char Ch;

    IStreamWrapper(std::istream& is) : is_(is) {}

    Ch Peek() const {
        int c = is_.peek();
        return c == std::char_traits<char>::eof() ? '\0' : (Ch)c;
    }

    Ch Take() { 
        int c = is_.get();
        return c == std::char_traits<char>::eof() ? '\0' : (Ch)c;
    }

    size_t Tell() const { return (size_t)is_.tellg(); }

    Ch* PutBegin() { assert(false); return 0; }
    void Put(Ch) { assert(false); }
    void Flush() { assert(false); }
    size_t PutEnd(Ch*) { assert(false); return 0; }

private:
    IStreamWrapper(const IStreamWrapper&);
    IStreamWrapper& operator=(const IStreamWrapper&);

    std::istream& is_;
};

TEST(Reader, Parse_IStreamWrapper_StringStream) {
    const char* json = "[1,2,3,4]";

    std::stringstream ss(json);
    IStreamWrapper is(ss);

    Reader reader;
    ParseArrayHandler<4> h;
    reader.Parse(is, h);
    EXPECT_FALSE(reader.HasParseError());   
}

// Test iterative parsing.

#define TESTERRORHANDLING(text, errorCode, offset)\
{\
    StringStream json(text); \
    BaseReaderHandler<> handler; \
    Reader reader; \
    reader.Parse<kParseIterativeFlag>(json, handler); \
    EXPECT_TRUE(reader.HasParseError()); \
    EXPECT_EQ(errorCode, reader.GetParseErrorCode()); \
    EXPECT_EQ(offset, reader.GetErrorOffset()); \
}

TEST(Reader, IterativeParsing_ErrorHandling) {
    TESTERRORHANDLING("{\"a\": a}", kParseErrorValueInvalid, 6u);

    TESTERRORHANDLING("", kParseErrorDocumentEmpty, 0u);
    TESTERRORHANDLING("{}{}", kParseErrorDocumentRootNotSingular, 2u);

    TESTERRORHANDLING("{1}", kParseErrorObjectMissName, 1u);
    TESTERRORHANDLING("{\"a\", 1}", kParseErrorObjectMissColon, 4u);
    TESTERRORHANDLING("{\"a\"}", kParseErrorObjectMissColon, 4u);
    TESTERRORHANDLING("{\"a\": 1", kParseErrorObjectMissCommaOrCurlyBracket, 7u);
    TESTERRORHANDLING("[1 2 3]", kParseErrorArrayMissCommaOrSquareBracket, 3u);
    TESTERRORHANDLING("{\"a: 1", kParseErrorStringMissQuotationMark, 5u);

    // Any JSON value can be a valid root element in RFC7159.
    TESTERRORHANDLING("\"ab", kParseErrorStringMissQuotationMark, 2u);
    TESTERRORHANDLING("truE", kParseErrorValueInvalid, 3u);
    TESTERRORHANDLING("False", kParseErrorValueInvalid, 0u);
    TESTERRORHANDLING("true, false", kParseErrorDocumentRootNotSingular, 4u);
    TESTERRORHANDLING("false, false", kParseErrorDocumentRootNotSingular, 5u);
    TESTERRORHANDLING("nulL", kParseErrorValueInvalid, 3u);
    TESTERRORHANDLING("null , null", kParseErrorDocumentRootNotSingular, 5u);
    TESTERRORHANDLING("1a", kParseErrorDocumentRootNotSingular, 1u);
}

template<typename Encoding = UTF8<> >
struct IterativeParsingReaderHandler {
    typedef typename Encoding::Ch Ch;

    const static int LOG_NULL = -1;
    const static int LOG_BOOL = -2;
    const static int LOG_INT = -3;
    const static int LOG_UINT = -4;
    const static int LOG_INT64 = -5;
    const static int LOG_UINT64 = -6;
    const static int LOG_DOUBLE = -7;
    const static int LOG_STRING = -8;
    const static int LOG_STARTOBJECT = -9;
    const static int LOG_KEY = -10;
    const static int LOG_ENDOBJECT = -11;
    const static int LOG_STARTARRAY = -12;
    const static int LOG_ENDARRAY = -13;

    const static size_t LogCapacity = 256;
    int Logs[LogCapacity];
    size_t LogCount;

    IterativeParsingReaderHandler() : LogCount(0) {
    }

    bool Null() { RAPIDJSON_ASSERT(LogCount < LogCapacity); Logs[LogCount++] = LOG_NULL; return true; }

    bool Bool(bool) { RAPIDJSON_ASSERT(LogCount < LogCapacity); Logs[LogCount++] = LOG_BOOL; return true; }

    bool Int(int) { RAPIDJSON_ASSERT(LogCount < LogCapacity); Logs[LogCount++] = LOG_INT; return true; }

    bool Uint(unsigned) { RAPIDJSON_ASSERT(LogCount < LogCapacity); Logs[LogCount++] = LOG_INT; return true; }

    bool Int64(int64_t) { RAPIDJSON_ASSERT(LogCount < LogCapacity); Logs[LogCount++] = LOG_INT64; return true; }

    bool Uint64(uint64_t) { RAPIDJSON_ASSERT(LogCount < LogCapacity); Logs[LogCount++] = LOG_UINT64; return true; }

    bool Double(double) { RAPIDJSON_ASSERT(LogCount < LogCapacity); Logs[LogCount++] = LOG_DOUBLE; return true; }

    bool String(const Ch*, SizeType, bool) { RAPIDJSON_ASSERT(LogCount < LogCapacity); Logs[LogCount++] = LOG_STRING; return true; }

    bool StartObject() { RAPIDJSON_ASSERT(LogCount < LogCapacity); Logs[LogCount++] = LOG_STARTOBJECT; return true; }

    bool Key (const Ch*, SizeType, bool) { RAPIDJSON_ASSERT(LogCount < LogCapacity); Logs[LogCount++] = LOG_KEY; return true; }
	
    bool EndObject(SizeType c) {
        RAPIDJSON_ASSERT(LogCount < LogCapacity);
        Logs[LogCount++] = LOG_ENDOBJECT;
        Logs[LogCount++] = (int)c;
        return true;
    }

    bool StartArray() { RAPIDJSON_ASSERT(LogCount < LogCapacity); Logs[LogCount++] = LOG_STARTARRAY; return true; }

    bool EndArray(SizeType c) {
        RAPIDJSON_ASSERT(LogCount < LogCapacity);
        Logs[LogCount++] = LOG_ENDARRAY;
        Logs[LogCount++] = (int)c;
        return true;
    }
};

TEST(Reader, IterativeParsing_General) {
    {
        StringStream is("[1, {\"k\": [1, 2]}, null, false, true, \"string\", 1.2]");
        Reader reader;
        IterativeParsingReaderHandler<> handler;

        ParseResult r = reader.Parse<kParseIterativeFlag>(is, handler);

        EXPECT_FALSE(r.IsError());
        EXPECT_FALSE(reader.HasParseError());

        int e[] = {
            handler.LOG_STARTARRAY,
            handler.LOG_INT,
            handler.LOG_STARTOBJECT,
            handler.LOG_KEY,
            handler.LOG_STARTARRAY,
            handler.LOG_INT,
            handler.LOG_INT,
            handler.LOG_ENDARRAY, 2,
            handler.LOG_ENDOBJECT, 1,
            handler.LOG_NULL,
            handler.LOG_BOOL,
            handler.LOG_BOOL,
            handler.LOG_STRING,
            handler.LOG_DOUBLE,
            handler.LOG_ENDARRAY, 7
        };

        EXPECT_EQ(sizeof(e) / sizeof(int), handler.LogCount);

        for (size_t i = 0; i < handler.LogCount; ++i) {
            EXPECT_EQ(e[i], handler.Logs[i]) << "i = " << i;
        }
    }
}

TEST(Reader, IterativeParsing_Count) {
    {
        StringStream is("[{}, {\"k\": 1}, [1], []]");
        Reader reader;
        IterativeParsingReaderHandler<> handler;

        ParseResult r = reader.Parse<kParseIterativeFlag>(is, handler);

        EXPECT_FALSE(r.IsError());
        EXPECT_FALSE(reader.HasParseError());

        int e[] = {
            handler.LOG_STARTARRAY,
            handler.LOG_STARTOBJECT,
            handler.LOG_ENDOBJECT, 0,
            handler.LOG_STARTOBJECT,
            handler.LOG_KEY,
            handler.LOG_INT,
            handler.LOG_ENDOBJECT, 1,
            handler.LOG_STARTARRAY,
            handler.LOG_INT,
            handler.LOG_ENDARRAY, 1,
            handler.LOG_STARTARRAY,
            handler.LOG_ENDARRAY, 0,
            handler.LOG_ENDARRAY, 4
        };

        EXPECT_EQ(sizeof(e) / sizeof(int), handler.LogCount);

        for (size_t i = 0; i < handler.LogCount; ++i) {
            EXPECT_EQ(e[i], handler.Logs[i]) << "i = " << i;
        }
    }
}

// Test iterative parsing on kParseErrorTermination.
struct HandlerTerminateAtStartObject : public IterativeParsingReaderHandler<> {
    bool StartObject() { return false; }
};

struct HandlerTerminateAtStartArray : public IterativeParsingReaderHandler<> {
    bool StartArray() { return false; }
};

struct HandlerTerminateAtEndObject : public IterativeParsingReaderHandler<> {
    bool EndObject(SizeType) { return false; }
};

struct HandlerTerminateAtEndArray : public IterativeParsingReaderHandler<> {
    bool EndArray(SizeType) { return false; }
};

TEST(Reader, IterativeParsing_ShortCircuit) {
    {
        HandlerTerminateAtStartObject handler;
        Reader reader;
        StringStream is("[1, {}]");

        ParseResult r = reader.Parse<kParseIterativeFlag>(is, handler);

        EXPECT_TRUE(reader.HasParseError());
        EXPECT_EQ(kParseErrorTermination, r.Code());
        EXPECT_EQ(4u, r.Offset());
    }

    {
        HandlerTerminateAtStartArray handler;
        Reader reader;
        StringStream is("{\"a\": []}");

        ParseResult r = reader.Parse<kParseIterativeFlag>(is, handler);

        EXPECT_TRUE(reader.HasParseError());
        EXPECT_EQ(kParseErrorTermination, r.Code());
        EXPECT_EQ(6u, r.Offset());
    }

    {
        HandlerTerminateAtEndObject handler;
        Reader reader;
        StringStream is("[1, {}]");

        ParseResult r = reader.Parse<kParseIterativeFlag>(is, handler);

        EXPECT_TRUE(reader.HasParseError());
        EXPECT_EQ(kParseErrorTermination, r.Code());
        EXPECT_EQ(5u, r.Offset());
    }

    {
        HandlerTerminateAtEndArray handler;
        Reader reader;
        StringStream is("{\"a\": []}");

        ParseResult r = reader.Parse<kParseIterativeFlag>(is, handler);

        EXPECT_TRUE(reader.HasParseError());
        EXPECT_EQ(kParseErrorTermination, r.Code());
        EXPECT_EQ(7u, r.Offset());
    }
}

// For covering BaseReaderHandler default functions
TEST(Reader, BaseReaderHandler_Default) {
    BaseReaderHandler<> h;
    Reader reader;
    StringStream is("[null, true, -1, 1, -1234567890123456789, 1234567890123456789, 3.14, \"s\", { \"a\" : 1 }]");
    EXPECT_TRUE(reader.Parse(is, h));
}

template <int e>
struct TerminateHandler {
    bool Null() { return e != 0; }
    bool Bool(bool) { return e != 1; }
    bool Int(int) { return e != 2; }
    bool Uint(unsigned) { return e != 3; }
    bool Int64(int64_t) { return e != 4; }
    bool Uint64(uint64_t) { return e != 5;  }
    bool Double(double) { return e != 6; }
    bool String(const char*, SizeType, bool) { return e != 7; }
    bool StartObject() { return e != 8; }
    bool Key(const char*, SizeType, bool)  { return e != 9; }
    bool EndObject(SizeType) { return e != 10; }
    bool StartArray() { return e != 11; }
    bool EndArray(SizeType) { return e != 12; }
};

#define TEST_TERMINATION(e, json)\
{\
    Reader reader;\
    TerminateHandler<e> h;\
    StringStream is(json);\
    EXPECT_FALSE(reader.Parse(is, h));\
    EXPECT_EQ(kParseErrorTermination, reader.GetParseErrorCode());\
}

TEST(Reader, ParseTerminationByHandler) {
    TEST_TERMINATION(0, "[null");
    TEST_TERMINATION(1, "[true");
    TEST_TERMINATION(1, "[false");
    TEST_TERMINATION(2, "[-1");
    TEST_TERMINATION(3, "[1");
    TEST_TERMINATION(4, "[-1234567890123456789");
    TEST_TERMINATION(5, "[1234567890123456789");
    TEST_TERMINATION(6, "[0.5]");
    TEST_TERMINATION(7, "[\"a\"");
    TEST_TERMINATION(8, "[{");
    TEST_TERMINATION(9, "[{\"a\"");
    TEST_TERMINATION(10, "[{}");
    TEST_TERMINATION(10, "[{\"a\":1}"); // non-empty object
    TEST_TERMINATION(11, "{\"a\":[");
    TEST_TERMINATION(12, "{\"a\":[]");
    TEST_TERMINATION(12, "{\"a\":[1]"); // non-empty array
}

#ifdef __GNUC__
RAPIDJSON_DIAG_POP
#endif
