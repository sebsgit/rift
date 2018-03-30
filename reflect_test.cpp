#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "rift/lightweight/function_traits.hpp"
#include "rift/lightweight/member_check.hpp"
#include "rift/lightweight/method_check.hpp"
#include "rift/lightweight/static_method_check.hpp"

struct Data {
private:
    int value;
};

struct Data2 {
    std::string str;
};

class Priv {
private:
    int x;

public:
    void f(double, int) {}

    static int build(const char) { return 8; }
};

namespace rift {
//TODO: fix with reference types
class GenericArgument {
public:
    template <typename T = int>
    GenericArgument(const T& t = T{})
        : _ref(&t)
    {
    }
    template <typename T>
    operator const T() const noexcept
    {
        return _ref ? *reinterpret_cast<const T*>(_ref) : T{};
    }

private:
    const void* _ref;
};

struct IgnoreThis {
};

class ReturnArgument {
public:
    template <typename T>
    ReturnArgument(T& t)
        : _ref(&t)
    {
    }
    template <typename T>
    auto operator=(const T& value) const
    {
        if (this->_ref)
            *reinterpret_cast<T*>(const_cast<ReturnArgument*>(this)->_ref) = value;
        return *this;
    }
    template <>
    auto operator=<IgnoreThis>(const IgnoreThis&) const
    {
        return *this;
    }

protected:
    ReturnArgument()
        : _ref(nullptr)
    {
    }

private:
    void* _ref;
};

class IgnoreReturnValue : public ReturnArgument {
};

template <bool isResultVoid, size_t argumentCount>
struct InvokeHelper {
public:
    template <typename Obj, typename Func, typename... Args>
    static decltype(auto) invokeIt(Obj&& obj, Func f, Args... args)
    {
        return rift::apply<argumentCount>([&](auto&& ... p) { return (obj.*f)(p...); }, std::forward<Args>(args)...);
    }
};

template <size_t argumentCount>
struct InvokeHelper<true, argumentCount> {
public:
    template <typename Obj, typename Func, typename... Args>
    static auto invokeIt(Obj&& obj, Func f, Args... args)
    {
        rift::apply<argumentCount>([&](auto&& ... p) { return (obj.*f)(p...); }, std::forward<Args>(args)...);
        return IgnoreThis();
    }
};

template <int argumentCount>
struct Invoker {
    template <typename Obj, typename Func, typename... Args>
    static decltype(auto) invokeIt(Obj&& obj, Func f, Args... args)
    {
        //TODO: remove this and use implementation from Invoker2
        using type = decltype((obj.*f)(args...));
        return InvokeHelper<std::is_same_v<type, void>, argumentCount>::invokeIt(std::forward<Obj>(obj), std::forward<Func>(f), std::forward<Args>(args)...);
    }
};

template <>
struct Invoker<0> {
    template <typename Obj, typename Func, typename... Args>
    static decltype(auto) invokeIt(Obj&& obj, Func f, Args...)
    {
        using type = decltype((obj.*f)());
        return InvokeHelper<std::is_same_v<type, void>, 0>::invokeIt(std::forward<Obj>(obj), std::forward<Func>(f));
    }
};


template <int argumentCount, typename RetType>
struct Invoker2 {
    template <typename Obj, typename Func, typename... Args>
    static decltype(auto) invokeIt(Obj&& obj, Func f, Args... args)
    {
        return InvokeHelper<std::is_same<RetType, void>::value, argumentCount>::invokeIt(std::forward<Obj>(obj), std::forward<Func>(f), std::forward<Args>(args)...);
    }
};

template <typename RetType>
struct Invoker2<0, RetType> {
    template <typename Obj, typename Func, typename... Args>
    static decltype(auto) invokeIt(Obj&& obj, Func f, Args...)
    {
        return InvokeHelper<std::is_same<RetType, void>::value, 0>::invokeIt(std::forward<Obj>(obj), std::forward<Func>(f));
    }
};
}

#define RIFT_IGNORE_RETURN() rift::IgnoreReturnValue()

#define RIFT_REFLECT(...)

#define RIFT_INVOKABLE_IMPL_1(func, arg_type) \
    if (riftFnName == #func)                  \
        ret = rift::Invoker<!std::is_same_v<void, arg_type>>::invokeIt(object, &func, arg0);
#define RIFT_INVOKABLE_IMPL_2(func, arg_type0, arg_type1) \
    if (riftFnName == #func)                              \
        ret = rift::Invoker<2>::invokeIt(object, &func, arg0, arg1);
#define RIFT_INVOKABLE_IMPL_3(func, arg_type0, arg_type1, arg_type2) \
    if (riftFnName == #func)                                         \
        ret = rift::Invoker<3>::invokeIt(object, &func, arg0, arg1, arg2);

#define RIFT_EXPAND(x) x

#define RIFT_SELECT_MACRO(_1, _2, _3, _4, MACRO_NAME, ...) MACRO_NAME
#define RIFT_INVOKABLE(...) RIFT_EXPAND(RIFT_SELECT_MACRO(__VA_ARGS__, RIFT_INVOKABLE_IMPL_3, RIFT_INVOKABLE_IMPL_2, RIFT_INVOKABLE_IMPL_1, 0)(__VA_ARGS__))

#define RIFT_DECLARE_INVOKABLE(klassName) template <typename T>  \
static void invoke(T&& object, const std::string& riftFnName,    \
    const rift::ReturnArgument& ret = rift::IgnoreReturnValue(), \
    rift::GenericArgument arg0 = rift::GenericArgument(),        \
    rift::GenericArgument arg1 = rift::GenericArgument(),        \
    rift::GenericArgument arg2 = rift::GenericArgument())

namespace rift {

template <typename T>
class AbstractFunctionInfo {
public:
    explicit AbstractFunctionInfo(const std::string& name)
        : _name(name)
    {
    }
    const std::string& name() const noexcept { return this->_name; }
    virtual AbstractFunctionInfo<T>* clone() const = 0;
    virtual void invoke(T& object, const rift::ReturnArgument& ret = rift::IgnoreReturnValue(),
        rift::GenericArgument arg0 = rift::GenericArgument(),
        rift::GenericArgument arg1 = rift::GenericArgument(),
        rift::GenericArgument arg2 = rift::GenericArgument())
        = 0;

    virtual void invoke(T& object,
        rift::GenericArgument arg0 = rift::GenericArgument(),
        rift::GenericArgument arg1 = rift::GenericArgument(),
        rift::GenericArgument arg2 = rift::GenericArgument())
        = 0;

private:
    const std::string _name;
};

template <typename T, typename Func>
class FunctionInfo : public AbstractFunctionInfo<T> {
public:
    template <typename F>
    FunctionInfo(const std::string& name, F&& f)
        : AbstractFunctionInfo<T>(name)
        , _func(std::forward<F>(f))
    {
    }
    AbstractFunctionInfo<T>* clone() const override
    {
        return new FunctionInfo<T, Func>(this->name(), _func);
    }
    static constexpr int argumentCount() noexcept
    {
        return rift::FunctionTraits<Func>::argumentCount;
    }
    void invoke(T& object, const rift::ReturnArgument& ret = rift::IgnoreReturnValue(),
        rift::GenericArgument arg0 = rift::GenericArgument(),
        rift::GenericArgument arg1 = rift::GenericArgument(),
        rift::GenericArgument arg2 = rift::GenericArgument()) override
    {
        ret = rift::Invoker2<argumentCount(), typename rift::FunctionTraits<Func>::ResultType>::invokeIt(object, _func, arg0, arg1, arg2);
    }

    void invoke(T& object,
        rift::GenericArgument arg0 = rift::GenericArgument(),
        rift::GenericArgument arg1 = rift::GenericArgument(),
        rift::GenericArgument arg2 = rift::GenericArgument()) override
    {
        rift::Invoker2<argumentCount(), typename rift::FunctionTraits<Func>::ResultType>::invokeIt(object, _func, arg0, arg1, arg2);
    }

private:
    Func _func;
};

class NoSuchMethod : public std::runtime_error {
public:
    explicit NoSuchMethod(const std::string& name)
        : std::runtime_error(name)
    {
    }
};

template <typename T>
class Reflect {
public:
    template <typename... FuncList>
    explicit Reflect(const FunctionInfo<T, FuncList>&... rest)
    {
        this->insert(rest...);
    }

    //TODO: support static functions, no return parameter etc...
    void invoke(T& object, const std::string& riftFnName,
        const rift::ReturnArgument& ret = rift::IgnoreReturnValue(),
        rift::GenericArgument arg0 = rift::GenericArgument(),
        rift::GenericArgument arg1 = rift::GenericArgument(),
        rift::GenericArgument arg2 = rift::GenericArgument())
    {
        auto it = this->_functions.find(riftFnName);
        if (it == this->_functions.end())
            throw NoSuchMethod(riftFnName);
        (*it).second->invoke(object, ret, arg0, arg1, arg2);
    }

    void invoke(T& object, const std::string& riftFnName,
        rift::GenericArgument arg0 = rift::GenericArgument(),
        rift::GenericArgument arg1 = rift::GenericArgument(),
        rift::GenericArgument arg2 = rift::GenericArgument())
    {
        auto it = this->_functions.find(riftFnName);
        if (it == this->_functions.end())
            throw NoSuchMethod(riftFnName);
        (*it).second->invoke(object, arg0, arg1, arg2);
    }

private:
    template <typename Func>
    void insert(const FunctionInfo<T, Func>& info)
    {
        this->_functions[info.name()].reset(info.clone());
    }
    template <typename Func, typename... FuncList>
    void insert(const FunctionInfo<T, Func>& info, const FunctionInfo<T, FuncList>&... rest)
    {
        this->insert(info);
        this->insert(rest...);
    }

private:
    std::unordered_map<std::string, std::unique_ptr<AbstractFunctionInfo<T>>> _functions;
};
}

struct Another {
    int x;
    double y;
    std::string str;

    void f() { std::cout << "hello\n"; }
    int g(int x) { return x * 2; }
    double h(double a, double b) { return a + b; }

    RIFT_REFLECT(x, y, str);

    //TODO: change to an object of type rift::Reflect
    RIFT_DECLARE_INVOKABLE(Another)
    {
        RIFT_INVOKABLE(f, void)
        RIFT_INVOKABLE(g, int)
        RIFT_INVOKABLE(h, double, double);
    }
};

class Base {
public:
    virtual ~Base() = default;
    virtual int value() const = 0;

    RIFT_DECLARE_INVOKABLE(Base)
    {
        RIFT_INVOKABLE(value, void)
    }
};

// adding reflection to a class without modifying it
class BaseReflect {
public:
    RIFT_DECLARE_INVOKABLE(Base)
    {
        RIFT_INVOKABLE(Base::value, void)
    }
};

class Derived : public Base {
public:
    int value() const override
    {
        return 10;
    }
};

class Person {
public:
    auto id() const { return this->_id; }
    auto name() const { return this->_name; }

    void setId(int newId) { _id = newId; }
    void setName(const std::string& newName) { _name = newName; }

    int doSomething(Base* base)
    {
        return base->value();
    }

    RIFT_DECLARE_INVOKABLE(Person)
    {
        RIFT_INVOKABLE(id, void)
        RIFT_INVOKABLE(name, void)
        RIFT_INVOKABLE(setId, int)
        RIFT_INVOKABLE(setName, std::string)
        RIFT_INVOKABLE(doSomething, Base*)
    }

    static rift::Reflect<Person>& reflection()
    {
        static rift::Reflect<Person> data{
            rift::FunctionInfo<Person, decltype(&setId)>{ "setId", &setId },
            rift::FunctionInfo<Person, decltype(&setName)>{ "setName", &setName }
        };
        return data;
    }

private:
    int _id;
    std::string _name;
};

RIFT_DECLARE_MEMBER_TEST(value);
RIFT_DECLARE_MEMBER_TEST(str);
RIFT_DECLARE_MEMBER_TEST(x);

RIFT_DECLARE_METHOD_TEST(f);
RIFT_DECLARE_METHOD_TEST(begin);
RIFT_DECLARE_METHOD_TEST(end);
RIFT_DECLARE_STATIC_METHOD_TEST(f);
RIFT_DECLARE_STATIC_METHOD_TEST(build);

#define RIFT_ASSERT(x)                              \
    if (!(x)) {                                     \
        std::cout << "Assertion: " #x " failed!\n"; \
        char c;                                     \
        std::cin >> c;                              \
        exit(-1);                                   \
    }

static void twoArgFunc(int, double)
{
}

static double funcWithNoArgs()
{
}

//TODO: separate into test cases
int main()
{
    using namespace rift;

    static_assert(!HasMember_value<Data2>::value, "");
    static_assert(HasMember_value<Data>::value, "");
    static_assert(HasMember_str<Data2>::value, "");

    static_assert(HasMember_str<Data2, std::string>::value, "");
    static_assert(!HasMember_str<Data2, int>::value, "");
    static_assert(!HasMember_str<Data2, const std::string>::value, "");

    static_assert(HasMember_x<Priv, int>::value, "");

    static_assert(HasMethod_f<Priv, double, int>::value, "");
    static_assert(!HasStaticMethod_f<Priv, double, int>::value, "");
    static_assert(HasStaticMethod_build<Priv, char>::value, "");

    static_assert(HasMethod_begin<std::vector<int>>::value, "");

    static_assert(rift::FunctionTraits<decltype(twoArgFunc)>::argumentCount == 2, "");
    static_assert(std::is_same<typename rift::FunctionTraits<decltype(twoArgFunc)>::ResultType, void>::value, "");
    static_assert(rift::FunctionTraits<decltype(funcWithNoArgs)>::argumentCount == 0, "");
    static_assert(std::is_same<typename rift::FunctionTraits<decltype(funcWithNoArgs)>::ResultType, double>::value, "");
    static_assert(rift::FunctionTraits<decltype(&Another::g)>::argumentCount == 1, "");
    static_assert(std::is_same<typename rift::FunctionTraits<decltype(&Another::g)>::ResultType, int>::value, "");

    auto fn = rift::FunctionInfo<Person, decltype(&Person::setId)>{ "setId", &Person::setId };

    std::cout << "arg count: " << fn.argumentCount() << '\n';

    Another obj;
    Another::invoke(obj, "f");

    int value = 0;
    Another::invoke(obj, "g", value, 2);
    std::cout << value << '\n';

    double h_result = 0.0;
    Another::invoke(obj, "h", h_result, 3.0, 4.0);
    std::cout << h_result << "\n";

    Person p;
    Person::reflection().invoke(p, "setName", std::string("johnny"));
    std::string name;
    Person::invoke(p, "name", name);
    Person::reflection().invoke(p, "setId", 17);
    RIFT_ASSERT(p.id() == 17);
    RIFT_ASSERT(name == "johnny");

    value = 0;
    const Derived der;
    Base::invoke(der, "value", value);
    std::cout << value << '\n';
    value = 0;
    BaseReflect::invoke(der, "Base::value", value);
    std::cout << value << '\n';
    Person::invoke(p, "doSomething", RIFT_IGNORE_RETURN(), &der);

    auto lb = [](int x, int y) {
        return x + y;
    };
    std::cout << rift::apply<2>(lb, 1, 2, 3, 4) << '\n';

    std::getchar();

    return 0;
}
