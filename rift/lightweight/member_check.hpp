#pragma once
#include <type_traits>

/**
    Tests if a class contains a data member with a given name.
    Usage:
        RIFT_DECLARE_MEMBER_TEST(foo);
        bool contains = rift::HasMember_foo<MyClass, RequestedTypeOfFoo>::value;
*/
#define RIFT_DECLARE_MEMBER_TEST(member)                                                                                \
    namespace rift {                                                                                                    \
    namespace detail {                                                                                                  \
        template <bool, typename T, typename U>                                                                         \
        struct TypeCheck_##member;                                                                                      \
                                                                                                                        \
        template <typename T, typename U>                                                                               \
        struct TypeCheck_##member<false, T, U> {                                                                        \
            static constexpr bool value = false;                                                                        \
        };                                                                                                              \
                                                                                                                        \
        template <typename T, typename U>                                                                               \
        struct TypeCheck_##member<true, T, U> {                                                                         \
            static constexpr bool value = std::is_same<decltype(U::member), T>::value || std::is_same<void, T>::value;  \
        };                                                                                                              \
    }                                                                                                                   \
    template <typename T, typename TargetType = void>                                                                   \
    struct HasMember_##member {                                                                                         \
    private:                                                                                                            \
        template <typename U>                                                                                           \
        static std::false_type test(...);                                                                               \
                                                                                                                        \
        template <typename U>                                                                                           \
        static std::true_type test(decltype(U::member)*);                                                               \
                                                                                                                        \
        static constexpr bool memberPresent = std::is_same<std::true_type, decltype(test<T>(nullptr))>::value;          \
                                                                                                                        \
    public:                                                                                                             \
        static constexpr bool value = detail::TypeCheck_##member<memberPresent, TargetType, T>::value; \
    };                                                                                                                  \
    }
