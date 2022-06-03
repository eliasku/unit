#define UNIT_MAIN

#include <unit.h>

SUITE(fail) {
    IT("should exit program with failure status") {
        REQUIRE(0);
    }
}
