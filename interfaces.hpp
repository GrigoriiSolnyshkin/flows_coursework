
#ifndef FLOWS_COURSEWORK_INTERFACES_HPP
#define FLOWS_COURSEWORK_INTERFACES_HPP

#include <vector>

namespace flows_coursework {

using vertex_t = std::size_t;

template <typename DataType>
struct capacity_edge {
    vertex_t from{};
    vertex_t to{};
    DataType capacity{};

    capacity_edge(vertex_t from, vertex_t to, DataType capacity):
          from(from), to(to), capacity(capacity) {
    }

    capacity_edge() = default;
};

template <typename DataType>
struct flows_solver {
    virtual std::vector<DataType> solve(
        std::size_t graph_size,
        vertex_t source, vertex_t to,
        const std::vector<capacity_edge<DataType>> & edges
        ) = 0;

    virtual ~flows_solver() = default;
};

}



#endif // FLOWS_COURSEWORK_INTERFACES_HPP
