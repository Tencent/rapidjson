#include "unittest.h"

#include "rapidjson/document.h"
#include "rapidjson/reader.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;

TEST(Writer, Compact) {
	StringStream s("{ \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3] } ");
	StringBuffer buffer;
	Writer<StringBuffer> writer(buffer);
	Reader reader;
	reader.Parse<0>(s, writer);
	EXPECT_STREQ("{\"hello\":\"world\",\"t\":true,\"f\":false,\"n\":null,\"i\":123,\"pi\":3.1416,\"a\":[1,2,3]}", buffer.GetString());
	EXPECT_EQ(77u, buffer.GetSize());
	EXPECT_TRUE(writer.IsComplete());
}

// json -> parse -> writer -> json
#define TEST_ROUNDTRIP(json) \
	{ \
		StringStream s(json); \
		StringBuffer buffer; \
		Writer<StringBuffer> writer(buffer); \
		Reader reader; \
		reader.Parse<0>(s, writer); \
		EXPECT_STREQ(json, buffer.GetString()); \
		EXPECT_TRUE(writer.IsComplete()); \
	}

TEST(Writer, Int) {
	TEST_ROUNDTRIP("[-1]");
	TEST_ROUNDTRIP("[-123]");
	TEST_ROUNDTRIP("[-2147483648]");
}

TEST(Writer, UInt) {
	TEST_ROUNDTRIP("[0]");
	TEST_ROUNDTRIP("[1]");
	TEST_ROUNDTRIP("[123]");
	TEST_ROUNDTRIP("[2147483647]");
	TEST_ROUNDTRIP("[4294967295]");
}

TEST(Writer, Int64) {
	TEST_ROUNDTRIP("[-1234567890123456789]");
	TEST_ROUNDTRIP("[-9223372036854775808]");
}

TEST(Writer, Uint64) {
	TEST_ROUNDTRIP("[1234567890123456789]");
	TEST_ROUNDTRIP("[9223372036854775807]");
}

TEST(Writer, String) {
	TEST_ROUNDTRIP("[\"Hello\"]");
	TEST_ROUNDTRIP("[\"Hello\\u0000World\"]");
	TEST_ROUNDTRIP("[\"\\\"\\\\/\\b\\f\\n\\r\\t\"]");
}

TEST(Writer,DoublePrecision) {
	const char json[] = "[1.2345,1.2345678,0.123456789012,1234567.8]";

	StringBuffer buffer;
	Writer<StringBuffer> writer(buffer);

	const int kDefaultDoublePrecision = 6;
	// handling the double precision
	EXPECT_EQ(writer.GetDoublePrecision(), kDefaultDoublePrecision);
	writer.SetDoublePrecision(17);
	EXPECT_EQ(writer.GetDoublePrecision(), 17);
	writer.SetDoublePrecision(-1); // negative equivalent to reset
	EXPECT_EQ(writer.GetDoublePrecision(), kDefaultDoublePrecision);
	writer.SetDoublePrecision(1);
	writer.SetDoublePrecision();   // reset again
	EXPECT_EQ(writer.GetDoublePrecision(), kDefaultDoublePrecision);

	{ // write with explicitly increased precision
		StringStream s(json);
		Reader reader;
		reader.Parse<0>(s, writer.SetDoublePrecision(12));
		EXPECT_EQ(writer.GetDoublePrecision(), 12);
		EXPECT_STREQ(json, buffer.GetString());
	}
	{ // explicit individual double precisions
		buffer.Clear();
		writer.Reset(buffer);
		writer.SetDoublePrecision(2);
		writer.StartArray();
		writer.Double(1.2345, 5);
		writer.Double(1.2345678, 9);
		writer.Double(0.123456789012, 12);
		writer.Double(1234567.8, 8);
		writer.EndArray();

		EXPECT_EQ(writer.GetDoublePrecision(), 2);
		EXPECT_STREQ(json, buffer.GetString());
	}
	{ // write with default precision (output with precision loss)
		Document d;
		d.Parse<0>(json);
		buffer.Clear();
		writer.Reset(buffer);
		d.Accept(writer.SetDoublePrecision());

		// parsed again to avoid platform-dependent floating point outputs
		// (e.g. width of exponents)
		d.Parse<0>(buffer.GetString());
		EXPECT_EQ(writer.GetDoublePrecision(), kDefaultDoublePrecision);
		EXPECT_DOUBLE_EQ(d[0u].GetDouble(), 1.2345);
		EXPECT_DOUBLE_EQ(d[1u].GetDouble(), 1.23457);
		EXPECT_DOUBLE_EQ(d[2u].GetDouble(), 0.123457);
		EXPECT_DOUBLE_EQ(d[3u].GetDouble(), 1234570);
	}
}

TEST(Writer, Transcode) {
	const char json[] = "{\"hello\":\"world\",\"t\":true,\"f\":false,\"n\":null,\"i\":123,\"pi\":3.1416,\"a\":[1,2,3],\"dollar\":\"\x24\",\"cents\":\"\xC2\xA2\",\"euro\":\"\xE2\x82\xAC\",\"gclef\":\"\xF0\x9D\x84\x9E\"}";

	// UTF8 -> UTF16 -> UTF8
	{
		StringStream s(json);
		StringBuffer buffer;
		Writer<StringBuffer, UTF16<>, UTF8<> > writer(buffer);
		GenericReader<UTF8<>, UTF16<> > reader;
		reader.Parse(s, writer);
		EXPECT_STREQ(json, buffer.GetString());
	}

	// UTF8 -> UTF8 -> ASCII -> UTF8 -> UTF8
	{
		StringStream s(json);
		StringBuffer buffer;
		Writer<StringBuffer, UTF8<>, ASCII<> > writer(buffer);
		Reader reader;
		reader.Parse(s, writer);

		StringBuffer buffer2;
		Writer<StringBuffer> writer2(buffer2);
		GenericReader<ASCII<>, UTF8<> > reader2;
		StringStream s2(buffer.GetString());
		reader2.Parse(s2, writer2);

		EXPECT_STREQ(json, buffer2.GetString());
	}
}

#include <sstream>

class OStreamWrapper {
public:
	typedef char Ch;

	OStreamWrapper(std::ostream& os) : os_(os) {}

	Ch Peek() const { assert(false); return '\0'; }
	Ch Take() { assert(false); return '\0'; }
	size_t Tell() const { return 0; }

	Ch* PutBegin() { assert(false); return 0; }
	void Put(Ch c) { os_.put(c); }
	void Flush() { os_.flush(); }
	size_t PutEnd(Ch*) { assert(false); return 0; }

private:
	OStreamWrapper(const OStreamWrapper&);
	OStreamWrapper& operator=(const OStreamWrapper&);

	std::ostream& os_;
};

TEST(Writer, OStreamWrapper) {
	StringStream s("{ \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3] } ");
	
	std::stringstream ss;
	OStreamWrapper os(ss);
	
	Writer<OStreamWrapper> writer(os);

	Reader reader;
	reader.Parse<0>(s, writer);
	
	std::string actual = ss.str();
	EXPECT_STREQ("{\"hello\":\"world\",\"t\":true,\"f\":false,\"n\":null,\"i\":123,\"pi\":3.1416,\"a\":[1,2,3]}", actual.c_str());
}

TEST(Writer, AssertRootMustBeArrayOrObject) {
#define T(x)\
	{\
		StringBuffer buffer;\
		Writer<StringBuffer> writer(buffer);\
		ASSERT_THROW(x, AssertException);\
	}
	T(writer.Bool(false));
	T(writer.Bool(true));
	T(writer.Null());
	T(writer.Int(0));
	T(writer.Uint(0));
	T(writer.Int64(0));
	T(writer.Uint64(0));
	T(writer.Double(0));
	T(writer.String("foo"));
#undef T
}

TEST(Writer, AssertIncorrectObjectLevel) {
	StringBuffer buffer;
	Writer<StringBuffer> writer(buffer);
	writer.StartObject();
	writer.EndObject();
	ASSERT_THROW(writer.EndObject(), AssertException);
}

TEST(Writer, AssertIncorrectArrayLevel) {
	StringBuffer buffer;
	Writer<StringBuffer> writer(buffer);
	writer.StartArray();
	writer.EndArray();
	ASSERT_THROW(writer.EndArray(), AssertException);
}

TEST(Writer, AssertIncorrectEndObject) {
	StringBuffer buffer;
	Writer<StringBuffer> writer(buffer);
	writer.StartObject();
	ASSERT_THROW(writer.EndArray(), AssertException);
}

TEST(Writer, AssertIncorrectEndArray) {
	StringBuffer buffer;
	Writer<StringBuffer> writer(buffer);
	writer.StartObject();
	ASSERT_THROW(writer.EndArray(), AssertException);
}

TEST(Writer, AssertObjectKeyNotString) {
#define T(x)\
	{\
		StringBuffer buffer;\
		Writer<StringBuffer> writer(buffer);\
		writer.StartObject();\
		ASSERT_THROW(x, AssertException); \
	}
	T(writer.Bool(false));
	T(writer.Bool(true));
	T(writer.Null());
	T(writer.Int(0));
	T(writer.Uint(0));
	T(writer.Int64(0));
	T(writer.Uint64(0));
	T(writer.Double(0));
	T(writer.StartObject());
	T(writer.StartArray());
#undef T
}

TEST(Writer, AssertMultipleRoot) {
	StringBuffer buffer;
	Writer<StringBuffer> writer(buffer);
	writer.StartObject();
	writer.EndObject();
	ASSERT_THROW(writer.StartObject(), AssertException);
}

TEST(Writer, RootObjectIsComplete) {
	StringBuffer buffer;
	Writer<StringBuffer> writer(buffer);
	EXPECT_FALSE(writer.IsComplete());
	writer.StartObject();
	EXPECT_FALSE(writer.IsComplete());
	writer.String("foo");
	EXPECT_FALSE(writer.IsComplete());
	writer.Int(1);
	EXPECT_FALSE(writer.IsComplete());
	writer.EndObject();
	EXPECT_TRUE(writer.IsComplete());
}

TEST(Writer, RootArrayIsComplete) {
	StringBuffer buffer;
	Writer<StringBuffer> writer(buffer);
	EXPECT_FALSE(writer.IsComplete());
	writer.StartArray();
	EXPECT_FALSE(writer.IsComplete());
	writer.String("foo");
	EXPECT_FALSE(writer.IsComplete());
	writer.Int(1);
	EXPECT_FALSE(writer.IsComplete());
	writer.EndArray();
	EXPECT_TRUE(writer.IsComplete());
}
