#pragma once
#include <type_traits>

/**
    Tests if a class contains a member method.
    Usage:
        RIFT_DECLARE_METHOD_TEST(bar);
        bool hasBarWithNoArgs = rift::HasMethod_bar<MyClass, void>::value;
        bool hasBarWithArgs = rift::HasMethod_bar<MyClass, int, double, std::string>:value;
*/
#define RIFT_DECLARE_METHOD_TEST(name)                                                           \
    namespace rift {                                                                             \
    template <typename T, typename... Args>                                                      \
    struct HasMethod_##name {                                                                    \
        template <typename U>                                                                    \
        static std::false_type test(...);                                                        \
        template <typename U>                                                                    \
        static std::true_type                                                                    \
        test(decltype(std::declval<U>().name(std::declval<Args>()...))*);                        \
                                                                                                 \
        static constexpr bool value = std::is_same<std::true_type, decltype(test<T>(0))>::value; \
    };                                                                                           \
                                                                                                 \
    template <typename T>                                                                        \
    struct HasMethod_##name<T, void> {                                                           \
        template <typename U>                                                                    \
        static std::false_type test(...);                                                        \
        template <typename U>                                                                    \
        static std::true_type test(decltype(std::declval<U>().name())*);                         \
                                                                                                 \
        static constexpr bool value = std::is_same<std::true_type, decltype(test<T>(0))>::value; \
    };                                                                                           \
    }
