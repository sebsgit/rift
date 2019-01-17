#include "test_runner.hpp"

#include "rift/lightweight/reflection.hpp"

#include <sstream>

namespace {
    struct Another {
        int x;
        double y;
        std::string str;

        void f(std::ostream* out) { *out << "hello"; }
        int g(int x) { return x * 2; }
        double h(double a, double b) { return a + b; }

        RIFT_REFLECT(x, y, str);

        RIFT_DECLARE_INVOKABLE(Another,
            RIFT_INVOKABLE(f)
            RIFT_INVOKABLE(g)
            RIFT_INVOKABLE(h)
        );
    };

    class Base {
    public:
        virtual ~Base() = default;
        virtual int value() const = 0;

        RIFT_DECLARE_INVOKABLE(Base,
            RIFT_INVOKABLE(value)
        );
    };

    // adding reflection to a class without modifying it
    class BaseReflect {
    public:
        RIFT_DECLARE_INVOKABLE(Base,
            RIFT_INVOKABLE(Base::value)
        );
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

        RIFT_DECLARE_INVOKABLE(Person,
            RIFT_INVOKABLE(setId)
            RIFT_INVOKABLE(setName)
            RIFT_INVOKABLE(name)
            RIFT_INVOKABLE(doSomething)
        );

    private:
        int _id;
        std::string _name;
    };
}

static void test_basic_reflection() {
    using namespace rift;

    static_assert(rift::FunctionTraits<decltype(&Another::g)>::argumentCount == 1, "");
    static_assert(std::is_same<typename rift::FunctionTraits<decltype(&Another::g)>::ResultType, int>::value, "");

    auto fn = rift::FunctionInfo<Person, decltype(&Person::setId)>{ "setId", &Person::setId };
    RIFT_ASSERT(fn.argumentCount() == 1);

    std::stringstream ss;
    Another obj;
    Another::reflection().invoke(obj, "f", &ss);
    RIFT_ASSERT(ss.str() == "hello");

    int value = 0;
    Another::reflection().invoke(obj, "g", rift::ReturnArgument(value), 2);
    RIFT_ASSERT(value == 4);

    double h_result = 0.0;
    Another::reflection().invoke(obj, "h", rift::ReturnArgument(h_result), 3.0, 4.0);
    RIFT_ASSERT(h_result == 7.0);

    Person p;
    Person::reflection().invoke(p, "setName", std::string("johnny"));
    std::string name;
    Person::reflection().invoke(p, "name", rift::ReturnArgument(name));

    Person::reflection().invoke(p, "setId", 17);
    RIFT_ASSERT(p.id() == 17);
    RIFT_ASSERT(name == "johnny");

    value = 0;
    const Derived der;
    Base::reflection().invoke(const_cast<Derived&>(der), "value", rift::ReturnArgument(value));
    RIFT_ASSERT(value == 10);
    value = 0;
    //TODO: support const objects
    BaseReflect::reflection().invoke(const_cast<Derived&>(der), "Base::value", rift::ReturnArgument(value));
    RIFT_ASSERT(value == 10);
    Person::reflection().invoke(p, "doSomething", RIFT_IGNORE_RETURN(), &der);
}

static void another_approach()
{

}

RIFT_TEST(test_basic_reflection);
RIFT_TEST(another_approach);
