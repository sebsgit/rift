#include "rift/lightweight/utils.hpp"
#include "test_runner.hpp"

namespace {
class ReportCopy {
public:
    ReportCopy() = default;
    ReportCopy(ReportCopy&&) = default;
    ReportCopy& operator=(ReportCopy&&) = default;
    ReportCopy(const ReportCopy&) 
        :wasCopied(true)
    {}
    ReportCopy& operator=(const ReportCopy&) {
        this->wasCopied = true;
        return *this;
    }
    bool wasCopied = false;
};
}

static void test_apply() {
    auto lb = [](int x, int y) {
        return x + y;
    };
    RIFT_ASSERT(rift::apply<2>(lb, 1, 2, 3, 4) == 3);
    auto withRVal = [](ReportCopy x) {
        return x.wasCopied;
    };
    ReportCopy toCopy;
    RIFT_ASSERT(rift::apply<1>(withRVal, toCopy) == true);
    RIFT_ASSERT(rift::apply<1>(withRVal, std::move(toCopy)) == false);
}

RIFT_TEST(test_apply);
