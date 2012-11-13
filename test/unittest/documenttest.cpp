#include "unittest.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include <sstream>

using namespace rapidjson;

TEST(Document, Parse) {
	Document doc;

	doc.Parse<0>(" { \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3, 4] } ");

	EXPECT_TRUE(doc.IsObject());

	EXPECT_TRUE(doc.HasMember("hello"));
	Value& hello = doc["hello"];
	EXPECT_TRUE(hello.IsString());
	EXPECT_STREQ("world", hello.GetString());

	EXPECT_TRUE(doc.HasMember("t"));
	Value& t = doc["t"];
	EXPECT_TRUE(t.IsTrue());

	EXPECT_TRUE(doc.HasMember("f"));
	Value& f = doc["f"];
	EXPECT_TRUE(f.IsFalse());

	EXPECT_TRUE(doc.HasMember("n"));
	Value& n = doc["n"];
	EXPECT_TRUE(n.IsNull());

	EXPECT_TRUE(doc.HasMember("i"));
	Value& i = doc["i"];
	EXPECT_TRUE(i.IsNumber());
	EXPECT_EQ(123, i.GetInt());

	EXPECT_TRUE(doc.HasMember("pi"));
	Value& pi = doc["pi"];
	EXPECT_TRUE(pi.IsNumber());
	EXPECT_EQ(3.1416, pi.GetDouble());

	EXPECT_TRUE(doc.HasMember("a"));
	Value& a = doc["a"];
	EXPECT_TRUE(a.IsArray());
	EXPECT_EQ(4u, a.Size());
	for (SizeType i = 0; i < 4; i++)
		EXPECT_EQ(i + 1, a[i].GetUint());
}

// This should be slow due to assignment in inner-loop.
struct OutputStringStream : public std::ostringstream {
	typedef char Ch;

	void Put(char c) {
		put(c);
	}
	void Flush() {}
};

TEST(Document, AcceptWriter) {
	Document doc;
	doc.Parse<0>(" { \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3, 4] } ");

	OutputStringStream os;
	Writer<OutputStringStream> writer(os);
	doc.Accept(writer);

	EXPECT_EQ("{\"hello\":\"world\",\"t\":true,\"f\":false,\"n\":null,\"i\":123,\"pi\":3.1416,\"a\":[1,2,3,4]}", os.str());
}

// Issue 44:	SetStringRaw doesn't work with wchar_t
TEST(Document, UTF16_Document) {
	GenericDocument< UTF16<> > json;
	json.Parse<kParseValidateEncodingFlag>(L"[{\"created_at\":\"Wed Oct 30 17:13:20 +0000 2012\"}]");

	ASSERT_TRUE(json.IsArray());
	GenericValue< UTF16<> >& v = json[0u];
	ASSERT_TRUE(v.IsObject());

	GenericValue< UTF16<> >& s = v[L"created_at"];
	ASSERT_TRUE(s.IsString());

	EXPECT_EQ(0, wcscmp(L"Wed Oct 30 17:13:20 +0000 2012", s.GetString()));
}
