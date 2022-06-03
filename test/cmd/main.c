#define UNIT_MAIN

#include <unit.h>

SUITE(unit_cmd) {
    DESCRIBE(find_bool_arg) {
        IT("parse short form") {
            int val = 0;
            find_bool_arg(3, (const char* []) {"not-an-arg", "-a", "--along"}, &val, "", "a");
            REQUIRE_EQ(val, 1);
        }
        IT("parse long form") {
            int val = 5;
            find_bool_arg(3, (const char* []) {"", "-a", "--along"}, &val, "along", "");
            REQUIRE_EQ(val, 1);
        }
        IT("invert with no-") {
            int val = 2;
            find_bool_arg(3, (const char* []) {NULL, "-a", "--no-along"}, &val, "along", "");
            REQUIRE_EQ(val, 0);
        }
        IT("don't change on not found") {
            int val = 2;
            find_bool_arg(4, (const char* []) {"--etemp", "-nt", "-a", "--no-along"}, &val, "temp", "t");
            REQUIRE_EQ(val, 2);
        }
    }

    DESCRIBE(unit__parse_args) {
        IT("options") {
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
            REQUIRE_EQ(options.silent, 1);
            REQUIRE_EQ(options.animate, 1);
            REQUIRE_EQ(options.trace, 1);
            REQUIRE_EQ(options.doctest_xml, 1);
            REQUIRE_EQ(options.color, 0);
        }
    }
}
