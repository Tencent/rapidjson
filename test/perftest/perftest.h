#ifndef PERFTEST_H_
#define PERFTEST_H_

#define TEST_RAPIDJSON	1
#define TEST_JSONCPP	1
#define TEST_YAJL		1
#define TEST_ULTRAJSON  1
#define TEST_PLATFORM   1

#if TEST_RAPIDJSON
//#define RAPIDJSON_SSE2
//#define RAPIDJSON_SSE42
#endif

#if TEST_YAJL
#include "yajl/yajl_common.h"
#undef YAJL_MAX_DEPTH
#define YAJL_MAX_DEPTH 1024
#endif

////////////////////////////////////////////////////////////////////////////////
// Google Test

#ifdef __cplusplus

#include "gtest/gtest.h"

#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#pragma warning(disable : 4996) // 'function': was declared deprecated
#endif

//! Base class for all performance tests
class PerfTest : public ::testing::Test {
public:
	virtual void SetUp() {
		FILE *fp = fopen(filename_ = "data/sample.json", "rb");
		if (!fp) 
			fp = fopen(filename_ = "../../bin/data/sample.json", "rb");
		ASSERT_TRUE(fp != 0);

		fseek(fp, 0, SEEK_END);
		length_ = (size_t)ftell(fp);
		fseek(fp, 0, SEEK_SET);
		json_ = (char*)malloc(length_ + 1);
		ASSERT_EQ(length_, fread(json_, 1, length_, fp));
		json_[length_] = '\0';
		fclose(fp);

		// whitespace test
		whitespace_length_ = 1024 * 1024;
		whitespace_ = (char *)malloc(whitespace_length_  + 4);
		char *p = whitespace_;
		for (size_t i = 0; i < whitespace_length_; i += 4) {
			*p++ = ' ';
			*p++ = '\n';
			*p++ = '\r';
			*p++ = '\t';
		}
		*p++ = '[';
		*p++ = '0';
		*p++ = ']';
		*p++ = '\0';
	}

	virtual void TearDown() {
		free(json_);
		free(whitespace_);
	}

protected:
	const char* filename_;
	char *json_;
	size_t length_;
	char *whitespace_;
	size_t whitespace_length_;

	static const size_t kTrialCount = 1000;
};

#endif // __cplusplus

#endif // PERFTEST_H_
