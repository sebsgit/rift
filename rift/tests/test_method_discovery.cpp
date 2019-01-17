#include "test_runner.hpp"
#include "rift/lightweight/member_check.hpp"
#include "rift/lightweight/method_check.hpp"
#include "rift/lightweight/static_method_check.hpp"

#include <string>
#include <vector>

RIFT_DECLARE_MEMBER_TEST(value);
RIFT_DECLARE_MEMBER_TEST(str);
RIFT_DECLARE_MEMBER_TEST(x);

RIFT_DECLARE_METHOD_TEST(f);
RIFT_DECLARE_METHOD_TEST(begin);
RIFT_DECLARE_METHOD_TEST(end);
RIFT_DECLARE_STATIC_METHOD_TEST(f);
RIFT_DECLARE_STATIC_METHOD_TEST(build);

namespace {
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
}

using namespace rift;

static void test_data_members() {
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
}

RIFT_TEST(test_data_members);
