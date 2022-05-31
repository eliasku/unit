//#define UNIT_VERBOSE
//#define UNIT_ANIMATE
#define UNIT_MAIN
#include <unit.h>

suite(fail) {
    it("should exit program with failure status") {
        require(0);
    }
}
