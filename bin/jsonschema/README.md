JSON Schema Test Suite [![Build Status](https://travis-ci.org/json-schema/JSON-Schema-Test-Suite.png?branch=develop)](https://travis-ci.org/json-schema/JSON-Schema-Test-Suite)
======================

This repository contains a set of JSON objects that implementors of JSON Schema
validation libraries can use to test their validators.

It is meant to be language agnostic and should require only a JSON parser.

The conversion of the JSON objects into tests within your test framework of
choice is still the job of the validator implementor.

Structure of a Test
-------------------

If you're going to use this suite, you need to know how tests are laid out. The
tests are contained in the `tests` directory at the root of this repository.

Inside that directory is a subdirectory for each draft or version of the
schema. We'll use `draft3` as an example.

If you look inside the draft directory, there are a number of `.json` files,
which logically group a set of test cases together. Often the grouping is by
property under test, but not always, especially within optional test files
(discussed below).

Inside each `.json` file is a single array containing objects. It's easiest to
illustrate the structure of these with an example:

```json
    {
        "description": "the description of the test case",
        "schema": {"the schema that should" : "be validated against"},
        "tests": [
            {
                "description": "a specific test of a valid instance",
                "data": "the instance",
                "valid": true
            },
            {
                "description": "another specific test this time, invalid",
                "data": 15,
                "valid": false
            }
        ]
    }
```

So a description, a schema, and some tests, where tests is an array containing
one or more objects with descriptions, data, and a boolean indicating whether
they should be valid or invalid.

Coverage
--------

Draft 3 and 4 should have full coverage. If you see anything missing or think
there is a useful test missing, please send a pull request or open an issue.

Who Uses the Test Suite
-----------------------

This suite is being used by:

  * [json-schema-validator (Java)](https://github.com/fge/json-schema-validator)
  * [jsonschema (python)](https://github.com/Julian/jsonschema)
  * [aeson-schema (haskell)](https://github.com/timjb/aeson-schema)
  * [direct-schema (javascript)](https://github.com/IreneKnapp/direct-schema)
  * [jsonschema (javascript)](https://github.com/tdegrunt/jsonschema)
  * [JaySchema (javascript)](https://github.com/natesilva/jayschema)
  * [z-schema (javascript)](https://github.com/zaggino/z-schema)
  * [jassi (javascript)](https://github.com/iclanzan/jassi)
  * [json-schema-valid (javascript)](https://github.com/ericgj/json-schema-valid)
  * [jesse (Erlang)](https://github.com/klarna/jesse)
  * [json-schema (PHP)](https://github.com/justinrainbow/json-schema)
  * [gojsonschema (Go)](https://github.com/sigu-399/gojsonschema) 
  * [json_schema (Dart)](https://github.com/patefacio/json_schema) 
  * [tv4 (JavaScript)](https://github.com/geraintluff/tv4)
  * [Jsonary (JavaScript)](https://github.com/jsonary-js/jsonary)

If you use it as well, please fork and send a pull request adding yourself to
the list :).

Contributing
------------

If you see something missing or incorrect, a pull request is most welcome!

There are some sanity checks in place for testing the test suite. You can run
them with `bin/jsonschema_suite check`. They will be run automatically by
[Travis CI](https://travis-ci.org/) as well.
