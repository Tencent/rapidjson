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

TEST(Regex, Escape) {
    const char* s = "\\|\\(\\)\\?\\*\\+\\.\\[\\]\\\\\\f\\n\\r\\t\\v";
    Regex re(s);
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("|()?*+.[]\\\x0C\n\r\t\x0B"));
    EXPECT_FALSE(re.Match(s)); // Not escaping
}

#undef EURO
