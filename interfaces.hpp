
#ifndef FLOWS_COURSEWORK_INTERFACES_HPP
#define FLOWS_COURSEWORK_INTERFACES_HPP

#include <vector>

namespace flows_coursework {

using vertex_t = std::size_t;
using edge_index_t = std::size_t;

template <typename DataType>
struct capacity_edge {
    vertex_t from{};
    vertex_t to{};
    DataType capacity{};

    capacity_edge(vertex_t from, vertex_t to, DataType capacity)
        : from(from), to(to), capacity(capacity) {
    }

    capacity_edge() = default;
};

template <typename DataType>
struct flows_solver {
    virtual std::vector<DataType> solve(std::size_t, vertex_t, vertex_t,
                                        const std::vector<capacity_edge<DataType>> &) = 0;

    virtual ~flows_solver() = default;
};

} // namespace flows_coursework

#endif // FLOWS_COURSEWORK_INTERFACES_HPP
