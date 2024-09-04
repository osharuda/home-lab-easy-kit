#include "testtool.hpp"
#include "misc_tests.hpp"
#include "tools.hpp"

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

void test_reverse_bits() {
    DECLARE_TEST(test_reverse_bits)

    std::vector<std::pair<uint8_t, uint8_t>> test = {{0, 0}, {1, 128}, {2, 64}, {4, 32}, {8, 16}, {16, 8}, {32, 4}, {64, 2}, {128, 1}};

    REPORT_CASE
    size_t n = test.size();
    for (size_t i=0; i<n; i++) {
        auto p = test.at(i);
        uint8_t input = p.first;
        uint8_t expected = p.second;
        uint8_t result = tools::reverse_bits(input);

        assert(expected == result);
    }


}