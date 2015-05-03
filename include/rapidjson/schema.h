// Tencent is pleased to support the open source community by making RapidJSON available->
// 
// Copyright (C) 2015 THL A29 Limited, a Tencent company, and Milo Yip-> All rights reserved->
//
// Licensed under the MIT License (the "License"); you may not use this file except
// in compliance with the License-> You may obtain a copy of the License at
//
// http://opensource->org/licenses/MIT
//
// Unless required by applicable law or agreed to in writing, software distributed 
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
// CONDITIONS OF ANY KIND, either express or implied-> See the License for the 
// specific language governing permissions and limitations under the License->

#ifndef RAPIDJSON_SCHEMA_H_
#define RAPIDJSON_SCHEMA_H_

#include "document.h"
#include <cmath> // HUGE_VAL, fmod

#if defined(__GNUC__)
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(effc++)
RAPIDJSON_DIAG_OFF(float-equal)
#endif

RAPIDJSON_NAMESPACE_BEGIN

enum SchemaType {
    kNullSchemaType,
    kBooleanSchemaType,
    kObjectSchemaType,
    kArraySchemaType,
    kStringSchemaType,
    kNumberSchemaType,
    kIntegerSchemaType,
    kTotalBasicSchemaType,
    kTypelessSchemaType = kTotalBasicSchemaType,
    kMultiTypeSchemaType,
};

template <typename Encoding>
class BaseSchema;

template <typename Encoding>
struct SchemaValidationContext {
    SchemaValidationContext(const BaseSchema<Encoding>* s) : schema(s), valueSchema(), multiTypeSchema() {}

    ~SchemaValidationContext() {}

    const BaseSchema<Encoding>* schema;
    const BaseSchema<Encoding>* valueSchema;
    const BaseSchema<Encoding>* multiTypeSchema;
    SizeType objectRequiredCount;
    SizeType arrayElementIndex;
};

template <typename Encoding>
class BaseSchema {
public:
    typedef typename Encoding::Ch Ch;
    typedef SchemaValidationContext<Encoding> Context;

    BaseSchema() {}

    template <typename ValueType>
    BaseSchema(const ValueType& value) {
        typename ValueType::ConstMemberIterator enumItr = value.FindMember("enum");
        if (enumItr != value.MemberEnd()) {
            if (enumItr->value.IsArray() && enumItr->value.Size() > 0)
                enum_.CopyFrom(enumItr->value, allocator_);
            else {
                // Error
            }
        }
    }
    
    virtual ~BaseSchema() {}

    virtual SchemaType GetSchemaType() const = 0;

    virtual bool HandleMultiType(Context&, SchemaType) const { return true; }
    virtual bool BeginValue(Context&) const { return true; }

    virtual bool Null() const { return !enum_.IsArray() || CheckEnum(GenericValue<Encoding>().Move()); }
    virtual bool Bool(bool b) const { return !enum_.IsArray() || CheckEnum(GenericValue<Encoding>(b).Move()); }
    virtual bool Int(int i) const { return !enum_.IsArray() || CheckEnum(GenericValue<Encoding>(i).Move()); }
    virtual bool Uint(unsigned u) const { return !enum_.IsArray() || CheckEnum(GenericValue<Encoding>(u).Move()); }
    virtual bool Int64(int64_t i) const { return !enum_.IsArray() || CheckEnum(GenericValue<Encoding>(i).Move()); }
    virtual bool Uint64(uint64_t u) const { return !enum_.IsArray() || CheckEnum(GenericValue<Encoding>(u).Move()); }
    virtual bool Double(double d) const { return !enum_.IsArray() || CheckEnum(GenericValue<Encoding>(d).Move()); }
    virtual bool String(const Ch* s, SizeType length, bool) const { return !enum_.IsArray() || CheckEnum(GenericValue<Encoding>(s, length).Move()); }
    virtual bool StartObject(Context&) const { return true; }
    virtual bool Key(Context&, const Ch*, SizeType, bool) const { return true; }
    virtual bool EndObject(Context&, SizeType) const { return true; }
    virtual bool StartArray(Context&) const { return true; }
    virtual bool EndArray(Context&, SizeType) const { return true; }

protected:
    bool CheckEnum(const GenericValue<Encoding>& v) const {
        for (typename GenericValue<Encoding>::ConstValueIterator itr = enum_.Begin(); itr != enum_.End(); ++itr)
            if (v == *itr)
                return true;
        return false;
    }

    MemoryPoolAllocator<> allocator_;
    GenericValue<Encoding> enum_;
};

template <typename Encoding, typename ValueType>
inline BaseSchema<Encoding>* CreateSchema(const ValueType& value, const ValueType& type);

template <typename Encoding, typename ValueType>
inline BaseSchema<Encoding>* CreateSchema(const ValueType& value);

template <typename Encoding>
class TypelessSchema : public BaseSchema<Encoding> {
public:
    typedef SchemaValidationContext<Encoding> Context;

    TypelessSchema() {}

    template <typename ValueType>
    TypelessSchema(const ValueType& value) : BaseSchema<Encoding>(value) {}

    virtual SchemaType GetSchemaType() const { return kTypelessSchemaType; }

    virtual bool BeginValue(Context& context) const { context.valueSchema = this; return true; }
};

template <typename Encoding>
class MultiTypeSchema : public BaseSchema<Encoding> {
public:
    typedef SchemaValidationContext<Encoding> Context;

    template <typename ValueType>
    MultiTypeSchema(const ValueType& value, const ValueType& type) : BaseSchema<Encoding>(), typedSchemas_() {
        RAPIDJSON_ASSERT(type.IsArray());
        for (typename ValueType::ConstValueIterator itr = type.Begin(); itr != type.End(); ++itr) {
            if (itr->IsString()) {
                BaseSchema<Encoding>* schema = CreateSchema<Encoding, ValueType>(value, *itr);
                SchemaType schemaType = schema->GetSchemaType();
                RAPIDJSON_ASSERT(schemaType < kTotalBasicSchemaType);
                if (typedSchemas_[schemaType] == 0)
                    typedSchemas_[schemaType] = schema;
                else {
                    // Erorr: not unique type
                }
            }
            else {
                // Error
            }
        }
    }

    ~MultiTypeSchema() {
        for (size_t i = 0; i < kTotalBasicSchemaType; i++)
            delete typedSchemas_[i];
    }

    virtual SchemaType GetSchemaType() const { return kMultiTypeSchemaType; };

    virtual bool HandleMultiType(Context& context, SchemaType schemaType) const {
        RAPIDJSON_ASSERT(schemaType < kTotalBasicSchemaType);
        if (typedSchemas_[schemaType]) {
            context.multiTypeSchema = typedSchemas_[schemaType];
            return true;
        }
        else if (schemaType == kIntegerSchemaType && typedSchemas_[kNumberSchemaType]) {
            context.multiTypeSchema = typedSchemas_[kNumberSchemaType];
            return true;
        }
        else
            return false;
    }

private:
    BaseSchema<Encoding>* typedSchemas_[kTotalBasicSchemaType];
};

template <typename Encoding>
class NullSchema : public BaseSchema<Encoding> {
public:
    typedef typename Encoding::Ch Ch;
    typedef SchemaValidationContext<Encoding> Context;

    template <typename ValueType>
    NullSchema(const ValueType& value) : BaseSchema<Encoding>(value) {}

    virtual SchemaType GetSchemaType() const { return kNullSchemaType; }

    virtual bool Null() const { return BaseSchema<Encoding>::Null(); }
    virtual bool Bool(bool) const { return false; }
    virtual bool Int(int) const { return false; }
    virtual bool Uint(unsigned) const { return false; }
    virtual bool Int64(int64_t) const { return false; }
    virtual bool Uint64(uint64_t) const { return false; }
    virtual bool Double(double) const { return false; }
    virtual bool String(const Ch*, SizeType, bool) const { return false; }
    virtual bool StartObject(Context&) const { return false; }
    virtual bool Key(Context&, const Ch*, SizeType, bool) const { return false; }
    virtual bool EndObject(Context&, SizeType) const { return false; }
    virtual bool StartArray(Context&) const { return false; }
    virtual bool EndArray(Context&, SizeType) const { return false; }
};

template <typename Encoding>
class BooleanSchema : public BaseSchema<Encoding> {
public:
    typedef typename Encoding::Ch Ch;
    typedef SchemaValidationContext<Encoding> Context;

    template <typename ValueType>
    BooleanSchema(const ValueType& value) : BaseSchema<Encoding>(value) {}

    virtual SchemaType GetSchemaType() const { return kBooleanSchemaType; }

    virtual bool Null() const { return false; }
    virtual bool Bool(bool b) const { return BaseSchema<Encoding>::Bool(b); }
    virtual bool Int(int) const { return false; }
    virtual bool Uint(unsigned) const { return false; }
    virtual bool Int64(int64_t) const { return false; }
    virtual bool Uint64(uint64_t) const { return false; }
    virtual bool Double(double) const { return false; }
    virtual bool String(const Ch*, SizeType, bool) const { return false; }
    virtual bool StartObject(Context&) const { return false; }
    virtual bool Key(Context&, const Ch*, SizeType, bool) const { return false; }
    virtual bool EndObject(Context&, SizeType) const { return false; }
    virtual bool StartArray(Context&) const { return false; }
    virtual bool EndArray(Context&, SizeType) const { return false; }
};

template <typename Encoding>
class ObjectSchema : public BaseSchema<Encoding> {
public:
    typedef typename Encoding::Ch Ch;
    typedef SchemaValidationContext<Encoding> Context;

    template <typename ValueType>
    ObjectSchema(const ValueType& value) : 
        BaseSchema<Encoding>(value),
        properties_(),
        additionalPropertySchema_(),
        propertyCount_(),
        requiredCount_(),
        minProperties_(),
        maxProperties_(SizeType(~0)),
        additionalProperty_(true)
    {
        typename ValueType::ConstMemberIterator propretiesItr = value.FindMember("properties");
        if (propretiesItr != value.MemberEnd()) {
            const ValueType& properties = propretiesItr->value;
            properties_ = new Property[properties.MemberCount()];
            propertyCount_ = 0;

            for (typename ValueType::ConstMemberIterator propertyItr = properties.MemberBegin(); propertyItr != properties.MemberEnd(); ++propertyItr) {
                properties_[propertyCount_].name.SetString(propertyItr->name.GetString(), propertyItr->name.GetStringLength(), BaseSchema<Encoding>::allocator_);
                properties_[propertyCount_].schema = CreateSchema<Encoding>(propertyItr->value);    // TODO: Check error
                propertyCount_++;
            }
        }

        // Establish required after properties
        typename ValueType::ConstMemberIterator requiredItr = value.FindMember("required");
        if (requiredItr != value.MemberEnd()) {
            if (requiredItr->value.IsArray()) {
                for (typename ValueType::ConstValueIterator itr = requiredItr->value.Begin(); itr != requiredItr->value.End(); ++itr) {
                    if (itr->IsString()) {
                        SizeType index;
                        if (FindPropertyIndex(*itr, &index)) {
                            properties_[index].required = true;
                            requiredCount_++;
                        }
                    }
                }

                if (requiredCount_ != requiredItr->value.Size()) {
                    // Error
                }
            }
        }

        typename ValueType::ConstMemberIterator additionalPropretiesItr = value.FindMember("additionalProperties");
        if (additionalPropretiesItr != value.MemberEnd()) {
            if (additionalPropretiesItr->value.IsBool())
                additionalProperty_ = additionalPropretiesItr->value.GetBool();
            else if (additionalPropretiesItr->value.IsObject())
                additionalPropertySchema_ = CreateSchema<Encoding>(additionalPropretiesItr->value);
            else {
                // Error
            }
        }

        typename ValueType::ConstMemberIterator minPropertiesItr = value.FindMember("minProperties");
        if (minPropertiesItr != value.MemberEnd()) {
            if (minPropertiesItr->value.IsUint64() && minPropertiesItr->value.GetUint64() <= SizeType(~0))
                minProperties_ = static_cast<SizeType>(minPropertiesItr->value.GetUint64());
            else {
                // Error
            }
        }

        typename ValueType::ConstMemberIterator maxPropertiesItr = value.FindMember("maxProperties");
        if (maxPropertiesItr != value.MemberEnd()) {
            if (maxPropertiesItr->value.IsUint64() && maxPropertiesItr->value.GetUint64() <= SizeType(~0))
                maxProperties_ = static_cast<SizeType>(maxPropertiesItr->value.GetUint64());
            else {
                // Error
            }
        }
    }

    ~ObjectSchema() {
        delete [] properties_;
        delete additionalPropertySchema_;
    }

    virtual SchemaType GetSchemaType() const { return kObjectSchemaType; }

    virtual bool Null() const { return false; }
    virtual bool Bool(bool) const { return false; }
    virtual bool Int(int) const { return false; }
    virtual bool Uint(unsigned) const { return false; }
    virtual bool Int64(int64_t) const { return false; }
    virtual bool Uint64(uint64_t) const { return false; }
    virtual bool Double(double) const { return false; }
    virtual bool String(const Ch*, SizeType, bool) const { return false; }

    virtual bool StartObject(Context& context) const { 
        context.objectRequiredCount = 0;
        return true; 
    }
    
    virtual bool Key(Context& context, const Ch* str, SizeType len, bool) const {
        SizeType index;
        if (FindPropertyIndex(str, len, &index)) {
            context.valueSchema = properties_[index].schema;

            if (properties_[index].required)
                context.objectRequiredCount++;

            return true;
        }

        if (additionalPropertySchema_) {
            context.valueSchema = additionalPropertySchema_;
            return true;
        }
        else if (additionalProperty_) {
            context.valueSchema = &typeless_;
            return true;
        }
        else
            return false;
    }

    virtual bool EndObject(Context& context, SizeType memberCount) const {
        return context.objectRequiredCount == requiredCount_ &&
            memberCount >= minProperties_ &&
            memberCount <= maxProperties_;
    }

    virtual bool StartArray(Context&) const { return false; }
    virtual bool EndArray(Context&, SizeType) const { return false; }

private:
    // O(n)
    template <typename ValueType>
    bool FindPropertyIndex(const ValueType& name, SizeType* outIndex) const {
        for (SizeType index = 0; index < propertyCount_; index++) {
            if (properties_[index].name == name) {
                *outIndex = index;
                return true;
            }
        }
        return false;
    }

    // O(n)
    bool FindPropertyIndex(const Ch* str, SizeType length, SizeType* outIndex) const {
        for (SizeType index = 0; index < propertyCount_; index++) {
            if (properties_[index].name.GetStringLength() == length && 
                std::memcmp(properties_[index].name.GetString(), str, length) == 0)
            {
                *outIndex = index;
                return true;
            }
        }
        return false;
    }

    struct Property {
        Property() : schema(), required(false) {}
        ~Property() { 
            delete schema;
        }

        GenericValue<Encoding> name;
        BaseSchema<Encoding>* schema;
        bool required;
    };

    TypelessSchema<Encoding> typeless_;
    Property* properties_;
    BaseSchema<Encoding>* additionalPropertySchema_;
    SizeType propertyCount_;
    SizeType requiredCount_;
    SizeType minProperties_;
    SizeType maxProperties_;
    bool additionalProperty_;
};

template <typename Encoding>
class ArraySchema : public BaseSchema<Encoding> {
public:
    typedef typename Encoding::Ch Ch;
    typedef SchemaValidationContext<Encoding> Context;

    template <typename ValueType>
    ArraySchema(const ValueType& value) : 
        BaseSchema<Encoding>(value),
        itemsList_(),
        itemsTuple_(),
        itemsTupleCount_(),
        minItems_(),
        maxItems_(SizeType(~0)),
        additionalItems_(true)
    {
        typename ValueType::ConstMemberIterator itemsItr = value.FindMember("items");
        if (itemsItr != value.MemberEnd()) {
            if (itemsItr->value.IsObject())
                itemsList_ = CreateSchema<Encoding>(itemsItr->value); // List validation
            else if (itemsItr->value.IsArray()) {
                // Tuple validation
                itemsTuple_ = new BaseSchema<Encoding>*[itemsItr->value.Size()];
                for (typename ValueType::ConstValueIterator itr = itemsItr->value.Begin(); itr != itemsItr->value.End(); ++itr) {
                    itemsTuple_[itemsTupleCount_] = CreateSchema<Encoding>(*itr);
                    itemsTupleCount_++;
                }
            }
            else {
                // Error
            }
        }

        typename ValueType::ConstMemberIterator minItemsItr = value.FindMember("minItems");
        if (minItemsItr != value.MemberEnd()) {
            if (minItemsItr->value.IsUint64() && minItemsItr->value.GetUint64() <= SizeType(~0))
                minItems_ = static_cast<SizeType>(minItemsItr->value.GetUint64());
            else {
                // Error
            }
        }

        typename ValueType::ConstMemberIterator maxItemsItr = value.FindMember("maxItems");
        if (maxItemsItr != value.MemberEnd()) {
            if (maxItemsItr->value.IsUint64() && maxItemsItr->value.GetUint64() <= SizeType(~0))
                maxItems_ = static_cast<SizeType>(maxItemsItr->value.GetUint64());
            else {
                // Error
            }
        }

        typename ValueType::ConstMemberIterator additionalItemsItr = value.FindMember("additionalItems");
        if (additionalItemsItr != value.MemberEnd()) {
            if (additionalItemsItr->value.IsBool())
                additionalItems_ = additionalItemsItr->value.GetBool();
            else {
                // Error
            }
        }
    }

    ~ArraySchema() {
        delete itemsList_;
        for (SizeType i = 0; i < itemsTupleCount_; i++)
            delete itemsTuple_[i];
        delete [] itemsTuple_;
    }

    virtual SchemaType GetSchemaType() const { return kArraySchemaType; }

    virtual bool BeginValue(Context& context) const {
        if (itemsList_)
            context.valueSchema = itemsList_;
        else if (itemsTuple_) {
            if (context.arrayElementIndex < itemsTupleCount_)
                context.valueSchema = itemsTuple_[context.arrayElementIndex];
            else if (additionalItems_)
                context.valueSchema = &typeless_;
            else
                return false;
        }
        else
            context.valueSchema = &typeless_;

        context.arrayElementIndex++;
        return true;
    }

    virtual bool Null() const { return false; }
    virtual bool Bool(bool) const { return false; }
    virtual bool Int(int) const { return false; }
    virtual bool Uint(unsigned) const { return false; }
    virtual bool Int64(int64_t) const { return false; }
    virtual bool Uint64(uint64_t) const { return false; }
    virtual bool Double(double) const { return false; }
    virtual bool String(const Ch*, SizeType, bool) const { return false; }
    virtual bool StartObject(Context&) const { return false; }
    virtual bool Key(Context&, const Ch*, SizeType, bool) const { return false; }
    virtual bool EndObject(Context&, SizeType) const { return false; }

    virtual bool StartArray(Context& context) const { 
        context.arrayElementIndex = 0;
        return true;
    }

    virtual bool EndArray(Context&, SizeType elementCount) const { 
        return elementCount >= minItems_ && elementCount <= maxItems_;
    }

private:
    TypelessSchema<Encoding> typeless_;
    BaseSchema<Encoding>* itemsList_;
    BaseSchema<Encoding>** itemsTuple_;
    SizeType itemsTupleCount_;
    SizeType minItems_;
    SizeType maxItems_;
    bool additionalItems_;
};

template <typename Encoding>
class StringSchema : public BaseSchema<Encoding> {
public:
    typedef typename Encoding::Ch Ch;
    typedef SchemaValidationContext<Encoding> Context;

    template <typename ValueType>
    StringSchema(const ValueType& value) : 
        BaseSchema<Encoding>(value),
        minLength_(0),
        maxLength_(~SizeType(0))
    {
        typename ValueType::ConstMemberIterator minLengthItr = value.FindMember("minLength");
        if (minLengthItr != value.MemberEnd()) {
            if (minLengthItr->value.IsUint64() && minLengthItr->value.GetUint64() <= ~SizeType(0))
                minLength_ = static_cast<SizeType>(minLengthItr->value.GetUint64());
            else {
                // Error
            }
        }

        typename ValueType::ConstMemberIterator maxLengthItr = value.FindMember("maxLength");
        if (maxLengthItr != value.MemberEnd()) {
            if (maxLengthItr->value.IsUint64() && maxLengthItr->value.GetUint64() <= ~SizeType(0))
                maxLength_ = static_cast<SizeType>(maxLengthItr->value.GetUint64());
            else {
                // Error
            }
        }
    }

    virtual SchemaType GetSchemaType() const { return kStringSchemaType; }

    virtual bool Null() const { return false; }
    virtual bool Bool(bool) const { return false; }
    virtual bool Int(int) const { return false; }
    virtual bool Uint(unsigned) const { return false; }
    virtual bool Int64(int64_t) const { return false; }
    virtual bool Uint64(uint64_t) const { return false; }
    virtual bool Double(double) const { return false; }

    virtual bool String(const Ch* str, SizeType length, bool copy) const {
        return BaseSchema<Encoding>::String(str, length, copy) && length >= minLength_ && length <= maxLength_;
    }

    virtual bool StartObject(Context&) const { return false; }
    virtual bool Key(Context&, const Ch*, SizeType, bool) const { return false; }
    virtual bool EndObject(Context&, SizeType) const { return false; }
    virtual bool StartArray(Context&) const { return true; }
    virtual bool EndArray(Context&, SizeType) const { return true; }

private:
    SizeType minLength_;
    SizeType maxLength_;
};

template <typename Encoding>
class IntegerSchema : public BaseSchema<Encoding> {
public:
    typedef typename Encoding::Ch Ch;
    typedef SchemaValidationContext<Encoding> Context;

    template <typename ValueType>
    IntegerSchema(const ValueType& value) :
        BaseSchema<Encoding>(value),
        multipleOf_(0),
        exclusiveMinimum_(false),
        exclusiveMaximum_(false)
    {
        typename ValueType::ConstMemberIterator minimumItr = value.FindMember("minimum");
        if (minimumItr != value.MemberEnd()) {
            if (minimumItr->value.IsInt64())
                minimum_.SetInt64(minimumItr->value.GetInt64());
            else if (minimumItr->value.IsUint64())
                minimum_.SetUint64(minimumItr->value.GetUint64());
            else {
                // Error
            }
        }

        typename ValueType::ConstMemberIterator maximumItr = value.FindMember("maximum");
        if (maximumItr != value.MemberEnd()) {
            if (maximumItr->value.IsInt64())
                maximum_.SetInt64(maximumItr->value.GetInt64());
            else if (maximumItr->value.IsUint64())
                maximum_.SetUint64(maximumItr->value.GetUint64());
            else {
                // Error
            }
        }

        typename ValueType::ConstMemberIterator exclusiveMinimumItr = value.FindMember("exclusiveMinimum");
        if (exclusiveMinimumItr != value.MemberEnd()) {
            if (exclusiveMinimumItr->value.IsBool())
                exclusiveMinimum_ = exclusiveMinimumItr->value.GetBool();
            else {
                // Error
            }
        }

        typename ValueType::ConstMemberIterator exclusiveMaximumItr = value.FindMember("exclusiveMaximum");
        if (exclusiveMaximumItr != value.MemberEnd()) {
            if (exclusiveMaximumItr->value.IsBool())
                exclusiveMaximum_ = exclusiveMaximumItr->value.GetBool();
            else {
                // Error
            }
        }

        typename ValueType::ConstMemberIterator multipleOfItr = value.FindMember("multipleOf");
        if (multipleOfItr != value.MemberEnd()) {
            if (multipleOfItr->value.IsUint64())
                multipleOf_ = multipleOfItr->value.GetUint64();
            else {
                // Error
            }
        }
    }

    virtual SchemaType GetSchemaType() const { return kIntegerSchemaType; }

    virtual bool Null() const { return false; }
    virtual bool Bool(bool) const { return false; }

    virtual bool Int(int i) const { return BaseSchema<Encoding>::Int64(i) && Int64(i); }
    virtual bool Uint(unsigned u) const { return BaseSchema<Encoding>::Uint64(u) && Uint64(u); }
    virtual bool Int64(int64_t i) const { return BaseSchema<Encoding>::Int64(i) && CheckInt64(i); }
    virtual bool Uint64(uint64_t u) const { return BaseSchema<Encoding>::Uint64(u) && CheckUint64(u); }

    virtual bool Double(double) const { return false; }
    virtual bool String(const Ch*, SizeType, bool) const { return false; }
    virtual bool StartObject(Context&) const { return false; }
    virtual bool Key(Context&, const Ch*, SizeType, bool) const { return false; }
    virtual bool EndObject(Context&, SizeType) const { return false; }
    virtual bool StartArray(Context&) const { return false; }
    virtual bool EndArray(Context&, SizeType) const { return false; }

private:
    bool CheckInt64(int64_t i) const {
        if (!minimum_.IsNull()) {
            if (minimum_.IsInt64()) {
                if (exclusiveMinimum_ ? i <= minimum_.GetInt64() : i < minimum_.GetInt64())
                    return false;
            }
            else {
                RAPIDJSON_ASSERT(minimum_.IsUint64());
                if (i < 0 || (exclusiveMinimum_ ? static_cast<uint64_t>(i) <= minimum_.GetUint64() : static_cast<uint64_t>(i) < minimum_.GetUint64()))
                    return false;
            }
        }

        if (!maximum_.IsNull()) {
            if (maximum_.IsInt64()) {
                if (exclusiveMaximum_ ? i >= maximum_.GetInt64() : i > maximum_.GetInt64())
                    return false;
            }
            else {
                RAPIDJSON_ASSERT(maximum_.IsUint64());
                if (i >= 0 && (exclusiveMaximum_ ? static_cast<uint64_t>(i) >= maximum_.GetUint64() : static_cast<uint64_t>(i) < maximum_.GetUint64()))
                    return false;
            }
        }

        if (multipleOf_ != 0 && i % multipleOf_ != 0)
            return false;

        return true;
    }

    bool CheckUint64(uint64_t u) const {
        if (!minimum_.IsNull()) {
            if (minimum_.IsUint64()) {
                if (exclusiveMinimum_ ? u <= minimum_.GetUint64() : u < minimum_.GetUint64())
                    return false;
            }
            RAPIDJSON_ASSERT(minimum_.IsInt64() && minimum_.GetInt64() < 0); // In this case always valid
        }

        if (!maximum_.IsNull()) {
            if (maximum_.IsUint64()) {
                if (exclusiveMaximum_ ? u >= maximum_.GetUint64() : u > maximum_.GetUint64())
                    return false;
            }
            else {
                RAPIDJSON_ASSERT(maximum_.IsInt64() && minimum_.GetInt64() < 0); // In this case always invalid
                return false;
            }
        }

        if (multipleOf_ != 0 && u % multipleOf_ != 0)
            return false;

        return true;
    }

    GenericValue<Encoding> minimum_;    // Null means not specified
    GenericValue<Encoding> maximum_;    // Null means not specified
    uint64_t multipleOf_;               // 0 means not specified
    bool exclusiveMinimum_;
    bool exclusiveMaximum_;
};

template <typename Encoding>
class NumberSchema : public BaseSchema<Encoding> {
public:
    typedef typename Encoding::Ch Ch;
    typedef SchemaValidationContext<Encoding> Context;

    template <typename ValueType>
    NumberSchema(const ValueType& value) :
        BaseSchema<Encoding>(value),
        minimum_(-HUGE_VAL),
        maximum_(HUGE_VAL),
        multipleOf_(0),
        hasMultipleOf_(false),
        exclusiveMinimum_(false),
        exclusiveMaximum_(false)
    {
        typename ValueType::ConstMemberIterator minimumItr = value.FindMember("minimum");
        if (minimumItr != value.MemberEnd()) {
            if (minimumItr->value.IsNumber())
                minimum_ = minimumItr->value.GetDouble();
            else {
                // Error
            }
        }

        typename ValueType::ConstMemberIterator maximumItr = value.FindMember("maximum");
        if (maximumItr != value.MemberEnd()) {
            if (maximumItr->value.IsNumber())
                maximum_ = maximumItr->value.GetDouble();
            else {
                // Error
            }
        }

        typename ValueType::ConstMemberIterator exclusiveMinimumItr = value.FindMember("exclusiveMinimum");
        if (exclusiveMinimumItr != value.MemberEnd()) {
            if (exclusiveMinimumItr->value.IsBool())
                exclusiveMinimum_ = exclusiveMinimumItr->value.GetBool();
            else {
                // Error
            }
        }

        typename ValueType::ConstMemberIterator exclusiveMaximumItr = value.FindMember("exclusiveMaximum");
        if (exclusiveMaximumItr != value.MemberEnd()) {
            if (exclusiveMaximumItr->value.IsBool())
                exclusiveMaximum_ = exclusiveMaximumItr->value.GetBool();
            else {
                // Error
            }
        }

        typename ValueType::ConstMemberIterator multipleOfItr = value.FindMember("multipleOf");
        if (multipleOfItr != value.MemberEnd()) {
            if (multipleOfItr->value.IsNumber()) {
                multipleOf_ = multipleOfItr->value.GetDouble();
                hasMultipleOf_ = true;
            }
            else {
                // Error
            }
        }
    }

    virtual SchemaType GetSchemaType() const { return kNumberSchemaType; }

    virtual bool Null() const { return false; }
    virtual bool Bool(bool) const { return false; }

    virtual bool Int(int i) const         { return BaseSchema<Encoding>::Int(i)    && CheckDouble(i); }
    virtual bool Uint(unsigned u) const   { return BaseSchema<Encoding>::Uint(u)   && CheckDouble(u); }
    virtual bool Int64(int64_t i) const   { return BaseSchema<Encoding>::Int64(i)  && CheckDouble(i); }
    virtual bool Uint64(uint64_t u) const { return BaseSchema<Encoding>::Uint64(u) && CheckDouble(u); }
    virtual bool Double(double d) const   { return BaseSchema<Encoding>::Double(d) && CheckDouble(d); }

    virtual bool String(const Ch*, SizeType, bool) const { return false; }
    virtual bool StartObject(Context&) const { return false; }
    virtual bool Key(Context&, const Ch*, SizeType, bool) const { return false; }
    virtual bool EndObject(Context&, SizeType) const { return false; }
    virtual bool StartArray(Context&) const { return false; }
    virtual bool EndArray(Context&, SizeType) const { return false; }

private:
    bool CheckDouble(double d) const {
        if (exclusiveMinimum_ ? d <= minimum_ : d < minimum_) return false;
        if (exclusiveMaximum_ ? d >= maximum_ : d > maximum_) return false;
        if (hasMultipleOf_ && std::fmod(d, multipleOf_) != 0.0) return false;
        return true;
    }

    double minimum_;
    double maximum_;
    double multipleOf_;
    bool hasMultipleOf_;
    bool exclusiveMinimum_;
    bool exclusiveMaximum_;
};

template <typename Encoding, typename ValueType>
inline BaseSchema<Encoding>* CreateSchema(const ValueType& value, const ValueType& type) {
    if      (type == Value("null"   ).Move()) return new NullSchema<Encoding>(value);
    else if (type == Value("boolean").Move()) return new BooleanSchema<Encoding>(value);
    else if (type == Value("object" ).Move()) return new ObjectSchema<Encoding>(value);
    else if (type == Value("array"  ).Move()) return new ArraySchema<Encoding>(value);
    else if (type == Value("string" ).Move()) return new StringSchema<Encoding>(value);
    else if (type == Value("integer").Move()) return new IntegerSchema<Encoding>(value);
    else if (type == Value("number" ).Move()) return new NumberSchema<Encoding>(value);
    else                                      return 0;
}

template <typename Encoding, typename ValueType>
inline BaseSchema<Encoding>* CreateSchema(const ValueType& value) {
    if (!value.IsObject())
        return 0;

    typename ValueType::ConstMemberIterator typeItr = value.FindMember("type");

    if (typeItr == value.MemberEnd())  return new TypelessSchema<Encoding>(value);
    else if (typeItr->value.IsArray()) return new MultiTypeSchema<Encoding>(value, typeItr->value);
    else                               return CreateSchema<Encoding, ValueType>(value, typeItr->value);
}

template <typename Encoding, typename Allocator = MemoryPoolAllocator<> >
class GenericSchema {
public:
    template <typename T1, typename T2, typename T3>
    friend class GenericSchemaValidator;

    template <typename DocumentType>
    GenericSchema(const DocumentType& document) : root_() {
        root_ = CreateSchema<Encoding, typename DocumentType::ValueType>(document);
    }

    ~GenericSchema() {
        delete root_;
    }

    bool IsValid() const { return root_ != 0; }

private:
    BaseSchema<Encoding>* root_;
};

typedef GenericSchema<UTF8<> > Schema;

template <typename Encoding, typename OutputHandler = BaseReaderHandler<Encoding>, typename Allocator = CrtAllocator >
class GenericSchemaValidator {
public:
    typedef typename Encoding::Ch Ch;               //!< Character type derived from Encoding.
    typedef GenericSchema<Encoding> SchemaT;

    GenericSchemaValidator(
        const SchemaT& schema,
        Allocator* allocator = 0, 
        size_t schemaStackCapacity = kDefaultSchemaStackCapacity,
        size_t documentStackCapacity = kDefaultDocumentStackCapacity)
        :
        schema_(schema), 
        outputHandler_(nullOutputHandler_),
        schemaStack_(allocator, schemaStackCapacity),
        documentStack_(allocator, documentStackCapacity)
    {
        Reset();
    }

    GenericSchemaValidator( 
        const SchemaT& schema,
        OutputHandler& outputHandler,
        Allocator* allocator = 0,
        size_t schemaStackCapacity = kDefaultSchemaStackCapacity,
        size_t documentStackCapacity = kDefaultDocumentStackCapacity)
        :
        schema_(schema),
        outputHandler_(outputHandler),
        schemaStack_(allocator, schemaStackCapacity),
        documentStack_(allocator, documentStackCapacity)
    {
        Reset();
    }

    void Reset() {
        schemaStack_.Clear();
        documentStack_.Clear();
    };

    bool Null()               { return BeginValue(kNullSchemaType)    && CurrentSchema().Null()      && EndValue() && outputHandler_.Null();      }
    bool Bool(bool b)         { return BeginValue(kBooleanSchemaType) && CurrentSchema().Bool(b)     && EndValue() && outputHandler_.Bool(b);     }
    bool Int(int i)           { return BeginValue(kIntegerSchemaType) && CurrentSchema().Int(i)      && EndValue() && outputHandler_.Int(i);      }
    bool Uint(unsigned u)     { return BeginValue(kIntegerSchemaType) && CurrentSchema().Uint(u)     && EndValue() && outputHandler_.Uint(u);     }
    bool Int64(int64_t i64)   { return BeginValue(kIntegerSchemaType) && CurrentSchema().Int64(i64)  && EndValue() && outputHandler_.Int64(i64);  }
    bool Uint64(uint64_t u64) { return BeginValue(kIntegerSchemaType) && CurrentSchema().Uint64(u64) && EndValue() && outputHandler_.Uint64(u64); }
    bool Double(double d)     { return BeginValue(kNumberSchemaType)  && CurrentSchema().Double(d)   && EndValue() && outputHandler_.Double(d);   }
    bool String(const Ch* str, SizeType length, bool copy) { return BeginValue(kStringSchemaType) && CurrentSchema().String(str, length, copy) && EndValue() && outputHandler_.String(str, length, copy); }
    
    bool StartObject() { return BeginValue(kObjectSchemaType) && CurrentSchema().StartObject(CurrentContext()) && outputHandler_.StartObject(); }
    bool Key(const Ch* str, SizeType len, bool copy) { return CurrentSchema().Key(CurrentContext(), str, len, copy) && outputHandler_.Key(str, len, copy); }
    bool EndObject(SizeType memberCount) { return CurrentSchema().EndObject(CurrentContext(), memberCount) && EndValue() && outputHandler_.EndObject(memberCount); }
    
    bool StartArray() { return BeginValue(kArraySchemaType) && CurrentSchema().StartArray(CurrentContext()) ? outputHandler_.StartArray() : false; }
    bool EndArray(SizeType elementCount) { return CurrentSchema().EndArray(CurrentContext(), elementCount) && EndValue() && outputHandler_.EndArray(elementCount); }

private:
    typedef BaseSchema<Encoding> BaseSchemaType;
    typedef typename BaseSchemaType::Context Context;

    bool BeginValue(SchemaType schemaType) {
        if (schemaStack_.Empty())
            PushSchema(*schema_.root_);
        else {
            if (!CurrentSchema().BeginValue(CurrentContext()))
                return false;

            if (CurrentContext().valueSchema)
                PushSchema(*CurrentContext().valueSchema);
        }

        if (!CurrentSchema().HandleMultiType(CurrentContext(), schemaType))
            return false;

        if (CurrentContext().multiTypeSchema)
            PushSchema(*CurrentContext().multiTypeSchema);

        return true;
    }

    bool EndValue() {
        PopSchema();
        if (!schemaStack_.Empty() && CurrentContext().multiTypeSchema)
             PopSchema();
        return true;
    }

    void PushSchema(const BaseSchemaType& schema) { *schemaStack_.template Push<Context>() = Context(&schema); }
    const BaseSchemaType& PopSchema() { return *schemaStack_.template Pop<Context>(1)->schema; }
    const BaseSchemaType& CurrentSchema() { return *schemaStack_.template Top<Context>()->schema; }
    Context& CurrentContext() { return *schemaStack_.template Top<Context>(); }

    static const size_t kDefaultSchemaStackCapacity = 256;
    static const size_t kDefaultDocumentStackCapacity = 256;
    const SchemaT& schema_;
    BaseReaderHandler<Encoding> nullOutputHandler_;
    OutputHandler& outputHandler_;
    internal::Stack<Allocator> schemaStack_;    //!< stack to store the current path of schema (BaseSchemaType *)
    internal::Stack<Allocator> documentStack_;  //!< stack to store the current path of validating document (Value *)
};

typedef GenericSchemaValidator<UTF8<> > SchemaValidator;

RAPIDJSON_NAMESPACE_END

#if defined(__GNUC__)
RAPIDJSON_DIAG_POP
#endif

#endif // RAPIDJSON_SCHEMA_H_
