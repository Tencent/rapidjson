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

#ifndef RAPIDJSON_POINTER_H_
#define RAPIDJSON_POINTER_H_

#include "document.h"

RAPIDJSON_NAMESPACE_BEGIN

static const SizeType kPointerInvalidIndex = ~SizeType(0);

enum PointerParseErrorCode {
    kPointerParseErrorNone = 0,

    kPointerParseErrorTokenMustBeginWithSolidus,
    kPointerParseErrorInvalidEscape,
    kPointerParseErrorInvalidPercentEncoding,
    kPointerParseErrorCharacterMustPercentEncode
};

template <typename ValueType, typename Allocator = CrtAllocator>
class GenericPointer {
public:
    typedef typename ValueType::EncodingType EncodingType;
    typedef typename EncodingType::Ch Ch;

    struct Token {
        const Ch* name;
        SizeType length;
        SizeType index;             //!< A valid index if not equal to kPointerInvalidIndex.
    };

    GenericPointer() :
        allocator_(),
        ownAllocator_(),
        nameBuffer_(),
        tokens_(),
        tokenCount_(),
        parseErrorOffset_(),
        parseErrorCode_(kPointerParseErrorNone)
    {
    }

    explicit GenericPointer(const Ch* source, Allocator* allocator = 0) :
        allocator_(allocator),
        ownAllocator_(),
        nameBuffer_(),
        tokens_(),
        tokenCount_(),
        parseErrorOffset_(),
        parseErrorCode_(kPointerParseErrorNone)
    {
        Parse(source, internal::StrLen(source));
    }

    GenericPointer(const Ch* source, size_t length, Allocator* allocator = 0) :
        allocator_(allocator),
        ownAllocator_(),
        nameBuffer_(),
        tokens_(),
        tokenCount_(),
        parseErrorOffset_(),
        parseErrorCode_(kPointerParseErrorNone)
    {
        Parse(source, length);
    }

    GenericPointer(const Token* tokens, size_t tokenCount) :
        allocator_(),
        ownAllocator_(),
        nameBuffer_(),
        tokens_(const_cast<Token*>(tokens)),
        tokenCount_(tokenCount),
        parseErrorOffset_(),
        parseErrorCode_(kPointerParseErrorNone)
    {
    }

    GenericPointer(const GenericPointer& rhs) : 
        allocator_(),
        ownAllocator_(),
        nameBuffer_(),
        tokens_(),
        tokenCount_(),
        parseErrorOffset_(),
        parseErrorCode_(kPointerParseErrorNone)
    {
        *this = rhs;
    }

    ~GenericPointer() {
        if (nameBuffer_) {
            Allocator::Free(nameBuffer_);
            Allocator::Free(tokens_);
        }
        RAPIDJSON_DELETE(ownAllocator_);
    }

    GenericPointer& operator=(const GenericPointer& rhs) {
        this->~GenericPointer();

        tokenCount_ = rhs.tokenCount_;
        parseErrorOffset_ = rhs.parseErrorOffset_;
        parseErrorCode_ = rhs.parseErrorCode_;

        if (rhs.nameBuffer_) {
            if (!allocator_)
                ownAllocator_ = allocator_ = RAPIDJSON_NEW(Allocator());

            size_t nameBufferSize = tokenCount_; // null terminators
            for (Token *t = rhs.tokens_; t != rhs.tokens_ + tokenCount_; ++t)
                nameBufferSize += t->length;
            nameBuffer_ = (Ch*)allocator_->Malloc(nameBufferSize * sizeof(Ch));
            std::memcpy(nameBuffer_, rhs.nameBuffer_, nameBufferSize);

            tokens_ = (Token*)allocator_->Malloc(tokenCount_ * sizeof(Token));
            std::memcpy(tokens_, rhs.tokens_, tokenCount_ * sizeof(Token));

            // Adjust pointers to name buffer
            std::ptrdiff_t diff = nameBuffer_ - rhs.nameBuffer_;
            for (Token *t = rhs.tokens_; t != rhs.tokens_ + tokenCount_; ++t)
                t->name += diff;
        }
        else
            tokens_ = rhs.tokens_;

        return *this;
    }

    bool IsValid() const { return parseErrorCode_ == kPointerParseErrorNone; }

    size_t GetParseErrorOffset() const { return parseErrorOffset_; }

    PointerParseErrorCode GetParseErrorCode() const { return parseErrorCode_; }

    const Token* GetTokens() const { return tokens_; }

    size_t GetTokenCount() const { return tokenCount_; }

    template<typename OutputStream>
    void Stringify(OutputStream& os) const {
        RAPIDJSON_ASSERT(IsValid());
        for (Token *t = tokens_; t != tokens_ + tokenCount_; ++t) {
            os.Put('/');
            for (size_t j = 0; j < t->length; j++) {
                Ch c = t->name[j];
                if      (c == '~') { os.Put('~'); os.Put('0'); }
                else if (c == '/') { os.Put('~'); os.Put('1'); }
                else os.Put(c);
            }
        }
    }

    ValueType& Create(ValueType& root, typename ValueType::AllocatorType& allocator, bool* alreadyExist = 0) const {
        RAPIDJSON_ASSERT(IsValid());
        ValueType* v = &root;
        bool exist = true;
        for (Token *t = tokens_; t != tokens_ + tokenCount_; ++t) {
            if (t->index == kPointerInvalidIndex) { // object name
                // Handling of '-' for last element of array
                if (t->name[0] == '-' && t->length == 1) {
                    if (!v->IsArray())
                        v->SetArray(); // Change to Array
                    v->PushBack(Value().Move(), allocator);
                    v = &((*v)[v->Size() - 1]);
                    exist = false;
                }
                else {
                    if (!v->IsObject())
                        v->SetObject(); // Change to Object

                    typename ValueType::MemberIterator m = v->FindMember(GenericStringRef<Ch>(t->name, t->length));
                    if (m == v->MemberEnd()) {
                        v->AddMember(Value(t->name, t->length, allocator).Move(), Value().Move(), allocator);
                        v = &(--v->MemberEnd())->value; // Assumes AddMember() appends at the end
                        exist = false;
                    }
                    else
                        v = &m->value;
                }
            }
            else { // array index
                if (!v->IsArray())
                    v->SetArray(); // Change to Array

                if (t->index >= v->Size()) {
                    v->Reserve(t->index + 1, allocator);
                    while (t->index >= v->Size())
                        v->PushBack(Value().Move(), allocator);
                    exist = false;
                }
                v = &((*v)[t->index]);
            }
        }

        if (alreadyExist)
            *alreadyExist = exist;

        return *v;
    }

    template <typename stackAllocator>
    ValueType& Create(GenericDocument<EncodingType, typename ValueType::AllocatorType, stackAllocator>& root, bool* alreadyExist = 0) const {
        return Create(root, root.GetAllocator(), alreadyExist);
    }

    ValueType* Get(ValueType& root) const {
        RAPIDJSON_ASSERT(IsValid());
        ValueType* v = &root;
        for (Token *t = tokens_; t != tokens_ + tokenCount_; ++t) {
            switch (v->GetType()) {
            case kObjectType:
                {
                    typename ValueType::MemberIterator m = v->FindMember(GenericStringRef<Ch>(t->name, t->length));
                    if (m == v->MemberEnd())
                        return 0;
                    v = &m->value;
                }
                break;
            case kArrayType:
                if (t->index == kPointerInvalidIndex || t->index >= v->Size())
                    return 0;
                v = &((*v)[t->index]);
                break;
            default:
                return 0;
            }
        }
        return v;
    }

    const ValueType* Get(const ValueType& root) const {
        return Get(const_cast<ValueType&>(root));
    }

    ValueType& GetWithDefault(ValueType& root, const ValueType& defaultValue, typename ValueType::AllocatorType& allocator) const {
        bool alreadyExist;
        Value& v = Create(root, allocator, &alreadyExist);
        if (!alreadyExist) {
            Value clone(defaultValue, allocator);
            v = clone;
        }
        return v;
    }

    ValueType& GetWithDefault(ValueType& root, GenericStringRef<Ch> defaultValue, typename ValueType::AllocatorType& allocator) const {
        ValueType v(defaultValue);
        return GetWithDefault(root, v, allocator);
    }

    ValueType& GetWithDefault(ValueType& root, const Ch* defaultValue, typename ValueType::AllocatorType& allocator) const {
        bool alreadyExist;
        Value& v = Create(root, allocator, &alreadyExist);
        if (!alreadyExist) {
            Value clone(defaultValue, allocator); // This has overhead, so do it inside if.
            v = clone;
        }
        return v;
    }

    template <typename T>
    RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T>, internal::IsGenericValue<T> >), (ValueType&))
    GetWithDefault(ValueType& root, T defaultValue, typename ValueType::AllocatorType& allocator) const {
        ValueType v(defaultValue);
        return GetWithDefault(root, v, allocator);
    }

    template <typename stackAllocator>
    ValueType& GetWithDefault(GenericDocument<EncodingType, typename ValueType::AllocatorType, stackAllocator>& root, const ValueType& defaultValue) const {
        return GetWithDefault(root, defaultValue, root.GetAllocator());
    }

    template <typename stackAllocator>
    ValueType& GetWithDefault(GenericDocument<EncodingType, typename ValueType::AllocatorType, stackAllocator>& root, GenericStringRef<Ch> defaultValue) const {
        return GetWithDefault(root, defaultValue, root.GetAllocator());
    }

    template <typename stackAllocator>
    ValueType& GetWithDefault(GenericDocument<EncodingType, typename ValueType::AllocatorType, stackAllocator>& root, const Ch* defaultValue) const {
        return GetWithDefault(root, defaultValue, root.GetAllocator());
    }
    
    template <typename T, typename stackAllocator>
    RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T>, internal::IsGenericValue<T> >), (ValueType&))
    GetWithDefault(GenericDocument<EncodingType, typename ValueType::AllocatorType, stackAllocator>& root, T defaultValue) const {
        return GetWithDefault(root, defaultValue, root.GetAllocator());
    }

    // Move semantics, create parents if non-exist
    ValueType& Set(ValueType& root, ValueType& value, typename ValueType::AllocatorType& allocator) const {
        return Create(root, allocator) = value;
    }

    // Copy semantics, create parents if non-exist
    ValueType& Set(ValueType& root, const ValueType& value, typename ValueType::AllocatorType& allocator) const {
        return Create(root, allocator).CopyFrom(value, allocator);
    }

    ValueType& Set(ValueType& root, GenericStringRef<Ch> value, typename ValueType::AllocatorType& allocator) const {
        ValueType v(value);
        return Create(root, allocator) = v;
    }

    ValueType& Set(ValueType& root, const Ch* value, typename ValueType::AllocatorType& allocator) const {
        ValueType v(value, allocator);
        return Create(root, allocator) = v;
    }

    template <typename T>
    RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T>, internal::IsGenericValue<T> >), (ValueType&))
    Set(ValueType& root, T value, typename ValueType::AllocatorType& allocator) const {
        ValueType v(value);
        return Create(root, allocator) = v;
    }

    template <typename stackAllocator>
    ValueType& Set(GenericDocument<EncodingType, typename ValueType::AllocatorType, stackAllocator>& root, ValueType& value) const {
        return Create(root) = value;
    }

    template <typename stackAllocator>
    ValueType& Set(GenericDocument<EncodingType, typename ValueType::AllocatorType, stackAllocator>& root, const ValueType& value) const {
        return Create(root).CopyFrom(value, root.GetAllocator());
    }

    template <typename stackAllocator>
    ValueType& Set(GenericDocument<EncodingType, typename ValueType::AllocatorType, stackAllocator>& root, GenericStringRef<Ch> value) const {
        ValueType v(value);
        return Create(root) = v;
    }

    template <typename stackAllocator>
    ValueType& Set(GenericDocument<EncodingType, typename ValueType::AllocatorType, stackAllocator>& root, const Ch* value) const {
        ValueType v(value, root.GetAllocator());
        return Create(root) = v;
    }

    template <typename T, typename stackAllocator>
    RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T>, internal::IsGenericValue<T> >), (ValueType&))
    Set(GenericDocument<EncodingType, typename ValueType::AllocatorType, stackAllocator>& root, T value) const {
        ValueType v(value);
        return Create(root) = v;
    }

    // Create parents if non-exist
    ValueType& Swap(ValueType& root, ValueType& value, typename ValueType::AllocatorType& allocator) const {
        return Create(root, allocator).Swap(value);
    }

    template <typename stackAllocator>
    ValueType& Swap(GenericDocument<EncodingType, typename ValueType::AllocatorType, stackAllocator>& root, ValueType& value) const {
        return Create(root).Swap(value);
    }

private:
    //! Parse a JSON String or its URI fragment representation into tokens.
    /*!
        \param source Either a JSON Pointer string, or its URI fragment representation. Not need to be null terminated.
        \param length Length of the source string.
        \note Source cannot be JSON String Representation of JSON Pointer, e.g. In "/\u0000", \u0000 will not be unescaped.
    */
    void Parse(const Ch* source, size_t length) {
        // Create own allocator if user did not supply.
        if (!allocator_)
            ownAllocator_ = allocator_ = RAPIDJSON_NEW(Allocator());

        // Create a buffer as same size of source
        RAPIDJSON_ASSERT(nameBuffer_ == 0);
        nameBuffer_ = (Ch*)allocator_->Malloc(length * sizeof(Ch));

        RAPIDJSON_ASSERT(tokens_ == 0);
        tokens_ = (Token*)allocator_->Malloc(length * sizeof(Token)); // Maximum possible tokens in the source

        tokenCount_ = 0;
        Ch* name = nameBuffer_;

        size_t i = 0;

        // Detect if it is a URI fragment
        bool uriFragment = false;
        if (source[i] == '#') {
            uriFragment = true;
            i++;
        }

        if (i != length && source[i] != '/') {
            parseErrorCode_ = kPointerParseErrorTokenMustBeginWithSolidus;
            goto error;
        }

        while (i < length) {
            RAPIDJSON_ASSERT(source[i] == '/');
            i++; // consumes '/'

            Token& token = tokens_[tokenCount_++];
            token.name = name;
            bool isNumber = true;

            while (i < length && source[i] != '/') {
                Ch c = source[i++];

                if (uriFragment) {
                    // Decoding percent-encoding for URI fragment
                    if (c == '%') {
                        c = 0;
                        for (int j = 0; j < 2; j++) {
                            c <<= 4;
                            Ch h = source[i];
                            if      (h >= '0' && h <= '9') c += h - '0';
                            else if (h >= 'A' && h <= 'F') c += h - 'A' + 10;
                            else if (h >= 'a' && h <= 'f') c += h - 'a' + 10;
                            else {
                                parseErrorCode_ = kPointerParseErrorInvalidPercentEncoding;
                                goto error;
                            }
                            i++;
                        }
                    }
                    else if (!((c >= '0' && c <= '9') || (c >= 'A' && c <='Z') || (c >= 'a' && c <= 'z') || c == '-' || c == '.' || c == '_' || c =='~')) {
                        // RFC 3986 2.3 Unreserved Characters
                        i--;
                        parseErrorCode_ = kPointerParseErrorCharacterMustPercentEncode;
                        goto error;
                    }
                }
                
                // Escaping "~0" -> '~', "~1" -> '/'
                if (c == '~') {
                    if (i < length) {
                        c = source[i];
                        if (c == '0')       c = '~';
                        else if (c == '1')  c = '/';
                        else {
                            parseErrorCode_ = kPointerParseErrorInvalidEscape;
                            goto error;
                        }
                        i++;
                    }
                    else {
                        parseErrorCode_ = kPointerParseErrorInvalidEscape;
                        goto error;
                    }
                }

                // First check for index: all of characters are digit
                if (c < '0' || c > '9')
                    isNumber = false;

                *name++ = c;
            }
            token.length = name - token.name;
            *name++ = '\0'; // Null terminator

            // Second check for index: more than one digit cannot have leading zero
            if (isNumber && token.length > 1 && token.name[0] == '0')
                isNumber = false;

            // String to SizeType conversion
            SizeType n = 0;
            if (isNumber) {
                for (size_t j = 0; j < token.length; j++) {
                    SizeType m = n * 10 + static_cast<SizeType>(token.name[j] - '0');
                    if (m < n) {   // overflow detection
                        isNumber = false;
                        break;
                    }
                    n = m;
                }
            }

            token.index = isNumber ? n : kPointerInvalidIndex;
        }

        RAPIDJSON_ASSERT(name <= nameBuffer_ + length); // Should not overflow buffer
        tokens_ = (Token*)allocator_->Realloc(tokens_, length * sizeof(Token), tokenCount_ * sizeof(Token)); // Shrink tokens_
        parseErrorCode_ = kPointerParseErrorNone;
        return;

    error:
        Allocator::Free(nameBuffer_);
        Allocator::Free(tokens_);
        nameBuffer_ = 0;
        tokens_ = 0;
        tokenCount_ = 0;
        parseErrorOffset_ = i;
        return;
    }

    Allocator* allocator_;
    Allocator* ownAllocator_;
    Ch* nameBuffer_;
    Token* tokens_;
    size_t tokenCount_;
    size_t parseErrorOffset_;
    PointerParseErrorCode parseErrorCode_;
};

typedef GenericPointer<Value> Pointer;

//////////////////////////////////////////////////////////////////////////////

template <typename T>
typename T::ValueType& CreateValueByPointer(T& root, const GenericPointer<typename T::ValueType>& pointer, typename T::AllocatorType& a) {
    return pointer.Create(root, a);
}

template <typename T, typename CharType, size_t N>
typename T::ValueType& CreateValueByPointer(T& root, const CharType(&source)[N], typename T::AllocatorType& a) {
    const GenericPointer<typename T::ValueType> pointer(source, N - 1);
    return CreateValueByPointer(root, pointer, a);
}

// No allocator parameter

template <typename T>
typename T::ValueType& CreateValueByPointer(T& root, const GenericPointer<typename T::ValueType>& pointer) {
    return pointer.Create(root);
}

template <typename T, typename CharType, size_t N>
typename T::ValueType& CreateValueByPointer(T& root, const CharType(&source)[N]) {
    const GenericPointer<typename T::ValueType> pointer(source, N - 1);
    return CreateValueByPointer(root, pointer);
}

//////////////////////////////////////////////////////////////////////////////

template <typename T>
typename T::ValueType* GetValueByPointer(T& root, const GenericPointer<typename T::ValueType>& pointer) {
    return pointer.Get(root);
}

template <typename T>
const typename T::ValueType* GetValueByPointer(const T& root, const GenericPointer<typename T::ValueType>& pointer) {
    return pointer.Get(root);
}

template <typename T, typename CharType, size_t N>
typename T::ValueType* GetValueByPointer(T& root, const CharType (&source)[N]) {
    const GenericPointer<typename T::ValueType> pointer(source, N - 1);
    return GetValueByPointer(root, pointer);
}

template <typename T, typename CharType, size_t N>
const typename T::ValueType* GetValueByPointer(const T& root, const CharType(&source)[N]) {
    const GenericPointer<typename T::ValueType> pointer(source, N - 1);
    return GetValueByPointer(root, pointer);
}

//////////////////////////////////////////////////////////////////////////////

template <typename T>
typename T::ValueType& GetValueByPointerWithDefault(T& root, const GenericPointer<typename T::ValueType>& pointer, const typename T::ValueType& defaultValue, typename T::AllocatorType& a) {
    return pointer.GetWithDefault(root, defaultValue, a);
}

template <typename T>
typename T::ValueType& GetValueByPointerWithDefault(T& root, const GenericPointer<typename T::ValueType>& pointer, GenericStringRef<typename T::Ch> defaultValue, typename T::AllocatorType& a) {
    return pointer.GetWithDefault(root, defaultValue, a);
}

template <typename T>
typename T::ValueType& GetValueByPointerWithDefault(T& root, const GenericPointer<typename T::ValueType>& pointer, const typename T::Ch* defaultValue, typename T::AllocatorType& a) {
    return pointer.GetWithDefault(root, defaultValue, a);
}

template <typename T, typename T2>
RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T2>, internal::IsGenericValue<T2> >), (typename T::ValueType&))
GetValueByPointerWithDefault(T& root, const GenericPointer<typename T::ValueType>& pointer, T2 defaultValue, typename T::AllocatorType& a) {
    return pointer.GetWithDefault(root, defaultValue, a);
}

template <typename T, typename CharType, size_t N>
typename T::ValueType& GetValueByPointerWithDefault(T& root, const CharType(&source)[N], const typename T::ValueType& defaultValue, typename T::AllocatorType& a) {
    const GenericPointer<typename T::ValueType> pointer(source, N - 1);
    return GetValueByPointerWithDefault(root, pointer, defaultValue, a);
}

template <typename T, typename CharType, size_t N>
typename T::ValueType& GetValueByPointerWithDefault(T& root, const CharType(&source)[N], GenericStringRef<typename T::Ch> defaultValue, typename T::AllocatorType& a) {
    const GenericPointer<typename T::ValueType> pointer(source, N - 1);
    return GetValueByPointerWithDefault(root, pointer, defaultValue, a);
}

template <typename T, typename CharType, size_t N>
typename T::ValueType& GetValueByPointerWithDefault(T& root, const CharType(&source)[N], const typename T::Ch* defaultValue, typename T::AllocatorType& a) {
    const GenericPointer<typename T::ValueType> pointer(source, N - 1);
    return GetValueByPointerWithDefault(root, pointer, defaultValue, a);
}

template <typename T, typename CharType, size_t N, typename T2>
RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T2>, internal::IsGenericValue<T2> >), (typename T::ValueType&))
GetValueByPointerWithDefault(T& root, const CharType(&source)[N], T2 defaultValue, typename T::AllocatorType& a) {
    const GenericPointer<typename T::ValueType> pointer(source, N - 1);
    return GetValueByPointerWithDefault(root, pointer, defaultValue, a);
}

// No allocator parameter

template <typename T>
typename T::ValueType& GetValueByPointerWithDefault(T& root, const GenericPointer<typename T::ValueType>& pointer, const typename T::ValueType& defaultValue) {
    return pointer.GetWithDefault(root, defaultValue);
}

template <typename T>
typename T::ValueType& GetValueByPointerWithDefault(T& root, const GenericPointer<typename T::ValueType>& pointer, GenericStringRef<typename T::Ch> defaultValue) {
    return pointer.GetWithDefault(root, defaultValue);
}

template <typename T>
typename T::ValueType& GetValueByPointerWithDefault(T& root, const GenericPointer<typename T::ValueType>& pointer, const typename T::Ch* defaultValue) {
    return pointer.GetWithDefault(root, defaultValue);
}

template <typename T, typename T2>
RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T2>, internal::IsGenericValue<T2> >), (typename T::ValueType&))
GetValueByPointerWithDefault(T& root, const GenericPointer<typename T::ValueType>& pointer, T2 defaultValue) {
    return pointer.GetWithDefault(root, defaultValue);
}

template <typename T, typename CharType, size_t N>
typename T::ValueType& GetValueByPointerWithDefault(T& root, const CharType(&source)[N], const typename T::ValueType& defaultValue) {
    const GenericPointer<typename T::ValueType> pointer(source, N - 1);
    return GetValueByPointerWithDefault(root, pointer, defaultValue);
}

template <typename T, typename CharType, size_t N>
typename T::ValueType& GetValueByPointerWithDefault(T& root, const CharType(&source)[N], GenericStringRef<typename T::Ch> defaultValue) {
    const GenericPointer<typename T::ValueType> pointer(source, N - 1);
    return GetValueByPointerWithDefault(root, pointer, defaultValue);
}

template <typename T, typename CharType, size_t N>
typename T::ValueType& GetValueByPointerWithDefault(T& root, const CharType(&source)[N], const typename T::Ch* defaultValue) {
    const GenericPointer<typename T::ValueType> pointer(source, N - 1);
    return GetValueByPointerWithDefault(root, pointer, defaultValue);
}

template <typename T, typename CharType, size_t N, typename T2>
RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T2>, internal::IsGenericValue<T2> >), (typename T::ValueType&))
GetValueByPointerWithDefault(T& root, const CharType(&source)[N], T2 defaultValue) {
    const GenericPointer<typename T::ValueType> pointer(source, N - 1);
    return GetValueByPointerWithDefault(root, pointer, defaultValue);
}

//////////////////////////////////////////////////////////////////////////////

template <typename T>
typename T::ValueType& SetValueByPointer(T& root, const GenericPointer<typename T::ValueType>& pointer, typename T::ValueType& value, typename T::AllocatorType& a) {
    return pointer.Set(root, value, a);
}

template <typename T>
typename T::ValueType& SetValueByPointer(T& root, const GenericPointer<typename T::ValueType>& pointer, GenericStringRef<typename T::Ch> value, typename T::AllocatorType& a) {
    return pointer.Set(root, value, a);
}

template <typename T>
typename T::ValueType& SetValueByPointer(T& root, const GenericPointer<typename T::ValueType>& pointer, const typename T::Ch* value, typename T::AllocatorType& a) {
    return pointer.Set(root, value, a);
}

template <typename T, typename T2>
RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T2>, internal::IsGenericValue<T2> >), (typename T::ValueType&))
SetValueByPointer(T& root, const GenericPointer<typename T::ValueType>& pointer, T2 value, typename T::AllocatorType& a) {
    return pointer.Set(root, value, a);
}

template <typename T, typename CharType, size_t N>
typename T::ValueType& SetValueByPointer(T& root, const CharType(&source)[N], typename T::ValueType& value, typename T::AllocatorType& a) {
    const GenericPointer<typename T::ValueType> pointer(source, N - 1);
    return SetValueByPointer(root, pointer, value, a);
}

template <typename T, typename CharType, size_t N>
typename T::ValueType& SetValueByPointer(T& root, const CharType(&source)[N], GenericStringRef<typename T::Ch> value, typename T::AllocatorType& a) {
    const GenericPointer<typename T::ValueType> pointer(source, N - 1);
    return SetValueByPointer(root, pointer, value, a);
}

template <typename T, typename CharType, size_t N>
typename T::ValueType& SetValueByPointer(T& root, const CharType(&source)[N], const typename T::Ch* value, typename T::AllocatorType& a) {
    const GenericPointer<typename T::ValueType> pointer(source, N - 1);
    return SetValueByPointer(root, pointer, value, a);
}

template <typename T, typename CharType, size_t N, typename T2>
RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T2>, internal::IsGenericValue<T2> >), (typename T::ValueType&))
SetValueByPointer(T& root, const CharType(&source)[N], T2 value, typename T::AllocatorType& a) {
    const GenericPointer<typename T::ValueType> pointer(source, N - 1);
    return SetValueByPointer(root, pointer, value, a);
}

// No allocator parameter

template <typename T>
typename T::ValueType& SetValueByPointer(T& root, const GenericPointer<typename T::ValueType>& pointer, typename T::ValueType& value) {
    return pointer.Set(root, value);
}

template <typename T>
typename T::ValueType& SetValueByPointer(T& root, const GenericPointer<typename T::ValueType>& pointer, GenericStringRef<typename T::Ch> value) {
    return pointer.Set(root, value);
}

template <typename T>
typename T::ValueType& SetValueByPointer(T& root, const GenericPointer<typename T::ValueType>& pointer, const typename T::Ch* value) {
    return pointer.Set(root, value);
}

template <typename T, typename T2>
RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T2>, internal::IsGenericValue<T2> >), (typename T::ValueType&))
SetValueByPointer(T& root, const GenericPointer<typename T::ValueType>& pointer, T2 value) {
    return pointer.Set(root, value);
}

template <typename T, typename CharType, size_t N>
typename T::ValueType& SetValueByPointer(T& root, const CharType(&source)[N], typename T::ValueType& value) {
    const GenericPointer<typename T::ValueType> pointer(source, N - 1);
    return SetValueByPointer(root, pointer, value);
}

template <typename T, typename CharType, size_t N>
typename T::ValueType& SetValueByPointer(T& root, const CharType(&source)[N], GenericStringRef<typename T::Ch> value) {
    const GenericPointer<typename T::ValueType> pointer(source, N - 1);
    return SetValueByPointer(root, pointer, value);
}

template <typename T, typename CharType, size_t N>
typename T::ValueType& SetValueByPointer(T& root, const CharType(&source)[N], const typename T::Ch* value) {
    const GenericPointer<typename T::ValueType> pointer(source, N - 1);
    return SetValueByPointer(root, pointer, value);
}

template <typename T, typename CharType, size_t N, typename T2>
RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T2>, internal::IsGenericValue<T2> >), (typename T::ValueType&))
SetValueByPointer(T& root, const CharType(&source)[N], T2 value) {
    const GenericPointer<typename T::ValueType> pointer(source, N - 1);
    return SetValueByPointer(root, pointer, value);
}

//////////////////////////////////////////////////////////////////////////////

template <typename T>
typename T::ValueType& SwapValueByPointer(T& root, const GenericPointer<typename T::ValueType>& pointer, typename T::ValueType& value, typename T::AllocatorType& a) {
    return pointer.Swap(root, value, a);
}

template <typename T, typename CharType, size_t N>
typename T::ValueType& SwapValueByPointer(T& root, const CharType(&source)[N], typename T::ValueType& value, typename T::AllocatorType& a) {
    const GenericPointer<typename T::ValueType> pointer(source, N - 1);
    return SwapValueByPointer(root, pointer, value, a);
}

template <typename T>
typename T::ValueType& SwapValueByPointer(T& root, const GenericPointer<typename T::ValueType>& pointer, typename T::ValueType& value) {
    return pointer.Swap(root, value);
}

template <typename T, typename CharType, size_t N>
typename T::ValueType& SwapValueByPointer(T& root, const CharType(&source)[N], typename T::ValueType& value) {
    const GenericPointer<typename T::ValueType> pointer(source, N - 1);
    return SwapValueByPointer(root, pointer, value);
}

RAPIDJSON_NAMESPACE_END

#endif // RAPIDJSON_POINTER_H_