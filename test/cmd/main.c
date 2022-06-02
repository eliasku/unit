#define UNIT_MAIN

#include <unit.h>

suite(unit_cmd) {
    describe(find_bool_arg) {
        it("parse short form") {
            int val = 0;
            find_bool_arg(3, (const char* []) {"not-an-arg", "-a", "--along"}, &val, "", "a");
            require_eq(val, 1);
        }
        it("parse long form") {
            int val = 5;
            find_bool_arg(3, (const char* []) {"", "-a", "--along"}, &val, "along", "");
            require_eq(val, 1);
        }
        it("invert with no-") {
            int val = 2;
            find_bool_arg(3, (const char* []) {NULL, "-a", "--no-along"}, &val, "along", "");
            require_eq(val, 0);
        }
        it("don't change on not found") {
            int val = 2;
            find_bool_arg(4, (const char* []) {"--etemp", "-nt", "-a", "--no-along"}, &val, "temp", "t");
            require_eq(val, 2);
        }
    }

    describe(unit__parse_args) {
        it("options") {
            struct unit_run_options options = {0};
            unit__parse_args(6,
                             (const char* []) {
                                     "--silent",
                                     "--no-colors",
                                     "-r=xml",
                                     "-t",
                                     "-a",
                                     "not-found",
                                     NULL
                             }, &options);
            require_eq(options.silent, 1);
            require_eq(options.animate, 1);
            require_eq(options.trace, 1);
            require_eq(options.doctest_xml, 1);
            require_eq(options.color, 0);
        }
    }
}
