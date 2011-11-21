#include "unittest.h"
#include "rapidjson/filestream.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"

using namespace rapidjson;

class FileStreamTest : public ::testing::Test {
	virtual void SetUp() {
		FILE *fp = fopen(filename_ = "data/sample.json", "rb");
		if (!fp) 
			fp = fopen(filename_ = "../../bin/data/sample.json", "rb");
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

TEST_F(FileStreamTest, FileStream_Read) {
	FILE *fp = fopen(filename_, "rb");
	ASSERT_TRUE(fp != 0);
	FileStream s(fp);

	for (size_t i = 0; i < length_; i++) {
		EXPECT_EQ(json_[i], s.Peek());
		EXPECT_EQ(json_[i], s.Peek());	// 2nd time should be the same
		EXPECT_EQ(json_[i], s.Take());
	}

	EXPECT_EQ(length_, s.Tell());
	EXPECT_EQ('\0', s.Peek());

	fclose(fp);
}

TEST_F(FileStreamTest, FileReadStream) {
	FILE *fp = fopen(filename_, "rb");
	ASSERT_TRUE(fp != 0);
	char buffer[65536];
	FileReadStream s(fp, buffer, sizeof(buffer));

	for (size_t i = 0; i < length_; i++) {
		EXPECT_EQ(json_[i], s.Peek());
		EXPECT_EQ(json_[i], s.Peek());	// 2nd time should be the same
		EXPECT_EQ(json_[i], s.Take());
	}

	EXPECT_EQ(length_, s.Tell());
	EXPECT_EQ('\0', s.Peek());

	fclose(fp);
}

TEST_F(FileStreamTest, FileWriteStream) {
	char filename[L_tmpnam];
	tmpnam(filename);

	FILE *fp = fopen(filename, "wb");
	char buffer[65536];
	FileWriteStream os(fp, buffer, sizeof(buffer));
	for (size_t i = 0; i < length_; i++)
		os.Put(json_[i]);
	os.Flush();
	fclose(fp);

	// Read it back to verify
	fp = fopen(filename, "rb");
	FileReadStream is(fp, buffer, sizeof(buffer));

	for (size_t i = 0; i < length_; i++)
		EXPECT_EQ(json_[i], is.Take());

	EXPECT_EQ(length_, is.Tell());
	fclose(fp);

	//std::cout << filename << std::endl;
	remove(filename);
}
