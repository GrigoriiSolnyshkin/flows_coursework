#include <doctest.h>
#include <random>

#include "../dinics_solvers.hpp"

using flows_coursework::capacity_edge;
using flows_coursework::flows_solver;
using flows_coursework::dinics_solvers::basic_dinics_solver;
using flows_coursework::dinics_solvers::linkcut_dinics_solver;
using flows_coursework::edmonds_solvers::edmonds_solver;
using flows_coursework::flows_utils::flow_size;

static std::mt19937 generator{42}; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

template <typename T>
std::vector<std::unique_ptr<flows_solver<T>>> all_solvers() {
    std::vector<std::unique_ptr<flows_solver<T>>> result;
    result.emplace_back(new basic_dinics_solver<T>);
    result.emplace_back(new edmonds_solver<T>);
    result.emplace_back(new linkcut_dinics_solver<T>);
    return result;
}

TEST_CASE("simple path") {
    std::vector<capacity_edge<int64_t>> data;

    for (int i = 0; i < 999; ++i) {
        data.emplace_back(i, i + 1, 1000);
    }

    data[500].capacity = 999;

    for (auto &solver : all_solvers<int64_t>()) {
        auto res = solver->solve(1000, 0, 999, data);

        CHECK_EQ(res[0], 999);
        CHECK_EQ(flow_size(0, data, res), 999);
    }
}

TEST_CASE("parallel edges") {
    std::vector<capacity_edge<int64_t>> data;

    for (int i = 0; i < 1000; ++i) {
        data.emplace_back(0, 1, i + 1);
    }

    for (auto &solver : all_solvers<int64_t>()) {
        auto res = solver->solve(2, 0, 1, data);
        for (int i = 0; i < 1000; ++i) {
            CHECK_EQ(res[i], i + 1);
        }
        CHECK_EQ(flow_size(0, data, res), 1000 * 1001 / 2);
    }
}

TEST_CASE("matching") {
    std::vector<capacity_edge<int64_t>> data;
    std::size_t n = 1'000;
    std::size_t s = 2 * n;
    std::size_t t = 2 * n + 1;

    data.reserve(4 * n);

    for (std::size_t i = 0; i < n; ++i) {
        data.emplace_back(s, i, 1);
        data.emplace_back(i + n, t, 1);
    }

    for (std::size_t i = 0; i < n / 3; ++i) {
        data.emplace_back(i, i + n + n / 3, 1);
        data.emplace_back(i + n / 3, i + n, 1);
        data.emplace_back(i, i + n, 1);
    }

    for (auto &solver : all_solvers<int64_t>()) {
        auto res = solver->solve(2 * n + 2, s, t, data);

        CHECK_EQ(flow_size(s, data, res), 2 * n / 3);
    }
}

TEST_CASE("bridge test") {
    std::size_t n = 100;
    std::vector<capacity_edge<int64_t>> data;
    for (std::size_t i = 0; i < n / 2; ++i) {
        for (std::size_t j = 0; j < n / 2; ++j) {
            if (i == j) {
                continue;
            }

            data.emplace_back(i, j, 1000);
        }
    }

    for (std::size_t i = n / 2; i < n; ++i) {
        for (std::size_t j = n / 2; j < n; ++j) {
            if (i == j) {
                continue;
            }

            data.emplace_back(i, j, 1000);
        }
    }
    data.emplace_back(n / 2 - 1, n / 2, 10000);

    for (auto &solver : all_solvers<int64_t>()) {
        auto res = solver->solve(n, 0, n - 1, data);
        CHECK_EQ(flow_size(0, data, res), 10000);
    }
    data.pop_back();
    data.emplace_back(n / 2 - 1, n / 2, 10000000);
    for (auto &solver : all_solvers<int64_t>()) {
        auto res = solver->solve(n, 0, n - 1, data);

        CHECK_EQ(flow_size(0, data, res), 1000 * (n / 2 - 1));
    }
}

template <typename T>
void check_solvers_coincide(std::size_t n, std::size_t s, std::size_t t,
                            const std::vector<capacity_edge<T>> &data,
                            const std::vector<std::unique_ptr<flows_solver<T>>> &solvers) {

    std::vector<T> results;
    for (auto &solver : solvers) {
        results.push_back(flow_size(s, data, solver->solve(n, s, t, data)));
    }

    for (std::size_t i = 1; i < results.size(); ++i) {
        CHECK_EQ(results[i - 1], results[i]);
    }
}

void test_random_edges(int n, int m) {
    std::vector<capacity_edge<int64_t>> data;
    std::vector<int64_t> results;

    for (int j = 0; j < m; ++j) {
        std::size_t u = std::uniform_int_distribution<std::size_t>(0, n - 1)(generator);
        std::size_t v = std::uniform_int_distribution<std::size_t>(0, n - 2)(generator);
        if (v >= u) {
            ++v;
        }
        data.emplace_back(u, v,
                          std::uniform_int_distribution<int64_t>(1, 1'000'000'000)(generator));
    }

    check_solvers_coincide(n, 0, n - 1, data, all_solvers<int64_t>());
}

void test_all_edges(int n) {
    std::vector<capacity_edge<int64_t>> data;

    for (int u = 0; u < n; ++u) {
        for (int v = 0; v < u; ++v) {
            data.emplace_back(u, v,
                              std::uniform_int_distribution<int64_t>(1, 1'000'000'000)(generator));
        }
    }

    check_solvers_coincide(n, 0, n - 1, data, all_solvers<int64_t>());
}

TEST_CASE("all algorithms coincide on random edges") {
    int n = 10;
    int m = 500;
    int iterations = 200;

    while (iterations--) {
        test_random_edges(n, m);
    }
}

TEST_CASE("all algorithms coincide on random edges small") {
    int n = 10;
    int m = 20;
    int iterations = 20000;
    while (iterations--) {
        test_random_edges(n, m);
    }
}

TEST_CASE("all algorithms coincide on all edges") {
    int n = 200;
    int iterations = 200;
    while (iterations--) {
        test_all_edges(n);
    }
}
