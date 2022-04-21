#pragma once

#include "../3_Functions.h"
#include <format>

template<typename T>
concept is_container = requires(T & cont) {
    cont.begin();
    cont.begin()++;
    cont.end();
};

struct functionGetID {
    const Meta::function& func;
};

template<typename T>
struct AdaptType {
    using type = T;
};

template<typename T, typename AdaptType>
struct FormatAdapter {
    FormatAdapter(const T& adaptee, AdaptType)
        : adaptee(adaptee)
    {}

    using adapt_type = typename AdaptType::type;
    const T& adaptee;
};

template<typename T, typename AdaptType>
FormatAdapter(const T& adaptee, AdaptType)->FormatAdapter<std::remove_cvref_t<T>, AdaptType>;


template<typename T, typename AdaptType>
struct ContainerFormatAdapter {
    ContainerFormatAdapter(const T& adaptee, AdaptType)
        : adaptee(adaptee)
    {}

    using adapt_type = typename AdaptType::type;
    const T& adaptee;
};

template<typename T, typename AdaptType>
ContainerFormatAdapter(const T& adaptee, AdaptType)->ContainerFormatAdapter<std::remove_cvref_t<T>, AdaptType>;


template<typename T, typename AdaptType>
struct std::formatter<FormatAdapter<T, AdaptType>>
{
    using adapt_type = typename FormatAdapter<T, AdaptType>::adapt_type;

    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const FormatAdapter<T, AdaptType>& adapter, FormatContext& ctx) {
        return std::format_to(ctx.out(), "{}", adapt_type{ adapter.adaptee });
    }
};

template<typename T, typename AdaptType>
struct std::formatter<ContainerFormatAdapter<T, AdaptType>>
{
    using adapt_type = typename ContainerFormatAdapter<T, AdaptType>::adapt_type;

    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    constexpr auto fmt(auto ctxIt, auto contIt, auto contEnd) {
        if (contIt == contEnd)
            return ctxIt;

        return fmt(std::format_to(ctxIt, ", {}", adapt_type{ *contIt }), contIt + 1, contEnd);
    }

    template<typename FormatContext>
    constexpr auto format(const ContainerFormatAdapter<T, AdaptType>& adapter, FormatContext& ctx) {
        const auto& cont = adapter.adaptee;

        if (cont.empty())
            return ctx.out();

        return fmt(std::format_to(ctx.out(), "{}", adapt_type{ *cont.begin() }), cont.begin() + 1, cont.end());
    }
};

template<>
struct std::formatter<functionGetID>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const functionGetID& wrapper, FormatContext& ctx) {
        return std::format_to(ctx.out(), "{}", wrapper.func.ID);
    }
};

template<is_container T>
struct std::formatter<T>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    constexpr auto fmt(auto ctxIt, auto contIt, auto contEnd) {
        if (contIt == contEnd)
            return ctxIt;

        return fmt(std::format_to(ctxIt, ", {}", *contIt), contIt + 1, contEnd);
    }

    template<typename FormatContext>
    constexpr auto format(const T& cont, FormatContext& ctx) {
        if (cont.empty())
            return ctx.out();

        return fmt(std::format_to(ctx.out(), "{}", *cont.begin()), cont.begin() + 1, cont.end());
    }
};