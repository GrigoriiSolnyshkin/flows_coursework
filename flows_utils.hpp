#ifndef FLOWS_COURSEWORK_FLOWS_UTILS_HPP
#define FLOWS_COURSEWORK_FLOWS_UTILS_HPP

#include "interfaces.hpp"

namespace flows_coursework {

namespace flows_utils {

using edge_index_t = std::size_t;

template <typename DataType>
struct flow_edge : capacity_edge<DataType> {
    DataType flow_value{};

    flow_edge(const capacity_edge<DataType> &base) :
          capacity_edge<DataType>(base) {
    }

    flow_edge(): capacity_edge<DataType>() {
    }
};

template <typename DataType>
struct flow_graph {

    using edge_t = flow_edge<DataType>;
    using edge_p = flow_edge<DataType> *;
  private:
    vertex_t source_m;
    vertex_t target_m;
    std::vector<std::vector<edge_p>> graph_m;
    std::size_t n_edges_m{};
    edge_p edges_buf_m{};
  public:

    [[nodiscard]] static DataType may_push(vertex_t u, const edge_t &edge) {
        return u == edge.from ? edge.capacity - edge.flow_value : edge.flow_value;
    }

    static void push(vertex_t u, edge_t &edge, DataType amount) {
        edge.flow_value += (u == edge.from ? amount : -amount);
    }

    static vertex_t adjacent(vertex_t u, const edge_t &edge) {
        return u == edge.from ? edge.to : edge.from;
    }

    [[nodiscard]] std::size_t size() const {
        return graph_m.size();
    }

    [[nodiscard]] std::size_t degree(vertex_t u) const {
        return graph_m[u].size();
    }

    edge_t &get_edge_by_vertex(vertex_t u, std::size_t i) {
        return *(graph_m[u][i]);
    }

    const edge_t &get_edge_by_vertex(vertex_t u, std::size_t i) const {
        return *graph_m[u][i];
    }

    edge_t &get_edge_by_index(std::size_t index) {
        return *edges_buf_m[index];
    }

    const edge_t &get_edge_by_index(std::size_t index) const {
        return *edges_buf_m[index];
    }

    [[nodiscard]] vertex_t source() const {
        return source_m;
    }

    [[nodiscard]] vertex_t target() const {
        return target_m;
    }

    flow_graph(std::size_t n_vertices,
               vertex_t source,
               vertex_t target,
               const std::vector<capacity_edge<DataType>> &edges) :
          source_m(source),
          target_m(target),
          graph_m(n_vertices),
          n_edges_m(edges.size()),
          edges_buf_m(new edge_t[edges.size()]) {

        for (std::size_t i = 0; i < n_edges_m; ++i) {
            auto graph_edge = edges_buf_m + i;
            *graph_edge = edge_t(edges[i]);
            graph_m[graph_edge->from].push_back(graph_edge);
            graph_m[graph_edge->to].push_back(graph_edge);
        }
    }

    ~flow_graph() {
        delete[] edges_buf_m;
    }

    flow_graph(flow_graph &&) noexcept = default;

    flow_graph(const flow_graph &other):
          source_m(other.source_m),
          target_m(other.target_m),
          graph_m(other.size()),
          n_edges_m(other.n_edges_m),
          edges_buf_m(new edge_t[other.n_edges_m]) {

        for (vertex_t i = 0; i < size(); ++i) {
            graph_m[i].reserve(other.degree(i));
        }

        for (vertex_t u = 0; u < size(); ++u) {
            for (const auto edge: other.graph_m[u]) {
                edges_buf_m[edge - other.edges_buf_m]
                    = edge_t(*edge);
                graph_m[u].push_back(edges_buf_m + (edge - other.edges_buf_m));
            }
        }
    }

    flow_graph &operator=(flow_graph &&other) noexcept {
        std::swap(source_m, other.source_m);
        std::swap(target_m, other.target_m);
        std::swap(graph_m, other.graph_m);
        std::swap(n_edges_m, other.n_edges_m);
        std::swap(edges_buf_m, other.edges_buf_m);
        return *this;
    }

    flow_graph &operator=(const flow_graph &other) {
        if (this != &other) {
            *this = flow_graph(other);
        }
        return *this;
    }
};

}
}

#endif // FLOWS_COURSEWORK_FLOWS_UTILS_HPP
