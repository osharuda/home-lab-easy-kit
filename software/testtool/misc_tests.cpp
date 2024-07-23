#include "testtool.hpp"
#include "misc_tests.hpp"

void test_append_vector() {
    DECLARE_TEST(test_append_vector)

    std::vector<int> ires;

    REPORT_CASE
    std::vector<int> v1 = {0,1,2,3};
    std::vector<int> v2 = {4,5,6,7};
    ires = tools::append_vector(v1,v2);
    assert(ires==std::vector<int>({0,1,2,3,4,5,6,7}));

    REPORT_CASE
            v1 = {0};
    v2 = {1};
    ires = tools::append_vector(v1,v2);
    assert(ires==std::vector<int>({0,1}));

    REPORT_CASE
            v1 = {};
    v2 = {1};
    ires = tools::append_vector(v1,v2);
    assert(ires==std::vector<int>({1}));

    REPORT_CASE
            v1 = {0};
    v2 = {};
    ires = tools::append_vector(v1,v2);
    assert(ires==std::vector<int>({0}));

    REPORT_CASE
            v1 = {0,1};
    v2 = {2};
    ires = tools::append_vector(v1,v2);
    assert(ires==std::vector<int>({0,1,2}));

    REPORT_CASE
            v1 = {0};
    v2 = {1,2};
    ires = tools::append_vector(v1,v2);
    assert(ires==std::vector<int>({0,1,2}));

    REPORT_CASE
            v1 = {};
    v2 = {0,1,2};
    ires = tools::append_vector(v1,v2);
    assert(ires==std::vector<int>({0,1,2}));

    REPORT_CASE
            v1 = {0,1,2};
    v2 = {};
    ires = tools::append_vector(v1,v2);
    assert(ires==std::vector<int>({0,1,2}));
}