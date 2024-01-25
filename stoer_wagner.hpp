
#ifndef FLOWS_COURSEWORK_STOER_WAGNER_HPP
#define FLOWS_COURSEWORK_STOER_WAGNER_HPP

#include <vector>
#include <numeric>
#include <limits>
#include <queue>
#include "interfaces.hpp"

namespace flows_coursework {

namespace undirected_cuts {

template <typename DataType>
class stoer_wagner_mincut_solver {
    std::vector<capacity_edge<DataType>> edges_m;
    std::vector<std::vector<edge_index_t>> graph_m;
    std::vector<std::vector<vertex_t>> descendants_m;
    std::vector<bool> deleted_m;
    std::vector<bool> result_m;

    void merge_two(vertex_t a, vertex_t b) {
        // merges b into a

        descendants_m[a].push_back(b);
        deleted_m[b] = true;
        for (auto edge_i : graph_m[b]) {
            if (edges_m[edge_i].from == b) {
                edges_m[edge_i].from = a;
            } else if (edges_m[edge_i].to == b) {
                edges_m[edge_i].to = a;
            }
        }

        while (!graph_m[b].empty()) {
            graph_m[a].push_back(graph_m[b].back());
            graph_m[b].pop_back();
        }
    }

    void mark_descendants(vertex_t u) {
        result_m[u] = true;
        for (auto v: descendants_m[u]) {
            mark_descendants(v);
        }
    }

  public:
    [[nodiscard]] std::vector<bool> find_mincut(std::size_t n_vertices,
                                  const std::vector<capacity_edge<DataType>> &edges) {
        edges_m = edges;
        graph_m.resize(n_vertices);
        descendants_m.resize(n_vertices);
        deleted_m.resize(n_vertices);

        for (edge_index_t i = 0; i < edges.size(); ++i) {
            graph_m[edges[i].from].push_back(i);
            graph_m[edges[i].to].push_back(i);
        }

        std::size_t best_cut_vertex = n_vertices;
        DataType best_cut_ans = std::numeric_limits<DataType>::max();
        std::vector<DataType> weights(n_vertices);
        std::vector<bool> chosen(n_vertices);

        for (std::size_t phase = 0; phase + 1 < n_vertices; ++phase) {
            std::fill(weights.begin(), weights.end(), 0);
            std::fill(chosen.begin(), chosen.end(), false);
            std::priority_queue<std::pair<DataType, vertex_t>> queue_weights;
            for (vertex_t v = 0; v < n_vertices; ++v) {
                if (!deleted_m[v]) {
                    queue_weights.emplace(0, v);
                }
            }

            std::size_t last_chosen;

            for (std::size_t op = n_vertices - phase; op--; ) {
                while (chosen[queue_weights.top().second]) {
                    queue_weights.pop();
                }
                vertex_t current = queue_weights.top().second;
                DataType current_mincut = queue_weights.top().first;
                chosen[current] = true;

                for (edge_index_t edge_index: graph_m[current]) {
                    auto other = edges_m[edge_index].to + edges_m[edge_index].from - current;
                    if (!chosen[other]) {
                        weights[other] += edges_m[edge_index].capacity;
                        queue_weights.emplace(weights[other], other);
                    }
                }

                if (op == 0) {
                    if (current_mincut <= best_cut_ans) {
                        best_cut_ans = current_mincut;
                        best_cut_vertex = current;
                    }
                    merge_two(last_chosen, current);
                }

                last_chosen = current;
            }
        }

        result_m.resize(n_vertices);
        mark_descendants(best_cut_vertex);
        return result_m;
    }
};

template<typename DataType>
DataType cut_size(const std::vector<capacity_edge<DataType>> &edges,
                  const std::vector<bool> &cut) {

    DataType res{};
    for (const auto &edge: edges) {
        if (cut[edge.to] ^ cut[edge.from]) {
            res += edge.capacity;
        }
    }
    return res;
}
}


}


#endif // FLOWS_COURSEWORK_STOER_WAGNER_HPP
