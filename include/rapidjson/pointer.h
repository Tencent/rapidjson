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

template <typename ValueType, typename Allocator = CrtAllocator>
class GenericPointer {
public:
    typedef typename ValueType::EncodingType EncodingType;
    typedef typename EncodingType::Ch Ch;

    struct Token {
        const Ch* name;
        SizeType length;
        SizeType index;             //!< A valid index if not equal to kInvalidIndex.
    };

    static const SizeType kInvalidIndex = ~SizeType(0);

    GenericPointer()
        : allocator_(),
        ownAllocator_(),
        nameBuffer_(),
        tokens_(),
        tokenCount_(),
        valid_(true)
    {
    }

    GenericPointer(const Ch* source, Allocator* allocator = 0) 
        : allocator_(allocator),
          ownAllocator_(),
          nameBuffer_(),
          tokens_(),
          tokenCount_(),
          valid_(true)
    {
        Parse(source, internal::StrLen(source));
    }

    GenericPointer(const Ch* source, size_t length, Allocator* allocator = 0)
        : allocator_(allocator),
          ownAllocator_(),
          nameBuffer_(),
          tokens_(),
          tokenCount_(),
          valid_(true)
    {
        Parse(source, length);
    }

    GenericPointer(const Token* tokens, size_t tokenCount)
        : allocator_(),
          ownAllocator_(),
          nameBuffer_(),
          tokens_(const_cast<Token*>(tokens)),
          tokenCount_(tokenCount),
          valid_(true)
    {
    }

    GenericPointer(const GenericPointer& rhs)
        : allocator_(),
          ownAllocator_(),
          nameBuffer_(),
          tokens_(),
          tokenCount_(),
          valid_()
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
        valid_ = rhs.valid_;

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

    bool IsValid() const { return valid_; }

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
            if (v->GetType() != kObjectType && v->GetType() != kArrayType) {
                if (t->index == kInvalidIndex)
                    v->SetObject();
                else
                    v->SetArray();
            }

            switch (v->GetType()) {
            case kObjectType:
                {
                    typename ValueType::MemberIterator m = v->FindMember(GenericStringRef<Ch>(t->name, t->length));
                    if (m == v->MemberEnd()) {
                        v->AddMember(Value(t->name, t->length, allocator).Move(), Value().Move(), allocator);
                        v = &(--v->MemberEnd())->value; // Assumes AddMember() appends at the end
                        exist = false;
                    }
                    else
                        v = &m->value;
                }
                break;
            case kArrayType:
                if (t->index == kInvalidIndex) 
                    v->SetArray(); // Change to Array
                if (t->index >= v->Size()) {
                    v->Reserve(t->index + 1, allocator);
                    while (t->index >= v->Size())
                        v->PushBack(Value().Move(), allocator);
                    exist = false;
                }
                v = &((*v)[t->index]);
                break;
            default:
                // Impossible.
                RAPIDJSON_ASSERT(false);
                break;
            }
        }

        if (alreadyExist)
            *alreadyExist = exist;

        return *v;
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
                if (t->index == kInvalidIndex || t->index >= v->Size())
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

    // Move semantics, create parents if non-exist
    ValueType& Set(ValueType& root, ValueType& value, typename ValueType::AllocatorType& allocator) const {
        return Create(root, allocator) = value;
    }

    // Create parents if non-exist
    ValueType& Swap(ValueType& root, ValueType& value, typename ValueType::AllocatorType& allocator) const {
        return Create(root, allocator).Swap(value);
    }

private:
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

        for (size_t i = 0; i < length;) {
            if (source[i++] != '/') // Consumes '/'
                goto error;

            Token& token = tokens_[tokenCount_++];
            token.name = name;
            bool isNumber = true;

            while (i < length && source[i] != '/') {
                Ch c = source[i++];
                
                // Escaping "~0" -> '~', "~1" -> '/'
                if (c == '~') {
                    if (i < length) {
                        c = source[i++];
                        if (c == '0')       c = '~';
                        else if (c == '1')  c = '/';
                        else                goto error;
                    }
                    else
                        goto error;
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

            token.index = isNumber ? n : kInvalidIndex;
        }

        RAPIDJSON_ASSERT(name <= nameBuffer_ + length); // Should not overflow buffer
        tokens_ = (Token*)allocator_->Realloc(tokens_, length * sizeof(Token), tokenCount_ * sizeof(Token)); // Shrink tokens_
        return;

    error:
        Allocator::Free(nameBuffer_);
        Allocator::Free(tokens_);
        nameBuffer_ = 0;
        tokens_ = 0;
        tokenCount_ = 0;
        valid_ = false;
        return;
    }

    Allocator* allocator_;
    Allocator* ownAllocator_;
    Ch* nameBuffer_;
    Token* tokens_;
    size_t tokenCount_;
    bool valid_;
};

template <typename T>
typename T::ValueType& CreateValueByPointer(T& root, const GenericPointer<typename T::ValueType>& pointer, typename T::AllocatorType& a) {
    return pointer.Create(root, a);
}

template <typename T, typename CharType, size_t N>
typename T::ValueType& CreateValueByPointer(T& root, const CharType(&source)[N], typename T::AllocatorType& a) {
    const GenericPointer<typename T::ValueType> pointer(source, N - 1);
    return pointer.Create(root, a);
}

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
    return pointer.Get(root);
}

template <typename T, typename CharType, size_t N>
const typename T::ValueType* GetValueByPointer(const T& root, const CharType(&source)[N]) {
    const GenericPointer<typename T::ValueType> pointer(source, N - 1);
    return pointer.Get(root);
}

template <typename T>
typename T::ValueType& GetValueByPointerWithDefault(T& root, const GenericPointer<typename T::ValueType>& pointer, const typename T::ValueType& defaultValue, typename T::AllocatorType& a) {
    return pointer.GetWithDefault(root, defaultValue, a);
}

template <typename T, typename CharType, size_t N>
typename T::ValueType& GetValueByPointerWithDefault(T& root, const CharType(&source)[N], const typename T::ValueType& defaultValue, typename T::AllocatorType& a) {
    const GenericPointer<typename T::ValueType> pointer(source, N - 1);
    return pointer.GetWithDefault(root, defaultValue, a);
}

template <typename T>
typename T::ValueType& SetValueByPointer(T& root, const GenericPointer<typename T::ValueType>& pointer, typename T::ValueType& value, typename T::AllocatorType& a) {
    return pointer.Set(root, value, a);
}

template <typename T, typename CharType, size_t N>
typename T::ValueType& SetValueByPointer(T& root, const CharType(&source)[N], typename T::ValueType& value, typename T::AllocatorType& a) {
    const GenericPointer<typename T::ValueType> pointer(source, N - 1);
    return pointer.Set(root, value , a);
}

template <typename T>
typename T::ValueType& SwapValueByPointer(T& root, const GenericPointer<typename T::ValueType>& pointer, typename T::ValueType& value, typename T::AllocatorType& a) {
    return pointer.Swap(root, value, a);
}

template <typename T, typename CharType, size_t N>
typename T::ValueType& SwapValueByPointer(T& root, const CharType(&source)[N], typename T::ValueType& value, typename T::AllocatorType& a) {
    const GenericPointer<typename T::ValueType> pointer(source, N - 1);
    return pointer.Swap(root, value, a);
}

typedef GenericPointer<Value> Pointer;

RAPIDJSON_NAMESPACE_END

#endif // RAPIDJSON_POINTER_H_
