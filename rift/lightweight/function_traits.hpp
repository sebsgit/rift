#pragma once

#include <tuple>
#include <functional>

namespace rift {
enum class FunctionType {
    Static,
    MemberMethod,
    Functor,
    Invalid
};
/**
        Describes properties of a given function.
        @tparam FuncType type of the function.

        TODO: support va_args
    */
template <typename FuncType>
class FunctionTraits {
private:
    template <typename... seq, typename Ret>
    static std::tuple<Ret, std::integral_constant<int, sizeof...(seq)>, void> discover_traits(Ret (*fn)(seq...));

    template <typename... seq, typename Ret, typename Klass>
    static std::tuple<Ret, std::integral_constant<int, sizeof...(seq)>, Klass> discover_traits(Ret (Klass::*fn)(seq...));

    template <typename... seq, typename Ret, typename Klass>
    static std::tuple<Ret, std::integral_constant<int, sizeof...(seq)>, Klass> discover_traits(Ret(Klass::*fn)(seq...) const);

    template <typename... seq, typename Ret>
    static std::tuple<Ret, std::integral_constant<int, sizeof...(seq)>, std::function<Ret(seq...)>> discover_traits(std::function<Ret(seq...)>);

    template <typename T>
    static auto discover_traits(T&&) -> decltype(discover_traits(std::declval<decltype(&T::operator())>()));

    static std::tuple<std::void_t<>, std::integral_constant<int, -1>, std::void_t<>> discover_traits(...);

    using Tuple = decltype(discover_traits(std::declval<FuncType>()));
    using ArgCount = typename std::tuple_element<1, Tuple>::type;

public:
    static constexpr int argumentCount = ArgCount::value;
    using ResultType = typename std::tuple_element<0, Tuple>::type;
    using ClassType = typename std::tuple_element<2, Tuple>::type;
};
} // namespace rift
