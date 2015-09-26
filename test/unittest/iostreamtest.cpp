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

#include <fstream>
#include <stdio.h>

#include "unittest.h"
#include "rapidjson/iostreamreadstream.h"
#include "rapidjson/iostreamwritestream.h"
#include "rapidjson/encodedstream.h"

using namespace rapidjson;

class IOStreamStreamTest : public ::testing::Test {
public:
    IOStreamStreamTest() : filename_(), json_(), length_() {}

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
    IOStreamStreamTest(const IOStreamStreamTest&);
    IOStreamStreamTest& operator=(const IOStreamStreamTest&);

protected:
    const char* filename_;
    char *json_;
    size_t length_;
};

TEST_F(IOStreamStreamTest, IOStreamReadStream) {

  //#if _MSC_VER
    std::ifstream stream(filename_, std::ios_base::in | std::ios_base::binary);
    //#else
    //std::ifstream stream(filename_);
    //#endif

    ASSERT_TRUE(stream.good());
    IOStreamReadStream s(stream);

    for (size_t i = 0; i < length_; i++) {
        EXPECT_EQ(json_[i], s.Peek());
        EXPECT_EQ(json_[i], s.Peek());  // 2nd time should be the same
        EXPECT_EQ(json_[i], s.Take());
    }

    EXPECT_EQ(length_, s.Tell());
    EXPECT_EQ('\0', s.Peek());
}

TEST_F(IOStreamStreamTest, IOStreamWriteStream) {
    char filename[L_tmpnam];
    TempFileName(filename);
    {
      // scope the stream so it shut at the end of the block
#if _MSC_VER
      std::ofstream stream(filename, std::ios_base::out | std::ios_base::binary);
#else
      std::ofstream stream(filename, std::ios_base::out);
#endif
      ASSERT_TRUE(stream.good());

      IOStreamWriteStream os(stream);
      for (size_t i = 0; i < length_; i++)
        os.Put(json_[i]);
      os.Flush();
    }

    // Read it back to verify
#if _MSC_VER
    std::ifstream istream(filename_, std::ios_base::in | std::ios_base::binary);
#else
    std::ifstream istream(filename_);
#endif
    IOStreamReadStream is(istream);

    for (size_t i = 0; i < length_; i++)
        EXPECT_EQ(json_[i], is.Take());

    EXPECT_EQ(length_, is.Tell());

    //std::cout << filename << std::endl;
    remove(filename);
}
