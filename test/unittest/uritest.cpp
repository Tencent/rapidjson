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
    typedef GenericUri<Value, MemoryPoolAllocator<> > UriType;
    MemoryPoolAllocator<CrtAllocator> allocator;

    String s = "http://auth/path?query#frag";
    Value v;
    v.SetString(s, allocator);
    UriType u = UriType(v);
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
    u = UriType(v);
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
    u = UriType(v);
    EXPECT_TRUE(u.GetScheme() == "");
    EXPECT_TRUE(u.GetAuth() == "");
    EXPECT_TRUE(u.GetPath() == "");
    EXPECT_TRUE(u.GetBase() == "");
    EXPECT_TRUE(u.GetQuery() == "");
    EXPECT_TRUE(u.GetFrag() == "");

    s = "http://auth/";
    v.SetString(s, allocator);
    u = UriType(v);
    EXPECT_TRUE(u.GetScheme() == "http:");
    EXPECT_TRUE(u.GetAuth() == "//auth");
    EXPECT_TRUE(u.GetPath() == "/");
    EXPECT_TRUE(u.GetBase() == "http://auth/");
    EXPECT_TRUE(u.GetQuery() == "");
    EXPECT_TRUE(u.GetFrag() == "");

    s = "/path/sub";
    u = UriType(s);
    EXPECT_TRUE(u.GetScheme() == "");
    EXPECT_TRUE(u.GetAuth() == "");
    EXPECT_TRUE(u.GetPath() == "/path/sub");
    EXPECT_TRUE(u.GetBase() == "/path/sub");
    EXPECT_TRUE(u.GetQuery() == "");
    EXPECT_TRUE(u.GetFrag() == "");

    // absolute path gets normalized
    s = "/path/../sub/";
    u = UriType(s);
    EXPECT_TRUE(u.GetScheme() == "");
    EXPECT_TRUE(u.GetAuth() == "");
    EXPECT_TRUE(u.GetPath() == "/sub/");
    EXPECT_TRUE(u.GetBase() == "/sub/");
    EXPECT_TRUE(u.GetQuery() == "");
    EXPECT_TRUE(u.GetFrag() == "");

    // relative path does not
    s = "path/../sub";
    u = UriType(s);
    EXPECT_TRUE(u.GetScheme() == "");
    EXPECT_TRUE(u.GetAuth() == "");
    EXPECT_TRUE(u.GetPath() == "path/../sub");
    EXPECT_TRUE(u.GetBase() == "path/../sub");
    EXPECT_TRUE(u.GetQuery() == "");
    EXPECT_TRUE(u.GetFrag() == "");

    s = "http://auth#frag/stuff";
    u = UriType(s);
    EXPECT_TRUE(u.GetScheme() == "http:");
    EXPECT_TRUE(u.GetAuth() == "//auth");
    EXPECT_TRUE(u.GetPath() == "");
    EXPECT_TRUE(u.GetBase() == "http://auth");
    EXPECT_TRUE(u.GetQuery() == "");
    EXPECT_TRUE(u.GetFrag() == "#frag/stuff");
    EXPECT_TRUE(u.Get() == s);

    s = "#frag/stuff";
    u = UriType(s);
    EXPECT_TRUE(u.GetScheme() == "");
    EXPECT_TRUE(u.GetAuth() == "");
    EXPECT_TRUE(u.GetPath() == "");
    EXPECT_TRUE(u.GetBase() == "");
    EXPECT_TRUE(u.GetQuery() == "");
    EXPECT_TRUE(u.GetFrag() == "#frag/stuff");
    EXPECT_TRUE(u.Get() == s);

    Value::Ch c[] = { '#', 'f', 'r', 'a', 'g', '/', 's', 't', 'u', 'f', 'f', '\0'};
    u = UriType(c, 11);
    EXPECT_TRUE(String(u.GetString()) == "#frag/stuff");
    EXPECT_TRUE(u.GetStringLength() == 11);
    EXPECT_TRUE(String(u.GetBaseString()) == "");
    EXPECT_TRUE(u.GetBaseStringLength() == 0);
    EXPECT_TRUE(String(u.GetFragString()) == "#frag/stuff");
    EXPECT_TRUE(u.GetFragStringLength() == 11);
}

TEST(Uri, Resolve) {
    typedef std::basic_string<Value::Ch> String;
    typedef GenericUri<Value, MemoryPoolAllocator<> > UriType;

    // ref is full uri
    UriType base = UriType(String("http://auth/path/#frag"));
    UriType ref = UriType(String("http://newauth/newpath#newfrag"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://newauth/newpath#newfrag");

    base = UriType(String("/path/#frag"));
    ref = UriType(String("http://newauth/newpath#newfrag"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://newauth/newpath#newfrag");

    // ref is alternate uri
    base = UriType(String("http://auth/path/#frag"));
    ref = UriType(String("urn:uuid:ee564b8a-7a87-4125-8c96-e9f123d6766f"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "urn:uuid:ee564b8a-7a87-4125-8c96-e9f123d6766f");

    // ref is absolute path
    base = UriType(String("http://auth/path/#"));
    ref = UriType(String("/newpath#newfrag"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://auth/newpath#newfrag");

    // ref is relative path
    base = UriType(String("http://auth/path/file.json#frag"));
    ref = UriType(String("newfile.json#"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://auth/path/newfile.json#");

    base = UriType(String("http://auth/path/file.json#frag/stuff"));
    ref = UriType(String("newfile.json#newfrag/newstuff"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://auth/path/newfile.json#newfrag/newstuff");

    base = UriType(String("file.json"));
    ref = UriType(String("newfile.json"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "newfile.json");

    base = UriType(String("file.json"));
    ref = UriType(String("./newfile.json"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "newfile.json");

    base = UriType(String("file.json"));
    ref = UriType(String("parent/../newfile.json"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "newfile.json");

    base = UriType(String("file.json"));
    ref = UriType(String("parent/./newfile.json"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "parent/newfile.json");

    base = UriType(String("file.json"));
    ref = UriType(String("../../parent/.././newfile.json"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "newfile.json");

    base = UriType(String("http://auth"));
    ref = UriType(String("newfile.json"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://auth/newfile.json");

    // ref is fragment
    base = UriType(String("#frag/stuff"));
    ref = UriType(String("#newfrag/newstuff"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "#newfrag/newstuff");

    // test ref fragment always wins
    base = UriType(String("/path#frag"));
    ref = UriType(String(""));
    EXPECT_TRUE(ref.Resolve(base).Get() == "/path");

    // Examples from RFC3896
    base = UriType(String("http://a/b/c/d;p?q"));
    ref = UriType(String("g:h"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "g:h");
    ref = UriType(String("g"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/g");
    ref = UriType(String("./g"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/g");
    ref = UriType(String("g/"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/g/");
    ref = UriType(String("/g"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/g");
    ref = UriType(String("//g"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://g");
    ref = UriType(String("?y"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/d;p?y");
    ref = UriType(String("g?y"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/g?y");
    ref = UriType(String("#s"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/d;p?q#s");
    ref = UriType(String("g#s"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/g#s");
    ref = UriType(String("g?y#s"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/g?y#s");
    ref = UriType(String(";x"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/;x");
    ref = UriType(String("g;x"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/g;x");
    ref = UriType(String("g;x?y#s"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/g;x?y#s");
    ref = UriType(String(""));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/d;p?q");
    ref = UriType(String("."));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/");
    ref = UriType(String("./"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/");
    ref = UriType(String(".."));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/");
    ref = UriType(String("../"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/");
    ref = UriType(String("../g"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/g");
    ref = UriType(String("../.."));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/");
    ref = UriType(String("../../"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/");
     ref = UriType(String("../../g"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/g");
    ref = UriType(String("../../../g"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/g");
    ref = UriType(String("../../../../g"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/g");
    ref = UriType(String("/./g"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/g");
    ref = UriType(String("/../g"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/g");
    ref = UriType(String("g."));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/g.");
    ref = UriType(String(".g"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/.g");
    ref = UriType(String("g.."));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/g..");
    ref = UriType(String("..g"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/..g");
    ref = UriType(String("g#s/../x"));
    EXPECT_TRUE(ref.Resolve(base).Get() == "http://a/b/c/g#s/../x");
}

#if defined(_MSC_VER) || defined(__clang__)
RAPIDJSON_DIAG_POP
#endif
