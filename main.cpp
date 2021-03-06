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

#define RIFT_GET_CHAR_1(str, i) ((i) < sizeof(str) ? str[(i)] : '\n')
#define RIFT_GET_CHAR_4(str, i) RIFT_GET_CHAR_1(str, i), RIFT_GET_CHAR_1(str, i+1), RIFT_GET_CHAR_1(str, i+2), RIFT_GET_CHAR_1(str, i+3)
#define RIFT_GET_CHAR_16(str, i) RIFT_GET_CHAR_4(str, i), RIFT_GET_CHAR_4(str, i+4), RIFT_GET_CHAR_4(str, i+8), RIFT_GET_CHAR_4(str, i+12)

#define RIFT_UNWRAP(str) RIFT_GET_CHAR_16(str, 0)

enum class FType {
    Static,
    Member
};

template <typename Result, Result(*T)()>
struct FunctionWrapper {
    inline static auto ptr = T;
    static constexpr FType kind = FType::Static;
};

template <typename Result, typename Klass, Result(Klass::*T)()>
struct FunctionWrapper2 {
    inline static auto ptr = T;
    static constexpr FType kind = FType::Member;
};

template <typename Type, char ... _name>
struct TypeWrapper {
    using type = Type;
    static constexpr char name[] = { _name ... };

    static bool compare(const char * str) noexcept
    {
        return strcmp(str, name) == 0;
    }
};

//TODO: select wrapper with a template
using T1 = FunctionWrapper<void, &test_func>;
using T2 = FunctionWrapper2<void, Test, &Test::method>;

using WrappedMethod = TypeWrapper<T2, RIFT_UNWRAP("method")>;
using WrappedFunction = TypeWrapper<T1, RIFT_UNWRAP("test_func")>;

using ReflectedItems = rift::TypeList<WrappedFunction, WrappedMethod>;

template <typename FuncType, typename Result, FType type = FuncType::kind>
struct dispatch_helper;

template <typename FuncType, typename Result>
struct dispatch_helper<FuncType, Result, FType::Static>
{
    template <typename ... Args>
    static Result call(Args&& ... args)
    {
        static_assert(FuncType::kind == FType::Static);
        using FinalType = Result(*)(Args...);
        auto ptr = (FinalType*)(&FuncType::ptr);
        return (*ptr)(std::forward<Args>(args)...);
    }
};

template <typename FuncType, typename Result>
struct dispatch_helper<FuncType, Result, FType::Member>
{
    template <typename Klass, typename ... Args>
    static Result call(Klass& obj, Args&& ... args) //TODO: sfinae: is_convertible<FuncType::Klass, Klass>
    {
        static_assert(FuncType::kind == FType::Member);
        using FinalType = Result(Klass::*)(Args...);
        auto ptr = (FinalType*)(&FuncType::ptr);
        return (obj.*(*ptr))(std::forward<Args>(args)...);
    }

    template <typename ... Args>
    static Result call(Args&& ... args)
    {
        std::cout << "member method call needs object\n";
        throw ""; //TODO add runtime rift exception class (insufficient parameters for member method call)
    }
};

template <typename FuncType, typename Result, typename ... Args>
static Result dispatch(Args&& ... args)
{
    return dispatch_helper<FuncType, Result>::call(std::forward<Args>(args)...);
}

template <typename WrappedItems, size_t index, typename Result>
struct TryCall {
    using T = typename rift::Indexer<WrappedItems, index>::type;
    using FuncType = typename T::type;
    template <typename ... Args>
    Result operator() (const char* str, Args&& ... args) const {
        if (T::compare(str)) {
            std::cout << "function found at " << index << '\n';
            return dispatch<FuncType, Result>(std::forward<Args>(args)...);
        }
        else {
            return TryCall<WrappedItems, index - 1, Result>().operator()(str, std::forward<Args>(args)...);
        }
    }
};

template <typename WrappedItems, typename Result>
struct TryCall <WrappedItems, 0, Result>{
    template <typename ... Args>
    Result operator() (const char* str, Args&& ... args) const {
        using T = typename rift::Indexer<WrappedItems, 0>::type;
        using FuncType = typename T::type;
        if (T::compare(str)) {
            std::cout << "function found at 0 \n";
            return dispatch<FuncType, Result>(std::forward<Args>(args)...);
        }
        else {
            std::cout << "function not found\n";
            //TODO: throw
        }
    }
};

template <typename WrappedItems, typename Result, typename ... Args>
static Result call(const char* str, Args&& ... args)
{
    return TryCall<WrappedItems, WrappedItems::length - 1, Result>().operator()(str, std::forward<Args>(args)...);
}

int main()
{
    static_assert(std::is_same_v<int, Type1>, "");
    static_assert(std::is_same_v<double, Type2>, "");

    call<ReflectedItems, void>("test_func");
    Test obj;
    call<ReflectedItems, void>("method", obj);

    try {
        rift::test_runner::run();
    } catch (const std::exception& exc) {
        std::cout << exc.what();
        return -1;
    }
}
