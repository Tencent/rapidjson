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

#if !defined(RAPIDJSON_SCHEMA_USE_STDREGEX) && (__cplusplus >=201103L || (defined(_MSC_VER) && _MSC_VER >= 1800))
#define RAPIDJSON_SCHEMA_USE_STDREGEX 1
#endif

#if RAPIDJSON_SCHEMA_USE_STDREGEX
#include <regex>
#endif

#if RAPIDJSON_SCHEMA_USE_STDREGEX // or some other implementation
#define RAPIDJSON_SCHEMA_HAS_REGEX 1
#else
#define RAPIDJSON_SCHEMA_HAS_REGEX 0
#endif

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
    kTotalSchemaType
};

template <typename Encoding>
class BaseSchema;

template <typename Encoding>
class ISchemaValidator {
public:
    typedef typename Encoding::Ch Ch;

    virtual ~ISchemaValidator() {};
    virtual bool IsValid() = 0;
    virtual bool Null() = 0;
    virtual bool Bool(bool) = 0;
    virtual bool Int(int) = 0;
    virtual bool Uint(unsigned) = 0;
    virtual bool Int64(int64_t) = 0;
    virtual bool Uint64(uint64_t) = 0;
    virtual bool Double(double) = 0;
    virtual bool String(const Ch*, SizeType, bool) = 0;
    virtual bool StartObject() = 0;
    virtual bool Key(const Ch*, SizeType, bool) = 0;
    virtual bool EndObject(SizeType) = 0;
    virtual bool StartArray() = 0;
    virtual bool EndArray(SizeType) = 0;
};

template <typename Encoding>
class ISchemaValidatorFactory {
public:
    virtual ~ISchemaValidatorFactory() {}
    virtual ISchemaValidator<Encoding>* CreateSchemaValidator(const BaseSchema<Encoding>& root) = 0;
};

template <typename Encoding>
struct SchemaValidatorArray {
    SchemaValidatorArray() : validators(), count() {}
    ~SchemaValidatorArray() {
        if (validators) {
            for (SizeType i = 0; i < count; i++)
                delete validators[i];
            delete[] validators;
        }
    }

    ISchemaValidator<Encoding>** validators;
    SizeType count;
};

template <typename Encoding>
struct BaseSchemaArray {
    BaseSchemaArray() : schemas(), count() {}
    ~BaseSchemaArray() {
        if (schemas) {
            for (SizeType i = 0; i < count; i++)
                delete schemas[i];
            delete[] schemas;
        }
    }

    BaseSchema<Encoding>** schemas;
    SizeType count;
};

template <typename Encoding>
struct SchemaValidationContext {
    SchemaValidationContext(ISchemaValidatorFactory<Encoding>* factory, const BaseSchema<Encoding>* s) : 
        schemaValidatorFactory(factory), schema(s), valueSchema(), multiTypeSchema(), notValidator(), objectDependencies(), inArray(false)
    {
    }

    ~SchemaValidationContext() {
        delete notValidator;
        delete[] objectDependencies;
    }

    ISchemaValidatorFactory<Encoding>* schemaValidatorFactory;
    const BaseSchema<Encoding>* schema;
    const BaseSchema<Encoding>* valueSchema;
    const BaseSchema<Encoding>* multiTypeSchema;
    SchemaValidatorArray<Encoding> allOfValidators;
    SchemaValidatorArray<Encoding> anyOfValidators;
    SchemaValidatorArray<Encoding> oneOfValidators;
    ISchemaValidator<Encoding>* notValidator;
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
        properties_(),
        additionalPropertySchema_(),
#if RAPIDJSON_SCHEMA_HAS_REGEX
        patternProperties_(),
        patternPropertyCount_(),
#endif
        propertyCount_(),
        requiredCount_(),
        minProperties_(),
        maxProperties_(SizeType(~0)),
        additionalProperty_(true),
        hasDependencies_(),
        itemsList_(),
        itemsTuple_(),
        itemsTupleCount_(),
        minItems_(),
        maxItems_(SizeType(~0)),
        additionalItems_(true),
#if RAPIDJSON_SCHEMA_USE_STDREGEX
        pattern_(),
#endif
        minLength_(0),
        maxLength_(~SizeType(0)),
        minimum_(-HUGE_VAL),
        maximum_(HUGE_VAL),
        multipleOf_(0),
        hasMultipleOf_(false),
        exclusiveMinimum_(false),
        exclusiveMaximum_(false)
    {
        type_ = (1 << kTotalSchemaType) - 1; // typeless

        typename ValueType::ConstMemberIterator typeItr = value.FindMember("type");
        if (typeItr != value.MemberEnd()) {
            if (typeItr->value.IsString()) {
                type_ = 0;
                AddType(typeItr->value);
            }
            else if (typeItr->value.IsArray()) {
                type_ = 0;
                for (typename ValueType::ConstValueIterator itr = typeItr->value.Begin(); itr != typeItr->value.End(); ++itr) 
                    AddType(*itr);
            }
            else {
                // Error
            }
        }

        typename ValueType::ConstMemberIterator enumItr = value.FindMember("enum");
        if (enumItr != value.MemberEnd()) {
            if (enumItr->value.IsArray() && enumItr->value.Size() > 0)
                enum_.CopyFrom(enumItr->value, allocator_);
            else {
                // Error
            }
        }

        typename ValueType::ConstMemberIterator allOfItr = value.FindMember("allOf");
        if (allOfItr != value.MemberEnd())
            CreateLogicalSchemas(allOfItr->value, allOf_);

        typename ValueType::ConstMemberIterator anyOfItr = value.FindMember("anyOf");
        if (anyOfItr != value.MemberEnd())
            CreateLogicalSchemas(anyOfItr->value, anyOf_);

        typename ValueType::ConstMemberIterator oneOfItr = value.FindMember("oneOf");
        if (oneOfItr != value.MemberEnd())
            CreateLogicalSchemas(oneOfItr->value, oneOf_);

        typename ValueType::ConstMemberIterator notItr = value.FindMember("not");
        if (notItr != value.MemberEnd()) {
            if (notItr->value.IsObject())
                not_ = new BaseSchema<Encoding>(notItr->value);
        }

        // Object
        typename ValueType::ConstMemberIterator propretiesItr = value.FindMember("properties");
        if (propretiesItr != value.MemberEnd()) {
            const ValueType& properties = propretiesItr->value;
            properties_ = new Property[properties.MemberCount()];
            propertyCount_ = 0;

            for (typename ValueType::ConstMemberIterator propertyItr = properties.MemberBegin(); propertyItr != properties.MemberEnd(); ++propertyItr) {
                properties_[propertyCount_].name.SetString(propertyItr->name.GetString(), propertyItr->name.GetStringLength(), BaseSchema<Encoding>::allocator_);
                properties_[propertyCount_].schema = new BaseSchema(propertyItr->value);    // TODO: Check error
                propertyCount_++;
            }
        }

#if RAPIDJSON_SCHEMA_HAS_REGEX
        typename ValueType::ConstMemberIterator patternPropretiesItr = value.FindMember("patternProperties");
        if (patternPropretiesItr != value.MemberEnd()) {
            const ValueType& patternProperties = patternPropretiesItr->value;
            patternProperties_ = new PatternProperty[patternProperties.MemberCount()];
            patternPropertyCount_ = 0;

            for (typename ValueType::ConstMemberIterator propertyItr = patternProperties.MemberBegin(); propertyItr != patternProperties.MemberEnd(); ++propertyItr) {
#if RAPIDJSON_SCHEMA_USE_STDREGEX
                try {
                    patternProperties_[patternPropertyCount_].pattern = new std::basic_regex<Ch>(
                        propertyItr->name.GetString(),
                        std::size_t(propertyItr->name.GetStringLength()),
                        std::regex_constants::ECMAScript);
                }
                catch (const std::regex_error&) {
                    // Error
                }
#endif
                patternProperties_[patternPropertyCount_].schema = new BaseSchema<Encoding>(propertyItr->value);    // TODO: Check error
                patternPropertyCount_++;
            }
        }
#endif

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
                        else {
                            // Error
                        }
                    }
                    else {
                        // Error
                    }
                }
            }
            else {
                // Error
            }
        }

        // Establish dependencies after properties
        typename ValueType::ConstMemberIterator dependenciesItr = value.FindMember("dependencies");
        if (dependenciesItr != value.MemberEnd()) {
            if (dependenciesItr->value.IsObject()) {
                hasDependencies_ = true;
                for (typename ValueType::ConstMemberIterator itr = dependenciesItr->value.MemberBegin(); itr != dependenciesItr->value.MemberEnd(); ++itr) {
                    SizeType sourceIndex;
                    if (FindPropertyIndex(itr->name, &sourceIndex)) {
                        properties_[sourceIndex].dependencies = new bool[propertyCount_];
                        std::memset(properties_[sourceIndex].dependencies, 0, sizeof(bool) * propertyCount_);
                        if (itr->value.IsArray()) {
                            for (typename ValueType::ConstValueIterator targetItr = itr->value.Begin(); targetItr != itr->value.End(); ++targetItr) {
                                SizeType targetIndex;
                                if (FindPropertyIndex(*targetItr, &targetIndex)) {
                                    properties_[sourceIndex].dependencies[targetIndex] = true;
                                }
                                else {
                                    // Error
                                }
                            }
                        }
                        else {
                            // Error
                        }
                    }
                    else {
                        // Error
                    }
                }
            }
            else {
                // Error
            }
        }

        typename ValueType::ConstMemberIterator additionalPropretiesItr = value.FindMember("additionalProperties");
        if (additionalPropretiesItr != value.MemberEnd()) {
            if (additionalPropretiesItr->value.IsBool())
                additionalProperty_ = additionalPropretiesItr->value.GetBool();
            else if (additionalPropretiesItr->value.IsObject())
                additionalPropertySchema_ = new BaseSchema<Encoding>(additionalPropretiesItr->value);
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

        // Array
        typename ValueType::ConstMemberIterator itemsItr = value.FindMember("items");
        if (itemsItr != value.MemberEnd()) {
            if (itemsItr->value.IsObject())
                itemsList_ = new BaseSchema<Encoding>(itemsItr->value); // List validation
            else if (itemsItr->value.IsArray()) {
                // Tuple validation
                itemsTuple_ = new BaseSchema<Encoding>*[itemsItr->value.Size()];
                for (typename ValueType::ConstValueIterator itr = itemsItr->value.Begin(); itr != itemsItr->value.End(); ++itr) {
                    itemsTuple_[itemsTupleCount_] = new BaseSchema<Encoding>(*itr);
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

        // String
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

#if RAPIDJSON_SCHEMA_HAS_REGEX
        typename ValueType::ConstMemberIterator patternItr = value.FindMember("pattern");
        if (patternItr != value.MemberEnd()) {
            if (patternItr->value.IsString()) {
#if RAPIDJSON_SCHEMA_USE_STDREGEX
                try {
                    pattern_ = new std::basic_regex<Ch>(
                        patternItr->value.GetString(),
                        std::size_t(patternItr->value.GetStringLength()),
                        std::regex_constants::ECMAScript);
                }
                catch (const std::regex_error&) {
                    // Error
                }
#endif // RAPIDJSON_SCHEMA_USE_STDREGEX
            }
            else {
                // Error
            }
        }
#endif // RAPIDJSON_SCHEMA_HAS_REGEX

        // Number
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

    ~BaseSchema() {
        delete not_;

        delete [] properties_;
        delete additionalPropertySchema_;
#if RAPIDJSON_SCHEMA_HAS_REGEX
        delete [] patternProperties_;
#endif

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
        if (allOf_.schemas) {
            for (SizeType i_ = 0; i_ < allOf_.count; i_++)
                if (!context.allOfValidators.validators[i_]->IsValid())
                    return false;
        }
        if (anyOf_.schemas) {
            bool anyValid = false;
            for (SizeType i_ = 0; i_ < anyOf_.count; i_++)
                if (context.anyOfValidators.validators[i_]->IsValid()) {
                    anyValid = true;
                    break;
                }
            if (!anyValid)
                return false;
        }
        if (oneOf_.schemas) {
            CreateSchemaValidators(context, context.oneOfValidators, oneOf_);
            bool oneValid = false;
            for (SizeType i_ = 0; i_ < oneOf_.count; i_++)
                if (context.oneOfValidators.validators[i_]->IsValid()) {
                    if (oneValid)
                        return false;
                    else
                        oneValid = true;
                }
            if (!oneValid)
                return false;
        }
        if (not_) {
            if (context.notValidator->IsValid())
                return false;
        }
        return true;
    }

    bool Null(Context& context) const { 
        CreateLogicValidators(context);
        return
            (type_ & (1 << kNullSchemaType)) &&
            (!enum_.IsArray() || CheckEnum(GenericValue<Encoding>().Move()));
    }
    
    bool Bool(Context& context, bool b) const { 
        CreateLogicValidators(context);
        return
            (type_ & (1 << kBooleanSchemaType)) &&
            (!enum_.IsArray() || CheckEnum(GenericValue<Encoding>(b).Move()));
    }

    bool Int(Context& context, int i) const {
        CreateLogicValidators(context);
        if ((type_ & ((1 << kIntegerSchemaType) | (1 << kNumberSchemaType))) == 0)
            return false;

        return CheckDouble(i) && (!enum_.IsArray() || CheckEnum(GenericValue<Encoding>(i).Move()));
    }

    bool Uint(Context& context, unsigned u) const {
        CreateLogicValidators(context);
        if ((type_ & ((1 << kIntegerSchemaType) | (1 << kNumberSchemaType))) == 0)
            return false;

        return CheckDouble(u) && (!enum_.IsArray() || CheckEnum(GenericValue<Encoding>(u).Move()));
    }

    bool Int64(Context& context, int64_t i) const {
        CreateLogicValidators(context);
        if ((type_ & ((1 << kIntegerSchemaType) | (1 << kNumberSchemaType))) == 0)
            return false;

        return CheckDouble(i) && (!enum_.IsArray() || CheckEnum(GenericValue<Encoding>(i).Move()));
    }

    bool Uint64(Context& context, uint64_t u) const {
        CreateLogicValidators(context);
        if ((type_ & ((1 << kIntegerSchemaType) | (1 << kNumberSchemaType))) == 0)
            return false;

        return CheckDouble(u) && (!enum_.IsArray() || CheckEnum(GenericValue<Encoding>(u).Move()));
    }

    bool Double(Context& context, double d) const {
        CreateLogicValidators(context);
        if ((type_ & (1 << kNumberSchemaType)) == 0)
            return false;

        return CheckDouble(d) && (!enum_.IsArray() || CheckEnum(GenericValue<Encoding>(d).Move()));
    }
    
    bool String(Context& context, const Ch* str, SizeType length, bool) const {
        (void)str;
        CreateLogicValidators(context);
        if ((type_ & (1 << kStringSchemaType)) == 0)
            return false;

        if (length < minLength_ || length > maxLength_)
            return false;

#if RAPIDJSON_SCHEMA_HAS_REGEX
        if (pattern_) {
#if RAPIDJSON_SCHEMA_USE_STDREGEX
            std::match_results<const Ch*> r;
            if (!std::regex_search(str, str + length, r, *pattern_))
                return false;
#endif // RAPIDJSON_SCHEMA_USE_STDREGEX
        }
#endif // RAPIDJSON_SCHEMA_HAS_REGEX

        return !enum_.IsArray() || CheckEnum(GenericValue<Encoding>(str, length).Move());
    }

    bool StartObject(Context& context) const { 
        CreateLogicValidators(context);
        if ((type_ & (1 << kObjectSchemaType)) == 0)
            return false;

        context.objectRequiredCount = 0;
        if (hasDependencies_) {
            context.objectDependencies = new bool[propertyCount_];
            std::memset(context.objectDependencies, 0, sizeof(bool) * propertyCount_);
        }
        return true; 
    }
    
    bool Key(Context& context, const Ch* str, SizeType len, bool) const {
        CreateLogicValidators(context);
        if ((type_ & (1 << kObjectSchemaType)) == 0)
            return false;
        
        SizeType index;
        if (FindPropertyIndex(str, len, &index)) {
            context.valueSchema = properties_[index].schema;

            if (properties_[index].required)
                context.objectRequiredCount++;

            if (hasDependencies_)
                context.objectDependencies[index] = true;

            return true;
        }

#if RAPIDJSON_SCHEMA_HAS_REGEX
        if (patternProperties_) {
            for (SizeType i = 0; i < patternPropertyCount_; i++) {
#if RAPIDJSON_SCHEMA_USE_STDREGEX
                if (patternProperties_[i].pattern) {
                    std::match_results<const Ch*> r;
                    if (std::regex_search(str, str + len, r, *patternProperties_[i].pattern)) {
                        context.valueSchema = patternProperties_[i].schema;
                        return true;
                    }
                }
#endif // RAPIDJSON_SCHEMA_USE_STDREGEX
            }
        }
#endif

        if (additionalPropertySchema_) {
            context.valueSchema = additionalPropertySchema_;
            return true;
        }
        else if (additionalProperty_) {
            context.valueSchema = GetTypeless();
            return true;
        }
        else
            return false;
    }

    bool EndObject(Context& context, SizeType memberCount) const {
        CreateLogicValidators(context);
        if ((type_ & (1 << kObjectSchemaType)) == 0)
            return false;
        
        if (context.objectRequiredCount != requiredCount_ || memberCount < minProperties_ || memberCount > maxProperties_)
            return false;

        if (hasDependencies_) {
            for (SizeType sourceIndex = 0; sourceIndex < propertyCount_; sourceIndex++)
                if (context.objectDependencies[sourceIndex] && properties_[sourceIndex].dependencies)
                    for (SizeType targetIndex = 0; targetIndex < propertyCount_; targetIndex++)
                        if (properties_[sourceIndex].dependencies[targetIndex] && !context.objectDependencies[targetIndex])
                            return false;
        }

        return true;
    }

    bool StartArray(Context& context) const { 
        CreateLogicValidators(context);
        if ((type_ & (1 << kArraySchemaType)) == 0)
            return false;
        
        context.arrayElementIndex = 0;
        context.inArray = true;
        return true;
    }

    bool EndArray(Context& context, SizeType elementCount) const { 
        CreateLogicValidators(context);
        if ((type_ & (1 << kArraySchemaType)) == 0)
            return false;
        
        context.inArray = false;
        return elementCount >= minItems_ && elementCount <= maxItems_;
    }

#undef RAPIDJSON_BASESCHEMA_HANDLER_LGOICAL_
#undef RAPIDJSON_BASESCHEMA_HANDLER_

protected:
    static const BaseSchema<Encoding>* GetTypeless() {
        static BaseSchema<Encoding> typeless(Value(kObjectType).Move());
        return &typeless;
    }

    void AddType(const Value& type) {
        if      (type == Value("null"   ).Move()) type_ |= 1 << kNullSchemaType;
        else if (type == Value("boolean").Move()) type_ |= 1 << kBooleanSchemaType;
        else if (type == Value("object" ).Move()) type_ |= 1 << kObjectSchemaType;
        else if (type == Value("array"  ).Move()) type_ |= 1 << kArraySchemaType;
        else if (type == Value("string" ).Move()) type_ |= 1 << kStringSchemaType;
        else if (type == Value("integer").Move()) type_ |= 1 << kIntegerSchemaType;
        else if (type == Value("number" ).Move()) type_ |= (1 << kNumberSchemaType) | (1 << kIntegerSchemaType);
        else {
            // Error
        }
    }

    void CreateLogicalSchemas(const Value& logic, BaseSchemaArray<Encoding>& logicSchemas) {
        if (logic.IsArray() && logic.Size() > 0) {
            logicSchemas.count = logic.Size();
            logicSchemas.schemas = new BaseSchema*[logicSchemas.count];
            memset(logicSchemas.schemas, 0, sizeof(BaseSchema*) * logicSchemas.count);
            for (SizeType i = 0; i < logicSchemas.count; i++)
                logicSchemas.schemas[i] = new BaseSchema<Encoding>(logic[i]);
        }
        else {
            // Error
        }
    }

    bool CheckEnum(const GenericValue<Encoding>& v) const {
        for (typename GenericValue<Encoding>::ConstValueIterator itr = enum_.Begin(); itr != enum_.End(); ++itr)
            if (v == *itr)
                return true;
        return false;
    }

    void CreateLogicValidators(Context& context) const {
        if (allOf_.schemas) CreateSchemaValidators(context, context.allOfValidators, allOf_);
        if (anyOf_.schemas) CreateSchemaValidators(context, context.anyOfValidators, anyOf_);
        if (oneOf_.schemas) CreateSchemaValidators(context, context.oneOfValidators, oneOf_);
        if (not_ && !context.notValidator)
            context.notValidator = context.schemaValidatorFactory->CreateSchemaValidator(*not_);
    }

    void CreateSchemaValidators(Context& context, SchemaValidatorArray<Encoding>& validators, const BaseSchemaArray<Encoding>& schemas) const {
        if (!validators.validators) {
            validators.validators = new ISchemaValidator<Encoding>*[schemas.count];
            validators.count = schemas.count;
            for (SizeType i = 0; i < schemas.count; i++)
                validators.validators[i] = context.schemaValidatorFactory->CreateSchemaValidator(*schemas.schemas[i]);
        }
    }

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

    bool CheckDouble(double d) const {
        if (exclusiveMinimum_ ? d <= minimum_ : d < minimum_) return false;
        if (exclusiveMaximum_ ? d >= maximum_ : d > maximum_) return false;
        if (hasMultipleOf_ && std::fmod(d, multipleOf_) != 0.0) return false;
        return true;
    }

    struct Property {
        Property() : schema(), dependencies(), required(false) {}
        ~Property() { 
            delete schema;
            delete[] dependencies;
        }

        GenericValue<Encoding> name;
        BaseSchema<Encoding>* schema;
        bool* dependencies;
        bool required;
    };

#if RAPIDJSON_SCHEMA_HAS_REGEX
    struct PatternProperty {
        PatternProperty() : schema(), pattern() {}
        ~PatternProperty() {
            delete schema;
            delete pattern;
        }

        BaseSchema<Encoding>* schema;
#if RAPIDJSON_SCHEMA_USE_STDREGEX
        std::basic_regex<Ch>* pattern;
#endif
    };
#endif

    MemoryPoolAllocator<> allocator_;
    GenericValue<Encoding> enum_;
    BaseSchemaArray<Encoding> allOf_;
    BaseSchemaArray<Encoding> anyOf_;
    BaseSchemaArray<Encoding> oneOf_;
    BaseSchema<Encoding>* not_;
    unsigned type_; // bitmask of kSchemaType

    Property* properties_;
    BaseSchema<Encoding>* additionalPropertySchema_;
#if RAPIDJSON_SCHEMA_HAS_REGEX
    PatternProperty* patternProperties_;
    SizeType patternPropertyCount_;
#endif
    SizeType propertyCount_;
    SizeType requiredCount_;
    SizeType minProperties_;
    SizeType maxProperties_;
    bool additionalProperty_;
    bool hasDependencies_;

    BaseSchema<Encoding>* itemsList_;
    BaseSchema<Encoding>** itemsTuple_;
    SizeType itemsTupleCount_;
    SizeType minItems_;
    SizeType maxItems_;
    bool additionalItems_;

#if RAPIDJSON_SCHEMA_USE_STDREGEX
    std::basic_regex<Ch>* pattern_;
#endif
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

    bool IsValid() const { return root_ != 0; }

private:
    BaseSchema<Encoding>* root_;
};

typedef GenericSchema<UTF8<> > Schema;

template <typename Encoding, typename OutputHandler = BaseReaderHandler<Encoding>, typename Allocator = CrtAllocator >
class GenericSchemaValidator : public ISchemaValidator<Encoding>, public ISchemaValidatorFactory<Encoding> {
public:
    typedef typename Encoding::Ch Ch;               //!< Character type derived from Encoding.
    typedef GenericSchema<Encoding> SchemaT;

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

    // Implementation of ISchemaValidator<Encoding>
    virtual bool IsValid() { return valid_; }

#define RAPIDJSON_SCHEMA_HANDLE_BEGIN_(method, arg1)\
    if (!valid_) return false; \
    if (!BeginValue() || !CurrentSchema().method arg1) return valid_ = false;

#define RAPIDJSON_SCHEMA_HANDLE_LOGIC_(method, arg2)\
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
    }

#define RAPIDJSON_SCHEMA_HANDLE_END_(method, arg2)\
    return valid_ = EndValue() && outputHandler_.method arg2

#define RAPIDJSON_SCHEMA_HANDLE_VALUE_(method, arg1, arg2) \
    RAPIDJSON_SCHEMA_HANDLE_BEGIN_(method, arg1);\
    RAPIDJSON_SCHEMA_HANDLE_LOGIC_(method, arg2);\
    RAPIDJSON_SCHEMA_HANDLE_END_  (method, arg2)

    virtual bool Null()             { RAPIDJSON_SCHEMA_HANDLE_VALUE_(Null,   (CurrentContext()   ), ( )); }
    virtual bool Bool(bool b)       { RAPIDJSON_SCHEMA_HANDLE_VALUE_(Bool,   (CurrentContext(), b), (b)); }
    virtual bool Int(int i)         { RAPIDJSON_SCHEMA_HANDLE_VALUE_(Int,    (CurrentContext(), i), (i)); }
    virtual bool Uint(unsigned u)   { RAPIDJSON_SCHEMA_HANDLE_VALUE_(Uint,   (CurrentContext(), u), (u)); }
    virtual bool Int64(int64_t i)   { RAPIDJSON_SCHEMA_HANDLE_VALUE_(Int64,  (CurrentContext(), i), (i)); }
    virtual bool Uint64(uint64_t u) { RAPIDJSON_SCHEMA_HANDLE_VALUE_(Uint64, (CurrentContext(), u), (u)); }
    virtual bool Double(double d)   { RAPIDJSON_SCHEMA_HANDLE_VALUE_(Double, (CurrentContext(), d), (d)); }
    virtual bool String(const Ch* str, SizeType length, bool copy)
                                    { RAPIDJSON_SCHEMA_HANDLE_VALUE_(String, (CurrentContext(), str, length, copy), (str, length, copy)); }

    virtual bool StartObject() {
        RAPIDJSON_SCHEMA_HANDLE_BEGIN_(StartObject, (CurrentContext()));
        RAPIDJSON_SCHEMA_HANDLE_LOGIC_(StartObject, ());
        return valid_ = outputHandler_.StartObject();
    }
    
    virtual bool Key(const Ch* str, SizeType len, bool copy) {
        if (!valid_) return false;
        if (!CurrentSchema().Key(CurrentContext(), str, len, copy)) return valid_ = false;
        RAPIDJSON_SCHEMA_HANDLE_LOGIC_(Key, (str, len, copy));
        return valid_ = outputHandler_.Key(str, len, copy);
    }
    
    virtual bool EndObject(SizeType memberCount) { 
        if (!valid_) return false;
        if (!CurrentSchema().EndObject(CurrentContext(), memberCount)) return valid_ = false;
        RAPIDJSON_SCHEMA_HANDLE_LOGIC_(EndObject, (memberCount));
        RAPIDJSON_SCHEMA_HANDLE_END_  (EndObject, (memberCount));
    }

    virtual bool StartArray() {
        RAPIDJSON_SCHEMA_HANDLE_BEGIN_(StartArray, (CurrentContext()));
        RAPIDJSON_SCHEMA_HANDLE_LOGIC_(StartArray, ());
        return valid_ = outputHandler_.StartArray();
    }
    
    virtual bool EndArray(SizeType elementCount) {
        if (!valid_) return false;
        if (!CurrentSchema().EndArray(CurrentContext(), elementCount)) return valid_ = false;
        RAPIDJSON_SCHEMA_HANDLE_LOGIC_(EndArray, (elementCount));
        RAPIDJSON_SCHEMA_HANDLE_END_  (EndArray, (elementCount));
    }

#undef RAPIDJSON_SCHEMA_HANDLE_BEGIN_
#undef RAPIDJSON_SCHEMA_HANDLE_LOGIC_
#undef RAPIDJSON_SCHEMA_HANDLE_VALUE_

    // Implementation of ISchemaValidatorFactory<Encoding>
    virtual ISchemaValidator<Encoding>* CreateSchemaValidator(const BaseSchema<Encoding>& root) {
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

            if (CurrentContext().valueSchema)
                PushSchema(*CurrentContext().valueSchema);
        }
        return true;
    }

    bool EndValue() {
        if (!CurrentSchema().EndValue(CurrentContext()))
            return false;

        PopSchema();
        if (!schemaStack_.Empty() && CurrentContext().multiTypeSchema)
             PopSchema();

        return true;
    }

    void PushSchema(const BaseSchemaType& schema) { *schemaStack_.template Push<Context>() = Context(this, &schema); }
    void PopSchema() { schemaStack_.template Pop<Context>(1)->~Context(); }
    const BaseSchemaType& CurrentSchema() { return *schemaStack_.template Top<Context>()->schema; }
    Context& CurrentContext() { return *schemaStack_.template Top<Context>(); }

    static const size_t kDefaultSchemaStackCapacity = 256;
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
