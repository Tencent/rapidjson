// JSON pretty formatting example

#include "rapidjson/reader.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filestream.h"

using namespace rapidjson;

int main(int argc, char* argv[]) {
	// Prepare reader and input stream.
	Reader reader;
	FileStream is(stdin);

	// Prepare writer and output stream.
	FileStream os(stdout);
	PrettyWriter<FileStream> writer(os);

	// JSON reader parse from the input stream and let writer generate the output.
	if (!reader.Parse<0>(is, writer)) {
		fprintf(stderr, "\nError(%u): %s\n", (unsigned)reader.GetErrorOffset(), reader.GetParseError());
		return 1;
	}

	return 0;
}
