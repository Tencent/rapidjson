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
    SchemaValidator validator(schema);\
    Document d;\
    /*printf("\n%s\n", json);*/\
    d.Parse(json);\
    EXPECT_FALSE(d.HasParseError());\
    EXPECT_TRUE(expected == d.Accept(validator));\
    EXPECT_TRUE(expected == validator.IsValid());\
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
    VALIDATE(s, "[\"Life\", \"the universe\", \"and everything\"]", false);
}

TEST(SchemaValidator, Enum_Typed) {
    Document sd;
    sd.Parse("{ \"type\": \"string\", \"enum\" : [\"red\", \"amber\", \"green\"] }");
    SchemaDocument s(sd);

    VALIDATE(s, "\"red\"", true);
    VALIDATE(s, "\"blue\"", false);
}

TEST(SchemaValidator, Enum_Typless) {
    Document sd;
    sd.Parse("{  \"enum\": [\"red\", \"amber\", \"green\", null, 42] }");
    SchemaDocument s(sd);

    VALIDATE(s, "\"red\"", true);
    VALIDATE(s, "null", true);
    VALIDATE(s, "42", true);
    VALIDATE(s, "0", false);
}

TEST(SchemaValidator, Enum_InvalidType) {
    Document sd;
    sd.Parse("{ \"type\": \"string\", \"enum\": [\"red\", \"amber\", \"green\", null] }");
    SchemaDocument s(sd);

    VALIDATE(s, "\"red\"", true);
    VALIDATE(s, "null", false);
}

TEST(SchemaValidator, AllOf) {
    {
        Document sd;
        sd.Parse("{\"allOf\": [{ \"type\": \"string\" }, { \"type\": \"string\", \"maxLength\": 5 }]}"); // need "type": "string" now
        SchemaDocument s(sd);

        //VALIDATE(s, "\"ok\"", true);
        VALIDATE(s, "\"too long\"", false);
    }
    {
        Document sd;
        sd.Parse("{\"allOf\": [{ \"type\": \"string\" }, { \"type\": \"number\" } ] }");
        SchemaDocument s(sd);

        VALIDATE(s, "\"No way\"", false);
        VALIDATE(s, "-1", false);
    }
}

TEST(SchemaValidator, AnyOf) {
    Document sd;
    sd.Parse("{\"anyOf\": [{ \"type\": \"string\" }, { \"type\": \"number\" } ] }");
    SchemaDocument s(sd);

    //VALIDATE(s, "\"Yes\"", true);
    //VALIDATE(s, "42", true);
    VALIDATE(s, "{ \"Not a\": \"string or number\" }", false);
}

TEST(SchemaValidator, OneOf) {
    Document sd;
    sd.Parse("{\"oneOf\": [{ \"type\": \"number\", \"multipleOf\": 5 }, { \"type\": \"number\", \"multipleOf\": 3 } ] }");
    SchemaDocument s(sd);

    VALIDATE(s, "10", true);
    VALIDATE(s, "9", true);
    VALIDATE(s, "2", false);
    VALIDATE(s, "15", false);
}

TEST(SchemaValidator, Not) {
    Document sd;
    sd.Parse("{\"not\":{ \"type\": \"string\"}}");
    SchemaDocument s(sd);

    VALIDATE(s, "42", true);
    VALIDATE(s, "{ \"key\": \"value\" }", true); // TO FIX
    VALIDATE(s, "\"I am a string\"", false);
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

    VALIDATE(s, "{\"shipping_address\": {\"street_address\": \"1600 Pennsylvania Avenue NW\", \"city\": \"Washington\", \"state\": \"DC\"} }", false);
    VALIDATE(s, "{\"shipping_address\": {\"street_address\": \"1600 Pennsylvania Avenue NW\", \"city\": \"Washington\", \"state\": \"DC\", \"type\": \"business\"} }", true);
}

TEST(SchemaValidator, String) {
    Document sd;
    sd.Parse("{\"type\":\"string\"}");
    SchemaDocument s(sd);

    VALIDATE(s, "\"I'm a string\"", true);
    VALIDATE(s, "42", false);
}

TEST(SchemaValidator, String_LengthRange) {
    Document sd;
    sd.Parse("{\"type\":\"string\",\"minLength\":2,\"maxLength\":3}");
    SchemaDocument s(sd);

    VALIDATE(s, "\"A\"", false);
    VALIDATE(s, "\"AB\"", true);
    VALIDATE(s, "\"ABC\"", true);
    VALIDATE(s, "\"ABCD\"", false);
}

#if RAPIDJSON_SCHEMA_HAS_REGEX
TEST(SchemaValidator, String_Pattern) {
    Document sd;
    sd.Parse("{\"type\":\"string\",\"pattern\":\"^(\\\\([0-9]{3}\\\\))?[0-9]{3}-[0-9]{4}$\"}");
    SchemaDocument s(sd);

    VALIDATE(s, "\"555-1212\"", true);
    VALIDATE(s, "\"(888)555-1212\"", true);
    VALIDATE(s, "\"(888)555-1212 ext. 532\"", false);
    VALIDATE(s, "\"(800)FLOWERS\"", false);
}
#endif

TEST(SchemaValidator, Integer) {
    Document sd;
    sd.Parse("{\"type\":\"integer\"}");
    SchemaDocument s(sd);

    VALIDATE(s, "42", true);
    VALIDATE(s, "-1", true);
    VALIDATE(s, "3.1415926", false);
    VALIDATE(s, "\"42\"", false);
}

TEST(SchemaValidator, Integer_Range) {
    Document sd;
    sd.Parse("{\"type\":\"integer\",\"minimum\":0,\"maximum\":100,\"exclusiveMaximum\":true}");
    SchemaDocument s(sd);

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
    SchemaDocument s(sd);

    VALIDATE(s, "0", true);
    VALIDATE(s, "10", true);
    VALIDATE(s, "20", true);
    VALIDATE(s, "23", false);
}

TEST(SchemaValidator, Number_Range) {
    Document sd;
    sd.Parse("{\"type\":\"number\",\"minimum\":0,\"maximum\":100,\"exclusiveMaximum\":true}");
    SchemaDocument s(sd);

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
    SchemaDocument s(sd);

    VALIDATE(s, "0", true);
    VALIDATE(s, "10", true);
    VALIDATE(s, "20", true);
    VALIDATE(s, "23", false);
}

TEST(SchemaValidator, Number_MultipleOfOne) {
    Document sd;
    sd.Parse("{\"type\":\"number\",\"multipleOf\":1}");
    SchemaDocument s(sd);

    VALIDATE(s, "42", true);
    VALIDATE(s, "42.0", true);
    VALIDATE(s, "3.1415926", false);
}

TEST(SchemaValidator, Object) {
    Document sd;
    sd.Parse("{\"type\":\"object\"}");
    SchemaDocument s(sd);

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

    SchemaDocument s(sd);

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

    SchemaDocument s(sd);

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
    SchemaDocument s(sd);

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
    SchemaDocument s(sd);

    VALIDATE(s, "{ \"name\": \"William Shakespeare\", \"email\" : \"bill@stratford-upon-avon.co.uk\" }", true);
    VALIDATE(s, "{ \"name\": \"William Shakespeare\", \"email\" : \"bill@stratford-upon-avon.co.uk\", \"address\" : \"Henley Street, Stratford-upon-Avon, Warwickshire, England\", \"authorship\" : \"in question\"}", true);
    VALIDATE(s, "{ \"name\": \"William Shakespeare\", \"address\" : \"Henley Street, Stratford-upon-Avon, Warwickshire, England\" }", false);
}


TEST(SchemaValidator, Object_PropertiesRange) {
    Document sd;
    sd.Parse("{\"type\":\"object\", \"minProperties\":2, \"maxProperties\":3}");
    SchemaDocument s(sd);

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
    SchemaDocument s(sd);

    VALIDATE(s, "{ \"name\": \"John Doe\",  \"credit_card\": 5555555555555555, \"billing_address\": \"555 Debtor's Lane\" }", true);
    VALIDATE(s, "{ \"name\": \"John Doe\", \"credit_card\": 5555555555555555 }", false);
    VALIDATE(s, "{ \"name\": \"John Doe\"}", true);
    VALIDATE(s, "{ \"name\": \"John Doe\", \"billing_address\": \"555 Debtor's Lane\" }", true);
}

TEST(SchemaValidator, Object_SchemaDependencies) {
    Document sd;
    sd.Parse(
        "{"
        "    \"type\": \"object\","
        "    \"properties\" : {"
        "    \"name\": { \"type\": \"string\" },"
        "        \"credit_card\" : { \"type\": \"number\" }"
        "    },"
        "    \"required\" : [\"name\"],"
        "        \"dependencies\" : {"
        "        \"credit_card\": {"
        "            \"properties\": {"
        "                \"billing_address\": { \"type\": \"string\" }"
        "            },"
        "            \"required\" : [\"billing_address\"]"
        "        }"
        "    }"
        "}");
    SchemaDocument s(sd);

    //VALIDATE(s, "{\"name\": \"John Doe\", \"credit_card\" : 5555555555555555,\"billing_address\" : \"555 Debtor's Lane\"}", true);
    VALIDATE(s, "{\"name\": \"John Doe\", \"credit_card\" : 5555555555555555 }", false);
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
    SchemaDocument s(sd);

    VALIDATE(s, "{ \"builtin\": 42 }", true);
    VALIDATE(s, "{ \"keyword\": \"value\" }", true);
    VALIDATE(s, "{ \"keyword\": 42 }", false);
}
#endif

TEST(SchemaValidator, Array) {
    Document sd;
    sd.Parse("{\"type\":\"array\"}");
    SchemaDocument s(sd);

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
    SchemaDocument s(sd);

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
    SchemaDocument s(sd);

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
    SchemaDocument s(sd);

    VALIDATE(s, "[1600, \"Pennsylvania\", \"Avenue\", \"NW\"]", true);
    VALIDATE(s, "[1600, \"Pennsylvania\", \"Avenue\"]", true);
    VALIDATE(s, "[1600, \"Pennsylvania\", \"Avenue\", \"NW\", \"Washington\"]", false);
}

TEST(SchemaValidator, Array_ItemsRange) {
    Document sd;
    sd.Parse("{\"type\": \"array\",\"minItems\": 2,\"maxItems\" : 3}");
    SchemaDocument s(sd);

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
    SchemaDocument s(sd);

    VALIDATE(s, "[1, 2, 3, 4, 5]", true);
    VALIDATE(s, "[1, 2, 3, 4, 5]", false);
}
#endif

TEST(SchemaValidator, Boolean) {
    Document sd;
    sd.Parse("{\"type\":\"boolean\"}");
    SchemaDocument s(sd);

    VALIDATE(s, "true", true);
    VALIDATE(s, "false", true);
    VALIDATE(s, "\"true\"", false);
    VALIDATE(s, "0", false);
}

TEST(SchemaValidator, Null) {
    Document sd;
    sd.Parse("{\"type\":\"null\"}");
    SchemaDocument s(sd);

    VALIDATE(s, "null", true);
    VALIDATE(s, "false", false);
    VALIDATE(s, "0", false);
    VALIDATE(s, "\"\"", false);
}

// Additional tests

TEST(SchemaValidator, ObjectInArray) {
    Document sd;
    sd.Parse("{\"type\":\"array\", \"items\": { \"type\":\"string\" }}");
    SchemaDocument s(sd);

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
    SchemaDocument s(sd);

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
    SchemaDocument s(sd);

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
    SchemaDocument s(sd);

    VALIDATE(s, "\"ok\"", true);
    VALIDATE(s, "\"OK\"", true);
    VALIDATE(s, "\"okay\"", false);
    VALIDATE(s, "\"o\"", false);
    VALIDATE(s, "\"n\"", false);
    VALIDATE(s, "\"too long\"", false);
    VALIDATE(s, "123", false);
}

static char* ReadFile(const char* filename, size_t& length) {
    const char *paths[] = {
        "%s",
        "bin/%s",
        "../bin/%s",
        "../../bin/%s",
        "../../../bin/%s"
    };
    char buffer[1024];
    FILE *fp = 0;
    for (size_t i = 0; i < sizeof(paths) / sizeof(paths[0]); i++) {
        sprintf(buffer, paths[i], filename);
        fp = fopen(buffer, "rb");
        if (fp)
            break;
    }

    if (!fp)
        return 0;

    fseek(fp, 0, SEEK_END);
    length = (size_t)ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char* json = (char*)malloc(length + 1);
    size_t readLength = fread(json, 1, length, fp);
    json[readLength] = '\0';
    fclose(fp);
    return json;
}

class RemoteSchemaDocumentProvider : public IRemoteSchemaDocumentProvider {
public:
    RemoteSchemaDocumentProvider() {
        const char* filenames[kCount] = {
            "jsonschema/remotes/integer.json",
            "jsonschema/remotes/subSchemas.json",
            "jsonschema/remotes/folder/folderInteger.json",
            "draft-04/schema"
        };

        for (size_t i = 0; i < kCount; i++) {
            d_[i] = 0;
            sd_[i] = 0;

            size_t length;
            char* json = ReadFile(filenames[i], length);
            if (!json) {
                printf("json remote file %s not found", filenames[i]);
                ADD_FAILURE();
            }
            else {
                d_[i] = new Document;
                d_[i]->Parse(json);
                sd_[i] = new SchemaDocument(*d_[i]);
                free(json);
            }
        };
    }

    ~RemoteSchemaDocumentProvider() {
        for (size_t i = 0; i < kCount; i++) {
            delete d_[i];
            delete sd_[i];
        }
    }

    virtual const SchemaDocument* GetRemoteDocument(const char* uri, SizeType length) {
        const char* uris[kCount] = {
            "http://localhost:1234/integer.json",
            "http://localhost:1234/subSchemas.json",
            "http://localhost:1234/folder/folderInteger.json",
            "http://json-schema.org/draft-04/schema"
        };

        for (size_t i = 0; i < kCount; i++)
            if (strncmp(uri, uris[i], length) == 0)
                return sd_[i];
        return 0;
    }

private:
    static const size_t kCount = 4;
    Document* d_[kCount];
    SchemaDocument* sd_[kCount];
};

TEST(SchemaValidator, TestSuite) {
    const char* filenames[] = {
        "properties.json",
        "additionalItems.json",
        "additionalProperties.json",
        "allOf.json",
        "anyOf.json",
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
        //"uniqueItems.json"
    };

    const char* onlyRunDescription = 0;
    //const char* onlyRunDescription = "a string is a string";

    unsigned testCount = 0;
    unsigned passCount = 0;

    RemoteSchemaDocumentProvider provider;

    for (size_t i = 0; i < sizeof(filenames) / sizeof(filenames[0]); i++) {
        char filename[FILENAME_MAX];
        sprintf(filename, "jsonschema/tests/draft4/%s", filenames[i]);
        size_t length;
        char* json = ReadFile(filename, length);
        if (!json) {
            printf("json test suite file %s not found", filename);
            ADD_FAILURE();
        }
        else {
            Document d;
            d.Parse(json);
            if (d.HasParseError()) {
                printf("json test suite file %s has parse error", filename);
                ADD_FAILURE();
            }
            else {
                for (Value::ConstValueIterator schemaItr = d.Begin(); schemaItr != d.End(); ++schemaItr) {
                    SchemaDocument schema((*schemaItr)["schema"], &provider);
                    SchemaValidator validator(schema);
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
                }
            }
        }
        free(json);
    }
    printf("%d / %d passed (%2d%%)\n", passCount, testCount, passCount * 100 / testCount);
    // if (passCount != testCount)
    //     ADD_FAILURE();
}