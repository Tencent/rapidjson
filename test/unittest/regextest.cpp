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

#include "unittest.h"
#include "rapidjson/internal/regex.h"

using namespace rapidjson::internal;

TEST(Regex, Concatenation) {
    Regex re("abc");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("abc"));
    EXPECT_FALSE(re.Match(""));
    EXPECT_FALSE(re.Match("a"));
    EXPECT_FALSE(re.Match("b"));
    EXPECT_FALSE(re.Match("ab"));
    EXPECT_FALSE(re.Match("abcd"));
}

TEST(Regex, Alternation1) {
    Regex re("abab|abbb");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("abab"));
    EXPECT_TRUE(re.Match("abbb"));
    EXPECT_FALSE(re.Match(""));
    EXPECT_FALSE(re.Match("ab"));
    EXPECT_FALSE(re.Match("ababa"));
    EXPECT_FALSE(re.Match("abb"));
    EXPECT_FALSE(re.Match("abbbb"));
}

TEST(Regex, Alternation2) {
    Regex re("a|b|c");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("a"));
    EXPECT_TRUE(re.Match("b"));
    EXPECT_TRUE(re.Match("c"));
    EXPECT_FALSE(re.Match(""));
    EXPECT_FALSE(re.Match("aa"));
    EXPECT_FALSE(re.Match("ab"));
}

TEST(Regex, Parenthesis1) {
    Regex re("(ab)c");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("abc"));
    EXPECT_FALSE(re.Match(""));
    EXPECT_FALSE(re.Match("a"));
    EXPECT_FALSE(re.Match("b"));
    EXPECT_FALSE(re.Match("ab"));
    EXPECT_FALSE(re.Match("abcd"));
}

TEST(Regex, Parenthesis2) {
    Regex re("a(bc)");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("abc"));
    EXPECT_FALSE(re.Match(""));
    EXPECT_FALSE(re.Match("a"));
    EXPECT_FALSE(re.Match("b"));
    EXPECT_FALSE(re.Match("ab"));
    EXPECT_FALSE(re.Match("abcd"));
}

TEST(Regex, Parenthesis3) {
    Regex re("(a|b)(c|d)");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("ac"));
    EXPECT_TRUE(re.Match("ad"));
    EXPECT_TRUE(re.Match("bc"));
    EXPECT_TRUE(re.Match("bd"));
    EXPECT_FALSE(re.Match(""));
    EXPECT_FALSE(re.Match("ab"));
    EXPECT_FALSE(re.Match("cd"));
}

TEST(Regex, ZeroOrOne1) {
    Regex re("a?");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match(""));
    EXPECT_TRUE(re.Match("a"));
    EXPECT_FALSE(re.Match("aa"));
}

TEST(Regex, ZeroOrOne2) {
    Regex re("a?b");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("b"));
    EXPECT_TRUE(re.Match("ab"));
    EXPECT_FALSE(re.Match("a"));
    EXPECT_FALSE(re.Match("aa"));
    EXPECT_FALSE(re.Match("bb"));
    EXPECT_FALSE(re.Match("ba"));
}

TEST(Regex, ZeroOrOne3) {
    Regex re("ab?");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("a"));
    EXPECT_TRUE(re.Match("ab"));
    EXPECT_FALSE(re.Match("b"));
    EXPECT_FALSE(re.Match("aa"));
    EXPECT_FALSE(re.Match("bb"));
    EXPECT_FALSE(re.Match("ba"));
}

TEST(Regex, ZeroOrOne4) {
    Regex re("a?b?");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match(""));
    EXPECT_TRUE(re.Match("a"));
    EXPECT_TRUE(re.Match("b"));
    EXPECT_TRUE(re.Match("ab"));
    EXPECT_FALSE(re.Match("aa"));
    EXPECT_FALSE(re.Match("bb"));
    EXPECT_FALSE(re.Match("ba"));
    EXPECT_FALSE(re.Match("abc"));
}

TEST(Regex, ZeroOrOne5) {
    Regex re("a(ab)?b");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("ab"));
    EXPECT_TRUE(re.Match("aabb"));
    EXPECT_FALSE(re.Match("aab"));
    EXPECT_FALSE(re.Match("abb"));
}

TEST(Regex, ZeroOrMore1) {
    Regex re("a*");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match(""));
    EXPECT_TRUE(re.Match("a"));
    EXPECT_TRUE(re.Match("aa"));
    EXPECT_FALSE(re.Match("b"));
    EXPECT_FALSE(re.Match("ab"));
}

TEST(Regex, ZeroOrMore2) {
    Regex re("a*b");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("b"));
    EXPECT_TRUE(re.Match("ab"));
    EXPECT_TRUE(re.Match("aab"));
    EXPECT_FALSE(re.Match(""));
    EXPECT_FALSE(re.Match("bb"));
}

TEST(Regex, ZeroOrMore3) {
    Regex re("a*b*");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match(""));
    EXPECT_TRUE(re.Match("a"));
    EXPECT_TRUE(re.Match("aa"));
    EXPECT_TRUE(re.Match("b"));
    EXPECT_TRUE(re.Match("bb"));
    EXPECT_TRUE(re.Match("ab"));
    EXPECT_TRUE(re.Match("aabb"));
    EXPECT_FALSE(re.Match("ba"));
}

TEST(Regex, ZeroOrMore4) {
    Regex re("a(ab)*b");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("ab"));
    EXPECT_TRUE(re.Match("aabb"));
    EXPECT_TRUE(re.Match("aababb"));
    EXPECT_FALSE(re.Match(""));
    EXPECT_FALSE(re.Match("aa"));
}

TEST(Regex, OneOrMore1) {
    Regex re("a+");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("a"));
    EXPECT_TRUE(re.Match("aa"));
    EXPECT_FALSE(re.Match(""));
    EXPECT_FALSE(re.Match("b"));
    EXPECT_FALSE(re.Match("ab"));
}

TEST(Regex, OneOrMore2) {
    Regex re("a+b");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("ab"));
    EXPECT_TRUE(re.Match("aab"));
    EXPECT_FALSE(re.Match(""));
    EXPECT_FALSE(re.Match("b"));
}

TEST(Regex, OneOrMore3) {
    Regex re("a+b+");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("ab"));
    EXPECT_TRUE(re.Match("aab"));
    EXPECT_TRUE(re.Match("abb"));
    EXPECT_TRUE(re.Match("aabb"));
    EXPECT_FALSE(re.Match(""));
    EXPECT_FALSE(re.Match("b"));
    EXPECT_FALSE(re.Match("ba"));
}

TEST(Regex, OneOrMore4) {
    Regex re("a(ab)+b");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("aabb"));
    EXPECT_TRUE(re.Match("aababb"));
    EXPECT_FALSE(re.Match(""));
    EXPECT_FALSE(re.Match("ab"));
}

TEST(Regex, QuantifierExact1) {
    Regex re("ab{3}c");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("abbbc"));
    EXPECT_FALSE(re.Match("ac"));
    EXPECT_FALSE(re.Match("abc"));
    EXPECT_FALSE(re.Match("abbc"));
    EXPECT_FALSE(re.Match("abbbbc"));
}

TEST(Regex, QuantifierExact2) {
    Regex re("a(bc){3}d");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("abcbcbcd"));
    EXPECT_FALSE(re.Match("ad"));
    EXPECT_FALSE(re.Match("abcd"));
    EXPECT_FALSE(re.Match("abcbcd"));
    EXPECT_FALSE(re.Match("abcbcbcbcd"));
}

TEST(Regex, QuantifierExact3) {
    Regex re("a(b|c){3}d");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("abbbd"));
    EXPECT_TRUE(re.Match("acccd"));
    EXPECT_TRUE(re.Match("abcbd"));
    EXPECT_FALSE(re.Match("ad"));
    EXPECT_FALSE(re.Match("abbd"));
    EXPECT_FALSE(re.Match("accccd"));
    EXPECT_FALSE(re.Match("abbbbd"));
}

TEST(Regex, QuantifierMin1) {
    Regex re("ab{3,}c");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("abbbc"));
    EXPECT_TRUE(re.Match("abbbbc"));
    EXPECT_TRUE(re.Match("abbbbbc"));
    EXPECT_FALSE(re.Match("ac"));
    EXPECT_FALSE(re.Match("abc"));
    EXPECT_FALSE(re.Match("abbc"));
}

TEST(Regex, QuantifierMin2) {
    Regex re("a(bc){3,}d");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("abcbcbcd"));
    EXPECT_TRUE(re.Match("abcbcbcbcd"));
    EXPECT_FALSE(re.Match("ad"));
    EXPECT_FALSE(re.Match("abcd"));
    EXPECT_FALSE(re.Match("abcbcd"));
}

TEST(Regex, QuantifierMin3) {
    Regex re("a(b|c){3,}d");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("abbbd"));
    EXPECT_TRUE(re.Match("acccd"));
    EXPECT_TRUE(re.Match("abcbd"));
    EXPECT_TRUE(re.Match("accccd"));
    EXPECT_TRUE(re.Match("abbbbd"));
    EXPECT_FALSE(re.Match("ad"));
    EXPECT_FALSE(re.Match("abbd"));
}

TEST(Regex, QuantifierMinMax1) {
    Regex re("ab{3,5}c");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("abbbc"));
    EXPECT_TRUE(re.Match("abbbbc"));
    EXPECT_TRUE(re.Match("abbbbbc"));
    EXPECT_FALSE(re.Match("ac"));
    EXPECT_FALSE(re.Match("abc"));
    EXPECT_FALSE(re.Match("abbc"));
    EXPECT_FALSE(re.Match("abbbbbbc"));
}

TEST(Regex, QuantifierMinMax2) {
    Regex re("a(bc){3,5}d");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("abcbcbcd"));
    EXPECT_TRUE(re.Match("abcbcbcbcd"));
    EXPECT_TRUE(re.Match("abcbcbcbcbcd"));
    EXPECT_FALSE(re.Match("ad"));
    EXPECT_FALSE(re.Match("abcd"));
    EXPECT_FALSE(re.Match("abcbcd"));
    EXPECT_FALSE(re.Match("abcbcbcbcbcbcd"));
}

TEST(Regex, QuantifierMinMax3) {
    Regex re("a(b|c){3,5}d");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("abbbd"));
    EXPECT_TRUE(re.Match("acccd"));
    EXPECT_TRUE(re.Match("abcbd"));
    EXPECT_TRUE(re.Match("accccd"));
    EXPECT_TRUE(re.Match("abbbbd"));
    EXPECT_TRUE(re.Match("acccccd"));
    EXPECT_TRUE(re.Match("abbbbbd"));
    EXPECT_FALSE(re.Match("ad"));
    EXPECT_FALSE(re.Match("abbd"));
    EXPECT_FALSE(re.Match("accccccd"));
    EXPECT_FALSE(re.Match("abbbbbbd"));
}

// Issue538
TEST(Regex, QuantifierMinMax4) {
    Regex re("a(b|c){0,3}d");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("ad"));
    EXPECT_TRUE(re.Match("abd"));
    EXPECT_TRUE(re.Match("acd"));
    EXPECT_TRUE(re.Match("abbd"));
    EXPECT_TRUE(re.Match("accd"));
    EXPECT_TRUE(re.Match("abcd"));
    EXPECT_TRUE(re.Match("abbbd"));
    EXPECT_TRUE(re.Match("acccd"));
    EXPECT_FALSE(re.Match("abbbbd"));
    EXPECT_FALSE(re.Match("add"));
    EXPECT_FALSE(re.Match("accccd"));
    EXPECT_FALSE(re.Match("abcbcd"));
}

// Issue538
TEST(Regex, QuantifierMinMax5) {
    Regex re("a(b|c){0,}d");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("ad"));
    EXPECT_TRUE(re.Match("abd"));
    EXPECT_TRUE(re.Match("acd"));
    EXPECT_TRUE(re.Match("abbd"));
    EXPECT_TRUE(re.Match("accd"));
    EXPECT_TRUE(re.Match("abcd"));
    EXPECT_TRUE(re.Match("abbbd"));
    EXPECT_TRUE(re.Match("acccd"));
    EXPECT_TRUE(re.Match("abbbbd"));
    EXPECT_TRUE(re.Match("accccd"));
    EXPECT_TRUE(re.Match("abcbcd"));
    EXPECT_FALSE(re.Match("add"));
    EXPECT_FALSE(re.Match("aad"));
}

#define EURO "\xE2\x82\xAC" // "\xE2\x82\xAC" is UTF-8 sequence of Euro sign U+20AC

TEST(Regex, Unicode) {
    Regex re("a" EURO "+b"); 
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("a" EURO "b"));
    EXPECT_TRUE(re.Match("a" EURO EURO "b"));
    EXPECT_FALSE(re.Match("a?b"));
    EXPECT_FALSE(re.Match("a" EURO "\xAC" "b")); // unaware of UTF-8 will match
}

TEST(Regex, AnyCharacter) {
    Regex re(".");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("a"));
    EXPECT_TRUE(re.Match("b"));
    EXPECT_TRUE(re.Match(EURO));
    EXPECT_FALSE(re.Match(""));
    EXPECT_FALSE(re.Match("aa"));
}

TEST(Regex, CharacterRange1) {
    Regex re("[abc]");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("a"));
    EXPECT_TRUE(re.Match("b"));
    EXPECT_TRUE(re.Match("c"));
    EXPECT_FALSE(re.Match(""));
    EXPECT_FALSE(re.Match("`"));
    EXPECT_FALSE(re.Match("d"));
    EXPECT_FALSE(re.Match("aa"));
}

TEST(Regex, CharacterRange2) {
    Regex re("[^abc]");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("`"));
    EXPECT_TRUE(re.Match("d"));
    EXPECT_FALSE(re.Match("a"));
    EXPECT_FALSE(re.Match("b"));
    EXPECT_FALSE(re.Match("c"));
    EXPECT_FALSE(re.Match(""));
    EXPECT_FALSE(re.Match("aa"));
}

TEST(Regex, CharacterRange3) {
    Regex re("[a-c]");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("a"));
    EXPECT_TRUE(re.Match("b"));
    EXPECT_TRUE(re.Match("c"));
    EXPECT_FALSE(re.Match(""));
    EXPECT_FALSE(re.Match("`"));
    EXPECT_FALSE(re.Match("d"));
    EXPECT_FALSE(re.Match("aa"));
}

TEST(Regex, CharacterRange4) {
    Regex re("[^a-c]");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("`"));
    EXPECT_TRUE(re.Match("d"));
    EXPECT_FALSE(re.Match("a"));
    EXPECT_FALSE(re.Match("b"));
    EXPECT_FALSE(re.Match("c"));
    EXPECT_FALSE(re.Match(""));
    EXPECT_FALSE(re.Match("aa"));
}

TEST(Regex, CharacterRange5) {
    Regex re("[-]");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("-"));
    EXPECT_FALSE(re.Match(""));
    EXPECT_FALSE(re.Match("a"));
}

TEST(Regex, CharacterRange6) {
    Regex re("[a-]");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("a"));
    EXPECT_TRUE(re.Match("-"));
    EXPECT_FALSE(re.Match(""));
    EXPECT_FALSE(re.Match("`"));
    EXPECT_FALSE(re.Match("b"));
}

TEST(Regex, CharacterRange7) {
    Regex re("[-a]");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("a"));
    EXPECT_TRUE(re.Match("-"));
    EXPECT_FALSE(re.Match(""));
    EXPECT_FALSE(re.Match("`"));
    EXPECT_FALSE(re.Match("b"));
}

TEST(Regex, CharacterRange8) {
    Regex re("[a-zA-Z0-9]*");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("Milo"));
    EXPECT_TRUE(re.Match("MT19937"));
    EXPECT_TRUE(re.Match("43"));
    EXPECT_FALSE(re.Match("a_b"));
    EXPECT_FALSE(re.Match("!"));
}

TEST(Regex, Search) {
    Regex re("abc");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Search("abc"));
    EXPECT_TRUE(re.Search("_abc"));
    EXPECT_TRUE(re.Search("abc_"));
    EXPECT_TRUE(re.Search("_abc_"));
    EXPECT_TRUE(re.Search("__abc__"));
    EXPECT_TRUE(re.Search("abcabc"));
    EXPECT_FALSE(re.Search("a"));
    EXPECT_FALSE(re.Search("ab"));
    EXPECT_FALSE(re.Search("bc"));
    EXPECT_FALSE(re.Search("cba"));
}

TEST(Regex, Search_BeginAnchor) {
    Regex re("^abc");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Search("abc"));
    EXPECT_TRUE(re.Search("abc_"));
    EXPECT_TRUE(re.Search("abcabc"));
    EXPECT_FALSE(re.Search("_abc"));
    EXPECT_FALSE(re.Search("_abc_"));
    EXPECT_FALSE(re.Search("a"));
    EXPECT_FALSE(re.Search("ab"));
    EXPECT_FALSE(re.Search("bc"));
    EXPECT_FALSE(re.Search("cba"));
}

TEST(Regex, Search_EndAnchor) {
    Regex re("abc$");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Search("abc"));
    EXPECT_TRUE(re.Search("_abc"));
    EXPECT_TRUE(re.Search("abcabc"));
    EXPECT_FALSE(re.Search("abc_"));
    EXPECT_FALSE(re.Search("_abc_"));
    EXPECT_FALSE(re.Search("a"));
    EXPECT_FALSE(re.Search("ab"));
    EXPECT_FALSE(re.Search("bc"));
    EXPECT_FALSE(re.Search("cba"));
}

TEST(Regex, Search_BothAnchor) {
    Regex re("^abc$");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Search("abc"));
    EXPECT_FALSE(re.Search(""));
    EXPECT_FALSE(re.Search("a"));
    EXPECT_FALSE(re.Search("b"));
    EXPECT_FALSE(re.Search("ab"));
    EXPECT_FALSE(re.Search("abcd"));
}

TEST(Regex, Escape) {
    const char* s = "\\^\\$\\|\\(\\)\\?\\*\\+\\.\\[\\]\\{\\}\\\\\\f\\n\\r\\t\\v[\\b][\\[][\\]]";
    Regex re(s);
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("^$|()?*+.[]{}\\\x0C\n\r\t\x0B\b[]"));
    EXPECT_FALSE(re.Match(s)); // Not escaping
}

TEST(Regex, Invalid) {
#define TEST_INVALID(s) \
    {\
        Regex re(s);\
        EXPECT_FALSE(re.IsValid());\
    }

    TEST_INVALID("");
    TEST_INVALID("a|");
    TEST_INVALID("()");
    TEST_INVALID(")");
    TEST_INVALID("(a))");
    TEST_INVALID("(a|)");
    TEST_INVALID("(a||b)");
    TEST_INVALID("(|b)");
    TEST_INVALID("?");
    TEST_INVALID("*");
    TEST_INVALID("+");
    TEST_INVALID("{");
    TEST_INVALID("{}");
    TEST_INVALID("a{a}");
    TEST_INVALID("a{0}");
    TEST_INVALID("a{-1}");
    TEST_INVALID("a{}");
    // TEST_INVALID("a{0,}");   // Support now
    TEST_INVALID("a{,0}");
    TEST_INVALID("a{1,0}");
    TEST_INVALID("a{-1,0}");
    TEST_INVALID("a{-1,1}");
    TEST_INVALID("[]");
    TEST_INVALID("[^]");
    TEST_INVALID("[\\a]");
    TEST_INVALID("\\a");

#undef TEST_INVALID
}

TEST(Regex, Issue538) {
    Regex re("^[0-9]+(\\\\.[0-9]+){0,2}");
    EXPECT_TRUE(re.IsValid());
}

#undef EURO
