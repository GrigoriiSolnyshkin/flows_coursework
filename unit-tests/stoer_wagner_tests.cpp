#include <cstdint>
#include <doctest.h>
#include <random>
#include <vector>

#include "../stoer_wagner.hpp"

using flows_coursework::capacity_edge;
using flows_coursework::flows_utils::flow_size;
using flows_coursework::undirected_cuts::cut_size;
using flows_coursework::undirected_cuts::global_mincut_size_dinics;
using flows_coursework::undirected_cuts::global_mincut_size_wagner;
using flows_coursework::undirected_cuts::stoer_wagner_mincut_solver;

static std::mt19937 generator{21}; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

TEST_CASE("cycle") {
    int n = 100;

    std::vector<capacity_edge<int64_t>> data;
    for (int i = 0; i < n; ++i) {
        data.emplace_back(i, (i + 1) % n, 1);
    }

    CHECK_EQ(2, global_mincut_size_wagner(n, data));
}

TEST_CASE("cycle-2") {
    std::vector<capacity_edge<int>> data{
        {0, 1, 1},
        {1, 2, 2},
        {2, 3, 1},
        {3, 0, 2}
    };

    CHECK_EQ(2, global_mincut_size_wagner(4, data));
}

TEST_CASE("two clicks") {
    int n = 20;

    std::vector<capacity_edge<int64_t>> data;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            data.emplace_back(i, j, 1);
            data.emplace_back(n + i, n + j, 1);
        }
    }

    for (int i = 0; i < n / 2; ++i) {
        data.emplace_back(0, n, i);
        auto res = stoer_wagner_mincut_solver<int64_t>().find_mincut(2 * n, data);
        for (int j = 1; j < n; ++j) {
            CHECK_EQ(res[j], res[j - 1]);
            CHECK_EQ(res[n + j], res[n + j - 1]);
        }
        CHECK_NE(res[n], res[0]);
        data.pop_back();
    }
}

TEST_CASE("stress with dinics") {
    int n = 10;
    int iterations = 1000;

    while (iterations--) {
        std::vector<capacity_edge<int64_t>> data;
        for (int j = 0; j < n; ++j) {
            for (int i = 0; i < j; ++i) {
                data.emplace_back(i, j, std::uniform_int_distribution<int64_t>(0, 99)(generator));
            }
        }

        CHECK_EQ(global_mincut_size_wagner(n, data), global_mincut_size_dinics(n, data));
    }
}
