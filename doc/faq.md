# FAQ

## General

1. What is RapidJSON?

   RapidJSON is a C++ library for parsing and generating JSON. You may check all [features](features.md) of it.

2. Why is it named so?

   It is inspired by [RapidXML](http://rapidxml.sourceforge.net/), which is a fast XML DOM parser. RapidJSON borrowed some designs of RapidXML, including *in situ* parsing, header-only library, and more.

3. Is it similar to RapidXML?
4. Is it free?
5. Is it small? What are its dependencies? 
6. How to install RapidJSON?
7. Can it run on my platform?
8. Does it support C++03? C++11?
9. Does it really work in real applications?
10. How it is tested?
11. Is it well documented?
12. Are there alternatives?

## JSON

1. What is JSON?
2. What is application of JSON?
2. Does RapidJSON conform to the JSON standard?
3. Does RapidJSON support relaxed syntax?

## DOM and SAX

1. What is DOM style API?
2. What is SAX style API?
3. Should I choose DOM or SAX?
4. What is *in situ* parsing?
5. When will parsing generates an error?
6. What error information is provided? 
7. Why not just using `double` to represent JSON number?

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
