#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "dinics_solvers.hpp"

using solver_t = flows_coursework::flows_solver<int64_t>;

int main([[maybe_unused]] int argc, char *argv[]) {
    std::string mode = argv[1];

    std::mt19937 generator{42};
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

    } else if (mode == "time") {
        std::string algo = argv[2];

        std::unique_ptr<solver_t> solver;

        if (algo == "edmonds") {
            solver = std::unique_ptr<solver_t>(
                new flows_coursework::edmonds_solvers::edmonds_solver<int64_t>);
        } else if (algo == "dinics") {
            solver = std::unique_ptr<solver_t>(
                new flows_coursework::dinics_solvers::basic_dinics_solver<int64_t>);
        } else if (algo == "linkcut") {
            solver = std::unique_ptr<solver_t>(
                new flows_coursework::dinics_solvers::linkcut_dinics_solver<int64_t>);
        }

        std::string method = argv[3];

        std::vector<flows_coursework::capacity_edge<int64_t>> data;

        int n;
        int s;
        int t;
        if (method == "random") {
            n = std::atoi(argv[4]);
            int m = std::atoi(argv[5]);

            s = 0;
            t = 1;
            for (int j = 0; j < m; ++j) {
                int u = std::uniform_int_distribution<std::size_t>(0, n - 1)(generator);
                int v = std::uniform_int_distribution<std::size_t>(0, n - 2)(generator);
                if (v >= u) {
                    ++v;
                }
                int c = std::uniform_int_distribution<int64_t>(1, 1'000'000)(generator);
                data.emplace_back(u, v, c);
            }
        } else if (method == "random-full") {
            n = std::atoi(argv[4]);
            s = 0;
            t = 1;
            for (int u = 0; u < n; ++u) {
                for (int v = 0; v < n; ++v) {
                    if (v == u) {
                        continue;
                    }
                    int c = std::uniform_int_distribution<int64_t>(1, 1'000'000)(generator);
                    data.emplace_back(u, v, c);
                }
            }
        } else if (method == "akc-hard") {
            n = std::atoi(argv[4]);
            data = flows_coursework::flows_utils::akc_test(n);
            s = 0;
            t = 1;
            n = 4 * n + 6;
        }
        auto start_exec = std::chrono::steady_clock::now();
        solver->solve(n, s, t, data);
        auto finish_exec = std::chrono::steady_clock::now();

        auto elapsed_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(finish_exec - start_exec);
        std::cout << elapsed_ms.count() << " milliseconds" << std::endl;
    }
}
