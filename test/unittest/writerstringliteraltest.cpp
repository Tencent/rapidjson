// Tencent is pleased to support the open source community by making RapidJSON available.
//
// Copyright (C) 2015 THL A29 Limited, a Tencent company, Milo Yip and Bruno Nicoletti. All rights reserved.
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

#ifndef RAPIDJSON_HAS_STDSTRING
#define RAPIDJSON_HAS_STDSTRING
#endif

#include "unittest.h"
#include "rapidjson/writer.h"

using namespace rapidjson;

TEST(StringLiterals, IsCallingStringLiterals)
{
  // Unless this is enabled across all translation units
  // the unit test wont work. The linker appears to be eliding
  // versions compiled into this translation unit in favour of
  // others. Poo.
#ifdef RAPIDJSON_UNIT_TEST_STRING_LITERALS
  StringBuffer buffer;
  Writer<StringBuffer> writer(buffer);
  writer.StartObject();

  writer.String("banana");
  EXPECT_EQ(writer.lastStringCallWasToStringLiteralVersion(), true);

  const char *str2 = strdup("oranges");
  writer.String(str2);
  EXPECT_EQ(writer.lastStringCallWasToStringLiteralVersion(), false);
  free((void*)str2);

  std::string str3("pears");
  writer.String(str3);
  EXPECT_EQ(writer.lastStringCallWasToStringLiteralVersion(), false);

  writer.String("grapes");
  EXPECT_EQ(writer.lastStringCallWasToStringLiteralVersion(), true);

  writer.EndObject();
#endif
}
