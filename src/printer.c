

// region Цвета, текстовые сообщения и логи

#ifndef UNIT_NO_COLORS
#define UNIT_COLOR_RESET "\033[0m"
#define UNIT_COLOR_BOLD "\033[1m"
#define UNIT_COLOR_WHITE "\033[97m"
#define UNIT_COLOR_MAYBE "\033[35m"
#define UNIT_COLOR_COMMENT "\033[36m"
#define UNIT_COLOR_SUCCESS "\033[92m"
#define UNIT_COLOR_FAIL "\033[91m"
#define UNIT_COLOR_DESC "\033[33m"
#else
#define UNIT_COLOR_RESET
#define UNIT_COLOR_BOLD
#define UNIT_COLOR_WHITE
#define UNIT_COLOR_MAYBE
#define UNIT_COLOR_COMMENT
#define UNIT_COLOR_SUCCESS
#define UNIT_COLOR_FAIL
#define UNIT_COLOR_DESC
#endif

// $prefix 0 Status: $message
#define UNIT_MSG_TESTING "\n%s" "◆ " UNIT_COLOR_BOLD UNIT_COLOR_WHITE "Testing %s" UNIT_COLOR_RESET ":\n"
#define UNIT_MSG_RUN "%s" UNIT_COLOR_BOLD UNIT_COLOR_SUCCESS "▶ " UNIT_COLOR_RESET UNIT_COLOR_BOLD UNIT_COLOR_WHITE "%s" UNIT_COLOR_RESET "\n"
#define UNIT_MSG_SUCCESS "%s" UNIT_COLOR_BOLD UNIT_COLOR_SUCCESS "✓ " UNIT_COLOR_RESET UNIT_COLOR_SUCCESS "Success: " UNIT_COLOR_RESET UNIT_COLOR_BOLD UNIT_COLOR_DESC "%s" UNIT_COLOR_RESET " (%0.2lf ms)\n"
#define UNIT_MSG_SKIPPED "%s" UNIT_COLOR_BOLD "Ⅱ " UNIT_COLOR_RESET "Skipped: " UNIT_COLOR_RESET UNIT_COLOR_BOLD UNIT_COLOR_DESC "%s" UNIT_COLOR_RESET " (%0.2lf ms)\n"
#define UNIT_MSG_FAILED "%s" UNIT_COLOR_BOLD UNIT_COLOR_FAIL "✕ " UNIT_COLOR_RESET UNIT_COLOR_FAIL "Failed: " UNIT_COLOR_RESET UNIT_COLOR_BOLD UNIT_COLOR_DESC "%s" UNIT_COLOR_RESET " (%0.2lf ms)\n"

#define UNIT_MSG_TEST_PASSED "%s" UNIT_COLOR_BOLD UNIT_COLOR_WHITE "%s: Passed %d/%d tests" UNIT_COLOR_RESET ". (%0.2lf ms)\n"

//#define UNIT_MSG_ECHO "💬"
//#define UNIT_MSG_ECHO "#"
#define UNIT_MSG_ECHO "%s" UNIT_COLOR_BOLD UNIT_COLOR_COMMENT " ⃫ " UNIT_COLOR_RESET UNIT_COLOR_COMMENT "%s" UNIT_COLOR_RESET "\n"

#define UNIT_PRINTF(fmt, ...) printf(fmt, __VA_ARGS__)

// region reporting

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

const char* unit__status_msg(int status) {
    const char* dict[3] = {UNIT_MSG_SUCCESS,
                           UNIT_MSG_SKIPPED,
                           UNIT_MSG_FAILED};
    return dict[status];
}

static bool unit_prev_print_results = false;

void unit__on_begin(struct unit_test* unit) {
    const char* fmt = unit->kind == 0 ? UNIT_MSG_TESTING : UNIT_MSG_RUN;
    UNIT_PRINTF(fmt, unit__spaces(0), unit->name);
    ++unit_depth;
    unit_prev_print_results = false;
}

void unit__on_end(struct unit_test* unit) {
    if (unit->kind == 1) {
        UNIT_PRINTF(unit__status_msg(unit->status),
                    unit__spaces(0), unit->name,
                    unit->elapsed_time);
        --unit_depth;
        return;
    }

    // добавляем дополнительный отступ между блоками
    if (unit_prev_print_results) {
        putchar('\n');
    }
    --unit_depth;
    if (unit->skip) {
        UNIT_PRINTF(UNIT_MSG_SKIPPED, unit__spaces(0), unit->name, unit->elapsed_time);
    } else {
        UNIT_PRINTF(UNIT_MSG_TEST_PASSED, unit__spaces(0), unit->name, unit->passed, unit->total, unit->elapsed_time);
    }
    unit_prev_print_results = true;
}

void unit__on_fail(struct unit_test* unit, const char* msg) {
    UNIT_PRINTF("%s%s\n"
                "%sin %s (%s)\n",
                unit__spaces(1), msg,
                unit__spaces(1), unit->assert_loc, unit_cur->name);
}

void unit__on_assert(struct unit_test* unit, int status) {
    const char* fmt = NULL;
    if (status == UNIT_STATUS_SKIPPED) {
        fmt = "%sⅡ Skip: %s\n";
    } else if (status == UNIT_STATUS_SUCCESS) {
        fmt = "%s" UNIT_COLOR_BOLD UNIT_COLOR_SUCCESS "✓ " UNIT_COLOR_RESET UNIT_COLOR_SUCCESS "Pass: " UNIT_COLOR_RESET "%s\n";
    } else if (status == UNIT_STATUS_FAILED) {
        fmt = "%s" UNIT_COLOR_BOLD UNIT_COLOR_FAIL "✕ " UNIT_COLOR_RESET UNIT_COLOR_FAIL "Failed: " UNIT_COLOR_RESET "%s\n";
    }
    if (fmt) {
        const char* cm = unit->assert_comment;
        const char* desc = (cm && cm[0] != '\0') ? cm : unit->assert_desc;
        UNIT_PRINTF(fmt, unit__spaces(0), desc);
    }
}

void unit__on_echo(const char* msg) {
    UNIT_PRINTF(UNIT_MSG_ECHO, unit__spaces(0), msg);
}

// endregion reporting

struct unit_printer unit_printer = (struct unit_printer) {
        .begin = unit__on_begin,
        .end = unit__on_end,
        .fail = unit__on_fail,
        .assertion = unit__on_assert,
        .echo = unit__on_echo
};
