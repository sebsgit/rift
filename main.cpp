#include "rift/tests/test_runner.hpp"
#include <iostream>

#include "rift/lightweight/utils.hpp"

// another approach - the reflection data must be constexpr

using Types = rift::TypeList<int, double>;

using Type1 = rift::Indexer<Types, 0>::type;
using Type2 = rift::Indexer<Types, 1>::type;

static void test_func()
{
    std::cout << "static func\n";
}

class Test {
public:
    void method() {
        std::cout << "member method\n";
    }
};


template <typename Result, Result(*T)()>
struct FunctionWrapper {
    inline static auto ptr = T;
};

template <typename Result, typename Klass, Result(Klass::*T)()>
struct FunctionWrapper2 {
    inline static auto ptr = T;
};

//TODO: select wrapper with a template
using T1 = FunctionWrapper<void, &test_func>;
using T2 = FunctionWrapper2<void, Test, &Test::method>;

int main()
{
    static_assert(std::is_same_v<int, Type1>, "");
    static_assert(std::is_same_v<double, Type2>, "");

    Test obj;
    (obj.*T2::ptr)();

    (*T1::ptr)();

    try {
        rift::test_runner::run();
    } catch (const std::exception& exc) {
        std::cout << exc.what();
        return -1;
    }
}
