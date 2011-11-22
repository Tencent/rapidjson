#include "unittest.h"

#define private public	// For testing private members
#include "rapidjson/reader.h"

using namespace rapidjson;

template<bool expect>
struct ParseBoolHandler : BaseReaderHandler<> {
	ParseBoolHandler() : step_(0) {}
	void Default() { FAIL(); }
	void Bool(bool b) { EXPECT_EQ(expect, b); ++step_; }

	unsigned step_;
};

TEST(Reader, ParseTrue) {
	StringStream s("true");
	ParseBoolHandler<true> h;
	Reader reader;
	reader.ParseTrue<0>(s, h);
	EXPECT_EQ(1, h.step_);
}

TEST(Reader, ParseFalse) {
	StringStream s("false");
	ParseBoolHandler<false> h;
	Reader reader;
	reader.ParseFalse<0>(s, h);
	EXPECT_EQ(1, h.step_);
}

struct ParseIntHandler : BaseReaderHandler<> {
	ParseIntHandler() : step_(0) {}
	void Default() { FAIL(); }
	void Int(int i) { actual_ = i; step_++; }

	unsigned step_;
	int actual_;
};

struct ParseUintHandler : BaseReaderHandler<> {
	ParseUintHandler() : step_(0) {}
	void Default() { FAIL(); }
	void Uint(unsigned i) { actual_ = i; step_++; }

	unsigned step_;
	unsigned actual_;
};

struct ParseInt64Handler : BaseReaderHandler<> {
	ParseInt64Handler() : step_(0) {}
	void Default() { FAIL(); }
	void Int64(int64_t i) { actual_ = i; step_++; }

	unsigned step_;
	int64_t actual_;
};

struct ParseUint64Handler : BaseReaderHandler<> {
	ParseUint64Handler() : step_(0) {}
	void Default() { FAIL(); }
	void Uint64(uint64_t i) { actual_ = i; step_++; }

	unsigned step_;
	uint64_t actual_;
};

struct ParseDoubleHandler : BaseReaderHandler<> {
	ParseDoubleHandler() : step_(0) {}
	void Default() { FAIL(); }
	void Double(double d) { actual_ = d; step_++; }

	unsigned step_;
	double actual_;
};

TEST(Reader, ParseNumberHandler) {
#define TEST_NUMBER(Handler, str, x) \
	{ \
		StringStream s(str); \
		Handler h; \
		Reader reader; \
		reader.ParseNumber<0>(s, h); \
		EXPECT_EQ(1, h.step_); \
		EXPECT_EQ(x, h.actual_); \
	}

#define TEST_DOUBLE(str, x) \
	{ \
		StringStream s(str); \
		ParseDoubleHandler h; \
		Reader reader; \
		reader.ParseNumber<0>(s, h); \
		EXPECT_EQ(1, h.step_); \
		EXPECT_DOUBLE_EQ(x, h.actual_); \
	}

	TEST_NUMBER(ParseUintHandler, "0", 0);
	TEST_NUMBER(ParseUintHandler, "123", 123);
	TEST_NUMBER(ParseUintHandler, "2147483648", 2147483648u);		// 2^31 - 1 (cannot be stored in int)
	TEST_NUMBER(ParseUintHandler, "4294967295", 4294967295u);

	TEST_NUMBER(ParseIntHandler, "-123", -123);
	TEST_NUMBER(ParseIntHandler, "-2147483648", -2147483648LL);		// -2^31 (min of int)

	TEST_NUMBER(ParseUint64Handler, "4294967296", 4294967296ULL);	// 2^32 (max of unsigned + 1, force to use uint64_t)
	TEST_NUMBER(ParseUint64Handler, "18446744073709551615", 18446744073709551615ULL);	// 2^64 - 1 (max of uint64_t)

	TEST_NUMBER(ParseInt64Handler, "-2147483649", -2147483649LL);	// -2^31 -1 (min of int - 1, force to use int64_t)
	TEST_NUMBER(ParseInt64Handler, "-9223372036854775808", (-9223372036854775807LL - 1));		// -2^63 (min of int64_t)

	TEST_DOUBLE("0.0", 0.0);
	TEST_DOUBLE("1.0", 1.0);
	TEST_DOUBLE("-1.0", -1.0);
	TEST_DOUBLE("1.5", 1.5);
	TEST_DOUBLE("-1.5", -1.5);
	TEST_DOUBLE("3.1416", 3.1416);
	TEST_DOUBLE("1E10", 1E10);
	TEST_DOUBLE("1e10", 1e10);
	TEST_DOUBLE("1E+10", 1E+10);
	TEST_DOUBLE("1E-10", 1E-10);
	TEST_DOUBLE("-1E10", -1E10);
	TEST_DOUBLE("-1e10", -1e10);
	TEST_DOUBLE("-1E+10", -1E+10);
	TEST_DOUBLE("-1E-10", -1E-10);
	TEST_DOUBLE("1.234E+10", 1.234E+10);
	TEST_DOUBLE("1.234E-10", 1.234E-10);
	TEST_DOUBLE("1.79769e+308", 1.79769e+308);
	//TEST_DOUBLE("2.22507e-308", 2.22507e-308);	// TODO: underflow
	TEST_DOUBLE("-1.79769e+308", -1.79769e+308);
	//TEST_DOUBLE("-2.22507e-308", -2.22507e-308);	// TODO: underflow
	TEST_DOUBLE("18446744073709551616", 18446744073709551616.0);	// 2^64 (max of uint64_t + 1, force to use double)
	TEST_DOUBLE("-9223372036854775809", -9223372036854775809.0);	// -2^63 - 1(min of int64_t + 1, force to use double)

	{
		char n1e308[310];	// '1' followed by 308 '0'
		n1e308[0] = '1';
		for (int i = 1; i < 309; i++)
			n1e308[i] = '0';
		n1e308[309] = '\0';
		TEST_DOUBLE(n1e308, 1E308);
	}
#undef TEST_NUMBER
#undef TEST_DOUBLE
}

TEST(Reader, ParseNumberHandler_Error) {
#define TEST_NUMBER_ERROR(str) \
	{ \
		char buffer[1001]; \
		sprintf(buffer, "[%s]", str); \
		InsituStringStream s(buffer); \
		BaseReaderHandler<> h; \
		Reader reader; \
		EXPECT_FALSE(reader.Parse<0>(s, h)); \
	}

	TEST_NUMBER_ERROR("a");		// At least one digit in integer part
	TEST_NUMBER_ERROR(".1");	// At least one digit in integer part
	
	{
		char n1e309[311];	// '1' followed by 309 '0'
		n1e309[0] = '1';
		for (int i = 1; i < 310; i++)
			n1e309[i] = '0';
		n1e309[310] = '\0';
		TEST_NUMBER_ERROR(n1e309);	// Number too big to store in double
	}

	TEST_NUMBER_ERROR("1.");	// At least one digit in fraction part
	TEST_NUMBER_ERROR("1e309"); // Number too big to store in double
	TEST_NUMBER_ERROR("1e_");	// At least one digit in exponent

#undef TEST_NUMBER_ERROR
}

template <typename Encoding>
struct ParseStringHandler : BaseReaderHandler<Encoding> {
	ParseStringHandler() : str_(0), length_(0) {}
	~ParseStringHandler() { EXPECT_TRUE(str_ != 0); if (copy_) free(const_cast<typename Encoding::Ch*>(str_)); }
	void Default() { FAIL(); }
	void String(const typename Encoding::Ch* str, size_t length, bool copy) { 
		EXPECT_EQ(0, str_);
		if (copy) {
			str_ = (typename Encoding::Ch*)malloc((length + 1) * sizeof(typename Encoding::Ch));
			memcpy((void*)str_, str, (length + 1) * sizeof(typename Encoding::Ch));
		}
		else
			str_ = str;
		length_ = length; 
		copy_ = copy; 
	}

	const typename Encoding::Ch* str_;
	size_t length_;
	bool copy_;
};

TEST(Reader, ParseString) {
#define TEST_STRING(Encoding, e, x) \
	{ \
		Encoding::Ch* buffer = StrDup(x); \
		GenericInsituStringStream<Encoding> is(buffer); \
		ParseStringHandler<Encoding> h; \
		GenericReader<Encoding> reader; \
		reader.ParseString<kParseInsituFlag | kParseValidateEncodingFlag>(is, h); \
		EXPECT_EQ(0, StrCmp<Encoding::Ch>(e, h.str_)); \
		EXPECT_EQ(StrLen(e), h.length_); \
		free(buffer); \
		GenericStringStream<Encoding> s(x); \
		ParseStringHandler<Encoding> h2; \
		GenericReader<Encoding> reader2; \
		reader2.ParseString<0>(s, h2); \
		EXPECT_EQ(0, StrCmp<Encoding::Ch>(e, h2.str_)); \
		EXPECT_EQ(StrLen(e), h2.length_); \
	}

	// String constant L"\xXX" can only specify character code in bytes, which is not endianness-neutral. 
	// And old compiler does not support u"" and U"" string literal. So here specify string literal by array of Ch.
#define ARRAY(...) { __VA_ARGS__ }
#define TEST_STRINGARRAY(Encoding, array, x) \
	{ \
		static const Encoding::Ch e[] = array; \
		TEST_STRING(Encoding, e, x); \
	}

#define TEST_STRINGARRAY2(Encoding, earray, xarray) \
	{ \
		static const Encoding::Ch e[] = earray; \
		static const Encoding::Ch x[] = xarray; \
		TEST_STRING(Encoding, e, x); \
	}

	TEST_STRING(UTF8<>, "", "\"\"");
	TEST_STRING(UTF8<>, "Hello", "\"Hello\"");
	TEST_STRING(UTF8<>, "Hello\nWorld", "\"Hello\\nWorld\"");
	TEST_STRING(UTF8<>, "\"\\/\b\f\n\r\t", "\"\\\"\\\\/\\b\\f\\n\\r\\t\"");
	TEST_STRING(UTF8<>, "\x24", "\"\\u0024\"");			// Dollar sign U+0024
	TEST_STRING(UTF8<>, "\xC2\xA2", "\"\\u00A2\"");		// Cents sign U+00A2
	TEST_STRING(UTF8<>, "\xE2\x82\xAC", "\"\\u20AC\""); // Euro sign U+20AC
	TEST_STRING(UTF8<>, "\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");	// G clef sign U+1D11E

	// UTF16
	TEST_STRING(UTF16<>, L"", L"\"\"");
	TEST_STRING(UTF16<>, L"Hello", L"\"Hello\"");
	TEST_STRING(UTF16<>, L"Hello\nWorld", L"\"Hello\\nWorld\"");
	TEST_STRING(UTF16<>, L"\"\\/\b\f\n\r\t", L"\"\\\"\\\\/\\b\\f\\n\\r\\t\"");
	TEST_STRINGARRAY(UTF16<>, ARRAY(0x0024, 0x0000), L"\"\\u0024\"");
	TEST_STRINGARRAY(UTF16<>, ARRAY(0x00A2, 0x0000), L"\"\\u00A2\"");	// Cents sign U+00A2
	TEST_STRINGARRAY(UTF16<>, ARRAY(0x20AC, 0x0000), L"\"\\u20AC\"");	// Euro sign U+20AC
	TEST_STRINGARRAY(UTF16<>, ARRAY(0xD834, 0xDD1E, 0x0000), L"\"\\uD834\\uDD1E\"");	// G clef sign U+1D11E

	// UTF32
	TEST_STRINGARRAY2(UTF32<>, ARRAY('\0'), ARRAY('\"', '\"', '\0'));
	TEST_STRINGARRAY2(UTF32<>, ARRAY('H', 'e', 'l', 'l', 'o', '\0'), ARRAY('\"', 'H', 'e', 'l', 'l', 'o', '\"', '\0'));
	TEST_STRINGARRAY2(UTF32<>, ARRAY('H', 'e', 'l', 'l', 'o', '\n', 'W', 'o', 'r', 'l', 'd', '\0'), ARRAY('\"', 'H', 'e', 'l', 'l', 'o', '\\', 'n', 'W', 'o', 'r', 'l', 'd', '\"', '\0'));
	TEST_STRINGARRAY2(UTF32<>, ARRAY('\"', '\\', '/', '\b', '\f', '\n', '\r', '\t', '\0'), ARRAY('\"', '\\', '\"', '\\', '\\', '/', '\\', 'b', '\\', 'f', '\\', 'n', '\\', 'r', '\\', 't', '\"', '\0'));
	TEST_STRINGARRAY2(UTF32<>, ARRAY(0x00024, 0x0000), ARRAY('\"', '\\', 'u', '0', '0', '2', '4', '\"', '\0'));
	TEST_STRINGARRAY2(UTF32<>, ARRAY(0x000A2, 0x0000), ARRAY('\"', '\\', 'u', '0', '0', 'A', '2', '\"', '\0'));	// Cents sign U+00A2
	TEST_STRINGARRAY2(UTF32<>, ARRAY(0x020AC, 0x0000), ARRAY('\"', '\\', 'u', '2', '0', 'A', 'C', '\"', '\0'));	// Euro sign U+20AC
	TEST_STRINGARRAY2(UTF32<>, ARRAY(0x1D11E, 0x0000), ARRAY('\"', '\\', 'u', 'D', '8', '3', '4', '\\', 'u', 'D', 'D', '1', 'E', '\"', '\0'));	// G clef sign U+1D11E

#undef TEST_STRINGARRAY
#undef ARRAY
#undef TEST_STRING

	// Support of null character in string
	{
		StringStream s("\"Hello\\u0000World\"");
		const char e[] = "Hello\0World";
		ParseStringHandler<UTF8<> > h;
		Reader reader;
		reader.ParseString<0>(s, h);
		EXPECT_EQ(0, memcmp(e, h.str_, h.length_ + 1));
		EXPECT_EQ(11, h.length_);
	}
}

TEST(Reader, ParseString_NonDestructive) {
	StringStream s("\"Hello\\nWorld\"");
	ParseStringHandler<UTF8<> > h;
	Reader reader;
	reader.ParseString<0>(s, h);
	EXPECT_EQ(0, StrCmp("Hello\nWorld", h.str_));
	EXPECT_EQ(11, h.length_);
}

TEST(Reader, ParseString_Error) {
#define TEST_STRING_ERROR(str) \
	{ \
		char buffer[1001]; \
		strncpy(buffer, str, 1000); \
		InsituStringStream s(buffer); \
		BaseReaderHandler<> h; \
		Reader reader; \
		EXPECT_FALSE(reader.Parse<kParseValidateEncodingFlag>(s, h)); \
	}

#define ARRAY(...) { __VA_ARGS__ }
#define TEST_STRINGARRAY_ERROR(Encoding, array) \
	{ \
		static const Encoding::Ch e[] = array; \
		TEST_STRING_ERROR(e); \
	}

	TEST_STRING_ERROR("[\"\\a\"]");				// Unknown escape character
	TEST_STRING_ERROR("[\"\\uABCG\"]");			// Incorrect hex digit after \\u escape
	TEST_STRING_ERROR("[\"\\uD800X\"]");		// Missing the second \\u in surrogate pair
	TEST_STRING_ERROR("[\"\\uD800\\uFFFF\"]");	// The second \\u in surrogate pair is invalid
	TEST_STRING_ERROR("[\"Test]");				// lacks ending quotation before the end of string
	TEST_STRINGARRAY_ERROR(UTF8<>, ARRAY('[', 0x80u, ']'));			// Incorrect UTF8 sequence
	TEST_STRINGARRAY_ERROR(UTF8<>, ARRAY('[', 0xC0u, 0x40, ']'));	// Incorrect UTF8 sequence

#undef ARRAY
#undef TEST_STRINGARRAY_ERROR
#undef TEST_STRING_ERROR
}

template <unsigned count>
struct ParseArrayHandler : BaseReaderHandler<> {
	ParseArrayHandler() : step_(0) {}

	void Default() { FAIL(); }
	void Uint(unsigned i) { EXPECT_EQ(step_, i); step_++; } 
	void StartArray() { EXPECT_EQ(0, step_); step_++; }
	void EndArray(SizeType elementCount) { step_++; }

	unsigned step_;
};

TEST(Reader, ParseEmptyArray) {
	char *json = StrDup("[ ] ");
	InsituStringStream s(json);
	ParseArrayHandler<0> h;
	Reader reader;
	reader.ParseArray<0>(s, h);
	EXPECT_EQ(2, h.step_);
	free(json);
}

TEST(Reader, ParseArray) {
	char *json = StrDup("[1, 2, 3, 4]");
	InsituStringStream s(json);
	ParseArrayHandler<4> h;
	Reader reader;
	reader.ParseArray<0>(s, h);
	EXPECT_EQ(6, h.step_);
	free(json);
}

TEST(Reader, ParseArray_Error) {
#define TEST_ARRAY_ERROR(str) \
	{ \
		char buffer[1001]; \
		strncpy(buffer, str, 1000); \
		InsituStringStream s(buffer); \
		BaseReaderHandler<> h; \
		GenericReader<UTF8<>, CrtAllocator> reader; \
		EXPECT_FALSE(reader.Parse<0>(s, h)); \
	}

	// Must be a comma or ']' after an array element.
	TEST_ARRAY_ERROR("[");
	TEST_ARRAY_ERROR("[}");
	TEST_ARRAY_ERROR("[1 2]");

#undef TEST_ARRAY_ERROR
}

struct ParseObjectHandler : BaseReaderHandler<> {
	ParseObjectHandler() : step_(0) {}

	void Null() { EXPECT_EQ(8, step_); step_++; }
	void Bool(bool b) { 
		switch(step_) {
			case 4: EXPECT_TRUE(b); step_++; break;
			case 6: EXPECT_FALSE(b); step_++; break;
			default: FAIL();
		}
	}
	void Int(int i) { 
		switch(step_) {
			case 10: EXPECT_EQ(123, i); step_++; break;
			case 15: EXPECT_EQ(1, i); step_++; break;
			case 16: EXPECT_EQ(2, i); step_++; break;
			case 17: EXPECT_EQ(3, i); step_++; break;
			default: FAIL();
		}
	}
	void Uint(unsigned i) { Int(i); }
	void Double(double d) { EXPECT_EQ(12, step_); EXPECT_EQ(3.1416, d); step_++; }
	void String(const char* str, size_t length, bool copy) { 
		switch(step_) {
			case 1: EXPECT_STREQ("hello", str); step_++; break;
			case 2: EXPECT_STREQ("world", str); step_++; break;
			case 3: EXPECT_STREQ("t", str); step_++; break;
			case 5: EXPECT_STREQ("f", str); step_++; break;
			case 7: EXPECT_STREQ("n", str); step_++; break;
			case 9: EXPECT_STREQ("i", str); step_++; break;
			case 11: EXPECT_STREQ("pi", str); step_++; break;
			case 13: EXPECT_STREQ("a", str); step_++; break;
			default: FAIL();
		}
	}
	void StartObject() { EXPECT_EQ(0, step_); step_++; }
	void EndObject(SizeType memberCount) { EXPECT_EQ(19, step_); EXPECT_EQ(7, memberCount); step_++;}
	void StartArray() { EXPECT_EQ(14, step_); step_++; }
	void EndArray(SizeType elementCount) { EXPECT_EQ(18, step_); EXPECT_EQ(3, elementCount); step_++;}

	unsigned step_;
};

TEST(Reader, ParseObject) {
	const char* json = "{ \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3] } ";

	// Insitu
	{
		char* json2 = StrDup(json);
		InsituStringStream s(json2);
		ParseObjectHandler h;
		Reader reader;
		reader.ParseObject<kParseInsituFlag>(s, h);
		EXPECT_EQ(20, h.step_);
		free(json2);
	}

	// Normal
	{
		StringStream s(json);
		ParseObjectHandler h;
		Reader reader;
		reader.ParseObject<0>(s, h);
		EXPECT_EQ(20, h.step_);
	}
}

struct ParseEmptyObjectHandler : BaseReaderHandler<> {
	ParseEmptyObjectHandler() : step_(0) {}

	void Default() { FAIL(); }
	void StartObject() { EXPECT_EQ(0, step_); step_++; }
	void EndObject(SizeType memberCount) { EXPECT_EQ(1, step_); step_++; }

	unsigned step_;
};

TEST(Reader, Parse_EmptyObject) {
	StringStream s("{ } ");
	ParseEmptyObjectHandler h;
	Reader reader;
	reader.ParseObject<0>(s, h);
	EXPECT_EQ(2, h.step_);
}

TEST(Reader, ParseObject_Error) {
#define TEST_OBJECT_ERROR(str) \
	{ \
		char buffer[1001]; \
		strncpy(buffer, str, 1000); \
		InsituStringStream s(buffer); \
		BaseReaderHandler<> h; \
		GenericReader<UTF8<>, CrtAllocator> reader; \
		EXPECT_FALSE(reader.Parse<0>(s, h)); \
	}

	// Name of an object member must be a string
	TEST_OBJECT_ERROR("{null:1}");
	TEST_OBJECT_ERROR("{true:1}");
	TEST_OBJECT_ERROR("{false:1}");
	TEST_OBJECT_ERROR("{1:1}");
	TEST_OBJECT_ERROR("{[]:1}");
	TEST_OBJECT_ERROR("{{}:1}");
	TEST_OBJECT_ERROR("{xyz:1}");

	// There must be a colon after the name of object member
	TEST_OBJECT_ERROR("{\"a\" 1}");
	TEST_OBJECT_ERROR("{\"a\",1}");

	// Must be a comma or '}' after an object member
	TEST_OBJECT_ERROR("{]");
	TEST_OBJECT_ERROR("{\"a\":1]");

#undef TEST_OBJECT_ERROR
}

TEST(Reader, Parse_Error) {
#define TEST_ERROR(str) \
	{ \
		char buffer[1001]; \
		strncpy(buffer, str, 1000); \
		InsituStringStream s(buffer); \
		BaseReaderHandler<> h; \
		Reader reader; \
		EXPECT_FALSE(reader.Parse<0>(s, h)); \
	}

	// Text only contains white space(s)
	TEST_ERROR("");
	TEST_ERROR(" ");
	TEST_ERROR(" \n");

	// Expect either an object or array at root
	TEST_ERROR("null");
	TEST_ERROR("true");
	TEST_ERROR("false");
	TEST_ERROR("\"s\"");
	TEST_ERROR("0");

	// Nothing should follow the root object or array
	TEST_ERROR("[] 0");
	TEST_ERROR("{} 0");

	// Invalid value
	TEST_ERROR("nulL");
	TEST_ERROR("truE");
	TEST_ERROR("falsE");

#undef TEST_ERROR
}
