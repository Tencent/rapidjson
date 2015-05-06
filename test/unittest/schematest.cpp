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

using namespace rapidjson;

// Test cases following http://spacetelescope.github.io/understanding-json-schema

#define VALIDATE(schema, json, expected) \
{\
    ASSERT_TRUE(schema.IsValid());\
    SchemaValidator validator(schema);\
    Document d;\
    /*printf("\n%s\n", json);*/\
    d.Parse(json);\
    EXPECT_FALSE(d.HasParseError());\
    if (expected) {\
        EXPECT_TRUE(d.Accept(validator));\
        EXPECT_TRUE(validator.IsValid());\
    }\
    else {\
        EXPECT_FALSE(d.Accept(validator));\
        EXPECT_FALSE(validator.IsValid()); \
    }\
}

TEST(SchemaValidator, Typeless) {
    Document sd;
    sd.Parse("{}");
    Schema s(sd);
    
    VALIDATE(s, "42", true);
    VALIDATE(s, "\"I'm a string\"", true);
    VALIDATE(s, "{ \"an\": [ \"arbitrarily\", \"nested\" ], \"data\": \"structure\" }", true);
}

TEST(SchemaValidator, MultiType) {
    Document sd;
    sd.Parse("{ \"type\": [\"number\", \"string\"] }");
    Schema s(sd);

    VALIDATE(s, "42", true);
    VALIDATE(s, "\"Life, the universe, and everything\"", true);
    VALIDATE(s, "[\"Life\", \"the universe\", \"and everything\"]", false);
}

TEST(SchemaValidator, Enum_Typed) {
    Document sd;
    sd.Parse("{ \"type\": \"string\", \"enum\" : [\"red\", \"amber\", \"green\"] }");
    Schema s(sd);

    VALIDATE(s, "\"red\"", true);
    VALIDATE(s, "\"blue\"", false);
}

TEST(SchemaValidator, Enum_Typless) {
    Document sd;
    sd.Parse("{  \"enum\": [\"red\", \"amber\", \"green\", null, 42] }");
    Schema s(sd);

    VALIDATE(s, "\"red\"", true);
    VALIDATE(s, "null", true);
    VALIDATE(s, "42", true);
    VALIDATE(s, "0", false);
}

TEST(SchemaValidator, Enum_InvalidType) {
    Document sd;
    sd.Parse("{ \"type\": \"string\", \"enum\": [\"red\", \"amber\", \"green\", null] }");
    Schema s(sd);

    VALIDATE(s, "\"red\"", true);
    VALIDATE(s, "null", false);
}

TEST(SchemaValidator, AllOf) {
    {
        Document sd;
        sd.Parse("{\"allOf\": [{ \"type\": \"string\" }, { \"type\": \"string\", \"maxLength\": 5 }]}"); // need "type": "string" now
        Schema s(sd);

        //VALIDATE(s, "\"ok\"", true);
        VALIDATE(s, "\"too long\"", false);
    }
    {
        Document sd;
        sd.Parse("{\"allOf\": [{ \"type\": \"string\" }, { \"type\": \"number\" } ] }");
        Schema s(sd);

        VALIDATE(s, "\"No way\"", false);
        VALIDATE(s, "-1", false);
    }
}

TEST(SchemaValidator, AnyOf) {
    Document sd;
    sd.Parse("{\"anyOf\": [{ \"type\": \"string\" }, { \"type\": \"number\" } ] }");
    Schema s(sd);

    //VALIDATE(s, "\"Yes\"", true);
    //VALIDATE(s, "42", true);
    VALIDATE(s, "{ \"Not a\": \"string or number\" }", false);
}

TEST(SchemaValidator, OneOf) {
    Document sd;
    sd.Parse("{\"oneOf\": [{ \"type\": \"number\", \"multipleOf\": 5 }, { \"type\": \"number\", \"multipleOf\": 3 } ] }");
    Schema s(sd);

    VALIDATE(s, "10", true);
    VALIDATE(s, "9", true);
    VALIDATE(s, "2", false);
    VALIDATE(s, "15", false);
}

TEST(SchemaValidator, Not) {
    Document sd;
    sd.Parse("{\"not\":{ \"type\": \"string\"}}");
    Schema s(sd);

    VALIDATE(s, "42", true);
    VALIDATE(s, "{ \"key\": \"value\" }", true); // TO FIX
    VALIDATE(s, "\"I am a string\"", false);
}

TEST(SchemaValidator, String) {
    Document sd;
    sd.Parse("{\"type\":\"string\"}");
    Schema s(sd);

    VALIDATE(s, "\"I'm a string\"", true);
    VALIDATE(s, "42", false);
}

TEST(SchemaValidator, String_LengthRange) {
    Document sd;
    sd.Parse("{\"type\":\"string\",\"minLength\":2,\"maxLength\":3}");
    Schema s(sd);

    VALIDATE(s, "\"A\"", false);
    VALIDATE(s, "\"AB\"", true);
    VALIDATE(s, "\"ABC\"", true);
    VALIDATE(s, "\"ABCD\"", false);
}

#if RAPIDJSON_SCHEMA_HAS_REGEX
TEST(SchemaValidator, String_Pattern) {
    Document sd;
    sd.Parse("{\"type\":\"string\",\"pattern\":\"^(\\\\([0-9]{3}\\\\))?[0-9]{3}-[0-9]{4}$\"}");
    Schema s(sd);

    VALIDATE(s, "\"555-1212\"", true);
    VALIDATE(s, "\"(888)555-1212\"", true);
    VALIDATE(s, "\"(888)555-1212 ext. 532\"", false);
    VALIDATE(s, "\"(800)FLOWERS\"", false);
}
#endif

TEST(SchemaValidator, Integer) {
    Document sd;
    sd.Parse("{\"type\":\"integer\"}");
    Schema s(sd);

    VALIDATE(s, "42", true);
    VALIDATE(s, "-1", true);
    VALIDATE(s, "3.1415926", false);
    VALIDATE(s, "\"42\"", false);
}

TEST(SchemaValidator, Integer_Range) {
    Document sd;
    sd.Parse("{\"type\":\"integer\",\"minimum\":0,\"maximum\":100,\"exclusiveMaximum\":true}");
    Schema s(sd);

    VALIDATE(s, "-1", false);
    VALIDATE(s, "0", true);
    VALIDATE(s, "10", true);
    VALIDATE(s, "99", true);
    VALIDATE(s, "100", false);
    VALIDATE(s, "101", false);
}

TEST(SchemaValidator, Integer_MultipleOf) {
    Document sd;
    sd.Parse("{\"type\":\"integer\",\"multipleOf\":10}");
    Schema s(sd);

    VALIDATE(s, "0", true);
    VALIDATE(s, "10", true);
    VALIDATE(s, "20", true);
    VALIDATE(s, "23", false);
}

TEST(SchemaValidator, Number_Range) {
    Document sd;
    sd.Parse("{\"type\":\"number\",\"minimum\":0,\"maximum\":100,\"exclusiveMaximum\":true}");
    Schema s(sd);

    VALIDATE(s, "-1", false);
    VALIDATE(s, "0", true);
    VALIDATE(s, "10", true);
    VALIDATE(s, "99", true);
    VALIDATE(s, "100", false);
    VALIDATE(s, "101", false);
}

TEST(SchemaValidator, Number_MultipleOf) {
    Document sd;
    sd.Parse("{\"type\":\"number\",\"multipleOf\":10}");
    Schema s(sd);

    VALIDATE(s, "0", true);
    VALIDATE(s, "10", true);
    VALIDATE(s, "20", true);
    VALIDATE(s, "23", false);
}

TEST(SchemaValidator, Number_MultipleOfOne) {
    Document sd;
    sd.Parse("{\"type\":\"number\",\"multipleOf\":1}");
    Schema s(sd);

    VALIDATE(s, "42", true);
    VALIDATE(s, "42.0", true);
    VALIDATE(s, "3.1415926", false);
}

TEST(SchemaValidator, Object) {
    Document sd;
    sd.Parse("{\"type\":\"object\"}");
    Schema s(sd);

    VALIDATE(s, "{\"key\":\"value\",\"another_key\":\"another_value\"}", true);
    VALIDATE(s, "{\"Sun\":1.9891e30,\"Jupiter\":1.8986e27,\"Saturn\":5.6846e26,\"Neptune\":10.243e25,\"Uranus\":8.6810e25,\"Earth\":5.9736e24,\"Venus\":4.8685e24,\"Mars\":6.4185e23,\"Mercury\":3.3022e23,\"Moon\":7.349e22,\"Pluto\":1.25e22}", true);    
    VALIDATE(s, "[\"An\", \"array\", \"not\", \"an\", \"object\"]", false);
    VALIDATE(s, "\"Not an object\"", false);
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

    Schema s(sd);

    VALIDATE(s, "{ \"number\": 1600, \"street_name\": \"Pennsylvania\", \"street_type\": \"Avenue\" }", true);
    VALIDATE(s, "{ \"number\": \"1600\", \"street_name\": \"Pennsylvania\", \"street_type\": \"Avenue\" }", false);
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

    Schema s(sd);

    VALIDATE(s, "{ \"number\": 1600, \"street_name\": \"Pennsylvania\", \"street_type\": \"Avenue\" }", true);
    VALIDATE(s, "{ \"number\": 1600, \"street_name\": \"Pennsylvania\", \"street_type\": \"Avenue\", \"direction\": \"NW\" }", false);
}

TEST(SchemaValidator, Object_AdditionalPropertiesObject) {
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
        "    \"additionalProperties\": { \"type\": \"string\" }"
        "}");
    Schema s(sd);

    VALIDATE(s, "{ \"number\": 1600, \"street_name\": \"Pennsylvania\", \"street_type\": \"Avenue\" }", true);
    VALIDATE(s, "{ \"number\": 1600, \"street_name\": \"Pennsylvania\", \"street_type\": \"Avenue\", \"direction\": \"NW\" }", true);
    VALIDATE(s, "{ \"number\": 1600, \"street_name\": \"Pennsylvania\", \"street_type\": \"Avenue\", \"office_number\": 201 }", false);
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
    Schema s(sd);

    VALIDATE(s, "{ \"name\": \"William Shakespeare\", \"email\" : \"bill@stratford-upon-avon.co.uk\" }", true);
    VALIDATE(s, "{ \"name\": \"William Shakespeare\", \"email\" : \"bill@stratford-upon-avon.co.uk\", \"address\" : \"Henley Street, Stratford-upon-Avon, Warwickshire, England\", \"authorship\" : \"in question\"}", true);
    VALIDATE(s, "{ \"name\": \"William Shakespeare\", \"address\" : \"Henley Street, Stratford-upon-Avon, Warwickshire, England\" }", false);
}


TEST(SchemaValidator, Object_PropertiesRange) {
    Document sd;
    sd.Parse("{\"type\":\"object\", \"minProperties\":2, \"maxProperties\":3}");
    Schema s(sd);

    VALIDATE(s, "{}", false);
    VALIDATE(s, "{\"a\":0}", false);
    VALIDATE(s, "{\"a\":0,\"b\":1}", true);
    VALIDATE(s, "{\"a\":0,\"b\":1,\"c\":2}", true);
    VALIDATE(s, "{\"a\":0,\"b\":1,\"c\":2,\"d\":3}", false);
}

TEST(SchemaValidator, Object_PropertyDependencies) {
    Document sd;
    sd.Parse(
        "{"
        "  \"type\": \"object\","
        "  \"properties\": {"
        "    \"name\": { \"type\": \"string\" },"
        "    \"credit_card\": { \"type\": \"number\" },"
        "    \"billing_address\": { \"type\": \"string\" }"
        "  },"
        "  \"required\": [\"name\"],"
        "  \"dependencies\": {"
        "    \"credit_card\": [\"billing_address\"]"
        "  }"
        "}");
    Schema s(sd);

    VALIDATE(s, "{ \"name\": \"John Doe\",  \"credit_card\": 5555555555555555, \"billing_address\": \"555 Debtor's Lane\" }", true);
    VALIDATE(s, "{ \"name\": \"John Doe\", \"credit_card\": 5555555555555555 }", false);
    VALIDATE(s, "{ \"name\": \"John Doe\"}", true);
    VALIDATE(s, "{ \"name\": \"John Doe\", \"billing_address\": \"555 Debtor's Lane\" }", true);
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
    Schema s(sd);

    VALIDATE(s, "{ \"S_25\": \"This is a string\" }", true);
    VALIDATE(s, "{ \"I_0\": 42 }", true);
    VALIDATE(s, "{ \"S_0\": 42 }", false);
    VALIDATE(s, "{ \"I_42\": \"This is a string\" }", false);
    VALIDATE(s, "{ \"keyword\": \"value\" }", true);
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
    Schema s(sd);

    VALIDATE(s, "{ \"builtin\": 42 }", true);
    VALIDATE(s, "{ \"keyword\": \"value\" }", true);
    VALIDATE(s, "{ \"keyword\": 42 }", false);
}

#endif // RAPIDJSON_SCHEMA_HAS_REGEX

TEST(SchemaValidator, Array) {
    Document sd;
    sd.Parse("{\"type\":\"array\"}");
    Schema s(sd);

    VALIDATE(s, "[1, 2, 3, 4, 5]", true);
    VALIDATE(s, "[3, \"different\", { \"types\" : \"of values\" }]", true);
    VALIDATE(s, "{\"Not\": \"an array\"}", false);
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
    Schema s(sd);

    VALIDATE(s, "[1, 2, 3, 4, 5]", true);
    VALIDATE(s, "[1, 2, \"3\", 4, 5]", false);
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
    Schema s(sd);

    VALIDATE(s, "[1600, \"Pennsylvania\", \"Avenue\", \"NW\"]", true);
    VALIDATE(s, "[24, \"Sussex\", \"Drive\"]", false);
    VALIDATE(s, "[\"Palais de l'Elysee\"]", false);
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
    Schema s(sd);

    VALIDATE(s, "[1600, \"Pennsylvania\", \"Avenue\", \"NW\"]", true);
    VALIDATE(s, "[1600, \"Pennsylvania\", \"Avenue\"]", true);
    VALIDATE(s, "[1600, \"Pennsylvania\", \"Avenue\", \"NW\", \"Washington\"]", false);
}

TEST(SchemaValidator, Array_ItemsRange) {
    Document sd;
    sd.Parse("{\"type\": \"array\",\"minItems\": 2,\"maxItems\" : 3}");
    Schema s(sd);

    VALIDATE(s, "[]", false);
    VALIDATE(s, "[1]", false);
    VALIDATE(s, "[1, 2]", true);
    VALIDATE(s, "[1, 2, 3]", true);
    VALIDATE(s, "[1, 2, 3, 4]", false);
}

#if 0
// TODO
TEST(SchemaValidator, Array_Uniqueness) {
    Document sd;
    sd.Parse("{\"type\": \"array\", \"uniqueItems\": true}");
    Schema s(sd);

    VALIDATE(s, "[1, 2, 3, 4, 5]", true);
    VALIDATE(s, "[1, 2, 3, 4, 5]", false);
}
#endif

TEST(SchemaValidator, Boolean) {
    Document sd;
    sd.Parse("{\"type\":\"boolean\"}");
    Schema s(sd);

    VALIDATE(s, "true", true);
    VALIDATE(s, "false", true);
    VALIDATE(s, "\"true\"", false);
    VALIDATE(s, "0", false);
}

TEST(SchemaValidator, Null) {
    Document sd;
    sd.Parse("{\"type\":\"null\"}");
    Schema s(sd);

    VALIDATE(s, "null", true);
    VALIDATE(s, "false", false);
    VALIDATE(s, "0", false);
    VALIDATE(s, "\"\"", false);
}

// Additional tests

TEST(SchemaValidator, ObjectInArray) {
    Document sd;
    sd.Parse("{\"type\":\"array\", \"items\": { \"type\":\"string\" }}");
    Schema s(sd);

    VALIDATE(s, "[\"a\"]", true);
    VALIDATE(s, "[1]", false);
    VALIDATE(s, "[{}]", false);
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
    Schema s(sd);

    VALIDATE(s, "{ \"tel\": 999 }", true);
    VALIDATE(s, "{ \"tel\": \"123-456\" }", true);
    VALIDATE(s, "{ \"tel\": true }", false);
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
    Schema s(sd);

    VALIDATE(s, "\"Hello\"", true);
    VALIDATE(s, "{ \"tel\": 999 }", true);
    VALIDATE(s, "{ \"tel\": \"fail\" }", false);
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
    Schema s(sd);

    VALIDATE(s, "\"ok\"", true);
    VALIDATE(s, "\"OK\"", true);
    VALIDATE(s, "\"okay\"", false);
    VALIDATE(s, "\"o\"", false);
    VALIDATE(s, "\"n\"", false);
    VALIDATE(s, "\"too long\"", false);
    VALIDATE(s, "123", false);
}
