#include "../link_cut.hpp"
#include <cstdint>
#include <doctest.h>
#include <random>
#include <algorithm>
#include <utility>

using flows_coursework::link_cut::link_cut;

static std::mt19937 generator{42}; //NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

TEST_CASE("depths and roots test") {
    link_cut<int64_t> lc(100);

    for (int i = 0; i < 100; ++i) {
        if (i % 10 == 0) {
            continue;
        }
        lc.link_cut_link(i, i / 10 * 10);
    }

    for (int i = 0; i < 100; ++i) {
        CHECK_EQ(lc.link_cut_root(i), i / 10 * 10);
    }

    for (int i = 0; i < 100; ++i) {
        if (i % 10) {
            CHECK_EQ(lc.link_cut_depth(i), 1);
        } else {
            CHECK_EQ(lc.link_cut_depth(i), 0);
        }
    }

    lc.init();

    for (int i = 0; i < 100; ++i) {
        if (i % 10 == 0) {
            continue;
        }
        lc.link_cut_link(i, i - 1);
    }

    for (int i = 0; i < 100; ++i) {
        CHECK_EQ(lc.link_cut_root(i), i / 10 * 10);
        CHECK_EQ(lc.link_cut_depth(i), i % 10);
    }
}

TEST_CASE("link cut parents test") {
    link_cut<int64_t> lc(100);
    for (int i = 1; i < 100; ++i) {
        lc.link_cut_link(i, i - 1);
    }
    CHECK_EQ(lc.link_cut_parent(0), std::nullopt);
    for (int i = 1; i < 100; ++i) {
        CHECK_EQ(lc.link_cut_parent(i), i - 1);
        CHECK_EQ(lc.link_cut_root(i), 0);
        CHECK_EQ(lc.link_cut_depth(i), i);
    }

    for (int i = 5; i < 100; i += 5) {
        lc.link_cut_cut(i);
    }

    for (int i = 0; i < 100; ++i) {
        if (i % 5) {
            CHECK_EQ(lc.link_cut_parent(i), i - 1);
        } else {
            CHECK_EQ(lc.link_cut_parent(i), std::nullopt);
        }

        CHECK_EQ(lc.link_cut_root(i), i / 5 * 5);
        CHECK_EQ(lc.link_cut_depth(i), i % 5);
    }

}

TEST_CASE("binary tree test") {
    link_cut<int64_t> lc(128);

    for (int i = 2; i < 128; ++i) {
        lc.link_cut_link(i, i / 2);
    }

    for (int i = 1; i < 128; ++i) {
        CHECK_EQ(lc.link_cut_root(i), 1);
    }

    for (int i = 0; i < 7; ++i) {
        for (int j = (1 << i); j < (1 << (i + 1)); ++j) {
            CHECK_EQ(lc.link_cut_depth(j), i);
        }
    }

    for (int j = 16; j < 32; ++j) {
        lc.link_cut_cut(j);
    }

    for (int i = 4; i < 7; ++i) {
        for (int j = (1 << i); j < (1 << (i + 1)); ++j) {
            CHECK_EQ(lc.link_cut_depth(j), i - 4);
            CHECK_EQ(lc.link_cut_root(j), j / (1 << (i - 4)));
        }
    }

    for (int j = 16; j < 32; ++j) {
        lc.link_cut_link(j, 1);
    }

    for (int i = 0; i < 7; ++i) {
        for (int j = (1 << i); j < (1 << (i + 1)); ++j) {
            CHECK_EQ(lc.link_cut_depth(j), i < 4 ? i : i - 3);
            CHECK_EQ(lc.link_cut_root(j), 1);
        }
    }
}

TEST_CASE("big path test") {
    link_cut<int64_t> lc(1'000'000);

    for (int i = 1; i < 1'000'000; ++i) {
        lc.link_cut_link(i, i - 1);
    }

    for (int i = 0; i < 1'000'000; ++i) {
        CHECK_EQ(lc.link_cut_depth(i), i);
        CHECK_EQ(lc.link_cut_root(i), 0);
    }

    for (int i = 1; i < 1'000'000; ++i) {
        lc.link_cut_cut(i);
    }
}

TEST_CASE("big path random test") {
    link_cut<int64_t> lc(1'000'000);
    std::vector<std::size_t> order(999'999);
    std::iota(order.begin(), order.end(), 1);

    std::shuffle(order.begin(), order.end(), generator);
    for (int i = 0; i < 999'999; ++i) {
        lc.link_cut_link(order[i], order[i] - 1);
    }

    std::shuffle(order.begin(), order.end(), generator);
    for (int i = 0; i < 999'999; ++i) {
        CHECK_EQ(lc.link_cut_root(order[i]), 0);
    }

    std::shuffle(order.begin(), order.end(), generator);
    for (int i = 0; i < 999'999; ++i) {
        CHECK_EQ(lc.link_cut_depth(order[i]), order[i]);
    }

    std::shuffle(order.begin(), order.end(), generator);
    for (int i = 0; i < 999'999; ++i) {
        lc.link_cut_cut(order[i]);
    }
}

TEST_CASE("lca tests") {
    link_cut<int64_t> lc(128);
    for (int i = 2; i < 128; ++i) {
        lc.link_cut_link(i, i / 2);
    }

    for (int i_pow = 0; i_pow < 7; ++i_pow) {
        for (int i = 1 << i_pow; i < (1 << (i_pow + 1)); ++i) {
            for (int j_pow = 0; j_pow < 7; ++j_pow) {
                for (int j = 1 << j_pow; j < (1 << (j_pow + 1)); ++j) {
                    int i_copy = i;
                    int j_copy = j;
                    if (i_pow > j_pow) {
                        i_copy >>= (i_pow - j_pow);
                    } else {
                        j_copy >>= (j_pow - i_pow);
                    }

                    while (i_copy != j_copy) {
                        i_copy >>= 1;
                        j_copy >>= 1;
                    }

                    CHECK_EQ(lc.link_cut_lca(i, j), i_copy);
                }
            }
        }
    }
}

TEST_CASE("test ops add and get") {
    link_cut<int64_t> lc(128);

    for (int i = 2; i < 128; ++i) {
        lc.link_cut_link(i, i / 2);
    }

    for (int i = 0; i < 128; ++i) {
        lc.link_cut_add(i, i);
    }

    for (int i = 0; i < 128; ++i) {
        CHECK_EQ(lc.link_cut_get(i), i);
    }

    lc.init();

    for (int i = 1; i < 128; ++i) {
        lc.link_cut_link(i, i - 1);
        lc.link_cut_add(i, i);
    }

    lc.link_cut_cut(64);

    for (int i = 0; i < 128; ++i) {
        CHECK_EQ(lc.link_cut_get(i), i);
    }
}

TEST_CASE("test ops add on path") {
    link_cut<int64_t> lc(128);

    for (int i = 2; i < 128; ++i) {
        lc.link_cut_link(i, i / 2);
    }

    for (int i = 64; i < 128; ++i) {
        lc.link_cut_add_on_path(i, 1);
    }

    for (int j = 0; j < 7; ++j) {
        for (int i = (1 << j); i < (1 << (j + 1)); ++i) {
            CHECK_EQ(lc.link_cut_get(i), 1 << (6 - j));
        }
    }

    lc.init();

    for (int i = 0; i < 128; ++i) {
        if (i % 16) {
            lc.link_cut_link(i, i / 16 * 16);
        }
    }

    for (int i = 0; i < 128; ++i) {
        lc.link_cut_add_on_path(i, 1);
    }

    for (int i = 0; i < 128; ++i) {
        CHECK_EQ(lc.link_cut_get(i), i % 16 == 0 ? 16 : 1);
    }

    lc.init();

    lc.link_cut_add_on_path(0, 1);
    for (int i = 1; i < 128; ++i) {
        lc.link_cut_link(i, i - 1);
        lc.link_cut_add_on_path(i, 1);
    }

    for (int i = 0; i < 128; ++i) {
        CHECK_EQ(lc.link_cut_get(i), 128 - i);
    }
}

TEST_CASE("test ops get on path") {
    link_cut<int64_t> lc(128);

    for (int i = 0; i < 128; ++i) {
        lc.link_cut_add(i, i);
    }

    for (int i = 0; i < 128; ++i) {
        if (i % 16) {
            lc.link_cut_link(i, i - 1);
        }
    }

    for (int i = 128; i--;) {
        auto result = lc.link_cut_get_min_on_path(i);
        CHECK_EQ(result.first, i / 16 * 16);
        CHECK_EQ(result.second, i / 16 * 16);
    }

    for (int i = 8; i < 128; i += 16) {
        lc.link_cut_cut(i);
    }

    for (int i = 0; i < 128; ++i) {
        auto result = lc.link_cut_get_min_on_path(i);
        CHECK_EQ(result.first, i / 8 * 8);
        CHECK_EQ(result.second, i / 8 * 8);
    }

    for (int i = 8; i < 128; i += 16) {
        lc.link_cut_add_on_path(i - 1, 16);
        lc.link_cut_link(i, i - 1);
    }

    for (int i = 0; i < 128; ++i) {
        CHECK_EQ(lc.link_cut_get(i), i & 8 ? i : i + 16);
    }

    for (int i = 0; i < 128; ++i) {
        auto result = lc.link_cut_get_min_on_path(i);
        CHECK_EQ(result.first, i / 8 * 8);
        CHECK_EQ(result.second, i & 8 ? i / 8 * 8 : i / 8 * 8 + 16);
    }
}

TEST_CASE("test ops random path test") {
    link_cut<int64_t> lc(1000);

    std::vector<int64_t> data(1000);

    for (int i = 0; i < 1000; ++i) {
        int64_t entry = std::uniform_int_distribution<int64_t>(0, 999)(generator);
        lc.link_cut_add(i, entry);
        data[i] = entry;
        if (i) {
            lc.link_cut_link(i, i - 1);
        }
    }

    for (int lo = 0; lo < 1000; ++lo) {

        if (lo) {
            lc.link_cut_cut(lo);
        }

        int64_t min_entry = data[lo];
        std::size_t argmin_entry = lo;

        for (int hi = lo; hi < 1000; ++hi) {
            if (data[hi] < min_entry) {
                min_entry = data[hi];
                argmin_entry = hi;
            }

            auto result = lc.link_cut_get_min_on_path(hi);
            CHECK_EQ(result.first, argmin_entry);
            CHECK_EQ(result.second, min_entry);
        }

        if (lo) {
            lc.link_cut_link(lo, lo - 1);
        }
    }
}

