# FAQ

## General

1. What is RapidJSON?

   RapidJSON is a C++ library for parsing and generating JSON. You may check all [features](doc/features.md) of it.

2. Why is RapidJSON named so?

   It is inspired by [RapidXML](http://rapidxml.sourceforge.net/), which is a fast XML DOM parser.

3. Is RapidJSON similar to RapidXML?

   RapidJSON borrowed some designs of RapidXML, including *in situ* parsing, header-only library. But the two APIs are completely different. Also RapidJSON provide many features that are not in RapidXML.

4. Is RapidJSON free?

   Yes, it is free under MIT license. It can be used in commercial applications. Please check the details in [license.txt](https://github.com/miloyip/rapidjson/blob/master/license.txt).

5. Is RapidJSON small? What are its dependencies? 

   Yes. A simple executable which parses a JSON and prints its statistics is less than 30KB on Windows.

   RapidJSON depends on C++ standard library only.

6. How to install RapidJSON?

   Check [Installation section](http://miloyip.github.io/rapidjson/).

7. Can RapidJSON run on my platform?

   RapidJSON has been tested in many combinations of operating systems, compilers and CPU architecture by the community. But we cannot ensure that it can be run on your particular platform. Building and running the unit test suite will give you the answer.

8. Does RapidJSON support C++03? C++11?

   RapidJSON was firstly implemented for C++03. Later it added optional support of some C++11 features (e.g., move constructor, `noexcept`). RapidJSON shall be compatible with C++03 or C++11 compliant compilers.

9. Does RapidJSON really work in real applications?

   Yes. It is deployed in both client and server real applications. A community member reported that RapidJSON in their system parses 50 million JSONs daily.

10. How RapidJSON is tested?

   RapidJSON contains a unit test suite for automatic testing. [Travis](https://travis-ci.org/miloyip/rapidjson/) will compile and run the unit test suite for all modifications. The test process also uses Valgrind to detect memory leaks.

11. Is RapidJSON well documented?

   RapidJSON provides [user guide and API documentationn](http://miloyip.github.io/rapidjson/index.html).

12. Are there alternatives?

   Yes, there are a lot alternatives. For example, [nativejson-benchmark](https://github.com/miloyip/nativejson-benchmark) has a listing of open-source C/C++ JSON libraries. [json.org](http://www.json.org/) also has a list.

## JSON

1. What is JSON?

   JSON (JavaScript Object Notation) is a lightweight data-interchange format. It uses human readable text format. More details of JSON can be referred to [RFC7159](http://www.ietf.org/rfc/rfc7159.txt) and [ECMA-404](http://www.ecma-international.org/publications/standards/Ecma-404.htm).

2. What are applications of JSON?

   JSON are commonly used in web applications for transferring structured data. It is also used as a file format for data persistence.

2. Does RapidJSON conform to the JSON standard?

   Yes. RapidJSON is fully compliance with [RFC7159](http://www.ietf.org/rfc/rfc7159.txt) and [ECMA-404](http://www.ecma-international.org/publications/standards/Ecma-404.htm). It can handle corner cases, such as supporting null character and surrogate pairs in JSON strings.

3. Does RapidJSON support relaxed syntax?

   Currently no. RapidJSON only support the strict standardized format. Support on related syntax is under discussion in this [issue](https://github.com/miloyip/rapidjson/issues/36).

## DOM and SAX

1. What is DOM style API?

   Document Object Model (DOM) is an in-memory representation of JSON for query and manipulation.

2. What is SAX style API?

   SAX is an event-driven API for parsing and generation.

3. Should I choose DOM or SAX?

   DOM is easy for query and manipulation. SAX is very fast and memory-saving but often more difficult to be applied.

4. What is *in situ* parsing?

   *in situ* parsing decodes the JSON strings directly into the input JSON. This is an optimization which can reduce memory consumption and improve performance, but the input JSON will be modified. Check [in-situ parsing](http://miloyip.github.io/rapidjson/md_doc_dom.html#InSituParsing) for details.

5. When does parsing generate an error?

   The parser generates an error when the input JSON contains invalid syntax, or a value can not be represented (a number is too big), or the handler of parsers terminate the parsing. Check [parse error](http://miloyip.github.io/rapidjson/md_doc_dom.html#ParseError) for details.

6. What error information is provided? 

   The error is stored in `ParseResult`, which includes the error code and offset (number of characters from the beginning of JSON). The error code can be translated into human-readable error message.

7. Why not just using `double` to represent JSON number?

   Some applications use 64-bit unsigned/signed integers. And these integers cannot be converted into `double` without loss of precision. So the parsers detects whether a JSON number is convertible to different types of integers and/or `double`.

## Document/Value (DOM)

1. What is move semantics? Why?
2. How to copy a value?
3. Why do I need to provide the length of string?
4. Why do I need to provide allocator in many DOM manipulation API?
5. Does it convert between numerical types?

## Reader/Writer (SAX)

1. Why not just `printf` a JSON? Why need a `Writer`? 
2. Why can't I parse a JSON which is just a number?
3. Can I pause the parsing process and resume it later?

## Unicode

1. Does it support UTF-8, UTF-16 and other format?
2. Can it validate the encoding?
3. What is surrogate pair? Does RapidJSON support it?
4. Can it handle '\u0000' (null character) in JSON string?
5. Can I output '\uxxxx' for all non-ASCII character?

## Stream

1. I have a big JSON file. Should I load the whole file to memory?
2. Can I parse JSON while it is streamed from network?
3. I don't know what format will the JSON be. How to handle them?
4. What is BOM? How RapidJSON handle it?
5. Why little/big endian is related?

## Performance

1. Is RapidJSON really fast?
2. Why is it fast?
3. What is SIMD? How it is applied in RapidJSON?
4. Does it consume a lot of memory?
5. What is the purpose of being high performance?

## Gossip

1. Who are the developers of RapidJSON?
2. Why do you develop RapidJSON?
3. Why there is a long empty period of development?
4. Why did the repository move from Google Code to GitHub?
