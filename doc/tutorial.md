# RapidJSON Tutorial

This tutorial introduces Document Object Model(DOM) API of RapidJSON.

As shown in [Usage at a glance](../readme.md#usage-at-a-glance), a JSON text can be parsed into DOM, and then be quried and modfied easily, and finally convert back to JSON text.

## Value

Each JSON value is stored in a type called `Value`. A `Document`, representing the DOM, contains the root of `Value`.

### Querying Value

In this section, we will use excerpt of [`example/tutorial/tutorial.cpp`](../example/tutorial/tutorial.cpp).

Assumes we have a JSON text stored in a C string (`const char* json`):
```js
{
    "hello": "world",
    "t": true ,
    "f": false,
    "n": null,
    "i": 123,
    "pi": 3.1416,
    "a": [1, 2, 3, 4]
}
```

Parse it into a `Document`
```cpp
#include "rapidjson/document.h"

using namespace rapidjson;

// ...
Document document;
document.Parse(json);
```

The JSON text is now parsed into `document` as a DOM tree.

The root of a conforming JSON should be either an object or an array. In this case, the root is an object with 7 members.
```cpp
assert(document.IsObject());
```

Query whether a `"hello"` member exists in the root object. Since a `Value` can contain different types of value, we may need to verify its type and use suitable API to obtain the value. In this example, `"hello"` member associates with a JSON string.
```cpp
assert(document.HasMember("hello"));
assert(document["hello"].IsString());
printf("hello = %s\n", document["hello"].GetString());
```

JSON true/false values are represented as `bool`.
```cpp
assert(document["t"].IsBool());
printf("t = %s\n", document["t"].GetBool() ? "true" : "false");
```

JSON null can be queryed by `IsNull()`.
```cpp
printf("n = %s\n", document["n"].IsNull() ? "null" : "?");
```

JSON number type represents all numeric values. However, C++ needs more specific type for manipulation.

```cpp
assert(document["i"].IsNumber());

// In this case, IsUint()/IsInt64()/IsUInt64() also return true.
assert(document["i"].IsInt());          
printf("i = %d\n", document["i"].GetInt());
// Alternative (int)document["i"]

assert(document["pi"].IsNumber());
assert(document["pi"].IsDouble());
printf("pi = %g\n", document["pi"].GetDouble());
```

JSON array contains a number of elements
```cpp
// Using a reference for consecutive access is handy and faster.
const Value& a = document["a"];
assert(a.IsArray());
for (SizeType i = 0; i < a.Size(); i++) // Uses SizeType instead of size_t
        printf("a[%d] = %d\n", i, a[i].GetInt());
```

Note that, RapidJSON do not automatically converting between JSON types. if a value is a string, it is invalid to call `GetInt()`.  In debug mode it will assert. In release mode, the behavior is undefined.

## Create/Modify Values

## Object

## Array

## String

## Number

## True/False/Null
