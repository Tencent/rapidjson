#include "unittest.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/encodedstream.h"

using namespace rapidjson;

class EncodingsTest : public ::testing::Test {
public:
	FILE* Open(const char* filename) {
		char buffer[1024];
		sprintf(buffer, "encodings/%s", filename);
		FILE *fp = fopen(buffer, "rb");
		if (!fp) {
			sprintf(buffer, "../../bin/encodings/%s", filename);
			fp = fopen(buffer, "rb");
		}
		return fp;
	}

	virtual void SetUp() {
		FILE *fp = Open("utf8.json");
		ASSERT_TRUE(fp != 0);

		fseek(fp, 0, SEEK_END);
		length_ = (size_t)ftell(fp);
		fseek(fp, 0, SEEK_SET);
		json_ = (char*)malloc(length_ + 1);
		fread(json_, 1, length_, fp);
		json_[length_] = '\0';
		fclose(fp);
	}

	virtual void TearDown() {
		free(json_);
	}

protected:
	const char* filename_;
	char *json_;
	size_t length_;
};

TEST_F(EncodingsTest, EncodedInputStream_UTF8BOM) {
	char buffer[16];
	FILE *fp = Open("utf8bom.json");
	ASSERT_TRUE(fp != 0);
	FileReadStream fs(fp, buffer, sizeof(buffer));
	EncodedInputStream<UTF8<>, FileReadStream> eis(fs);
	StringStream s(json_);

	while (eis.Peek() != '\0') {
		unsigned expected, actual;
		UTF8<>::Decode(s, &expected);
		UTF8<>::Decode(eis, &actual);
		EXPECT_EQ(expected, actual);
	}
	EXPECT_EQ('\0', s.Peek());
	fclose(fp);
}

TEST_F(EncodingsTest, EncodedInputStream_UTF16LEBOM) {
	char buffer[16];
	FILE *fp = Open("utf16lebom.json");
	ASSERT_TRUE(fp != 0);
	FileReadStream fs(fp, buffer, sizeof(buffer));
	EncodedInputStream<UTF16LE<>, FileReadStream> eis(fs);
	StringStream s(json_);

	while (eis.Peek() != '\0') {
		unsigned expected, actual;
		UTF8<>::Decode(s, &expected);
		UTF16<>::Decode(eis, &actual);
		EXPECT_EQ(expected, actual);
	}
	EXPECT_EQ('\0', s.Peek());
	fclose(fp);
}

TEST_F(EncodingsTest, EncodedInputStream_UTF16BEBOM) {
	char buffer[16];
	FILE *fp = Open("utf16bebom.json");
	ASSERT_TRUE(fp != 0);
	FileReadStream fs(fp, buffer, sizeof(buffer));
	EncodedInputStream<UTF16BE<>, FileReadStream> eis(fs);
	StringStream s(json_);

	while (eis.Peek() != '\0') {
		unsigned expected, actual;
		UTF8<>::Decode(s, &expected);
		UTF16<>::Decode(eis, &actual);
		EXPECT_EQ(expected, actual);
	}
	EXPECT_EQ('\0', s.Peek());
	fclose(fp);
}

TEST_F(EncodingsTest, AutoUTFInputStream) {
#define TEST_FILE(filename) \
	{ \
		char buffer[16]; \
		FILE *fp = Open(filename); \
		ASSERT_TRUE(fp != 0); \
		FileReadStream fs(fp, buffer, sizeof(buffer)); \
		AutoUTFInputStream<wchar_t, FileReadStream> eis(fs); \
		StringStream s(json_); \
		while (eis.Peek() != '\0') { \
			unsigned expected, actual; \
			UTF8<>::Decode(s, &expected); \
			AutoUTF<wchar_t>::Decode(eis, &actual); \
			EXPECT_EQ(expected, actual); \
		} \
		EXPECT_EQ('\0', s.Peek()); \
		fclose(fp); \
	}

	TEST_FILE("utf8.json");
	TEST_FILE("utf8bom.json");
	TEST_FILE("utf16lebom.json");
	TEST_FILE("utf16bebom.json");
#undef TEST_FILE
}
