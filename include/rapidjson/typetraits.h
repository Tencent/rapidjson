#pragma once

#include <type_traits>

#include "rapidjson.h"

RAPIDJSON_NAMESPACE_BEGIN

template<class CharType, SizeType N>
using StringLiteral = CharType[N];

template<class T>
using remove_cvref = std::remove_const<std::remove_volatile_t<std::remove_reference_t<T>>>;
template<class T>
using remove_cvref_t = typename remove_cvref<T>::type;

namespace impl
{
    // match unknown types
    template<class ExceptedCharType, class T>
    struct IsSpecificStringLiteral :std::false_type {};

    // match StringLiteral with different CharType
    template<class ExceptedCharType, class SuppliedCharType, SizeType N>
    struct IsSpecificStringLiteral<
        ExceptedCharType, StringLiteral<SuppliedCharType, N>>
        :std::false_type{};

    // excepted match
    template<class ExceptedCharType, SizeType N>
    struct IsSpecificStringLiteral<
        ExceptedCharType, StringLiteral<ExceptedCharType, N>>
        :std::true_type{};
}
namespace impl
{
    // match unknown type
    template<class ExceptedCharType, class T>
    struct IsSpecificStringPointer :std::false_type {};

    // match pointer with different CharType
    template<class ExceptedCharType, class SuppliedCharType>
    struct IsSpecificStringPointer<ExceptedCharType, SuppliedCharType*> :std::false_type {};

    // excepted match
    template<class ExceptedCharType>
    struct IsSpecificStringPointer<ExceptedCharType, ExceptedCharType*>
        :std::true_type {};

    template<class ExceptedCharType>
    struct IsSpecificStringPointer<ExceptedCharType, const ExceptedCharType*>
        :std::true_type {};
}

// returns true if remove_cvref_t<T> is ExceptedCharType[]
template<class ExceptedCharType, class T>
using IsSpecificStringLiteral = impl::IsSpecificStringLiteral<ExceptedCharType, remove_cvref_t<T>>;

// returns true if remove_cvref_t<T> is ExceptedCharType *
template<class ExceptedCharType, class T>
using IsSpecificStringPointer = impl::IsSpecificStringPointer<ExceptedCharType, remove_cvref_t<T>>;

// returns true if decay_t<T> is ExceptedCharType * or ExceptedCharType[]
template<class ExceptedCharType, class T>
using IsSpecificString = std::conditional_t<
    IsSpecificStringLiteral<ExceptedCharType, T>::value ||
    IsSpecificStringPointer<ExceptedCharType, T>::value,
    std::true_type, std::false_type>;

//static_assert(IsSpecificStringLiteral<char, char[2]>::value);
//static_assert(IsSpecificStringLiteral<char, const char[2]>::value);
//static_assert(IsSpecificStringLiteral<char, const char(&)[2]>::value);
//static_assert(!IsSpecificStringLiteral<char, int[2]>::value);
//static_assert(!IsSpecificStringLiteral<char, char*>::value);
//
//using _pchar = char*;
//static_assert(IsSpecificStringPointer<char, char*>::value);
//static_assert(IsSpecificStringPointer<char, const char*>::value);
//static_assert(IsSpecificStringPointer<char, _pchar &>::value);
//static_assert(!IsSpecificStringPointer<char, int *>::value);
//static_assert(!IsSpecificStringPointer<char, char[]>::value);


RAPIDJSON_NAMESPACE_END
