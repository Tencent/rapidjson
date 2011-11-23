#include "perftest.h"

#if TEST_ULTRAJSON

#include "ultrajson/ultrajsondec.c"
#include "ultrajson/ultrajsonenc.c"

class UltraJson : public PerfTest {
};

static char dummy = 0;

static void Object_objectAddKey(JSOBJ obj, JSOBJ name, JSOBJ value) {}
static void Object_arrayAddItem(JSOBJ obj, JSOBJ value) {}

static JSOBJ Object_newString(wchar_t *start, wchar_t *end)	{ return &dummy; }
static JSOBJ Object_newTrue(void)							{ return &dummy; }
static JSOBJ Object_newFalse(void)							{ return &dummy; }
static JSOBJ Object_newNull(void)							{ return &dummy; }
static JSOBJ Object_newObject(void)							{ return &dummy; }
static JSOBJ Object_newArray(void)							{ return &dummy; }
static JSOBJ Object_newInteger(JSINT32 value)				{ return &dummy; }
static JSOBJ Object_newLong(JSINT64 value)					{ return &dummy; }
static JSOBJ Object_newDouble(double value)					{ return &dummy; }

static void Object_releaseObject(JSOBJ obj) {}

static JSONObjectDecoder decoder = {
	Object_newString,
	Object_objectAddKey,
	Object_arrayAddItem,
	Object_newTrue,
	Object_newFalse,
	Object_newNull,
	Object_newObject,
	Object_newArray,
	Object_newInteger,
	Object_newLong,
	Object_newDouble,
	Object_releaseObject,
	malloc,
	free,
	realloc
};

TEST_F(UltraJson, Decode) {
	for (int i = 0; i < kTrialCount; i++) {
		decoder.errorStr = NULL;
		decoder.errorOffset = NULL;
		void *ret = JSON_DecodeObject(&decoder, json_, length_);
		ASSERT_TRUE(ret != 0);
	}
}

TEST_F(UltraJson, Whitespace) {
	for (int i = 0; i < kTrialCount; i++) {
		decoder.errorStr = NULL;
		decoder.errorOffset = NULL;
		void *ret = JSON_DecodeObject(&decoder, whitespace_, whitespace_length_);
		ASSERT_TRUE(ret != 0);
	}
}

#endif // TEST_ULTRAJSON
