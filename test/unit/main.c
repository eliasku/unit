//#define UNIT_VERBOSE
//#define UNIT_ANIMATE
#define UNIT_MAIN
#include <unit.h>

suite(unit) {
    echo("start");

    describe((UNIT_TEST, it, test)) {
        it("without body");
        it("in one-line") check_gt(1, 0), echo("wow");
        it("could be skipped", .skip=1) require(0, skipped);
    }

    describe((UNIT_DESCRIBE, describe)) {
        describe();
        // this is not possible because `test` declares static variable before scope
        //describe("in one-line") it("p");

        describe(unit_cur->skip, .skip=1) {
            it("which skip any test inside") require(0, SKIP);
        }

        describe(unit_cur->allow_fail, .failing=1) {
            it("which pass in case of tests failure") {
                require_eq(2, 2);
                echo("Next require will fail");
                require_eq(1, 0);
                echo("Execution actually continues,");
                echo("but another checks will be ignored after fail.");
                require(0);
                require(NULL);
            }
        }
    }

    describe(skip) {
        it("start skipping checks in the middle") {
            require(1, OK);
            skip();
            require(0, SKIP);
        }
    }
}

suite(unit skip, .skip=1) {
    it("should skip the whole suite");
}

suite(unit fail, .failing=1) {
    it("should pass suite in case of failure") {
        require(0);
    }
}
