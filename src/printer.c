// region Цвета, текстовые сообщения и логи

#ifndef UNIT_NO_COLORS
#define UNIT_COLOR_RESET "\033[0m"
#define UNIT_COLOR_BOLD "\033[1m"
#define UNIT_COLOR_DIM "\033[2m"
#define UNIT_COLOR_UNDERLINE "\033[4m"
#define UNIT_COLOR_WHITE "\033[97m"
#define UNIT_COLOR_MAYBE "\033[35m"
#define UNIT_COLOR_COMMENT "\033[36m"
#define UNIT_COLOR_SUCCESS "\033[32m"
#define UNIT_COLOR_FAIL "\033[91m"
#define UNIT_COLOR_DESC "\033[33m"
#else
#define UNIT_COLOR_RESET
#define UNIT_COLOR_BOLD
#define UNIT_COLOR_DIM
#define UNIT_COLOR_UNDERLINE
#define UNIT_COLOR_WHITE
#define UNIT_COLOR_MAYBE
#define UNIT_COLOR_COMMENT
#define UNIT_COLOR_SUCCESS
#define UNIT_COLOR_FAIL
#define UNIT_COLOR_DESC
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
#define UNIT_MSG_CASE       "%s" UNIT__ICON_LI UNIT_COLOR_BOLD "%s" UNIT_COLOR_RESET ":\n"
#define UNIT_MSG_TEST       "%s" UNIT__ICON_RUN "%s" UNIT_COLOR_RESET "\n"
#define UNIT_MSG_OK         "%s" UNIT__ICON_OK UNIT__TXT_OK UNIT_COLOR_DIM "%s" UNIT_COLOR_RESET
#define UNIT_MSG_SKIP       "%s" UNIT__ICON_SKIP UNIT__TXT_SKIP UNIT_COLOR_DIM "%s" UNIT_COLOR_RESET
#define UNIT_MSG_FAIL       "%s" UNIT__ICON_FAIL UNIT__TXT_FAIL UNIT_COLOR_DIM "%s" UNIT_COLOR_RESET
#define UNIT_MSG_ECHO       "%s" UNIT__ICON_MSG UNIT_COLOR_COMMENT "%s" UNIT_COLOR_RESET "\n"

#define UNIT_MSG_RESULT_SKIP "%s" UNIT__ICON_SKIP UNIT__TXT_SKIP UNIT_COLOR_BOLD "%s: passed %d/%d tests" UNIT_COLOR_RESET "."
#define UNIT_MSG_RESULT_FAIL "%s" UNIT__ICON_FAIL UNIT__TXT_FAIL UNIT_COLOR_BOLD "%s: passed %d/%d tests" UNIT_COLOR_RESET "."
#define UNIT_MSG_RESULT_OK  "%s" UNIT__ICON_OK UNIT__TXT_OK UNIT_COLOR_BOLD "%s: passed %d/%d tests" UNIT_COLOR_RESET "."

#define UNIT_PRINTF(fmt, ...) printf(fmt, __VA_ARGS__)

#define UNIT__LOG_PREFIX "` "

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

static bool unit_prev_print_results = false;

void unit__on_begin(struct unit_test* unit) {
    if (unit->skip) {
        UNIT_PRINTF(UNIT_MSG_SKIP, unit__spaces(0), unit->name);
        unit__end_line(0.0);
    } else {
        if (unit->kind == 0) {
            putchar('\n');
            UNIT_PRINTF(UNIT_MSG_CASE, unit__spaces(0), unit->name);
        } else {
#ifdef UNIT_VERBOSE
            UNIT_PRINTF(UNIT__LOG_PREFIX UNIT_MSG_TEST, unit__log_prefix(0), unit->name);
#endif
        }
    }

    ++unit_depth;
    unit_prev_print_results = false;
}

void unit__on_end(struct unit_test* unit) {
    if (unit->kind == 1) {
        --unit_depth;
        if (!unit->skip) {
            UNIT_PRINTF(unit__status_msg(unit->status),
                        unit__spaces(0), unit->name);
            unit__end_line(unit->elapsed_time);
        }
        return;
    }

    // добавляем дополнительный отступ между блоками
    if (unit_prev_print_results) {
        putchar('\n');
    }
    --unit_depth;
    const char* result = 0;
    if (unit->skip) {

    } else if (unit->passed < unit->total) {
        result = UNIT_MSG_RESULT_FAIL;
    } else {
        result = UNIT_MSG_RESULT_OK;
    }
    if (result) {
#ifndef UNIT_VERBOSE
        if (unit_depth == 0)
#endif
        {
            UNIT_PRINTF(result, unit__spaces(0), unit->name, unit->passed, unit->total);
            unit__end_line(unit->elapsed_time);
            unit_prev_print_results = true;
        }
    }
}

void unit__on_fail(struct unit_test* unit, const char* msg) {
    UNIT_PRINTF("%s" UNIT__ICON_ASSERT UNIT_COLOR_BOLD UNIT_COLOR_FAIL "%s" UNIT_COLOR_RESET "\n\n",
                unit__spaces(0), unit_cur->name);
    UNIT_PRINTF("%s" "%s" "\n", unit__spaces(1), msg);
#ifndef UNIT_NO_FILEPOS
    UNIT_PRINTF(
            "%s" UNIT_COLOR_DIM "@ " UNIT_COLOR_RESET UNIT_COLOR_COMMENT UNIT_COLOR_UNDERLINE "%s" UNIT_COLOR_RESET "\n\n",
            unit__spaces(1), unit->assert_loc);
#endif
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

struct unit_printer unit_printer = (struct unit_printer) {
        .begin = unit__on_begin,
        .end = unit__on_end,
        .fail = unit__on_fail,
        .assertion = unit__on_assert,
        .echo = unit__on_echo
};
