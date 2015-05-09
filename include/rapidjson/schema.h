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
#include "pointer.h"
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

///////////////////////////////////////////////////////////////////////////////
// Forward declarations

template <typename ValueType, typename Allocator>
class GenericSchemaDocument;

namespace internal {

template <typename SchemaDocumentType>
class Schema;

///////////////////////////////////////////////////////////////////////////////
// ISchemaValidator

class ISchemaValidator {
public:
    virtual ~ISchemaValidator() {}
    virtual bool IsValid() const = 0;
};

///////////////////////////////////////////////////////////////////////////////
// ISchemaValidatorFactory

template <typename SchemaType>
class ISchemaValidatorFactory {
public:
    virtual ~ISchemaValidatorFactory() {}
    virtual ISchemaValidator* CreateSchemaValidator(const SchemaType&) const = 0;
};

///////////////////////////////////////////////////////////////////////////////
// SchemaValidationContext

template <typename SchemaDocumentType>
struct SchemaValidationContext {
    typedef Schema<SchemaDocumentType> SchemaType;
    typedef ISchemaValidatorFactory<SchemaType> SchemaValidatorFactoryType;

    enum PatternValidatorType {
        kPatternValidatorOnly,
        kPatternValidatorWithProperty,
        kPatternValidatorWithAdditionalProperty
    };

    struct SchemaValidatorArray {
        SchemaValidatorArray() : validators(), count() {}
        ~SchemaValidatorArray() {
            for (SizeType i = 0; i < count; i++)
                delete validators[i];
            delete[] validators;
        }

        ISchemaValidator** validators;
        SizeType count;
    };

    SchemaValidationContext(const SchemaValidatorFactoryType* f, const SchemaType* s) :
        factory(f),
        schema(s),
        valueSchema(),
        patternPropertiesSchemas(),
        notValidator(),
        refValidator(),
        patternPropertiesSchemaCount(),
        valuePatternValidatorType(kPatternValidatorOnly),
        objectDependencies(),
        inArray(false)
    {
    }

    ~SchemaValidationContext() {
        delete notValidator;
        delete refValidator;
        delete[] patternPropertiesSchemas;
        delete[] objectDependencies;
    }

    const SchemaValidatorFactoryType* factory;
    const SchemaType* schema;
    const SchemaType* valueSchema;
    SchemaValidatorArray allOfValidators;
    SchemaValidatorArray anyOfValidators;
    SchemaValidatorArray oneOfValidators;
    SchemaValidatorArray dependencyValidators;
    SchemaValidatorArray patternPropertiesValidators;
    const SchemaType** patternPropertiesSchemas;
    ISchemaValidator* notValidator;
    ISchemaValidator* refValidator;
    SizeType patternPropertiesSchemaCount;
    PatternValidatorType valuePatternValidatorType;
    PatternValidatorType objectPatternValidatorType;
    SizeType objectRequiredCount;
    SizeType arrayElementIndex;
    bool* objectDependencies;
    bool inArray;
};

///////////////////////////////////////////////////////////////////////////////
// Schema

template <typename SchemaDocumentType>
class Schema {
public:
    typedef typename SchemaDocumentType::ValueType ValueType;
    typedef typename SchemaDocumentType::AllocatorType AllocatorType;
    typedef typename SchemaDocumentType::PointerType PointerType;
    typedef typename ValueType::EncodingType EncodingType;
    typedef typename EncodingType::Ch Ch;
    typedef SchemaValidationContext<SchemaDocumentType> Context;
    typedef Schema<SchemaDocumentType> SchemaType;
    friend SchemaDocumentType;

    Schema(SchemaDocumentType* document, const PointerType& p, const ValueType& value) :
        not_(),
        ref_(),
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
        typedef typename SchemaDocumentType::ValueType ValueType;
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

        AssigIfExist(allOf_, document, p, value, "allOf");
        AssigIfExist(anyOf_, document, p, value, "anyOf");
        AssigIfExist(oneOf_, document, p, value, "oneOf");

        if (const ValueType* v = GetMember(value, "not"))
            not_ = document->CreateSchema(p.Append("not"), *v);

        if (const ValueType* v = GetMember(value, "$ref"))
            document->AddRefSchema(this, *v);

        // Object

        const ValueType* properties = GetMember(value, "properties");
        const ValueType* required = GetMember(value, "required");
        const ValueType* dependencies = GetMember(value, "dependencies");
        {
            // Gather properties from properties/required/dependencies
            typedef ValueType SValue;
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

        if (properties && properties->IsObject()) {
            PointerType q = p.Append("properties");
            for (ConstMemberIterator itr = properties->MemberBegin(); itr != properties->MemberEnd(); ++itr) {
                SizeType index;
                if (FindPropertyIndex(itr->name, &index)) {
                    properties_[index].schema = document->CreateSchema(q.Append(itr->name), itr->value);
                    properties_[index].typeless = false;
                }
            }
        }

        if (const ValueType* v = GetMember(value, "patternProperties")) {
            PointerType q = p.Append("patternProperties");
            patternProperties_ = new PatternProperty[v->MemberCount()];
            patternPropertyCount_ = 0;

            for (ConstMemberIterator itr = v->MemberBegin(); itr != v->MemberEnd(); ++itr) {
                patternProperties_[patternPropertyCount_].pattern = CreatePattern(itr->name);
                patternProperties_[patternPropertyCount_].schema = document->CreateSchema(q.Append(itr->name), itr->value);
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
            PointerType q = p.Append("dependencies");
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
                        properties_[sourceIndex].dependenciesSchema = document->CreateSchema(q.Append(itr->name), itr->value);
                    }
                }
            }
        }

        if (const ValueType* v = GetMember(value, "additionalProperties")) {
            if (v->IsBool())
                additionalProperties_ = v->GetBool();
            else if (v->IsObject())
                additionalPropertiesSchema_ = document->CreateSchema(p.Append("additionalProperties"), *v);
        }

        AssignIfExist(minProperties_, value, "minProperties");
        AssignIfExist(maxProperties_, value, "maxProperties");

        // Array
        if (const ValueType* v = GetMember(value, "items")) {
            if (v->IsObject()) // List validation
                itemsList_ = document->CreateSchema(p, *v);
            else if (v->IsArray()) { // Tuple validation
                PointerType q = p.Append("items");
                itemsTuple_ = new const Schema*[v->Size()];
                SizeType index = 0;
                for (ConstValueIterator itr = v->Begin(); itr != v->End(); ++itr, index++)
                    itemsTuple_[itemsTupleCount_++] = document->CreateSchema(q.Append(index), *itr);
            }
        }

        AssignIfExist(minItems_, value, "minItems");
        AssignIfExist(maxItems_, value, "maxItems");

        if (const ValueType* v = GetMember(value, "additionalItems")) {
            if (v->IsBool())
                additionalItems_ = v->GetBool();
            else if (v->IsObject())
                additionalItemsSchema_ = document->CreateSchema(p.Append("additionalItems"), *v);
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

    ~Schema() {
        delete [] properties_;
        delete [] patternProperties_;
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
            if (context.objectPatternValidatorType != Context::kPatternValidatorOnly)
                otherValid = context.patternPropertiesValidators.validators[--count]->IsValid();

            bool patternValid = true;
            for (SizeType i = 0; i < count; i++)
                if (!context.patternPropertiesValidators.validators[i]->IsValid()) {
                    patternValid = false;
                    break;
                }

            if (context.objectPatternValidatorType == Context::kPatternValidatorOnly) {
                if (!patternValid)
                    return false;
            }
            else if (context.objectPatternValidatorType == Context::kPatternValidatorWithProperty) {
                if (!patternValid || !otherValid)
                    return false;
            }
            else if (!patternValid && !otherValid) // kPatternValidatorWithAdditionalProperty)
                return false;
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

        if (not_ && context.notValidator->IsValid())
            return false;

        return !ref_ || context.refValidator->IsValid();
    }

    bool Null(Context& context) const { 
        CreateParallelValidator(context);
        return
            (type_ & (1 << kNullSchemaType)) &&
            (!enum_.IsArray() || CheckEnum(ValueType().Move()));
    }
    
    bool Bool(Context& context, bool b) const { 
        CreateParallelValidator(context);
        return
            (type_ & (1 << kBooleanSchemaType)) &&
            (!enum_.IsArray() || CheckEnum(ValueType(b).Move()));
    }

    bool Int(Context& context, int i) const {
        CreateParallelValidator(context);
        if ((type_ & ((1 << kIntegerSchemaType) | (1 << kNumberSchemaType))) == 0)
            return false;

        return CheckDouble(i) && (!enum_.IsArray() || CheckEnum(ValueType(i).Move()));
    }

    bool Uint(Context& context, unsigned u) const {
        CreateParallelValidator(context);
        if ((type_ & ((1 << kIntegerSchemaType) | (1 << kNumberSchemaType))) == 0)
            return false;

        return CheckDouble(u) && (!enum_.IsArray() || CheckEnum(ValueType(u).Move()));
    }

    bool Int64(Context& context, int64_t i) const {
        CreateParallelValidator(context);
        if ((type_ & ((1 << kIntegerSchemaType) | (1 << kNumberSchemaType))) == 0)
            return false;

        return CheckDouble(i) && (!enum_.IsArray() || CheckEnum(ValueType(i).Move()));
    }

    bool Uint64(Context& context, uint64_t u) const {
        CreateParallelValidator(context);
        if ((type_ & ((1 << kIntegerSchemaType) | (1 << kNumberSchemaType))) == 0)
            return false;

        return CheckDouble(u) && (!enum_.IsArray() || CheckEnum(ValueType(u).Move()));
    }

    bool Double(Context& context, double d) const {
        CreateParallelValidator(context);
        if ((type_ & (1 << kNumberSchemaType)) == 0)
            return false;

        return CheckDouble(d) && (!enum_.IsArray() || CheckEnum(ValueType(d).Move()));
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
            if (internal::CountStringCodePoint<EncodingType>(str, length, &count) && (count < minLength_ || count > maxLength_))
                return false;
        }

        if (pattern_ && !IsPatternMatch(pattern_, str, length))
            return false;

        return !enum_.IsArray() || CheckEnum(ValueType(str, length).Move());
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
            context.patternPropertiesSchemas = new const SchemaType*[count];
            context.patternPropertiesSchemaCount = 0;
            std::memset(context.patternPropertiesSchemas, 0, sizeof(SchemaType*) * count);
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
            const SchemaType* propertySchema = properties_[index].typeless ? GetTypeless() : properties_[index].schema;
            if (context.patternPropertiesSchemaCount > 0) {
                context.patternPropertiesSchemas[context.patternPropertiesSchemaCount++] = propertySchema;
                context.valueSchema = GetTypeless();
                context.valuePatternValidatorType = Context::kPatternValidatorWithProperty;
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
                context.valuePatternValidatorType = Context::kPatternValidatorWithAdditionalProperty;
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
    enum SchemaValueType {
        kNullSchemaType,
        kBooleanSchemaType,
        kObjectSchemaType,
        kArraySchemaType,
        kStringSchemaType,
        kNumberSchemaType,
        kIntegerSchemaType,
        kTotalSchemaType
    };

#if RAPIDJSON_SCHEMA_USE_STDREGEX
        typedef std::basic_regex<Ch> RegexType;
#else
        typedef char RegexType;
#endif

    struct SchemaArray {
        SchemaArray() : schemas(), count() {}
        ~SchemaArray() { delete[] schemas; }
        const SchemaType** schemas;
        SizeType count;
    };

    static const SchemaType* GetTypeless() {
        static SchemaType typeless(0, Pointer(), Value(kObjectType).Move());
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

    template <typename DocumentType, typename ValueType, typename PointerType>
    static void AssigIfExist(SchemaArray& out, const DocumentType& document, const PointerType& p, const ValueType& value, const char* name) {
        if (const ValueType* v = GetMember(value, name)) {
            if (v->IsArray() && v->Size() > 0) {
                PointerType q = p.Append(name);
                out.count = v->Size();
                out.schemas = new const Schema*[out.count];
                memset(out.schemas, 0, sizeof(Schema*)* out.count);
                for (SizeType i = 0; i < out.count; i++)
                    out.schemas[i] = document->CreateSchema(q.Append(i), (*v)[i]);
            }
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

    bool CheckEnum(const ValueType& v) const {
        for (typename ValueType::ConstValueIterator itr = enum_.Begin(); itr != enum_.End(); ++itr)
            if (v == *itr)
                return true;
        return false;
    }

    void CreateParallelValidator(Context& context) const {
        if (allOf_.schemas) CreateSchemaValidators(context, context.allOfValidators, allOf_);
        if (anyOf_.schemas) CreateSchemaValidators(context, context.anyOfValidators, anyOf_);
        if (oneOf_.schemas) CreateSchemaValidators(context, context.oneOfValidators, oneOf_);
        if (not_ && !context.notValidator)
            context.notValidator = context.factory->CreateSchemaValidator(*not_);
        if (ref_ && !context.refValidator)
            context.refValidator = context.factory->CreateSchemaValidator(*ref_);

        if (hasSchemaDependencies_ && !context.dependencyValidators.validators) {
            context.dependencyValidators.validators = new ISchemaValidator*[propertyCount_];
            context.dependencyValidators.count = propertyCount_;
            for (SizeType i = 0; i < propertyCount_; i++)
                context.dependencyValidators.validators[i] = properties_[i].dependenciesSchema ? context.factory->CreateSchemaValidator(*properties_[i].dependenciesSchema) : 0;
        }
    }

    void CreateSchemaValidators(Context& context, typename Context::SchemaValidatorArray& validators, const SchemaArray& schemas) const {
        if (!validators.validators) {
            validators.validators = new ISchemaValidator*[schemas.count];
            validators.count = schemas.count;
            for (SizeType i = 0; i < schemas.count; i++)
                validators.validators[i] = context.factory->CreateSchemaValidator(*schemas.schemas[i]);
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
        ~Property() { delete[] dependencies; }
        ValueType name;
        const SchemaType* schema;
        const SchemaType* dependenciesSchema;
        bool* dependencies;
        bool required;
        bool typeless;
    };

    struct PatternProperty {
        PatternProperty() : schema(), pattern() {}
        ~PatternProperty() { delete pattern; }
        const SchemaType* schema;
        const RegexType* pattern;
    };

    AllocatorType allocator_;
    ValueType enum_;
    SchemaArray allOf_;
    SchemaArray anyOf_;
    SchemaArray oneOf_;
    const SchemaType* not_;
    const SchemaType* ref_;
    unsigned type_; // bitmask of kSchemaType

    Property* properties_;
    const SchemaType* additionalPropertiesSchema_;
    PatternProperty* patternProperties_;
    SizeType patternPropertyCount_;
    SizeType propertyCount_;
    SizeType requiredCount_;
    SizeType minProperties_;
    SizeType maxProperties_;
    bool additionalProperties_;
    bool hasDependencies_;
    bool hasSchemaDependencies_;

    const SchemaType* additionalItemsSchema_;
    const SchemaType* itemsList_;
    const SchemaType** itemsTuple_;
    SizeType itemsTupleCount_;
    SizeType minItems_;
    SizeType maxItems_;
    bool additionalItems_;

    const RegexType* pattern_;
    SizeType minLength_;
    SizeType maxLength_;

    double minimum_;
    double maximum_;
    double multipleOf_;
    bool hasMultipleOf_;
    bool exclusiveMinimum_;
    bool exclusiveMaximum_;
};

} // namespace internal

///////////////////////////////////////////////////////////////////////////////
// IGenericRemoteSchemaDocumentProvider

template <typename ValueType, typename Allocator = MemoryPoolAllocator<> >
class IGenericRemoteSchemaDocumentProvider {
public:
    typedef GenericSchemaDocument<ValueType, Allocator> SchemaDocumentType;
    typedef typename ValueType::Ch Ch;

    virtual ~IGenericRemoteSchemaDocumentProvider() {}
    virtual SchemaDocumentType* GetRemoteDocument(const Ch* uri, SizeType length) = 0;
};

typedef IGenericRemoteSchemaDocumentProvider<Value> IRemoteSchemaDocumentProvider;

///////////////////////////////////////////////////////////////////////////////
// GenericSchemaDocument

template <typename ValueT, typename Allocator = MemoryPoolAllocator<> >
class GenericSchemaDocument {
public:
    typedef ValueT ValueType;
    typedef IGenericRemoteSchemaDocumentProvider<ValueType, Allocator> IRemoteSchemaDocumentProviderType;
    typedef Allocator AllocatorType;
    typedef typename ValueType::EncodingType EncodingType;
    typedef typename EncodingType::Ch Ch;
    typedef internal::Schema<GenericSchemaDocument> SchemaType;
    typedef GenericPointer<ValueType> PointerType;
    friend class internal::Schema<GenericSchemaDocument>;

    GenericSchemaDocument(const ValueType& document, IRemoteSchemaDocumentProviderType* remoteProvider = 0, Allocator* allocator = 0) : 
        document_(document),
        remoteProvider_(remoteProvider),
        root_(),
        schemaMap_(allocator, kInitialSchemaMapSize),
        schemaRef_(allocator, kInitialSchemaRefSize)
    {
        
        // Generate root schema, it will call CreateSchema() to create sub-schemas,
        // And call AddRefSchema() if there are $ref.
        root_ = CreateSchema(PointerType(), static_cast<const ValueType&>(document));

        // Resolve $ref
        while (!schemaRef_.Empty()) {
            SchemaEntry* refEntry = schemaRef_.template Pop<SchemaEntry>(1);
            PointerType p = refEntry->pointer;      // Due to re-entrance,
            SchemaType* source = refEntry->schema;  // backup the entry first,
            refEntry->~SchemaEntry();              // and then destruct it.
            source->ref_ = GetSchema(p);
        }
    }

    ~GenericSchemaDocument() {
        while (!schemaMap_.Empty()) {
            SchemaEntry* e = schemaMap_.template Pop<SchemaEntry>(1);
            delete e->schema;
            e->~SchemaEntry();
        }
    }

    const SchemaType& GetRoot() const { return *root_; }

private:
    struct SchemaEntry {
        SchemaEntry(const GenericPointer<ValueType>& p, SchemaType* s) : pointer(p), schema(s) {}
        GenericPointer<ValueType> pointer;
        SchemaType* schema;
    };

    const SchemaType* CreateSchema(const GenericPointer<ValueType>& pointer, const ValueType& v) {
        RAPIDJSON_ASSERT(pointer.IsValid());
        SchemaType* schema = new SchemaType(this, pointer, v);
        new (schemaMap_.template Push<SchemaEntry>()) SchemaEntry(pointer, schema);
        return schema;
    }

    void AddRefSchema(SchemaType* schema, const ValueType& v) {
        if (v.IsString()) {
            SizeType len = v.GetStringLength();
            if (len > 0) {
                const Ch* s = v.GetString();
                SizeType i = 0;
                while (i < len && s[i] != '#') // Find the first #
                    i++;

                if (i > 0) { // Remote reference, resolve immediately
                    if (remoteProvider_) {
                        if (GenericSchemaDocument* remoteDocument = remoteProvider_->GetRemoteDocument(s, i - 1)) {
                            printf("remote fragment: %*s\n", len - i, &s[i]);
                            GenericPointer<ValueType> pointer(&s[i], len - i);
                            if (pointer.IsValid())
                                schema->ref_ = remoteDocument->GetSchema(pointer);
                        }
                    }
                }
                else if (s[i] == '#') { // Local reference, defer resolution
                    printf("local fragment: %*s\n", len - i, &s[i]);
                    GenericPointer<ValueType> pointer(&s[i], len - i);
                    if (pointer.IsValid())
                        new (schemaRef_.template Push<SchemaEntry>()) SchemaEntry(pointer, schema);
                }
            }
        }
    }

    const SchemaType* GetSchema(const GenericPointer<ValueType>& pointer) {
        for (SchemaEntry* target = schemaMap_.template Bottom<SchemaEntry>(); target <= schemaMap_.template Top<SchemaEntry>(); ++target)
            if (pointer == target->pointer)
                return target->schema;

        if (const ValueType* v = pointer.Get(document_))
            return CreateSchema(pointer, *v);
        else
            return 0;
    }

    static const size_t kInitialSchemaMapSize = 1024;
    static const size_t kInitialSchemaRefSize = 1024;

    const ValueType& document_;
    IRemoteSchemaDocumentProviderType* remoteProvider_;
    const SchemaType* root_;                //!< Root schema.
    internal::Stack<Allocator> schemaMap_;  // Stores created Pointer -> Schemas
    internal::Stack<Allocator> schemaRef_;  // Stores Pointer from $ref and schema which holds the $ref
};

typedef GenericSchemaDocument<Value> SchemaDocument;

///////////////////////////////////////////////////////////////////////////////
// GenericSchemaValidator

template <typename SchemaDocumentType, typename OutputHandler = BaseReaderHandler<typename SchemaDocumentType::SchemaType::EncodingType>, typename StateAllocator = CrtAllocator >
class GenericSchemaValidator :
    public internal::ISchemaValidatorFactory<typename SchemaDocumentType::SchemaType>, 
    public internal::ISchemaValidator
{
public:
    typedef typename SchemaDocumentType::SchemaType SchemaType;
    typedef typename SchemaType::EncodingType EncodingType;
    typedef typename EncodingType::Ch Ch;

    GenericSchemaValidator(
        const SchemaDocumentType& schemaDocument,
        StateAllocator* allocator = 0, 
        size_t schemaStackCapacity = kDefaultSchemaStackCapacity/*,
        size_t documentStackCapacity = kDefaultDocumentStackCapacity*/)
        :
        root_(schemaDocument.GetRoot()),
        outputHandler_(nullOutputHandler_),
        schemaStack_(allocator, schemaStackCapacity),
        // documentStack_(allocator, documentStackCapacity),
        valid_(true)
    {
    }

    GenericSchemaValidator( 
        const SchemaDocumentType& schemaDocument,
        OutputHandler& outputHandler,
        StateAllocator* allocator = 0,
        size_t schemaStackCapacity = kDefaultSchemaStackCapacity/*,
        size_t documentStackCapacity = kDefaultDocumentStackCapacity*/)
        :
        root_(schemaDocument.GetRoot()),
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

    // Implementation of ISchemaValidator
    virtual bool IsValid() const { return valid_; }

#define RAPIDJSON_SCHEMA_HANDLE_BEGIN_(method, arg1)\
    if (!valid_) return false; \
    if (!BeginValue() || !CurrentSchema().method arg1) return valid_ = false;

#define RAPIDJSON_SCHEMA_HANDLE_PARALLEL_(method, arg2)\
    for (Context* context = schemaStack_.template Bottom<Context>(); context <= schemaStack_.template Top<Context>(); context++) {\
        if (context->allOfValidators.validators)\
            for (SizeType i_ = 0; i_ < context->allOfValidators.count; i_++)\
                static_cast<GenericSchemaValidator*>(context->allOfValidators.validators[i_])->method arg2;\
        if (context->anyOfValidators.validators)\
            for (SizeType i_ = 0; i_ < context->anyOfValidators.count; i_++)\
                static_cast<GenericSchemaValidator*>(context->anyOfValidators.validators[i_])->method arg2;\
        if (context->oneOfValidators.validators)\
            for (SizeType i_ = 0; i_ < context->oneOfValidators.count; i_++)\
                static_cast<GenericSchemaValidator*>(context->oneOfValidators.validators[i_])->method arg2;\
        if (context->notValidator)\
            static_cast<GenericSchemaValidator*>(context->notValidator)->method arg2;\
        if (context->refValidator)\
            static_cast<GenericSchemaValidator*>(context->refValidator)->method arg2;\
        if (context->dependencyValidators.validators)\
            for (SizeType i_ = 0; i_ < context->dependencyValidators.count; i_++)\
                if (context->dependencyValidators.validators[i_])\
                    static_cast<GenericSchemaValidator*>(context->dependencyValidators.validators[i_])->method arg2;\
        if (context->patternPropertiesValidators.validators)\
            for (SizeType i_ = 0; i_ < context->patternPropertiesValidators.count; i_++)\
                if (context->patternPropertiesValidators.validators[i_])\
                    static_cast<GenericSchemaValidator*>(context->patternPropertiesValidators.validators[i_])->method arg2; \
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

    // Implementation of ISchemaValidatorFactory<SchemaType>
    virtual ISchemaValidator* CreateSchemaValidator(const SchemaType& root) const {
        return new GenericSchemaValidator(root);
    }

private:
    typedef typename SchemaType::Context Context;

    GenericSchemaValidator( 
        const SchemaType& root,
        StateAllocator* allocator = 0,
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
            const SchemaType** sa = CurrentContext().patternPropertiesSchemas;
            typename Context::PatternValidatorType patternValidatorType = CurrentContext().valuePatternValidatorType;

            if (CurrentContext().valueSchema)
                PushSchema(*CurrentContext().valueSchema);

            if (count > 0) {
                CurrentContext().objectPatternValidatorType = patternValidatorType;
                typename Context::SchemaValidatorArray& va = CurrentContext().patternPropertiesValidators;
                va.validators = new ISchemaValidator*[count];
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

    void PushSchema(const SchemaType& schema) { new (schemaStack_.template Push<Context>()) Context(this, &schema); }
    void PopSchema() { schemaStack_.template Pop<Context>(1)->~Context(); }
    const SchemaType& CurrentSchema() { return *schemaStack_.template Top<Context>()->schema; }
    Context& CurrentContext() { return *schemaStack_.template Top<Context>(); }

    static const size_t kDefaultSchemaStackCapacity = 1024;
    //static const size_t kDefaultDocumentStackCapacity = 256;
    const SchemaType& root_;
    BaseReaderHandler<EncodingType> nullOutputHandler_;
    OutputHandler& outputHandler_;
    internal::Stack<StateAllocator> schemaStack_;    //!< stack to store the current path of schema (BaseSchemaType *)
    //internal::Stack<Allocator> documentStack_;  //!< stack to store the current path of validating document (Value *)
    bool valid_;
};

typedef GenericSchemaValidator<SchemaDocument> SchemaValidator;

RAPIDJSON_NAMESPACE_END

#if defined(__GNUC__)
RAPIDJSON_DIAG_POP
#endif

#endif // RAPIDJSON_SCHEMA_H_
