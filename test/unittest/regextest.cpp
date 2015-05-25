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

TEST(Regex, concatenation) {
    Regex re("abc");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("abc"));
    EXPECT_FALSE(re.Match(""));
    EXPECT_FALSE(re.Match("a"));
    EXPECT_FALSE(re.Match("b"));
    EXPECT_FALSE(re.Match("ab"));
    EXPECT_FALSE(re.Match("abcd"));
}

TEST(Regex, split1) {
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

TEST(Regex, split2) {
    Regex re("a|b|c");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("a"));
    EXPECT_TRUE(re.Match("b"));
    EXPECT_TRUE(re.Match("c"));
    EXPECT_FALSE(re.Match(""));
    EXPECT_FALSE(re.Match("aa"));
    EXPECT_FALSE(re.Match("ab"));
}

TEST(Regex, parenthesis1) {
    Regex re("(ab)c");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("abc"));
    EXPECT_FALSE(re.Match(""));
    EXPECT_FALSE(re.Match("a"));
    EXPECT_FALSE(re.Match("b"));
    EXPECT_FALSE(re.Match("ab"));
    EXPECT_FALSE(re.Match("abcd"));
}

TEST(Regex, parenthesis2) {
    Regex re("a(bc)");
    ASSERT_TRUE(re.IsValid());
    EXPECT_TRUE(re.Match("abc"));
    EXPECT_FALSE(re.Match(""));
    EXPECT_FALSE(re.Match("a"));
    EXPECT_FALSE(re.Match("b"));
    EXPECT_FALSE(re.Match("ab"));
    EXPECT_FALSE(re.Match("abcd"));
}

TEST(Regex, parenthesis3) {
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
