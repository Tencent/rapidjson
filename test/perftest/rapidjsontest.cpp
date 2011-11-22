#include "perftest.h"

#if TEST_RAPIDJSON

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filestream.h"
#include "rapidjson/filereadstream.h"

#ifdef RAPIDJSON_SSE2
#define SIMD_SUFFIX(name) name##_SSE2
#elif defined(RAPIDJSON_SSE42)
#define SIMD_SUFFIX(name) name##_SSE42
#else
#define SIMD_SUFFIX(name) name
#endif

using namespace rapidjson;

class RapidJson : public PerfTest {
public:
	virtual void SetUp() {
		PerfTest::SetUp();

		// temp buffer for insitu parsing.
		temp_ = (char *)malloc(length_ + 1);

		// Parse as a document
		EXPECT_FALSE(doc_.Parse<0>(json_).IsNull());
	}

	virtual void TearDown() {
		PerfTest::TearDown();
		free(temp_);
	}

protected:
	char *temp_;
	Document doc_;
};

TEST_F(RapidJson, SIMD_SUFFIX(ReaderParseInsitu_NullHandler)) {
	for (int i = 0; i < kTrialCount; i++) {
		memcpy(temp_, json_, length_ + 1);
		InsituStringStream s(temp_);
		BaseReaderHandler<> h;
		Reader reader;
		reader.Parse<kParseInsituFlag>(s, h);
	}
}

TEST_F(RapidJson, SIMD_SUFFIX(ReaderParse_NullHandler)) {
	for (int i = 0; i < kTrialCount; i++) {
		StringStream s(json_);
		BaseReaderHandler<> h;
		Reader reader;
		reader.Parse<0>(s, h);
	}
}

TEST_F(RapidJson, SIMD_SUFFIX(DoucmentParseInsitu_MemoryPoolAllocator)) {
	//const size_t userBufferSize = 128 * 1024;
	//char* userBuffer = (char*)malloc(userBufferSize);

	for (int i = 0; i < kTrialCount; i++) {
		memcpy(temp_, json_, length_ + 1);
		//MemoryPoolAllocator<> allocator(userBuffer, userBufferSize);
		//Document doc(&allocator);
		Document doc;
		doc.ParseInsitu<0>(temp_);
		ASSERT_TRUE(doc.IsObject());
		//if (i == 0) {
		//	size_t size = doc.GetAllocator().Size();
		//	size_t capacity = doc.GetAllocator().Capacity();
		//	size_t stack_capacity = doc.GetStackCapacity();
		//	size_t actual = size - stack_capacity;
		//	std::cout << "Size:" << size << " Capacity:" << capacity  << " Stack:" << stack_capacity << " Actual:" << actual << std::endl;
		//}
	}

	//free(userBuffer);
}

TEST_F(RapidJson, SIMD_SUFFIX(DoucmentParse_MemoryPoolAllocator)) {
	//const size_t userBufferSize = 128 * 1024;
	//char* userBuffer = (char*)malloc(userBufferSize);

	for (int i = 0; i < kTrialCount; i++) {
		//MemoryPoolAllocator<> allocator(userBuffer, userBufferSize);
		//Document doc(&allocator);
		Document doc;
		doc.Parse<0>(json_);
		ASSERT_TRUE(doc.IsObject());
		//if (i == 0) {
		//	size_t size = doc.GetAllocator().Size();
		//	size_t capacity = doc.GetAllocator().Capacity();
		//	size_t stack_capacity = doc.GetStackCapacity();
		//	size_t actual = size - stack_capacity;
		//	std::cout << "Size:" << size << " Capacity:" << capacity  << " Stack:" << stack_capacity << " Actual:" << actual << std::endl;
		//}
	}

	//free(userBuffer);
}

TEST_F(RapidJson, SIMD_SUFFIX(DoucmentParse_CrtAllocator)) {
	for (int i = 0; i < kTrialCount; i++) {
		memcpy(temp_, json_, length_ + 1);
		GenericDocument<UTF8<>, CrtAllocator> doc;
		doc.Parse<0>(temp_);
		ASSERT_TRUE(doc.IsObject());
	}
}

template<typename T>
size_t Traverse(const T& value) {
	size_t count = 1;
	switch(value.GetType()) {
		case kObjectType:
			for (typename T::ConstMemberIterator itr = value.MemberBegin(); itr != value.MemberEnd(); ++itr) {
				count++;	// name
				count += Traverse(itr->value);
			}
			break;

		case kArrayType:
			for (typename T::ConstValueIterator itr = value.Begin(); itr != value.End(); ++itr)
				count += Traverse(*itr);
			break;
	}
	return count;
}

TEST_F(RapidJson, DocumentTraverse) {
	for (int i = 0; i < kTrialCount; i++) {
		size_t count = Traverse(doc_);
		EXPECT_EQ(4339, count);
		//if (i == 0)
		//	std::cout << count << std::endl;
	}
}

struct ValueCounter : public BaseReaderHandler<> {
	ValueCounter() : count_(1) {}	// root

	void EndObject(SizeType memberCount) { count_ += memberCount * 2; }
	void EndArray(SizeType elementCount) { count_ += elementCount; }

	SizeType count_;
};

TEST_F(RapidJson, DocumentAccept) {
	for (int i = 0; i < kTrialCount; i++) {
		ValueCounter counter;
		doc_.Accept(counter);
		EXPECT_EQ(4339, counter.count_);
	}
}

struct NullStream {
	NullStream() /*: length_(0)*/ {}
	void Put(char c) { /*++length_;*/ }
	void Flush() {}
	//size_t length_;
};

TEST_F(RapidJson, Writer_NullStream) {
	for (int i = 0; i < kTrialCount; i++) {
		NullStream s;
		Writer<NullStream> writer(s);
		doc_.Accept(writer);
		//if (i == 0)
		//	std::cout << s.length_ << std::endl;
	}
}

TEST_F(RapidJson, Writer_StringBuffer) {
	for (int i = 0; i < kTrialCount; i++) {
		StringBuffer s(0, 1024 * 1024);
		Writer<StringBuffer> writer(s);
		doc_.Accept(writer);
		const char* str = s.GetString();
		//if (i == 0)
		//	std::cout << strlen(str) << std::endl;
	}
}

TEST_F(RapidJson, PrettyWriter_StringBuffer) {
	for (int i = 0; i < kTrialCount; i++) {
		StringBuffer s(0, 2048 * 1024);
		PrettyWriter<StringBuffer> writer(s);
		writer.SetIndent(' ', 1);
		doc_.Accept(writer);
		const char* str = s.GetString();
		//if (i == 0)
		//	std::cout << strlen(str) << std::endl;
	}
}

TEST_F(RapidJson, internal_Pow10) {
	double sum = 0;
	for (int i = 0; i < kTrialCount * kTrialCount; i++)
		sum += internal::Pow10(i & 255);
	EXPECT_GT(sum, 0.0);
}

TEST_F(RapidJson, SIMD_SUFFIX(Whitespace)) {
	for (int i = 0; i < kTrialCount; i++) {
		Document doc;
		ASSERT_TRUE(doc.Parse<0>(whitespace_).IsArray());
	}		
}

// Depreciated.
//TEST_F(RapidJson, FileStream_Read) {
//	for (int i = 0; i < kTrialCount; i++) {
//		FILE *fp = fopen(filename_, "rb");
//		FileStream s(fp);
//		while (s.Take() != '\0')
//			;
//		fclose(fp);
//	}
//}

TEST_F(RapidJson, FileReadStream) {
	for (int i = 0; i < kTrialCount; i++) {
		FILE *fp = fopen(filename_, "rb");
		char buffer[65536];
		FileReadStream s(fp, buffer, sizeof(buffer));
		while (s.Take() != '\0')
			;
		fclose(fp);
	}
}

TEST_F(RapidJson, SIMD_SUFFIX(ReaderParse_NullHandler_FileReadStream)) {
	for (int i = 0; i < kTrialCount; i++) {
		FILE *fp = fopen(filename_, "rb");
		char buffer[65536];
		FileReadStream s(fp, buffer, sizeof(buffer));
		BaseReaderHandler<> h;
		Reader reader;
		reader.Parse<0>(s, h);
		fclose(fp);
	}
}

#endif // TEST_RAPIDJSON
