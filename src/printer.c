#include "fmemopen.c"

// region Цвета, текстовые сообщения и логи

#define UNIT_COLOR_RESET "\033[m"
#define UNIT_COLOR_BOLD "\033[1m"
#define UNIT_COLOR_DIM "\033[2m"
#define UNIT_COLOR_UNDERLINE "\033[4m"
#define UNIT_COLOR_MAYBE "\033[35m"
#define UNIT_COLOR_COMMENT "\033[36m"
#define UNIT_COLOR_SUCCESS "\033[32m"
#define UNIT_COLOR_FAIL "\033[31m"
#define UNIT_COLOR_DESC "\033[33m"
#define UNIT_COLOR_INVERT_PASS "\033[30;42m"
#define UNIT_COLOR_LABEL_PASS UNIT_COLOR_BOLD UNIT_COLOR_INVERT_PASS
#define UNIT_COLOR_LABEL_FAIL UNIT_COLOR_BOLD "\033[30;41m"
#define UNIT_COLOR_LABEL_SKIP UNIT_COLOR_BOLD "\033[30;47m"
#define UNIT_COLOR_LABEL_RUNS UNIT_COLOR_BOLD "\033[30;46m"

/**
 * Alternative icons:
 * message icons: "💬 ", "# ", "ℹ ", " ⃫ "
 * skip icons: "Ⅱ "
 */

enum {
    ICON_RUN = 0,
    ICON_OK = 1,
    ICON_SKIP = 2,
    ICON_FAIL = 3,
    ICON_ASSERT = 4,
    ICON_MSG = 5
};

static const char* icon(int type) {
    static const char* pretty[] = {
            UNIT_COLOR_BOLD UNIT_COLOR_SUCCESS "▶ " UNIT_COLOR_RESET,
            UNIT_COLOR_BOLD UNIT_COLOR_SUCCESS "✓ " UNIT_COLOR_RESET,
            UNIT_COLOR_BOLD UNIT_COLOR_DIM "∅ " UNIT_COLOR_RESET,
            UNIT_COLOR_BOLD UNIT_COLOR_FAIL "✕ " UNIT_COLOR_RESET,
            UNIT_COLOR_BOLD UNIT_COLOR_FAIL "● " UNIT_COLOR_RESET,
            UNIT_COLOR_BOLD UNIT_COLOR_COMMENT "» " UNIT_COLOR_RESET,
    };
    static const char* plain[] = {
            "> ",
            "+ ",
            ". ",
            "x ",
            "! ",
            "# ",
    };
    return unit__opts.color ? pretty[type] : plain[type];
}

// region reporting

static void unit__sleep(double seconds) {
#ifndef _WIN32
    const long secs = (long) seconds;
    const long nanos = (long) ((seconds - (double) secs) * 1000000000.0);
    struct timespec ts = {secs, nanos};
    nanosleep(&ts, NULL);
#endif
}

static void print_wait(FILE* f) {
    fflush(f);
    if (unit__opts.animate) {
        unit__sleep(0.1);
    }
}

void begin_style(FILE* file, const char* style) {
    if (style && unit__opts.color) {
        fputs(style, file);
    }
}

void end_style(FILE* file) {
    if (unit__opts.color) {
        fputs(UNIT_COLOR_RESET, file);
    }
}

void print_text(FILE* file, const char* text, const char* style) {
    begin_style(file, style);
    fputs(text, file);
    end_style(file);
}

static char unit__fails_mem[4096];
static FILE* unit__fails = 0;

const char* unit_spaces[8] = {
        "",
        "  ",
        "    ",
        "      ",
        "        ",
        "          ",
        "            ",
        "              "
};

void print_elapsed_time(FILE* f, double elapsed_time) {
    if (elapsed_time >= 0.01) {
        begin_style(f, UNIT_COLOR_DIM);
        fprintf(f, " (%0.2lf ms)", elapsed_time);
        end_style(f);
    }
}

void unit__end_line(FILE* f, double elapsed_time) {
    print_elapsed_time(f, elapsed_time);
    fputc('\n', f);
}

static const char* beautify_name(const char* name) {
    return UNIT__IS_NOT_EMPTY_STR(_, name) ? name : "(anonymous)";
}

static void print_label(FILE* f, struct unit_test* node) {
    static const char* pretty[] = {
            UNIT_COLOR_LABEL_RUNS " RUNS " UNIT_COLOR_RESET,
            UNIT_COLOR_LABEL_PASS " PASS " UNIT_COLOR_RESET,
            UNIT_COLOR_LABEL_SKIP " SKIP " UNIT_COLOR_RESET,
            UNIT_COLOR_LABEL_FAIL " FAIL " UNIT_COLOR_RESET,
    };
    static const char* plain[] = {
            "[RUNS]",
            "[PASS]",
            "[SKIP]",
            "[FAIL]",
    };
    const int type = node->status;
    const char* lbl = (unit__opts.color ? pretty : plain)[type];

    fputs(lbl, f);
    fputc(' ', f);
    begin_style(f, UNIT_COLOR_BOLD);
    fputs(beautify_name(node->name), f);
    end_style(f);
    fputc(' ', f);
    switch (node->status) {
        case UNIT_STATUS_RUN:
            break;
        case UNIT_STATUS_SUCCESS:
        case UNIT_STATUS_FAILED:
            fprintf(f, ": passed %d/%d tests", node->passed, node->total);
            print_elapsed_time(f, node->elapsed);
            break;
        default:
            break;
    }
    print_wait(f);
}

void printer_def_begin(struct unit_test* unit) {
    FILE* f = stdout;
    if (!unit->parent) {
        print_label(f, unit);
    }
}

int def_depth = 0;

const char* unit__spaces(int delta) {
    int i = def_depth + delta;
    if (i < 0) i = 0;
    if (i > 7) i = 7;
    return unit_spaces[i];
}

static void print_node(struct unit_test* node) {
    FILE* f = stdout;
    ++def_depth;
    const char* name = beautify_name(node->name);
    fputs(unit__spaces(0), f);
    if (node->type == UNIT__TYPE_CASE) {
        if (node->status == UNIT_STATUS_SKIPPED) {
            print_text(f, name, UNIT_COLOR_DIM);
        } else {
            print_text(f, name, NULL);
        }
    } else {
        fputs(icon(node->status), f);
        print_text(f, name, UNIT_COLOR_DIM);
    }
    unit__end_line(f, node->elapsed);
    print_wait(f);
    for (struct unit_test* child = node->children; child; child = child->next) {
        print_node(child);
    }
    --def_depth;
}

void printer_def_end(struct unit_test* unit) {
    FILE* f = stdout;
    if (unit->parent) {
        if (unit->type == UNIT__TYPE_TEST) {
            fputs(icon(unit->status), f);
            print_wait(f);
        }
        return;
    }
    // go back to the beginning of line
    fputc('\n', f);
    begin_style(f, "\033[1A\033[999D");
    const char* name = beautify_name(unit->name);
    print_label(f, unit);
    fputc('\n', f);

    if (unit__fails) {
        for (struct unit_test* child = unit->children; child; child = child->next) {
            print_node(child);
        }
        fputc('\n', stdout);

        const long pos = ftell(unit__fails);
        unit__fails_mem[pos] = 0;
        fclose(unit__fails);
        unit__fails = 0;
        fputs(unit__fails_mem, f);

        fputc('\n', f);
    }
}

static void unit__breadcrumbs(FILE* f, struct unit_test* test) {
    if (test->parent) {
        unit__breadcrumbs(f, test->parent);
        print_text(f, unit__opts.color ? " → " : " > ", UNIT_COLOR_BOLD UNIT_COLOR_DIM);
    }
    print_text(f, beautify_name(test->name), UNIT_COLOR_BOLD UNIT_COLOR_FAIL);
}

void printer_def_fail(struct unit_test* unit, const char* msg) {
    if (unit__fails == 0) {
        // TODO: change to `tmpfile` ?
        unit__fails = fmemopen(unit__fails_mem, sizeof unit__fails_mem, "w");
    }
    FILE* f = unit__fails;
    fputs(unit_spaces[1], f);
    fputs(icon(ICON_ASSERT), f);
    unit__breadcrumbs(f, unit_cur);
    fputc('\n', f);
    fputc('\n', f);

    fputs(unit_spaces[2], f);
    fputs(msg, f);
    fputc('\n', f);
#ifndef UNIT_NO_FILEPOS
    if (unit->assert_file) {
        fputs(unit_spaces[2], f);
        print_text(f, "@ ", UNIT_COLOR_COMMENT UNIT_COLOR_DIM UNIT_COLOR_BOLD);
        begin_style(f, UNIT_COLOR_COMMENT UNIT_COLOR_UNDERLINE);
        fprintf(f, "%s:%d", unit->assert_file, unit->assert_line);
        end_style(f);
        fputc('\n', f);
    }
#endif
    fputc('\n', f);
}

// endregion reporting

static void printer_def(int cmd, struct unit_test* unit, const char* msg) {
    switch (cmd) {
        case UNIT__PRINTER_SETUP:
            fputs(unit__opts.color ? "\n\033[1;30;42m" " ✓ηỉτ " "\033[0;30;46m" " v" UNIT_VERSION " " "\33[m\n\n" :
                  "\n[unit] v" UNIT_VERSION "\n\n", stdout);
            print_wait(stdout);
            break;
        case UNIT__PRINTER_BEGIN:
            printer_def_begin(unit);
            break;
        case UNIT__PRINTER_END:
            printer_def_end(unit);
            break;
        case UNIT__PRINTER_FAIL:
            printer_def_fail(unit, msg);
            break;
        default:
            break;
    }
}

// debug printer
static int debug_depth = 0;

static const char* debug_spaces(int delta) {
    int i = debug_depth + delta;
    if (i < 0) i = 0;
    if (i > 7) i = 7;
    return unit_spaces[i];
}

static void printer_debug(int cmd, struct unit_test* unit, const char* msg) {
    FILE* f = stdout;
    switch (cmd) {
        case UNIT__PRINTER_BEGIN:
            fputs(debug_spaces(0), f);
            print_text(f, beautify_name(unit->name), UNIT_COLOR_BOLD);
            print_text(f, " {\n", UNIT_COLOR_DIM);
            ++debug_depth;
            break;
        case UNIT__PRINTER_END:
            --debug_depth;
            fputs(debug_spaces(0), f);
            print_text(f, "}\n", UNIT_COLOR_DIM);
            break;
        case UNIT__PRINTER_ECHO:
            fputs(debug_spaces(0), f);
            fputs(icon(ICON_MSG), f);
            print_text(f, msg, UNIT_COLOR_COMMENT);
            fputc('\n', f);
            break;
        case UNIT__PRINTER_FAIL:
            fputs(debug_spaces(0), f);
            fputs("    Failed: ", f);
            fputs(beautify_name(unit->name), f);
            fputc('\n', f);

            fputs(debug_spaces(0), f);
            fputs("    ", f);
            fputs(msg, f);
            fputc('\n', f);
            break;
        case UNIT__PRINTER_ASSERTION: {
            fputs(debug_spaces(0), f);
            fputs(icon(unit->status), f);

            const char* cm = unit->assert_comment;
            const char* desc = (cm && cm[0] != '\0') ? cm : unit->assert_desc;
            fputs(desc, f);

            fputc('\n', f);
        }
            break;
        default:
            break;
    }
}

static const char* doctest_get_node_type(struct unit_test* node) {
    if (node->parent) {
        if(node->parent->parent) {
            return "SubCase";
        }
        return "TestCase";
    }
    return "TestSuite";
}

static void print_doctest_xml(int cmd, struct unit_test* node, const char* msg) {
    FILE* f = stdout;
    switch (cmd) {
        case UNIT__PRINTER_SETUP:
            fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
            // binary="/absolute/path/to/test/executable"
            fprintf(f, "<unit version=\"" UNIT_VERSION "\">\n");
            fprintf(f,
                    "  <Options order_by=\"file\" rand_seed=\"0\" first=\"0\" last=\"4294967295\" abort_after=\"0\" subcase_filter_levels=\"2147483647\" case_sensitive=\"false\" no_throw=\"false\" no_skip=\"false\"/>\n");
            ++def_depth;
            break;
        case UNIT__PRINTER_SHUTDOWN: {
            int total = 0;
            int passed = 0;
            for (struct unit_test* u = unit_tests; u; u = u->next) {
                passed += u->passed;
                total += u->total;
            }
            fprintf(f, "  <OverallResultsTestCases successes=\"%d\" failures=\"%d\" expectedFailures=\"%d\" />\n",
                    passed, total - passed, 0);
        }
            --def_depth;
            fprintf(f, "</unit>\n");
            fflush(f);
            break;
        case UNIT__PRINTER_BEGIN:
            fputs(unit__spaces(0), f);
            fprintf(f, "<%s name=\"%s\" filename=\"%s\" line=\"%d\" skipped=\"%s\">\n", doctest_get_node_type(node),
                    node->name, node->file,
                    node->line, node->status == UNIT_STATUS_SKIPPED ? "true" : "false");
            ++def_depth;
            break;
        case UNIT__PRINTER_END:
            --def_depth;
            fputs(unit__spaces(0), f);
            fprintf(f, "</%s>\n", doctest_get_node_type(node));
            break;
        case UNIT__PRINTER_ECHO:
            break;
        case UNIT__PRINTER_FAIL:
            fputs(unit__spaces(0), f);
            fprintf(f, "<Expression success=\"%s\" type=\"%s\" filename=\"%s\" line=\"%d\">\n",
                    node->status != UNIT_STATUS_FAILED ? "true" : "false", "REQUIRE", node->assert_file,
                    node->assert_line);
            fputs(unit__spaces(1), f);
            fprintf(f, "<Original>\n");
            fputs(unit__spaces(1), f);
            fprintf(f, "%s\n", node->assert_desc);
            fputs(unit__spaces(1), f);
            fprintf(f, "</Original>\n");
            fputs(unit__spaces(1), f);
            fprintf(f, "<Expanded>\n");
            fputs(unit__spaces(1), f);
            fprintf(f, "%s\n", node->assert_desc);
            fputs(unit__spaces(1), f);
            fprintf(f, "</Expanded>\n");
            fputs(unit__spaces(0), f);
            fprintf(f, "</Expression>\n");
            break;
        case UNIT__PRINTER_ASSERTION:
            break;
        default:
            break;
    }
}
