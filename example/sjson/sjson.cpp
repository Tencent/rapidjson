// JSON to SJSON conversion utility.
// SJSON is an Autodesk format, which has some exceptions and simplifications to the json format.
// http://help.autodesk.com/view/Stingray/ENU/?guid=__stingray_help_managing_content_sjson_html
// This example parses JSON or SJSON text from stdin with validation, 
// and converts to SJSON or JSON format to stdout.

#include "rapidjson/reader.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/error/en.h"
#include <cstdio>

using namespace rapidjson;

template<typename ReaderType, typename WriterType> 
int performParsing(ReaderType& reader, WriterType& writer, FileReadStream& is) {
    // JSON reader parse from the input stream and let writer generate the output.
    if (reader.Parse<kParseSJSONDefaultFlags>(is, writer).IsError()) {
        fprintf(stderr, "\nError(%u): %s\n", static_cast<unsigned>(reader.GetErrorOffset()), GetParseError_En(reader.GetParseErrorCode()));
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    bool json_output = true;
    bool pretty_output = true;
    // Read command line arguments.
    for (int i = 0; i < argc; ++i) {
        if (!strcmp(argv[i], "--sjson-output"))
            json_output = false;
        else if (!strcmp(argv[i], "--no-pretty"))
            pretty_output = false;
        else if (!strcmp(argv[i], "--help")) {
            fprintf(stdout, "--sjson-output  Output file is in written with sjson simplifications (not compatible with json-only readers).\n");
            fprintf(stdout, "--no-pretty     Output is condensed (otherwise, PrettyWriter is used).\n");
            fprintf(stdout, "--help          Write this help and exit.\n");
            return 0;
        }
    }

    // Prepare JSON reader and input stream.
    Reader reader;
    char readBuffer[65536];
    FileReadStream is(stdin, readBuffer, sizeof(readBuffer));

    // Prepare JSON writer and output stream.
    char writeBuffer[65536];
    FileWriteStream os(stdout, writeBuffer, sizeof(writeBuffer));

    if (!json_output) {
        if (pretty_output) {
            PrettyWriter<FileWriteStream, UTF8<>, UTF8<>, CrtAllocator, kWriteSJSONDefaultFlags> writer(os);
            writer.SetFormatOptions(kFormatSingleLineArray);
            return performParsing(reader, writer, is);
        }
        else {
            Writer<FileWriteStream, UTF8<>, UTF8<>, CrtAllocator, kWriteSJSONDefaultFlags> writer(os);
            return performParsing(reader, writer, is);
        }
    }
    else {
        if (pretty_output) {
            PrettyWriter<FileWriteStream> writer(os);
            writer.SetFormatOptions(kFormatSingleLineArray);
            return performParsing(reader, writer, is);
        }
        else {
            Writer<FileWriteStream> writer(os);
            return performParsing(reader, writer, is);
        }
    }

    return 0;
}
