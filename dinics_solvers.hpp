#ifndef FLOWS_COURSEWORK_DINICS_SOLVERS_HPP
#define FLOWS_COURSEWORK_DINICS_SOLVERS_HPP

#include "flows_utils.hpp"
#include "link_cut.hpp"
#include <deque>
#include <limits>
#include <memory>

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


    std::vector<std::size_t> next_edge_iterators_m;
    std::vector<vertex_t> path_m;
    std::vector<std::size_t> distances_m;
    std::deque<vertex_t> vertex_queue_m;

  protected:
    std::vector<std::vector<edge_index_t>> next_edge_indices_m;
    flow_graph graph_m;

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

    [[nodiscard]] virtual bool dfs_steps() {
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

template <typename DataType>
class scaled_dinics_solver final : public basic_dinics_solver<DataType> {
    DataType threshold_m = 1;

    void clear_indices() {
        for (vertex_t vertex = 0; vertex < this->graph_m.size(); ++vertex) {
            for (edge_index_t i = 0; i < this->next_edge_indices_m[vertex].size();) {
                const auto &edge = this->graph_m.get_edge_by_vertex(
                    vertex, this->next_edge_indices_m[vertex][i]);
                auto may_push = this->graph_m.may_push(vertex, edge);

                if (may_push >= threshold_m) {
                    ++i;
                } else {
                    std::swap(this->next_edge_indices_m[vertex][i],
                              this->next_edge_indices_m[vertex].back());
                    this->next_edge_indices_m[vertex].pop_back();
                }
            }
        }
    }

  protected:
    [[nodiscard]] bool dfs_steps() override {
        if (threshold_m == 0) {
            return false;
        }
        this->update_edges_to_next_layers();
        clear_indices();
        bool result = false;
        while (this->dfs_step()) {
            result = true;
        }
        if (!result) {
            threshold_m /= 2;
        }
        return true;
    }

    std::vector<DataType> solve(std::size_t graph_size, vertex_t source, vertex_t target,
                                const std::vector<capacity_edge<DataType>>& edges) override {
        for (const auto &edge: edges) {
            threshold_m = std::max(threshold_m, edge.capacity);
        }
        return dinics_solver<DataType>::solve(graph_size, source, target, edges);
    }

};

template <typename DataType>
class linkcut_dinics_solver final : public dinics_solver<DataType> {
    using link_cut = link_cut::link_cut<DataType>;

    link_cut linkcut_m;
    std::vector<bool> deleted_m;

    void mark_deleted(vertex_t vertex) {
        deleted_m[vertex] = true;
        for (edge_index_t i = 0; i < this->graph_m.degree(vertex); ++i) {
            auto& edge = this->graph_m.get_edge_by_vertex(vertex, i);
            auto adjacent = this->graph_m.adjacent(vertex, edge);
            if (linkcut_m.link_cut_parent(adjacent) == vertex) {
                auto& back_edge = this->current_edge(adjacent);
                auto initial_val = this->graph_m.may_push(adjacent, back_edge);
                auto current_val = linkcut_m.link_cut_get(adjacent);

                this->graph_m.push(adjacent, back_edge, initial_val - current_val);
                linkcut_m.link_cut_cut(adjacent);
            }
        }
    }

  protected:
    bool dfs_steps() override {
        linkcut_m.reinit(this->graph_m.size());
        deleted_m.assign(this->graph_m.size(), false);

        return dinics_solver<DataType>::dfs_steps();
    }

    bool dfs_step() override {
        vertex_t vertex = this->graph_m.source();

        while (vertex != this->graph_m.target()) {
            if (linkcut_m.link_cut_parent(vertex).has_value()) {
                vertex = linkcut_m.link_cut_root(vertex);
            } else if (this->iteration_finished(vertex)) {
                if (vertex == this->graph_m.source()) {
                    for (vertex_t i = 0; i < this->graph_m.size(); ++i) {
                        if (linkcut_m.link_cut_parent(i).has_value()) {
                            auto& edge = this->current_edge(i);
                            this->graph_m.push(i, edge,
                                               this->graph_m.may_push(i, edge) -
                                                   linkcut_m.link_cut_get(i));
                        }
                    }
                    return false;
                } else {
                    mark_deleted(vertex);
                    vertex = this->graph_m.source();
                }
            } else {
                auto& edge = this->current_edge(vertex);
                vertex_t adjacent = this->graph_m.adjacent(vertex, edge);
                DataType may_push = this->graph_m.may_push(vertex, edge);
                if (!may_push || deleted_m[adjacent]) {
                    this->iterate(vertex);
                } else {
                    linkcut_m.link_cut_link(vertex, adjacent);
                    linkcut_m.link_cut_set(vertex, may_push);
                    vertex = adjacent;
                }
            }
        }

        linkcut_m.link_cut_set(this->graph_m.target(), std::numeric_limits<DataType>::max());

        auto min_entry = linkcut_m.link_cut_get_min_on_path(this->graph_m.source());
        auto min_val = min_entry.second;
        auto min_argval = min_entry.first;

        linkcut_m.link_cut_add_on_path(this->graph_m.source(), -min_val);
        min_val = 0;
        while (min_val == 0 && min_argval != this->graph_m.target()) {
            linkcut_m.link_cut_cut(min_argval);
            auto& edge = this->current_edge(min_argval);
            vertex_t adjacent = this->graph_m.adjacent(min_argval, edge);

            this->graph_m.push(min_argval, edge, this->graph_m.may_push(min_argval, edge));
            min_entry = linkcut_m.link_cut_get_min_on_path(adjacent);
            min_val = min_entry.second;
            min_argval = min_entry.first;
        }

        return true;
    }
};

} // namespace dinics_solvers

} // namespace flows_coursework

#endif // FLOWS_COURSEWORK_DINICS_SOLVERS_HPP
