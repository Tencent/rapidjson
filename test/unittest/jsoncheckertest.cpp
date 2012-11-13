#include "unittest.h"

#include "rapidjson/document.h"

using namespace rapidjson;

static char* ReadFile(const char* filename, size_t& length) {
	FILE *fp = fopen(filename, "rb");
	if (!fp)
		fp = fopen(filename, "rb");
	if (!fp)
		return 0;

	fseek(fp, 0, SEEK_END);
	length = (size_t)ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char* json = (char*)malloc(length + 1);
	fread(json, 1, length, fp);
	json[length] = '\0';
	fclose(fp);
	return json;
}

TEST(JsonChecker, Reader) {
	char filename[256];

	// jsonchecker/failXX.json
	for (int i = 1; i <= 33; i++) {
		if (i == 18)	// fail18.json is valid in rapidjson, which has no limitation on depth of nesting.
			continue;

		sprintf(filename, "jsonchecker/fail%d.json", i);
		size_t length;
		char* json = ReadFile(filename, length);
		if (!json) {
			sprintf(filename, "../../bin/jsonchecker/fail%d.json", i);
			json = ReadFile(filename, length);
			if (!json) {
				printf("jsonchecker file %s not found", filename);
				continue;
			}
		}

		GenericDocument<UTF8<>, CrtAllocator> document;	// Use Crt allocator to check exception-safety (no memory leak)
		if (!document.Parse<0>((const char*)json).HasParseError())
			FAIL();
		//printf("%s(%u):%s\n", filename, (unsigned)document.GetErrorOffset(), document.GetParseError());
		free(json);
	}

	// passX.json
	for (int i = 1; i <= 3; i++) {
		sprintf(filename, "jsonchecker/pass%d.json", i);
		size_t length;
		char* json = ReadFile(filename, length);
		if (!json) {
			sprintf(filename, "../../bin/jsonchecker/pass%d.json", i);
			json = ReadFile(filename, length);
			if (!json) {
				printf("jsonchecker file %s not found", filename);
				continue;
			}
		}

		GenericDocument<UTF8<>, CrtAllocator> document;	// Use Crt allocator to check exception-safety (no memory leak)
		document.Parse<0>((const char*)json);
		EXPECT_TRUE(!document.HasParseError());
		free(json);
	}
}
