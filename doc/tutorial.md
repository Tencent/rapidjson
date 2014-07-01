# RapidJSON Tutorial

This tutorial introduces the basics of the Document Object Model(DOM) API.

As shown in [Usage at a glance](../readme.md#usage-at-a-glance), a JSON text can be parsed into DOM, and then the DOM can be queried and modfied easily, and finally be converted back to JSON text.

## Value & Document

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

The JSON text is now parsed into `document` as a DOM tree:

![tutorial](diagram/tutorial.png?raw=true)

The root of a conforming JSON should be either an object or an array. In this case, the root is an object.
```cpp
assert(document.IsObject());
```

Query whether a `"hello"` member exists in the root object. Since a `Value` can contain different types of value, we may need to verify its type and use suitable API to obtain the value. In this example, `"hello"` member associates with a JSON string.
```cpp
assert(document.HasMember("hello"));
assert(document["hello"].IsString());
printf("hello = %s\n", document["hello"].GetString());
```

```
world
```

JSON true/false values are represented as `bool`.
```cpp
assert(document["t"].IsBool());
printf("t = %s\n", document["t"].GetBool() ? "true" : "false");
```

```
true
```

JSON null can be queryed by `IsNull()`.
```cpp
printf("n = %s\n", document["n"].IsNull() ? "null" : "?");
```

```
null
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

```
i = 123
pi = 3.1416
```

JSON array contains a number of elements.
```cpp
// Using a reference for consecutive access is handy and faster.
const Value& a = document["a"];
assert(a.IsArray());
for (SizeType i = 0; i < a.Size(); i++) // Uses SizeType instead of size_t
        printf("a[%d] = %d\n", i, a[i].GetInt());
```

```
a[0] = 1
a[1] = 2
a[2] = 3
a[3] = 4
```

Note that, RapidJSON does not automatically convert values between JSON types. If a value is a string, it is invalid to call `GetInt()`, for example. In debug mode it will fail an assertion. In release mode, the behavior is undefined.

In the following, details about querying individual types are discussed.

### Querying Array

By default, `SizeType` is typedef of `unsigned`. In most systems, array is limited to store up to 2^32-1 elements.

You may access the elements in array by integer literal, for example, `a[1]`, `a[2]`. However, `a[0]` will generate a compiler error. It is because two overloaded operators `operator[](SizeType)` and `operator[](const char*)` is avaliable, and C++ can treat `0` as a null pointer. Workarounds:
* `a[SizeType(0)]`
* `a[0u]`

Array is similar to `std::vector`, instead of using indices, you may also use iterator to access all the elements.
```cpp
for (Value::ConstValueIterator itr = a.Begin(); itr != a.End(); ++itr)
    printf("%d ", itr->GetInt());
```

And other familar query functions:
* `SizeType Capacity() const`
* `bool Empty() const`

### Quering Object

Similarly, we can iterate object members by iterator:

```cpp
static const char* kTypeNames[] = 
    { "Null", "False", "True", "Object", "Array", "String", "Number" };

for (Value::ConstMemberIterator itr = document.MemberBegin();
    itr != document.MemberEnd(); ++itr)
{
    printf("Type of member %s is %s\n",
        itr->name.GetString(), kTypeNames[itr->value.GetType()]);
}
```

```
Type of member hello is String
Type of member t is True
Type of member f is False
Type of member n is Null
Type of member i is Number
Type of member pi is Number
Type of member a is Array
```

Note that, when `operator[](const char*)` cannot find the member, it will fail an assertion.

If we are unsure whether a member exists, we need to call `HasMember()` before calling `operator[](const char*)`. However, this incurs two lookup. A better way is to call `FindMember()`, which can check the existence of member and obtain its value at once:

```cpp
Value::ConstMemberIerator itr = document.FindMember("hello");
if (itr != 0)
    printf("%s %s\n", itr->value.GetString());
```

### Querying Number

JSON provide a single numerical type called Number. Number can be integer or real numbers. RFC 4627 says the range of Number is specified by parser.

As C++ provides several integer and floating point number types, the DOM trys to handle these with widest possible range and good performance.

When the DOM parses a Number, it stores it as either one of the following type:

Type       | Description
-----------|---------------------------------------
`unsigned` | 32-bit unsigned integer
`int`      | 32-bit signed integer
`uint64_t` | 64-bit unsigned integer
`int64_t`  | 64-bit signed integer
`double`   | 64-bit double precision floating point

When querying a number, you can check whether the number can be obtained as target type:

Checking          | Obtaining
------------------|---------------------
`bool IsNumber()` | N/A
`bool IsUint()`   | `unsigned GetUint()`
`bool IsInt()`    | `int GetInt()`
`bool IsUint64()` | `uint64_t GetUint()`
`bool IsInt64()`  | `int64_t GetInt64()`
`bool IsDouble()` | `double GetDouble()`

Note that, an integer value may be obtained in various ways without conversion. For example, A value `x` containing 123 will make `x.IsInt() == x.IsUint() == x.Int64() == x.Uint64() == ture`. But a value `y` containing -3000000000 will only makes `x.int64() == true`.

When obtaining the numeric values, `GetDouble()` will convert internal integer representation to a `double`. Note that, `int` and `uint` can be safely convert to `double`, but `int64_t` and `uint64_t` may lose precision (since mantissa of `double` is only 52-bits).

### Querying String

In addition to `GetString()`, the `Value` class also contains `GetStringLength()`. Here explains why.

According to RFC 4627, JSON strings can contain unicode character `U+0000`, which must be escaped as `"\u0000"`. The problem is that, C/C++ often uses null-terminated string, which treats ``\0'` as the terminator symbol.

To conform RFC 4627, RapidJSON supports string containing `U+0000`. If you need to handle this, you can use `GetStringLength()` API to obtain the correct length of string.

For example, after parsing a the following JSON string to `Document d`.

```js
{ "s" :  "a\u0000b" }
```
The correct length of the value `"a\u0000b"` is 3. But `strlen()` returns 1.

`GetStringLength()` can also improve performance, as user may often need to call `strlen()` for allocating buffer.

Besides, `std::string` also support a constructor:

```cpp
string( const char* s, size_type count);
```

which accepts the length of string as parameter. This constructor supports storing null character within the string, and should also provide better performance.

## Create/Modify Values

### Object

### Array

### String

