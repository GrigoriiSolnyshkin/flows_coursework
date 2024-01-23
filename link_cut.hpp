#ifndef FLOWS_COURSEWORK_LINK_CUT_HPP
#define FLOWS_COURSEWORK_LINK_CUT_HPP

#include <optional>
#include <utility>
#include <vector>

namespace flows_coursework {

using vertex_t = std::size_t;

namespace link_cut {

template <typename DataType>
class link_cut {
    static constexpr vertex_t NO_VERTEX = 0;
    using child_type = bool;

    static constexpr child_type LEFT = true;
    static constexpr child_type RIGHT = false;

    struct subtree_min_t {
        vertex_t node = NO_VERTEX;
        DataType result{};

        subtree_min_t() = default;
        subtree_min_t(vertex_t node, DataType result) : node(node), result(result) {
        }
    };

    struct splay_node {
        vertex_t left = NO_VERTEX;
        vertex_t right = NO_VERTEX;
        vertex_t parent = NO_VERTEX;
        std::size_t subtree_size = 1;
        vertex_t link_cut_parent = NO_VERTEX;
        vertex_t extra_parent = NO_VERTEX;

        DataType data{};
        DataType lazy_data{};
        subtree_min_t subtree_min{};

        splay_node() = default;
    };
    std::vector<splay_node> nodes_m;

    [[nodiscard]] vertex_t &get_parent(vertex_t node) {
        return nodes_m[node].parent;
    }

    [[nodiscard]] vertex_t &get_extra_parent(vertex_t node) {
        return nodes_m[node].extra_parent;
    }

    template <child_type CHILD_TYPE>
    [[nodiscard]] vertex_t &get_child(vertex_t node) {
        if constexpr (CHILD_TYPE) {
            return nodes_m[node].left;
        } else {
            return nodes_m[node].right;
        }
    }

    [[nodiscard]] vertex_t &get_link_cut_parent(vertex_t node) {
        return nodes_m[node].link_cut_parent;
    }

    void update(vertex_t node) {
        if (node) {
            vertex_t left = get_child<LEFT>(node);
            vertex_t right = get_child<RIGHT>(node);

            nodes_m[node].subtree_size =
                1 + nodes_m[left].subtree_size + nodes_m[right].subtree_size;

            subtree_min_t left_subtree_min = nodes_m[left].subtree_min;
            subtree_min_t right_subtree_min = nodes_m[right].subtree_min;

            left_subtree_min.result += nodes_m[left].lazy_data;
            right_subtree_min.result += nodes_m[right].lazy_data;

            subtree_min_t subtree_min = {node, nodes_m[node].data};

            if (left_subtree_min.node && left_subtree_min.result <= subtree_min.result) {
                subtree_min = left_subtree_min;
            }

            if (right_subtree_min.node && right_subtree_min.result < subtree_min.result) {
                subtree_min = right_subtree_min;
            }

            nodes_m[node].subtree_min = subtree_min;
        }
    }

    void lazy_update(vertex_t node) {
        if (auto lazy_data = nodes_m[node].lazy_data; node && lazy_data) {
            nodes_m[node].subtree_min.result += lazy_data;
            nodes_m[node].data += lazy_data;

            if (vertex_t left = get_child<LEFT>(node)) {
                nodes_m[left].lazy_data += lazy_data;
            }

            if (vertex_t right = get_child<RIGHT>(node)) {
                nodes_m[right].lazy_data += lazy_data;
            }

            nodes_m[node].lazy_data = DataType{};
        }
    }

    template <child_type CHILD_TYPE>
    void zig(vertex_t node) {
        vertex_t parent_of_node = get_parent(node);
        vertex_t parent_of_parent_of_node = get_parent(parent_of_node);
        vertex_t child_of_node = get_child<!CHILD_TYPE>(node);

        lazy_update(parent_of_node);
        lazy_update(node);

        if (parent_of_parent_of_node) {
            if (get_child<RIGHT>(parent_of_parent_of_node) == parent_of_node) {
                get_child<RIGHT>(parent_of_parent_of_node) = node;
            } else {
                get_child<LEFT>(parent_of_parent_of_node) = node;
            }
        }

        get_parent(node) = parent_of_parent_of_node;
        get_parent(parent_of_node) = node;

        get_child<!CHILD_TYPE>(node) = parent_of_node;
        get_child<CHILD_TYPE>(parent_of_node) = child_of_node;
        if (child_of_node) {
            get_parent(child_of_node) = parent_of_node;
        }

        update(parent_of_node);
        update(node);

        get_extra_parent(node) = get_extra_parent(parent_of_node);
    }

    template <child_type CHILD_TYPE>
    void splay_step(vertex_t node, vertex_t parent) {
        if (auto parent_parent = get_parent(parent); parent_parent == NO_VERTEX) {

            zig<CHILD_TYPE>(node);
        } else if (parent == get_child<CHILD_TYPE>(parent_parent)) {
            zig<CHILD_TYPE>(parent);
            zig<CHILD_TYPE>(node);
        } else {
            zig<CHILD_TYPE>(node);
            zig<!CHILD_TYPE>(node);
        }
    }

    void splay(vertex_t node) {
        vertex_t parent;
        while ((parent = get_parent(node))) {
            if (node == get_child<RIGHT>(parent)) {
                splay_step<RIGHT>(node, parent);
            } else {
                splay_step<LEFT>(node, parent);
            }
        }
    }

    void expose_remove_child(vertex_t node) {
        if (auto child = get_child<RIGHT>(node)) {
            lazy_update(node);

            get_child<RIGHT>(node) = NO_VERTEX;
            get_parent(child) = NO_VERTEX;

            update(node);

            get_extra_parent(child) = node;
        }
    }

    void expose_add_child(vertex_t node, vertex_t child) {
        lazy_update(node);

        get_child<RIGHT>(node) = child;
        get_parent(child) = node;

        update(node);
    }

    vertex_t expose(vertex_t node) {
        splay(node);
        expose_remove_child(node);

        vertex_t next_node;

        while ((next_node = get_extra_parent(node))) {
            splay(next_node);
            expose_remove_child(next_node);
            expose_add_child(next_node, node);

            node = next_node;
        }
        return node;
    }

    void cut(vertex_t node) {
        vertex_t link_cut_parent = get_link_cut_parent(node);
        expose(link_cut_parent);
        get_link_cut_parent(node) = NO_VERTEX;
        splay(node);
        get_extra_parent(node) = NO_VERTEX;
    }

    void link(vertex_t node, vertex_t parent) {
        get_link_cut_parent(node) = parent;

        splay(node);
        get_extra_parent(node) = parent;
        expose(node);
    }

  public:
    explicit link_cut(std::size_t n_vertices) : nodes_m(n_vertices + 1) {
        init();
    }

    [[nodiscard]] std::size_t size() const {
        return nodes_m.size() - 1;
    }

    void init() {
        std::fill(nodes_m.begin(), nodes_m.end(), splay_node{});
        nodes_m[0].subtree_size = 0;
        for (vertex_t i = 1; i < nodes_m.size(); ++i) {
            nodes_m[i].subtree_min.node = i;
        }
    }

    void link_cut_cut(vertex_t node) {
        cut(++node);
    }

    [[nodiscard]] vertex_t link_cut_root(vertex_t node) {
        node = expose(++node);
        vertex_t child;
        while ((child = get_child<LEFT>(node))) {
            node = child;
        }
        splay(node);
        return node - 1;
    }

    [[nodiscard]] std::optional<vertex_t> link_cut_parent(vertex_t node) {
        vertex_t lc_parent = get_link_cut_parent(++node);
        return lc_parent ? std::make_optional(lc_parent - 1) : std::nullopt;
    }

    void link_cut_link(vertex_t node, vertex_t parent) {
        link(++node, ++parent);
    }

    [[nodiscard]] std::size_t link_cut_depth(vertex_t node) {
        return nodes_m[expose(++node)].subtree_size - 1;
    }

    [[nodiscard]] vertex_t link_cut_lca(vertex_t node, vertex_t other) {
        expose(++node);
        return expose(++other) - 1;
    }

    void link_cut_add(vertex_t node, DataType data) {
        expose(++node);
        splay(node);
        nodes_m[node].lazy_data += data;
        if (vertex_t left = get_child<LEFT>(node)) {
            nodes_m[left].lazy_data -= data;
        }
        if (vertex_t right = get_child<RIGHT>(node)) {
            nodes_m[right].lazy_data -= data;
        }
    }

    void link_cut_add_on_path(vertex_t node, DataType data) {
        nodes_m[expose(++node)].lazy_data += data;
    }

    [[nodiscard]] DataType link_cut_get(vertex_t node) {
        splay(++node);
        return nodes_m[node].data + nodes_m[node].lazy_data;
    }

    [[nodiscard]] std::pair<vertex_t, DataType> link_cut_get_min_on_path(vertex_t node) {

        vertex_t path_root = expose(++node);

        return {nodes_m[path_root].subtree_min.node - 1,
                nodes_m[path_root].subtree_min.result + nodes_m[path_root].lazy_data};
    }
};

} // namespace link_cut
} // namespace flows_coursework

#endif // FLOWS_COURSEWORK_LINK_CUT_HPP
