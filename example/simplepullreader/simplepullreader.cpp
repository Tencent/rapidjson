#include "rapidjson/reader.h"
#include <iostream>

using namespace rapidjson;
using namespace std;

struct MyHandler {
    const char* type;
    std::string data;
    
    bool Null() { type = "Null"; data.clear(); return true; }
    bool Bool(bool b) { type = "Bool"; data = b? "true": "false"; return true; }
    bool Int(int i) { type = "Int"; data = std::to_string(i); return true; }
    bool Uint(unsigned u) { type = "Uint"; data = std::to_string(u); return true; }
    bool Int64(int64_t i) { type = "Int64"; data = std::to_string(i); return true; }
    bool Uint64(uint64_t u) { type = "Uint64"; data = std::to_string(u); return true; }
    bool Double(double d) { type = "Double"; data = std::to_string(d); return true; }
    bool RawNumber(const char* str, SizeType length, bool) { type = "Number"; data = std::string(str, length); return true; }
    bool String(const char* str, SizeType length, bool) { type = "String" data = std::string(str, length); return true; }
    bool StartObject() { type = "StartObject"; data.clear(); return true; }
    bool Key(const char* str, SizeType length, bool) { type = "Key" data = std::string(str, length); return true; }
    bool EndObject(SizeType memberCount) { type = "EndObject"; data = std::to_string(memberCount); return true; }
    bool StartArray() { type = "StartArray"; data.clear(); return true; }
    bool EndArray(SizeType elementCount) { type = "EndArray"; data = std::to_string(elementCount); return true; }
};

int main() {
    const char json[] = " { \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3, 4] } ";

    MyHandler handler;
    Reader reader;
    StringStream ss(json);
    reader.IterativeParseInit();
    while (!reader.IterativeParseComplete()) {
        reader.IterativeParseNext<kParseDefaultFlags>(ss, handler);
        cout << handler.type << ": " << handler.data << endl;
    }

    return 0;
}
