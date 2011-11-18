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
		//	std::cout << str.length() << std::endl;
	}
}

TEST_F(JsonCpp, StyledWriter) {
	for (int i = 0; i < kTrialCount; i++) {
		StyledWriter writer;
		std::string str = writer.write(root_);
		//if (i == 0)
		//	std::cout << str.length() << std::endl;
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
