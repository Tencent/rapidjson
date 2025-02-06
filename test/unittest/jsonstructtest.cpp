#include "rapidjson/rapidjson.h"
#if RAPIDJSON_HAS_CXX17
#include "rapidjson/jsonstruct.h"

#include "rapidjson/writer.h"

#include <gtest/gtest.h>

using namespace testing;

namespace jsonstruct
{
    //
    // Adhoc tests which are a bit easier to follow and give a feel for what the code does:
    //

    //
    // Define some fields.
    //
    // This requires a macro because pre c++17 there's no standard way
    // of making compile time string objects.
    //
    DEFINE_FIELD_NAME(price);
    DEFINE_FIELD_NAME(description);
    DEFINE_FIELD_NAME(content);

    //
    // Define a message structure.
    //
    // Order becomes a strongly typed struct-like type capable of
    // holding a double and a string.
    //
    using Order = Object<
        Field<price, Double>,
        Field<description, String> >;
    //
    // There is not space or time overhead incurred compared to using
    // the equivalent struct definition. In particular.
    //
    struct OrderStruct
    {
        Double price;
        String description;
    };
    static_assert(sizeof(Order) == sizeof(OrderStruct), "no space overhead");
    
    TEST(AnObject, hasGetMethod)
    {
        Order underTest;
        //
        // Get method is srongly type with field access checked at
        // compile time. Generated code is equivalent to struct field
        // access.
        //
        underTest.get<price>();
    }

    TEST(AnObject, canBeConstructedFromFields)
    {
        Order underTest{3.14, "hello"};
        EXPECT_DOUBLE_EQ(underTest.get<price>(), 3.14);
        EXPECT_EQ(underTest.get<description>(), "hello");
    }

    //
    // Objects defined in this way conform to the sax like reader API
    // of rapid json and so can store the result of a parse directly.
    //
    TEST(AnObject, canParse)
    {
        Order underTest;

        auto msg = "{ \"price\": 0.234, \"description\": \"foo\" }";
        rapidjson::StringStream buf(msg);
        rapidjson::Reader reader;
        reader.IterativeParseInit();

        underTest.Parse<rj::kParseIterativeFlag>(reader, buf);
        EXPECT_DOUBLE_EQ(underTest.get<price>(), 0.234);
        EXPECT_EQ(underTest.get<description>(), "foo");
    }

    TEST(AnObject, ignoresUnknownFields)
    {
        Order underTest;

        auto msg = "{ \"price\": 0.234, \"description\": \"foo\", \"that_not_be_there\": \"bar\" }";
        rapidjson::StringStream buf(msg);
        rapidjson::Reader reader;
        reader.IterativeParseInit();

        underTest.Parse<rj::kParseIterativeFlag>(reader, buf);
        EXPECT_DOUBLE_EQ(underTest.get<price>(), 0.234);
        EXPECT_EQ(underTest.get<description>(), "foo");
    }

    TEST(AnObject, DISABLED_ignoresUnknownFieldsAtBeginning)
    {
        Order underTest;

        auto msg = "{ \"that_not_be_there\": \"bar\", \"price\": 0.234, \"description\": \"foo\" }";
        rapidjson::StringStream buf(msg);
        rapidjson::Reader reader;
        reader.IterativeParseInit();

        underTest.Parse<rj::kParseIterativeFlag>(reader, buf);
        EXPECT_DOUBLE_EQ(underTest.get<price>(), 0.234);
        EXPECT_EQ(underTest.get<description>(), "foo");
    }    

    DEFINE_FIELD_NAME(field1);
    DEFINE_FIELD_NAME(field2);
    DEFINE_FIELD_NAME(field3);

    TEST(AnObject, canParseFieldsOfTheSameType)
    {
        using UnderTest = Object<Field<field1, Int>,
                                 Field<field2, Int> >;
        UnderTest underTest;

        auto msg = "{ \"field1\": 1, \"field2\": 2 }";
        rapidjson::StringStream buf(msg);
        rapidjson::Reader reader;
        reader.IterativeParseInit();

        underTest.Parse<rj::kParseIterativeFlag>(reader, buf);
        EXPECT_EQ(underTest.get<field1>(), 1);
        EXPECT_EQ(underTest.get<field2>(), 2);
    }

    struct ANestedObject: testing::Test
    {
        using UnderTest = Object<
            Field<field1, Int>,
            Field<content, Order> >;
        UnderTest underTest;
        
        const char* msg =
            R"___({"field1":42,"content":{"price":0.234,"description":"foo"}})___";
    };

    TEST_F(ANestedObject, canParse)
    {
        rapidjson::StringStream buf(msg);
        rapidjson::Reader reader;
        reader.IterativeParseInit();

        EXPECT_TRUE(underTest.Parse<rj::kParseIterativeFlag>(reader, buf));
        EXPECT_DOUBLE_EQ(underTest.get<content>().get<price>(), 0.234);
        EXPECT_EQ(underTest.get<content>().get<description>(), "foo");

        const auto& c = underTest.get<content>();

        EXPECT_DOUBLE_EQ(c.get<price>(), 0.234);
        EXPECT_EQ(c.get<description>(), "foo");
    }

    TEST_F(ANestedObject, canWrite)
    {
        underTest.set(field1{}, 42);
        underTest.get<content>().set(description{}, "foo");
        underTest.get<content>().get(price{}) = 0.234;
        
        rj::StringBuffer buffer;
        rj::Writer<rj::StringBuffer> writer(buffer);
        this->underTest.Accept(writer);
        EXPECT_STREQ(buffer.GetString(), msg);
    }    

    TEST(AnArray, canParse)
    {
        Array<Int> underTest;

        auto msg = "[0, 1, 2, 3, 4]";
        rapidjson::StringStream buf(msg);
        rapidjson::Reader reader;
        reader.IterativeParseInit();

        underTest.Parse<rj::kParseIterativeFlag>(reader, buf);

        EXPECT_EQ(underTest.size(), 5u);
        EXPECT_EQ(underTest[0].value, 0);
        EXPECT_EQ(underTest[1].value, 1);
        EXPECT_EQ(underTest[2].value, 2);
        EXPECT_EQ(underTest[3].value, 3);
        EXPECT_EQ(underTest[4].value, 4);
    }

    TEST(AnObjectInAnArray, canParse)
    {
        using UnderTest = Array<Order>;
        UnderTest underTest;

        auto msg = "[{ \"price\": 0.234, \"description\": \"foo\" }]";
        rapidjson::StringStream buf(msg);
        rapidjson::Reader reader;
        reader.IterativeParseInit();

        ASSERT_TRUE(underTest.Parse<rj::kParseIterativeFlag>(reader, buf));
        EXPECT_DOUBLE_EQ(underTest[0].get<price>(), 0.234);
        EXPECT_EQ(underTest[0].get<description>(), "foo");
    }

    //
    // Full tests are parametised and a bit harder to follow.
    //
    using Object1 = Object<Field<field2, Bool> >;
    static_assert(sizeof(Object1) == sizeof(bool), "");

    using Object2 = Object<
        Field<field2, Bool>,
        Field<field3, Int> >;
    static_assert(sizeof(Object2) == 2 * sizeof(int), "");

    /*RawNumber, Double - omitted because of warnings about floating
     * point comparisons. */
    using ForAllFieldTypes = ::testing::Types<
        Bool, Int, Uint, Int64, Uint64, String,
        Object1, Object2,
        Array<Bool>, Array<Int> >;

    template<typename T> struct AFieldTraits;

    template<>
    struct AFieldTraits<Bool>
    {
        const char* objectMsg = R"___({"field1":true})___";
        const char* arrayMsg = R"___([true])___";
        const Bool::type result = true;
        template<typename T>
        static decltype(auto) get(const T& v){ return v.value; }
    };
    template<>
    struct AFieldTraits<Int>
    {
        const char* objectMsg = R"___({"field1":-42})___";
        const char* arrayMsg = R"___([-42])___";
        const Int::type result = -42;
        template<typename T>
        static decltype(auto) get(const T& v){ return v.value; }
    };
    template<>
    struct AFieldTraits<Uint>
    {
        const char* objectMsg = R"___({"field1":42})___";
        const char* arrayMsg = R"___([42])___";
        const Uint::type result = 42;
        template<typename T>
        static decltype(auto) get(const T& v){ return v.value; }
    };
    template<>
    struct AFieldTraits<Int64>
    {
        const char* objectMsg = R"___({"field1":-9876543219876})___";
        const char* arrayMsg = R"___([-9876543219876])___";
        const Int64::type result = -9876543219876;
        template<typename T>
        static decltype(auto) get(const T& v){ return v.value; }
    };
    template<>
    struct AFieldTraits<Uint64>
    {
        const char* objectMsg = R"___({"field1":9876543219876})___";
        const char* arrayMsg = R"___([9876543219876])___";
        const Uint64::type result = 9876543219876;
        template<typename T>
        static decltype(auto) get(const T& v){ return v.value; }
    };
    template<>
    struct AFieldTraits<Double>
    {
        const char* objectMsg = R"___({"field1":3.14})___";
        const char* arrayMsg = R"___([3.14])___";
        const Double::type result = 3.14;
        template<typename T>
        static decltype(auto) get(const T& v){ return v.value; }
    };
    template<>
    struct AFieldTraits<RawNumber>
    {
        const char* objectMsg = R"___({"field1":987987987987989})___";
        const char* arrayMsg = R"___([987987987987989])___";
        const RawNumber::type result = "987987987987989";
        template<typename T>
        static decltype(auto) get(const T& v){ return v.value; }
    };
    template<>
    struct AFieldTraits<String>
    {
        const char* objectMsg = R"___({"field1":"Gordon Sumner"})___";
        const char* arrayMsg = R"___(["Gordon Sumner"])___";
        const String::type result = "Gordon Sumner";
        template<typename T>
        static decltype(auto) get(const T& v){ return v.value; }
    };

    template<>
    struct AFieldTraits<Object1>
    {
        const char* objectMsg = R"___({"field1":{"field2":true}})___";
        const char* arrayMsg = R"___([{"field2":true}])___";
        const Object1 result{true};
        template<typename T>
        static decltype(auto) get(const T& v){ return v; }
    };

    template<>
    struct AFieldTraits<Object2>
    {
        const char* objectMsg =
            R"___({"field1":{"field2":true,"field3":42}})___";
        const char* arrayMsg = R"___([{"field2":true,"field3":42}])___";
        const Object2 result{true,42};
        template<typename T>
        static decltype(auto) get(const T& v){ return v; }
    };

    template<>
    struct AFieldTraits<Array<Bool> >
    {
        const char* objectMsg = R"___({"field1":[true]})___";
        const char* arrayMsg = R"___([[true]])___";
        const Array<Bool> result{{true}};
        template<typename T>
        static decltype(auto) get(const T& v){ return v; }
    };

    template<>
    struct AFieldTraits<Array<Int> >
    {
        const char* objectMsg = R"___({"field1":[]})___";
        const char* arrayMsg = R"___([[]])___";
        const Array<Int> result{};
        template<typename T>
        static decltype(auto) get(const T& v){ return v; }
    };
    
    template<typename T>
    struct AFieldInAnObject: testing::Test
    {
        using UnderTest = Object<Field<field1, T> >;
        UnderTest underTest;
        AFieldTraits<T> traits;
    };

    TYPED_TEST_CASE(AFieldInAnObject, ForAllFieldTypes);
    
    TYPED_TEST(AFieldInAnObject, canBeConstructed){}

    TYPED_TEST(AFieldInAnObject, canGet)
    {
        this->underTest.get(field1{}) = this->traits.result;
    }

    TYPED_TEST(AFieldInAnObject, canGetConst)
    {
        this->underTest.get(field1{}) = this->traits.result;
        const auto& c = this->underTest;
        EXPECT_EQ(c.get(field1{}), this->traits.result);
    }
    
    TYPED_TEST(AFieldInAnObject, canParse)
    {
        rapidjson::StringStream buf(this->traits.objectMsg);
        rapidjson::Reader reader;
        reader.IterativeParseInit();
        EXPECT_TRUE(
            this->underTest.template Parse<rj::kParseIterativeFlag>(reader, buf));
        EXPECT_EQ(
            this->underTest.template get<field1>(), this->traits.result);

        EXPECT_EQ(this->underTest.get(field1{}), this->traits.result);
    }

    TYPED_TEST(AFieldInAnObject, canWrite)
    {
        this->underTest.get(field1{}) = this->traits.result;
        
        rj::StringBuffer buffer;
        rj::Writer<rj::StringBuffer> writer(buffer);
        this->underTest.Accept(writer);
        EXPECT_STREQ(buffer.GetString(), this->traits.objectMsg);
    }

    template<typename T>
    struct AFieldInAnArray: testing::Test
    {
        using UnderTest = Array<T>;
        UnderTest underTest;
        AFieldTraits<T> traits;
    };

    TYPED_TEST_CASE(AFieldInAnArray, ForAllFieldTypes);

    TYPED_TEST(AFieldInAnArray, canBeConstructed){}

    TYPED_TEST(AFieldInAnArray, canParse)
    {
        rapidjson::StringStream buf(this->traits.arrayMsg);
        rapidjson::Reader reader;
        reader.IterativeParseInit();
        ASSERT_TRUE(this->underTest.template Parse<rj::kParseIterativeFlag>(reader, buf));
        ASSERT_EQ(this->underTest.size(), 1u);
        EXPECT_EQ(this->traits.get(this->underTest[0]), this->traits.result);
    }

    TYPED_TEST(AFieldInAnArray, canWrite)
    {
        this->underTest.push_back(TypeParam{this->traits.result});
        
        rj::StringBuffer buffer;
        rj::Writer<rj::StringBuffer> writer(buffer);
        this->underTest.Accept(writer);
        EXPECT_STREQ(buffer.GetString(), this->traits.arrayMsg);
    }

}
#endif
