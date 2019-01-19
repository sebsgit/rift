#pragma once

#include <type_traits>
#include <functional>

namespace rift {
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

template <typename ...>
struct TypeList;

template <typename T>
struct TypeList<T> {
    using type = T;
    using base = std::void_t<>;
    static constexpr size_t length = 1;
};

template <typename T, typename ... Rest>
struct TypeList<T, Rest...> : public TypeList<Rest...>
{
    using base = TypeList<Rest...>;
    using type = T;
    static constexpr size_t length = base::length + 1;
};

template <typename List, int index>
struct Indexer;

template <typename ... Args>
struct Indexer<TypeList<Args...>, 0>
{
    static_assert(sizeof...(Args), "Indexing an empty list");
    using type = typename TypeList<Args...>::type;
};

template <typename ... Args, int index>
struct Indexer<TypeList<Args...>, index>
{
    static_assert(index >= 0 && index < sizeof...(Args), "List index out of range");
    using type = typename Indexer<typename TypeList<Args...>::base, index - 1>::type;
};

} // namespace rift
