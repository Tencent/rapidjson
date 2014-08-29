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

#include "perftest.h"

#if TEST_JSONCPP

#include "jsoncpp/src/lib_json/json_reader.cpp"
#include "jsoncpp/src/lib_json/json_value.cpp"
#include "jsoncpp/src/lib_json/json_writer.cpp"

using namespace Json;

class JsonCpp : public PerfTest {
public:
    virtual void SetUp() {
        PerfTest::SetUp();
        Reader reader;
        ASSERT_TRUE(reader.parse(json_, root_));
    }

protected:
    Value root_;
};

TEST_F(JsonCpp, ReaderParse) {
    for (int i = 0; i < kTrialCount; i++) {
        Value root;
        Reader reader;
        ASSERT_TRUE(reader.parse(json_, root));
    }
}

TEST_F(JsonCpp, FastWriter) {
    for (int i = 0; i < kTrialCount; i++) {
        FastWriter writer;
        std::string str = writer.write(root_);
        //if (i == 0)
        //  std::cout << str.length() << std::endl;
    }
}

TEST_F(JsonCpp, StyledWriter) {
    for (int i = 0; i < kTrialCount; i++) {
        StyledWriter writer;
        std::string str = writer.write(root_);
        //if (i == 0)
        //  std::cout << str.length() << std::endl;
    }
}

TEST_F(JsonCpp, Whitespace) {
    for (int i = 0; i < kTrialCount; i++) {
        Value root;
        Reader reader;
        ASSERT_TRUE(reader.parse(whitespace_, root));
    }
}

#endif // TEST_JSONCPP
