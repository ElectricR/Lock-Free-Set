#include "../src/flags.h"
#include <gtest/gtest.h>

TEST(MacroTest, Simple) {
    int x = 1;
    int *px = &x;
    SET_FLAG(int*, px, true);
    ASSERT_EQ(GET_FLAG(px), true);
    ASSERT_EQ(GET_POINTER(int*, px), &x);

    SET_FLAG(int*, px, false);
    ASSERT_EQ(px, &x);
    ASSERT_EQ(GET_FLAG(px), false);
    ASSERT_EQ(GET_POINTER(int*, px), &x);

    SET_FLAG(int*, px, true);
    ASSERT_EQ(GET_FLAG(px), true);
    ASSERT_EQ(GET_POINTER(int*, px), &x);
}
     
