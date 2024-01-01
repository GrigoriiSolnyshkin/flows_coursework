#ifndef FLOWS_COURSEWORK_LINK_CUT_HPP
#define FLOWS_COURSEWORK_LINK_CUT_HPP

#include <vector>

namespace flows_coursework {

namespace link_cut {

template <typename DataType>
class link_cut {
    using vertex_t = std::size_t;
    using child_type = bool;

    static constexpr child_type LEFT = true;
    static constexpr child_type RIGHT = false;

    struct splay_node {
        vertex_t left = 0;
        vertex_t right = 0;
        vertex_t parent = 0;
        std::size_t subtree_size = 1;

        DataType data{};

        explicit splay_node(DataType data) : data(data) {
        }

        splay_node() {
        }
    };
    std::vector<splay_node> nodes_m;

    [[nodiscard]] vertex_t &get_parent(vertex_t node) {
        return nodes_m[node].parent;
    }

    template <child_type CHILD_TYPE>
    vertex_t &get_child(vertex_t node) {
        if constexpr (CHILD_TYPE) {
            return nodes_m[node].left;
        } else {
            return nodes_m[node].right;
        }
    }

    void update(vertex_t node) {
        if (node) {
            nodes_m[node].subtree_size = 1 + nodes_m[get_child<LEFT>(node)].subtree_size +
                                         nodes_m[get_child<RIGHT>(node)].subtree_size;
        }
    }

    template <child_type CHILD_TYPE>
    void zig(vertex_t node) {
        vertex_t parent_of_node = get_parent(node);
        vertex_t parent_of_parent_of_node = get_parent(parent_of_node);
        vertex_t child_of_node = get_child<!CHILD_TYPE>(node);

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
        update(parent_of_parent_of_node);
    }

    template <child_type CHILD_TYPE>
    void splay_step(vertex_t node, vertex_t parent) {
        if (auto parent_parent = get_parent(); parent_parent == 0) {
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

  public:
    explicit link_cut(std::size_t n_vertices) : nodes_m(n_vertices + 1) {
        nodes_m[0].subtree_size = 0;
    }

    [[nodiscard]] std::size_t size() const {
        return nodes_m.size() - 1;
    }
};

} // namespace link_cut
} // namespace flows_coursework

#endif // FLOWS_COURSEWORK_LINK_CUT_HPP
