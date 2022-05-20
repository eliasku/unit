#ifndef UNIT_H
#define UNIT_H

// отключить вывод в цвете
//#define UNIT_NO_COLORS

// отключить точку входа по-умолчанию
//#define UNIT_NO_MAIN

// отключить измерение времени
//#define UNIT_NO_TIME

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

extern const char* unit__msg_echo;

#define UNIT_PRINTF(fmt, ...) printf(fmt, __VA_ARGS__)
#define UNIT_ECHO(msg) UNIT_PRINTF(unit__msg_echo, unit__spaces(0), msg)

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

    void (* fn)(void);

    struct unit_test* next;
    struct unit_test* parent;

    int total;
    int passed;
    bool allow_fail;
    bool skip;
};

extern int unit_depth;

const char* unit__spaces(int delta);

struct unit_test_state {
    // status of current assertion
    int status;
    // state for this test scope
    // позволять ли дальше работать другим проверкам в рамках этого теста
    int state;
    double t0;
    const char* desc;
    const char* assert_comment;
    const char* assert_desc;
    const char* assert_loc;
    int assert_level;
};

struct unit__it_flags {
    bool skip;
};

extern struct unit_test* unit_tests;
extern struct unit_test* unit_cur;
extern struct unit_test_state unit_test_cur;

int unit_it_begin(const char* desc, struct unit__it_flags flags);

void unit_it_end(void);

int unit_begin(struct unit_test* unit);

void unit_end(struct unit_test* unit);

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
    void UNIT__CONCAT(Var, _main) (void); \
    static struct unit_test UNIT__CONCAT(Var, _data) = \
    (struct unit_test) { .name = #Name, .src = UNIT__FILEPOS, .fn = &UNIT__CONCAT(Var, _main), __VA_ARGS__ }; \
    __attribute__((constructor)) void UNIT__CONCAT(Var, _ctor)(void) { \
        UNIT__CONCAT(Var, _data).next = unit_tests;    \
        unit_tests = &UNIT__CONCAT(Var, _data); \
    } \
    void UNIT__CONCAT(Var, _main) (void)

#define UNIT_SUITE(Name, ...) UNIT__SUITE(UNIT__X_CONCAT(UNIT__CONCAT(unit__, Name), __COUNTER__), Name, __VA_ARGS__)

#define UNIT__DESCRIBE(Var, Name, ...) \
    static struct unit_test Var = (struct unit_test){.name = #Name, .src = UNIT__FILEPOS, __VA_ARGS__}; \
    UNIT_TRY_SCOPE(unit_begin(&Var), unit_end(&Var))

#define UNIT_DESCRIBE(Name, ...) UNIT__DESCRIBE(UNIT__X_CONCAT(u__, __COUNTER__), Name, __VA_ARGS__)

#define UNIT_IT(Description, ...) UNIT_TRY_SCOPE(unit_it_begin(Description, (struct unit__it_flags){__VA_ARGS__}), unit_it_end())

/** Assert functions **/

void unit__fail_impl(const char* fmt, ...);

void unit__print_assert(int status);

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
        char*: unit__assert_str, \
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

#define UNIT_SKIP() unit_test_cur.state |= UNIT__LEVEL_REQUIRE

#ifdef __cplusplus
}
#endif

// region Короткие формы записи

#define suite(...) UNIT_SUITE(__VA_ARGS__)
#define describe(...) UNIT_DESCRIBE(__VA_ARGS__)
#define it(...) UNIT_IT(__VA_ARGS__)
#define test(...) UNIT_IT(__VA_ARGS__)
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

// endregion

#endif // UNIT_H

#ifdef UNIT_IMPL

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
const char* unit__msg_echo = "%s" UNIT_COLOR_BOLD UNIT_COLOR_COMMENT " ⃫ " UNIT_COLOR_RESET UNIT_COLOR_COMMENT "%s" UNIT_COLOR_RESET "\n";

enum {
    UNIT_STATUS_SUCCESS = 0,
    UNIT_STATUS_SKIPPED = 1,
    UNIT_STATUS_FAILED = 2
};

// endregion

#ifdef __cplusplus
extern "C" {
#endif

#ifndef UNIT_NO_MAIN

int main(int argc, char** argv) {
    srand(time(NULL));
    return unit_main(argc, argv);
}

#endif // UNIT_NO_MAIN

struct unit_test* unit_tests = NULL;
struct unit_test* unit_cur = NULL;
struct unit_test_state unit_test_cur = {0};

// region утилиты для вывода
const char* unit__vbprintf(const char* fmt, va_list args) {
    static char s_buffer[4096];
    vsnprintf(s_buffer, sizeof s_buffer, fmt, args);
    return s_buffer;
}

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
    unit__print_assert(pass ? UNIT_STATUS_SUCCESS : UNIT_STATUS_FAILED); \
    if (!pass) { \
        const char* expl = unit__op_expl[op];                 \
        const char* nexpl = unit__op_nexpl[op];                 \
        if (op < UNIT__OP_EQ) unit__fail_impl("Expected `%s`%s, but got `" #FormatType "%s`", sb, expl, b, nexpl); \
        else unit__fail_impl("Expected `%s`%s`%s`, but got `" #FormatType "%s" #FormatType "`", sa, expl, sb, a, nexpl, b); \
    } \
}

UNIT__FOR_ASSERTS(UNIT__IMPLEMENT_ASSERT)

void unit__fail_impl(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    const char* msg = unit__vbprintf(fmt, args);
    va_end(args);

    if (unit_test_cur.assert_level > UNIT__LEVEL_WARN) {
        unit_test_cur.status = UNIT_STATUS_FAILED;
        unit_test_cur.state |= unit_test_cur.assert_level;
    }
    UNIT_PRINTF("%s%s\n"
                "%sin %s (%s : %s)\n",
                unit__spaces(1), msg,
                unit__spaces(1), unit_test_cur.assert_loc, unit_cur->name, unit_test_cur.desc
    );
}

void unit__print_assert(int status) {
    const char* fmt = NULL;
    if (status == UNIT_STATUS_SKIPPED) {
        fmt = "%sⅡ Skip: %s\n";
    } else if (status == UNIT_STATUS_SUCCESS) {
        fmt = "%s" UNIT_COLOR_BOLD UNIT_COLOR_SUCCESS "✓ " UNIT_COLOR_RESET UNIT_COLOR_SUCCESS "Pass: " UNIT_COLOR_RESET "%s\n";
    } else if (status == UNIT_STATUS_FAILED) {
        fmt = "%s" UNIT_COLOR_BOLD UNIT_COLOR_FAIL "✕ " UNIT_COLOR_RESET UNIT_COLOR_FAIL "Failed: " UNIT_COLOR_RESET "%s\n";
    }
    if (fmt) {
        const char* cm = unit_test_cur.assert_comment;
        const char* desc = (cm && cm[0] != '\0') ? cm :
                           unit_test_cur.assert_desc;
        UNIT_PRINTF(fmt, unit__spaces(0), desc);
    }
}

bool unit__prepare_assert(int level, const char* loc, const char* comment, const char* desc) {
    unit_test_cur.assert_comment = comment;
    unit_test_cur.assert_desc = desc;
    unit_test_cur.assert_level = level;
    unit_test_cur.assert_loc = loc;
    if (unit_test_cur.state & UNIT__LEVEL_REQUIRE) {
        // пропустить проверку
        unit__print_assert(UNIT_STATUS_SKIPPED);
        return false;
    }
    return true;
}


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

int unit_it_begin(const char* desc, struct unit__it_flags flags) {
    UNIT_PRINTF(UNIT_MSG_RUN, unit__spaces(0), desc);
    unit_test_cur.state = 0;
    unit_test_cur.status = UNIT_STATUS_SUCCESS;
    unit_test_cur.t0 = unit__time(0.0);
    unit_test_cur.desc = desc;
    unit_test_cur.assert_desc = NULL;

    ++unit_depth;

    for (struct unit_test* u = unit_cur; u; u = u->parent) {
        u->total++;
    }

    if (flags.skip) {
        unit_test_cur.status = UNIT_STATUS_SKIPPED;
    }

    return !flags.skip;
}

const char* unit__status_msg(int status) {
    const char* dict[3] = {UNIT_MSG_SUCCESS,
                           UNIT_MSG_SKIPPED,
                           UNIT_MSG_FAILED};
    return dict[status];
}

void unit_it_end(void) {
    const bool success = unit_test_cur.status != UNIT_STATUS_FAILED;
    bool allow_fail = false;
    for (struct unit_test* u = unit_cur; u; u = u->parent) {
        allow_fail = allow_fail || u->allow_fail;
        if (success || allow_fail) {
            u->passed++;
        }
    }
    double elapsed_time = unit__time(unit_test_cur.t0);
    UNIT_PRINTF(unit__status_msg(unit_test_cur.status), unit__spaces(0), unit_test_cur.desc,
                elapsed_time);

    --unit_depth;
}

static bool unit_prev_print_results = false;

int unit_begin(struct unit_test* unit) {
    unit->parent = unit_cur;
    unit_cur = unit;
    unit->t0 = unit__time(0.0);

    UNIT_PRINTF(UNIT_MSG_TESTING, unit__spaces(0), unit->name);
    unit_prev_print_results = false;
    ++unit_depth;

    return !unit->skip;
}

void unit_end(struct unit_test* unit) {
    const double elapsed_time = unit__time(unit->t0);
    // добавляем дополнительный отступ между блоками
    if (unit_prev_print_results) {
        putchar('\n');
    }
    --unit_depth;
    if (unit->skip) {
        UNIT_PRINTF(UNIT_MSG_SKIPPED, unit__spaces(0), unit->name, elapsed_time);
    } else {
        UNIT_PRINTF(UNIT_MSG_TEST_PASSED, unit__spaces(0), unit->name, unit->passed, unit->total, elapsed_time);
    }
    unit_prev_print_results = true;

    unit_cur = unit->parent;
}

int unit_main(int argc, char** argv) {
    (void) (argc);
    (void) (argv);

    int failed = 0;
    for (struct unit_test* unit = unit_tests; unit; unit = unit->next) {
        UNIT_TRY_SCOPE(unit_begin(unit), unit_end(unit)) unit->fn();
        failed += (unit->passed < unit->total) ? 1 : 0;
    }

    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}

// endregion

#ifdef __cplusplus
}
#endif

#endif // UNIT_IMPL
