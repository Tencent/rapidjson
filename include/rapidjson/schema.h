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
#include <cmath> // HUGE_VAL, abs, floor

#if !defined(RAPIDJSON_SCHEMA_USE_STDREGEX) && (__cplusplus >=201103L || (defined(_MSC_VER) && _MSC_VER >= 1800))
#define RAPIDJSON_SCHEMA_USE_STDREGEX 1
#else
#define RAPIDJSON_SCHEMA_USE_STDREGEX 0
#endif

#if RAPIDJSON_SCHEMA_USE_STDREGEX
#include <regex>
#endif

#if RAPIDJSON_SCHEMA_USE_STDREGEX
#define RAPIDJSON_SCHEMA_HAS_REGEX 1
#else
#define RAPIDJSON_SCHEMA_HAS_REGEX 0
#endif

#if defined(__GNUC__)
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(effc++)
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
    kTotalSchemaType
};

template <typename Encoding>
class BaseSchema;

template <typename Encoding, typename OutputHandler, typename Allocator>
class GenericSchemaValidator;

template <typename Encoding>
struct SchemaValidatorArray {
    SchemaValidatorArray() : validators(), count() {}
    ~SchemaValidatorArray() {
        for (SizeType i = 0; i < count; i++)
            delete validators[i];
        delete[] validators;
    }

    GenericSchemaValidator<Encoding, BaseReaderHandler<>, CrtAllocator>** validators;
    SizeType count;
};

template <typename Encoding>
struct BaseSchemaArray {
    BaseSchemaArray() : schemas(), count() {}
    ~BaseSchemaArray() {
        for (SizeType i = 0; i < count; i++)
            delete schemas[i];
        delete[] schemas;
    }

    BaseSchema<Encoding>** schemas;
    SizeType count;
};

enum PatternValidatorType {
    kPatternValidatorOnly,
    kPatternValidatorWithProperty,
    kPatternValidatorWithAdditionalProperty
};

template <typename Encoding>
struct SchemaValidationContext {
    SchemaValidationContext(const BaseSchema<Encoding>* s) : 
        schema(s),
        valueSchema(),
        patternPropertiesSchemas(),
        notValidator(),
        patternPropertiesSchemaCount(),
        valuePatternValidatorType(kPatternValidatorOnly),
        objectDependencies(),
        inArray(false)
    {
    }

    ~SchemaValidationContext() {
        delete notValidator;
        delete patternPropertiesSchemas;
        delete[] objectDependencies;
    }

    const BaseSchema<Encoding>* schema;
    const BaseSchema<Encoding>* valueSchema;
    SchemaValidatorArray<Encoding> allOfValidators;
    SchemaValidatorArray<Encoding> anyOfValidators;
    SchemaValidatorArray<Encoding> oneOfValidators;
    SchemaValidatorArray<Encoding> dependencyValidators;
    SchemaValidatorArray<Encoding> patternPropertiesValidators;
    const BaseSchema<Encoding>** patternPropertiesSchemas;
    GenericSchemaValidator<Encoding, BaseReaderHandler<>, CrtAllocator>* notValidator;
    SizeType patternPropertiesSchemaCount;
    PatternValidatorType valuePatternValidatorType;
    PatternValidatorType objectPatternValidatorType;
    SizeType objectRequiredCount;
    SizeType arrayElementIndex;
    bool* objectDependencies;
    bool inArray;
};

template <typename Encoding>
class BaseSchema {
public:
    typedef typename Encoding::Ch Ch;
    typedef SchemaValidationContext<Encoding> Context;

    template <typename ValueType>
    BaseSchema(const ValueType& value) : 
        not_(),
        type_((1 << kTotalSchemaType) - 1), // typeless
        properties_(),
        additionalPropertiesSchema_(),
        patternProperties_(),
        patternPropertyCount_(),
        propertyCount_(),
        requiredCount_(),
        minProperties_(),
        maxProperties_(SizeType(~0)),
        additionalProperties_(true),
        hasDependencies_(),
        hasSchemaDependencies_(),
        additionalItemsSchema_(),
        itemsList_(),
        itemsTuple_(),
        itemsTupleCount_(),
        minItems_(),
        maxItems_(SizeType(~0)),
        additionalItems_(true),
        pattern_(),
        minLength_(0),
        maxLength_(~SizeType(0)),
        minimum_(-HUGE_VAL),
        maximum_(HUGE_VAL),
        multipleOf_(0),
        hasMultipleOf_(false),
        exclusiveMinimum_(false),
        exclusiveMaximum_(false)
    {
        typedef typename ValueType::ConstValueIterator ConstValueIterator;
        typedef typename ValueType::ConstMemberIterator ConstMemberIterator;

        if (!value.IsObject())
            return;

        if (const ValueType* v = GetMember(value, "type")) {
            type_ = 0;
            if (v->IsString())
                AddType(*v);
            else if (v->IsArray())
                for (ConstValueIterator itr = v->Begin(); itr != v->End(); ++itr)
                    AddType(*itr);
        }

        if (const ValueType* v = GetMember(value, "enum"))
            if (v->IsArray() && v->Size() > 0)
                enum_.CopyFrom(*v, allocator_);

        AssigIfExist(allOf_, value, "allOf");
        AssigIfExist(anyOf_, value, "anyOf");
        AssigIfExist(oneOf_, value, "oneOf");

        if (const ValueType* v = GetMember(value, "not"))
            not_ = new BaseSchema<Encoding>(*v);

        // Object

        const ValueType* properties = GetMember(value, "properties");
        const ValueType* required = GetMember(value, "required");
        const ValueType* dependencies = GetMember(value, "dependencies");
        {
            // Gather properties from properties/required/dependencies
            typedef GenericValue<Encoding, MemoryPoolAllocator<> > SValue;
            SValue allProperties(kArrayType);

            if (properties && properties->IsObject())
                for (ConstMemberIterator itr = properties->MemberBegin(); itr != properties->MemberEnd(); ++itr)
                    AddUniqueElement(allProperties, itr->name);
            
            if (required && required->IsArray())
                for (ConstValueIterator itr = required->Begin(); itr != required->End(); ++itr)
                    if (itr->IsString())
                        AddUniqueElement(allProperties, *itr);

            if (dependencies && dependencies->IsObject())
                for (ConstMemberIterator itr = dependencies->MemberBegin(); itr != dependencies->MemberEnd(); ++itr) {
                    AddUniqueElement(allProperties, itr->name);
                    if (itr->value.IsArray())
                        for (ConstValueIterator i = itr->value.Begin(); i != itr->value.End(); ++i)
                            if (i->IsString())
                                AddUniqueElement(allProperties, *i);
                }

            if (allProperties.Size() > 0) {
                propertyCount_ = allProperties.Size();
                properties_ = new Property[propertyCount_];
                for (SizeType i = 0; i < propertyCount_; i++) {
                    properties_[i].name = allProperties[i];
                }
            }
        }

        if (properties && properties->IsObject())
            for (ConstMemberIterator itr = properties->MemberBegin(); itr != properties->MemberEnd(); ++itr) {
                SizeType index;
                if (FindPropertyIndex(itr->name, &index))
                    properties_[index].schema = new BaseSchema(itr->value);
                    properties_[index].typeless = false;
            }

        if (const ValueType* v = GetMember(value, "patternProperties")) {
            patternProperties_ = new PatternProperty[v->MemberCount()];
            patternPropertyCount_ = 0;

            for (ConstMemberIterator itr = v->MemberBegin(); itr != v->MemberEnd(); ++itr) {
                patternProperties_[patternPropertyCount_].pattern = CreatePattern(itr->name);
                patternProperties_[patternPropertyCount_].schema = new BaseSchema<Encoding>(itr->value);
                patternPropertyCount_++;
            }
        }

        if (required && required->IsArray())
            for (ConstValueIterator itr = required->Begin(); itr != required->End(); ++itr)
                if (itr->IsString()) {
                    SizeType index;
                    if (FindPropertyIndex(*itr, &index)) {
                        properties_[index].required = true;
                        requiredCount_++;
                    }
                }

        if (dependencies && dependencies->IsObject()) {
            hasDependencies_ = true;
            for (ConstMemberIterator itr = dependencies->MemberBegin(); itr != dependencies->MemberEnd(); ++itr) {
                SizeType sourceIndex;
                if (FindPropertyIndex(itr->name, &sourceIndex)) {
                    if (itr->value.IsArray()) {
                        properties_[sourceIndex].dependencies = new bool[propertyCount_];
                        std::memset(properties_[sourceIndex].dependencies, 0, sizeof(bool)* propertyCount_);
                        for (ConstValueIterator targetItr = itr->value.Begin(); targetItr != itr->value.End(); ++targetItr) {
                            SizeType targetIndex;
                            if (FindPropertyIndex(*targetItr, &targetIndex))
                                properties_[sourceIndex].dependencies[targetIndex] = true;
                        }
                    }
                    else if (itr->value.IsObject()) {
                        hasSchemaDependencies_ = true;
                        properties_[sourceIndex].dependenciesSchema = new BaseSchema<Encoding>(itr->value);
                    }
                }
            }
        }

        if (const ValueType* v = GetMember(value, "additionalProperties")) {
            if (v->IsBool())
                additionalProperties_ = v->GetBool();
            else if (v->IsObject())
                additionalPropertiesSchema_ = new BaseSchema<Encoding>(*v);
        }

        AssignIfExist(minProperties_, value, "minProperties");
        AssignIfExist(maxProperties_, value, "maxProperties");

        // Array
        if (const ValueType* v = GetMember(value, "items")) {
            if (v->IsObject()) // List validation
                itemsList_ = new BaseSchema<Encoding>(*v);
            else if (v->IsArray()) { // Tuple validation
                itemsTuple_ = new BaseSchema<Encoding>*[v->Size()];
                for (ConstValueIterator itr = v->Begin(); itr != v->End(); ++itr)
                    itemsTuple_[itemsTupleCount_++] = new BaseSchema<Encoding>(*itr);
            }
        }

        AssignIfExist(minItems_, value, "minItems");
        AssignIfExist(maxItems_, value, "maxItems");

        if (const ValueType* v = GetMember(value, "additionalItems")) {
            if (v->IsBool())
                additionalItems_ = v->GetBool();
            else if (v->IsObject())
                additionalItemsSchema_ = new BaseSchema<Encoding>(*v);
        }

        // String
        AssignIfExist(minLength_, value, "minLength");
        AssignIfExist(maxLength_, value, "maxLength");

        if (const ValueType* v = GetMember(value, "pattern"))
            pattern_ = CreatePattern(*v);

        // Number
        ConstMemberIterator minimumItr = value.FindMember("minimum");
        if (minimumItr != value.MemberEnd())
            if (minimumItr->value.IsNumber())
                minimum_ = minimumItr->value.GetDouble();

        ConstMemberIterator maximumItr = value.FindMember("maximum");
        if (maximumItr != value.MemberEnd())
            if (maximumItr->value.IsNumber())
                maximum_ = maximumItr->value.GetDouble();

        AssignIfExist(exclusiveMinimum_, value, "exclusiveMinimum");
        AssignIfExist(exclusiveMaximum_, value, "exclusiveMaximum");

        ConstMemberIterator multipleOfItr = value.FindMember("multipleOf");
        if (multipleOfItr != value.MemberEnd()) {
            if (multipleOfItr->value.IsNumber()) {
                multipleOf_ = multipleOfItr->value.GetDouble();
                hasMultipleOf_ = true;
            }
        }
    }

    ~BaseSchema() {
        delete not_;
        delete [] properties_;
        delete additionalPropertiesSchema_;
        delete [] patternProperties_;
        delete additionalItemsSchema_;
        delete itemsList_;
        for (SizeType i = 0; i < itemsTupleCount_; i++)
            delete itemsTuple_[i];
        delete [] itemsTuple_;
#if RAPIDJSON_SCHEMA_USE_STDREGEX
        delete pattern_;
#endif
    }

    bool BeginValue(Context& context) const {
        if (context.inArray) {
            if (itemsList_)
                context.valueSchema = itemsList_;
            else if (itemsTuple_) {
                if (context.arrayElementIndex < itemsTupleCount_)
                    context.valueSchema = itemsTuple_[context.arrayElementIndex];
                else if (additionalItemsSchema_)
                    context.valueSchema = additionalItemsSchema_;
                else if (additionalItems_)
                    context.valueSchema = GetTypeless();
                else
                    return false;
            }
            else
                context.valueSchema = GetTypeless();

            context.arrayElementIndex++;
        }
        return true;
    }

    bool EndValue(Context& context) const {
        if (context.patternPropertiesValidators.count > 0) {
            bool otherValid = false;
            SizeType count = context.patternPropertiesValidators.count;
            if (context.objectPatternValidatorType != kPatternValidatorOnly)
                otherValid = context.patternPropertiesValidators.validators[--count]->IsValid();

            bool patternValid = true;
            for (SizeType i = 0; i < count; i++)
                if (!context.patternPropertiesValidators.validators[i]->IsValid()) {
                    patternValid = false;
                    break;
                }

            switch (context.objectPatternValidatorType) {
            case kPatternValidatorOnly:
                if (!patternValid)
                    return false;
                break;
            case kPatternValidatorWithProperty:
                if (!patternValid || !otherValid)
                    return false;
                break;
            case kPatternValidatorWithAdditionalProperty:
                if (!patternValid && !otherValid)
                    return false;
                break;
            }
        }

        if (allOf_.schemas)
            for (SizeType i = 0; i < allOf_.count; i++)
                if (!context.allOfValidators.validators[i]->IsValid())
                    return false;
        
        if (anyOf_.schemas) {
            for (SizeType i = 0; i < anyOf_.count; i++)
                if (context.anyOfValidators.validators[i]->IsValid())
                    goto foundAny;
            return false;
            foundAny:;
        }

        if (oneOf_.schemas) {
            bool oneValid = false;
            for (SizeType i = 0; i < oneOf_.count; i++)
                if (context.oneOfValidators.validators[i]->IsValid()) {
                    if (oneValid)
                        return false;
                    else
                        oneValid = true;
                }
            if (!oneValid)
                return false;
        }

        return !not_ || !context.notValidator->IsValid();
    }

    bool Null(Context& context) const { 
        CreateParallelValidator(context);
        return
            (type_ & (1 << kNullSchemaType)) &&
            (!enum_.IsArray() || CheckEnum(GenericValue<Encoding>().Move()));
    }
    
    bool Bool(Context& context, bool b) const { 
        CreateParallelValidator(context);
        return
            (type_ & (1 << kBooleanSchemaType)) &&
            (!enum_.IsArray() || CheckEnum(GenericValue<Encoding>(b).Move()));
    }

    bool Int(Context& context, int i) const {
        CreateParallelValidator(context);
        if ((type_ & ((1 << kIntegerSchemaType) | (1 << kNumberSchemaType))) == 0)
            return false;

        return CheckDouble(i) && (!enum_.IsArray() || CheckEnum(GenericValue<Encoding>(i).Move()));
    }

    bool Uint(Context& context, unsigned u) const {
        CreateParallelValidator(context);
        if ((type_ & ((1 << kIntegerSchemaType) | (1 << kNumberSchemaType))) == 0)
            return false;

        return CheckDouble(u) && (!enum_.IsArray() || CheckEnum(GenericValue<Encoding>(u).Move()));
    }

    bool Int64(Context& context, int64_t i) const {
        CreateParallelValidator(context);
        if ((type_ & ((1 << kIntegerSchemaType) | (1 << kNumberSchemaType))) == 0)
            return false;

        return CheckDouble(i) && (!enum_.IsArray() || CheckEnum(GenericValue<Encoding>(i).Move()));
    }

    bool Uint64(Context& context, uint64_t u) const {
        CreateParallelValidator(context);
        if ((type_ & ((1 << kIntegerSchemaType) | (1 << kNumberSchemaType))) == 0)
            return false;

        return CheckDouble(u) && (!enum_.IsArray() || CheckEnum(GenericValue<Encoding>(u).Move()));
    }

    bool Double(Context& context, double d) const {
        CreateParallelValidator(context);
        if ((type_ & (1 << kNumberSchemaType)) == 0)
            return false;

        return CheckDouble(d) && (!enum_.IsArray() || CheckEnum(GenericValue<Encoding>(d).Move()));
    }
    
    bool String(Context& context, const Ch* str, SizeType length, bool) const {
        (void)str;
        CreateParallelValidator(context);
        if ((type_ & (1 << kStringSchemaType)) == 0)
            return false;

        //if (length < minLength_ || length > maxLength_)
        //    return false;
        if (minLength_ != 0 || maxLength_ != SizeType(~0)) {
            SizeType count;
            if (internal::CountStringCodePoint<Encoding>(str, length, &count) && (count < minLength_ || count > maxLength_))
                return false;
        }

        if (pattern_ && !IsPatternMatch(pattern_, str, length))
            return false;

        return !enum_.IsArray() || CheckEnum(GenericValue<Encoding>(str, length).Move());
    }

    bool StartObject(Context& context) const { 
        CreateParallelValidator(context);
        if ((type_ & (1 << kObjectSchemaType)) == 0)
            return false;

        context.objectRequiredCount = 0;
        if (hasDependencies_) {
            context.objectDependencies = new bool[propertyCount_];
            std::memset(context.objectDependencies, 0, sizeof(bool) * propertyCount_);
        }

        if (patternProperties_) { // pre-allocate schema array
            SizeType count = patternPropertyCount_ + 1; // extra for valuePatternValidatorType
            context.patternPropertiesSchemas = new const BaseSchema<Encoding>*[count];
            context.patternPropertiesSchemaCount = 0;
            std::memset(context.patternPropertiesSchemas, 0, sizeof(BaseSchema<Encoding>*) * count);
        }

        return true; 
    }
    
    bool Key(Context& context, const Ch* str, SizeType len, bool) const {
        if (patternProperties_) {
            context.patternPropertiesSchemaCount = 0;
            for (SizeType i = 0; i < patternPropertyCount_; i++)
                if (patternProperties_[i].pattern && IsPatternMatch(patternProperties_[i].pattern, str, len))
                    context.patternPropertiesSchemas[context.patternPropertiesSchemaCount++] = patternProperties_[i].schema;
        }

        SizeType index;
        if (FindPropertyIndex(str, len, &index)) {
            const BaseSchema<Encoding>* propertySchema = properties_[index].typeless ? GetTypeless() : properties_[index].schema;
            if (context.patternPropertiesSchemaCount > 0) {
                context.patternPropertiesSchemas[context.patternPropertiesSchemaCount++] = propertySchema;
                context.valueSchema = GetTypeless();
                context.valuePatternValidatorType = kPatternValidatorWithProperty;
            }
            else
                context.valueSchema = propertySchema;

            if (properties_[index].required)
                context.objectRequiredCount++;

            if (hasDependencies_)
                context.objectDependencies[index] = true;

            return true;
        }

        if (additionalPropertiesSchema_) {
            if (additionalPropertiesSchema_ && context.patternPropertiesSchemaCount > 0) {
                context.patternPropertiesSchemas[context.patternPropertiesSchemaCount++] = additionalPropertiesSchema_;
                context.valueSchema = GetTypeless();
                context.valuePatternValidatorType = kPatternValidatorWithAdditionalProperty;
            }
            else
                context.valueSchema = additionalPropertiesSchema_;
            return true;
        }
        else if (additionalProperties_) {
            context.valueSchema = GetTypeless();
            return true;
        }

        return context.patternPropertiesSchemaCount != 0; // patternProperties are not additional properties
    }

    bool EndObject(Context& context, SizeType memberCount) const {
        if (context.objectRequiredCount != requiredCount_ || memberCount < minProperties_ || memberCount > maxProperties_)
            return false;

        if (hasDependencies_) {
            for (SizeType sourceIndex = 0; sourceIndex < propertyCount_; sourceIndex++)
                if (context.objectDependencies[sourceIndex]) {
                    if (properties_[sourceIndex].dependencies) {
                        for (SizeType targetIndex = 0; targetIndex < propertyCount_; targetIndex++)
                            if (properties_[sourceIndex].dependencies[targetIndex] && !context.objectDependencies[targetIndex])
                                return false;
                    }
                    else if (properties_[sourceIndex].dependenciesSchema)
                        if (!context.dependencyValidators.validators[sourceIndex]->IsValid())
                            return false;
                }
        }

        return true;
    }

    bool StartArray(Context& context) const { 
        CreateParallelValidator(context);
        if ((type_ & (1 << kArraySchemaType)) == 0)
            return false;
        
        context.arrayElementIndex = 0;
        context.inArray = true;
        return true;
    }

    bool EndArray(Context& context, SizeType elementCount) const { 
        context.inArray = false;
        return elementCount >= minItems_ && elementCount <= maxItems_;
    }

private:
#if RAPIDJSON_SCHEMA_USE_STDREGEX
        typedef std::basic_regex<Ch> RegexType;
#else
        typedef char RegexType;
#endif

    typedef GenericSchemaValidator<Encoding, BaseReaderHandler<>, CrtAllocator> SchemaValidatorType;
    static const BaseSchema<Encoding>* GetTypeless() {
        static BaseSchema<Encoding> typeless(Value(kObjectType).Move());
        return &typeless;
    }

    template <typename V1, typename V2>
    void AddUniqueElement(V1& a, const V2& v) {
        for (typename V1::ConstValueIterator itr = a.Begin(); itr != a.End(); ++itr)
            if (*itr == v)
                return;
        V1 c(v, allocator_);
        a.PushBack(c, allocator_);
    }

    template <typename ValueType>
    static const ValueType* GetMember(const ValueType& value, const char* name) {
        typename ValueType::ConstMemberIterator itr = value.FindMember(name);
        return itr != value.MemberEnd() ? &(itr->value) : 0;
    }

    template <typename ValueType>
    static void AssignIfExist(bool& out, const ValueType& value, const char* name) {
        if (const ValueType* v = GetMember(value, name))
            if (v->IsBool())
                out = v->GetBool();
    }

    template <typename ValueType>
    static void AssignIfExist(SizeType& out, const ValueType& value, const char* name) {
        if (const ValueType* v = GetMember(value, name))
            if (v->IsUint64() && v->GetUint64() <= SizeType(~0))
                out = static_cast<SizeType>(v->GetUint64());
    }

    template <typename ValueType>
    static void AssigIfExist(BaseSchemaArray<Encoding>& out, const ValueType& value, const char* name) {
        if (const ValueType* v = GetMember(value, name))
            if (v->IsArray() && v->Size() > 0) {
                out.count = v->Size();
                out.schemas = new BaseSchema*[out.count];
                memset(out.schemas, 0, sizeof(BaseSchema*)* out.count);
                for (SizeType i = 0; i < out.count; i++)
                    out.schemas[i] = new BaseSchema<Encoding>((*v)[i]);
            }
    }

#if RAPIDJSON_SCHEMA_USE_STDREGEX
    template <typename ValueType>
    static RegexType* CreatePattern(const ValueType& value) {
        if (value.IsString())
            try {
                return new RegexType(value.GetString(), std::size_t(value.GetStringLength()), std::regex_constants::ECMAScript);
            }
            catch (const std::regex_error&) {
            }
        return 0;
    }

    static bool IsPatternMatch(const RegexType* pattern, const Ch *str, SizeType length) {
        std::match_results<const Ch*> r;
        return std::regex_search(str, str + length, r, *pattern);
    }
#else
    template <typename ValueType>
    RegexType* CreatePattern(const ValueType&) { return 0; }

    static bool IsPatternMatch(const RegexType*, const Ch *, SizeType) { return true; }
#endif // RAPIDJSON_SCHEMA_USE_STDREGEX

    void AddType(const Value& type) {
        if      (type == "null"   ) type_ |= 1 << kNullSchemaType;
        else if (type == "boolean") type_ |= 1 << kBooleanSchemaType;
        else if (type == "object" ) type_ |= 1 << kObjectSchemaType;
        else if (type == "array"  ) type_ |= 1 << kArraySchemaType;
        else if (type == "string" ) type_ |= 1 << kStringSchemaType;
        else if (type == "integer") type_ |= 1 << kIntegerSchemaType;
        else if (type == "number" ) type_ |= (1 << kNumberSchemaType) | (1 << kIntegerSchemaType);
    }

    bool CheckEnum(const GenericValue<Encoding>& v) const {
        for (typename GenericValue<Encoding>::ConstValueIterator itr = enum_.Begin(); itr != enum_.End(); ++itr)
            if (v == *itr)
                return true;
        return false;
    }

    void CreateParallelValidator(Context& context) const {
        if (allOf_.schemas) CreateSchemaValidators(context.allOfValidators, allOf_);
        if (anyOf_.schemas) CreateSchemaValidators(context.anyOfValidators, anyOf_);
        if (oneOf_.schemas) CreateSchemaValidators(context.oneOfValidators, oneOf_);
        if (not_ && !context.notValidator)
            context.notValidator = new SchemaValidatorType(*not_);

        if (hasSchemaDependencies_ && !context.dependencyValidators.validators) {
            context.dependencyValidators.validators = new SchemaValidatorType*[propertyCount_];
            context.dependencyValidators.count = propertyCount_;
            for (SizeType i = 0; i < propertyCount_; i++)
                context.dependencyValidators.validators[i] = properties_[i].dependenciesSchema ? new SchemaValidatorType(*properties_[i].dependenciesSchema) : 0;
        }
    }

    void CreateSchemaValidators(SchemaValidatorArray<Encoding>& validators, const BaseSchemaArray<Encoding>& schemas) const {
        if (!validators.validators) {
            validators.validators = new SchemaValidatorType*[schemas.count];
            validators.count = schemas.count;
            for (SizeType i = 0; i < schemas.count; i++)
                validators.validators[i] = new SchemaValidatorType(*schemas.schemas[i]);
        }
    }

    // O(n)
    template <typename ValueType>
    bool FindPropertyIndex(const ValueType& name, SizeType* outIndex) const {
        for (SizeType index = 0; index < propertyCount_; index++)
            if (properties_[index].name == name) {
                *outIndex = index;
                return true;
            }
        return false;
    }

    // O(n)
    bool FindPropertyIndex(const Ch* str, SizeType length, SizeType* outIndex) const {
        for (SizeType index = 0; index < propertyCount_; index++)
            if (properties_[index].name.GetStringLength() == length && std::memcmp(properties_[index].name.GetString(), str, length) == 0) {
                *outIndex = index;
                return true;
            }
        return false;
    }

    bool CheckDouble(double d) const {
        if (exclusiveMinimum_ ? d <= minimum_ : d < minimum_) return false;
        if (exclusiveMaximum_ ? d >= maximum_ : d > maximum_) return false;
        if (hasMultipleOf_) {
            double a = std::abs(d), b = std::abs(multipleOf_);
            double q = std::floor(a / b);
            double r = a - q * b;
            if (r > 0.0)
                return false;
        }
        return true;
    }

    struct Property {
        Property() : schema(), dependenciesSchema(), dependencies(), required(false), typeless(true) {}
        ~Property() { 
            delete schema;
            delete dependenciesSchema;
            delete[] dependencies;
        }

        GenericValue<Encoding> name;
        const BaseSchema<Encoding>* schema;
        const BaseSchema<Encoding>* dependenciesSchema;
        bool* dependencies;
        bool required;
        bool typeless;
    };

    struct PatternProperty {
        PatternProperty() : schema(), pattern() {}
        ~PatternProperty() {
            delete schema;
            delete pattern;
        }

        BaseSchema<Encoding>* schema;
        RegexType* pattern;
    };

    MemoryPoolAllocator<> allocator_;
    GenericValue<Encoding> enum_;
    BaseSchemaArray<Encoding> allOf_;
    BaseSchemaArray<Encoding> anyOf_;
    BaseSchemaArray<Encoding> oneOf_;
    BaseSchema<Encoding>* not_;
    unsigned type_; // bitmask of kSchemaType

    Property* properties_;
    BaseSchema<Encoding>* additionalPropertiesSchema_;
    PatternProperty* patternProperties_;
    SizeType patternPropertyCount_;
    SizeType propertyCount_;
    SizeType requiredCount_;
    SizeType minProperties_;
    SizeType maxProperties_;
    bool additionalProperties_;
    bool hasDependencies_;
    bool hasSchemaDependencies_;

    BaseSchema<Encoding>* additionalItemsSchema_;
    BaseSchema<Encoding>* itemsList_;
    BaseSchema<Encoding>** itemsTuple_;
    SizeType itemsTupleCount_;
    SizeType minItems_;
    SizeType maxItems_;
    bool additionalItems_;

    RegexType* pattern_;
    SizeType minLength_;
    SizeType maxLength_;

    double minimum_;
    double maximum_;
    double multipleOf_;
    bool hasMultipleOf_;
    bool exclusiveMinimum_;
    bool exclusiveMaximum_;
};

template <typename Encoding, typename Allocator = MemoryPoolAllocator<> >
class GenericSchema {
public:
    template <typename T1, typename T2, typename T3>
    friend class GenericSchemaValidator;

    template <typename DocumentType>
    GenericSchema(const DocumentType& document) : root_() {
        root_ = new BaseSchema<Encoding>(static_cast<const typename DocumentType::ValueType&>(document));
    }

    ~GenericSchema() {
        delete root_;
    }

private:
    BaseSchema<Encoding>* root_;
};

typedef GenericSchema<UTF8<> > Schema;

template <typename Encoding, typename OutputHandler = BaseReaderHandler<Encoding>, typename Allocator = CrtAllocator >
class GenericSchemaValidator {
public:
    typedef typename Encoding::Ch Ch;               //!< Character type derived from Encoding.
    typedef GenericSchema<Encoding> SchemaT;
    friend class BaseSchema<Encoding>;

    GenericSchemaValidator(
        const SchemaT& schema,
        Allocator* allocator = 0, 
        size_t schemaStackCapacity = kDefaultSchemaStackCapacity/*,
        size_t documentStackCapacity = kDefaultDocumentStackCapacity*/)
        :
        root_(*schema.root_), 
        outputHandler_(nullOutputHandler_),
        schemaStack_(allocator, schemaStackCapacity),
        // documentStack_(allocator, documentStackCapacity),
        valid_(true)
    {
    }

    GenericSchemaValidator( 
        const SchemaT& schema,
        OutputHandler& outputHandler,
        Allocator* allocator = 0,
        size_t schemaStackCapacity = kDefaultSchemaStackCapacity/*,
        size_t documentStackCapacity = kDefaultDocumentStackCapacity*/)
        :
        root_(*schema.root_), 
        outputHandler_(outputHandler),
        schemaStack_(allocator, schemaStackCapacity),
        // documentStack_(allocator, documentStackCapacity),
        valid_(true)
    {
    }

    ~GenericSchemaValidator() {
        Reset();
    }

    void Reset() {
        while (!schemaStack_.Empty())
            PopSchema();
        //documentStack_.Clear();
        valid_ = true;
    };

    bool IsValid() { return valid_; }

#define RAPIDJSON_SCHEMA_HANDLE_BEGIN_(method, arg1)\
    if (!valid_) return false; \
    if (!BeginValue() || !CurrentSchema().method arg1) return valid_ = false;

#define RAPIDJSON_SCHEMA_HANDLE_PARALLEL_(method, arg2)\
    for (Context* context = schemaStack_.template Bottom<Context>(); context <= schemaStack_.template Top<Context>(); context++) {\
        if (context->allOfValidators.validators)\
            for (SizeType i_ = 0; i_ < context->allOfValidators.count; i_++)\
                context->allOfValidators.validators[i_]->method arg2;\
        if (context->anyOfValidators.validators)\
            for (SizeType i_ = 0; i_ < context->anyOfValidators.count; i_++)\
                context->anyOfValidators.validators[i_]->method arg2;\
        if (context->oneOfValidators.validators)\
            for (SizeType i_ = 0; i_ < context->oneOfValidators.count; i_++)\
                context->oneOfValidators.validators[i_]->method arg2;\
        if (context->notValidator)\
            context->notValidator->method arg2;\
        if (context->dependencyValidators.validators)\
            for (SizeType i_ = 0; i_ < context->dependencyValidators.count; i_++)\
                if (context->dependencyValidators.validators[i_])\
                    context->dependencyValidators.validators[i_]->method arg2;\
        if (context->patternPropertiesValidators.validators)\
            for (SizeType i_ = 0; i_ < context->patternPropertiesValidators.count; i_++)\
                if (context->patternPropertiesValidators.validators[i_])\
                    context->patternPropertiesValidators.validators[i_]->method arg2; \
    }

#define RAPIDJSON_SCHEMA_HANDLE_END_(method, arg2)\
    return valid_ = EndValue() && outputHandler_.method arg2

#define RAPIDJSON_SCHEMA_HANDLE_VALUE_(method, arg1, arg2) \
    RAPIDJSON_SCHEMA_HANDLE_BEGIN_   (method, arg1);\
    RAPIDJSON_SCHEMA_HANDLE_PARALLEL_(method, arg2);\
    RAPIDJSON_SCHEMA_HANDLE_END_     (method, arg2)

    bool Null()             { RAPIDJSON_SCHEMA_HANDLE_VALUE_(Null,   (CurrentContext()   ), ( )); }
    bool Bool(bool b)       { RAPIDJSON_SCHEMA_HANDLE_VALUE_(Bool,   (CurrentContext(), b), (b)); }
    bool Int(int i)         { RAPIDJSON_SCHEMA_HANDLE_VALUE_(Int,    (CurrentContext(), i), (i)); }
    bool Uint(unsigned u)   { RAPIDJSON_SCHEMA_HANDLE_VALUE_(Uint,   (CurrentContext(), u), (u)); }
    bool Int64(int64_t i)   { RAPIDJSON_SCHEMA_HANDLE_VALUE_(Int64,  (CurrentContext(), i), (i)); }
    bool Uint64(uint64_t u) { RAPIDJSON_SCHEMA_HANDLE_VALUE_(Uint64, (CurrentContext(), u), (u)); }
    bool Double(double d)   { RAPIDJSON_SCHEMA_HANDLE_VALUE_(Double, (CurrentContext(), d), (d)); }
    bool String(const Ch* str, SizeType length, bool copy)
                                    { RAPIDJSON_SCHEMA_HANDLE_VALUE_(String, (CurrentContext(), str, length, copy), (str, length, copy)); }

    bool StartObject() {
        RAPIDJSON_SCHEMA_HANDLE_BEGIN_(StartObject, (CurrentContext()));
        RAPIDJSON_SCHEMA_HANDLE_PARALLEL_(StartObject, ());
        return valid_ = outputHandler_.StartObject();
    }
    
    bool Key(const Ch* str, SizeType len, bool copy) {
        if (!valid_) return false;
        if (!CurrentSchema().Key(CurrentContext(), str, len, copy)) return valid_ = false;
        RAPIDJSON_SCHEMA_HANDLE_PARALLEL_(Key, (str, len, copy));
        return valid_ = outputHandler_.Key(str, len, copy);
    }
    
    bool EndObject(SizeType memberCount) { 
        if (!valid_) return false;
        RAPIDJSON_SCHEMA_HANDLE_PARALLEL_(EndObject, (memberCount));
        if (!CurrentSchema().EndObject(CurrentContext(), memberCount)) return valid_ = false;
        RAPIDJSON_SCHEMA_HANDLE_END_(EndObject, (memberCount));
    }

    bool StartArray() {
        RAPIDJSON_SCHEMA_HANDLE_BEGIN_(StartArray, (CurrentContext()));
        RAPIDJSON_SCHEMA_HANDLE_PARALLEL_(StartArray, ());
        return valid_ = outputHandler_.StartArray();
    }
    
    bool EndArray(SizeType elementCount) {
        if (!valid_) return false;
        RAPIDJSON_SCHEMA_HANDLE_PARALLEL_(EndArray, (elementCount));
        if (!CurrentSchema().EndArray(CurrentContext(), elementCount)) return valid_ = false;
        RAPIDJSON_SCHEMA_HANDLE_END_(EndArray, (elementCount));
    }

#undef RAPIDJSON_SCHEMA_HANDLE_BEGIN_
#undef RAPIDJSON_SCHEMA_HANDLE_PARALLEL_
#undef RAPIDJSON_SCHEMA_HANDLE_VALUE_

    // Implementation of ISchemaValidatorFactory<Encoding>
    GenericSchemaValidator<Encoding>* CreateSchemaValidator(const BaseSchema<Encoding>& root) {
        return new GenericSchemaValidator(root);
    }

private:
    typedef BaseSchema<Encoding> BaseSchemaType;
    typedef typename BaseSchemaType::Context Context;

    GenericSchemaValidator( 
        const BaseSchemaType& root,
        Allocator* allocator = 0,
        size_t schemaStackCapacity = kDefaultSchemaStackCapacity/*,
        size_t documentStackCapacity = kDefaultDocumentStackCapacity*/)
        :
        root_(root),
        outputHandler_(nullOutputHandler_),
        schemaStack_(allocator, schemaStackCapacity),
        // documentStack_(allocator, documentStackCapacity),
        valid_(true)
    {
    }

    bool BeginValue() {
        if (schemaStack_.Empty())
            PushSchema(root_);
        else {
            if (!CurrentSchema().BeginValue(CurrentContext()))
                return false;

            SizeType count = CurrentContext().patternPropertiesSchemaCount;
            const BaseSchemaType** sa = CurrentContext().patternPropertiesSchemas;
            PatternValidatorType patternValidatorType = CurrentContext().valuePatternValidatorType;

            if (CurrentContext().valueSchema)
                PushSchema(*CurrentContext().valueSchema);

            if (count > 0) {
                CurrentContext().objectPatternValidatorType = patternValidatorType;
                SchemaValidatorArray<Encoding>& va = CurrentContext().patternPropertiesValidators;
                va.validators = new GenericSchemaValidator*[count];
                for (SizeType i = 0; i < count; i++)
                    va.validators[va.count++] = CreateSchemaValidator(*sa[i]);
            }
        }
        return true;
    }

    bool EndValue() {
        if (!CurrentSchema().EndValue(CurrentContext()))
            return false;

        PopSchema();
        return true;
    }

    void PushSchema(const BaseSchemaType& schema) { *schemaStack_.template Push<Context>() = Context(&schema); }
    void PopSchema() { schemaStack_.template Pop<Context>(1)->~Context(); }
    const BaseSchemaType& CurrentSchema() { return *schemaStack_.template Top<Context>()->schema; }
    Context& CurrentContext() { return *schemaStack_.template Top<Context>(); }

    static const size_t kDefaultSchemaStackCapacity = 1024;
    //static const size_t kDefaultDocumentStackCapacity = 256;
    const BaseSchemaType& root_;
    BaseReaderHandler<Encoding> nullOutputHandler_;
    OutputHandler& outputHandler_;
    internal::Stack<Allocator> schemaStack_;    //!< stack to store the current path of schema (BaseSchemaType *)
    //internal::Stack<Allocator> documentStack_;  //!< stack to store the current path of validating document (Value *)
    bool valid_;
};

typedef GenericSchemaValidator<UTF8<> > SchemaValidator;

RAPIDJSON_NAMESPACE_END

#if defined(__GNUC__)
RAPIDJSON_DIAG_POP
#endif

#endif // RAPIDJSON_SCHEMA_H_
