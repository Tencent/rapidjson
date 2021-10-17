#include <rapidjson/reader.h>
#if RAPIDJSON_HAS_CXX17
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace rj = rapidjson;

namespace jsonstruct
{
    struct NullHandler: rj::BaseReaderHandler<rj::UTF8<>, NullHandler>
    {
        bool Default() { return false; }
    };

    template<typename Base = NullHandler>
    struct EndArrayHandler: Base
    {
        EndArrayHandler(bool& b, const Base& base = Base{}):
            Base(base), gotEnd(b) {}
            
        bool& gotEnd;
        bool EndArray(rj::SizeType){ return gotEnd = true; }
    };

    template<unsigned parseFlags, typename Reader, typename Stream>
    inline auto wrapParse(Reader& reader, Stream& s)
    {
        return [&](auto&& handler)
        {
            return reader.template IterativeParseNext<parseFlags>(s, handler);
        };
    }

    template<typename T, typename HANDLER>
    struct Value
    {
        using Handler = HANDLER;
        using type = T;
        T value;

        Value() = default;
        Value(T v): value(v) {}

        bool operator==(const Value& rhs) const { return rhs.value == value; }

        template<typename H>
        bool Accept(H& handler) const;
    };

    template<typename T>
    struct BaseHandler: NullHandler
    {
        BaseHandler(T* t): target(t) {}
        T* const  target;
        bool set(T t){ *target = t; return true; }
    };

    struct BoolHandler: BaseHandler<bool>
    {
        using BaseHandler::BaseHandler;
        bool Bool(bool b){ return set(b); }
    };
    using Bool = Value<bool, BoolHandler>;

    struct IntHandler: BaseHandler<int>
    {
        using BaseHandler::BaseHandler;
        bool Int(int i)       { return set(i); }
        bool Uint(unsigned i) { return set(static_cast<int>(i)); }
    };
    using Int = Value<int, IntHandler>;

    struct UintHandler: BaseHandler<unsigned>
    {
        using BaseHandler::BaseHandler;
        bool Uint(unsigned i)  { return set(i); }
    };
    using Uint = Value<unsigned, UintHandler>;

    struct Int64Handler: BaseHandler<std::int64_t>
    {
        using BaseHandler::BaseHandler;
        bool Int64(std::int64_t i)   { return set(i); }
        bool Uint64(std::uint64_t i) { return set(static_cast<std::int64_t>(i)); }
        bool Int(int i)              { return set(i); }
        bool Uint(unsigned i)        { return set(i); }
    };
    using Int64 = Value<std::int64_t, Int64Handler>;

    struct Uint64Handler: BaseHandler<std::uint64_t>
    {
        using BaseHandler::BaseHandler;
        bool Uint64(std::uint64_t i){ return set(i); }
    };
    using Uint64 = Value<std::uint64_t, Uint64Handler>;

    struct DoubleHandler: BaseHandler<double>
    {
        using BaseHandler::BaseHandler;
        bool Double(double d)        { return set(d); }
        bool Uint64(std::uint64_t i) { return set(static_cast<double>(i)); }
        bool Int64(std::int64_t i)   { return set(static_cast<double>(i)); }
        bool Uint(unsigned i)        { return set(i); }
    };
    using Double = Value<double, DoubleHandler>;

    struct RawNumberHandler: BaseHandler<std::string>
    {
        using BaseHandler::BaseHandler;
        template<typename Ch>
        bool RawNumber(const Ch* str, rj::SizeType length, bool)
        {
            target->assign(str, length);
            return true;
        }
    };
    using RawNumber = Value<std::string, RawNumberHandler>;

    struct StringHandler: BaseHandler<std::string>
    {
        using BaseHandler::BaseHandler;
        template<typename Ch>
        bool String(const Ch* str, rj::SizeType length, bool)
        {
            target->assign(str, length);
            return true;
        }
    };
    using String = Value<std::string, StringHandler>;

    namespace detail
    {
        template<typename W>
        inline bool Accept(const Bool& b, W& w)   { return w.Bool(b.value); }

        template<typename W>
        inline bool Accept(const Int& i, W& w)    { return w.Int(i.value); }

        template<typename W>
        inline bool Accept(const Uint& i, W& w)   { return w.Uint(i.value); }

        template<typename W>
        inline bool Accept(const Int64& i, W& w)  { return w.Int64(i.value); }

        template<typename W>
        inline bool Accept(const Uint64& i, W& w) { return w.Uint64(i.value); }

        template<typename W>
        inline bool Accept(const Double& d, W& w) { return w.Double(d.value); }

        template<typename W>
        inline bool Accept(const RawNumber& rn, W& writer)
        {
            return writer.RawNumber(rn.value.c_str(), rn.value.size(), true);
        }

        template<typename W>
        inline bool Accept(const String& str, W& writer)
        {
            return writer.String(str.value.c_str(), static_cast<rj::SizeType>(str.value.size()), true);
        }
    }

    template<typename T, typename Handler>
    template<typename H>
    inline bool Value<T, Handler>::Accept(H& handler) const
    {
        return detail::Accept(*this, handler);
    }

    template<unsigned parseFlags, typename T>
    struct ArrayMemberParser
    {
        using StartHandler = typename T::StartHandler;
        
        template<typename Reader, typename Stream>
        bool ParseMember(Reader& r, Stream& s, T& value, bool& gotEnd)
        {
            const auto next = wrapParse<parseFlags>(r, s);
            return next(EndArrayHandler<StartHandler>{gotEnd}) &&
                (gotEnd || value.template ParseContent<parseFlags>(r, s));
        }
    };

    template<unsigned parseFlags, typename T, typename Handler>
    struct ArrayMemberParser<parseFlags, Value<T, Handler> >
    {
        template<typename Reader, typename Stream>
        bool ParseMember(
            Reader& r, Stream& s, Value<T, Handler>& value, bool& gotEnd)
        {
            return wrapParse<parseFlags>(r, s)(
                EndArrayHandler<Handler>{gotEnd, &value.value});
        }
    };
    
    template<typename T>
    struct Array: std::vector<T>
    {
        using std::vector<T>::vector;
        using std::vector<T>::begin;
        using std::vector<T>::end;
        using std::vector<T>::size;

        template<unsigned parseFlags, typename Reader, typename Stream>
        bool ParseContent(Reader& reader, Stream& s)
        {
            T tmp;
            bool end = false;
            ArrayMemberParser<parseFlags, T> p;
            while(p.ParseMember(reader, s, tmp, end) && !end)
            {
                std::vector<T>::push_back(tmp);
            }
            return end;
        }

        template<unsigned parseFlags, typename Reader, typename Stream>
        bool Parse(Reader& reader, Stream& s)
        {
            return wrapParse<parseFlags>(reader, s)(StartHandler{}) &&
                ParseContent<parseFlags>(reader, s);
        }

        struct StartHandler: NullHandler
        {
            bool StartArray(){ return true; }
        };
        
        template<typename Handler>
        bool Accept(Handler& handler) const
        {
            return handler.StartArray() &&
                std::all_of(
                    begin(), end(),
                    [&](auto&& v){ return v.Accept(handler); }) &&
                handler.EndArray(static_cast<rj::SizeType>(size()));
        }
    };

#define DEFINE_FIELD_NAME(nm) \
    struct nm { static constexpr decltype(auto) name(){ return #nm; } };

    template<typename Name, typename Type>
    struct Field: Name
    {
        Type value;
        using Name::name;
        using StartHandler = typename Type::StartHandler;
        
        Field& operator=(const Type& rhs)
        {
            value = rhs;
            return *this;
        }
        
        template<unsigned parseFlags, typename Reader, typename Stream>
        bool Parse(Reader& r, Stream& s)
        {
            return value.template Parse<parseFlags>(r, s);
        }

        template<typename Writer>
        bool Accept(Writer& writer) const
        {
            return writer.Key(name(), sizeof(name()) - 1, false) &&
                value.Accept(writer);
        }
    };

    template<typename Name, typename T, typename H>
    struct Field<Name, Value<T, H> >: Name, Value<T, H>
    {
        using Handler = H;
        using Type = Value<T, H>;
        using Type::Type;
        using Type::value;
        using Name::name;

        Field& operator=(const Type& rhs)
        {
            Type::operator=(rhs);
            return *this;
        }
        
        template<unsigned parseFlags, typename Reader, typename Stream>
        bool Parse(Reader& r, Stream& s)
        {
            return wrapParse<parseFlags>(r, s)(Handler{&value});
        }

        template<typename Writer>
        bool Accept(Writer& writer) const
        {
            return writer.Key(name(), sizeof(name()) - 1, false) &&
                Type::Accept(writer);
        }
    };

    template<typename... Fields>
    struct ObjectIsBuiltFromFields: std::false_type {};

    template<typename... Names, typename... Types>
    struct ObjectIsBuiltFromFields<Field<Names, Types>...>: std::true_type {};

    template<typename... Fields>
    struct Object: Fields...
    {
        static_assert(ObjectIsBuiltFromFields<Fields...>::value,
                      "An Object's members must all be fields.");

        Object() = default;
        Object(const Object&) = default;
        Object(Object&&) = default;

        template<typename... T>
        Object(T&&... args): Fields(std::forward<T>(args))... {}
        
        Object& operator=(const Object& rhs) = default;

        template<typename Field, unsigned parseFlags, typename... Args>
            auto ParseField(Args&&... args)
        {
            return &Field::template Parse<parseFlags, Args...>(
                std::forward<Args>(args)...);
        }

        template<unsigned parseFlags, typename... Args>
        static auto objectIndex(bool (Object::*parseMethod)(Args&...))
        {
            return std::unordered_map<std::string, decltype(parseMethod)>{
                {
                    {
                        Fields::name(),
                        &Fields::template Parse<parseFlags, Args...>
                    }...
                }
            };
        }

        template<unsigned parseFlags, typename FieldParseMethod>
        struct KeyHandler: NullHandler
        {
            FieldParseMethod parseField;

            template<typename Ch>
            bool Key(const Ch* str, rj::SizeType length, bool)
            {
                static const auto& index = objectIndex<parseFlags>(parseField);
                const auto found = index.find(std::string(str, length));
                if(found != index.end())
                {
                    parseField = found->second;
                    return true;
                }
                return false;
            }

            bool EndObject(rj::SizeType)
            {
                parseField = nullptr;
                return true;
            }
        };
        
        template<unsigned parseFlags, typename Reader, typename Stream>
        bool ParseContent(Reader& r, Stream& s)
        {
            const auto next = wrapParse<parseFlags>(r, s);
            KeyHandler<parseFlags, bool (Object::*)(Reader&, Stream&)> handler;
            while(next(handler) && handler.parseField)
            {
                (this->*handler.parseField)(r, s);
            }
            return !handler.parseField;
        }

        template<unsigned parseFlags, typename Reader, typename Stream>
        bool Parse(Reader& reader, Stream& s)
        {
            return wrapParse<parseFlags>(reader, s)(StartHandler{}) &&
                ParseContent<parseFlags>(reader, s);
        }

        struct StartHandler: NullHandler
        {
            bool StartObject() { return true; }
        };

        template<typename Handler>
        bool Accept(Handler& handler) const
        {
            return handler.StartObject() &&
                std::min({Fields::Accept(handler)...}) &&
                handler.EndObject(sizeof...(Fields));
        }

        template<typename Name, typename Type>
        static       auto& getImpl(Name,       Field<Name, Type>& field)
        {
            return field.value;
        }

        template<typename Name, typename Type>
        static const auto& getImpl(Name, const Field<Name, Type>& field)
        {
            return field.value;
        }

        template<typename Name> const auto& get() const { return get(Name{}); }
        template<typename Name> auto& get() { return get(Name{}); }
        template<typename Name, typename T> void set(T&& t)       { return get(Name{}, t); }

        template<typename Name>
        const auto& get(Name nm) const { return getImpl(nm, *this); }

        template<typename Name>
        auto& get(Name nm) { return getImpl(nm, *this); }
        
        template<typename Name, typename T>
        void set(Name nm, T&& t)       { getImpl(nm, *this) = t; }
    };

    template<typename... Names, typename... Values>
    bool operator==(const Object<Field<Names, Values>...>& lhs,
                    const Object<Field<Names, Values>...>& rhs) 
    {
        return std::min({(lhs.get(Names{}) == rhs.get(Names{}))...});
    }
}
#endif
