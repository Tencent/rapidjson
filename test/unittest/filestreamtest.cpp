// Tencent is pleased to support the open source community by making RapidJSON available.
// 
// Copyright (C) 2015 THL A29 Limited, a Tencent company, and Milo Yip. All rights reserved.
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

#include "unittest.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/encodedstream.h"

using namespace rapidjson;

class FileStreamTest : public ::testing::Test {
public:
    FileStreamTest() : filename_(), json_(), length_() {}
    virtual ~FileStreamTest();

    virtual void SetUp() {
        const char *paths[] = {
            "data/sample.json",
            "bin/data/sample.json",
            "../bin/data/sample.json",
            "../../bin/data/sample.json",
            "../../../bin/data/sample.json"
        };
        FILE* fp = 0;
        for (size_t i = 0; i < sizeof(paths) / sizeof(paths[0]); i++) {
            fp = fopen(paths[i], "rb");
            if (fp) {
                filename_ = paths[i];
                break;
            }
        }
        ASSERT_TRUE(fp != 0);

        fseek(fp, 0, SEEK_END);
        length_ = static_cast<size_t>(ftell(fp));
        fseek(fp, 0, SEEK_SET);
        json_ = static_cast<char*>(malloc(length_ + 1));
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

FileStreamTest::~FileStreamTest() {}

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
