// Tencent is pleased to support the open source community by making RapidJSON available.
//
// (C) Copyright IBM Corporation 2021
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
typedef GenericUri<Value, MemoryPoolAllocator<> > UriType;
MemoryPoolAllocator<CrtAllocator> allocator;
Value v;
Value w;

v.SetString("http://auth/path/xxx?query#frag", allocator);
UriType u = UriType(v, &allocator);
EXPECT_TRUE(StrCmp(u.GetSchemeString(), "http:") == 0);
EXPECT_TRUE(StrCmp(u.GetAuthString(), "//auth") == 0);
EXPECT_TRUE(StrCmp(u.GetPathString(), "/path/xxx") == 0);
EXPECT_TRUE(StrCmp(u.GetBaseString(), "http://auth/path/xxx?query") == 0);
EXPECT_TRUE(StrCmp(u.GetQueryString(), "?query") == 0);
EXPECT_TRUE(StrCmp(u.GetFragString(), "#frag") == 0);
u.Get(w, allocator);
EXPECT_TRUE(*w.GetString() == *v.GetString());

#if RAPIDJSON_HAS_STDSTRING
typedef std::basic_string<Value::Ch> String;
String str = "http://auth/path/xxx?query#frag";
const UriType uri = UriType(str);
EXPECT_TRUE(UriType::GetScheme(uri) == "http:");
EXPECT_TRUE(UriType::GetAuth(uri) == "//auth");
EXPECT_TRUE(UriType::GetPath(uri) == "/path/xxx");
EXPECT_TRUE(UriType::GetBase(uri) == "http://auth/path/xxx?query");
EXPECT_TRUE(UriType::GetQuery(uri) == "?query");
EXPECT_TRUE(UriType::GetFrag(uri) == "#frag");
EXPECT_TRUE(UriType::Get(uri) == str);
#endif

v.SetString("urn:uuid:ee564b8a-7a87-4125-8c96-e9f123d6766f", allocator);
u = UriType(v);
EXPECT_TRUE(StrCmp(u.GetSchemeString(), "urn:") == 0);
EXPECT_TRUE(u.GetAuthStringLength() == 0);
EXPECT_TRUE(StrCmp(u.GetPathString(), "uuid:ee564b8a-7a87-4125-8c96-e9f123d6766f") == 0);
EXPECT_TRUE(StrCmp(u.GetBaseString(), "urn:uuid:ee564b8a-7a87-4125-8c96-e9f123d6766f") == 0);
EXPECT_TRUE(u.GetQueryStringLength() == 0);
EXPECT_TRUE(u.GetFragStringLength() == 0);
u.Get(w, allocator);
EXPECT_TRUE(*w.GetString() == *v.GetString());

v.SetString("", allocator);
u = UriType(v);
EXPECT_TRUE(u.GetSchemeStringLength() == 0);
EXPECT_TRUE(u.GetAuthStringLength() == 0);
EXPECT_TRUE(u.GetPathStringLength() == 0);
EXPECT_TRUE(u.GetBaseStringLength() == 0);
EXPECT_TRUE(u.GetQueryStringLength() == 0);
EXPECT_TRUE(u.GetFragStringLength() == 0);

v.SetString("http://auth/", allocator);
u = UriType(v);
EXPECT_TRUE(StrCmp(u.GetSchemeString(), "http:") == 0);
EXPECT_TRUE(StrCmp(u.GetAuthString(), "//auth") == 0);
EXPECT_TRUE(StrCmp(u.GetPathString(), "/") == 0);
EXPECT_TRUE(StrCmp(u.GetBaseString(), "http://auth/") == 0);
EXPECT_TRUE(u.GetQueryStringLength() == 0);
EXPECT_TRUE(u.GetFragStringLength() == 0);

u = UriType("/path/sub");
EXPECT_TRUE(u.GetSchemeStringLength() == 0);
EXPECT_TRUE(u.GetAuthStringLength() == 0);
EXPECT_TRUE(StrCmp(u.GetPathString(), "/path/sub") == 0);
EXPECT_TRUE(StrCmp(u.GetBaseString(), "/path/sub") == 0);
EXPECT_TRUE(u.GetQueryStringLength() == 0);
EXPECT_TRUE(u.GetFragStringLength() == 0);

// absolute path gets normalized
u = UriType("/path/../sub/");
EXPECT_TRUE(u.GetSchemeStringLength() == 0);
EXPECT_TRUE(u.GetAuthStringLength() == 0);
EXPECT_TRUE(StrCmp(u.GetPathString(), "/sub/") == 0);
EXPECT_TRUE(StrCmp(u.GetBaseString(), "/sub/") == 0);
EXPECT_TRUE(u.GetQueryStringLength() == 0);
EXPECT_TRUE(u.GetFragStringLength() == 0);

// relative path does not
u = UriType("path/../sub");
EXPECT_TRUE(u.GetSchemeStringLength() == 0);
EXPECT_TRUE(u.GetAuthStringLength() == 0);
EXPECT_TRUE(StrCmp(u.GetPathString(), "path/../sub") == 0);
EXPECT_TRUE(StrCmp(u.GetBaseString(), "path/../sub") == 0);
EXPECT_TRUE(u.GetQueryStringLength() == 0);
EXPECT_TRUE(u.GetFragStringLength() == 0);

u = UriType("http://auth#frag/stuff");
EXPECT_TRUE(StrCmp(u.GetSchemeString(), "http:") == 0);
EXPECT_TRUE(StrCmp(u.GetAuthString(), "//auth") == 0);
EXPECT_TRUE(u.GetPathStringLength() == 0);
EXPECT_TRUE(StrCmp(u.GetBaseString(), "http://auth") == 0);
EXPECT_TRUE(u.GetQueryStringLength() == 0);
EXPECT_TRUE(StrCmp(u.GetFragString(), "#frag/stuff") == 0);
EXPECT_TRUE(StrCmp(u.GetString(), "http://auth#frag/stuff") == 0);

const Value::Ch c[] = { '#', 'f', 'r', 'a', 'g', '/', 's', 't', 'u', 'f', 'f', '\0'};
SizeType len = internal::StrLen<Value::Ch>(c);
u = UriType(c, len);
EXPECT_TRUE(StrCmp(u.GetString(), "#frag/stuff") == 0);
EXPECT_TRUE(u.GetStringLength() == len);
EXPECT_TRUE(StrCmp(u.GetBaseString(), "") == 0);
EXPECT_TRUE(u.GetBaseStringLength() == 0);
EXPECT_TRUE(StrCmp(u.GetFragString(), "#frag/stuff") == 0);
EXPECT_TRUE(u.GetFragStringLength() == len);

u = UriType(c);
EXPECT_TRUE(StrCmp(u.GetString(), "#frag/stuff") == 0);
EXPECT_TRUE(u.GetStringLength() == len);
EXPECT_TRUE(StrCmp(u.GetBaseString(), "") == 0);
EXPECT_TRUE(u.GetBaseStringLength() == 0);
EXPECT_TRUE(StrCmp(u.GetFragString(), "#frag/stuff") == 0);
EXPECT_TRUE(u.GetFragStringLength() == len);
}

TEST(Uri, Parse_UTF16) {
typedef GenericValue<UTF16<> > Value;
typedef GenericUri<Value, MemoryPoolAllocator<> > UriType;
MemoryPoolAllocator<CrtAllocator> allocator;
Value v;
Value w;

v.SetString(L"http://auth/path/xxx?query#frag", allocator);
UriType u = UriType(v, &allocator);
EXPECT_TRUE(StrCmp(u.GetSchemeString(), L"http:") == 0);
EXPECT_TRUE(StrCmp(u.GetAuthString(), L"//auth") == 0);
EXPECT_TRUE(StrCmp(u.GetPathString(), L"/path/xxx") == 0);
EXPECT_TRUE(StrCmp(u.GetBaseString(), L"http://auth/path/xxx?query") == 0);
EXPECT_TRUE(StrCmp(u.GetQueryString(), L"?query") == 0);
EXPECT_TRUE(StrCmp(u.GetFragString(), L"#frag") == 0);
u.Get(w, allocator);
EXPECT_TRUE(*w.GetString() == *v.GetString());

#if RAPIDJSON_HAS_STDSTRING
typedef std::basic_string<Value::Ch> String;
String str = L"http://auth/path/xxx?query#frag";
const UriType uri = UriType(str);
EXPECT_TRUE(UriType::GetScheme(uri) == L"http:");
EXPECT_TRUE(UriType::GetAuth(uri) == L"//auth");
EXPECT_TRUE(UriType::GetPath(uri) == L"/path/xxx");
EXPECT_TRUE(UriType::GetBase(uri) == L"http://auth/path/xxx?query");
EXPECT_TRUE(UriType::GetQuery(uri) == L"?query");
EXPECT_TRUE(UriType::GetFrag(uri) == L"#frag");
EXPECT_TRUE(UriType::Get(uri) == str);
#endif

v.SetString(L"urn:uuid:ee564b8a-7a87-4125-8c96-e9f123d6766f", allocator);
u = UriType(v);
EXPECT_TRUE(StrCmp(u.GetSchemeString(), L"urn:") == 0);
EXPECT_TRUE(u.GetAuthStringLength() == 0);
EXPECT_TRUE(StrCmp(u.GetPathString(), L"uuid:ee564b8a-7a87-4125-8c96-e9f123d6766f") == 0);
EXPECT_TRUE(StrCmp(u.GetBaseString(), L"urn:uuid:ee564b8a-7a87-4125-8c96-e9f123d6766f") == 0);
EXPECT_TRUE(u.GetQueryStringLength() == 0);
EXPECT_TRUE(u.GetFragStringLength() == 0);
u.Get(w, allocator);
EXPECT_TRUE(*w.GetString() == *v.GetString());

v.SetString(L"", allocator);
u = UriType(v);
EXPECT_TRUE(u.GetSchemeStringLength() == 0);
EXPECT_TRUE(u.GetAuthStringLength() == 0);
EXPECT_TRUE(u.GetPathStringLength() == 0);
EXPECT_TRUE(u.GetBaseStringLength() == 0);
EXPECT_TRUE(u.GetQueryStringLength() == 0);
EXPECT_TRUE(u.GetFragStringLength() == 0);

v.SetString(L"http://auth/", allocator);
u = UriType(v);
EXPECT_TRUE(StrCmp(u.GetSchemeString(), L"http:") == 0);
EXPECT_TRUE(StrCmp(u.GetAuthString(), L"//auth") == 0);
EXPECT_TRUE(StrCmp(u.GetPathString(), L"/") == 0);
EXPECT_TRUE(StrCmp(u.GetBaseString(), L"http://auth/") == 0);
EXPECT_TRUE(u.GetQueryStringLength() == 0);
EXPECT_TRUE(u.GetFragStringLength() == 0);

u = UriType(L"/path/sub");
EXPECT_TRUE(u.GetSchemeStringLength() == 0);
EXPECT_TRUE(u.GetAuthStringLength() == 0);
EXPECT_TRUE(StrCmp(u.GetPathString(), L"/path/sub") == 0);
EXPECT_TRUE(StrCmp(u.GetBaseString(), L"/path/sub") == 0);
EXPECT_TRUE(u.GetQueryStringLength() == 0);
EXPECT_TRUE(u.GetFragStringLength() == 0);

// absolute path gets normalized
u = UriType(L"/path/../sub/");
EXPECT_TRUE(u.GetSchemeStringLength() == 0);
EXPECT_TRUE(u.GetAuthStringLength() == 0);
EXPECT_TRUE(StrCmp(u.GetPathString(), L"/sub/") == 0);
EXPECT_TRUE(StrCmp(u.GetBaseString(), L"/sub/") == 0);
EXPECT_TRUE(u.GetQueryStringLength() == 0);
EXPECT_TRUE(u.GetFragStringLength() == 0);

// relative path does not
u = UriType(L"path/../sub");
EXPECT_TRUE(u.GetSchemeStringLength() == 0);
EXPECT_TRUE(u.GetAuthStringLength() == 0);
EXPECT_TRUE(StrCmp(u.GetPathString(), L"path/../sub") == 0);
EXPECT_TRUE(StrCmp(u.GetBaseString(), L"path/../sub") == 0);
EXPECT_TRUE(u.GetQueryStringLength() == 0);
EXPECT_TRUE(u.GetFragStringLength() == 0);

u = UriType(L"http://auth#frag/stuff");
EXPECT_TRUE(StrCmp(u.GetSchemeString(), L"http:") == 0);
EXPECT_TRUE(StrCmp(u.GetAuthString(), L"//auth") == 0);
EXPECT_TRUE(u.GetPathStringLength() == 0);
EXPECT_TRUE(StrCmp(u.GetBaseString(), L"http://auth") == 0);
EXPECT_TRUE(u.GetQueryStringLength() == 0);
EXPECT_TRUE(StrCmp(u.GetFragString(), L"#frag/stuff") == 0);
EXPECT_TRUE(StrCmp(u.GetString(), L"http://auth#frag/stuff") == 0);

const Value::Ch c[] = { '#', 'f', 'r', 'a', 'g', '/', 's', 't', 'u', 'f', 'f', '\0'};
SizeType len = internal::StrLen<Value::Ch>(c);
u = UriType(c, len);
EXPECT_TRUE(StrCmp(u.GetString(), L"#frag/stuff") == 0);
EXPECT_TRUE(u.GetStringLength() == len);
EXPECT_TRUE(StrCmp(u.GetBaseString(), L"") == 0);
EXPECT_TRUE(u.GetBaseStringLength() == 0);
EXPECT_TRUE(StrCmp(u.GetFragString(), L"#frag/stuff") == 0);
EXPECT_TRUE(u.GetFragStringLength() == len);

u = UriType(c);
EXPECT_TRUE(StrCmp(u.GetString(), L"#frag/stuff") == 0);
EXPECT_TRUE(u.GetStringLength() == len);
EXPECT_TRUE(StrCmp(u.GetBaseString(), L"") == 0);
EXPECT_TRUE(u.GetBaseStringLength() == 0);
EXPECT_TRUE(StrCmp(u.GetFragString(), L"#frag/stuff") == 0);
EXPECT_TRUE(u.GetFragStringLength() == len);
}

TEST(Uri, Resolve) {
typedef GenericUri<Value> UriType;
CrtAllocator allocator;

// ref is full uri
UriType base = UriType("http://auth/path/#frag");
UriType ref = UriType("http://newauth/newpath#newfrag");
UriType res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://newauth/newpath#newfrag") == 0);

base = UriType("/path/#frag", &allocator);
ref = UriType("http://newauth/newpath#newfrag", &allocator);
res = ref.Resolve(base, &allocator);
EXPECT_TRUE(StrCmp(res.GetString(), "http://newauth/newpath#newfrag") == 0);

// ref is alternate uri
base = UriType("http://auth/path/#frag");
ref = UriType("urn:uuid:ee564b8a-7a87-4125-8c96-e9f123d6766f");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "urn:uuid:ee564b8a-7a87-4125-8c96-e9f123d6766f") == 0);

// ref is absolute path
base = UriType("http://auth/path/#");
ref = UriType("/newpath#newfrag");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://auth/newpath#newfrag") == 0);

// ref is relative path
base = UriType("http://auth/path/file.json#frag");
ref = UriType("newfile.json#");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://auth/path/newfile.json#") == 0);

base = UriType("http://auth/path/file.json#frag/stuff");
ref = UriType("newfile.json#newfrag/newstuff");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://auth/path/newfile.json#newfrag/newstuff") == 0);

base = UriType("file.json", &allocator);
ref = UriType("newfile.json", &base.GetAllocator());
res = ref.Resolve(base, &ref.GetAllocator());
EXPECT_TRUE(StrCmp(res.GetString(), "newfile.json") == 0);

base = UriType("file.json", &allocator);
ref = UriType("./newfile.json", &allocator);
res = ref.Resolve(base, &allocator);
EXPECT_TRUE(StrCmp(res.GetString(), "newfile.json") == 0);

base = UriType("file.json");
ref = UriType("parent/../newfile.json");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "newfile.json") == 0);

base = UriType("file.json");
ref = UriType("parent/./newfile.json");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "parent/newfile.json") == 0);

base = UriType("file.json");
ref = UriType("../../parent/.././newfile.json");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "newfile.json") == 0);

// This adds a joining slash so resolved length is base length + ref length + 1
base = UriType("http://auth");
ref = UriType("newfile.json");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://auth/newfile.json") == 0);

// ref is fragment
base = UriType("#frag/stuff");
ref = UriType("#newfrag/newstuff");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "#newfrag/newstuff") == 0);

// test ref fragment always wins
base = UriType("/path#frag");
ref = UriType("");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "/path") == 0);

// Examples from RFC3896
base = UriType("http://a/b/c/d;p?q");
ref = UriType("g:h");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "g:h") == 0);
ref = UriType("g");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://a/b/c/g") == 0);
ref = UriType("./g");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://a/b/c/g") == 0);
ref = UriType("g/");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://a/b/c/g/") == 0);
ref = UriType("/g");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://a/g") == 0);
ref = UriType("//g");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://g") == 0);
ref = UriType("?y");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://a/b/c/d;p?y") == 0);
ref = UriType("g?y");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://a/b/c/g?y") == 0);
ref = UriType("#s");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://a/b/c/d;p?q#s") == 0);
ref = UriType("g#s");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://a/b/c/g#s") == 0);
ref = UriType("g?y#s");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://a/b/c/g?y#s") == 0);
ref = UriType(";x");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://a/b/c/;x") == 0);
ref = UriType("g;x");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://a/b/c/g;x") == 0);
ref = UriType("g;x?y#s");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://a/b/c/g;x?y#s") == 0);
ref = UriType("");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://a/b/c/d;p?q") == 0);
ref = UriType(".");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://a/b/c/") == 0);
ref = UriType("./");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://a/b/c/") == 0);
ref = UriType("..");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://a/b/") == 0);
ref = UriType("../");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://a/b/") == 0);
ref = UriType("../g");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://a/b/g") == 0);
ref = UriType("../..");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://a/") == 0);
ref = UriType("../../");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://a/") == 0);
ref = UriType("../../g");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://a/g") == 0);
ref = UriType("../../../g");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://a/g") == 0);
ref = UriType("../../../../g");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://a/g") == 0);
ref = UriType("/./g");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://a/g") == 0);
ref = UriType("/../g");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://a/g") == 0);
ref = UriType("g.");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://a/b/c/g.") == 0);
ref = UriType(".g");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://a/b/c/.g") == 0);
ref = UriType("g..");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://a/b/c/g..") == 0);
ref = UriType("..g");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://a/b/c/..g") == 0);
ref = UriType("g#s/../x");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), "http://a/b/c/g#s/../x") == 0);
}

TEST(Uri, Resolve_UTF16) {
typedef GenericValue<UTF16<> > Value;
typedef GenericUri<Value> UriType;
CrtAllocator allocator;

// ref is full uri
UriType base = UriType(L"http://auth/path/#frag");
UriType ref = UriType(L"http://newauth/newpath#newfrag");
UriType res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://newauth/newpath#newfrag") == 0);

base = UriType(L"/path/#frag");
ref = UriType(L"http://newauth/newpath#newfrag");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://newauth/newpath#newfrag") == 0);

// ref is alternate uri
base = UriType(L"http://auth/path/#frag");
ref = UriType(L"urn:uuid:ee564b8a-7a87-4125-8c96-e9f123d6766f");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"urn:uuid:ee564b8a-7a87-4125-8c96-e9f123d6766f") == 0);

// ref is absolute path
base = UriType(L"http://auth/path/#");
ref = UriType(L"/newpath#newfrag");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://auth/newpath#newfrag") == 0);

// ref is relative path
base = UriType(L"http://auth/path/file.json#frag");
ref = UriType(L"newfile.json#");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://auth/path/newfile.json#") == 0);

base = UriType(L"http://auth/path/file.json#frag/stuff");
ref = UriType(L"newfile.json#newfrag/newstuff");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://auth/path/newfile.json#newfrag/newstuff") == 0);

base = UriType(L"file.json", &allocator);
ref = UriType(L"newfile.json", &base.GetAllocator());
res = ref.Resolve(base, &ref.GetAllocator());
EXPECT_TRUE(StrCmp(res.GetString(), L"newfile.json") == 0);

base = UriType(L"file.json", &allocator);
ref = UriType(L"./newfile.json", &allocator);
res = ref.Resolve(base, &allocator);
EXPECT_TRUE(StrCmp(res.GetString(), L"newfile.json") == 0);

base = UriType(L"file.json");
ref = UriType(L"parent/../newfile.json");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"newfile.json") == 0);

base = UriType(L"file.json");
ref = UriType(L"parent/./newfile.json");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"parent/newfile.json") == 0);

base = UriType(L"file.json");
ref = UriType(L"../../parent/.././newfile.json");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"newfile.json") == 0);

// This adds a joining slash so resolved length is base length + ref length + 1
base = UriType(L"http://auth");
ref = UriType(L"newfile.json");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://auth/newfile.json") == 0);

// ref is fragment
base = UriType(L"#frag/stuff");
ref = UriType(L"#newfrag/newstuff");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"#newfrag/newstuff") == 0);

// test ref fragment always wins
base = UriType(L"/path#frag");
ref = UriType(L"");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"/path") == 0);

// Examples from RFC3896
base = UriType(L"http://a/b/c/d;p?q");
ref = UriType(L"g:h");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"g:h") == 0);
ref = UriType(L"g");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://a/b/c/g") == 0);
ref = UriType(L"./g");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://a/b/c/g") == 0);
ref = UriType(L"g/");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://a/b/c/g/") == 0);
ref = UriType(L"/g");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://a/g") == 0);
ref = UriType(L"//g");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://g") == 0);
ref = UriType(L"?y");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://a/b/c/d;p?y") == 0);
ref = UriType(L"g?y");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://a/b/c/g?y") == 0);
ref = UriType(L"#s");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://a/b/c/d;p?q#s") == 0);
ref = UriType(L"g#s");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://a/b/c/g#s") == 0);
ref = UriType(L"g?y#s");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://a/b/c/g?y#s") == 0);
ref = UriType(L";x");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://a/b/c/;x") == 0);
ref = UriType(L"g;x");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://a/b/c/g;x") == 0);
ref = UriType(L"g;x?y#s");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://a/b/c/g;x?y#s") == 0);
ref = UriType(L"");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://a/b/c/d;p?q") == 0);
ref = UriType(L".");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://a/b/c/") == 0);
ref = UriType(L"./");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://a/b/c/") == 0);
ref = UriType(L"..");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://a/b/") == 0);
ref = UriType(L"../");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://a/b/") == 0);
ref = UriType(L"../g");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://a/b/g") == 0);
ref = UriType(L"../..");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://a/") == 0);
ref = UriType(L"../../");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://a/") == 0);
ref = UriType(L"../../g");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://a/g") == 0);
ref = UriType(L"../../../g");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://a/g") == 0);
ref = UriType(L"../../../../g");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://a/g") == 0);
ref = UriType(L"/./g");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://a/g") == 0);
ref = UriType(L"/../g");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://a/g") == 0);
ref = UriType(L"g.");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://a/b/c/g.") == 0);
ref = UriType(L".g");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://a/b/c/.g") == 0);
ref = UriType(L"g..");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://a/b/c/g..") == 0);
ref = UriType(L"..g");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://a/b/c/..g") == 0);
ref = UriType(L"g#s/../x");
res = ref.Resolve(base);
EXPECT_TRUE(StrCmp(res.GetString(), L"http://a/b/c/g#s/../x") == 0);
}

#if defined(_MSC_VER) || defined(__clang__)
RAPIDJSON_DIAG_POP
#endif
