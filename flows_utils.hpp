#ifndef FLOWS_COURSEWORK_FLOWS_UTILS_HPP
#define FLOWS_COURSEWORK_FLOWS_UTILS_HPP

#include "interfaces.hpp"

namespace flows_coursework {

namespace flows_utils {

template <typename DataType>
struct flow_edge : capacity_edge<DataType> {
    DataType flow_value{};

    flow_edge(const capacity_edge<DataType> &base) : capacity_edge<DataType>(base) {
    }

    flow_edge() : capacity_edge<DataType>() {
    }
};

template <typename DataType>
struct flow_graph {

    using edge_t = flow_edge<DataType>;

  private:
    vertex_t source_m;
    vertex_t target_m;
    std::vector<std::vector<edge_index_t>> graph_m;
    std::vector<edge_t> edges_buf_m{};

  public:
    [[nodiscard]] static DataType may_push(vertex_t u, const edge_t &edge) {
        return u == edge.from ? edge.capacity - edge.flow_value : edge.flow_value;
    }

    static void push(vertex_t u, edge_t &edge, DataType amount) {
        edge.flow_value += (u == edge.from ? amount : -amount);
    }

    [[nodiscard]] static vertex_t adjacent(vertex_t u, const edge_t &edge) {
        return u == edge.from ? edge.to : edge.from;
    }

    [[nodiscard]] std::size_t size() const {
        return graph_m.size();
    }

    [[nodiscard]] std::size_t degree(vertex_t u) const {
        return graph_m[u].size();
    }

    edge_t &get_edge_by_vertex(vertex_t u, std::size_t i) {
        return edges_buf_m[graph_m[u][i]];
    }

    const edge_t &get_edge_by_vertex(vertex_t u, std::size_t i) const {
        return edges_buf_m[graph_m[u][i]];
    }

    edge_t &get_edge_by_index(edge_index_t index) {
        return edges_buf_m[index];
    }

    const edge_t &get_edge_by_index(edge_index_t index) const {
        return edges_buf_m[index];
    }

    [[nodiscard]] vertex_t source() const {
        return source_m;
    }

    [[nodiscard]] vertex_t target() const {
        return target_m;
    }

    [[nodiscard]] std::size_t n_edges() const {
        return edges_buf_m.size();
    }

    flow_graph(std::size_t n_vertices, vertex_t source, vertex_t target,
               const std::vector<capacity_edge<DataType>> &edges)
        : source_m(source), target_m(target), graph_m(n_vertices), edges_buf_m(edges.size()) {

        for (std::size_t i = 0; i < n_edges(); ++i) {
            edges_buf_m[i] = edge_t(edges[i]);
            graph_m[edges[i].from].push_back(i);
            graph_m[edges[i].to].push_back(i);
        }
    }

    flow_graph() : source_m(0), target_m(0), graph_m(1), edges_buf_m(0) {
    }
};

template <typename DataType>
std::vector<DataType> flow_vector(const flow_graph<DataType> &graph) {
    std::vector<DataType> res(graph.n_edges());

    for (edge_index_t i = 0; i < graph.n_edges(); ++i) {
        res[i] = graph.get_edge_by_index(i).flow_value;
    }
    return res;
}

template <typename DataType>
DataType flow_size(vertex_t source, const std::vector<capacity_edge<DataType>> &edges,
                   const std::vector<DataType> &result) {

    DataType res{};
    for (edge_index_t edge_i = 0; edge_i < edges.size(); ++edge_i) {
        if (edges[edge_i].from == source) {
            res += result[edge_i];
        } else if (edges[edge_i].to == source) {
            res -= result[edge_i];
        }
    }
    return res;
}

} // namespace flows_utils
} // namespace flows_coursework

#endif // FLOWS_COURSEWORK_FLOWS_UTILS_HPP
