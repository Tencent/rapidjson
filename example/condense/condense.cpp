// JSON condenser exmaple

// This example parses JSON text from stdin with validation, 
// and re-output the JSON content to stdout without whitespace.

#include "rapidjson/reader.h"
#include "rapidjson/writer.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"

using namespace rapidjson;

int main(int argc, char* argv[]) {
	// Prepare JSON reader and input stream.
	Reader reader;
	char readBuffer[65536];
	FileReadStream is(stdin, readBuffer, sizeof(readBuffer));

	// Prepare JSON writer and output stream.
	char writeBuffer[65536];
	FileWriteStream os(stdout, writeBuffer, sizeof(writeBuffer));
	Writer<FileWriteStream> writer(os);

	// JSON reader parse from the input stream and let writer generate the output.
	if (!reader.Parse<0>(is, writer)) {
		fprintf(stderr, "\nError(%u): %s\n", (unsigned)reader.GetErrorOffset(), reader.GetParseError());
		return 1;
	}

	os.Flush();

	return 0;
}
