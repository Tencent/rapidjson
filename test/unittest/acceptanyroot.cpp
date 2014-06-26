#ifdef RAPIDJSON_ACCEPT_ANY_ROOT

#include "unittest.h"
#include "rapidjson/document.h"
#include "rapidjson/reader.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;

static struct json_tuple {
	const char* json;
	Type        type;
} const json_data[] = {
	{ "null", kNullType },
	{ "true", kTrueType },
	{ "false", kFalseType },
	{ "\"\"", kStringType },
//	{ "0", kNumberType }, // issue #57
	{ "[]", kArrayType },
	{ "{}", kObjectType },
	{ NULL, (Type)0 }
};

TEST(Reader,AcceptAnyRoot)
{
	BaseReaderHandler<> handler;
	Reader reader;
	for (json_tuple const* t = json_data; t->json; ++t) {
		reader.AcceptAnyRoot(false);
		{
			StringStream s(t->json);
			reader.Parse<0>(s,handler);
			EXPECT_EQ(!reader.HasParseError(),
					(t->type == kArrayType || t->type == kObjectType));
		}
		reader.AcceptAnyRoot(true);
		{
			StringStream s(t->json);
			reader.Parse<0>(s,handler);
			EXPECT_TRUE(!reader.HasParseError());
		}
	}
}

TEST(Writer,AcceptAnyRoot)
{
	StringBuffer buffer;
	Writer<StringBuffer> writer(buffer);
	for (json_tuple const* t = json_data; t->json; ++t) {
		Value v(t->type);
		writer.AcceptAnyRoot(!(t->type == kArrayType || t->type == kObjectType));
		v.Accept(writer);
		EXPECT_STREQ(t->json, buffer.GetString());
		buffer.Clear();
	}
}

TEST(Document,AcceptAnyRoot)
{
	Document d;
	for (json_tuple const* t = json_data; t->json; ++t) {
		d.AcceptAnyRoot(false).Parse<0>(t->json);
		EXPECT_EQ(!d.HasParseError(),
			(t->type == kArrayType || t->type == kObjectType));

		d.AcceptAnyRoot().Parse<0>(t->json);
		EXPECT_TRUE(!d.HasParseError());
		EXPECT_EQ(t->type, d.GetType());
	}
}

#endif // RAPIDJSON_ACCEPT_ANY_ROOT
