// Copy of simpledom example
// The allocation uses stub fixed memory buffers allocated on stack

#include <iostream>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

using namespace rapidjson;
using namespace std;

struct CustomAlloc {
	unsigned char * buffer;
	size_t capacity;
	CustomAlloc(size_t cap , unsigned char *b) : buffer(b),capacity(cap) {
	}
	CustomAlloc() { 
		RAPIDJSON_ASSERT(0); 
	}
	void * Realloc(void * origin, size_t o_size, size_t new_size) { 
		RAPIDJSON_ASSERT(new_size <= capacity);
		if (origin == buffer) {
			return buffer;
		}
		RAPIDJSON_ASSERT(!origin);
		RAPIDJSON_ASSERT(!o_size);
		return buffer; 
	}
	static void Free(void *) {}
	void * Malloc(size_t sz) { 
		RAPIDJSON_ASSERT(sz < capacity);
		return buffer; 
	}
};

template < size_t S >
struct TAlloc : public CustomAlloc {
	unsigned char p[S];
	TAlloc() : CustomAlloc(S, p) {}
};

typedef MemoryPoolAllocator<CustomAlloc> myMemoryPoolAlloc;
typedef GenericDocument<UTF8<>, myMemoryPoolAlloc , CustomAlloc > myDocument;
typedef GenericValue< UTF8<>, myMemoryPoolAlloc > myValue;

int main() {
	size_t chunk_size = 20;
	TAlloc<400> mStackAlloc0;
	TAlloc<400> mStackAlloc1;
	myMemoryPoolAlloc mMempAlloc(chunk_size , &mStackAlloc0);

	// 1. Parse a JSON string into DOM.
	myDocument d(&mMempAlloc, chunk_size, &mStackAlloc1);
	const char* json = "{\"project\":\"rapidjson\",\"stars\":10}";
	d.Parse(json);

	// 2. Modify it by DOM.
	myValue& s = d["stars"];
	s.SetInt(s.GetInt() + 1);

	// 3. Stringify the DOM
	StringBuffer buffer;
	Writer<StringBuffer> writer(buffer);
	d.Accept(writer);

	// Output {"project":"rapidjson","stars":11}
	std::cout << buffer.GetString() << std::endl;

    return 0;
}
