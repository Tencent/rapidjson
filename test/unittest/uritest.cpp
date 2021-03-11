// Tencent is pleased to support the open source community by making RapidJSON available.
// 
// Copyright (C) 2015 THL A29 Limited, a Tencent company, and Milo Yip.
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

#define RAPIDJSON_SCHEMA_VERBOSE 0
#define RAPIDJSON_HAS_STDSTRING 1

#include "unittest.h"
#include "rapidjson/document.h"
#include "rapidjson/uri.h"

#ifdef __clang__
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(variadic-macros)
#elif defined(_MSC_VER)
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(4822) // local class member function does not have a body
#endif

using namespace rapidjson;

TEST(Uri, Parse) {
    typedef std::basic_string<Value::Ch> String;
    typedef GenericUri<Value, MemoryPoolAllocator<> > Uri;
    MemoryPoolAllocator<CrtAllocator> allocator;

    String s = "http://auth/path?query#frag";
    Value v;
    v.SetString(s, allocator);
    Uri u = Uri(v);
    EXPECT_TRUE(u.GetScheme() == "http:");
    EXPECT_TRUE(u.GetAuth() == "//auth");
    EXPECT_TRUE(u.GetPath() == "/path");
    EXPECT_TRUE(u.GetBase() == "http://auth/path?query");
    EXPECT_TRUE(u.GetQuery() == "?query");
    EXPECT_TRUE(u.GetFrag() == "#frag");
    Value w;
    u.Get(w, allocator);
    EXPECT_TRUE(*w.GetString() == *v.GetString());

    s = "urn:uuid:ee564b8a-7a87-4125-8c96-e9f123d6766f";
    v.SetString(s, allocator);
    u = Uri(v);
    EXPECT_TRUE(u.GetScheme() == "urn:");
    EXPECT_TRUE(u.GetAuth() == "");
    EXPECT_TRUE(u.GetPath() == "uuid:ee564b8a-7a87-4125-8c96-e9f123d6766f");
    EXPECT_TRUE(u.GetBase() == "urn:uuid:ee564b8a-7a87-4125-8c96-e9f123d6766f");
    EXPECT_TRUE(u.GetQuery() == "");
    EXPECT_TRUE(u.GetFrag() == "");
    u.Get(w, allocator);
    EXPECT_TRUE(*w.GetString() == *v.GetString());

    s = "";
    v.SetString(s, allocator);
    u = Uri(v);
    EXPECT_TRUE(u.GetScheme() == "");
    EXPECT_TRUE(u.GetAuth() == "");
    EXPECT_TRUE(u.GetPath() == "");
    EXPECT_TRUE(u.GetBase() == "");
    EXPECT_TRUE(u.GetQuery() == "");
    EXPECT_TRUE(u.GetFrag() == "");

    s = "http://auth/";
    v.SetString(s, allocator);
    u = Uri(v);
    EXPECT_TRUE(u.GetScheme() == "http:");
    EXPECT_TRUE(u.GetAuth() == "//auth");
    EXPECT_TRUE(u.GetPath() == "/");
    EXPECT_TRUE(u.GetBase() == "http://auth/");
    EXPECT_TRUE(u.GetQuery() == "");
    EXPECT_TRUE(u.GetFrag() == "");

    s = "/path/sub";
    u = Uri(s);
    EXPECT_TRUE(u.GetScheme() == "");
    EXPECT_TRUE(u.GetAuth() == "");
    EXPECT_TRUE(u.GetPath() == "/path/sub");
    EXPECT_TRUE(u.GetBase() == "/path/sub");
    EXPECT_TRUE(u.GetQuery() == "");
    EXPECT_TRUE(u.GetFrag() == "");

    // absolute path gets normalized
    s = "/path/../sub/";
    u = Uri(s);
    EXPECT_TRUE(u.GetScheme() == "");
    EXPECT_TRUE(u.GetAuth() == "");
    EXPECT_TRUE(u.GetPath() == "/sub/");
    EXPECT_TRUE(u.GetBase() == "/sub/");
    EXPECT_TRUE(u.GetQuery() == "");
    EXPECT_TRUE(u.GetFrag() == "");

    // relative path does not
    s = "path/../sub";
    u = Uri(s);
    EXPECT_TRUE(u.GetScheme() == "");
    EXPECT_TRUE(u.GetAuth() == "");
    EXPECT_TRUE(u.GetPath() == "path/../sub");
    EXPECT_TRUE(u.GetBase() == "path/../sub");
    EXPECT_TRUE(u.GetQuery() == "");
    EXPECT_TRUE(u.GetFrag() == "");

    s = "http://auth#frag/stuff";
    u = Uri(s);
    EXPECT_TRUE(u.GetScheme() == "http:");
    EXPECT_TRUE(u.GetAuth() == "//auth");
    EXPECT_TRUE(u.GetPath() == "");
    EXPECT_TRUE(u.GetBase() == "http://auth");
    EXPECT_TRUE(u.GetQuery() == "");
    EXPECT_TRUE(u.GetFrag() == "#frag/stuff");
    EXPECT_TRUE(u.Get() == s);

    s = "#frag/stuff";
    u = Uri(s);
    EXPECT_TRUE(u.GetScheme() == "");
    EXPECT_TRUE(u.GetAuth() == "");
    EXPECT_TRUE(u.GetPath() == "");
    EXPECT_TRUE(u.GetBase() == "");
    EXPECT_TRUE(u.GetQuery() == "");
    EXPECT_TRUE(u.GetFrag() == "#frag/stuff");
    EXPECT_TRUE(u.Get() == s);

    Value::Ch c[] = { '#', 'f', 'r', 'a', 'g', '/', 's', 't', 'u', 'f', 'f', '\0'};
    u = Uri(c, 11);
    EXPECT_TRUE(String(u.GetString()) == "#frag/stuff");
    EXPECT_TRUE(u.GetStringLength() == 11);
    EXPECT_TRUE(String(u.GetBaseString()) == "");
    EXPECT_TRUE(u.GetBaseStringLength() == 0);
    EXPECT_TRUE(String(u.GetFragString()) == "#frag/stuff");
    EXPECT_TRUE(u.GetFragStringLength() == 11);
}

TEST(Uri, Resolve) {
    typedef std::basic_string<Value::Ch> String;
    typedef GenericUri<Value, MemoryPoolAllocator<> > Uri;

    // ref is full uri
    Uri base = Uri(String("http://auth/path/#frag"));
    Uri ref = Uri(String("http://newauth/newpath#newfrag"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://newauth/newpath#newfrag");

    base = Uri(String("/path/#frag"));
    ref = Uri(String("http://newauth/newpath#newfrag"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://newauth/newpath#newfrag");

    // ref is alternate uri
    base = Uri(String("http://auth/path/#frag"));
    ref = Uri(String("urn:uuid:ee564b8a-7a87-4125-8c96-e9f123d6766f"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "urn:uuid:ee564b8a-7a87-4125-8c96-e9f123d6766f");

    // ref is absolute path
    base = Uri(String("http://auth/path/#"));
    ref = Uri(String("/newpath#newfrag"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://auth/newpath#newfrag");

    // ref is relative path
    base = Uri(String("http://auth/path/file.json#frag"));
    ref = Uri(String("newfile.json#"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://auth/path/newfile.json#");

    base = Uri(String("http://auth/path/file.json#frag/stuff"));
    ref = Uri(String("newfile.json#newfrag/newstuff"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://auth/path/newfile.json#newfrag/newstuff");

    base = Uri(String("file.json"));
    ref = Uri(String("newfile.json"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "newfile.json");

    base = Uri(String("file.json"));
    ref = Uri(String("./newfile.json"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "newfile.json");

    base = Uri(String("file.json"));
    ref = Uri(String("parent/../newfile.json"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "newfile.json");

    base = Uri(String("file.json"));
    ref = Uri(String("parent/./newfile.json"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "parent/newfile.json");

    base = Uri(String("file.json"));
    ref = Uri(String("../../parent/.././newfile.json"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "newfile.json");

    base = Uri(String("http://auth"));
    ref = Uri(String("newfile.json"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://auth/newfile.json");

    // ref is fragment
    base = Uri(String("#frag/stuff"));
    ref = Uri(String("#newfrag/newstuff"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "#newfrag/newstuff");

    // test ref fragment always wins
    base = Uri(String("/path#frag"));
    ref = Uri(String(""));
    EXPECT_TRUE(ref.Resolve(base).Get() == "/path");

    // Examples from RFC3896
    base = Uri(String("http://a/b/c/d;p?q"));
    ref = Uri(String("g:h"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "g:h");
    ref = Uri(String("g"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/g");
    ref = Uri(String("./g"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/g");
    ref = Uri(String("g/"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/g/");
    ref = Uri(String("/g"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/g");
    ref = Uri(String("//g"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://g");
    ref = Uri(String("?y"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/d;p?y");
    ref = Uri(String("g?y"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/g?y");
    ref = Uri(String("#s"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/d;p?q#s");
    ref = Uri(String("g#s"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/g#s");
    ref = Uri(String("g?y#s"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/g?y#s");
    ref = Uri(String(";x"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/;x");
    ref = Uri(String("g;x"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/g;x");
    ref = Uri(String("g;x?y#s"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/g;x?y#s");
    ref = Uri(String(""));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/d;p?q");
    ref = Uri(String("."));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/");
    ref = Uri(String("./"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/");
    ref = Uri(String(".."));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/");
    ref = Uri(String("../"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/");
    ref = Uri(String("../g"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/g");
    ref = Uri(String("../.."));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/");
    ref = Uri(String("../../"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/");
     ref = Uri(String("../../g"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/g");
    ref = Uri(String("../../../g"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/g");
    ref = Uri(String("../../../../g"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/g");
    ref = Uri(String("/./g"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/g");
    ref = Uri(String("/../g"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/g");
    ref = Uri(String("g."));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/g.");
    ref = Uri(String(".g"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/.g");
    ref = Uri(String("g.."));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/g..");
    ref = Uri(String("..g"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/..g");
    ref = Uri(String("g#s/../x"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/g#s/../x");
}

#if defined(_MSC_VER) || defined(__clang__)
RAPIDJSON_DIAG_POP
#endif
