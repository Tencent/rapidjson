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

#ifndef RAPIDJSON_SCHEMA_VERBOSE
#define RAPIDJSON_SCHEMA_VERBOSE 0
#endif

#if RAPIDJSON_SCHEMA_VERBOSE
#include "stringbuffer.h"
#endif

#if defined(__GNUC__)
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(effc++)
#endif

#if RAPIDJSON_SCHEMA_VERBOSE
#define RAPIDJSON_INVALID_KEYWORD_VERBOSE(keyword) \
RAPIDJSON_MULTILINEMACRO_BEGIN\
    StringBuffer sb;\
    context.schema->GetPointer().Stringify(sb);\
    printf("Fail schema: %s\nFail keyword: %s\n", sb.GetString(), keyword);\
RAPIDJSON_MULTILINEMACRO_END
#else
#define RAPIDJSON_INVALID_KEYWORD_VERBOSE(keyword)
#endif

#define RAPIDJSON_INVALID_KEYWORD_RETURN(keyword)\
RAPIDJSON_MULTILINEMACRO_BEGIN\
    context.invalidKeyword = keyword;\
    RAPIDJSON_INVALID_KEYWORD_VERBOSE(keyword);\
    return false;\
RAPIDJSON_MULTILINEMACRO_END

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
// Hasher

// For comparison of compound value
template<typename ValueType, typename Allocator>
class Hasher {
public:
    typedef typename ValueType::Ch Ch;

    Hasher(Allocator* allocator = 0) : stack_(allocator, kDefaultSize) {}

    bool Null() { return WriteType(kNullType); }
    bool Bool(bool b) { return WriteType(b ? kTrueType : kFalseType); }
    bool Int(int i) { Number n; n.u.i = i; n.d = static_cast<double>(i); return WriteNumber(n); }
    bool Uint(unsigned u) { Number n; n.u.u = u; n.d = static_cast<double>(u); return WriteNumber(n); }
    bool Int64(int64_t i) { Number n; n.u.i = i; n.d = static_cast<double>(i); return WriteNumber(n); }
    bool Uint64(uint64_t u) { Number n; n.u.u = u; n.d = static_cast<double>(u); return WriteNumber(n); }
    bool Double(double d) { 
        Number n; 
        if (d < 0) n.u.i = static_cast<int64_t>(d);
        else       n.u.u = static_cast<uint64_t>(d); 
        n.d = d;
        return WriteNumber(n);
    }

    bool String(const Ch* str, SizeType len, bool) {
        WriteBuffer(kStringType, str, len * sizeof(Ch));
        return true;
    }

    bool StartObject() { return true; }
    bool Key(const Ch* str, SizeType len, bool copy) { return String(str, len, copy); }
    bool EndObject(SizeType memberCount) { 
        uint64_t h = Hash(0, kObjectType);
        uint64_t* kv = stack_.template Pop<uint64_t>(memberCount * 2);
        for (SizeType i = 0; i < memberCount; i++)
            h ^= Hash(kv[i * 2], kv[i * 2 + 1]);  // Use xor to achieve member order insensitive
        *stack_.template Push<uint64_t>() = h;
        return true;
    }
    
    bool StartArray() { return true; }
    bool EndArray(SizeType elementCount) { 
        uint64_t h = Hash(0, kArrayType);
        uint64_t* e = stack_.template Pop<uint64_t>(elementCount);
        for (SizeType i = 0; i < elementCount; i++)
            h = Hash(h, e[i]); // Use hash to achieve element order sensitive
        *stack_.template Push<uint64_t>() = h;
        return true;
    }

    bool IsValid() const { return stack_.GetSize() == sizeof(uint64_t); }

    uint64_t GetHashCode() const {
        RAPIDJSON_ASSERT(IsValid());
        return *stack_.template Top<uint64_t>();
    }

private:
    static const size_t kDefaultSize = 256;
    struct Number {
        union U {
            uint64_t u;
            int64_t i;
        }u;
        double d;
    };

    bool WriteType(Type type) { return WriteBuffer(type, 0, 0); }
    
    bool WriteNumber(const Number& n) { return WriteBuffer(kNumberType, &n, sizeof(n)); }
    
    bool WriteBuffer(Type type, const void* data, size_t len) {
        // FNV-1a from http://isthe.com/chongo/tech/comp/fnv/
        uint64_t h = Hash(RAPIDJSON_UINT64_C2(0x84222325, 0xcbf29ce4), type);
        const unsigned char* d = static_cast<const unsigned char*>(data);
        for (size_t i = 0; i < len; i++)
            h = Hash(h, d[i]);
        *stack_.template Push<uint64_t>() = h;
        return true;
    }

    static uint64_t Hash(uint64_t h, uint64_t d) {
        static const uint64_t kPrime = RAPIDJSON_UINT64_C2(0x00000100, 0x000001b3);
        h ^= d;
        h *= kPrime;
        return h;
    }

    Stack<Allocator> stack_;
};

///////////////////////////////////////////////////////////////////////////////
// SchemaValidationContext

template <typename SchemaDocumentType>
struct SchemaValidationContext {
    typedef Schema<SchemaDocumentType> SchemaType;
    typedef ISchemaValidatorFactory<SchemaType> SchemaValidatorFactoryType;
    typedef GenericValue<UTF8<>, CrtAllocator> HashCodeArray;
    typedef typename SchemaType::ValueType ValueType;
    typedef Hasher<ValueType, typename SchemaDocumentType::AllocatorType> HasherType;

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

    SchemaValidationContext(const SchemaValidatorFactoryType* f, CrtAllocator* a, const SchemaType* s) :
        factory(f),
        allocator(a),
        schema(s),
        valueSchema(),
        invalidKeyword(),
        hasher(),
        patternPropertiesSchemas(),
        notValidator(),
        patternPropertiesSchemaCount(),
        valuePatternValidatorType(kPatternValidatorOnly),
        objectDependencies(),
        inArray(false),
        valueUniqueness(false),
        arrayUniqueness(false)
    {
    }

    ~SchemaValidationContext() {
        delete hasher;
        delete notValidator;
        delete[] patternPropertiesSchemas;
        delete[] objectDependencies;
    }

    const SchemaValidatorFactoryType* factory;
    CrtAllocator* allocator; // For allocating memory for context
    const SchemaType* schema;
    const SchemaType* valueSchema;
    const char* invalidKeyword;
    HasherType* hasher;
    SchemaValidatorArray allOfValidators;
    SchemaValidatorArray anyOfValidators;
    SchemaValidatorArray oneOfValidators;
    SchemaValidatorArray dependencyValidators;
    SchemaValidatorArray patternPropertiesValidators;
    const SchemaType** patternPropertiesSchemas;
    ISchemaValidator* notValidator;
    SizeType patternPropertiesSchemaCount;
    PatternValidatorType valuePatternValidatorType;
    PatternValidatorType objectPatternValidatorType;
    HashCodeArray arrayElementHashCodes; // array of uint64_t
    SizeType objectRequiredCount;
    SizeType arrayElementIndex;
    bool* objectDependencies;
    bool inArray;
    bool valueUniqueness;
    bool arrayUniqueness;
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
    typedef Hasher<ValueType, AllocatorType> HasherType;
    typedef GenericValue<EncodingType, AllocatorType> SValue;
    friend class GenericSchemaDocument<ValueType, AllocatorType>;

    Schema(SchemaDocumentType* document, const PointerType& p, const ValueType& value, AllocatorType* allocator) :
        pointer_(p),
        allocator_(allocator),
        enum_(),
        enumCount_(),
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
        uniqueItems_(false),
        pattern_(),
        minLength_(0),
        maxLength_(~SizeType(0)),
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
            if (v->IsArray() && v->Size() > 0) {
                enum_ = static_cast<uint64_t*>(allocator_->Malloc(sizeof(uint64_t) * v->Size()));
                for (ConstValueIterator itr = v->Begin(); itr != v->End(); ++itr) {
                    HasherType h;
                    itr->Accept(h);
                    enum_[enumCount_++] = h.GetHashCode();
                }
            }

        AssigIfExist(allOf_, document, p, value, "allOf");
        AssigIfExist(anyOf_, document, p, value, "anyOf");
        AssigIfExist(oneOf_, document, p, value, "oneOf");

        if (const ValueType* v = GetMember(value, "not"))
            document->CreateSchema(&not_, p.Append("not"), *v);

        //if (const ValueType* v = GetMember(value, "$ref"))
        //    document->AddRefSchema(this, *v);

        // Object

        const ValueType* properties = GetMember(value, "properties");
        const ValueType* required = GetMember(value, "required");
        const ValueType* dependencies = GetMember(value, "dependencies");
        {
            // Gather properties from properties/required/dependencies
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
                properties_ = static_cast<Property*>(allocator_->Malloc(sizeof(Property) * propertyCount_));
                for (SizeType i = 0; i < propertyCount_; i++) {
                    new (&properties_[i]) Property();
                    properties_[i].name = allProperties[i];
                }
            }
        }

        if (properties && properties->IsObject()) {
            PointerType q = p.Append("properties");
            for (ConstMemberIterator itr = properties->MemberBegin(); itr != properties->MemberEnd(); ++itr) {
                SizeType index;
                if (FindPropertyIndex(itr->name, &index)) {
                    document->CreateSchema(&properties_[index].schema, q.Append(itr->name), itr->value);
                    properties_[index].typeless = false;
                }
            }
        }

        if (const ValueType* v = GetMember(value, "patternProperties")) {
            PointerType q = p.Append("patternProperties");
            patternProperties_ = static_cast<PatternProperty*>(allocator_->Malloc(sizeof(PatternProperty) * v->MemberCount()));
            patternPropertyCount_ = 0;

            for (ConstMemberIterator itr = v->MemberBegin(); itr != v->MemberEnd(); ++itr) {
                new (&patternProperties_[patternPropertyCount_]) PatternProperty();
                patternProperties_[patternPropertyCount_].pattern = CreatePattern(itr->name);
                document->CreateSchema(&patternProperties_[patternPropertyCount_].schema, q.Append(itr->name), itr->value);
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
                        properties_[sourceIndex].dependencies = static_cast<bool*>(allocator_->Malloc(sizeof(bool) * propertyCount_));
                        std::memset(properties_[sourceIndex].dependencies, 0, sizeof(bool)* propertyCount_);
                        for (ConstValueIterator targetItr = itr->value.Begin(); targetItr != itr->value.End(); ++targetItr) {
                            SizeType targetIndex;
                            if (FindPropertyIndex(*targetItr, &targetIndex))
                                properties_[sourceIndex].dependencies[targetIndex] = true;
                        }
                    }
                    else if (itr->value.IsObject()) {
                        hasSchemaDependencies_ = true;
                        document->CreateSchema(&properties_[sourceIndex].dependenciesSchema, q.Append(itr->name), itr->value);
                    }
                }
            }
        }

        if (const ValueType* v = GetMember(value, "additionalProperties")) {
            if (v->IsBool())
                additionalProperties_ = v->GetBool();
            else if (v->IsObject())
                document->CreateSchema(&additionalPropertiesSchema_, p.Append("additionalProperties"), *v);
        }

        AssignIfExist(minProperties_, value, "minProperties");
        AssignIfExist(maxProperties_, value, "maxProperties");

        // Array
        if (const ValueType* v = GetMember(value, "items")) {
            PointerType q = p.Append("items");
            if (v->IsObject()) // List validation
                document->CreateSchema(&itemsList_, q, *v);
            else if (v->IsArray()) { // Tuple validation
                itemsTuple_ = static_cast<const Schema**>(allocator_->Malloc(sizeof(const Schema*) * v->Size()));
                SizeType index = 0;
                for (ConstValueIterator itr = v->Begin(); itr != v->End(); ++itr, index++)
                    document->CreateSchema(&itemsTuple_[itemsTupleCount_++], q.Append(index), *itr);
            }
        }

        AssignIfExist(minItems_, value, "minItems");
        AssignIfExist(maxItems_, value, "maxItems");

        if (const ValueType* v = GetMember(value, "additionalItems")) {
            if (v->IsBool())
                additionalItems_ = v->GetBool();
            else if (v->IsObject())
                document->CreateSchema(&additionalItemsSchema_, p.Append("additionalItems"), *v);
        }

        AssignIfExist(uniqueItems_, value, "uniqueItems");

        // String
        AssignIfExist(minLength_, value, "minLength");
        AssignIfExist(maxLength_, value, "maxLength");

        if (const ValueType* v = GetMember(value, "pattern"))
            pattern_ = CreatePattern(*v);

        // Number
        if (const ValueType* v = GetMember(value, "minimum"))
            if (v->IsNumber())
                minimum_.CopyFrom(*v, *allocator_);

        if (const ValueType* v = GetMember(value, "maximum"))
            if (v->IsNumber())
                maximum_.CopyFrom(*v, *allocator_);

        AssignIfExist(exclusiveMinimum_, value, "exclusiveMinimum");
        AssignIfExist(exclusiveMaximum_, value, "exclusiveMaximum");

        if (const ValueType* v = GetMember(value, "multipleOf"))
            if (v->IsNumber() && v->GetDouble() > 0.0)
                multipleOf_.CopyFrom(*v, *allocator_);
    }

    ~Schema() {
        allocator_->Free(enum_);
        if (properties_) {
            for (SizeType i = 0; i < propertyCount_; i++)
                properties_[i].~Property();
            AllocatorType::Free(properties_);
        }
        if (patternProperties_) {
            for (SizeType i = 0; i < patternPropertyCount_; i++)
                patternProperties_[i].~PatternProperty();
            AllocatorType::Free(patternProperties_);
        }
        AllocatorType::Free(itemsTuple_);
#if RAPIDJSON_SCHEMA_USE_STDREGEX
        if (pattern_) {
            pattern_->~RegexType();
            allocator_->Free(pattern_);
        }
#endif
    }

    const PointerType& GetPointer() const { return pointer_; }

    bool BeginValue(Context& context) const {
        if (context.inArray) {
            if (uniqueItems_)
                context.valueUniqueness = true;

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
                    RAPIDJSON_INVALID_KEYWORD_RETURN("items");
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
                    RAPIDJSON_INVALID_KEYWORD_RETURN("patternProperties");
            }
            else if (context.objectPatternValidatorType == Context::kPatternValidatorWithProperty) {
                if (!patternValid || !otherValid)
                    RAPIDJSON_INVALID_KEYWORD_RETURN("patternProperties");
            }
            else if (!patternValid && !otherValid) // kPatternValidatorWithAdditionalProperty)
                RAPIDJSON_INVALID_KEYWORD_RETURN("patternProperties");
        }

        if (enum_) {
            const uint64_t h = context.hasher->GetHashCode();
            for (SizeType i = 0; i < enumCount_; i++)
                if (enum_[i] == h)
                    goto foundEnum;
            RAPIDJSON_INVALID_KEYWORD_RETURN("enum");
            foundEnum:;
        }

        if (allOf_.schemas)
            for (SizeType i = 0; i < allOf_.count; i++)
                if (!context.allOfValidators.validators[i]->IsValid())
                    RAPIDJSON_INVALID_KEYWORD_RETURN("allOf");
        
        if (anyOf_.schemas) {
            for (SizeType i = 0; i < anyOf_.count; i++)
                if (context.anyOfValidators.validators[i]->IsValid())
                    goto foundAny;
            RAPIDJSON_INVALID_KEYWORD_RETURN("anyOf");
            foundAny:;
        }

        if (oneOf_.schemas) {
            bool oneValid = false;
            for (SizeType i = 0; i < oneOf_.count; i++)
                if (context.oneOfValidators.validators[i]->IsValid()) {
                    if (oneValid)
                        RAPIDJSON_INVALID_KEYWORD_RETURN("oneOf");
                    else
                        oneValid = true;
                }
            if (!oneValid)
                RAPIDJSON_INVALID_KEYWORD_RETURN("oneOf");
        }

        if (not_ && context.notValidator->IsValid())
            RAPIDJSON_INVALID_KEYWORD_RETURN("not");

        return true;
    }

    bool Null(Context& context) const { 
        if (!(type_ & (1 << kNullSchemaType)))
            RAPIDJSON_INVALID_KEYWORD_RETURN("type");
        return CreateParallelValidator(context);
    }
    
    bool Bool(Context& context, bool) const { 
        if (!(type_ & (1 << kBooleanSchemaType)))
            RAPIDJSON_INVALID_KEYWORD_RETURN("type");
        return CreateParallelValidator(context);
    }

    bool Int(Context& context, int i) const {
        if (!CheckInt(context, i))
            return false;
        return CreateParallelValidator(context);
    }

    bool Uint(Context& context, unsigned u) const {
        if (!CheckUint(context, u))
            return false;
        return CreateParallelValidator(context);
    }

    bool Int64(Context& context, int64_t i) const {
        if (!CheckInt(context, i))
            return false;
        return CreateParallelValidator(context);
    }

    bool Uint64(Context& context, uint64_t u) const {
        if (!CheckUint(context, u))
            return false;
        return CreateParallelValidator(context);
    }

    bool Double(Context& context, double d) const {
        if (!(type_ & (1 << kNumberSchemaType)))
            RAPIDJSON_INVALID_KEYWORD_RETURN("type");

        if (!minimum_.IsNull() && !CheckDoubleMinimum(context, d))
            return false;

        if (!maximum_.IsNull() && !CheckDoubleMaximum(context, d))
            return false;
        
        if (!multipleOf_.IsNull() && !CheckDoubleMultipleOf(context, d))
            return false;
        
        return CreateParallelValidator(context);
    }
    
    bool String(Context& context, const Ch* str, SizeType length, bool) const {
        if (!(type_ & (1 << kStringSchemaType)))
            RAPIDJSON_INVALID_KEYWORD_RETURN("type");

        if (minLength_ != 0 || maxLength_ != SizeType(~0)) {
            SizeType count;
            if (internal::CountStringCodePoint<EncodingType>(str, length, &count)) {
                if (count < minLength_)
                    RAPIDJSON_INVALID_KEYWORD_RETURN("minLength");
                if (count > maxLength_)
                    RAPIDJSON_INVALID_KEYWORD_RETURN("maxLength");
            }
        }

        if (pattern_ && !IsPatternMatch(pattern_, str, length))
            RAPIDJSON_INVALID_KEYWORD_RETURN("pattern");

        return CreateParallelValidator(context);
    }

    bool StartObject(Context& context) const { 
        if (!(type_ & (1 << kObjectSchemaType)))
            RAPIDJSON_INVALID_KEYWORD_RETURN("type");

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

        return CreateParallelValidator(context);
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

        if (context.patternPropertiesSchemaCount == 0) // patternProperties are not additional properties
            RAPIDJSON_INVALID_KEYWORD_RETURN("additionalProperties");

        return true;
    }

    bool EndObject(Context& context, SizeType memberCount) const {
        if (context.objectRequiredCount != requiredCount_)
            RAPIDJSON_INVALID_KEYWORD_RETURN("required");

        if (memberCount < minProperties_)
            RAPIDJSON_INVALID_KEYWORD_RETURN("minProperties");

        if (memberCount > maxProperties_)
            RAPIDJSON_INVALID_KEYWORD_RETURN("maxProperties");

        if (hasDependencies_) {
            for (SizeType sourceIndex = 0; sourceIndex < propertyCount_; sourceIndex++)
                if (context.objectDependencies[sourceIndex]) {
                    if (properties_[sourceIndex].dependencies) {
                        for (SizeType targetIndex = 0; targetIndex < propertyCount_; targetIndex++)
                            if (properties_[sourceIndex].dependencies[targetIndex] && !context.objectDependencies[targetIndex])
                                RAPIDJSON_INVALID_KEYWORD_RETURN("dependencies");
                    }
                    else if (properties_[sourceIndex].dependenciesSchema)
                        if (!context.dependencyValidators.validators[sourceIndex]->IsValid())
                            RAPIDJSON_INVALID_KEYWORD_RETURN("dependencies");
                }
        }

        return true;
    }

    bool StartArray(Context& context) const { 
        if (!(type_ & (1 << kArraySchemaType)))
            RAPIDJSON_INVALID_KEYWORD_RETURN("type");

        if (uniqueItems_)
            context.arrayElementHashCodes.SetArray();

        context.arrayElementIndex = 0;
        context.inArray = true;

        return CreateParallelValidator(context);
    }

    bool EndArray(Context& context, SizeType elementCount) const { 
        context.inArray = false;
        
        if (elementCount < minItems_)
            RAPIDJSON_INVALID_KEYWORD_RETURN("minItems");
        
        if (elementCount > maxItems_)
            RAPIDJSON_INVALID_KEYWORD_RETURN("maxItems");

        return true;
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
        ~SchemaArray() { AllocatorType::Free(schemas); }
        const SchemaType** schemas;
        SizeType count;
    };

    static const SchemaType* GetTypeless() {
        static SchemaType typeless(0, PointerType(), Value(kObjectType).Move(), 0);
        return &typeless;
    }

    template <typename V1, typename V2>
    void AddUniqueElement(V1& a, const V2& v) {
        for (typename V1::ConstValueIterator itr = a.Begin(); itr != a.End(); ++itr)
            if (*itr == v)
                return;
        V1 c(v, *allocator_);
        a.PushBack(c, *allocator_);
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
    void AssigIfExist(SchemaArray& out, const DocumentType& document, const PointerType& p, const ValueType& value, const char* name) {
        if (const ValueType* v = GetMember(value, name)) {
            if (v->IsArray() && v->Size() > 0) {
                PointerType q = p.Append(name);
                out.count = v->Size();
                out.schemas = static_cast<const Schema**>(allocator_->Malloc(out.count * sizeof(const Schema*)));
                memset(out.schemas, 0, sizeof(Schema*)* out.count);
                for (SizeType i = 0; i < out.count; i++)
                    document->CreateSchema(&out.schemas[i], q.Append(i), (*v)[i]);
            }
        }
    }

#if RAPIDJSON_SCHEMA_USE_STDREGEX
    template <typename ValueType>
    RegexType* CreatePattern(const ValueType& value) {
        if (value.IsString())
            try {
                return new (allocator_->Malloc(sizeof(RegexType))) RegexType(value.GetString(), std::size_t(value.GetStringLength()), std::regex_constants::ECMAScript);
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

    bool CreateParallelValidator(Context& context) const {
        if (enum_ || context.arrayUniqueness)
            context.hasher = new HasherType;

        if (allOf_.schemas)
            CreateSchemaValidators(context, context.allOfValidators, allOf_);

        if (anyOf_.schemas)
            CreateSchemaValidators(context, context.anyOfValidators, anyOf_);
        
        if (oneOf_.schemas)
            CreateSchemaValidators(context, context.oneOfValidators, oneOf_);
        
        if (not_ && !context.notValidator)
            context.notValidator = context.factory->CreateSchemaValidator(*not_);
        
        if (hasSchemaDependencies_ && !context.dependencyValidators.validators) {
            context.dependencyValidators.validators = new ISchemaValidator*[propertyCount_];
            context.dependencyValidators.count = propertyCount_;
            for (SizeType i = 0; i < propertyCount_; i++)
                context.dependencyValidators.validators[i] = properties_[i].dependenciesSchema ? context.factory->CreateSchemaValidator(*properties_[i].dependenciesSchema) : 0;
        }
        return true;
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

    bool CheckInt(Context& context, int64_t i) const {
        if (!(type_ & ((1 << kIntegerSchemaType) | (1 << kNumberSchemaType))))
            RAPIDJSON_INVALID_KEYWORD_RETURN("type");

        if (!minimum_.IsNull()) {
            if (minimum_.IsInt64()) {
                if (exclusiveMinimum_ ? i <= minimum_.GetInt64() : i < minimum_.GetInt64())
                    RAPIDJSON_INVALID_KEYWORD_RETURN("minimum");
            }
            else if (!CheckDoubleMinimum(context, static_cast<double>(i)))
                return false;
        }

        if (!maximum_.IsNull()) {
            if (maximum_.IsInt64()) {
                if (exclusiveMaximum_ ? i >= maximum_.GetInt64() : i > maximum_.GetInt64())
                    RAPIDJSON_INVALID_KEYWORD_RETURN("maximum");
            }
            else if (!CheckDoubleMaximum(context, static_cast<double>(i)))
                return false;
        }

        if (!multipleOf_.IsNull()) {
            if (multipleOf_.IsUint64()) {
                if (static_cast<uint64_t>(i >= 0 ? i : -i) % multipleOf_.GetUint64() != 0)
                    RAPIDJSON_INVALID_KEYWORD_RETURN("multipleOf");
            }
            else if (!CheckDoubleMultipleOf(context, static_cast<double>(i)))
                return false;
        }

        return true;
    }

    bool CheckUint(Context& context, uint64_t i) const {
        if (!(type_ & ((1 << kIntegerSchemaType) | (1 << kNumberSchemaType))))
            RAPIDJSON_INVALID_KEYWORD_RETURN("type");

        if (!minimum_.IsNull()) {
            if (minimum_.IsUint64()) {
                if (exclusiveMinimum_ ? i <= minimum_.GetUint64() : i < minimum_.GetUint64())
                    RAPIDJSON_INVALID_KEYWORD_RETURN("minimum");
            }
            else if (!CheckDoubleMinimum(context, static_cast<double>(i)))
                return false;
        }

        if (!maximum_.IsNull()) {
            if (maximum_.IsUint64()) {
                if (exclusiveMaximum_ ? i >= maximum_.GetUint64() : i > maximum_.GetUint64())
                    RAPIDJSON_INVALID_KEYWORD_RETURN("maximum");
            }
            else if (!CheckDoubleMaximum(context, static_cast<double>(i)))
                return false;
        }

        if (!multipleOf_.IsNull()) {
            if (multipleOf_.IsUint64()) {
                if (i % multipleOf_.GetUint64() != 0)
                    RAPIDJSON_INVALID_KEYWORD_RETURN("multipleOf");
            }
            else if (!CheckDoubleMultipleOf(context, static_cast<double>(i)))
                return false;
        }

        return true;
    }

    bool CheckDoubleMinimum(Context& context, double d) const {
        if (exclusiveMinimum_ ? d <= minimum_.GetDouble() : d < minimum_.GetDouble())
            RAPIDJSON_INVALID_KEYWORD_RETURN("minimum");
        return true;
    }

    bool CheckDoubleMaximum(Context& context, double d) const {
        if (exclusiveMaximum_ ? d >= maximum_.GetDouble() : d > maximum_.GetDouble())
            RAPIDJSON_INVALID_KEYWORD_RETURN("maximum");
        return true;
    }

    bool CheckDoubleMultipleOf(Context& context, double d) const {
        double a = std::abs(d), b = std::abs(multipleOf_.GetDouble());
        double q = std::floor(a / b);
        double r = a - q * b;
        if (r > 0.0)
            RAPIDJSON_INVALID_KEYWORD_RETURN("multipleOf");
        return true;
    }

    struct Property {
        Property() : schema(), dependenciesSchema(), dependencies(), required(false), typeless(true) {}
        ~Property() { AllocatorType::Free(dependencies); }
        SValue name;
        const SchemaType* schema;
        const SchemaType* dependenciesSchema;
        bool* dependencies;
        bool required;
        bool typeless;
    };

    struct PatternProperty {
        PatternProperty() : schema(), pattern() {}
        ~PatternProperty() { 
            if (pattern) {
                pattern->~RegexType();
                AllocatorType::Free(pattern);
            }
        }
        const SchemaType* schema;
        RegexType* pattern;
    };

    PointerType pointer_;
    AllocatorType* allocator_;
    uint64_t* enum_;
    SizeType enumCount_;
    SchemaArray allOf_;
    SchemaArray anyOf_;
    SchemaArray oneOf_;
    const SchemaType* not_;
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
    bool uniqueItems_;

    RegexType* pattern_;
    SizeType minLength_;
    SizeType maxLength_;

    SValue minimum_;
    SValue maximum_;
    SValue multipleOf_;
    bool exclusiveMinimum_;
    bool exclusiveMaximum_;
};

} // namespace internal

///////////////////////////////////////////////////////////////////////////////
// IGenericRemoteSchemaDocumentProvider

template <typename SchemaDocumentType>
class IGenericRemoteSchemaDocumentProvider {
public:
    typedef typename SchemaDocumentType::Ch Ch;

    virtual ~IGenericRemoteSchemaDocumentProvider() {}
    virtual const SchemaDocumentType* GetRemoteDocument(const Ch* uri, SizeType length) = 0;
};

///////////////////////////////////////////////////////////////////////////////
// GenericSchemaDocument

template <typename ValueT, typename Allocator = MemoryPoolAllocator<> > // Temp to use MemoryPoolAllocator now for profiling
class GenericSchemaDocument {
public:
    typedef ValueT ValueType;
    typedef IGenericRemoteSchemaDocumentProvider<GenericSchemaDocument> IRemoteSchemaDocumentProviderType;
    typedef Allocator AllocatorType;
    typedef typename ValueType::EncodingType EncodingType;
    typedef typename EncodingType::Ch Ch;
    typedef internal::Schema<GenericSchemaDocument> SchemaType;
    typedef GenericPointer<ValueType, CrtAllocator> PointerType;
    friend class internal::Schema<GenericSchemaDocument>;
    template <typename, typename, typename>
    friend class GenericSchemaValidator;

    GenericSchemaDocument(const ValueType& document, IRemoteSchemaDocumentProviderType* remoteProvider = 0, Allocator* allocator = 0) : 
        document_(&document),
        remoteProvider_(remoteProvider),
        allocator_(allocator),
        ownAllocator_(),
        root_(),
        schemaMap_(allocator, kInitialSchemaMapSize),
        schemaRef_(allocator, kInitialSchemaRefSize)
    {
        if (!allocator_)
            ownAllocator_ = allocator_ = RAPIDJSON_NEW(Allocator());

        // Generate root schema, it will call CreateSchema() to create sub-schemas,
        // And call AddRefSchema() if there are $ref.
        CreateSchemaRecursive(&root_, PointerType(), static_cast<const ValueType&>(document));

        // Resolve $ref
        while (!schemaRef_.Empty()) {
            SchemaRefEntry* refEntry = schemaRef_.template Pop<SchemaRefEntry>(1);
            if (const SchemaType* s = GetSchema(refEntry->target)) {
                *refEntry->schema = s;

                // Create entry in map if not exist
                if (!GetSchema(refEntry->source)) {
                    new (schemaMap_.template Push<SchemaEntry>()) SchemaEntry(const_cast<SchemaType*>(s), false);
                }
            }
            refEntry->~SchemaRefEntry();
        }

        RAPIDJSON_ASSERT(root_ != 0);

        schemaRef_.ShrinkToFit(); // Deallocate all memory for ref
    }

    ~GenericSchemaDocument() {
        while (!schemaMap_.Empty())
            schemaMap_.template Pop<SchemaEntry>(1)->~SchemaEntry();

        RAPIDJSON_DELETE(ownAllocator_);
    }

    const SchemaType& GetRoot() const { return *root_; }

private:
    struct SchemaRefEntry {
        SchemaRefEntry(const PointerType& s, const PointerType& t, const SchemaType** outSchema) : source(s), target(t), schema(outSchema) {}
        PointerType source;
        PointerType target;
        const SchemaType** schema;
    };

    struct SchemaEntry {
        SchemaEntry(SchemaType* s, bool o) : schema(s), owned(o) {}
        ~SchemaEntry() {
            if (owned)
                schema->~SchemaType();
        }
        SchemaType* schema;
        bool owned;
    };

    void CreateSchemaRecursive(const SchemaType** schema, const PointerType& pointer, const ValueType& v) {
        if (schema)
            *schema = SchemaType::GetTypeless();

        if (v.GetType() == kObjectType) {
            const SchemaType* s = GetSchema(pointer);
            if (!s)
                CreateSchema(schema, pointer, v);
            else if (schema)
                *schema = s;

            for (typename ValueType::ConstMemberIterator itr = v.MemberBegin(); itr != v.MemberEnd(); ++itr)
                CreateSchemaRecursive(0, pointer.Append(itr->name), itr->value);
        }
        else if (v.GetType() == kArrayType)
            for (SizeType i = 0; i < v.Size(); i++)
                CreateSchemaRecursive(0, pointer.Append(i), v[i]);
    }

    void CreateSchema(const SchemaType** schema, const PointerType& pointer, const ValueType& v) {
        RAPIDJSON_ASSERT(pointer.IsValid());
        if (v.IsObject()) {
            if (!schema || !HandleRefSchema(pointer, schema, v)) {
                SchemaType* s = new (allocator_->Malloc(sizeof(SchemaType))) SchemaType(this, pointer, v, allocator_);
                new (schemaMap_.template Push<SchemaEntry>()) SchemaEntry(s, true);
                if (schema)
                    *schema = s;
            }
        }
    }

    bool HandleRefSchema(const Pointer& source, const SchemaType** schema, const ValueType& v) {
        RAPIDJSON_ASSERT(schema != 0);
        typename ValueType::ConstMemberIterator itr = v.FindMember("$ref");
        if (itr == v.MemberEnd())
            return false;

        if (itr->value.IsString()) {
            SizeType len = itr->value.GetStringLength();
            if (len > 0) {
                const Ch* s = itr->value.GetString();
                SizeType i = 0;
                while (i < len && s[i] != '#') // Find the first #
                    i++;

                if (i > 0) { // Remote reference, resolve immediately
                    if (remoteProvider_) {
                        if (const GenericSchemaDocument* remoteDocument = remoteProvider_->GetRemoteDocument(s, i - 1)) {
                            PointerType pointer(&s[i], len - i);
                            if (pointer.IsValid()) {
                                if (const SchemaType* s = remoteDocument->GetSchema(pointer)) {
                                    *schema = s;
                                    return true;
                                }
                            }
                        }
                    }
                }
                else if (s[i] == '#') { // Local reference, defer resolution
                    PointerType pointer(&s[i], len - i);
                    if (pointer.IsValid()) {
                        if (const ValueType* nv = pointer.Get(*document_))
                            if (HandleRefSchema(source, schema, *nv))
                                return true;

                        new (schemaRef_.template Push<SchemaRefEntry>()) SchemaRefEntry(source, pointer, schema);
                        return true;
                    }
                }
            }
        }
        return false;
    }

    const SchemaType* GetSchema(const PointerType& pointer) const {
        for (const SchemaEntry* target = schemaMap_.template Bottom<SchemaEntry>(); target != schemaMap_.template End<SchemaEntry>(); ++target)
            if (pointer == target->schema->pointer_)
                return target->schema;
        return 0;
    }

    PointerType GetPointer(const SchemaType* schema) const {
        return schema->pointer_;
    }

    static const size_t kInitialSchemaMapSize = 64;
    static const size_t kInitialSchemaRefSize = 64;

    const ValueType* document_;             //!< Only temporarily for constructor
    IRemoteSchemaDocumentProviderType* remoteProvider_;
    Allocator *allocator_;
    Allocator *ownAllocator_;
    const SchemaType* root_;                //!< Root schema.
    internal::Stack<Allocator> schemaMap_;  // Stores created Pointer -> Schemas
    internal::Stack<Allocator> schemaRef_;  // Stores Pointer from $ref and schema which holds the $ref
};

typedef GenericSchemaDocument<Value> SchemaDocument;
typedef IGenericRemoteSchemaDocumentProvider<SchemaDocument> IRemoteSchemaDocumentProvider;

///////////////////////////////////////////////////////////////////////////////
// GenericSchemaValidator

template <typename SchemaDocumentType, typename OutputHandler = BaseReaderHandler<typename SchemaDocumentType::SchemaType::EncodingType>, typename StateAllocator = CrtAllocator >
class GenericSchemaValidator :
    public internal::ISchemaValidatorFactory<typename SchemaDocumentType::SchemaType>, 
    public internal::ISchemaValidator
{
public:
    typedef typename SchemaDocumentType::SchemaType SchemaType;
    typedef typename SchemaDocumentType::PointerType PointerType;
    typedef typename SchemaType::EncodingType EncodingType;
    typedef typename EncodingType::Ch Ch;

    GenericSchemaValidator(
        const SchemaDocumentType& schemaDocument,
        StateAllocator* allocator = 0, 
        size_t schemaStackCapacity = kDefaultSchemaStackCapacity,
        size_t documentStackCapacity = kDefaultDocumentStackCapacity)
        :
        schemaDocument_(&schemaDocument),
        root_(schemaDocument.GetRoot()),
        outputHandler_(nullOutputHandler_),
        schemaStack_(allocator, schemaStackCapacity),
        documentStack_(allocator, documentStackCapacity),
        valid_(true)
#if RAPIDJSON_SCHEMA_VERBOSE
        , depth_(0)
#endif
    {
    }

    GenericSchemaValidator( 
        const SchemaDocumentType& schemaDocument,
        OutputHandler& outputHandler,
        StateAllocator* allocator = 0,
        size_t schemaStackCapacity = kDefaultSchemaStackCapacity,
        size_t documentStackCapacity = kDefaultDocumentStackCapacity)
        :
        schemaDocument_(&schemaDocument),
        root_(schemaDocument.GetRoot()),
        outputHandler_(outputHandler),
        schemaStack_(allocator, schemaStackCapacity),
        documentStack_(allocator, documentStackCapacity),
        valid_(true)
#if RAPIDJSON_SCHEMA_VERBOSE
        , depth_(0)
#endif
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

    PointerType GetInvalidSchemaPointer() const {
        return schemaStack_.Empty() ? PointerType() : CurrentSchema().GetPointer();
    }

    const char* GetInvalidSchemaKeyword() const {
        return schemaStack_.Empty() ? 0 : CurrentContext().invalidKeyword;
    }

    PointerType GetInvalidDocumentPointer() const {
        return documentStack_.Empty() ? PointerType() : PointerType(documentStack_.template Bottom<Ch>(), documentStack_.GetSize() / sizeof(Ch));
    }

#if RAPIDJSON_SCHEMA_VERBOSE
#define RAPIDJSON_SCHEMA_HANDLE_BEGIN_VERBOSE_() \
RAPIDJSON_MULTILINEMACRO_BEGIN\
    *documentStack_.template Push<Ch>() = '\0';\
    documentStack_.template Pop<Ch>(1);\
    printf("Fail document: %s\n\n", documentStack_.template Bottom<Ch>());\
RAPIDJSON_MULTILINEMACRO_END
#else
#define RAPIDJSON_SCHEMA_HANDLE_BEGIN_VERBOSE_()
#endif

#define RAPIDJSON_SCHEMA_HANDLE_BEGIN_(method, arg1)\
    if (!valid_) return false; \
    if (!BeginValue() || !CurrentSchema().method arg1) {\
        RAPIDJSON_SCHEMA_HANDLE_BEGIN_VERBOSE_();\
        return valid_ = false;\
    }

#define RAPIDJSON_SCHEMA_HANDLE_PARALLEL_(method, arg2)\
    for (Context* context = schemaStack_.template Bottom<Context>(); context != schemaStack_.template End<Context>(); context++) {\
        if (context->hasher)\
            context->hasher->method arg2;\
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
        AppendToken(str, len);
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
        return new GenericSchemaValidator(*schemaDocument_, root
#if RAPIDJSON_SCHEMA_VERBOSE
        , depth_ + 1
#endif
        );
    }

private:
    typedef typename SchemaType::Context Context;

    GenericSchemaValidator( 
        const SchemaDocumentType& schemaDocument,
        const SchemaType& root,
#if RAPIDJSON_SCHEMA_VERBOSE
        unsigned depth,
#endif
        StateAllocator* allocator = 0,
        size_t schemaStackCapacity = kDefaultSchemaStackCapacity,
        size_t documentStackCapacity = kDefaultDocumentStackCapacity)
        :
        schemaDocument_(&schemaDocument),
        root_(root),
        outputHandler_(nullOutputHandler_),
        schemaStack_(allocator, schemaStackCapacity),
        documentStack_(allocator, documentStackCapacity),
        valid_(true)
#if RAPIDJSON_SCHEMA_VERBOSE
        , depth_(depth)
#endif
    {
    }

    bool BeginValue() {
        if (schemaStack_.Empty())
            PushSchema(root_);
        else {
            if (CurrentContext().inArray)
                AppendToken(CurrentContext().arrayElementIndex);

            if (!CurrentSchema().BeginValue(CurrentContext()))
                return false;

            SizeType count = CurrentContext().patternPropertiesSchemaCount;
            const SchemaType** sa = CurrentContext().patternPropertiesSchemas;
            typename Context::PatternValidatorType patternValidatorType = CurrentContext().valuePatternValidatorType;
            bool valueUniqueness = CurrentContext().valueUniqueness;
            if (CurrentContext().valueSchema)
                PushSchema(*CurrentContext().valueSchema);

            if (count > 0) {
                CurrentContext().objectPatternValidatorType = patternValidatorType;
                typename Context::SchemaValidatorArray& va = CurrentContext().patternPropertiesValidators;
                va.validators = new ISchemaValidator*[count];
                for (SizeType i = 0; i < count; i++)
                    va.validators[va.count++] = CreateSchemaValidator(*sa[i]);
            }

            CurrentContext().arrayUniqueness = valueUniqueness;
        }
        return true;
    }

    bool EndValue() {
        if (!CurrentSchema().EndValue(CurrentContext()))
            return false;

#if RAPIDJSON_SCHEMA_VERBOSE
        StringBuffer sb;
        CurrentSchema().GetPointer().Stringify(sb);

        *documentStack_.template Push<Ch>() = '\0';
        documentStack_.template Pop<Ch>(1);
        printf("S: %*s%s\nD: %*s%s\n\n", depth_ * 4, " ", sb.GetString(), depth_ * 4, " ", documentStack_.template Bottom<Ch>());
#endif

        uint64_t h = CurrentContext().arrayUniqueness ? CurrentContext().hasher->GetHashCode() : 0;
        
        PopSchema();

        if (!schemaStack_.Empty()) {
            Context& context = CurrentContext();
            if (context.valueUniqueness) {
                for (typename Context::HashCodeArray::ConstValueIterator itr = context.arrayElementHashCodes.Begin(); itr != context.arrayElementHashCodes.End(); ++itr)
                    if (itr->GetUint64() == h)
                        RAPIDJSON_INVALID_KEYWORD_RETURN("uniqueItems");
                context.arrayElementHashCodes.PushBack(h, *context.allocator);
            }
        }

        // Remove the last token of document pointer
        while (!documentStack_.Empty() && *documentStack_.template Pop<Ch>(1) != '/')
            ;

        return true;
    }

    void AppendToken(const Ch* str, SizeType len) {
        *documentStack_.template Push<Ch>() = '/';
        for (SizeType i = 0; i < len; i++) {
            if (str[i] == '~') {
                *documentStack_.template Push<Ch>() = '~';
                *documentStack_.template Push<Ch>() = '0';
            }
            else if (str[i] == '/') {
                *documentStack_.template Push<Ch>() = '~';
                *documentStack_.template Push<Ch>() = '1';
            }
            else
                *documentStack_.template Push<Ch>() = str[i];
        }
    }

    void AppendToken(SizeType index) {
        *documentStack_.template Push<Ch>() = '/';
        char buffer[21];
        SizeType length = (sizeof(SizeType) == 4 ? internal::u32toa(index, buffer): internal::u64toa(index, buffer)) - buffer;
        for (SizeType i = 0; i < length; i++)
            *documentStack_.template Push<Ch>() = buffer[i];
    }

    void PushSchema(const SchemaType& schema) { new (schemaStack_.template Push<Context>()) Context(this, &contextAllocator_, &schema); }
    void PopSchema() { schemaStack_.template Pop<Context>(1)->~Context(); }
    const SchemaType& CurrentSchema() const { return *schemaStack_.template Top<Context>()->schema; }
    Context& CurrentContext() { return *schemaStack_.template Top<Context>(); }
    const Context& CurrentContext() const { return *schemaStack_.template Top<Context>(); }

    static const size_t kDefaultSchemaStackCapacity = 1024;
    static const size_t kDefaultDocumentStackCapacity = 256;
    const SchemaDocument* schemaDocument_;
    const SchemaType& root_;
    BaseReaderHandler<EncodingType> nullOutputHandler_;
    OutputHandler& outputHandler_;
    CrtAllocator contextAllocator_;
    internal::Stack<StateAllocator> schemaStack_;    //!< stack to store the current path of schema (BaseSchemaType *)
    internal::Stack<StateAllocator> documentStack_;  //!< stack to store the current path of validating document (Ch)
    bool valid_;
#if RAPIDJSON_SCHEMA_VERBOSE
    unsigned depth_;
#endif
};

typedef GenericSchemaValidator<SchemaDocument> SchemaValidator;

RAPIDJSON_NAMESPACE_END

#if defined(__GNUC__)
RAPIDJSON_DIAG_POP
#endif

#endif // RAPIDJSON_SCHEMA_H_
