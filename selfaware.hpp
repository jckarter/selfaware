// (c)2012 Durian Software bsd license

#ifndef selfaware_hpp
#define selfaware_hpp

#include <cstdint>
#include <tuple>
#include <type_traits>

#define SELFAWARE_IDENTIFIER(fieldname) \
    template<typename Q_T> \
    struct fieldname { \
        template<typename U, typename ::selfaware::detail::enum_value_type<Q_T>::type N> \
        struct Q_enum { \
            static constexpr U fieldname = U(N); \
        }; \
        Q_T fieldname; \
        using Q_type = Q_T; \
        constexpr static char const *Q_name() { return #fieldname; } \
        Q_T &Q_value() & { return fieldname; } \
        Q_T const &Q_value() const & { return fieldname; } \
        Q_T &&Q_value() && { return fieldname; } \
    };

namespace selfaware {
    namespace detail {
        template<typename T, typename Enable = void> struct enum_value_type;
        template<typename T>
        struct enum_value_type<T, typename std::enable_if<std::is_integral<T>::value>::type> { using type = T; };
        template<typename T>
        struct enum_value_type<T, typename std::enable_if<!std::is_integral<T>::value>::type> { using type = int; };

        template<template<typename T> class FieldTemplate, typename...FieldInstances>
        struct find_field;
        template<template<typename T> class FieldTemplate, typename T, typename...FieldInstances>
        struct find_field<FieldTemplate, FieldTemplate<T>, FieldInstances...> {
            using field = FieldTemplate<T>;
            using type = T;
        };
        template<template<typename T> class FieldTemplate, typename FieldInstance, typename...FieldInstances>
        struct find_field<FieldTemplate, FieldInstance, FieldInstances...> : find_field<FieldTemplate, FieldInstances...> {};

        template<typename...> struct type_pack {};
        template<size_t...> struct integral_pack {};

        template<size_t N, size_t...I>
        struct make_seq_pack : make_seq_pack<N-1, N-1, I...> {};
        template<size_t...I>
        struct make_seq_pack<0, I...> { using type = integral_pack<I...>; };
    }
    
    template<typename...Fields>
    struct Struct : Fields... {
        using struct_type = Struct;
        using tuple_type = std::tuple<typename Fields::Q_type...>;
        using fields_pack = detail::type_pack<Fields...>;
        using indices_pack = typename detail::make_seq_pack<sizeof...(Fields)>::type;
        
        Struct() = default;
        Struct(Struct const &) = default;
        constexpr Struct(typename Fields::Q_type &&...x)
            : Fields{x}... {}

        template<typename...TT, size_t...NN>
        Struct(std::tuple<TT...> const &x, detail::integral_pack<NN...> indices)
            : Fields{std::get<NN>(x)}... {}

        template<typename...TT>
        Struct(std::tuple<TT...> const &x) : Struct(x, indices_pack()) {}

        // FIXME relies on undefined behavior, can't be constexpr
        template<typename Field>
        /*constexpr*/ static std::uintptr_t offset_of() {
            return reinterpret_cast<std::uintptr_t>(&static_cast<Struct*>(nullptr)->Field::Q_value());
        }
        
        template<template<typename T> class Field>
        /*constexpr*/ static std::uintptr_t offset_of() {
            return reinterpret_cast<std::uintptr_t>(&static_cast<Struct*>(nullptr)->detail::find_field<Field, Fields...>::field::Q_value());
        }
        
        template<typename Field>
        constexpr static std::uintptr_t size_of() { return sizeof(typename Field::Q_type); }

        template<template<typename T> class Field>
        constexpr static std::uintptr_t size_of() { return sizeof(typename detail::find_field<Field, Fields...>::type); }
        
        template<typename Field>
        constexpr static char const *name_of() { return Field::Q_name(); }

        template<template<typename T> class Field>
        constexpr static char const *name_of() {
            return detail::find_field<Field, Fields...>::field::Q_name();
        }
        
        template<template<typename T> class Field>
        using type_of = typename detail::find_field<Field, Fields...>::type;
        
        template<template<typename T> class Trait, typename Function>
        static void each_field(Function &&f)
        {
            char __attribute__((unused)) discard[] = {(
                f(Fields::Q_name(), offset_of<Fields>(), size_of<Fields>(),
                  Trait<typename Fields::Q_type>::value())
            , '\0')...};
        }

        template<typename Function>
        auto apply(Function &&f) & -> decltype(f(this->Fields::Q_value()...))
        { return f(this->Fields::Q_value()...); }
        template<typename Function>
        auto apply(Function &&f) const & -> decltype(f(this->Fields::Q_value()...))
        { return f(this->Fields::Q_value()...); }
        template<typename Function>
        auto apply(Function &&f) && -> decltype(f(this->Fields::Q_value()...))
        { return f(this->Fields::Q_value()...); }

        tuple_type as_tuple() const { return tuple_type(this->Fields::Q_value()...); }
        operator tuple_type() const { return tuple_type(this->Fields::Q_value()...); }
    };

    namespace detail {
        template<typename Subclass, typename Underlying>
        struct enum_base {
            using underlying_type = Underlying;
            Underlying value;
            
            enum_base() = default;
            constexpr explicit enum_base(Underlying value) : value(value) {}
            
            constexpr explicit operator Underlying() const { return value; }
            
            constexpr bool operator==(enum_base o) const { return value == o.value; }
            constexpr bool operator!=(enum_base o) const { return value != o.value; }
        };
    
        template<typename T, template<typename> class Member>
        struct enum_member {
            using type = typename Member<typename T::underlying_type>::template Q_enum<T, 0>;
        };
    }
    
    template<typename Underlying, template<typename> class...Members>
    struct Enum :
        detail::enum_base<Enum<Underlying, Members...>, Underlying>,
        detail::enum_member<detail::enum_base<Enum<Underlying, Members...>, Underlying>, Members>::type...
    {
        Enum() = default;
        constexpr Enum(detail::enum_base<Enum, Underlying> super) : detail::enum_base<Enum, Underlying>(super) {}
        constexpr explicit Enum(Underlying value) : detail::enum_base<Enum, Underlying>(value) {}
    };

}

#endif
