#include <chrono>
#include <functional>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <vector>

#include "dinics_solvers.hpp"

using solver_t = flows_coursework::flows_solver<int64_t>;
using edges_set_t = std::vector<flows_coursework::capacity_edge<int64_t>>;

template <typename SolverType>
std::unique_ptr<solver_t> create_solver() {
    return std::unique_ptr<solver_t>(new SolverType);
}

const std::map<std::string, std::function<std::unique_ptr<solver_t>()>> STRING_TO_SOLVER{
    {"edmonds", create_solver<flows_coursework::edmonds_solvers::edmonds_solver<int64_t>>},
    {"dinics", create_solver<flows_coursework::dinics_solvers::basic_dinics_solver<int64_t>>},
    {"linkcut", create_solver<flows_coursework::dinics_solvers::linkcut_dinics_solver<int64_t>>},
    {"scaled-dinics",
     create_solver<flows_coursework::dinics_solvers::scaled_dinics_solver<int64_t>>}};

static std::mt19937 generator{42}; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables

const std::map<std::string, std::function<void(edges_set_t &, int &, int &, int &)>>
    STRING_TO_GENERATION{{"random-full",
                          [](edges_set_t &data, int &n, int &s, int &t) -> void {
                              data.clear();
                              s = 0;
                              t = 1;
                              for (int u = 0; u < n; ++u) {
                                  for (int v = 0; v < n; ++v) {
                                      if (v == u) {
                                          continue;
                                      }
                                      int c = std::uniform_int_distribution<int64_t>(1, 1'000'000)(
                                          generator);
                                      data.emplace_back(u, v, c);
                                  }
                              }
                          }},
                         {"akc-hard",
                          [](edges_set_t &data, int &n, int &s, int &t) -> void {
                              data = flows_coursework::flows_utils::akc_test(n);
                              s = 0;
                              t = 1;
                              n = 4 * n + 6;
                          }

                         }};

int main([[maybe_unused]] int argc, char *argv[]) {
    std::string mode = argv[1];

    if (mode == "interactive") {
        std::cout << "Enter number of vertices:" << std::endl;
        int n;
        std::cin >> n;

        std::cout << "Enter number of edges:" << std::endl;
        int m;
        std::cin >> m;

        int s;
        std::cout << "Enter source:" << std::endl;
        std::cin >> s;

        int t;
        std::cout << "Enter target:" << std::endl;
        std::cin >> t;

        std::cout << "Enter edges:" << std::endl;
        std::vector<flows_coursework::capacity_edge<int64_t>> data;

        while (m--) {
            int u, v;
            int64_t c;

            std::cin >> u >> v >> c;
            data.emplace_back(u, v, c);
        }

        auto res =
            flows_coursework::dinics_solvers::linkcut_dinics_solver<int64_t>().solve(n, s, t, data);

        std::cout << "Maximum flow size:" << std::endl;
        std::cout << flows_coursework::flows_utils::flow_size(s, data, res) << std::endl;

        std::cout << "Capacities of edges:" << std::endl;
        for (auto val : res) {
            std::cout << val << std::endl;
        }

    } else if (mode == "timeit") {

        std::unique_ptr<solver_t> solver = STRING_TO_SOLVER.find(argv[2])->second();

        std::vector<flows_coursework::capacity_edge<int64_t>> data;

        int n = std::atoi(argv[4]);
        int s;
        int t;

        STRING_TO_GENERATION.find(argv[3])->second(data, n, s, t);
        auto start_exec = std::chrono::steady_clock::now();
        solver->solve(n, s, t, data);
        auto finish_exec = std::chrono::steady_clock::now();

        auto elapsed_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(finish_exec - start_exec);
        std::cout << elapsed_ms.count() << " milliseconds" << std::endl;
    }
}
