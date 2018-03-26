#pragma once

#include <type_traits>

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
} // namespace rift
