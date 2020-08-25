#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <iostream>

using namespace rapidjson;

int main() {
    const char* json = "{\"working\":\"false\"}";
    Document d;
    d.Parse(json);

    Value& w = d["working"];
    w.SetString("true", 4);

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);

    std::cout << buffer.GetString() << std::endl;
    return 0;
}
