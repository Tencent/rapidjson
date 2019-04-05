#include "jsonstruct.h"

#include "rapidjson/writer.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace testing;

namespace jsonstruct
{
    DEFINE_FIELD_NAME(field1);
    DEFINE_FIELD_NAME(field2);
    DEFINE_FIELD_NAME(field3);

    using Object1 = Object<Field<field2, Bool> >;
    static_assert(sizeof(Object1) == sizeof(bool), "");

    using Object2 = Object<
        Field<field2, Bool>,
        Field<field3, Int> >;
    static_assert(sizeof(Object2) == 2 * sizeof(int), "");

    /*RawNumber,*/     
    using ForAllFieldTypes = ::testing::Types<
        Bool, Int, Uint, Int64, Uint64, Double, String,
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
        EXPECT_THAT(c.get(field1{}), Eq(this->traits.result));
    }
    
    TYPED_TEST(AFieldInAnObject, canParse)
    {
        rapidjson::StringStream buf(this->traits.objectMsg);
        rapidjson::Reader reader;
        reader.IterativeParseInit();
        EXPECT_THAT(
            this->underTest.template Parse<rj::kParseIterativeFlag>(reader, buf),
            Eq(true));
        EXPECT_THAT(
            this->underTest.template get<field1>(),
            Eq(this->traits.result));

        EXPECT_THAT(this->underTest.get(field1{}),
                    Eq(this->traits.result));
    }

    TYPED_TEST(AFieldInAnObject, canWrite)
    {
        this->underTest.get(field1{}) = this->traits.result;
        
        rj::StringBuffer buffer;
        rj::Writer<rj::StringBuffer> writer(buffer);
        this->underTest.Accept(writer);
        EXPECT_THAT(buffer.GetString(), StrEq(this->traits.objectMsg));
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
        ASSERT_THAT(
            this->underTest.template Parse<rj::kParseIterativeFlag>(reader, buf),
            Eq(true));
        ASSERT_THAT(this->underTest.size(), Eq(1));
        EXPECT_THAT(this->traits.get(this->underTest[0]),
                    Eq(this->traits.result));
    }

    TYPED_TEST(AFieldInAnArray, canWrite)
    {
        this->underTest.push_back(TypeParam{this->traits.result});
        
        rj::StringBuffer buffer;
        rj::Writer<rj::StringBuffer> writer(buffer);
        this->underTest.Accept(writer);
        EXPECT_THAT(buffer.GetString(), StrEq(this->traits.arrayMsg));
    }

    DEFINE_FIELD_NAME(rate);
    DEFINE_FIELD_NAME(lockSide);
    DEFINE_FIELD_NAME(content);

    using Quote = Object<
        Field<rate, Double>,
        Field<lockSide, String> >;

    TEST(AnObject, hasGetMethod)
    {
        Quote underTest;
        underTest.get<rate>();
    }

    TEST(AnObject, canBeConstructedFromFields)
    {
        Quote underTest{3.14, "hello"};
        EXPECT_THAT(underTest.get<rate>(), Eq(3.14));
        EXPECT_THAT(underTest.get<lockSide>(), Eq("hello"));
    }

    // TEST(AnObject, getFaildsNice)
    // {
    //     Quote underTest;
    //     underTest.get<field1>();
    // }
    
    TEST(AnObject, canParse)
    {
        Quote underTest;

        auto msg = "{ \"rate\": 0.234, \"lockSide\": \"foo\" }";
        rapidjson::StringStream buf(msg);
        rapidjson::Reader reader;
        reader.IterativeParseInit();

        underTest.Parse<rj::kParseIterativeFlag>(reader, buf);
        EXPECT_THAT(underTest.get<rate>(), Eq(0.234));
        EXPECT_THAT(underTest.get<lockSide>(), Eq("foo"));
    }

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
        EXPECT_THAT(underTest.get<field1>(), Eq(1));
        EXPECT_THAT(underTest.get<field2>(), Eq(2));
    }

    struct ANestedObject: testing::Test
    {
        using UnderTest = Object<
            Field<field1, Int>,
            Field<content, Quote> >;
        UnderTest underTest;
        
        const char* msg =
            R"___({"field1":42,"content":{"rate":0.234,"lockSide":"foo"}})___";
    };

    TEST_F(ANestedObject, canParse)
    {
        rapidjson::StringStream buf(msg);
        rapidjson::Reader reader;
        reader.IterativeParseInit();

        EXPECT_THAT(
            underTest.Parse<rj::kParseIterativeFlag>(reader, buf),
            Eq(true));
        EXPECT_THAT(underTest.get<content>().get<rate>(), Eq(0.234));
        EXPECT_THAT(underTest.get<content>().get<lockSide>(), Eq("foo"));

        const auto& c = underTest.get<content>();

        EXPECT_THAT(c.get<rate>(), Eq(0.234));
        EXPECT_THAT(c.get<lockSide>(), Eq("foo")); 
    }

    TEST_F(ANestedObject, canWrite)
    {
        underTest.get(field1{}) = 42;
        underTest.get<content>().get(lockSide{}) = "foo";
        underTest.get<content>().get(rate{}) = 0.234;
        
        rj::StringBuffer buffer;
        rj::Writer<rj::StringBuffer> writer(buffer);
        this->underTest.Accept(writer);
        EXPECT_THAT(buffer.GetString(), StrEq(msg));
    }    

    TEST(AnArray, canParse)
    {
        Array<Int> underTest;

        auto msg = "[0, 1, 2, 3, 4]";
        rapidjson::StringStream buf(msg);
        rapidjson::Reader reader;
        reader.IterativeParseInit();

        underTest.Parse<rj::kParseIterativeFlag>(reader, buf);

        EXPECT_THAT(underTest.size(), Eq(5));
        EXPECT_THAT(underTest[0].value, Eq(0));
        EXPECT_THAT(underTest[1].value, Eq(1));
        EXPECT_THAT(underTest[2].value, Eq(2));
        EXPECT_THAT(underTest[3].value, Eq(3));
        EXPECT_THAT(underTest[4].value, Eq(4));
    }

    TEST(AnObjectInAnArray, canParse)
    {
        using UnderTest = Array<Quote>;
        UnderTest underTest;

        auto msg = "[{ \"rate\": 0.234, \"lockSide\": \"foo\" }]";
        rapidjson::StringStream buf(msg);
        rapidjson::Reader reader;
        reader.IterativeParseInit();

        ASSERT_TRUE(underTest.Parse<rj::kParseIterativeFlag>(reader, buf));
        EXPECT_THAT(underTest[0].get<rate>(), Eq(0.234));
        EXPECT_THAT(underTest[0].get<lockSide>(), Eq("foo"));
    }    
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
