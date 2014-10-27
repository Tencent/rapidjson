// Copyright (C) 2011 Milo Yip
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include "unittest.h"
#include "rapidjson/filestream.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/encodedstream.h"

using namespace rapidjson;

class FileStreamTest : public ::testing::Test {
public:
    FileStreamTest() : filename_(), json_(), length_() {}

    virtual void SetUp() {
        FILE *fp = fopen(filename_ = "data/sample.json", "rb");
        if (!fp) 
            fp = fopen(filename_ = "../../bin/data/sample.json", "rb");
        ASSERT_TRUE(fp != 0);

        fseek(fp, 0, SEEK_END);
        length_ = (size_t)ftell(fp);
        fseek(fp, 0, SEEK_SET);
        json_ = (char*)malloc(length_ + 1);
        size_t readLength = fread(json_, 1, length_, fp);
        json_[readLength] = '\0';
        fclose(fp);
    }

    virtual void TearDown() {
        free(json_);
        json_ = 0;
    }

private:
    FileStreamTest(const FileStreamTest&);
    FileStreamTest& operator=(const FileStreamTest&);
    
protected:
    const char* filename_;
    char *json_;
    size_t length_;
};

// Deprecated
//TEST_F(FileStreamTest, FileStream_Read) {
//  FILE *fp = fopen(filename_, "rb");
//  ASSERT_TRUE(fp != 0);
//  FileStream s(fp);
//
//  for (size_t i = 0; i < length_; i++) {
//      EXPECT_EQ(json_[i], s.Peek());
//      EXPECT_EQ(json_[i], s.Peek());  // 2nd time should be the same
//      EXPECT_EQ(json_[i], s.Take());
//  }
//
//  EXPECT_EQ(length_, s.Tell());
//  EXPECT_EQ('\0', s.Peek());
//
//  fclose(fp);
//}

TEST_F(FileStreamTest, FileReadStream) {
    FILE *fp = fopen(filename_, "rb");
    ASSERT_TRUE(fp != 0);
    char buffer[65536];
    FileReadStream s(fp, buffer, sizeof(buffer));

    for (size_t i = 0; i < length_; i++) {
        EXPECT_EQ(json_[i], s.Peek());
        EXPECT_EQ(json_[i], s.Peek());  // 2nd time should be the same
        EXPECT_EQ(json_[i], s.Take());
    }

    EXPECT_EQ(length_, s.Tell());
    EXPECT_EQ('\0', s.Peek());

    fclose(fp);
}

TEST_F(FileStreamTest, FileWriteStream) {
    char filename[L_tmpnam];
    FILE* fp = TempFile(filename);

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
