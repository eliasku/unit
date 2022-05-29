#include "fmemopen.c"

// TODO: remove test flag
//#define UNIT_ANIMATE


// region Цвета, текстовые сообщения и логи

#ifndef UNIT_NO_COLORS
#define UNIT_COLOR_RESET "\033[0m"
#define UNIT_COLOR_BOLD "\033[1m"
#define UNIT_COLOR_DIM "\033[2m"
#define UNIT_COLOR_UNDERLINE "\033[4m"
#define UNIT_COLOR_MAYBE "\033[35m"
#define UNIT_COLOR_COMMENT "\033[36m"
#define UNIT_COLOR_SUCCESS "\033[32m"
#define UNIT_COLOR_FAIL "\033[31m"
#define UNIT_COLOR_DESC "\033[33m"
#define UNIT_COLOR_LABEL_PASS UNIT_COLOR_BOLD "\033[30;42m"
#define UNIT_COLOR_LABEL_FAIL UNIT_COLOR_BOLD "\033[30;41m"
#define UNIT_COLOR_LABEL_SKIP UNIT_COLOR_BOLD "\033[30;47m"
#define UNIT_COLOR_LABEL_RUNS UNIT_COLOR_BOLD "\033[30;46m"
#else
#define UNIT_COLOR_RESET
#define UNIT_COLOR_BOLD
#define UNIT_COLOR_DIM
#define UNIT_COLOR_UNDERLINE
#define UNIT_COLOR_MAYBE
#define UNIT_COLOR_COMMENT
#define UNIT_COLOR_SUCCESS
#define UNIT_COLOR_FAIL
#define UNIT_COLOR_DESC
#define UNIT_COLOR_LABEL_PASS
#define UNIT_COLOR_LABEL_FAIL
#define UNIT_COLOR_LABEL_SKIP
#define UNIT_COLOR_LABEL_RUNS
#endif

/**
 * Alternative icons:
 * message icons: "💬 ", "# ", "ℹ ", " ⃫ "
 * skip icons: "Ⅱ "
 */
#define UNIT__ICON_OK       UNIT_COLOR_BOLD UNIT_COLOR_SUCCESS "✓ " UNIT_COLOR_RESET
#define UNIT__ICON_FAIL     UNIT_COLOR_BOLD UNIT_COLOR_FAIL "✕ " UNIT_COLOR_RESET
#define UNIT__ICON_ASSERT   UNIT_COLOR_BOLD UNIT_COLOR_FAIL "● " UNIT_COLOR_RESET
#define UNIT__ICON_RUN      UNIT_COLOR_BOLD UNIT_COLOR_SUCCESS "▶ " UNIT_COLOR_RESET
#define UNIT__ICON_SKIP     UNIT_COLOR_BOLD UNIT_COLOR_DIM "∅ " UNIT_COLOR_RESET
#define UNIT__ICON_MSG      UNIT_COLOR_BOLD UNIT_COLOR_COMMENT "» " UNIT_COLOR_RESET
//#define UNIT__ICON_LI       "◆ "
#define UNIT__ICON_LI

//#define UNIT__TXT_OK        UNIT_COLOR_BOLD UNIT_COLOR_SUCCESS "OK: " UNIT_COLOR_RESET
//#define UNIT__TXT_FAIL      UNIT_COLOR_BOLD UNIT_COLOR_FAIL "Fail: " UNIT_COLOR_RESET
//#define UNIT__TXT_SKIP      UNIT_COLOR_BOLD "Skip: " UNIT_COLOR_RESET

#define UNIT__TXT_OK
#define UNIT__TXT_FAIL
#define UNIT__TXT_SKIP

// $prefix 0 Status: $message
#define UNIT_MSG_CASE       "%s" UNIT__ICON_LI UNIT_COLOR_BOLD "%s" UNIT_COLOR_RESET "\n"
#define UNIT_MSG_TEST       "%s" UNIT__ICON_RUN "%s" UNIT_COLOR_RESET "\n"
#define UNIT_MSG_OK         "%s" UNIT__ICON_OK UNIT__TXT_OK UNIT_COLOR_DIM "%s" UNIT_COLOR_RESET
#define UNIT_MSG_SKIP       "%s" UNIT__ICON_SKIP UNIT__TXT_SKIP UNIT_COLOR_DIM "%s" UNIT_COLOR_RESET
#define UNIT_MSG_FAIL       "%s" UNIT__ICON_FAIL UNIT__TXT_FAIL UNIT_COLOR_DIM "%s" UNIT_COLOR_RESET
#define UNIT_MSG_ECHO       "%s" UNIT__ICON_MSG UNIT_COLOR_COMMENT "%s" UNIT_COLOR_RESET "\n"

#define UNIT_MSG_RESULT_SKIP "%s" UNIT__ICON_SKIP UNIT__TXT_SKIP UNIT_COLOR_BOLD "%s: passed %d/%d tests" UNIT_COLOR_RESET
#define UNIT_MSG_RESULT_FAIL "%s" UNIT__ICON_FAIL UNIT__TXT_FAIL UNIT_COLOR_BOLD "%s: passed %d/%d tests" UNIT_COLOR_RESET
#define UNIT_MSG_RESULT_OK  "%s" UNIT__ICON_OK UNIT__TXT_OK UNIT_COLOR_BOLD "%s: passed %d/%d tests" UNIT_COLOR_RESET

#define UNIT_PRINTF(fmt, ...) fprintf(stdout, fmt, __VA_ARGS__)
#define UNIT_PUTS(str) fputs(str, stdout)

#define UNIT__LOG_PREFIX "` "

// region reporting

#ifndef UNIT_ANIMATE
#define PRINT_STEP do{}while(0)
#else

static void unit__sleep(double seconds) {
    const long secs = (long) seconds;
    const long nanos = (long) ((seconds - (double) secs) * 1000000000.0);
    struct timespec ts = {secs, nanos};
    nanosleep(&ts, NULL);
}

#define PRINT_STEP do{fflush(stdout);unit__sleep(0.1);}while(0)
#endif // UNIT_ANIMATE

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
int unit_depth = 0;

const char* unit__spaces(int delta) {
    int i = unit_depth + delta;
    if (i < 0) i = 0;
    if (i > 7) i = 7;
    return unit_spaces[i];
}

const char* unit__log_prefix(int delta) {
    return unit__spaces(delta - 1);
}

const char* unit__status_msg(int status) {
    const char* dict[3] = {UNIT_MSG_OK,
                           UNIT_MSG_SKIP,
                           UNIT_MSG_FAIL};
    return dict[status];
}

void unit__end_line(double elapsed_time) {
    if (elapsed_time >= 0.01) {
        UNIT_PRINTF(UNIT_COLOR_DIM " (%0.2lf ms)" UNIT_COLOR_RESET, elapsed_time);
    }
    putchar('\n');
}

static const char* beautify_name(const char* name) {
    return UNIT__IS_NOT_EMPTY_STR(_, name) ? name : "(anonymous)";
}

void unit__on_begin(struct unit_test* unit) {
    if (!unit->parent) {
        UNIT_PRINTF(UNIT_COLOR_LABEL_RUNS " RUNS " UNIT_COLOR_RESET UNIT_COLOR_BOLD " %s " UNIT_COLOR_RESET,
                    beautify_name(unit->name));
        PRINT_STEP;
    }
}

static void print_node(struct unit_test* node) {
    PRINT_STEP;
    ++unit_depth;
    const char* name = beautify_name(node->name);
    UNIT_PUTS(unit__spaces(0));
    if (node->kind == 0) {
        if (node->skip) {
            UNIT_PRINTF(UNIT_COLOR_DIM "%s" UNIT_COLOR_RESET, name);
        } else {
            UNIT_PRINTF("%s", name);
        }
    } else {
        if (node->skip) {
            UNIT_PRINTF(UNIT_MSG_SKIP, "", name);
        } else {
            UNIT_PRINTF(unit__status_msg(node->status), "", name);
        }
    }
    unit__end_line(node->elapsed_time);
    for (struct unit_test* child = node->children; child; child = child->next) {
        print_node(child);
    }
    --unit_depth;
}

void unit__on_end(struct unit_test* unit) {
    if (unit->parent) {
        if (unit->kind == 1) {
            if (unit->status == UNIT_STATUS_SKIPPED) {
                UNIT_PUTS(UNIT__ICON_SKIP);
            } else if (unit->status == UNIT_STATUS_FAILED) {
                UNIT_PUTS(UNIT__ICON_FAIL);
            } else if (unit->status == UNIT_STATUS_SUCCESS) {
                UNIT_PUTS(UNIT__ICON_OK);
            }
            PRINT_STEP;
        }
        return;
    }
    // go back to the beginning of line
    UNIT_PUTS("\n\033[1A\033[999D");
    const char* name = beautify_name(unit->name);
    if (unit->skip) {
        UNIT_PRINTF(UNIT_COLOR_LABEL_SKIP " SKIP " UNIT_COLOR_RESET UNIT_COLOR_BOLD " %s" UNIT_COLOR_RESET, name);
    } else if (unit->passed < unit->total) {
        UNIT_PRINTF(
                UNIT_COLOR_LABEL_FAIL " FAIL " UNIT_COLOR_RESET UNIT_COLOR_BOLD " %s " UNIT_COLOR_RESET ": passed %d/%d tests",
                name, unit->passed, unit->total);
    } else {
        UNIT_PRINTF(
                UNIT_COLOR_LABEL_PASS " PASS " UNIT_COLOR_RESET UNIT_COLOR_BOLD " %s" UNIT_COLOR_RESET ": passed %d/%d tests",
                name, unit->passed, unit->total);
    }
    unit__end_line(unit->elapsed_time);

    if (unit__fails) {
        print_node(unit->children);
        putchar('\n');

        const long pos = ftell(unit__fails);
        unit__fails_mem[pos] = 0;
        fclose(unit__fails);
        unit__fails = 0;
        UNIT_PUTS(unit__fails_mem);

        putchar('\n');
    }
}

static void unit__breadcrumbs(struct unit_test* test) {
    if (test->parent) {
        unit__breadcrumbs(test->parent);

        // skip file's root node
        if (test->parent->parent) {
            fputs(" → ", unit__fails);
        }
        fputs(beautify_name(test->name), unit__fails);
    }
}

void unit__on_fail(struct unit_test* unit, const char* msg) {
    const int indent = 1 - unit_depth;
    if (unit__fails == 0) {
        // TODO: change to `tmpfile` ?
        unit__fails = fmemopen(unit__fails_mem, sizeof unit__fails_mem, "w");
    }
    fputs(unit__spaces(indent), unit__fails);
    fputs(UNIT__ICON_ASSERT UNIT_COLOR_BOLD UNIT_COLOR_FAIL, unit__fails);
    unit__breadcrumbs(unit_cur);
    fputs(UNIT_COLOR_RESET "\n\n", unit__fails);

    fputs(unit__spaces(indent + 1), unit__fails);
    fputs(msg, unit__fails);
    fputs("\n", unit__fails);
#ifndef UNIT_NO_FILEPOS
    fprintf(unit__fails,
            "%s" UNIT_COLOR_DIM "@ " UNIT_COLOR_RESET UNIT_COLOR_COMMENT UNIT_COLOR_UNDERLINE "%s" UNIT_COLOR_RESET "\n",
            unit__spaces(indent + 1), unit->assert_loc);
#endif
    fputc('\n', unit__fails);
}

void unit__on_assert(struct unit_test* unit, int status) {
#ifdef UNIT_VERBOSE
    static const char* fmts[3] = {
            UNIT__LOG_PREFIX "%s" UNIT__ICON_OK UNIT__TXT_OK "%s\n",
            UNIT__LOG_PREFIX "%s" UNIT__ICON_SKIP UNIT__TXT_SKIP "%s\n",
            UNIT__LOG_PREFIX "%s" UNIT__ICON_FAIL UNIT__TXT_FAIL "%s\n"
    };
    const char* cm = unit->assert_comment;
    const char* desc = (cm && cm[0] != '\0') ? cm : unit->assert_desc;
    UNIT_PRINTF(fmts[status], unit__log_prefix(0), desc);
#endif
}

void unit__on_echo(const char* msg) {
#ifdef UNIT_VERBOSE
    UNIT_PRINTF(UNIT__LOG_PREFIX UNIT_MSG_ECHO, unit__log_prefix(0), msg);
#endif
}

// endregion reporting

static void print_header(void) {
    UNIT_PUTS("\n" UNIT_COLOR_LABEL_PASS " ✓ηỉτ " UNIT_COLOR_LABEL_FAIL " v" UNIT_VERSION " " UNIT_COLOR_RESET "\n\n");
    PRINT_STEP;
}

struct unit_printer unit_printer = (struct unit_printer) {
        .setup = print_header,
        .begin = unit__on_begin,
        .end = unit__on_end,
        .fail = unit__on_fail,
        .assertion = unit__on_assert,
        .echo = unit__on_echo
};
