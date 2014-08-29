// Copyright (C) 2011 Milo Yip
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef RAPIDJSON_INTERNAL_META_H_
#define RAPIDJSON_INTERNAL_META_H_

#ifdef __GNUC__
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(effc++)
#endif

//@cond RAPIDJSON_INTERNAL
namespace rapidjson {
namespace internal {

template <int N> struct IntegralC { enum { Value = N }; };
template <bool Cond> struct BoolType : IntegralC<Cond> {};
struct TrueType : BoolType<true> {};
struct FalseType : BoolType<false> {};

template <typename T> struct AddConst { typedef const T Type; };
template <typename T> struct RemoveConst { typedef T Type; };
template <typename T> struct RemoveConst<const T> { typedef T Type; };

template <bool Condition, typename T1, typename T2> struct SelectIfCond;
template <typename T1, typename T2> struct SelectIfCond<true,T1,T2> { typedef T1 Type; };
template <typename T1, typename T2> struct SelectIfCond<false,T1,T2> { typedef T2 Type; };

template <typename Condition, typename T1, typename T2>
struct SelectIf : SelectIfCond<Condition::Value,T1,T2> {};

template <bool Constify, typename T>
struct MaybeAddConst : SelectIfCond<Constify, const T, T> {};

template <typename T, typename U> struct IsSame  : FalseType {};
template <typename T> struct IsSame<T,T>  : TrueType {};

template <typename T> struct IsConst : FalseType {};
template <typename T> struct IsConst<const T> : TrueType {};

template <typename T> struct IsPointer : FalseType {};
template <typename T> struct IsPointer<T*> : TrueType {};

template <typename CT, typename T>
struct IsMoreConst {
    enum { Value =
        ( IsSame< typename RemoveConst<CT>::Type, typename RemoveConst<T>::Type>::Value
        && ( IsConst<CT>::Value >= IsConst<T>::Value ) )
    };
};

template <bool Condition, typename T = void> struct EnableIfCond;
template <typename T> struct EnableIfCond<true, T> { typedef T Type; };
template <typename T> struct EnableIfCond<false, T> { /* empty */ };

template <bool Condition, typename T = void>
struct DisableIfCond : EnableIfCond<!Condition, T> {};

template <typename Condition, typename T = void>
struct EnableIf : EnableIfCond<Condition::Value, T> {};

template <typename Condition, typename T = void>
struct DisableIf : DisableIfCond<Condition::Value, T> {};

// SFINAE helpers
struct SfinaeResultTag {};
template <typename T> struct RemoveSfinaeFptr {};
template <typename T> struct RemoveSfinaeFptr<SfinaeResultTag&(*)(T)> { typedef T Type; };

#define RAPIDJSON_REMOVEFPTR_(type) \
    typename ::rapidjson::internal::RemoveSfinaeFptr \
        < ::rapidjson::internal::SfinaeResultTag&(*) type>::Type

#define RAPIDJSON_ENABLEIF(cond) \
    typename ::rapidjson::internal::EnableIf \
        <RAPIDJSON_REMOVEFPTR_(cond)>::Type * = NULL

#define RAPIDJSON_DISABLEIF_RETURN(cond,returntype) \
    typename ::rapidjson::internal::DisableIf<cond,returntype>::Type

} // namespace internal
} // namespace rapidjson
//@endcond

#ifdef __GNUC__
RAPIDJSON_DIAG_POP
#endif

#endif // RAPIDJSON_INTERNAL_META_H_
