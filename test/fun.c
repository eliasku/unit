#include <unit.h>

static int fun__counter = 0;
static int fun__init() {
    return ++fun__counter;
}
static int* fun__is[3];

suite(fun_statics) {
    //static int i0 = fun__init();
    static int i1 = 1;
    fun__is[0] = &i1;
    it("dhh") {
        static int i2 = 2;
        fun__is[1] = &i2;
    }

    it("correct and accessible") {
        require_eq(*fun__is[0], 1);
        require_eq(*fun__is[1], 2);
    }
}
