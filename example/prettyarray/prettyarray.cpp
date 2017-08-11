// This example demonstrates the different PrettyFormatOptions: 
// - kFormatDefault
// - kFormatSingleLineArray
// - kFormatPretty2DArray

#include "rapidjson/document.h"     // rapidjson's DOM-style API
#include "rapidjson/prettywriter.h" // for stringify JSON
#include <cstdio>

using namespace rapidjson;
using namespace std;

int main(int, char*[]) {
    ////////////////////////////////////////////////////////////////////////////
    // 1. Parse a JSON text string to a document.

    const char json[] = " { \"OneDim\":[1, 2, 3, 4], \
        \"TwoDim\":[[11, 12, 13, 14],[21, 22, 23, 24]], \
        \"ThreeDim\":[[[111,112],[121,122],[131,132]],[[211,212],[221,221],[231,232]],[[311,312],[321,322]]], \
        \"Mixed\":[ 1, [21, 22], 3, 4, [51, 52], 6 ]} ";
    printf("Original JSON:\n %s\n", json);

    Document document;  // Default template parameter uses UTF8 and MemoryPoolAllocator.

    // In-situ parsing, decode strings directly in the source string. Source must be string.
    char buffer[sizeof(json)];
    memcpy(buffer, json, sizeof(json));
    if (document.Parse(buffer).HasParseError())
    {
        printf("\nParsing document failed.\n");
        return 1;
    }

    printf("\nParsing document succeeded.\n");

    assert(document.IsObject());    // Document is a JSON value represents the root of DOM. Root can be either an object or array.

    ////////////////////////////////////////////////////////////////////////////
    // 2. Stringify JSON

    StringBuffer sb;
    PrettyWriter<StringBuffer> writer(sb);
    writer.SetFormatOptions(kFormatSingleLineArray);
    document.Accept(writer);
    printf("\nModified JSON with kFormatSingleLineArray:\n");
    puts(sb.GetString());

    sb.Clear();
    writer.SetFormatOptions(kFormatPretty2DArray);
    document.Accept(writer);
    printf("\nModified JSON with kFormatPretty2DArray:\n");
    puts(sb.GetString());

    sb.Clear();
    writer.SetFormatOptions(kFormatDefault);
    document.Accept(writer);
    printf("\nunModified JSON:\n");
    puts(sb.GetString());

    return 0;
}
