#include "../link_cut.hpp"
#include <cstdint>
#include <doctest.h>

using flows_coursework::link_cut::link_cut;


TEST_CASE("create link cut") {
    link_cut<int64_t> lc(10);
    lc.init();
}
