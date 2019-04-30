// Tencent is pleased to support the open source community by making RapidJSON available.
// 
// Copyright (C) 2015 THL A29 Limited, a Tencent company, and Milo Yip. All rights reserved.
//
// Licensed under the MIT License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// http://opensource.org/licenses/MIT
//
// Unless required by applicable law or agreed to in writing, software distributed 
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
// CONDITIONS OF ANY KIND, either express or implied. See the License for the 
// specific language governing permissions and limitations under the License.

#include "unittest.h"
#include "rapidjson/schema.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#ifdef __clang__
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(variadic-macros)
#elif defined(_MSC_VER)
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(4822) // local class member function does not have a body
#endif

using namespace rapidjson;

#define TEST_HASHER(json1, json2, expected) \
{\
    Document d1, d2;\
    d1.Parse(json1);\
    ASSERT_FALSE(d1.HasParseError());\
    d2.Parse(json2);\
    ASSERT_FALSE(d2.HasParseError());\
    internal::Hasher<Value, CrtAllocator> h1, h2;\
    d1.Accept(h1);\
    d2.Accept(h2);\
    ASSERT_TRUE(h1.IsValid());\
    ASSERT_TRUE(h2.IsValid());\
    /*printf("%s: 0x%016llx\n%s: 0x%016llx\n\n", json1, h1.GetHashCode(), json2, h2.GetHashCode());*/\
    EXPECT_TRUE(expected == (h1.GetHashCode() == h2.GetHashCode()));\
}

TEST(SchemaValidator, Hasher) {
    TEST_HASHER("null", "null", true);

    TEST_HASHER("true", "true", true);
    TEST_HASHER("false", "false", true);
    TEST_HASHER("true", "false", false);
    TEST_HASHER("false", "true", false);
    TEST_HASHER("true", "null", false);
    TEST_HASHER("false", "null", false);

    TEST_HASHER("1", "1", true);
    TEST_HASHER("2147483648", "2147483648", true); // 2^31 can only be fit in unsigned
    TEST_HASHER("-2147483649", "-2147483649", true); // -2^31 - 1 can only be fit in int64_t
    TEST_HASHER("2147483648", "2147483648", true); // 2^31 can only be fit in unsigned
    TEST_HASHER("4294967296", "4294967296", true); // 2^32 can only be fit in int64_t
    TEST_HASHER("9223372036854775808", "9223372036854775808", true); // 2^63 can only be fit in uint64_t
    TEST_HASHER("1.5", "1.5", true);
    TEST_HASHER("1", "1.0", true);
    TEST_HASHER("1", "-1", false);
    TEST_HASHER("0.0", "-0.0", false);
    TEST_HASHER("1", "true", false);
    TEST_HASHER("0", "false", false);
    TEST_HASHER("0", "null", false);

    TEST_HASHER("\"\"", "\"\"", true);
    TEST_HASHER("\"\"", "\"\\u0000\"", false);
    TEST_HASHER("\"Hello\"", "\"Hello\"", true);
    TEST_HASHER("\"Hello\"", "\"World\"", false);
    TEST_HASHER("\"Hello\"", "null", false);
    TEST_HASHER("\"Hello\\u0000\"", "\"Hello\"", false);
    TEST_HASHER("\"\"", "null", false);
    TEST_HASHER("\"\"", "true", false);
    TEST_HASHER("\"\"", "false", false);

    TEST_HASHER("[]", "[ ]", true);
    TEST_HASHER("[1, true, false]", "[1, true, false]", true);
    TEST_HASHER("[1, true, false]", "[1, true]", false);
    TEST_HASHER("[1, 2]", "[2, 1]", false);
    TEST_HASHER("[[1], 2]", "[[1, 2]]", false);
    TEST_HASHER("[1, 2]", "[1, [2]]", false);
    TEST_HASHER("[]", "null", false);
    TEST_HASHER("[]", "true", false);
    TEST_HASHER("[]", "false", false);
    TEST_HASHER("[]", "0", false);
    TEST_HASHER("[]", "0.0", false);
    TEST_HASHER("[]", "\"\"", false);

    TEST_HASHER("{}", "{ }", true);
    TEST_HASHER("{\"a\":1}", "{\"a\":1}", true);
    TEST_HASHER("{\"a\":1}", "{\"b\":1}", false);
    TEST_HASHER("{\"a\":1}", "{\"a\":2}", false);
    TEST_HASHER("{\"a\":1, \"b\":2}", "{\"b\":2, \"a\":1}", true); // Member order insensitive
    TEST_HASHER("{}", "null", false);
    TEST_HASHER("{}", "false", false);
    TEST_HASHER("{}", "true", false);
    TEST_HASHER("{}", "0", false);
    TEST_HASHER("{}", "0.0", false);
    TEST_HASHER("{}", "\"\"", false);
}

// Test cases following http://spacetelescope.github.io/understanding-json-schema

#define VALIDATE(schema, json, expected) \
{\
    SchemaValidator validator(schema);\
    Document d;\
    /*printf("\n%s\n", json);*/\
    d.Parse(json);\
    EXPECT_FALSE(d.HasParseError());\
    EXPECT_TRUE(expected == d.Accept(validator));\
    EXPECT_TRUE(expected == validator.IsValid());\
    if ((expected) && !validator.IsValid()) {\
        StringBuffer sb;\
        validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);\
        printf("Invalid schema: %s\n", sb.GetString());\
        printf("Invalid keyword: %s\n", validator.GetInvalidSchemaKeyword());\
        sb.Clear();\
        validator.GetInvalidDocumentPointer().StringifyUriFragment(sb);\
        printf("Invalid document: %s\n", sb.GetString());\
        sb.Clear();\
        Writer<StringBuffer> w(sb);\
        validator.GetError().Accept(w);\
        printf("Validation error: %s\n", sb.GetString());\
    }\
}

#define INVALIDATE(schema, json, invalidSchemaPointer, invalidSchemaKeyword, invalidDocumentPointer, error) \
{\
    INVALIDATE_(schema, json, invalidSchemaPointer, invalidSchemaKeyword, invalidDocumentPointer, error, SchemaValidator, Pointer) \
}

#define INVALIDATE_(schema, json, invalidSchemaPointer, invalidSchemaKeyword, invalidDocumentPointer, error, \
    SchemaValidatorType, PointerType) \
{\
    SchemaValidatorType validator(schema);\
    Document d;\
    /*printf("\n%s\n", json);*/\
    d.Parse(json);\
    EXPECT_FALSE(d.HasParseError());\
    EXPECT_FALSE(d.Accept(validator));\
    EXPECT_FALSE(validator.IsValid());\
    if (validator.GetInvalidSchemaPointer() != PointerType(invalidSchemaPointer)) {\
        StringBuffer sb;\
        validator.GetInvalidSchemaPointer().Stringify(sb);\
        printf("GetInvalidSchemaPointer() Expected: %s Actual: %s\n", invalidSchemaPointer, sb.GetString());\
        ADD_FAILURE();\
    }\
    ASSERT_TRUE(validator.GetInvalidSchemaKeyword() != 0);\
    if (strcmp(validator.GetInvalidSchemaKeyword(), invalidSchemaKeyword) != 0) {\
        printf("GetInvalidSchemaKeyword() Expected: %s Actual %s\n", invalidSchemaKeyword, validator.GetInvalidSchemaKeyword());\
        ADD_FAILURE();\
    }\
    if (validator.GetInvalidDocumentPointer() != PointerType(invalidDocumentPointer)) {\
        StringBuffer sb;\
        validator.GetInvalidDocumentPointer().Stringify(sb);\
        printf("GetInvalidDocumentPointer() Expected: %s Actual: %s\n", invalidDocumentPointer, sb.GetString());\
        ADD_FAILURE();\
    }\
    Document e;\
    e.Parse(error);\
    if (validator.GetError() != e) {\
        StringBuffer sb;\
        Writer<StringBuffer> w(sb);\
        validator.GetError().Accept(w);\
        printf("GetError() Expected: %s Actual: %s\n", error, sb.GetString());\
        ADD_FAILURE();\
    }\
}

TEST(SchemaValidator, Typeless) {
    Document sd;
    sd.Parse("{}");
    SchemaDocument s(sd);
    
    VALIDATE(s, "42", true);
    VALIDATE(s, "\"I'm a string\"", true);
    VALIDATE(s, "{ \"an\": [ \"arbitrarily\", \"nested\" ], \"data\": \"structure\" }", true);
}

TEST(SchemaValidator, MultiType) {
    Document sd;
    sd.Parse("{ \"type\": [\"number\", \"string\"] }");
    SchemaDocument s(sd);

    VALIDATE(s, "42", true);
    VALIDATE(s, "\"Life, the universe, and everything\"", true);
    INVALIDATE(s, "[\"Life\", \"the universe\", \"and everything\"]", "", "type", "",
        "{ \"type\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"string\", \"number\"], \"actual\": \"array\""
        "}}");
}

TEST(SchemaValidator, Enum_Typed) {
    Document sd;
    sd.Parse("{ \"type\": \"string\", \"enum\" : [\"red\", \"amber\", \"green\"] }");
    SchemaDocument s(sd);

    VALIDATE(s, "\"red\"", true);
    INVALIDATE(s, "\"blue\"", "", "enum", "",
        "{ \"enum\": { \"instanceRef\": \"#\", \"schemaRef\": \"#\" }}");
}

TEST(SchemaValidator, Enum_Typless) {
    Document sd;
    sd.Parse("{  \"enum\": [\"red\", \"amber\", \"green\", null, 42] }");
    SchemaDocument s(sd);

    VALIDATE(s, "\"red\"", true);
    VALIDATE(s, "null", true);
    VALIDATE(s, "42", true);
    INVALIDATE(s, "0", "", "enum", "",
        "{ \"enum\": { \"instanceRef\": \"#\", \"schemaRef\": \"#\" }}");
}

TEST(SchemaValidator, Enum_InvalidType) {
    Document sd;
    sd.Parse("{ \"type\": \"string\", \"enum\": [\"red\", \"amber\", \"green\", null] }");
    SchemaDocument s(sd);

    VALIDATE(s, "\"red\"", true);
    INVALIDATE(s, "null", "", "type", "",
        "{ \"type\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"string\"], \"actual\": \"null\""
        "}}");
}

TEST(SchemaValidator, AllOf) {
    {
        Document sd;
        sd.Parse("{\"allOf\": [{ \"type\": \"string\" }, { \"type\": \"string\", \"maxLength\": 5 }]}");
        SchemaDocument s(sd);

        VALIDATE(s, "\"ok\"", true);
        INVALIDATE(s, "\"too long\"", "", "allOf", "",
            "{ \"maxLength\": { "
            "    \"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/1\", "
            "    \"expected\": 5, \"actual\": \"too long\""
            "}}");
    }
    {
        Document sd;
        sd.Parse("{\"allOf\": [{ \"type\": \"string\" }, { \"type\": \"number\" } ] }");
        SchemaDocument s(sd);

        VALIDATE(s, "\"No way\"", false);
        INVALIDATE(s, "-1", "", "allOf", "",
            "{ \"type\": { \"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/0\","
            "    \"expected\": [\"string\"], \"actual\": \"integer\""
            "}}");
    }
}

TEST(SchemaValidator, AnyOf) {
    Document sd;
    sd.Parse("{\"anyOf\": [{ \"type\": \"string\" }, { \"type\": \"number\" } ] }");
    SchemaDocument s(sd);

    VALIDATE(s, "\"Yes\"", true);
    VALIDATE(s, "42", true);
    INVALIDATE(s, "{ \"Not a\": \"string or number\" }", "", "anyOf", "",
        "{ \"anyOf\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\", "
        "    \"errors\": ["
        "      { \"type\": {"
        "          \"instanceRef\": \"#\", \"schemaRef\": \"#/anyOf/0\","
        "          \"expected\": [\"string\"], \"actual\": \"object\""
        "      }},"
        "      { \"type\": {"
        "          \"instanceRef\": \"#\", \"schemaRef\": \"#/anyOf/1\","
        "          \"expected\": [\"number\"], \"actual\": \"object\""
        "      }}"
        "    ]"
        "}}");
}

TEST(SchemaValidator, OneOf) {
    Document sd;
    sd.Parse("{\"oneOf\": [{ \"type\": \"number\", \"multipleOf\": 5 }, { \"type\": \"number\", \"multipleOf\": 3 } ] }");
    SchemaDocument s(sd);

    VALIDATE(s, "10", true);
    VALIDATE(s, "9", true);
    INVALIDATE(s, "2", "", "oneOf", "",
        "{ \"oneOf\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"errors\": ["
        "      { \"multipleOf\": {"
        "          \"instanceRef\": \"#\", \"schemaRef\": \"#/oneOf/0\","
        "          \"expected\": 5, \"actual\": 2"
        "      }},"
        "      { \"multipleOf\": {"
        "          \"instanceRef\": \"#\", \"schemaRef\": \"#/oneOf/1\","
        "          \"expected\": 3, \"actual\": 2"
        "      }}"
        "    ]"
        "}}");
    INVALIDATE(s, "15", "", "oneOf", "",
        "{ \"oneOf\": { \"instanceRef\": \"#\", \"schemaRef\": \"#\", \"errors\": [{}, {}]}}");
}

TEST(SchemaValidator, Not) {
    Document sd;
    sd.Parse("{\"not\":{ \"type\": \"string\"}}");
    SchemaDocument s(sd);

    VALIDATE(s, "42", true);
    VALIDATE(s, "{ \"key\": \"value\" }", true);
    INVALIDATE(s, "\"I am a string\"", "", "not", "",
        "{ \"not\": { \"instanceRef\": \"#\", \"schemaRef\": \"#\" }}");
}

TEST(SchemaValidator, Ref) {
    Document sd;
    sd.Parse(
        "{"
        "  \"$schema\": \"http://json-schema.org/draft-04/schema#\","
        ""
        "  \"definitions\": {"
        "    \"address\": {"
        "      \"type\": \"object\","
        "      \"properties\": {"
        "        \"street_address\": { \"type\": \"string\" },"
        "        \"city\":           { \"type\": \"string\" },"
        "        \"state\":          { \"type\": \"string\" }"
        "      },"
        "      \"required\": [\"street_address\", \"city\", \"state\"]"
        "    }"
        "  },"
        "  \"type\": \"object\","
        "  \"properties\": {"
        "    \"billing_address\": { \"$ref\": \"#/definitions/address\" },"
        "    \"shipping_address\": { \"$ref\": \"#/definitions/address\" }"
        "  }"
        "}");
    SchemaDocument s(sd);

    VALIDATE(s, "{\"shipping_address\": {\"street_address\": \"1600 Pennsylvania Avenue NW\", \"city\": \"Washington\", \"state\": \"DC\"}, \"billing_address\": {\"street_address\": \"1st Street SE\", \"city\": \"Washington\", \"state\": \"DC\"} }", true);
}

TEST(SchemaValidator, Ref_AllOf) {
    Document sd;
    sd.Parse(
        "{"
        "  \"$schema\": \"http://json-schema.org/draft-04/schema#\","
        ""
        "  \"definitions\": {"
        "    \"address\": {"
        "      \"type\": \"object\","
        "      \"properties\": {"
        "        \"street_address\": { \"type\": \"string\" },"
        "        \"city\":           { \"type\": \"string\" },"
        "        \"state\":          { \"type\": \"string\" }"
        "      },"
        "      \"required\": [\"street_address\", \"city\", \"state\"]"
        "    }"
        "  },"
        "  \"type\": \"object\","
        "  \"properties\": {"
        "    \"billing_address\": { \"$ref\": \"#/definitions/address\" },"
        "    \"shipping_address\": {"
        "      \"allOf\": ["
        "        { \"$ref\": \"#/definitions/address\" },"
        "        { \"properties\":"
        "          { \"type\": { \"enum\": [ \"residential\", \"business\" ] } },"
        "          \"required\": [\"type\"]"
        "        }"
        "      ]"
        "    }"
        "  }"
        "}");
    SchemaDocument s(sd);

    INVALIDATE(s, "{\"shipping_address\": {\"street_address\": \"1600 Pennsylvania Avenue NW\", \"city\": \"Washington\", \"state\": \"DC\"} }", "/properties/shipping_address", "allOf", "/shipping_address",
        "{ \"required\": {"
        "    \"instanceRef\": \"#/shipping_address\","
        "    \"schemaRef\": \"#/properties/shipping_address/allOf/1\","
        "    \"missing\": [\"type\"]"
        "}}");
    VALIDATE(s, "{\"shipping_address\": {\"street_address\": \"1600 Pennsylvania Avenue NW\", \"city\": \"Washington\", \"state\": \"DC\", \"type\": \"business\"} }", true);
}

TEST(SchemaValidator, String) {
    Document sd;
    sd.Parse("{\"type\":\"string\"}");
    SchemaDocument s(sd);

    VALIDATE(s, "\"I'm a string\"", true);
    INVALIDATE(s, "42", "", "type", "",
        "{ \"type\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"string\"], \"actual\": \"integer\""
        "}}");
    INVALIDATE(s, "2147483648", "", "type", "",
        "{ \"type\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"string\"], \"actual\": \"integer\""
        "}}"); // 2^31 can only be fit in unsigned
    INVALIDATE(s, "-2147483649", "", "type", "",
        "{ \"type\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"string\"], \"actual\": \"integer\""
        "}}"); // -2^31 - 1 can only be fit in int64_t
    INVALIDATE(s, "4294967296", "", "type", "",
        "{ \"type\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"string\"], \"actual\": \"integer\""
        "}}"); // 2^32 can only be fit in int64_t
    INVALIDATE(s, "3.1415926", "", "type", "",
        "{ \"type\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"string\"], \"actual\": \"number\""
        "}}");
}

TEST(SchemaValidator, String_LengthRange) {
    Document sd;
    sd.Parse("{\"type\":\"string\",\"minLength\":2,\"maxLength\":3}");
    SchemaDocument s(sd);

    INVALIDATE(s, "\"A\"", "", "minLength", "",
        "{ \"minLength\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 2, \"actual\": \"A\""
        "}}");
    VALIDATE(s, "\"AB\"", true);
    VALIDATE(s, "\"ABC\"", true);
    INVALIDATE(s, "\"ABCD\"", "", "maxLength", "",
        "{ \"maxLength\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 3, \"actual\": \"ABCD\""
        "}}");
}

#if RAPIDJSON_SCHEMA_HAS_REGEX
TEST(SchemaValidator, String_Pattern) {
    Document sd;
    sd.Parse("{\"type\":\"string\",\"pattern\":\"^(\\\\([0-9]{3}\\\\))?[0-9]{3}-[0-9]{4}$\"}");
    SchemaDocument s(sd);

    VALIDATE(s, "\"555-1212\"", true);
    VALIDATE(s, "\"(888)555-1212\"", true);
    INVALIDATE(s, "\"(888)555-1212 ext. 532\"", "", "pattern", "",
        "{ \"pattern\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"actual\": \"(888)555-1212 ext. 532\""
        "}}");
    INVALIDATE(s, "\"(800)FLOWERS\"", "", "pattern", "",
        "{ \"pattern\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"actual\": \"(800)FLOWERS\""
        "}}");
}

TEST(SchemaValidator, String_Pattern_Invalid) {
    Document sd;
    sd.Parse("{\"type\":\"string\",\"pattern\":\"a{0}\"}"); // TODO: report regex is invalid somehow
    SchemaDocument s(sd);

    VALIDATE(s, "\"\"", true);
    VALIDATE(s, "\"a\"", true);
    VALIDATE(s, "\"aa\"", true);
}
#endif

TEST(SchemaValidator, Integer) {
    Document sd;
    sd.Parse("{\"type\":\"integer\"}");
    SchemaDocument s(sd);

    VALIDATE(s, "42", true);
    VALIDATE(s, "-1", true);
    VALIDATE(s, "2147483648", true); // 2^31 can only be fit in unsigned
    VALIDATE(s, "-2147483649", true); // -2^31 - 1 can only be fit in int64_t
    VALIDATE(s, "2147483648", true); // 2^31 can only be fit in unsigned
    VALIDATE(s, "4294967296", true); // 2^32 can only be fit in int64_t
    INVALIDATE(s, "3.1415926", "", "type", "",
        "{ \"type\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"integer\"], \"actual\": \"number\""
        "}}");
    INVALIDATE(s, "\"42\"", "", "type", "",
        "{ \"type\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"integer\"], \"actual\": \"string\""
        "}}");
}

TEST(SchemaValidator, Integer_Range) {
    Document sd;
    sd.Parse("{\"type\":\"integer\",\"minimum\":0,\"maximum\":100,\"exclusiveMaximum\":true}");
    SchemaDocument s(sd);

    INVALIDATE(s, "-1", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 0, \"actual\": -1"
        "}}");
    VALIDATE(s, "0", true);
    VALIDATE(s, "10", true);
    VALIDATE(s, "99", true);
    INVALIDATE(s, "100", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 100, \"exclusiveMaximum\": true, \"actual\": 100"
        "}}");
    INVALIDATE(s, "101", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 100, \"exclusiveMaximum\": true, \"actual\": 101"
        "}}");
}

TEST(SchemaValidator, Integer_Range64Boundary) {
    Document sd;
    sd.Parse("{\"type\":\"integer\",\"minimum\":-9223372036854775807,\"maximum\":9223372036854775806}");
    SchemaDocument s(sd);

    INVALIDATE(s, "-9223372036854775808", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": -9223372036854775807, \"actual\": -9223372036854775808"
        "}}");
    VALIDATE(s, "-9223372036854775807", true);
    VALIDATE(s, "-2147483648", true); // int min
    VALIDATE(s, "0", true);
    VALIDATE(s, "2147483647", true);  // int max
    VALIDATE(s, "2147483648", true);  // unsigned first
    VALIDATE(s, "4294967295", true);  // unsigned max
    VALIDATE(s, "9223372036854775806", true);
    INVALIDATE(s, "9223372036854775807", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 9223372036854775806, \"actual\": 9223372036854775807"
        "}}");
    INVALIDATE(s, "18446744073709551615", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 9223372036854775806, \"actual\": 18446744073709551615"
        "}}");   // uint64_t max
}

TEST(SchemaValidator, Integer_RangeU64Boundary) {
    Document sd;
    sd.Parse("{\"type\":\"integer\",\"minimum\":9223372036854775808,\"maximum\":18446744073709551614}");
    SchemaDocument s(sd);

    INVALIDATE(s, "-9223372036854775808", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 9223372036854775808, \"actual\": -9223372036854775808"
        "}}");
    INVALIDATE(s, "9223372036854775807", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 9223372036854775808, \"actual\": 9223372036854775807"
        "}}");
    INVALIDATE(s, "-2147483648", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 9223372036854775808, \"actual\": -2147483648"
        "}}"); // int min
    INVALIDATE(s, "0", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 9223372036854775808, \"actual\": 0"
        "}}");
    INVALIDATE(s, "2147483647", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 9223372036854775808, \"actual\": 2147483647"
        "}}");  // int max
    INVALIDATE(s, "2147483648", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 9223372036854775808, \"actual\": 2147483648"
        "}}");  // unsigned first
    INVALIDATE(s, "4294967295", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 9223372036854775808, \"actual\": 4294967295"
        "}}");  // unsigned max
    VALIDATE(s, "9223372036854775808", true);
    VALIDATE(s, "18446744073709551614", true);
    INVALIDATE(s, "18446744073709551615", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 18446744073709551614, \"actual\": 18446744073709551615"
        "}}");
}

TEST(SchemaValidator, Integer_Range64BoundaryExclusive) {
    Document sd;
    sd.Parse("{\"type\":\"integer\",\"minimum\":-9223372036854775808,\"maximum\":18446744073709551615,\"exclusiveMinimum\":true,\"exclusiveMaximum\":true}");
    SchemaDocument s(sd);

    INVALIDATE(s, "-9223372036854775808", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": -9223372036854775808, \"exclusiveMinimum\": true, "
        "    \"actual\": -9223372036854775808"
        "}}");
    VALIDATE(s, "-9223372036854775807", true);
    VALIDATE(s, "18446744073709551614", true);
    INVALIDATE(s, "18446744073709551615", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 18446744073709551615, \"exclusiveMaximum\": true, "
        "    \"actual\": 18446744073709551615"
        "}}");
}

TEST(SchemaValidator, Integer_MultipleOf) {
    Document sd;
    sd.Parse("{\"type\":\"integer\",\"multipleOf\":10}");
    SchemaDocument s(sd);

    VALIDATE(s, "0", true);
    VALIDATE(s, "10", true);
    VALIDATE(s, "-10", true);
    VALIDATE(s, "20", true);
    INVALIDATE(s, "23", "", "multipleOf", "",
        "{ \"multipleOf\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 10, \"actual\": 23"
        "}}");
    INVALIDATE(s, "-23", "", "multipleOf", "",
        "{ \"multipleOf\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 10, \"actual\": -23"
        "}}");
}

TEST(SchemaValidator, Integer_MultipleOf64Boundary) {
    Document sd;
    sd.Parse("{\"type\":\"integer\",\"multipleOf\":18446744073709551615}");
    SchemaDocument s(sd);

    VALIDATE(s, "0", true);
    VALIDATE(s, "18446744073709551615", true);
    INVALIDATE(s, "18446744073709551614", "", "multipleOf", "",
        "{ \"multipleOf\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 18446744073709551615, \"actual\": 18446744073709551614"
        "}}");
}

TEST(SchemaValidator, Number_Range) {
    Document sd;
    sd.Parse("{\"type\":\"number\",\"minimum\":0,\"maximum\":100,\"exclusiveMaximum\":true}");
    SchemaDocument s(sd);

    INVALIDATE(s, "-1", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 0, \"actual\": -1"
        "}}");
    VALIDATE(s, "0", true);
    VALIDATE(s, "0.1", true);
    VALIDATE(s, "10", true);
    VALIDATE(s, "99", true);
    VALIDATE(s, "99.9", true);
    INVALIDATE(s, "100", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 100, \"exclusiveMaximum\": true, \"actual\": 100"
        "}}");
    INVALIDATE(s, "100.0", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 100, \"exclusiveMaximum\": true, \"actual\": 100.0"
        "}}");
    INVALIDATE(s, "101.5", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 100, \"exclusiveMaximum\": true, \"actual\": 101.5"
        "}}");
}

TEST(SchemaValidator, Number_RangeInt) {
    Document sd;
    sd.Parse("{\"type\":\"number\",\"minimum\":-100,\"maximum\":-1,\"exclusiveMaximum\":true}");
    SchemaDocument s(sd);

    INVALIDATE(s, "-101", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": -100, \"actual\": -101"
        "}}");
    INVALIDATE(s, "-100.1", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": -100, \"actual\": -100.1"
        "}}");
    VALIDATE(s, "-100", true);
    VALIDATE(s, "-2", true);
    INVALIDATE(s, "-1", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": -1, \"exclusiveMaximum\": true, \"actual\": -1"
        "}}");
    INVALIDATE(s, "-0.9", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": -1, \"exclusiveMaximum\": true, \"actual\": -0.9"
        "}}");
    INVALIDATE(s, "0", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": -1, \"exclusiveMaximum\": true, \"actual\": 0"
        "}}");
    INVALIDATE(s, "2147483647", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": -1, \"exclusiveMaximum\": true, \"actual\": 2147483647"
        "}}");  // int max
    INVALIDATE(s, "2147483648", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": -1, \"exclusiveMaximum\": true, \"actual\": 2147483648"
        "}}");  // unsigned first
    INVALIDATE(s, "4294967295", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": -1, \"exclusiveMaximum\": true, \"actual\": 4294967295"
        "}}");  // unsigned max
    INVALIDATE(s, "9223372036854775808", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": -1, \"exclusiveMaximum\": true, \"actual\": 9223372036854775808"
        "}}");
    INVALIDATE(s, "18446744073709551614", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": -1, \"exclusiveMaximum\": true, \"actual\": 18446744073709551614"
        "}}");
    INVALIDATE(s, "18446744073709551615", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": -1, \"exclusiveMaximum\": true, \"actual\": 18446744073709551615"
        "}}");
}

TEST(SchemaValidator, Number_RangeDouble) {
    Document sd;
    sd.Parse("{\"type\":\"number\",\"minimum\":0.1,\"maximum\":100.1,\"exclusiveMaximum\":true}");
    SchemaDocument s(sd);

    INVALIDATE(s, "-9223372036854775808", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 0.1, \"actual\": -9223372036854775808"
        "}}");
    INVALIDATE(s, "-2147483648", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 0.1, \"actual\": -2147483648"
        "}}"); // int min
    INVALIDATE(s, "-1", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 0.1, \"actual\": -1"
        "}}");
    VALIDATE(s, "0.1", true);
    VALIDATE(s, "10", true);
    VALIDATE(s, "99", true);
    VALIDATE(s, "100", true);
    INVALIDATE(s, "101", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 100.1, \"exclusiveMaximum\": true, \"actual\": 101"
        "}}");
    INVALIDATE(s, "101.5", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 100.1, \"exclusiveMaximum\": true, \"actual\": 101.5"
        "}}");
    INVALIDATE(s, "18446744073709551614", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 100.1, \"exclusiveMaximum\": true, \"actual\": 18446744073709551614"
        "}}");
    INVALIDATE(s, "18446744073709551615", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 100.1, \"exclusiveMaximum\": true, \"actual\": 18446744073709551615"
        "}}");
    INVALIDATE(s, "2147483647", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 100.1, \"exclusiveMaximum\": true, \"actual\": 2147483647"
        "}}");  // int max
    INVALIDATE(s, "2147483648", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 100.1, \"exclusiveMaximum\": true, \"actual\": 2147483648"
        "}}");  // unsigned first
    INVALIDATE(s, "4294967295", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 100.1, \"exclusiveMaximum\": true, \"actual\": 4294967295"
        "}}");  // unsigned max
    INVALIDATE(s, "9223372036854775808", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 100.1, \"exclusiveMaximum\": true, \"actual\": 9223372036854775808"
        "}}");
    INVALIDATE(s, "18446744073709551614", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 100.1, \"exclusiveMaximum\": true, \"actual\": 18446744073709551614"
        "}}");
    INVALIDATE(s, "18446744073709551615", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 100.1, \"exclusiveMaximum\": true, \"actual\": 18446744073709551615"
        "}}");
}

TEST(SchemaValidator, Number_RangeDoubleU64Boundary) {
    Document sd;
    sd.Parse("{\"type\":\"number\",\"minimum\":9223372036854775808.0,\"maximum\":18446744073709550000.0}");
    SchemaDocument s(sd);

    INVALIDATE(s, "-9223372036854775808", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 9223372036854775808.0, \"actual\": -9223372036854775808"
        "}}");
    INVALIDATE(s, "-2147483648", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 9223372036854775808.0, \"actual\": -2147483648"
        "}}"); // int min
    INVALIDATE(s, "0", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 9223372036854775808.0, \"actual\": 0"
        "}}");
    INVALIDATE(s, "2147483647", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 9223372036854775808.0, \"actual\": 2147483647"
        "}}");  // int max
    INVALIDATE(s, "2147483648", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 9223372036854775808.0, \"actual\": 2147483648"
        "}}");  // unsigned first
    INVALIDATE(s, "4294967295", "", "minimum", "",
        "{ \"minimum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 9223372036854775808.0, \"actual\": 4294967295"
        "}}");  // unsigned max
    VALIDATE(s, "9223372036854775808", true);
    VALIDATE(s, "18446744073709540000", true);
    INVALIDATE(s, "18446744073709551615", "", "maximum", "",
        "{ \"maximum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 18446744073709550000.0, \"actual\": 18446744073709551615"
        "}}");
}

TEST(SchemaValidator, Number_MultipleOf) {
    Document sd;
    sd.Parse("{\"type\":\"number\",\"multipleOf\":10.0}");
    SchemaDocument s(sd);

    VALIDATE(s, "0", true);
    VALIDATE(s, "10", true);
    VALIDATE(s, "-10", true);
    VALIDATE(s, "20", true);
    INVALIDATE(s, "23", "", "multipleOf", "",
        "{ \"multipleOf\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 10.0, \"actual\": 23"
        "}}");
    INVALIDATE(s, "-2147483648", "", "multipleOf", "",
        "{ \"multipleOf\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 10.0, \"actual\": -2147483648"
        "}}");  // int min
    VALIDATE(s, "-2147483640", true);
    INVALIDATE(s, "2147483647", "", "multipleOf", "",
        "{ \"multipleOf\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 10.0, \"actual\": 2147483647"
        "}}");  // int max
    INVALIDATE(s, "2147483648", "", "multipleOf", "",
        "{ \"multipleOf\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 10.0, \"actual\": 2147483648"
        "}}");  // unsigned first
    VALIDATE(s, "2147483650", true);
    INVALIDATE(s, "4294967295", "", "multipleOf", "",
        "{ \"multipleOf\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 10.0, \"actual\": 4294967295"
        "}}");  // unsigned max
    VALIDATE(s, "4294967300", true);
}

TEST(SchemaValidator, Number_MultipleOfOne) {
    Document sd;
    sd.Parse("{\"type\":\"number\",\"multipleOf\":1}");
    SchemaDocument s(sd);

    VALIDATE(s, "42", true);
    VALIDATE(s, "42.0", true);
    INVALIDATE(s, "3.1415926", "", "multipleOf", "",
        "{ \"multipleOf\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 1, \"actual\": 3.1415926"
        "}}");
}

TEST(SchemaValidator, Object) {
    Document sd;
    sd.Parse("{\"type\":\"object\"}");
    SchemaDocument s(sd);

    VALIDATE(s, "{\"key\":\"value\",\"another_key\":\"another_value\"}", true);
    VALIDATE(s, "{\"Sun\":1.9891e30,\"Jupiter\":1.8986e27,\"Saturn\":5.6846e26,\"Neptune\":10.243e25,\"Uranus\":8.6810e25,\"Earth\":5.9736e24,\"Venus\":4.8685e24,\"Mars\":6.4185e23,\"Mercury\":3.3022e23,\"Moon\":7.349e22,\"Pluto\":1.25e22}", true);    
    INVALIDATE(s, "[\"An\", \"array\", \"not\", \"an\", \"object\"]", "", "type", "",
        "{ \"type\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"object\"], \"actual\": \"array\""
        "}}");
    INVALIDATE(s, "\"Not an object\"", "", "type", "",
        "{ \"type\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"object\"], \"actual\": \"string\""
        "}}");
}

TEST(SchemaValidator, Object_Properties) {
    Document sd;
    sd.Parse(
        "{"
        "    \"type\": \"object\","
        "    \"properties\" : {"
        "        \"number\": { \"type\": \"number\" },"
        "        \"street_name\" : { \"type\": \"string\" },"
        "        \"street_type\" : { \"type\": \"string\", \"enum\" : [\"Street\", \"Avenue\", \"Boulevard\"] }"
        "    }"
        "}");

    SchemaDocument s(sd);

    VALIDATE(s, "{ \"number\": 1600, \"street_name\": \"Pennsylvania\", \"street_type\": \"Avenue\" }", true);
    INVALIDATE(s, "{ \"number\": \"1600\", \"street_name\": \"Pennsylvania\", \"street_type\": \"Avenue\" }", "/properties/number", "type", "/number",
        "{ \"type\": {"
        "    \"instanceRef\": \"#/number\", \"schemaRef\": \"#/properties/number\","
        "    \"expected\": [\"number\"], \"actual\": \"string\""
        "}}");
    INVALIDATE(s, "{ \"number\": \"One\", \"street_name\": \"Microsoft\", \"street_type\": \"Way\" }",
        "/properties/number", "type", "/number",
        "{ \"type\": {"
        "    \"instanceRef\": \"#/number\", \"schemaRef\": \"#/properties/number\","
        "    \"expected\": [\"number\"], \"actual\": \"string\""
        "}}"); // fail fast
    VALIDATE(s, "{ \"number\": 1600, \"street_name\": \"Pennsylvania\" }", true);
    VALIDATE(s, "{}", true);
    VALIDATE(s, "{ \"number\": 1600, \"street_name\": \"Pennsylvania\", \"street_type\": \"Avenue\", \"direction\": \"NW\" }", true);
}

TEST(SchemaValidator, Object_AdditionalPropertiesBoolean) {
    Document sd;
    sd.Parse(
        "{"
        "    \"type\": \"object\","
        "        \"properties\" : {"
        "        \"number\": { \"type\": \"number\" },"
        "            \"street_name\" : { \"type\": \"string\" },"
        "            \"street_type\" : { \"type\": \"string\","
        "            \"enum\" : [\"Street\", \"Avenue\", \"Boulevard\"]"
        "        }"
        "    },"
        "    \"additionalProperties\": false"
        "}");

    SchemaDocument s(sd);

    VALIDATE(s, "{ \"number\": 1600, \"street_name\": \"Pennsylvania\", \"street_type\": \"Avenue\" }", true);
    INVALIDATE(s, "{ \"number\": 1600, \"street_name\": \"Pennsylvania\", \"street_type\": \"Avenue\", \"direction\": \"NW\" }", "", "additionalProperties", "/direction",
        "{ \"additionalProperties\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"disallowed\": \"direction\""
        "}}");
}

TEST(SchemaValidator, Object_AdditionalPropertiesObject) {
    Document sd;
    sd.Parse(
        "{"
        "    \"type\": \"object\","
        "    \"properties\" : {"
        "        \"number\": { \"type\": \"number\" },"
        "        \"street_name\" : { \"type\": \"string\" },"
        "        \"street_type\" : { \"type\": \"string\","
        "            \"enum\" : [\"Street\", \"Avenue\", \"Boulevard\"]"
        "        }"
        "    },"
        "    \"additionalProperties\": { \"type\": \"string\" }"
        "}");
    SchemaDocument s(sd);

    VALIDATE(s, "{ \"number\": 1600, \"street_name\": \"Pennsylvania\", \"street_type\": \"Avenue\" }", true);
    VALIDATE(s, "{ \"number\": 1600, \"street_name\": \"Pennsylvania\", \"street_type\": \"Avenue\", \"direction\": \"NW\" }", true);
    INVALIDATE(s, "{ \"number\": 1600, \"street_name\": \"Pennsylvania\", \"street_type\": \"Avenue\", \"office_number\": 201 }", "/additionalProperties", "type", "/office_number",
        "{ \"type\": {"
        "    \"instanceRef\": \"#/office_number\", \"schemaRef\": \"#/additionalProperties\","
        "    \"expected\": [\"string\"], \"actual\": \"integer\""
        "}}");
}

TEST(SchemaValidator, Object_Required) {
    Document sd;
    sd.Parse(
        "{"
        "    \"type\": \"object\","
        "    \"properties\" : {"
        "        \"name\":      { \"type\": \"string\" },"
        "        \"email\" : { \"type\": \"string\" },"
        "        \"address\" : { \"type\": \"string\" },"
        "        \"telephone\" : { \"type\": \"string\" }"
        "    },"
        "    \"required\":[\"name\", \"email\"]"
        "}");
    SchemaDocument s(sd);

    VALIDATE(s, "{ \"name\": \"William Shakespeare\", \"email\" : \"bill@stratford-upon-avon.co.uk\" }", true);
    VALIDATE(s, "{ \"name\": \"William Shakespeare\", \"email\" : \"bill@stratford-upon-avon.co.uk\", \"address\" : \"Henley Street, Stratford-upon-Avon, Warwickshire, England\", \"authorship\" : \"in question\"}", true);
    INVALIDATE(s, "{ \"name\": \"William Shakespeare\", \"address\" : \"Henley Street, Stratford-upon-Avon, Warwickshire, England\" }", "", "required", "",
        "{ \"required\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"missing\": [\"email\"]"
        "}}");
    INVALIDATE(s, "{}", "", "required", "",
        "{ \"required\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"missing\": [\"name\", \"email\"]"
        "}}");
}

TEST(SchemaValidator, Object_Required_PassWithDefault) {
    Document sd;
    sd.Parse(
        "{"
        "    \"type\": \"object\","
        "    \"properties\" : {"
        "        \"name\":      { \"type\": \"string\", \"default\": \"William Shakespeare\" },"
        "        \"email\" : { \"type\": \"string\", \"default\": \"\" },"
        "        \"address\" : { \"type\": \"string\" },"
        "        \"telephone\" : { \"type\": \"string\" }"
        "    },"
        "    \"required\":[\"name\", \"email\"]"
        "}");
    SchemaDocument s(sd);

    VALIDATE(s, "{ \"email\" : \"bill@stratford-upon-avon.co.uk\", \"address\" : \"Henley Street, Stratford-upon-Avon, Warwickshire, England\", \"authorship\" : \"in question\"}", true);
    INVALIDATE(s, "{ \"name\": \"William Shakespeare\", \"address\" : \"Henley Street, Stratford-upon-Avon, Warwickshire, England\" }", "", "required", "",
        "{ \"required\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"missing\": [\"email\"]"
        "}}");
    INVALIDATE(s, "{}", "", "required", "",
        "{ \"required\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"missing\": [\"email\"]"
        "}}");
}

TEST(SchemaValidator, Object_PropertiesRange) {
    Document sd;
    sd.Parse("{\"type\":\"object\", \"minProperties\":2, \"maxProperties\":3}");
    SchemaDocument s(sd);

    INVALIDATE(s, "{}", "", "minProperties", "",
        "{ \"minProperties\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 2, \"actual\": 0"
        "}}");
    INVALIDATE(s, "{\"a\":0}", "", "minProperties", "",
        "{ \"minProperties\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 2, \"actual\": 1"
        "}}");
    VALIDATE(s, "{\"a\":0,\"b\":1}", true);
    VALIDATE(s, "{\"a\":0,\"b\":1,\"c\":2}", true);
    INVALIDATE(s, "{\"a\":0,\"b\":1,\"c\":2,\"d\":3}", "", "maxProperties", "",
        "{ \"maxProperties\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\", "
        "    \"expected\": 3, \"actual\": 4"
        "}}");
}

TEST(SchemaValidator, Object_PropertyDependencies) {
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"object\","
        "  \"properties\": {"
        "    \"name\": { \"type\": \"string\" },"
        "    \"credit_card\": { \"type\": \"number\" },"
        "    \"cvv_code\": { \"type\": \"number\" },"
        "    \"billing_address\": { \"type\": \"string\" }"
        "  },"
        "  \"required\": [\"name\"],"
        "  \"dependencies\": {"
        "    \"credit_card\": [\"cvv_code\", \"billing_address\"]"
        "  }"
        "}");
    SchemaDocument s(sd);

    VALIDATE(s, "{ \"name\": \"John Doe\", \"credit_card\": 5555555555555555, \"cvv_code\": 777, "
        "\"billing_address\": \"555 Debtor's Lane\" }", true);
    INVALIDATE(s, "{ \"name\": \"John Doe\", \"credit_card\": 5555555555555555 }", "", "dependencies", "",
        "{ \"dependencies\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"errors\": {\"credit_card\": [\"cvv_code\", \"billing_address\"]}"
        "}}");
    VALIDATE(s, "{ \"name\": \"John Doe\"}", true);
    VALIDATE(s, "{ \"name\": \"John Doe\", \"cvv_code\": 777, \"billing_address\": \"555 Debtor's Lane\" }", true);
}

TEST(SchemaValidator, Object_SchemaDependencies) {
    Document sd;
    sd.Parse(
        "{"
        "    \"type\": \"object\","
        "    \"properties\" : {"
        "        \"name\": { \"type\": \"string\" },"
        "        \"credit_card\" : { \"type\": \"number\" }"
        "    },"
        "    \"required\" : [\"name\"],"
        "    \"dependencies\" : {"
        "        \"credit_card\": {"
        "            \"properties\": {"
        "                \"billing_address\": { \"type\": \"string\" }"
        "            },"
        "            \"required\" : [\"billing_address\"]"
        "        }"
        "    }"
        "}");
    SchemaDocument s(sd);

    VALIDATE(s, "{\"name\": \"John Doe\", \"credit_card\" : 5555555555555555,\"billing_address\" : \"555 Debtor's Lane\"}", true);
    INVALIDATE(s, "{\"name\": \"John Doe\", \"credit_card\" : 5555555555555555 }", "", "dependencies", "",
        "{ \"dependencies\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"errors\": {"
        "      \"credit_card\": {"
        "        \"required\": {"
        "          \"instanceRef\": \"#\", \"schemaRef\": \"#/dependencies/credit_card\","
        "          \"missing\": [\"billing_address\"]"
        "    } } }"
        "}}");
    VALIDATE(s, "{\"name\": \"John Doe\", \"billing_address\" : \"555 Debtor's Lane\"}", true);
}

#if RAPIDJSON_SCHEMA_HAS_REGEX
TEST(SchemaValidator, Object_PatternProperties) {
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"object\","
        "  \"patternProperties\": {"
        "    \"^S_\": { \"type\": \"string\" },"
        "    \"^I_\": { \"type\": \"integer\" }"
        "  }"
        "}");
    SchemaDocument s(sd);

    VALIDATE(s, "{ \"S_25\": \"This is a string\" }", true);
    VALIDATE(s, "{ \"I_0\": 42 }", true);
    INVALIDATE(s, "{ \"S_0\": 42 }", "", "patternProperties", "/S_0",
        "{ \"type\": {"
        "    \"instanceRef\": \"#/S_0\", \"schemaRef\": \"#/patternProperties/%5ES_\","
        "    \"expected\": [\"string\"], \"actual\": \"integer\""
        "}}");
    INVALIDATE(s, "{ \"I_42\": \"This is a string\" }", "", "patternProperties", "/I_42",
        "{ \"type\": {"
        "    \"instanceRef\": \"#/I_42\", \"schemaRef\": \"#/patternProperties/%5EI_\","
        "    \"expected\": [\"integer\"], \"actual\": \"string\""
        "}}");
    VALIDATE(s, "{ \"keyword\": \"value\" }", true);
}

TEST(SchemaValidator, Object_PattternProperties_ErrorConflict) {
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"object\","
        "  \"patternProperties\": {"
        "    \"^I_\": { \"multipleOf\": 5 },"
        "    \"30$\": { \"multipleOf\": 6 }"
        "  }"
        "}");
    SchemaDocument s(sd);

    VALIDATE(s, "{ \"I_30\": 30 }", true);
    INVALIDATE(s, "{ \"I_30\": 7 }", "", "patternProperties", "/I_30",
        "{ \"multipleOf\": ["
        "    {"
        "      \"instanceRef\": \"#/I_30\", \"schemaRef\": \"#/patternProperties/%5EI_\","
        "      \"expected\": 5, \"actual\": 7"
        "    }, {"
        "      \"instanceRef\": \"#/I_30\", \"schemaRef\": \"#/patternProperties/30%24\","
        "      \"expected\": 6, \"actual\": 7"
        "    }"
        "]}");
}

TEST(SchemaValidator, Object_Properties_PatternProperties) {
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"object\","
        "  \"properties\": {"
        "    \"I_42\": { \"type\": \"integer\", \"minimum\": 73 }"
        "  },"
        "  \"patternProperties\": {"
        "    \"^I_\": { \"type\": \"integer\", \"multipleOf\": 6 }"
        "  }"
        "}");
    SchemaDocument s(sd);

    VALIDATE(s, "{ \"I_6\": 6 }", true);
    VALIDATE(s, "{ \"I_42\": 78 }", true);
    INVALIDATE(s, "{ \"I_42\": 42 }", "", "patternProperties", "/I_42",
        "{ \"minimum\": {"
        "    \"instanceRef\": \"#/I_42\", \"schemaRef\": \"#/properties/I_42\","
        "    \"expected\": 73, \"actual\": 42"
        "}}");
    INVALIDATE(s, "{ \"I_42\": 7 }", "", "patternProperties", "/I_42",
        "{ \"minimum\": {"
        "    \"instanceRef\": \"#/I_42\", \"schemaRef\": \"#/properties/I_42\","
        "    \"expected\": 73, \"actual\": 7"
        "  },"
        "  \"multipleOf\": {"
        "    \"instanceRef\": \"#/I_42\", \"schemaRef\": \"#/patternProperties/%5EI_\","
        "    \"expected\": 6, \"actual\": 7"
        "  }"
        "}");
}

TEST(SchemaValidator, Object_PatternProperties_AdditionalProperties) {
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"object\","
        "  \"properties\": {"
        "    \"builtin\": { \"type\": \"number\" }"
        "  },"
        "  \"patternProperties\": {"
        "    \"^S_\": { \"type\": \"string\" },"
        "    \"^I_\": { \"type\": \"integer\" }"
        "  },"
        "  \"additionalProperties\": { \"type\": \"string\" }"
        "}");
    SchemaDocument s(sd);

    VALIDATE(s, "{ \"builtin\": 42 }", true);
    VALIDATE(s, "{ \"keyword\": \"value\" }", true);
    INVALIDATE(s, "{ \"keyword\": 42 }", "/additionalProperties", "type", "/keyword",
        "{ \"type\": {"
        "    \"instanceRef\": \"#/keyword\", \"schemaRef\": \"#/additionalProperties\","
        "    \"expected\": [\"string\"], \"actual\": \"integer\""
        "}}");
}
#endif

TEST(SchemaValidator, Array) {
    Document sd;
    sd.Parse("{\"type\":\"array\"}");
    SchemaDocument s(sd);

    VALIDATE(s, "[1, 2, 3, 4, 5]", true);
    VALIDATE(s, "[3, \"different\", { \"types\" : \"of values\" }]", true);
    INVALIDATE(s, "{\"Not\": \"an array\"}", "", "type", "",
        "{ \"type\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"array\"], \"actual\": \"object\""
        "}}");
}

TEST(SchemaValidator, Array_ItemsList) {
    Document sd;
    sd.Parse(
        "{"
        "    \"type\": \"array\","
        "    \"items\" : {"
        "        \"type\": \"number\""
        "    }"
        "}");
    SchemaDocument s(sd);

    VALIDATE(s, "[1, 2, 3, 4, 5]", true);
    INVALIDATE(s, "[1, 2, \"3\", 4, 5]", "/items", "type", "/2",
        "{ \"type\": {"
        "    \"instanceRef\": \"#/2\", \"schemaRef\": \"#/items\","
        "    \"expected\": [\"number\"], \"actual\": \"string\""
        "}}");
    VALIDATE(s, "[]", true);
}

TEST(SchemaValidator, Array_ItemsTuple) {
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"array\","
        "  \"items\": ["
        "    {"
        "      \"type\": \"number\""
        "    },"
        "    {"
        "      \"type\": \"string\""
        "    },"
        "    {"
        "      \"type\": \"string\","
        "      \"enum\": [\"Street\", \"Avenue\", \"Boulevard\"]"
        "    },"
        "    {"
        "      \"type\": \"string\","
        "      \"enum\": [\"NW\", \"NE\", \"SW\", \"SE\"]"
        "    }"
        "  ]"
        "}");
    SchemaDocument s(sd);

    VALIDATE(s, "[1600, \"Pennsylvania\", \"Avenue\", \"NW\"]", true);
    INVALIDATE(s, "[24, \"Sussex\", \"Drive\"]", "/items/2", "enum", "/2",
        "{ \"enum\": { \"instanceRef\": \"#/2\", \"schemaRef\": \"#/items/2\" }}");
    INVALIDATE(s, "[\"Palais de l'Elysee\"]", "/items/0", "type", "/0",
        "{ \"type\": {"
        "    \"instanceRef\": \"#/0\", \"schemaRef\": \"#/items/0\","
        "    \"expected\": [\"number\"], \"actual\": \"string\""
        "}}");
    INVALIDATE(s, "[\"Twenty-four\", \"Sussex\", \"Drive\"]", "/items/0", "type", "/0",
        "{ \"type\": {"
        "    \"instanceRef\": \"#/0\", \"schemaRef\": \"#/items/0\","
        "    \"expected\": [\"number\"], \"actual\": \"string\""
        "}}"); // fail fast
    VALIDATE(s, "[10, \"Downing\", \"Street\"]", true);
    VALIDATE(s, "[1600, \"Pennsylvania\", \"Avenue\", \"NW\", \"Washington\"]", true);
}

TEST(SchemaValidator, Array_AdditionalItmes) {
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"array\","
        "  \"items\": ["
        "    {"
        "      \"type\": \"number\""
        "    },"
        "    {"
        "      \"type\": \"string\""
        "    },"
        "    {"
        "      \"type\": \"string\","
        "      \"enum\": [\"Street\", \"Avenue\", \"Boulevard\"]"
        "    },"
        "    {"
        "      \"type\": \"string\","
        "      \"enum\": [\"NW\", \"NE\", \"SW\", \"SE\"]"
        "    }"
        "  ],"
        "  \"additionalItems\": false"
        "}");
    SchemaDocument s(sd);

    VALIDATE(s, "[1600, \"Pennsylvania\", \"Avenue\", \"NW\"]", true);
    VALIDATE(s, "[1600, \"Pennsylvania\", \"Avenue\"]", true);
    INVALIDATE(s, "[1600, \"Pennsylvania\", \"Avenue\", \"NW\", \"Washington\"]", "", "items", "/4",
        "{ \"additionalItems\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"disallowed\": 4"
        "}}");
}

TEST(SchemaValidator, Array_ItemsRange) {
    Document sd;
    sd.Parse("{\"type\": \"array\",\"minItems\": 2,\"maxItems\" : 3}");
    SchemaDocument s(sd);

    INVALIDATE(s, "[]", "", "minItems", "",
        "{ \"minItems\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 2, \"actual\": 0"
        "}}");
    INVALIDATE(s, "[1]", "", "minItems", "",
        "{ \"minItems\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 2, \"actual\": 1"
        "}}");
    VALIDATE(s, "[1, 2]", true);
    VALIDATE(s, "[1, 2, 3]", true);
    INVALIDATE(s, "[1, 2, 3, 4]", "", "maxItems", "",
        "{ \"maxItems\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 3, \"actual\": 4"
        "}}");
}

TEST(SchemaValidator, Array_UniqueItems) {
    Document sd;
    sd.Parse("{\"type\": \"array\", \"uniqueItems\": true}");
    SchemaDocument s(sd);

    VALIDATE(s, "[1, 2, 3, 4, 5]", true);
    INVALIDATE(s, "[1, 2, 3, 3, 4]", "", "uniqueItems", "/3",
        "{ \"uniqueItems\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"duplicates\": [2, 3]"
        "}}");
    INVALIDATE(s, "[1, 2, 3, 3, 3]", "", "uniqueItems", "/3",
        "{ \"uniqueItems\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"duplicates\": [2, 3]"
        "}}"); // fail fast
    VALIDATE(s, "[]", true);
}

TEST(SchemaValidator, Boolean) {
    Document sd;
    sd.Parse("{\"type\":\"boolean\"}");
    SchemaDocument s(sd);

    VALIDATE(s, "true", true);
    VALIDATE(s, "false", true);
    INVALIDATE(s, "\"true\"", "", "type", "",
        "{ \"type\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"boolean\"], \"actual\": \"string\""
        "}}");
    INVALIDATE(s, "0", "", "type", "",
        "{ \"type\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"boolean\"], \"actual\": \"integer\""
        "}}");
}

TEST(SchemaValidator, Null) {
    Document sd;
    sd.Parse("{\"type\":\"null\"}");
    SchemaDocument s(sd);

    VALIDATE(s, "null", true);
    INVALIDATE(s, "false", "", "type", "",
        "{ \"type\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"null\"], \"actual\": \"boolean\""
        "}}");
    INVALIDATE(s, "0", "", "type", "",
        "{ \"type\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"null\"], \"actual\": \"integer\""
        "}}");
    INVALIDATE(s, "\"\"", "", "type", "",
        "{ \"type\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"null\"], \"actual\": \"string\""
        "}}");
}

// Additional tests

TEST(SchemaValidator, ObjectInArray) {
    Document sd;
    sd.Parse("{\"type\":\"array\", \"items\": { \"type\":\"string\" }}");
    SchemaDocument s(sd);

    VALIDATE(s, "[\"a\"]", true);
    INVALIDATE(s, "[1]", "/items", "type", "/0",
        "{ \"type\": {"
        "    \"instanceRef\": \"#/0\", \"schemaRef\": \"#/items\","
        "    \"expected\": [\"string\"], \"actual\": \"integer\""
        "}}");
    INVALIDATE(s, "[{}]", "/items", "type", "/0",
        "{ \"type\": {"
        "    \"instanceRef\": \"#/0\", \"schemaRef\": \"#/items\","
        "    \"expected\": [\"string\"], \"actual\": \"object\""
        "}}");
}

TEST(SchemaValidator, MultiTypeInObject) {
    Document sd;
    sd.Parse(
        "{"
        "    \"type\":\"object\","
        "    \"properties\": {"
        "        \"tel\" : {"
        "            \"type\":[\"integer\", \"string\"]"
        "        }"
        "    }"
        "}");
    SchemaDocument s(sd);

    VALIDATE(s, "{ \"tel\": 999 }", true);
    VALIDATE(s, "{ \"tel\": \"123-456\" }", true);
    INVALIDATE(s, "{ \"tel\": true }", "/properties/tel", "type", "/tel",
        "{ \"type\": {"
        "    \"instanceRef\": \"#/tel\", \"schemaRef\": \"#/properties/tel\","
        "    \"expected\": [\"string\", \"integer\"], \"actual\": \"boolean\""
        "}}");
}

TEST(SchemaValidator, MultiTypeWithObject) {
    Document sd;
    sd.Parse(
        "{"
        "    \"type\": [\"object\",\"string\"],"
        "    \"properties\": {"
        "        \"tel\" : {"
        "            \"type\": \"integer\""
        "        }"
        "    }"
        "}");
    SchemaDocument s(sd);

    VALIDATE(s, "\"Hello\"", true);
    VALIDATE(s, "{ \"tel\": 999 }", true);
    INVALIDATE(s, "{ \"tel\": \"fail\" }", "/properties/tel", "type", "/tel",
        "{ \"type\": {"
        "    \"instanceRef\": \"#/tel\", \"schemaRef\": \"#/properties/tel\","
        "    \"expected\": [\"integer\"], \"actual\": \"string\""
        "}}");
}

TEST(SchemaValidator, AllOf_Nested) {
    Document sd;
    sd.Parse(
    "{"
    "    \"allOf\": ["
    "        { \"type\": \"string\", \"minLength\": 2 },"
    "        { \"type\": \"string\", \"maxLength\": 5 },"
    "        { \"allOf\": [ { \"enum\" : [\"ok\", \"okay\", \"OK\", \"o\"] }, { \"enum\" : [\"ok\", \"OK\", \"o\"]} ] }"
    "    ]"
    "}");
    SchemaDocument s(sd);

    VALIDATE(s, "\"ok\"", true);
    VALIDATE(s, "\"OK\"", true);
    INVALIDATE(s, "\"okay\"", "", "allOf", "",
        "{ \"enum\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/2/allOf/1\""
        "}}");
    INVALIDATE(s, "\"o\"", "", "allOf", "",
        "{ \"minLength\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/0\","
        "    \"expected\": 2, \"actual\": \"o\""
        "}}");
    INVALIDATE(s, "\"n\"", "", "allOf", "",
        "{ \"minLength\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/0\","
        "    \"expected\": 2, \"actual\": \"n\""
        "  },"
        "  \"enum\": ["
        "    {\"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/2/allOf/0\"},"
        "    {\"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/2/allOf/1\"}"
        "  ]"
        "}")
    INVALIDATE(s, "\"too long\"", "", "allOf", "",
        "{ \"maxLength\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/1\","
        "    \"expected\": 5, \"actual\": \"too long\""
        "  },"
        "  \"enum\": ["
        "    {\"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/2/allOf/0\"},"
        "    {\"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/2/allOf/1\"}"
        "  ]"
        "}");
    INVALIDATE(s, "123", "", "allOf", "",
        "{ \"type\": ["
        "    { \"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/0\","
        "      \"expected\": [\"string\"], \"actual\": \"integer\""
        "    },"
        "    { \"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/1\","
        "      \"expected\": [\"string\"], \"actual\": \"integer\""
        "    }"
        "  ],"
        "  \"enum\": ["
        "    {\"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/2/allOf/0\"},"
        "    {\"instanceRef\": \"#\", \"schemaRef\": \"#/allOf/2/allOf/1\"}"
        "  ]"
        "}");
}

TEST(SchemaValidator, EscapedPointer) {
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"object\","
        "  \"properties\": {"
        "    \"~/\": { \"type\": \"number\" }"
        "  }"
        "}");
    SchemaDocument s(sd);
    INVALIDATE(s, "{\"~/\":true}", "/properties/~0~1", "type", "/~0~1",
        "{ \"type\": {"
        "    \"instanceRef\": \"#/~0~1\", \"schemaRef\": \"#/properties/~0~1\","
        "    \"expected\": [\"number\"], \"actual\": \"boolean\""
        "}}");
}

template <typename Allocator>
static char* ReadFile(const char* filename, Allocator& allocator) {
    const char *paths[] = {
        "",
        "bin/",
        "../bin/",
        "../../bin/",
        "../../../bin/"
    };
    char buffer[1024];
    FILE *fp = 0;
    for (size_t i = 0; i < sizeof(paths) / sizeof(paths[0]); i++) {
        sprintf(buffer, "%s%s", paths[i], filename);
        fp = fopen(buffer, "rb");
        if (fp)
            break;
    }

    if (!fp)
        return 0;

    fseek(fp, 0, SEEK_END);
    size_t length = static_cast<size_t>(ftell(fp));
    fseek(fp, 0, SEEK_SET);
    char* json = reinterpret_cast<char*>(allocator.Malloc(length + 1));
    size_t readLength = fread(json, 1, length, fp);
    json[readLength] = '\0';
    fclose(fp);
    return json;
}

TEST(SchemaValidator, ValidateMetaSchema) {
    CrtAllocator allocator;
    char* json = ReadFile("draft-04/schema", allocator);
    Document d;
    d.Parse(json);
    ASSERT_FALSE(d.HasParseError());
    SchemaDocument sd(d);
    SchemaValidator validator(sd);
    if (!d.Accept(validator)) {
        StringBuffer sb;
        validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);
        printf("Invalid schema: %s\n", sb.GetString());
        printf("Invalid keyword: %s\n", validator.GetInvalidSchemaKeyword());
        sb.Clear();
        validator.GetInvalidDocumentPointer().StringifyUriFragment(sb);
        printf("Invalid document: %s\n", sb.GetString());
        sb.Clear();
        Writer<StringBuffer> w(sb);
        validator.GetError().Accept(w);
        printf("Validation error: %s\n", sb.GetString());
        ADD_FAILURE();
    }
    CrtAllocator::Free(json);
}

TEST(SchemaValidator, ValidateMetaSchema_UTF16) {
    typedef GenericDocument<UTF16<> > D;
    typedef GenericSchemaDocument<D::ValueType> SD;
    typedef GenericSchemaValidator<SD> SV;

    CrtAllocator allocator;
    char* json = ReadFile("draft-04/schema", allocator);

    D d;
    StringStream ss(json);
    d.ParseStream<0, UTF8<> >(ss);
    ASSERT_FALSE(d.HasParseError());
    SD sd(d);
    SV validator(sd);
    if (!d.Accept(validator)) {
        GenericStringBuffer<UTF16<> > sb;
        validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);
        wprintf(L"Invalid schema: %ls\n", sb.GetString());
        wprintf(L"Invalid keyword: %ls\n", validator.GetInvalidSchemaKeyword());
        sb.Clear();
        validator.GetInvalidDocumentPointer().StringifyUriFragment(sb);
        wprintf(L"Invalid document: %ls\n", sb.GetString());
        sb.Clear();
        Writer<GenericStringBuffer<UTF16<> >, UTF16<> > w(sb);
        validator.GetError().Accept(w);
        printf("Validation error: %ls\n", sb.GetString());
        ADD_FAILURE();
    }
    CrtAllocator::Free(json);
}

template <typename SchemaDocumentType = SchemaDocument>
class RemoteSchemaDocumentProvider : public IGenericRemoteSchemaDocumentProvider<SchemaDocumentType> {
public:
    RemoteSchemaDocumentProvider() : 
        documentAllocator_(documentBuffer_, sizeof(documentBuffer_)), 
        schemaAllocator_(schemaBuffer_, sizeof(schemaBuffer_)) 
    {
        const char* filenames[kCount] = {
            "jsonschema/remotes/integer.json",
            "jsonschema/remotes/subSchemas.json",
            "jsonschema/remotes/folder/folderInteger.json",
            "draft-04/schema"
        };
        const char* uris[kCount] = {
            "http://localhost:1234/integer.json",
            "http://localhost:1234/subSchemas.json",
            "http://localhost:1234/folder/folderInteger.json",
            "http://json-schema.org/draft-04/schema"
        };

        for (size_t i = 0; i < kCount; i++) {
            sd_[i] = 0;

            char jsonBuffer[8192];
            MemoryPoolAllocator<> jsonAllocator(jsonBuffer, sizeof(jsonBuffer));
            char* json = ReadFile(filenames[i], jsonAllocator);
            if (!json) {
                printf("json remote file %s not found", filenames[i]);
                ADD_FAILURE();
            }
            else {
                char stackBuffer[4096];
                MemoryPoolAllocator<> stackAllocator(stackBuffer, sizeof(stackBuffer));
                DocumentType d(&documentAllocator_, 1024, &stackAllocator);
                d.Parse(json);
                sd_[i] = new SchemaDocumentType(d, uris[i], static_cast<SizeType>(strlen(uris[i])), 0, &schemaAllocator_);
                MemoryPoolAllocator<>::Free(json);
            }
        };
    }

    ~RemoteSchemaDocumentProvider() {
        for (size_t i = 0; i < kCount; i++)
            delete sd_[i];
    }

    virtual const SchemaDocumentType* GetRemoteDocument(const char* uri, SizeType length) {
        for (size_t i = 0; i < kCount; i++)
            if (typename SchemaDocumentType::URIType(uri, length) == sd_[i]->GetURI())
                return sd_[i];
        return 0;
    }

private:
    typedef GenericDocument<typename SchemaDocumentType::EncodingType, MemoryPoolAllocator<>, MemoryPoolAllocator<> > DocumentType;

    RemoteSchemaDocumentProvider(const RemoteSchemaDocumentProvider&);
    RemoteSchemaDocumentProvider& operator=(const RemoteSchemaDocumentProvider&);

    static const size_t kCount = 4;
    SchemaDocumentType* sd_[kCount];
    typename DocumentType::AllocatorType documentAllocator_;
    typename SchemaDocumentType::AllocatorType schemaAllocator_;
    char documentBuffer_[16384];
    char schemaBuffer_[128u * 1024];
};

TEST(SchemaValidator, TestSuite) {
    const char* filenames[] = {
        "additionalItems.json",
        "additionalProperties.json",
        "allOf.json",
        "anyOf.json",
        "default.json",
        "definitions.json",
        "dependencies.json",
        "enum.json",
        "items.json",
        "maximum.json",
        "maxItems.json",
        "maxLength.json",
        "maxProperties.json",
        "minimum.json",
        "minItems.json",
        "minLength.json",
        "minProperties.json",
        "multipleOf.json",
        "not.json",
        "oneOf.json",
        "pattern.json",
        "patternProperties.json",
        "properties.json",
        "ref.json",
        "refRemote.json",
        "required.json",
        "type.json",
        "uniqueItems.json"
    };

    const char* onlyRunDescription = 0;
    //const char* onlyRunDescription = "a string is a string";

    unsigned testCount = 0;
    unsigned passCount = 0;

    typedef GenericSchemaDocument<Value, MemoryPoolAllocator<> > SchemaDocumentType;
    RemoteSchemaDocumentProvider<SchemaDocumentType> provider;

    char jsonBuffer[65536];
    char documentBuffer[65536];
    char documentStackBuffer[65536];
    char schemaBuffer[65536];
    char validatorBuffer[65536];
    MemoryPoolAllocator<> jsonAllocator(jsonBuffer, sizeof(jsonBuffer));
    MemoryPoolAllocator<> documentAllocator(documentBuffer, sizeof(documentBuffer));
    MemoryPoolAllocator<> documentStackAllocator(documentStackBuffer, sizeof(documentStackBuffer));
    MemoryPoolAllocator<> schemaAllocator(schemaBuffer, sizeof(schemaBuffer));
    MemoryPoolAllocator<> validatorAllocator(validatorBuffer, sizeof(validatorBuffer));

    for (size_t i = 0; i < sizeof(filenames) / sizeof(filenames[0]); i++) {
        char filename[FILENAME_MAX];
        sprintf(filename, "jsonschema/tests/draft4/%s", filenames[i]);
        char* json = ReadFile(filename, jsonAllocator);
        if (!json) {
            printf("json test suite file %s not found", filename);
            ADD_FAILURE();
        }
        else {
            GenericDocument<UTF8<>, MemoryPoolAllocator<>, MemoryPoolAllocator<> > d(&documentAllocator, 1024, &documentStackAllocator);
            d.Parse(json);
            if (d.HasParseError()) {
                printf("json test suite file %s has parse error", filename);
                ADD_FAILURE();
            }
            else {
                for (Value::ConstValueIterator schemaItr = d.Begin(); schemaItr != d.End(); ++schemaItr) {
                    {
                        SchemaDocumentType schema((*schemaItr)["schema"], filenames[i], static_cast<SizeType>(strlen(filenames[i])), &provider, &schemaAllocator);
                        GenericSchemaValidator<SchemaDocumentType, BaseReaderHandler<UTF8<> >, MemoryPoolAllocator<> > validator(schema, &validatorAllocator);
                        const char* description1 = (*schemaItr)["description"].GetString();
                        const Value& tests = (*schemaItr)["tests"];
                        for (Value::ConstValueIterator testItr = tests.Begin(); testItr != tests.End(); ++testItr) {
                            const char* description2 = (*testItr)["description"].GetString();
                            if (!onlyRunDescription || strcmp(description2, onlyRunDescription) == 0) {
                                const Value& data = (*testItr)["data"];
                                bool expected = (*testItr)["valid"].GetBool();
                                testCount++;
                                validator.Reset();
                                bool actual = data.Accept(validator);
                                if (expected != actual)
                                    printf("Fail: %30s \"%s\" \"%s\"\n", filename, description1, description2);
                                else
                                    passCount++;
                            }
                        }
                        //printf("%zu %zu %zu\n", documentAllocator.Size(), schemaAllocator.Size(), validatorAllocator.Size());
                    }
                    schemaAllocator.Clear();
                    validatorAllocator.Clear();
                }
            }
        }
        documentAllocator.Clear();
        MemoryPoolAllocator<>::Free(json);
        jsonAllocator.Clear();
    }
    printf("%d / %d passed (%2d%%)\n", passCount, testCount, passCount * 100 / testCount);
    // if (passCount != testCount)
    //     ADD_FAILURE();
}

TEST(SchemaValidatingReader, Simple) {
    Document sd;
    sd.Parse("{ \"type\": \"string\", \"enum\" : [\"red\", \"amber\", \"green\"] }");
    SchemaDocument s(sd);

    Document d;
    StringStream ss("\"red\"");
    SchemaValidatingReader<kParseDefaultFlags, StringStream, UTF8<> > reader(ss, s);
    d.Populate(reader);
    EXPECT_TRUE(reader.GetParseResult());
    EXPECT_TRUE(reader.IsValid());
    EXPECT_TRUE(d.IsString());
    EXPECT_STREQ("red", d.GetString());
}

TEST(SchemaValidatingReader, Invalid) {
    Document sd;
    sd.Parse("{\"type\":\"string\",\"minLength\":2,\"maxLength\":3}");
    SchemaDocument s(sd);

    Document d;
    StringStream ss("\"ABCD\"");
    SchemaValidatingReader<kParseDefaultFlags, StringStream, UTF8<> > reader(ss, s);
    d.Populate(reader);
    EXPECT_FALSE(reader.GetParseResult());
    EXPECT_FALSE(reader.IsValid());
    EXPECT_EQ(kParseErrorTermination, reader.GetParseResult().Code());
    EXPECT_STREQ("maxLength", reader.GetInvalidSchemaKeyword());
    EXPECT_TRUE(reader.GetInvalidSchemaPointer() == SchemaDocument::PointerType(""));
    EXPECT_TRUE(reader.GetInvalidDocumentPointer() == SchemaDocument::PointerType(""));
    EXPECT_TRUE(d.IsNull());
    Document e;
    e.Parse(
        "{ \"maxLength\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 3, \"actual\": \"ABCD\""
        "}}");
    if (e != reader.GetError()) {
        ADD_FAILURE();
    }
}

TEST(SchemaValidatingWriter, Simple) {
    Document sd;
    sd.Parse("{\"type\":\"string\",\"minLength\":2,\"maxLength\":3}");
    SchemaDocument s(sd);

    Document d;
    StringBuffer sb;
    Writer<StringBuffer> writer(sb);
    GenericSchemaValidator<SchemaDocument, Writer<StringBuffer> > validator(s, writer);

    d.Parse("\"red\"");
    EXPECT_TRUE(d.Accept(validator));
    EXPECT_TRUE(validator.IsValid());
    EXPECT_STREQ("\"red\"", sb.GetString());

    sb.Clear();
    validator.Reset();
    d.Parse("\"ABCD\"");
    EXPECT_FALSE(d.Accept(validator));
    EXPECT_FALSE(validator.IsValid());
    EXPECT_TRUE(validator.GetInvalidSchemaPointer() == SchemaDocument::PointerType(""));
    EXPECT_TRUE(validator.GetInvalidDocumentPointer() == SchemaDocument::PointerType(""));
    Document e;
    e.Parse(
        "{ \"maxLength\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": 3, \"actual\": \"ABCD\""
        "}}");
    EXPECT_EQ(e, validator.GetError());
}

TEST(Schema, Issue848) {
    rapidjson::Document d;
    rapidjson::SchemaDocument s(d);
    rapidjson::GenericSchemaValidator<rapidjson::SchemaDocument, rapidjson::Document> v(s);
}

#if RAPIDJSON_HAS_CXX11_RVALUE_REFS

static SchemaDocument ReturnSchemaDocument() {
    Document sd;
    sd.Parse("{ \"type\": [\"number\", \"string\"] }");
    SchemaDocument s(sd);
    return s;
}

TEST(Schema, Issue552) {
    SchemaDocument s = ReturnSchemaDocument();
    VALIDATE(s, "42", true);
    VALIDATE(s, "\"Life, the universe, and everything\"", true);
    INVALIDATE(s, "[\"Life\", \"the universe\", \"and everything\"]", "", "type", "",
        "{ \"type\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"expected\": [\"string\", \"number\"], \"actual\": \"array\""
        "}}");
}

#endif // RAPIDJSON_HAS_CXX11_RVALUE_REFS

TEST(SchemaValidator, Issue608) {
    Document sd;
    sd.Parse("{\"required\": [\"a\", \"b\"] }");
    SchemaDocument s(sd);

    VALIDATE(s, "{\"a\" : null, \"b\": null}", true);
    INVALIDATE(s, "{\"a\" : null, \"a\" : null}", "", "required", "",
        "{ \"required\": {"
        "    \"instanceRef\": \"#\", \"schemaRef\": \"#\","
        "    \"missing\": [\"b\"]"
        "}}");
}

// Fail to resolve $ref in allOf causes crash in SchemaValidator::StartObject()
TEST(SchemaValidator, Issue728_AllOfRef) {
    Document sd;
    sd.Parse("{\"allOf\": [{\"$ref\": \"#/abc\"}]}");
    SchemaDocument s(sd);
    VALIDATE(s, "{\"key1\": \"abc\", \"key2\": \"def\"}", true);
}

TEST(SchemaValidator, Issue825) {
    Document sd;
    sd.Parse("{\"type\": \"object\", \"additionalProperties\": false, \"patternProperties\": {\"^i\": { \"type\": \"string\" } } }");
    SchemaDocument s(sd);
    VALIDATE(s, "{ \"item\": \"hello\" }", true);
}

TEST(SchemaValidator, Issue1017_allOfHandler) {
    Document sd;
    sd.Parse("{\"allOf\": [{\"type\": \"object\",\"properties\": {\"cyanArray2\": {\"type\": \"array\",\"items\": { \"type\": \"string\" }}}},{\"type\": \"object\",\"properties\": {\"blackArray\": {\"type\": \"array\",\"items\": { \"type\": \"string\" }}},\"required\": [ \"blackArray\" ]}]}");
    SchemaDocument s(sd);
    StringBuffer sb;
    Writer<StringBuffer> writer(sb);
    GenericSchemaValidator<SchemaDocument, Writer<StringBuffer> > validator(s, writer);
    EXPECT_TRUE(validator.StartObject());
    EXPECT_TRUE(validator.Key("cyanArray2", 10, false));
    EXPECT_TRUE(validator.StartArray());    
    EXPECT_TRUE(validator.EndArray(0));    
    EXPECT_TRUE(validator.Key("blackArray", 10, false));
    EXPECT_TRUE(validator.StartArray());    
    EXPECT_TRUE(validator.EndArray(0));    
    EXPECT_TRUE(validator.EndObject(0));
    EXPECT_TRUE(validator.IsValid());
    EXPECT_STREQ("{\"cyanArray2\":[],\"blackArray\":[]}", sb.GetString());
}

TEST(SchemaValidator, Ref_remote) {
    typedef GenericSchemaDocument<Value, MemoryPoolAllocator<> > SchemaDocumentType;
    RemoteSchemaDocumentProvider<SchemaDocumentType> provider;
    Document sd;
    sd.Parse("{\"$ref\": \"http://localhost:1234/subSchemas.json#/integer\"}");
    SchemaDocumentType s(sd, 0, 0, &provider);
    typedef GenericSchemaValidator<SchemaDocumentType, BaseReaderHandler<UTF8<> >, MemoryPoolAllocator<> > SchemaValidatorType;
    typedef GenericPointer<Value, MemoryPoolAllocator<> > PointerType;
    INVALIDATE_(s, "null", "/integer", "type", "",
        "{ \"type\": {"
        "    \"instanceRef\": \"#\","
        "    \"schemaRef\": \"http://localhost:1234/subSchemas.json#/integer\","
        "    \"expected\": [\"integer\"], \"actual\": \"null\""
        "}}",
        SchemaValidatorType, PointerType);
}

TEST(SchemaValidator, Ref_remote_issue1210) {
    class SchemaDocumentProvider : public IRemoteSchemaDocumentProvider {
        SchemaDocument** collection;

        SchemaDocumentProvider(const SchemaDocumentProvider&);
        SchemaDocumentProvider& operator=(const SchemaDocumentProvider&);

        public:
          SchemaDocumentProvider(SchemaDocument** collection) : collection(collection) { }
          virtual const SchemaDocument* GetRemoteDocument(const char* uri, SizeType length) {
            int i = 0;
            while (collection[i] && SchemaDocument::URIType(uri, length) != collection[i]->GetURI()) ++i;
            return collection[i];
          }
    };
    SchemaDocument* collection[] = { 0, 0, 0 };
    SchemaDocumentProvider provider(collection);

    Document x, y, z;
    x.Parse("{\"properties\":{\"country\":{\"$ref\":\"y.json#/definitions/country_remote\"}},\"type\":\"object\"}");
    y.Parse("{\"definitions\":{\"country_remote\":{\"$ref\":\"z.json#/definitions/country_list\"}}}");
    z.Parse("{\"definitions\":{\"country_list\":{\"enum\":[\"US\"]}}}");

    SchemaDocument sz(z, "z.json", 6, &provider);
    collection[0] = &sz;
    SchemaDocument sy(y, "y.json", 6, &provider);
    collection[1] = &sy;
    SchemaDocument sx(x, "x.json", 6, &provider);

    VALIDATE(sx, "{\"country\":\"UK\"}", false);
    VALIDATE(sx, "{\"country\":\"US\"}", true);
}

#if defined(_MSC_VER) || defined(__clang__)
RAPIDJSON_DIAG_POP
#endif
