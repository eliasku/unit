/**
 * unit.h - v0.0.2 - Simple header-only testing library for C - https://github.com/eliasku/unit
 *
 * Minimal example. Compile executable with `-D UNIT_TESTING` to enable tests.
 *
 * ```c
 * #define UNIT_IMPL
 * #include "unit.h"
 *
 * suite( you_should_define_the_suite_name_here ) {
 *   describe( optionally_add_the_subject ) {
 *     it( "describe the test behaviour" ) {
 *       check("use warn / check / require functions");
 *     }
 *   }
 * }
 * ```
 *
 **/

/**
 * Enable unit testing
 **/
// #define UNIT_TESTING

/**
 * Disable colorful output
 */
// #define UNIT_NO_COLORS

/**
 * Disable default `main` generation
 */
// #define UNIT_NO_MAIN

/**
 * Disable time measurements
 */
// #define UNIT_NO_TIME

/**
 * Add run details to the output (echo, assertions)
 */
// #define UNIT_VERBOSE
#ifndef UNIT_H
#define UNIT_H

#ifndef UNIT_TESTING

#define UNIT__CONCAT_(a, b) a ## b
#define UNIT__CONCAT(a, b) UNIT__CONCAT_(a, b)
#define UNIT_SUITE(Name, ...) __attribute__((unused)) static void UNIT__CONCAT(Name, __COUNTER__)(void)
#define UNIT_DESCRIBE(Name, ...) while(0)
#define UNIT_TEST(Description, ...) while(0)

#define UNIT_ECHO(...)

#define UNIT_WARN(x, ...)
#define UNIT_WARN_FALSE(x, ...)
#define UNIT_WARN_EQ(a, b, ...)
#define UNIT_WARN_NE(a, b, ...)
#define UNIT_WARN_GT(a, b, ...)
#define UNIT_WARN_GE(a, b, ...)
#define UNIT_WARN_LT(a, b, ...)
#define UNIT_WARN_LE(a, b, ...)

#define UNIT_CHECK(x, ...)
#define UNIT_CHECK_FALSE(x, ...)
#define UNIT_CHECK_EQ(a, b, ...)
#define UNIT_CHECK_NE(a, b, ...)
#define UNIT_CHECK_GT(a, b, ...)
#define UNIT_CHECK_GE(a, b, ...)
#define UNIT_CHECK_LT(a, b, ...)
#define UNIT_CHECK_LE(a, b, ...)

#define UNIT_REQUIRE(x, ...)
#define UNIT_REQUIRE_FALSE(x, ...)
#define UNIT_REQUIRE_EQ(a, b, ...)
#define UNIT_REQUIRE_NE(a, b, ...)
#define UNIT_REQUIRE_GT(a, b, ...)
#define UNIT_REQUIRE_GE(a, b, ...)
#define UNIT_REQUIRE_LT(a, b, ...)
#define UNIT_REQUIRE_LE(a, b, ...)

#define UNIT_SKIP()


#else

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    UNIT_STATUS_SUCCESS = 0,
    UNIT_STATUS_SKIPPED = 1,
    UNIT_STATUS_FAILED = 2
};

enum {
    UNIT__OP_TRUE = 0,
    UNIT__OP_FALSE = 1,
    UNIT__OP_EQ = 2,
    UNIT__OP_NE = 3,
    UNIT__OP_LT = 4,
    UNIT__OP_LE = 5,
    UNIT__OP_GT = 6,
    UNIT__OP_GE = 7,

    // не помечает тест неудачным, просто предупреждает
    UNIT__LEVEL_WARN = 0,
    // проваливает тест, но продолжает выполнять другие проверки
    UNIT__LEVEL_CHECK = 1,
    // проваливает тест и пропускает следующие проверки в текущем тесте
    UNIT__LEVEL_REQUIRE = 2,
};

struct unit_test {
    const char* name;
    const char* src;
    double t0;
    double elapsed_time;

    // 0 - group, 1 - test
    int kind;

    void (* fn)(void);

    struct unit_test* next;
    struct unit_test* parent;

    int total;
    int passed;
    bool allow_fail;
    bool skip;

    // test
    // status of current assertion
    int status;
    // state for this test scope
    // позволять ли дальше работать другим проверкам в рамках этого теста
    int state;
    const char* assert_comment;
    const char* assert_desc;
    const char* assert_loc;
    int assert_level;
};

extern struct unit_test* unit_tests;
extern struct unit_test* unit_cur;

struct unit_printer {
    void (* begin)(struct unit_test* unit);

    void (* end)(struct unit_test* unit);

    void (* fail)(struct unit_test* unit, const char* msg);

    void (* assertion)(struct unit_test* unit, int status);

    void (* echo)(const char* msg);
};

extern struct unit_printer unit_printer;

int unit__begin(struct unit_test* unit);

void unit__end(struct unit_test* unit);

int unit_main(int argc, char** argv);

// https://gcc.gnu.org/onlinedocs/cpp/Stringizing.html
#define UNIT__STR(x) #x
#define UNIT__X_STR(x) UNIT__STR(x)

#define UNIT__CONCAT(a, b) a ## b
#define UNIT__X_CONCAT(a, b) UNIT__CONCAT(a, b)

#define UNIT__FILEPOS __FILE__ ":" UNIT__X_STR(__LINE__)

#define UNIT__SCOPE_BODY(begin, end, Var) for (int Var = (begin, 0); !Var; ++Var, end)
#define UNIT_SCOPE(begin, end) UNIT__SCOPE_BODY(begin, end, UNIT__X_CONCAT(s__, __COUNTER__))

#define UNIT__TRY_BODY(begin, end, Var) for (int Var = (begin) ? 0 : (end, 1); !Var; ++Var, end)
#define UNIT_TRY_SCOPE(begin, end) UNIT__TRY_BODY(begin, end, UNIT__X_CONCAT(s__, __COUNTER__))

#define UNIT__SUITE(Var, Name, ...) \
    static void Var(void); \
    static struct unit_test UNIT__CONCAT(Var, _data) = \
    (struct unit_test) { .name = #Name, .kind = 0, .src = UNIT__FILEPOS, .fn = &Var, __VA_ARGS__ }; \
    __attribute__((constructor)) static void UNIT__CONCAT(Var, _ctor)(void) { \
        UNIT__CONCAT(Var, _data).next = unit_tests;    \
        unit_tests = &UNIT__CONCAT(Var, _data); \
    } \
    static void Var(void)

#define UNIT_SUITE(Name, ...) UNIT__SUITE(UNIT__X_CONCAT(unit__, __COUNTER__), Name, __VA_ARGS__)

#define UNIT__DECL(IsTest, Var, Name, ...) \
    static struct unit_test Var = (struct unit_test){.name = #Name, .kind = IsTest, .src = UNIT__FILEPOS, __VA_ARGS__}; \
    UNIT_TRY_SCOPE(unit__begin(&Var), unit__end(&Var))

#define UNIT_DESCRIBE(Name, ...) UNIT__DECL(0, UNIT__X_CONCAT(u__, __COUNTER__), Name, __VA_ARGS__)
#define UNIT_TEST(Name, ...) UNIT__DECL(1, UNIT__X_CONCAT(u__, __COUNTER__), Name, __VA_ARGS__)

bool unit__prepare_assert(int level, const char* loc, const char* comment, const char* desc);

// устанавливает читаемое описание проверки, в случае нормального состояния выполняет проверку,
// если установлено состояние пропускать тесты - НЕ ВЫЧИСЛЯЕТ аргументы для проверки
#define UNIT__ASSERT_LAZY(Assertion, Level, Comment, Description) \
if(unit__prepare_assert(Level, UNIT__FILEPOS, Comment, Description)) Assertion

#define UNIT__IS_TRUE(_, x) (!!(x))
#define UNIT__IS_NOT_EMPTY_STR(_, x) ((x) && (*(x) != '\0'))
#define UNIT__CMP(a, b) ((a) == (b) ? 0 : ((a) > (b) ? 1 : -1))
#define UNIT__STRCMP(a, b) ((a) == (b) ? 0 : strcmp((a), (b)))

#define UNIT__FOR_ASSERTS(macro) \
macro(int, intmax_t, %jd, UNIT__CMP, UNIT__IS_TRUE) \
macro(uint, uintmax_t, %ju, UNIT__CMP, UNIT__IS_TRUE) \
macro(dbl, long double, %Lg, UNIT__CMP, UNIT__IS_TRUE) \
macro(ptr, const void*, %p, UNIT__CMP, UNIT__IS_TRUE) \
macro(str, const char*, %s, UNIT__STRCMP, UNIT__IS_NOT_EMPTY_STR)

#define UNIT__DEFINE_ASSERT(Tag, Type, ...) \
void unit__assert_ ## Tag(Type a, Type b, int op, const char* expr, const char* sa, const char* sb);

UNIT__FOR_ASSERTS(UNIT__DEFINE_ASSERT)

#define UNIT__SELECT_ASSERT(x) \
    _Generic((x), \
        void*: unit__assert_ptr, \
        const void*: unit__assert_ptr, \
        char*: unit__assert_str, \
        const char*: unit__assert_str, \
        bool: unit__assert_int, \
        int: unit__assert_int, \
        long long: unit__assert_int, \
        unsigned int: unit__assert_uint, \
        unsigned long: unit__assert_uint, \
        unsigned long long: unit__assert_uint, \
        float: unit__assert_dbl, \
        double: unit__assert_dbl, \
        long double: unit__assert_dbl)

#define UNIT__ASSERT(Level, Op, a, b, Desc, ...)  UNIT__ASSERT_LAZY(UNIT__SELECT_ASSERT(b)(a, b, Op, Desc, #a, #b), Level, "" #__VA_ARGS__, Desc)

#define UNIT_WARN(x, ...)       UNIT__ASSERT(UNIT__LEVEL_WARN, UNIT__OP_TRUE,  0, x, "warn " #x, __VA_ARGS__)
#define UNIT_WARN_FALSE(x, ...) UNIT__ASSERT(UNIT__LEVEL_WARN, UNIT__OP_FALSE, 0, x, "warn " #x " is not true", __VA_ARGS__)
#define UNIT_WARN_EQ(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_WARN, UNIT__OP_EQ, a, b, "warn " #a " == " #b, __VA_ARGS__)
#define UNIT_WARN_NE(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_WARN, UNIT__OP_NE, a, b, "warn " #a " != " #b, __VA_ARGS__)
#define UNIT_WARN_GT(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_WARN, UNIT__OP_GT, a, b, "warn " #a " > " #b, __VA_ARGS__)
#define UNIT_WARN_GE(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_WARN, UNIT__OP_GE, a, b, "warn " #a " >= " #b, __VA_ARGS__)
#define UNIT_WARN_LT(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_WARN, UNIT__OP_LT, a, b, "warn " #a " < " #b, __VA_ARGS__)
#define UNIT_WARN_LE(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_WARN, UNIT__OP_LE, a, b, "warn " #a " <= " #b, __VA_ARGS__)

#define UNIT_CHECK(x, ...)       UNIT__ASSERT(UNIT__LEVEL_CHECK, UNIT__OP_TRUE,  0, x, "check " #x, __VA_ARGS__)
#define UNIT_CHECK_FALSE(x, ...) UNIT__ASSERT(UNIT__LEVEL_CHECK, UNIT__OP_FALSE, 0, x, "check " #x " is not true", __VA_ARGS__)
#define UNIT_CHECK_EQ(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_CHECK, UNIT__OP_EQ, a, b, "check " #a " == " #b, __VA_ARGS__)
#define UNIT_CHECK_NE(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_CHECK, UNIT__OP_NE, a, b, "check " #a " != " #b, __VA_ARGS__)
#define UNIT_CHECK_GT(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_CHECK, UNIT__OP_GT, a, b, "check " #a " > " #b, __VA_ARGS__)
#define UNIT_CHECK_GE(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_CHECK, UNIT__OP_GE, a, b, "check " #a " >= " #b, __VA_ARGS__)
#define UNIT_CHECK_LT(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_CHECK, UNIT__OP_LT, a, b, "check " #a " < " #b, __VA_ARGS__)
#define UNIT_CHECK_LE(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_CHECK, UNIT__OP_LE, a, b, "check " #a " <= " #b, __VA_ARGS__)

#define UNIT_REQUIRE(x, ...)       UNIT__ASSERT(UNIT__LEVEL_REQUIRE, UNIT__OP_TRUE,  0, x, "require " #x, __VA_ARGS__)
#define UNIT_REQUIRE_FALSE(x, ...) UNIT__ASSERT(UNIT__LEVEL_REQUIRE, UNIT__OP_FALSE, 0, x, "require " #x " is not true", __VA_ARGS__)
#define UNIT_REQUIRE_EQ(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_REQUIRE, UNIT__OP_EQ, a, b, "require " #a " == " #b, __VA_ARGS__)
#define UNIT_REQUIRE_NE(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_REQUIRE, UNIT__OP_NE, a, b, "require " #a " != " #b, __VA_ARGS__)
#define UNIT_REQUIRE_GT(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_REQUIRE, UNIT__OP_GT, a, b, "require " #a " > " #b, __VA_ARGS__)
#define UNIT_REQUIRE_GE(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_REQUIRE, UNIT__OP_GE, a, b, "require " #a " >= " #b, __VA_ARGS__)
#define UNIT_REQUIRE_LT(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_REQUIRE, UNIT__OP_LT, a, b, "require " #a " < " #b, __VA_ARGS__)
#define UNIT_REQUIRE_LE(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_REQUIRE, UNIT__OP_LE, a, b, "require " #a " <= " #b, __VA_ARGS__)

#define UNIT_SKIP() unit_cur->state |= UNIT__LEVEL_REQUIRE
#define UNIT_ECHO(msg) unit_printer.echo(msg)

#ifdef __cplusplus
}
#endif

#endif // UNIT_TESTING

#define suite(...) UNIT_SUITE(__VA_ARGS__)
#define describe(...) UNIT_DESCRIBE(__VA_ARGS__)
#define it(...) UNIT_TEST(__VA_ARGS__)
#define test(...) UNIT_TEST(__VA_ARGS__)
#define echo(...) UNIT_ECHO(__VA_ARGS__)

#define warn(...)       UNIT_WARN(__VA_ARGS__)
#define warn_false(...) UNIT_WARN_FALSE(__VA_ARGS__)
#define warn_eq(...)    UNIT_WARN_EQ(__VA_ARGS__)
#define warn_ne(...)    UNIT_WARN_NE(__VA_ARGS__)
#define warn_gt(...)    UNIT_WARN_GT(__VA_ARGS__)
#define warn_ge(...)    UNIT_WARN_GE(__VA_ARGS__)
#define warn_lt(...)    UNIT_WARN_LT(__VA_ARGS__)
#define warn_le(...)    UNIT_WARN_LE(__VA_ARGS__)

#define check(...)       UNIT_CHECK(__VA_ARGS__)
#define check_false(...) UNIT_CHECK_FALSE(__VA_ARGS__)
#define check_eq(...)    UNIT_CHECK_EQ(__VA_ARGS__)
#define check_ne(...)    UNIT_CHECK_NE(__VA_ARGS__)
#define check_gt(...)    UNIT_CHECK_GT(__VA_ARGS__)
#define check_ge(...)    UNIT_CHECK_GE(__VA_ARGS__)
#define check_lt(...)    UNIT_CHECK_LT(__VA_ARGS__)
#define check_le(...)    UNIT_CHECK_LE(__VA_ARGS__)

#define require(...)       UNIT_REQUIRE(__VA_ARGS__)
#define require_false(...) UNIT_REQUIRE_FALSE(__VA_ARGS__)
#define require_eq(...)    UNIT_REQUIRE_EQ(__VA_ARGS__)
#define require_ne(...)    UNIT_REQUIRE_NE(__VA_ARGS__)
#define require_gt(...)    UNIT_REQUIRE_GT(__VA_ARGS__)
#define require_ge(...)    UNIT_REQUIRE_GE(__VA_ARGS__)
#define require_lt(...)    UNIT_REQUIRE_LT(__VA_ARGS__)
#define require_le(...)    UNIT_REQUIRE_LE(__VA_ARGS__)

#define skip(...) UNIT_SKIP(__VA_ARGS__)


#endif // UNIT_H


#if defined(UNIT_IMPL) && !defined(UNIT_C_IMPLEMENTED) && defined(UNIT_TESTING)
#define UNIT_C_IMPLEMENTED
#ifdef __cplusplus
extern "C" {
#endif

#ifndef UNIT_NO_MAIN

int main(int argc, char** argv) {
    srand(time(NULL));
    return unit_main(argc, argv);
}

#endif // UNIT_NO_MAIN
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
        if(unit_depth == 0)
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
    UNIT_PRINTF(
            "%s" UNIT_COLOR_DIM "@ " UNIT_COLOR_RESET UNIT_COLOR_COMMENT UNIT_COLOR_UNDERLINE "%s" UNIT_COLOR_RESET "\n\n",
            unit__spaces(1), unit->assert_loc);
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


struct unit_test* unit_tests = NULL;
struct unit_test* unit_cur = NULL;

// region утилиты для вывода
const char* unit__vbprintf(const char* fmt, va_list args) {
    static char s_buffer[4096];
    vsnprintf(s_buffer, sizeof s_buffer, fmt, args);
    return s_buffer;
}

// endregion

const char* unit__op_expl[] = {
        " is true",
        " is false",
        " == ",
        " != ",
        " >= ",
        " > ",
        " <= ",
        " < ",
};

const char* unit__op_nexpl[] = {
        " is not true",
        " is not false",
        " != ",
        " == ",
        " < ",
        " <= ",
        " > ",
        " >= ",
};

bool unit__prepare_assert(int level, const char* loc, const char* comment, const char* desc) {
    unit_cur->assert_comment = comment;
    unit_cur->assert_desc = desc;
    unit_cur->assert_level = level;
    unit_cur->assert_loc = loc;
    if (unit_cur->state & UNIT__LEVEL_REQUIRE) {
        // пропустить проверку
        unit_printer.assertion(unit_cur, UNIT_STATUS_SKIPPED);
        return false;
    }
    return true;
}

static void unit__fail_impl(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    const char* msg = unit__vbprintf(fmt, args);
    va_end(args);

    if (unit_cur->assert_level > UNIT__LEVEL_WARN) {
        unit_cur->status = UNIT_STATUS_FAILED;
        unit_cur->state |= unit_cur->assert_level;
    }
    unit_printer.fail(unit_cur, msg);
}

#define UNIT__IMPLEMENT_ASSERT(Tag, Type, FormatType, BinaryOp, UnaryOp) \
void unit__assert_ ## Tag(Type a, Type b, int op, const char* expr, const char* sa, const char* sb) { \
    bool pass = false; \
    switch(op) {                                                       \
        case UNIT__OP_TRUE: pass = (UnaryOp(a, b)); break; \
        case UNIT__OP_FALSE: pass = !(UnaryOp(a, b)); break; \
        case UNIT__OP_EQ: pass = (BinaryOp(a, b)) == 0; break; \
        case UNIT__OP_NE: pass = (BinaryOp(a, b)) != 0; break; \
        case UNIT__OP_LT: pass = (BinaryOp(a, b)) < 0; break; \
        case UNIT__OP_LE: pass = (BinaryOp(a, b)) <= 0; break; \
        case UNIT__OP_GT: pass = (BinaryOp(a, b)) > 0; break; \
        case UNIT__OP_GE: pass = (BinaryOp(a, b)) >= 0; break; \
    } \
    unit_printer.assertion(unit_cur, pass ? UNIT_STATUS_SUCCESS : UNIT_STATUS_FAILED); \
    if (!pass) { \
        const char* expl = unit__op_expl[op];                 \
        const char* nexpl = unit__op_nexpl[op];                 \
        if (op < UNIT__OP_EQ) unit__fail_impl("Expected " UNIT_COLOR_SUCCESS "`%s`%s" UNIT_COLOR_RESET ", but got `" UNIT_COLOR_FAIL #FormatType "%s`" UNIT_COLOR_RESET, sb, expl, b, nexpl); \
        else unit__fail_impl("Expected " UNIT_COLOR_SUCCESS "`%s`%s`%s`" UNIT_COLOR_RESET ", but got " UNIT_COLOR_FAIL "`" #FormatType "%s" #FormatType "`" UNIT_COLOR_RESET, sa, expl, sb, a, nexpl, b); \
    } \
}

UNIT__FOR_ASSERTS(UNIT__IMPLEMENT_ASSERT)

/** Время **/

double unit__time(double prev) {
#ifndef UNIT_NO_TIME
    struct timespec ts = {0};
    bool success;
#ifdef _WIN32
    success = timespec_get(&ts, TIME_UTC) == TIME_UTC;
#else
    success = clock_gettime(CLOCK_REALTIME, &ts) == 0;
#endif
    if (!success) return 0.0;
    return (double) ts.tv_sec * 1000.0 + (double) ts.tv_nsec / 1000000.0 - prev;
#else // UNIT_NO_TIME
    return 0.0;
#endif // UNIT_NO_TIME
}

// region начало конец запуска каждого теста

int unit__begin(struct unit_test* unit) {
    unit->t0 = unit__time(0.0);
    unit->state = 0;
    unit->status = UNIT_STATUS_SUCCESS;
    unit->assert_desc = NULL;
    unit->parent = unit_cur;
    unit_cur = unit;
    unit_printer.begin(unit);

    if (unit->kind == 1) {
        for (struct unit_test* u = unit_cur; u; u = u->parent) {
            u->total++;
        }
    }

    if (unit->skip) {
        unit->status = UNIT_STATUS_SKIPPED;
    }

    return !unit->skip;
}

void unit__end(struct unit_test* unit) {
    if (unit->kind == 1) {
        const bool success = unit->status != UNIT_STATUS_FAILED;
        bool allow_fail = false;
        for (struct unit_test* u = unit_cur; u; u = u->parent) {
            allow_fail = allow_fail || u->allow_fail;
            if (success || allow_fail) {
                u->passed++;
            }
        }
    }
    unit->elapsed_time = unit__time(unit->t0);
    unit_printer.end(unit);
    unit_cur = unit->parent;
}

int unit_main(int argc, char** argv) {
    (void) (argc);
    (void) (argv);

    int failed = 0;
    for (struct unit_test* unit = unit_tests; unit; unit = unit->next) {
        UNIT_TRY_SCOPE(unit__begin(unit), unit__end(unit)) unit->fn();
        failed += (unit->passed < unit->total) ? 1 : 0;
    }

    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}

// endregion

#ifdef __cplusplus
}
#endif


#endif // => UNIT_C_IMPLEMENTED
