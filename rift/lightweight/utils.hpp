#pragma once

#include <type_traits>
#include <functional>

namespace rift {
/**
    A simple type pair.
    @tparam First first type.
    @tparam Second second type.
*/
template <typename First, typename Second>
struct TypePair {
    using first = First;
    using second = Second;
};

namespace detail {
    struct apply_helper {
        template <typename Func, typename ... Args, size_t ... index>
        constexpr static decltype(auto) apply(Func&& f, std::index_sequence<index...>, Args&&... args)
        {
            return f(std::get<index>(std::forward_as_tuple(std::forward<Args>(args)...)) ...);
        }
    };
}

/**
    Forwards n arguments from the generic pack to the function call.
    @tparam n Number of parameters to forward.
    @tparam Func Deduced function type.
    @tparam Args Deduced argument types.
*/
template <size_t n, typename Func, typename... Args>
constexpr decltype(auto) apply(Func && f, Args&&... args)
{
    static_assert(n <= sizeof...(args), "Insufficient number of parameters to apply");
    return detail::apply_helper::apply(std::forward<Func>(f), std::make_index_sequence<n>(), std::forward<Args>(args)...);
}

} // namespace rift
