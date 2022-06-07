#define UNIT_IMPLEMENT
#define UNIT__SELF_TEST

#include <unit.h>

int main(int argc, const char** argv) {
    (void) argc;
    (void) argv;
    int result = 0;
    result |= unit_main((struct unit_run_options){1});
    result |= unit_main((struct unit_run_options){0, 1});
    result |= unit_main((struct unit_run_options){0, 0, 1});
    result |= unit_main((struct unit_run_options){0, 0, 0, 0, 0, 0, 0, 0});
    result |= unit_main((struct unit_run_options){0, 0, 0, 1, 1, 0, 0, 0});
    result |= unit_main((struct unit_run_options){0, 0, 0, 0, 0, 1, 0, 0});
    result |= unit_main((struct unit_run_options){0, 0, 0, 1, 0, 0, 1, 0});
    result |= unit_main((struct unit_run_options){0, 0, 0, 0, 0, 0, 0, 1});
    return result;
}

SUITE(unit) {
    ECHO("start");

    DESCRIBE(UNIT_TEST) {
        IT("without body");
        IT("in one-line") CHECK_GT(1, 0), ECHO("wow");
        IT("could be skipped", .skip=1) REQUIRE(0, skipped);
    }

    DESCRIBE(UNIT_DESCRIBE) {
        DESCRIBE();
        // this is not possible because `test` declares static variable before scope
        //describe("in one-line") it("p");

        DESCRIBE(.skip, .skip=1) {
            IT("which skip any test inside") REQUIRE(0, skip);
        }

        // hint for IDE: (struct unit_test){.options.failing}
        DESCRIBE(.failing, .failing=1) {
            IT("which pass in case of tests failure") {
                REQUIRE_EQ(2, 2);
                ECHO("Next require will fail");
                REQUIRE_EQ(1, 0);
                ECHO("Execution actually continues,");
                ECHO("but another checks will be ignored after fail.");
                REQUIRE(0);
                REQUIRE(NULL);
            }
        }
    }

    DESCRIBE(SKIP) {
        IT("start skipping checks in the middle") {
            REQUIRE(1, OK);
            SKIP();
            REQUIRE(0, SKIP);
        }
    }
}

SUITE(unit skip, .skip=1) {
    IT("should skip the whole suite");
}

SUITE(unit fail, .failing=1) {
    IT("should pass suite in case of failure") {
        REQUIRE(0);
    }
}
