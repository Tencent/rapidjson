# Rapidjson

Copyright (c) 2011-2014 Milo Yip (miloyip@gmail.com)

https://github.com/miloyip/rapidjson/

## Introduction

Rapidjson is a JSON parser and generator for C++. It was inspired by [rapidxml](http://rapidxml.sourceforge.net/).

* Rapidjson is small but complete. It supports both SAX and DOM style API. The SAX parser is only a half thousand lines of code.

* Rapidjson is fast. Its performance can be comparable to `strlen()`. It also optionally supports SSE2/SSE4.1 for acceleration.

* Rapidjson is self-contained. It does not depend on external libraries such as BOOST. It even does not depend on STL.

* Rapidjson is memory friendly. Each JSON value occupies exactly 16/20 bytes for most 32/64-bit machines (excluding text string). By default it uses a fast memory allocator, and the parser allocates memory compactly during parsing.

For the full features please refer to the user guide.

JSON(JavaScript Object Notation) is a light-weight data exchange format. Rapidjson should be in fully compliance with RFC4627/ECMA-404. More information about JSON can be obtained at
* [Introducing JSON](http://json.org/)
* [RFC4627: The application/json Media Type for JavaScript Object Notation (JSON)](http://www.ietf.org/rfc/rfc4627.txt)
* [Standard ECMA-404: The JSON Data Interchange Format](http://www.ecma-international.org/publications/standards/Ecma-404.htm)

## Compatibility

Rapidjson is cross-platform. Some platform/compiler combinations which have been tested are shown as follows.
* Visual C++ 2008/2010/2013 on Windows (32/64-bit)
* GNU C++ 3.8.x on Cygwin
* Clang 3.4 on Mac OS X (32/64-bit) and iOS
* Clang 3.4 on Android NDK

Users can build and run the unit tests on their platform/compiler.

## Installation

Rapidjson is a header-only C++ library. Just copy the `rapidjson/include/rapidjson` folder to system or project's include path.

To build the tests and examples:

1. Obtain [premake4] (http://industriousone.com/premake/download).
2. Copy premake4 executable to rapidjson/build (or system path)
3. Run `rapidjson/build/premake.bat` on Windows, `rapidjson/build/premake.sh` on Linux or other platforms
4. On Windows, build the solution at `rapidjson/build/vs2008/` or `/vs2010/`
5. On other platforms, run GNU make at `rapidjson/build/gmake/` (e.g., `make -f test.make config=release32`, `make -f example.make config=debug32`)
6. On success, the executable are generated at `rapidjson/bin`
