#include <unit.h>

static int* fun__is[3];

SUITE(fun_statics) {
    static int i1 = 1;
    fun__is[0] = &i1;
    IT("dhh") {
        static int i2 = 2;
        fun__is[1] = &i2;
    }

    IT("correct and accessible") {
        REQUIRE_EQ(*fun__is[0], 1);
        REQUIRE_EQ(*fun__is[1], 2);
    }
}
