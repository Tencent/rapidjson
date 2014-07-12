# SAX

The term "SAX" originated from [Simple API for XML](http://en.wikipedia.org/wiki/Simple_API_for_XML). We borrowed this term for JSON parsing and generation.

In RapidJSON, `Reader` (typedef of `GenericReader<...>`) is the SAX-style parser for JSON, and `Writer` (typedef of `GenericWriter<...>`) is the SAX-style generator for JSON.

[TOC]

# Reader {#Reader}

`Reader` parses a JSON from a stream. While it reads characters from the stream, it analyze the characters according to the syntax of JSON, and publish events to a handler.

For example, here is a JSON.

~~~~~~~~~~js
{
    "hello": "world",
    "t": true ,
    "f": false,
    "n": null,
    "i": 123,
    "pi": 3.1416,
    "a": [1, 2, 3, 4]
}
~~~~~~~~~~

While a `Reader` parses the JSON, it will publish the following events to the handler sequentially:

~~~~~~~~~~
BeginObject()
String("hello", 5, true)
String("world", 5, true)
String("t", 1, true)
Bool(true)
String("f", 1, true)
Bool(false)
String("n", 1, true)
Null()
String("i")
UInt(123)
String("pi")
Double(3.1416)
String("a")
BeginArray()
Uint(1)
Uint(2)
Uint(3)
Uint(4)
EndArray(4)
EndObject(7)
~~~~~~~~~~

These events can be easily match up with the JSON, except some event parameters need further explanation. Let's see the simplereader example which produces exactly the same output as above:

~~~~~~~~~~cpp
#include "rapidjson/reader.h"
#include <iostream>

using namespace rapidjson;
using namespace std;

struct MyHandler {
    bool Null() { cout << "Null()" << endl; return true; }
    bool Bool(bool b) { cout << "Bool(" << boolalpha << b << ")" << endl; return true; }
    bool Int(int i) { cout << "Int(" << i << ")" << endl; return true; }
    bool Uint(unsigned u) { cout << "Uint(" << u << ")" << endl; return true; }
    bool Int64(int64_t i) { cout << "Int64(" << i << ")" << endl; return true; }
    bool Uint64(uint64_t u) { cout << "Uint64(" << u << ")" << endl; return true; }
    bool Double(double d) { cout << "Double(" << d << ")" << endl; return true; }
    bool String(const char* str, SizeType length, bool copy) { 
        cout << "String(" << str << ", " << length << ", " << boolalpha << copy << ")" << endl;
        return true;
    }
    bool StartObject() { cout << "StartObject()" << endl; return true; }
    bool EndObject(SizeType memberCount) { cout << "EndObject(" << memberCount << ")" << endl; return true; }
    bool StartArray() { cout << "StartArray()" << endl; return true; }
    bool EndArray(SizeType elementCount) { cout << "EndArray(" << elementCount << ")" << endl; return true; }
};

void main() {
    const char json[] = " { \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3, 4] } ";

    MyHandler handler;
    Reader reader;
    StringStream ss(json);
    reader.Parse(ss, handler);
}
~~~~~~~~~~

Note that, RapidJSON uses template to statically bind the `Reader` type and the handler type, instead of using class with virtual functions. This paradigm can improve the performance by inlining functions.

## Handler {#Handler}

As the previous example showed, user needs to implement a handler, which consumes the events (function calls) from `Reader`. The handler concept has the following member type and member functions.

~~~~~~~~~~cpp
concept Handler {
    bool Null();
    bool Bool(bool b);
    bool Int(int i);
    bool Uint(unsigned i);
    bool Int64(int64_t i);
    bool Uint64(uint64_t i);
    bool Double(double d);
    bool String(const Ch* str, SizeType length, bool copy);
    bool StartObject();
    bool EndObject(SizeType memberCount);
    bool StartArray();
    bool EndArray(SizeType elementCount);
};
~~~~~~~~~~

`Null()` is called when the `Reader` encounters a JSON null value.

`Bool(bool)` is called when the `Reader` encounters a JSON true or false value.

When the `Reader` encounters a JSON number, it chooses a suitable C++ type mapping. And then it calls *one* function out of `Int(int)`, `Uint(unsigned)`, `Int64(int64_t)`, `Uint64(uint64_t)` and `Double(double)`.

`String(const char* str, SizeType length, bool copy)` is called when the `Reader` encounters a string. The first parameter is pointer to the string. The second parameter is the length of the string (excluding the null terminator). Note that RapidJSON supports null character `'\0'` inside a string. If such situation happens, `strlen(str) < length`. The last `copy` indicates whether the handler needs to make a copy of the string. For normal parsing, `copy = true`. Only when *insitu* parsing is used, `copy = false`. And beware that, the character type depends on the target encoding, which will be explained later.

When the `Reader` encounters the beginning of an object, it calls `StartObject()`. An object in JSON is a set of name-value pairs. If the object contains members it first calls `String()` for the name of member, and then calls functions depending on the type of the value. These calls of name-value pairs repeats until calling `EndObject(SizeType memberCount)`. Note that the `memberCount` parameter is just an aid for the handler, user may not need this parameter.

Array is similar to object but simpler. At the beginning of an array, the `Reader` calls `BeginArary()`. If there is elements, it calls functions according to the types of element. Similarly, in the last call `EndArray(SizeType elementCount)`, the parameter `elementCount` is just an aid for the handler.

Every handler functions returns a `bool`. Normally it should returns `true`. If the handler encounters an error, it can return `false` to notify event publisher to stop further processing.

For example, when we parse a JSON with `Reader` and the handler detected that the JSON does not conform to the required schema, then the handler can return `false` and let the `Reader` stop further parsing. And the `Reader` will be in error state with error code `kParseErrorTermination`.

## GenericReader {#GenericReader}

As mentioned before, `Reader` is a typedef of a template class `GenericReader`:

~~~~~~~~~~cpp
namespace rapidjson {

template <typename SourceEncoding, typename TargetEncoding, typename Allocator = MemoryPoolAllocator<> >
class GenericReader {
    // ...
};

typedef GenericReader<UTF8<>, UTF8<> > Reader;

} // namespace rapidjson
~~~~~~~~~~

The `Reader` uses UTF-8 as both source and target encoding. The source encoding means the encoding in the JSON stream. The target encoding means the encoding of the `str` parameter in `String()` calls. For example, to parse a UTF-8 stream and outputs UTF-16 string events, you can define a reader by:

~~~~~~~~~~cpp
GenericReader<UTF8<>, UTF16<> > reader;
~~~~~~~~~~

Note that, the default character type of `UTF16` is `wchar_t`. So this `reader`needs to call `String(const wchar_t*, SizeType, bool)` of the handler.

The third template parameter `Allocator` is the allocator type for internal data structure (actually a stack).

## Parsing {#Parsing}

The one and only one function of `Reader` is to parse JSON. 

~~~~~~~~~~cpp
template <unsigned parseFlags, typename InputStream, typename Handler>
bool Parse(InputStream& is, Handler& handler);

// with parseFlags = kDefaultParseFlags
template <typename InputStream, typename Handler>
bool Parse(InputStream& is, Handler& handler);
~~~~~~~~~~

If an error occurs during parsing, it will return `false`. User can also calls `bool HasParseEror()`, `ParseErrorCode GetParseErrorCode()` and `size_t GetErrorOffset()` to obtain the error states. Actually `Document` uses these `Reader` functions to obtain parse errors. Please refer to [DOM](doc/dom.md) for details about parse error.

# Writer {#Writer}

## PrettyWriter {#PrettyWriter}

# Techniques {#Techniques}

## Parsing JSON to Custom Data Structure {#CustomDataStructure}

`Document`'s parsing capability is completely based on `Reader`. Actually `Document` is a handler which receives events from a reader to build a DOM during parsing.

User may uses `Reader` to build other data structures directly. This eliminates building of DOM, thus reducing memory and improving performance.

Example:
~~~~~~~~~~cpp
// Note: Ad hoc, not yet tested.
using namespace std;
using namespace rapidjson;

typedef map<string, string> MessageMap;

struct MessageHandler : public GenericBaseHandler<> {
    MessageHandler() : mState(kExpectStart) {
    }

    bool Default() {
        return false;
    }

    bool StartObject() {
        if (!kBeforeStart)
            return false;
        mState = mExpectName;
        return true;
    }

    bool String(const Ch* str, SizeType length, bool copy) {
        if (mState == kExpectName) {
            name_ = string(str, length);
            return true;
        }
        else if (mState == kExpectValue) {
            messages_.insert(MessageMap::value_type(name_, string(str, length)));
            return true;
        }
        else
            return false;
    }

    bool EndObject() {
        return mState == kExpectName;
    }

    MessageMap messages_;
    enum State {
        kExpectObjectStart,
        kExpectName,
        kExpectValue,
    }mState;
    std::string name_;
};

void ParseMessages(const char* json, MessageMap& messages) {
    Reader reader;
    MessageHandler handler;
    StringStream ss(json);
    if (reader.Parse(ss, handler))
        messages.swap(handler.messages_);
}

main() {
    MessageMap messages;
    ParseMessages("{ \"greeting\" : \"Hello!\", \"farewell\" : \"bye-bye!\" }", messages);
}
~~~~~~~~~~

~~~~~~~~~~cpp
// Parse a NxM array 
const char* json = "[3, 4, [1, 2, 3, 4], [5, 6, 7, 8], [9, 10, 11, 12]]"
~~~~~~~~~~

## Filtering of JSON {#Filtering}

