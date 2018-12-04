#define RAPIDJSON_HAS_STDSTRING 1
#include "rapidjson/document.h"
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include <algorithm>
#include <iostream>

using namespace rapidjson;
using namespace std;

void printIt(Document &doc)
{
    string output;
    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    doc.Accept(writer);

    output = buffer.GetString();
    cout << output << endl;
}

struct ValueNameComparator
{
    bool
    operator()(const GenericMember<UTF8<>, MemoryPoolAllocator<>> &lhs,
               const GenericMember<UTF8<>, MemoryPoolAllocator<>> &rhs) const
    {
        string lhss = string(lhs.name.GetString());
        string rhss = string(rhs.name.GetString());
        return lhss < rhss;
    }
};

int main()
{
    Document d = Document(kObjectType);
    Document::AllocatorType &allocator = d.GetAllocator();

    d.AddMember("zeta", Value().SetBool(false), allocator);
    d.AddMember("gama", Value().SetString("test string", allocator), allocator);
    d.AddMember("delta", Value().SetInt(123), allocator);

    Value a(kArrayType);
    d.AddMember("alpha", a, allocator);

    printIt(d);

    /**
{
    "zeta": false,
    "gama": "test string",
    "delta": 123,
    "alpha": []
}
**/

    std::sort(d.MemberBegin(), d.MemberEnd(), ValueNameComparator());

    printIt(d);
    /**
{
  "alpha": [],
  "delta": 123,
  "gama": "test string",
  "zeta": false
}
**/
    return 0;
}
