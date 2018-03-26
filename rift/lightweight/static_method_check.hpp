#pragma once
#include <type_traits>

/**
    Tests if a class contains a static method.
    Usage:
        RIFT_DECLARE_STATIC_METHOD_TEST(bar);
        bool hasBarWithNoArgs = rift::HasMethod_bar<MyClass, void>::value;
        bool hasBarWithArgs = rift::HasMethod_bar<MyClass, int, double, std::string>:value;
*/
#define RIFT_DECLARE_STATIC_METHOD_TEST(name)                                                    \
    namespace rift {                                                                             \
    template <typename T, typename... Args>                                                      \
    struct HasStaticMethod_##name {                                                              \
        template <typename U>                                                                    \
        static std::false_type test(...);                                                        \
        template <typename U>                                                                    \
        static std::true_type                                                                    \
        test(decltype(U::name(std::declval<Args>()...))*);                                       \
                                                                                                 \
        static constexpr bool value = std::is_same<std::true_type, decltype(test<T>(0))>::value; \
    };                                                                                           \
                                                                                                 \
    template <typename T>                                                                        \
    struct HasStaticMethod_##name<T, void> {                                                     \
        template <typename U>                                                                    \
        static std::false_type test(...);                                                        \
        template <typename U>                                                                    \
        static std::true_type test(decltype(U::name())*);                                        \
                                                                                                 \
        static constexpr bool value = std::is_same<std::true_type, decltype(test<T>(0))>::value; \
    };                                                                                           \
    }
