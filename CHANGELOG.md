# Change Log
All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](http://semver.org/).

## [Unreleased]

* Include rapidjson.h for all internal/error headers.

## [1.0.1] - 2015-04-25

### Added
* Changelog following [Keep a CHANGELOG](https://github.com/olivierlacan/keep-a-changelog) suggestions.

### Fixed
* Parsing of some numbers (e.g. "1e-00011111111111") causing assertion (#314).
* Visual C++ 32-bit compilation error in `diyfp.h` (#317).

## [1.0.0] - 2015-04-22

### Added
* 100% [Coverall](https://coveralls.io/r/miloyip/rapidjson?branch=master) coverage.
* Version macros (#311)

### Fixed
* A bug in trimming long number sequence (4824f12efbf01af72b8cb6fc96fae7b097b73015).
* Double quote in unicode escape (#288).
* Negative zero roundtrip (double only) (#289).
* Standardize behavior of `memcpy()` and `malloc()` (0c5c1538dcfc7f160e5a4aa208ddf092c787be5a, #305, 0e8bbe5e3ef375e7f052f556878be0bd79e9062d).

### Removed
* Remove an invalid `Document::ParseInsitu()` API (e7f1c6dd08b522cfcf9aed58a333bd9a0c0ccbeb).

## 1.0-beta - 2015-04-8

### Added
* RFC 7159 (#101)
* Optional Iterative Parser (#76)
* Deep-copy values (#20)
* Error code and message (#27)
* ASCII Encoding (#70)
* `kParseStopWhenDoneFlag` (#83)
* `kParseFullPrecisionFlag` (881c91d696f06b7f302af6d04ec14dd08db66ceb)
* Add `Key()` to handler concept (#134)
* C++11 compatibility and support (#128)
* Optimized number-to-string and vice versa conversions (#137, #80)
* Short-String Optimization (#131)
* Local stream optimization by traits (#32)
* Travis & Appveyor Continuous Integration, with Valgrind verification (#24, #242)
* Redo all documentation (English, Simplified Chinese)

### Changed
* Copyright ownership transfered to THL A29 Limited (a Tencent company).
* Migrating from Premake to CMAKE (#192)
* Resolve all warning reports

### Removed
* Remove other JSON libraries for performance comparison (#180)

## 0.11 - 2012-11-16

## 0.1 - 2011-11-18

[Unreleased]: https://github.com/miloyip/rapidjson/compare/v1.0.1...HEAD
[1.0.1]: https://github.com/miloyip/rapidjson/compare/v1.0.0...v1.0.1
[1.0.0]: https://github.com/miloyip/rapidjson/compare/v1.0-beta...v1.0.0
