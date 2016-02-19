# Schema

## Status: experimental, shall be included in v1.1

JSON Schema is a draft standard for describing the format of JSON data. The schema itself is also JSON data. By validating a JSON structure with JSON Schema, your code can safely access the DOM without manually checking types, or whether a key exists, etc. It can also ensure that the serialized JSON conform to a specified schema.

RapidJSON implemented a JSON Schema validator for [JSON Schema Draft v4](http://json-schema.org/documentation.html). If you are not familiar with JSON Schema, you may refer to [Understanding JSON Schema](http://spacetelescope.github.io/understanding-json-schema/).

[TOC]

## Basic Usage

First of all, you need to parse a JSON Schema into `Document`, and then compile the `Document` into a `SchemaDocument`.

Secondly, construct a `SchemaValidator` with the `SchemaDocument`. It is similar to a `Writer` in the sense of handling SAX events. So, you can use `document.Accept(validator)` to validate a document, and then check the validity.

~~~cpp
#include "rapidjson/schema.h"

// ...

Document sd;
if (!sd.Parse(schemaJson)) {
    // the schema is not a valid JSON.
    // ...       
}
SchemaDocument schema(sd); // Compile a Document to SchemaDocument
// sd is no longer needed here.

Document d;
if (!d.Parse(inputJson)) {
    // the input is not a valid JSON.
    // ...       
}

SchemaValidator validator(schema);
if (!d.Accept(validator)) {
    // Input JSON is invalid according to the schema
    // Output diagnostic information
    StringBuffer sb;
    validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);
    printf("Invalid schema: %s\n", sb.GetString());
    printf("Invalid keyword: %s\n", validator.GetInvalidSchemaKeyword());
    sb.Clear();
    validator.GetInvalidDocumentPointer().StringifyUriFragment(sb);
    printf("Invalid document: %s\n", sb.GetString());
}
~~~

Some notes:

* One `SchemaDocment` can be referenced by multiple `SchemaValidator`s. It will not be modified by `SchemaValidator`s.
* A `SchemaValidator` may be reused to validate multiple documents. To run it for other documents, call `validator.Reset()` first.

## Validation during parsing/serialization

Unlike most JSON Schema validator implementations, RapidJSON provides a SAX-based schema validator. Therefore, you can parse a JSON from a stream while validating it on the fly. If the validator encounters a JSON value that invalidates the supplied schema, the parsing will be terminated immediately. This design is especially useful for parsing large JSON files.

### DOM parsing

For using DOM in parsing, `Document` needs some preparation and finalizing tasks, in addition to receiving SAX events, thus it needs some work to route the reader, validator and the document. `SchemaValidatingReader` is a helper class that doing such work.

~~~cpp
#include "rapidjson/filereadstream.h"

// ...
SchemaDocument schema(sd); // Compile a Document to SchemaDocument

// Use reader to parse the JSON
FILE* fp = fopen("big.json", "r");
FileReadStream is(fp, buffer, sizeof(buffer));

// Parse JSON from reader, validate the SAX events, and store in d.
Document d;
SchemaValidatingReader<kParseDefaultFlags, FileReadStream, UTF8<> > reader(is, schema);
d.Populate(reader);

if (!reader.GetParseResult()) {
    // Not a valid JSON
    // When reader.GetParseResult().Code() == kParseErrorTermination,
    // it may be terminated by:
    // (1) the validator found that the JSON is invalid according to schema; or
    // (2) the input stream has I/O error.

    // Check the validation result
    if (!reader.IsValid()) {
        // Input JSON is invalid according to the schema
        // Output diagnostic information
        StringBuffer sb;
        reader.GetInvalidSchemaPointer().StringifyUriFragment(sb);
        printf("Invalid schema: %s\n", sb.GetString());
        printf("Invalid keyword: %s\n", reader.GetInvalidSchemaKeyword());
        sb.Clear();
        reader.GetInvalidDocumentPointer().StringifyUriFragment(sb);
        printf("Invalid document: %s\n", sb.GetString());
    }
}
~~~

### SAX parsing

For using SAX in parsing, it is much simpler. If it only need to validate the JSON without further processing, it is simply:

~~~
SchemaValidator validator(schema);
Reader reader;
if (!reader.Parse(stream, validator)) {
    if (!validator.IsValid()) {
        // ...    
    }
}
~~~

This is exactly the method used in the [schemavalidator](example/schemavalidator/schemavalidator.cpp) example. The distinct advantage is low memory usage, no matter how big the JSON was (the memory usage depends on the complexity of the schema).

If you need to handle the SAX events further, then you need to use the template class `GenericSchemaValidator` to set the output handler of the validator:

~~~
MyHandler handler;
GenericSchemaValidator<SchemaDocument, MyHandler> validator(schema, handler);
Reader reader;
if (!reader.Parse(ss, validator)) {
    if (!validator.IsValid()) {
        // ...    
    }
}
~~~

### Serialization

It is also possible to do validation during serializing. This can ensure the result JSON is valid according to the JSON schema.

~~~
StringBuffer sb;
Writer<StringBuffer> writer(sb);
GenericSchemaValidator<SchemaDocument, Writer<StringBuffer> > validator(s, writer);
if (!d.Accept(validator)) {
    // Some problem during Accept(), it may be validation or encoding issues.
    if (!validator.IsValid()) {
        // ...
    }
}
~~~

Of course, if your application only needs SAX-style serialization, it can simply send SAX events to `SchemaValidator` instead of `Writer`.

## Remote Schema

JSON Schema supports [`$ref` keyword](http://spacetelescope.github.io/understanding-json-schema/structuring.html), which is a [JSON pointer](pointer.md) referencing to a local or remote schema. Local pointer is prefixed with `#`, while remote pointer is an relative or absolute URI. For example:

~~~js
{ "$ref": "definitions.json#/address" }
~~~

As `SchemaValidator` does not know how to resolve such URI, it needs a user-provided `IRemoteSchemaDocumentProvider` instance to do so.

~~~
class MyRemoteSchemaDocumentProvider : public IRemoteSchemaDocumentProvider {
public:
    virtual const SchemaDocument* GetRemoteDocument(const char* uri, SizeTyp length) {
        // Resolve the uri and returns a pointer to that schema.
    }
};

// ...

MyRemoteSchemaDocumentProvider provider;
SchemaValidator validator(schema, &provider);
~~~

## Conformance

RapidJSON passed 262 out of 263 tests in [JSON Schema Test Suite](https://github.com/json-schema/JSON-Schema-Test-Suite) (Json Schema draft 4).

The failed test is "changed scope ref invalid" of "change resolution scope" in `refRemote.json`. It is due to that `id` schema keyword and URI combining function are not implemented.

Besides, the `format` schema keyword for string values is ignored, since it is not required by the specification.

### Regular Expression

The schema keyword `pattern` and `patternProperties` uses regular expression to match the required pattern.

RapidJSON implemented a simple NFA regular expression engine, which is used by default. It supports the following syntax.

|Syntax|Description|
|------|-----------|
|`ab`    | Concatenation
|`a|b`   | Alternation
|`a?`    | Zero or one
|`a*`    | Zero or more
|`a+`    | One or more
|`a{3}`  | Exactly 3 times
|`a{3,}` | At least 3 times
|`a{3,5}`| 3 to 5 times
|`(ab)`  | Grouping
|`^a`    | At the beginning
|`a$`    | At the end
|`.`     | Any character
|`[abc]` | Character classes
|`[a-c]` | Character class range
|`[a-z0-9_]` | Character class combination
|`[^abc]` | Negated character classes
|`[^a-c]` | Negated character class range
|`[\b]`   | Backspace (U+0008)
|`\|`, `\\`, ...  | Escape characters
|`\f` | Form feed (U+000C)
|`\n` | Line feed (U+000A)
|`\r` | Carriage return (U+000D)
|`\t` | Tab (U+0009)
|`\v` | Vertical tab (U+000B)

For C++11 compiler, it is also possible to use the `std::regex` by defining `RAPIDJSON_SCHEMA_USE_INTERNALREGEX=0` and `RAPIDJSON_SCHEMA_USE_STDREGEX=1`. If your schemas do not need `pattern` and `patternProperties`, you can set both macros to zero to disable this feature, which will reduce some code size.

## Performance

Most C++ JSON libraries do not yet support JSON Schema. So we tried to evaluate the performance of RapidJSON's JSON Schema validator according to [json-schema-benchmark](https://github.com/ebdrup/json-schema-benchmark), which tests 11 JavaScript libraries running on Node.js.

That benchmark runs validations on [JSON Schema Test Suite](https://github.com/json-schema/JSON-Schema-Test-Suite), in which some test suites and tests are excluded. We made the same benchmarking procedure in [`schematest.cpp`](test/perftest/schematest.cpp).

On a Mac Book Pro (2.8 GHz Intel Core i7), the following results are collected.

|Validator|Relative speed|Number of test runs per second|
|---------|:------------:|:----------------------------:|
|RapidJSON|155%|30682|
|[`ajv`](https://github.com/epoberezkin/ajv)|100%|19770 (± 1.31%)|
|[`is-my-json-valid`](https://github.com/mafintosh/is-my-json-valid)|70%|13835 (± 2.84%)|
|[`jsen`](https://github.com/bugventure/jsen)|57.7%|11411 (± 1.27%)|
|[`schemasaurus`](https://github.com/AlexeyGrishin/schemasaurus)|26%|5145 (± 1.62%)|
|[`themis`](https://github.com/playlyfe/themis)|19.9%|3935 (± 2.69%)|
|[`z-schema`](https://github.com/zaggino/z-schema)|7%|1388 (± 0.84%)|
|[`jsck`](https://github.com/pandastrike/jsck#readme)|3.1%|606 (± 2.84%)|
|[`jsonschema`](https://github.com/tdegrunt/jsonschema#readme)|0.9%|185 (± 1.01%)|
|[`skeemas`](https://github.com/Prestaul/skeemas#readme)|0.8%|154 (± 0.79%)|
|tv4|0.5%|93 (± 0.94%)|
|[`jayschema`](https://github.com/natesilva/jayschema)|0.1%|21 (± 1.14%)|

That is, RapidJSON is about 1.5x faster than the fastest JavaScript library (ajv). And 1400x faster than the slowest one.
