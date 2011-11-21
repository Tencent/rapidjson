// JSON pretty formatting example

#include "rapidjson/reader.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"

using namespace rapidjson;

int main(int argc, char* argv[]) {
	// Prepare reader and input stream.
	Reader reader;
	char readBuffer[65536];
	FileReadStream is(stdin, readBuffer, sizeof(readBuffer));

	// Prepare writer and output stream.
	char writeBuffer[65536];
	FileWriteStream os(stdout, writeBuffer, sizeof(writeBuffer));
	PrettyWriter<FileWriteStream> writer(os);

	// JSON reader parse from the input stream and let writer generate the output.
	if (!reader.Parse<0>(is, writer)) {
		fprintf(stderr, "\nError(%u): %s\n", (unsigned)reader.GetErrorOffset(), reader.GetParseError());
		return 1;
	}

	os.Flush();
	return 0;
}
