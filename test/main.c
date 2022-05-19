#define UNIT_IMPL

#include <unit.h>

suite(unit) {
    echo("start");

    it("contains test") {
        echo("inside first main test");
    }

    it("contains test") {
        echo("inside second main test");
    }

    it("allow test to be skipped", .skip=1) {
        require(NULL, todo);
    }

    describe(skip flag, .skip=1) {
        it("should not run any checks") {
            require(NULL, todo);
        }
    }

    describe(multiple tests) {
        it("block #1 in sub_description") {

        }
        it("block #2 in sub_description") {

        }
    }

    describe(linear execution) {
        it("and could contains skip directive") {
            require_false(NULL, done);
            skip();
            require(0 != 1, todo);
        }
    }

    describe(.allow_fail flag, .allow_fail=1) {
        it("which pass the test in case of failure") {
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

suite(skip_flag, .skip=1) {
    it("should skip the whole suite");
}
