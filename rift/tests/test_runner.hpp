#pragma once

#include <vector>
#include <functional>
#include <string>
#include <iostream>

namespace rift {
class test_runner {
public:
    static void register_test(const std::string& name, const std::function<void(void)>& f) {
        instance()._tests.push_back(test_case{name, f});
    }
    static void run() {
        for (const auto& tc : instance()._tests) {
            std::cout << "--> Running test: " << tc.name << "...\n";
            tc.fn();
        }
    }

    class register_helper {
    public:
        template <typename ... Args>
        register_helper(Args&& ... args) {
            test_runner::register_test(std::forward<Args>(args)...);
        }
    };

private:
    test_runner() = default;
    static test_runner& instance() {
        static test_runner runner;
        return runner;
    }

private:
    struct test_case {
        const std::string name;
        const std::function<void(void)> fn;
    };

    std::vector<test_case> _tests;
};
}

#define RIFT_TEST(f) static rift::test_runner::register_helper registering_test_ ## f (#f , f);
#define RIFT_ASSERT(what) if (!(what)) { throw std::runtime_error(__FUNCTION__ " -> " #what ": assertion failed!"); }
