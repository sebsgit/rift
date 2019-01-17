#include "rift/tests/test_runner.hpp"
#include "rift/lightweight/function_traits.hpp"

namespace {
class ClassForFuncTests {
public:
    static void noArgsStatic();
    static void oneArgStatic(int);
    void noArgsMem();
    void noArgsMemConst() const;
    void oneArgMem(int);
    void oneArgMemConst(int) const;

    int operator()() const;
};

static void twoArgFunc(int, double)
{
}

static double funcWithNoArgs()
{
}
}

static void test_methods() {
    static_assert(rift::FunctionTraits<decltype(twoArgFunc)>::argumentCount == 2, "");
    static_assert(std::is_same<typename rift::FunctionTraits<decltype(twoArgFunc)>::ResultType, void>::value, "");
    static_assert(rift::FunctionTraits<decltype(funcWithNoArgs)>::argumentCount == 0, "");
    static_assert(std::is_same<typename rift::FunctionTraits<decltype(funcWithNoArgs)>::ResultType, double>::value, "");

    static_assert(rift::FunctionTraits<decltype(&ClassForFuncTests::operator())>::argumentCount == 0, "");
    static_assert(std::is_same<typename rift::FunctionTraits<decltype(&ClassForFuncTests::operator())>::ResultType, int>::value, "");

    static_assert(rift::FunctionTraits<decltype(&ClassForFuncTests::noArgsStatic)>::argumentCount == 0, "");
    static_assert(rift::FunctionTraits<decltype(&ClassForFuncTests::noArgsMem)>::argumentCount == 0, "");
    static_assert(rift::FunctionTraits<decltype(&ClassForFuncTests::noArgsMemConst)>::argumentCount == 0, "");
    static_assert(rift::FunctionTraits<decltype(&ClassForFuncTests::oneArgMem)>::argumentCount == 1, "");
    static_assert(rift::FunctionTraits<decltype(&ClassForFuncTests::oneArgMemConst)>::argumentCount == 1, "");
    static_assert(rift::FunctionTraits<decltype(&ClassForFuncTests::oneArgStatic)>::argumentCount == 1, "");
    static_assert(std::is_same<void, typename rift::FunctionTraits<decltype(&ClassForFuncTests::noArgsStatic)>::ClassType>::value, "");
    static_assert(std::is_same<ClassForFuncTests, typename rift::FunctionTraits<decltype(&ClassForFuncTests::noArgsMem)>::ClassType>::value, "");
}

static void test_lambda() {
    auto lambda = [](int) { return 9.0; };
    using trait = rift::FunctionTraits<decltype(lambda)>;
//    static_assert(trait::argumentCount == 1, "");
  //  static_assert(std::is_same_v<double, trait::ResultType>);
    std::cout << "lambda class: " << typeid(trait::ClassType).name() << '\n';
    std::cout << "return type of lambda: " << typeid(trait::ResultType).name() << '\n';
}

static void test_invalid_types() {
    using traitNoFunc = rift::FunctionTraits<int>;
    static_assert(traitNoFunc::argumentCount == -1, "");
}

RIFT_TEST(test_methods);
RIFT_TEST(test_lambda);
RIFT_TEST(test_invalid_types);
