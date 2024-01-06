#include "../flows_utils.hpp"
#include <cstdint>
#include <doctest.h>

using flows_coursework::capacity_edge;
using flows_coursework::flows_utils::flow_graph;

TEST_CASE("flow graph_m copy, destruct, construct, move") {
    std::vector<capacity_edge<int64_t>> edges;

    for (int i = 0; i < 99; ++i) {
        edges.emplace_back(i, i + 1, 100);
    }

    flow_graph fg(100, 0, 99, edges);
    flow_graph fg_copy = fg;
    flow_graph fg_other(100, 5, 6, edges);

    fg_copy = std::move(fg_other);
    CHECK_EQ(fg_copy.source(), 5);
    CHECK_EQ(fg.source(), 0);
}

TEST_CASE("push may push") {
    std::vector<capacity_edge<int64_t>> edges;

    for (int i = 0; i < 99; ++i) {
        edges.emplace_back(i, i + 1, 100);
    }

    flow_graph fg(100, 0, 99, edges);

    CHECK_EQ(fg.size(), 100);
    for (int i = 1; i < 99; ++i) {
        CHECK_EQ(fg.degree(i), 2);

        auto &edge0 = fg.get_edge_by_vertex(i, 0);
        auto &edge1 = fg.get_edge_by_vertex(i, 1);
        CHECK_EQ(edge0.capacity, 100);
        CHECK_EQ(edge1.capacity, 100);
        CHECK_EQ(edge0.flow_value, 0);
        CHECK_EQ(edge1.flow_value, 0);
        CHECK_EQ(fg.may_push(i, edge0) + fg.may_push(i, edge1), 100);
    }

    for (int u = 0; u < 99; ++u) {
        for (std::size_t i = 0; i < fg.degree(u); ++i) {
            auto &edge = fg.get_edge_by_vertex(u, i);
            if (fg.adjacent(u, edge) == static_cast<flows_coursework::vertex_t>(u + 1)) {
                fg.push(u, edge, 50);
            }
            CHECK_EQ(edge.flow_value, 50);
        }
    }

    for (int i = 1; i < 99; ++i) {
        CHECK_EQ(fg.may_push(i, fg.get_edge_by_vertex(i, 1)), 50);
        CHECK_EQ(fg.may_push(i, fg.get_edge_by_vertex(i, 0)), 50);
    }
}
