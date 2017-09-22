# Schema violation reporting

(Unreleased as of 2017-09-20)

When validating an instance against a JSON Schema,
it is often desirable to report not only whether the instance is valid,
but also the ways in which it violates the schema.

The `SchemaValidator` class
collects errors encountered during validation
into a JSON `Value`.
This error object can then be accessed as `validator.GetError()`.

The structure of the error object is subject to change
in future versions of RapidJSON,
as there is no standard schema for violations.
The details below this point are provisional only.

[TOC]

# General provisions  {#General}

Validation of an instance value against a schema
produces an error value.
The error value is always an object.
An empty object `{}` indicates the instance is valid.

* The name of each member
  corresponds to the JSON Schema keyword that is violated.
* The value is either an object describing a single violation,
  or an array of such objects.

Each violation object contains two string-valued members
named `instanceRef` and `schemaRef`.
`instanceRef` contains the URI fragment serialization
of a JSON Pointer to the instance subobject
in which the violation was detected.
`schemaRef` contains the URI of the schema
and the fragment serialization of a JSON Pointer
to the subschema that was violated.

Individual violation objects can contain other keyword-specific members.
These are detailed further.

For example, validating this instance:

~~~json
{"numbers": [1, 2, "3", 4, 5]}
~~~

against this schema:

~~~json
{
  "type": "object",
  "properties": {
    "numbers": {"$ref": "numbers.schema.json"}
  }
}
~~~

where `numbers.schema.json` refers
(via a suitable `IRemoteSchemaDocumentProvider`)
to this schema:

~~~json
{
  "type": "array",
  "items": {"type": "number"}
}
~~~

produces the following error object:

~~~json
{
  "type": {
    "instanceRef": "#/numbers/2",
    "schemaRef": "numbers.schema.json#/items",
    "expected": ["number"],
    "actual": "string"
  }
}
~~~

# Validation keywords for numbers {#numbers}

## multipleOf {#multipleOf}

* `expected`: required number strictly greater than 0.
  The value of the `multipleOf` keyword specified in the schema.
* `actual`: required number.
  The instance value.

## maximum {#maximum}

* `expected`: required number.
  The value of the `maximum` keyword specified in the schema.
* `exclusiveMaximum`: optional boolean.
  This will be true if the schema specified `"exclusiveMaximum": true`,
  and will be omitted otherwise.
* `actual`: required number.
  The instance value.

## minimum {#minimum}

* `expected`: required number.
  The value of the `minimum` keyword specified in the schema.
* `exclusiveMinimum`: optional boolean.
  This will be true if the schema specified `"exclusiveMinimum": true`,
  and will be omitted otherwise.
* `actual`: required number.
  The instance value.

# Validation keywords for strings {#strings}

## maxLength {#maxLength}

* `expected`: required number greater than or equal to 0.
  The value of the `maxLength` keyword specified in the schema.
* `actual`: required string.
  The instance value.

## minLength {#minLength}

* `expected`: required number greater than or equal to 0.
  The value of the `minLength` keyword specified in the schema.
* `actual`: required string.
  The instance value.

## pattern {#pattern}

* `actual`: required string.
  The instance value.

(The expected pattern is not reported
because the internal representation in `SchemaDocument`
does not store the pattern in original string form.)

# Validation keywords for arrays {#arrays}

## additionalItems {#additionalItems}

This keyword is reported
when the value of `items` schema keyword is an array,
the value of `additionalItems` is `false`,
and the instance is an array
with more items than specified in the `items` array.

* `disallowed`: required integer greater than or equal to 0.
  The index of the first item that has no corresponding schema.

## maxItems and minItems {#maxItems}

* `expected`: required integer greater than or equal to 0.
  The value of `maxItems` (respectively, `minItems`)
  specified in the schema.
* `actual`: required integer greater than or equal to 0.
  Number of items in the instance array.

## uniqueItems {#uniqueItems}

* `duplicates`: required array
  whose items are integers greater than or equal to 0.
  Indices of items of the instance that are equal.

(RapidJSON only reports the first two equal items,
for performance reasons.)

# Validation keywords for objects

## maxProperties and minProperties {#maxProperties}

* `expected`: required integer greater than or equal to 0.
  The value of `maxProperties` (respectively, `minProperties`)
  specified in the schema.
* `actual`: required integer greater than or equal to 0.
  Number of properties in the instance object.

## required {#required}

* `missing`: required array of one or more unique strings.
  The names of properties
  that are listed in the value of the `required` schema keyword
  but not present in the instance object.

## additionalProperties {#additionalProperties}

This keyword is reported
when the schema specifies `additionalProperties: false`
and the name of a property of the instance is
neither listed in the `properties` keyword
nor matches any regular expression in the `patternProperties` keyword.

* `disallowed`: required string.
  Name of the offending property of the instance.

(For performance reasons,
RapidJSON only reports the first such property encountered.)

## dependencies {#dependencies}

* `errors`: required object with one or more properties.
  Names and values of its properties are described below.

Recall that JSON Schema Draft 04 supports
*schema dependencies*,
where presence of a named *controlling* property
requires the instance object to be valid against a subschema,
and *property dependencies*,
where presence of a controlling property
requires other *dependent* properties to be also present.

For a violated schema dependency,
`errors` will contain a property
with the name of the controlling property
and its value will be the error object
produced by validating the instance object
against the dependent schema.

For a violated property dependency,
`errors` will contain a property
with the name of the controlling property
and its value will be an array of one or more unique strings
listing the missing dependent properties.

# Validation keywords for any instance type {#anytypes}

## enum {#enum}

This keyword has no additional properties
beyond `instanceRef` and `schemaRef`.

* The allowed values are not listed
  because `SchemaDocument` does not store them in original form.
* The violating value is not reported
  because it might be unwieldy.

If you need to report these details to your users,
you can access the necessary information
by following `instanceRef` and `schemaRef`.

## type {#type}

* `expected`: required array of one or more unique strings,
  each of which is one of the seven primitive types
  defined by the JSON Schema Draft 04 Core specification.
  Lists the types allowed by the `type` schema keyword.
* `actual`: required string, also one of seven primitive types.
  The primitive type of the instance.

## allOf, anyOf, and oneOf {#allOf}

* `errors`: required array of at least one object.
  There will be as many items as there are subschemas
  in the `allOf`, `anyOf` or `oneOf` schema keyword, respectively.
  Each item will be the error value
  produced by validating the instance
  against the corresponding subschema.

For `allOf`, at least one error value will be non-empty.
For `anyOf`, all error values will be non-empty.
For `oneOf`, either all error values will be non-empty,
or more than one will be empty.

## not {#not}

This keyword has no additional properties
apart from `instanceRef` and `schemaRef`.
