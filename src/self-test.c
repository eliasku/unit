UNIT_SUITE(unit_internals) {
    UNIT_DESCRIBE(beautify_filename) {
        IT("short filenames") {
            CHECK_EQ(short_filename(NULL), (const char*) NULL);
            CHECK_EQ(short_filename("/path/to/file.c"), "file.c");
            CHECK_EQ(short_filename("file.c"), "file.c");
        }

        IT("depends on option") {
            const bool prev = unit__opts.short_filenames;
            unit__opts.short_filenames = 1;
            CHECK_EQ(beautify_filename("/path/to/file.c"), "file.c");
            unit__opts.short_filenames = 0;
            CHECK_EQ(beautify_filename("/path/to/file.c"), "/path/to/file.c");
            unit__opts.short_filenames = prev;
        }
    }

    UNIT_DESCRIBE(beautify_name) {
        IT("replace empty string with (anonymous)") {
            CHECK_EQ(beautify_name(NULL), "(anonymous)");
            CHECK_EQ(beautify_name(""), "(anonymous)");
            CHECK_EQ(beautify_name("name"), "name");
        }
    }

    UNIT_DESCRIBE(get_spaces) {
        IT("clamp index to available range") {
            CHECK_EQ(get_spaces(0), "");
            CHECK_EQ(get_spaces(-1), "");
            CHECK_EQ(get_spaces(9999), unit_spaces[sizeof(unit_spaces) / sizeof(unit_spaces[0]) - 1]);
        }
    }

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
                                     "--quiet",
                                     "--ascii",
                                     "-r=xml",
                                     "-t",
                                     "-a",
                                     "not-found",
                                     NULL
                             }, &options);
            REQUIRE_EQ(options.quiet, 1);
            REQUIRE_EQ(options.animate, 1);
            REQUIRE_EQ(options.trace, 1);
            REQUIRE_EQ(options.doctest_xml, 1);
            REQUIRE_EQ(options.ascii, 1);
        }
    }
}