#include "../link_cut.hpp"
#include <cstdint>
#include <doctest.h>
#include <random>
#include <algorithm>

using flows_coursework::link_cut::link_cut;

std::mt19937 generator{42};

TEST_CASE("depths and roots test") {
    link_cut<int64_t> lc(100);

    for (std::size_t i = 0; i < 100; ++i) {
        if (i % 10 == 0) {
            continue;
        }
        lc.link(i, i / 10 * 10);
    }

    for (std::size_t i = 0; i < 100; ++i) {
        CHECK_EQ(lc.link_cut_root(i), i / 10 * 10);
    }

    for (std::size_t i = 0; i < 100; ++i) {
        if (i % 10) {
            CHECK_EQ(lc.link_cut_depth(i), 1);
        } else {
            CHECK_EQ(lc.link_cut_depth(i), 0);
        }
    }

    lc.init();

    for (std::size_t i = 0; i < 100; ++i) {
        if (i % 10 == 0) {
            continue;
        }
        lc.link(i, i - 1);
    }

    for (std::size_t i = 0; i < 100; ++i) {
        CHECK_EQ(lc.link_cut_root(i), i / 10 * 10);
        CHECK_EQ(lc.link_cut_depth(i), i % 10);
    }
}

TEST_CASE("link cut parents test") {
    link_cut<int64_t> lc(100);
    for (int i = 1; i < 100; ++i) {
        lc.link(i, i - 1);
    }
    CHECK_EQ(lc.link_cut_parent(0), std::nullopt);
    for (int i = 1; i < 100; ++i) {
        CHECK_EQ(lc.link_cut_parent(i), i - 1);
        CHECK_EQ(lc.link_cut_root(i), 0);
        CHECK_EQ(lc.link_cut_depth(i), i);
    }

    for (std::size_t i = 5; i < 100; i += 5) {
        lc.cut(i);
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

    for (std::size_t i = 2; i < 128; ++i) {
        lc.link(i, i / 2);
    }

    for (std::size_t i = 1; i < 128; ++i) {
        CHECK_EQ(lc.link_cut_root(i), 1);
    }

    for (std::size_t i = 0; i < 7; ++i) {
        for (std::size_t j = (1 << i); j < (1 << (i + 1)); ++j) {
            CHECK_EQ(lc.link_cut_depth(j), i);
        }
    }

    for (std::size_t j = 16; j < 32; ++j) {
        lc.cut(j);
    }

    for (std::size_t i = 4; i < 7; ++i) {
        for (std::size_t j = (1 << i); j < (1 << (i + 1)); ++j) {
            CHECK_EQ(lc.link_cut_depth(j), i - 4);
            CHECK_EQ(lc.link_cut_root(j), j / (1 << (i - 4)));
        }
    }

    for (std::size_t j = 16; j < 32; ++j) {
        lc.link(j, 1);
    }

    for (std::size_t i = 0; i < 7; ++i) {
        for (std::size_t j = (1 << i); j < (1 << (i + 1)); ++j) {
            CHECK_EQ(lc.link_cut_depth(j), i < 4 ? i : i - 3);
            CHECK_EQ(lc.link_cut_root(j), 1);
        }
    }
}

TEST_CASE("big path test") {
    link_cut<int64_t> lc(1'000'000);

    for (std::size_t i = 1; i < 1'000'000; ++i) {
        lc.link(i, i - 1);
    }

    for (std::size_t i = 0; i < 1'000'000; ++i) {
        CHECK_EQ(lc.link_cut_depth(i), i);
        CHECK_EQ(lc.link_cut_root(i), 0);
    }

    for (std::size_t i = 1; i < 1'000'000; ++i) {
        lc.cut(i);
    }
}

TEST_CASE("big path random test") {
    link_cut<int64_t> lc(1'000'000);
    std::vector<std::size_t> order(999'999);
    std::iota(order.begin(), order.end(), 1);

    std::shuffle(order.begin(), order.end(), generator);
    for (std::size_t i = 0; i < 999'999; ++i) {
        lc.link(order[i], order[i] - 1);
    }

    std::shuffle(order.begin(), order.end(), generator);
    for (std::size_t i = 0; i < 999'999; ++i) {
        CHECK_EQ(lc.link_cut_root(order[i]), 0);
    }

    std::shuffle(order.begin(), order.end(), generator);
    for (std::size_t i = 0; i < 999'999; ++i) {
        CHECK_EQ(lc.link_cut_depth(order[i]), order[i]);
    }

    std::shuffle(order.begin(), order.end(), generator);
    for (std::size_t i = 0; i < 999'999; ++i) {
        lc.cut(order[i]);
    }
}
