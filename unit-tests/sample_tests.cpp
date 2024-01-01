#include <doctest.h>

TEST_CASE("sample") {
    int *p = new int[100];
    CHECK_EQ(2 * 2, 4);
}
