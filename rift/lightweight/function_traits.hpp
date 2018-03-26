#pragma once

#include "rift/lightweight/utils.hpp"

namespace rift {
enum class FunctionType {
    Static,
    MemberMethod
};
/**
        Describes properties of a given function.
        @tparam FuncType type of the function.

        TODO: support va_args
        TODO: support lambda functions
        TODO: return type list with function info
    */
template <typename FuncType>
class FunctionTraits {
private:
    template <typename... seq, typename Ret>
    static rift::TypePair<Ret, std::integral_constant<int, sizeof...(seq)>> discover_traits(Ret (*fn)(seq...));

    template <typename... seq, typename Ret, typename Klass>
    static rift::TypePair<Ret, std::integral_constant<int, sizeof...(seq)>> discover_traits(Ret (Klass::*fn)(seq...));

    using Pair = decltype(discover_traits(std::declval<FuncType>()));
    using ArgCount = typename Pair::second;

public:
    static constexpr int argumentCount = ArgCount::value;
    using ResultType = typename Pair::first;
};

} // namespace rift
