#define UNIT_MAIN

#include <unit.h>

suite(fail) {
    it("should exit program with failure status") {
        require(0);
    }
}
