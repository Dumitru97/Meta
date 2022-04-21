#pragma once

#include <iostream>
#include <iomanip>
#include <vector>
#include <array>
#include <span>
#include <experimental/meta>

namespace Meta
{
    namespace std = ::std;
    namespace meta = std::experimental::meta;

    template<typename T>
    using strip_type_t = std::remove_cvref_t<std::remove_pointer_t<T>>;

    template<typename T>
    concept isFundamentalStripped = std::is_fundamental_v<strip_type_t<T>>;

    template<typename T>
    concept isCompileTimeOnly = std::same_as<T, meta::info>;

    template<template<typename, auto> typename T, template<typename, auto> typename U>
    struct is_same_container_2 : std::false_type {};

    template<template<typename, auto> typename T>
    struct is_same_container_2<T, T> : std::true_type {};

    template<template<typename, auto> typename T, template<typename, auto> typename ...Us>
    inline constexpr bool is_any_of_containers_v_2 = ((is_same_container_2<T, Us>::value) || ...);

    template<template<typename, auto> typename C, typename T, auto N>
    consteval bool isArrayTypeCheck(const C<T, N>&) {
        return is_any_of_containers_v_2<C, std::array>;
    }

    template<template<typename...> typename T, template<typename...> typename U>
    struct is_same_container : std::false_type {};

    template<template<typename...> typename T>
    struct is_same_container<T, T> : std::true_type {};

    template<template<typename...> typename T, template<typename...> typename ...Us>
    inline constexpr bool is_any_of_containers_v = ((is_same_container<T, Us>::value) || ...);

    template<template<typename...> typename C, typename ...Ts>
    consteval bool isArrayTypeCheck(const C<Ts...>&) {
        return is_any_of_containers_v<C, std::vector>;
    }

    template<typename T>
    concept isArrayType = isArrayTypeCheck(T{});// || std::same_as<T, function>;

    struct Indent
    {
        constexpr Indent(int tabs) : tabs{ tabs } {};
        int tabs;
    };

    std::ostream& operator<<(std::ostream& os, const Indent& ind) {
        if (ind.tabs)
            return os << std::setw(ind.tabs) << std::setfill('\t') << "";
        else
            return os;
    }

    consteval const char* CheckToString(auto func, const char* trueString, meta::info objMeta) {
        return func(objMeta) ? trueString : "";
    }

    consteval void PrintName(meta::info objMeta, int tabs) {
        const char* type_name = meta::name_of(meta::type_of(objMeta));
        const char* var_name = meta::name_of(objMeta);
        const char* constexpr_str = CheckToString(meta::is_constexpr, "constexpr ", objMeta);
        const char* static_str = CheckToString(meta::is_static_data_member, "static ", objMeta);
        const char* specifiers = __concatenate(static_str, constexpr_str);

        ->fragment { std::cout << Indent(%{ tabs }) << %{specifiers} << %{ type_name } << " " << %{ var_name } << ";\n"; };
    }

    inline consteval void PrintNewLine() {
        ->fragment { std::cout << "\n"; };
    }

    consteval void cout(const isFundamentalStripped auto& val, int tabs, bool indent = true, bool nl = true) {
        if (!indent) { tabs = 0; }
        char nl_chr = nl ? '\n' : '\0';

        ->fragment {
            std::cout << Indent(%{ tabs }) << %{ val } << ' ' << %{ nl_chr };
        };
    }

    consteval void cout(const isCompileTimeOnly auto& val, int tabs, bool inden = true, bool nl = true) {
        ->fragment {
            std::cout << Indent(%{ tabs }) << "### Compile time only ###\n";
        };
    }

    template<size_t N>
    consteval void cout(const std::array<char, N>& arr, int tabs, bool indent = true, bool nl = true)
    {
        using ValueType = typename strip_type_t<decltype(arr)>::value_type;

        if (!isCompileTimeOnly<ValueType>) {
            if constexpr (isFundamentalStripped<ValueType>)
                ->fragment {
                std::cout << Indent(%{ tabs });
            };

            bool inner_indent = false;
            bool inner_nl = false;

            if constexpr (!isFundamentalStripped<ValueType>)
                inner_indent = true;

            for (const auto& el : arr) {
                cout((int)el, tabs, inner_indent, inner_nl);
                if constexpr (!isFundamentalStripped<ValueType>)
                    PrintNewLine();
            }

            if (isFundamentalStripped<ValueType> && indent && nl)
                PrintNewLine();
        }
        else {
            ->fragment {
                std::cout << Indent(%{ tabs }) << "### Compile time only ###\n";
            };
        }

    }

    consteval void cout(const isArrayType auto& arr, int tabs, bool indent = true, bool nl = true)
    {
        using ValueType = typename strip_type_t<decltype(arr)>::value_type;

        if (!isCompileTimeOnly<ValueType>) {
            if constexpr (isFundamentalStripped<ValueType>)
                ->fragment {
                    std::cout << Indent(%{ tabs });
                 };

            bool inner_indent = false;
            bool inner_nl = false;

            if constexpr (!isFundamentalStripped<ValueType>)
                inner_indent = true;

            for (const auto& el : arr) {
                cout(el, tabs, inner_indent, inner_nl);
                if constexpr (!isFundamentalStripped<ValueType>)
                    PrintNewLine();
            }

            if (isFundamentalStripped<ValueType> && indent && nl)
                PrintNewLine();
        }
        else {
            ->fragment {
                std::cout << Indent(%{ tabs }) << "### Compile time only ###\n";
            };
        }

    }

    template<meta::info el>
    consteval void PickCout(const auto& obj, int tabs, bool indent = true, bool nl = true) {
        if constexpr (!meta::is_static_data_member(el))
            cout(obj.[: el :], tabs, indent, nl);
        else
            cout([: el :], tabs, indent, nl);
    }

    template<bool firstPass = true>
    consteval void Print(const auto& obj, int tabs = 0, bool indent = true, bool nl = true) {
        constexpr auto objMeta = ^obj;
        if constexpr(firstPass) {
            PrintName(objMeta, tabs);
            cout(obj, tabs, indent, nl);
        }
        else {
            constexpr auto dataMembers = meta::data_member_range(meta::type_of(objMeta));
            template for (constexpr auto el : dataMembers) {
                PrintName(el, tabs + 1);
                PickCout<el>(obj, tabs + 1, indent, nl);
                PrintNewLine();
            }
        }
    }

    consteval void cout(const auto& val, int tabs, bool indent = true, bool nl = true) {
        Print<false>(val, tabs, indent, nl);
    }

    //TODO: consider specializing cout for "function", include function.h
}