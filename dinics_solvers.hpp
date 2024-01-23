#include "flows_utils.hpp"
#include <deque>
#include <memory>

#ifndef FLOWS_COURSEWORK_DINICS_SOLVERS_HPP
#define FLOWS_COURSEWORK_DINICS_SOLVERS_HPP

namespace flows_coursework {

namespace edmonds_solvers {

template <typename DataType>
class edmonds_solver final : public flows_solver<DataType> {
    flows_utils::flow_graph<DataType> graph_m;
    std::vector<edge_index_t> reached_by_m;
    std::deque<vertex_t> vertex_queue_m;

    bool bfs_step() {
        reached_by_m.resize(graph_m.size());
        for (vertex_t i = 0; i < graph_m.size(); ++i) {
            reached_by_m[i] = graph_m.degree(i);
        }

        vertex_queue_m.clear();

        vertex_queue_m.push_back(graph_m.source());

        while (!vertex_queue_m.empty()) {
            vertex_t current = vertex_queue_m.front();
            vertex_queue_m.pop_front();

            if (current == graph_m.target()) {
                auto step_calculate = [&]() -> DataType {
                    auto& edge = graph_m.get_edge_by_vertex(current, reached_by_m[current]);
                    current = graph_m.adjacent(current, edge);
                    return graph_m.may_push(current, edge);
                };

                DataType max_to_push = step_calculate();
                while (current != graph_m.source()) {
                    max_to_push = std::min(max_to_push, step_calculate());
                }

                current = graph_m.target();

                while (current != graph_m.source()) {
                    auto& edge = graph_m.get_edge_by_vertex(current, reached_by_m[current]);
                    current = graph_m.adjacent(current, edge);
                    graph_m.push(current, edge, max_to_push);
                }
                return true;
            }

            for (edge_index_t i = 0; i < graph_m.degree(current); ++i) {
                const auto& edge = graph_m.get_edge_by_vertex(current, i);
                if (!graph_m.may_push(current, edge)) {
                    continue;
                }
                vertex_t adjacent = graph_m.adjacent(current, edge);

                if (adjacent == graph_m.source() ||
                    reached_by_m[adjacent] != graph_m.degree(adjacent)) {

                    continue;
                }

                vertex_queue_m.push_back(adjacent);

                for (edge_index_t j = 0; j < graph_m.degree(adjacent); ++j) {
                    const auto& back_edge = graph_m.get_edge_by_vertex(adjacent, j);
                    if (graph_m.adjacent(adjacent, back_edge) == current &&
                        graph_m.may_push(current, back_edge)) {

                        reached_by_m[adjacent] = j;
                        break;
                    }
                }
            }
        }
        return false;
    }

  public:
    std::vector<DataType> solve(std::size_t graph_size, vertex_t source, vertex_t target,
                                const std::vector<capacity_edge<DataType>>& edges) override {

        graph_m = flows_utils::flow_graph(graph_size, source, target, edges);

        while (bfs_step()) {
        }

        return flows_utils::flow_vector(graph_m);
    }
};
} // namespace edmonds_solvers

namespace dinics_solvers {

template <typename DataType>
class dinics_solver : public flows_solver<DataType> {
    using flow_graph = flows_utils::flow_graph<DataType>;

    std::vector<std::vector<edge_index_t>> next_edge_indices_m;
    std::vector<std::size_t> next_edge_iterators_m;
    std::vector<vertex_t> path_m;
    std::vector<std::size_t> distances_m;
    std::deque<vertex_t> vertex_queue_m;

  protected:
    flow_graph graph_m;

  private:
    void update_edges_to_next_layers() {
        const std::size_t size = graph_m.size();
        bool target_encountered = false;

        distances_m.assign(size, size);

        distances_m[graph_m.source()] = 0;
        vertex_queue_m.clear();
        vertex_queue_m.push_back(graph_m.source());

        next_edge_indices_m.assign(size, std::vector<edge_index_t>{});
        next_edge_iterators_m.assign(size, 0);

        while (!vertex_queue_m.empty()) {
            vertex_t current = vertex_queue_m.front();
            vertex_queue_m.pop_front();

            for (std::size_t i = 0; i < graph_m.degree(current); ++i) {
                const auto& edge = graph_m.get_edge_by_vertex(current, i);
                vertex_t other = graph_m.adjacent(current, edge);
                if (!graph_m.may_push(current, edge)) {
                    continue;
                }
                if (distances_m[other] == size) {
                    distances_m[other] = distances_m[current] + 1;
                    vertex_queue_m.push_back(other);
                    target_encountered |= graph_m.target() == other;
                    if (!target_encountered || graph_m.target() == other) {
                        next_edge_indices_m[current].push_back(i);
                    }
                } else if (distances_m[other] == distances_m[current] + 1 &&
                           (!target_encountered || graph_m.target() == other)) {
                    next_edge_indices_m[current].push_back(i);
                }
            }
        }
    }

  protected:
    [[nodiscard]] flows_utils::flow_edge<DataType>& current_edge(vertex_t node) {
        return graph_m.get_edge_by_vertex(node,
                                          next_edge_indices_m[node][next_edge_iterators_m[node]]);
    }

    [[nodiscard]] bool iteration_finished(vertex_t node) const {
        return next_edge_iterators_m[node] == next_edge_indices_m[node].size();
    }

    void iterate(vertex_t node) {
        ++next_edge_iterators_m[node];
    }

    virtual bool dfs_step() = 0;

    [[nodiscard]] bool dfs_steps() {
        update_edges_to_next_layers();
        bool res = false;
        while (dfs_step()) {
            res = true;
        }
        return res;
    }

  public:
    std::vector<DataType> solve(std::size_t graph_size, vertex_t source, vertex_t target,
                                const std::vector<capacity_edge<DataType>>& edges) override {

        graph_m = flow_graph(graph_size, source, target, edges);

        while (dfs_steps()) {
        }

        return flows_utils::flow_vector(graph_m);
    }
};

template <typename DataType>
class basic_dinics_solver : public dinics_solver<DataType> {

    std::vector<vertex_t> path_m;

  protected:
    bool dfs_step() override {
        path_m.clear();
        path_m.push_back(this->graph_m.source());
        while (path_m.back() != this->graph_m.target()) {
            if (vertex_t current = path_m.back(); this->iteration_finished(current)) {
                if (current == this->graph_m.source()) {
                    return false;
                }
                path_m.pop_back();
                this->iterate(path_m.back());
            } else if (const auto& edge = this->current_edge(current);
                       !this->graph_m.may_push(current, edge)) {

                this->iterate(current);
            } else {
                path_m.push_back(this->graph_m.adjacent(current, edge));
            }
        }
        DataType max_pushable = this->graph_m.may_push(this->graph_m.source(),
                                                       this->current_edge(this->graph_m.source()));

        for (std::size_t i = 1; i + 1 < path_m.size(); ++i) {
            vertex_t current = path_m[i];
            max_pushable = std::min(this->graph_m.may_push(current, this->current_edge(current)),
                                    max_pushable);
        }

        for (std::size_t i = 0; i + 1 < path_m.size(); ++i) {
            vertex_t current = path_m[i];
            this->graph_m.push(current, this->current_edge(current), max_pushable);
        }

        return true;
    }
};

} // namespace dinics_solvers

} // namespace flows_coursework

#endif // FLOWS_COURSEWORK_DINICS_SOLVERS_HPP
