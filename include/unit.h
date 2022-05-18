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

/** Время **/

double unit__time(double prev) {
#ifndef UNIT_NO_TIME
    struct timespec ts = {0};
#ifdef _WIN32
    if (timespec_get(&ts, TIME_UTC) != TIME_UTC) {
        return 0.0;
    }
#else
    if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
        return 0.0;
    }
#endif
    return (double) ts.tv_sec * 1000.0 + (double) ts.tv_nsec / 1000000.0 - prev;
#else // UNIT_NO_TIME
    return 0.0;
#endif // UNIT_NO_TIME
}

/** Цвета, текстовые сообщения и логи **/

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

//#define UNIT_MSG_ECHO "%s💬 %s\n"
#define UNIT_MSG_ECHO "%s" UNIT_COLOR_BOLD UNIT_COLOR_COMMENT " ⃫ " UNIT_COLOR_RESET UNIT_COLOR_COMMENT "%s" UNIT_COLOR_RESET "\n"
//#define UNIT_MSG_ECHO "%s# %s\n"

#define UNIT_PRINTF(fmt, ...) printf(fmt, __VA_ARGS__)
#define UNIT_ECHO(msg) UNIT_PRINTF(UNIT_MSG_ECHO, unit__spaces(0), msg)

static char unit__tmp_buffer[1024];

const char* unit__vbprintf(const char* fmt, va_list args) {
    vsnprintf(unit__tmp_buffer, sizeof unit__tmp_buffer, fmt, args);
    return unit__tmp_buffer;
}

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
    UNIT__LEVEL_WARNING = 0,
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
};

static const char* unit_spaces[8] = {
        "",
        "  ",
        "    ",
        "      ",
        "        ",
        "          ",
        "            ",
        "              "
};
static int unit_depth = 0;

static const char* unit__spaces(int delta) {
    int i = unit_depth + delta;
    if (i < 0) i = 0;
    if (i > 7) i = 7;
    return unit_spaces[i];
}

static struct unit_test* unit_tests = NULL;
static struct unit_test* unit_cur = NULL;
static struct {
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
} unit_test_cur = {0};

static void unit_it_begin(const char* desc) {
    UNIT_PRINTF(UNIT_MSG_RUN, unit__spaces(0), desc);
    unit_test_cur.state = 0;
    unit_test_cur.status = UNIT_STATUS_SUCCESS;
    unit_test_cur.t0 = unit__time(0.0);
    unit_test_cur.desc = desc;
    unit_test_cur.assert_desc = NULL;
    for (struct unit_test* u = unit_cur; u; u = u->parent) {
        u->total++;
    }
    ++unit_depth;
}

static const char* unit_get_status_msg(int status) {
    switch (status) {
        case UNIT_STATUS_SUCCESS:
            return UNIT_MSG_SUCCESS;
        case UNIT_STATUS_SKIPPED:
            return UNIT_MSG_SKIPPED;
        case UNIT_STATUS_FAILED:
            return UNIT_MSG_FAILED;
        default:
            return "";
    }
}

static void unit_it_end(void) {
    const bool success = unit_test_cur.status == UNIT_STATUS_SUCCESS;
    bool allow_fail = false;
    for (struct unit_test* u = unit_cur; u; u = u->parent) {
        allow_fail = allow_fail || u->allow_fail;
        if (success || allow_fail) {
            u->passed++;
        }
    }
    double elapsed_time = unit__time(unit_test_cur.t0);
    UNIT_PRINTF(unit_get_status_msg(unit_test_cur.status), unit__spaces(0), unit_test_cur.desc,
                elapsed_time);

    --unit_depth;
}

// https://gcc.gnu.org/onlinedocs/cpp/Stringizing.html
#define UNIT__STR(x) #x
#define UNIT__X_STR(x) UNIT__STR(x)

#define UNIT__CONCAT(a, b) a ## b
#define UNIT__X_CONCAT(a, b) UNIT__CONCAT(a, b)

#define UNIT__FILEPOS __FILE__ ":" UNIT__X_STR(__LINE__)

#define UNIT__SCOPE_BODY(begin, end, Var) for (int Var = (begin, 0); !Var; ++Var, end)
#define UNIT_SCOPE(begin, end) UNIT__SCOPE_BODY(begin, end, UNIT__X_CONCAT(s__, __COUNTER__))

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
    UNIT_SCOPE(unit_begin(&Var), unit_end(&Var))

#define UNIT_DESCRIBE(Name, ...) UNIT__DESCRIBE(UNIT__X_CONCAT(u__, __COUNTER__), Name, __VA_ARGS__)

#define UNIT_IT(Description) UNIT_SCOPE(unit_it_begin(Description), unit_it_end())

static bool unit_prev_print_results = false;

static void unit_begin(struct unit_test* unit) {
    unit->parent = unit_cur;
    unit_cur = unit;
    unit->t0 = unit__time(0.0);

    UNIT_PRINTF(UNIT_MSG_TESTING, unit__spaces(0), unit->name);
    unit_prev_print_results = false;
    ++unit_depth;
}

static void unit_end(struct unit_test* unit) {
    const double elapsed_time = unit__time(unit->t0);
    // добавляем дополнительный отступ между блоками
    if (unit_prev_print_results) {
        putchar('\n');
    }
    --unit_depth;
    UNIT_PRINTF(UNIT_MSG_TEST_PASSED, unit__spaces(0), unit->name, unit->passed, unit->total, elapsed_time);
    unit_prev_print_results = true;

    unit_cur = unit->parent;
}

static int unit_main(int argc, char** argv) {
    (void) (argc);
    (void) (argv);

    int failed = 0;
    for (struct unit_test* unit = unit_tests; unit; unit = unit->next) {
        unit_begin(unit);
        unit->fn();
        unit_end(unit);
        if (unit->passed < unit->total) {
            ++failed;
        }
    }

    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}


/** Assert functions **/

void unit__fail_impl(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    const char* msg = unit__vbprintf(fmt, args);
    va_end(args);

    if (unit_test_cur.assert_level > UNIT__LEVEL_WARNING) {
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
        const char* desc = unit_test_cur.assert_comment[0] != '\0' ?
                           unit_test_cur.assert_comment :
                           unit_test_cur.assert_desc;
        UNIT_PRINTF(fmt, unit__spaces(0), desc);
    }
}

static bool unit__prepare_assert(int level, const char* loc, const char* comment, const char* desc) {
    unit_test_cur.assert_comment = comment;
    unit_test_cur.assert_desc = desc;
    unit_test_cur.assert_level = level;
    unit_test_cur.assert_loc = loc;
    if ((unit_test_cur.state & 2) != 2) {
        // выполнить проверку
        return true;
    }
    unit__print_assert(UNIT_STATUS_SKIPPED);
    return false;
}

// устанавливает читаемое описание проверки, в случае нормального состояния выполняет проверку,
// если установлено состояние пропускать тесты - НЕ ВЫЧИСЛЯЕТ аргументы для проверки
#define UNIT__ASSERT_LAZY(Assertion, Level, Comment, Description) \
if(unit__prepare_assert(Level, UNIT__FILEPOS, Comment, Description)) Assertion

#define UNIT__IS_TRUE(a, ...) (!!(a))
#define UNIT__IS_NOT_EMPTY_STR(a, ...) ((a) && *(a) != '\0')
#define UNIT__COMPARE_PRIMITIVE(a, b) ((a) == (b) ? 0 : ((a) > (b) ? 1 : -1))

#define UNIT__DECLARE_ASSERT(Tag, Type, FormatType, Cmp) \
static void unit__assert_ ## Tag(Type a, Type b, int op) { \
    bool pass = false; \
    const char* op_str = ""; \
    int res = Cmp(a, b); \
    switch(op) { \
        case UNIT__OP_TRUE: pass = res != 0; op_str = "is false"; break; \
        case UNIT__OP_FALSE: pass = res == 0; op_str = "is true"; break; \
        case UNIT__OP_EQ: pass = res == 0; op_str = "!="; break; \
        case UNIT__OP_NE: pass = res != 0; op_str = "=="; break; \
        case UNIT__OP_LT: pass = res < 0; op_str = ">="; break; \
        case UNIT__OP_LE: pass = res <= 0; op_str = ">"; break; \
        case UNIT__OP_GT: pass = res > 0; op_str = "<="; break; \
        case UNIT__OP_GE: pass = res >= 0; op_str = "<"; break; \
        default: break; \
    } \
    if(pass) unit__print_assert(UNIT_STATUS_SUCCESS); \
    else { \
        unit__print_assert(UNIT_STATUS_FAILED);                 \
        if(op < UNIT__OP_EQ)                                    \
            unit__fail_impl("Assertion failed: " #FormatType " %s", a, op_str); \
        else                                                    \
            unit__fail_impl("Assertion failed: " #FormatType " %s " #FormatType, a, op_str, b); \
    } \
}

UNIT__DECLARE_ASSERT(unary_int, intmax_t, %jd, UNIT__IS_TRUE)
UNIT__DECLARE_ASSERT(unary_uint, uintmax_t, %ju, UNIT__IS_TRUE)
UNIT__DECLARE_ASSERT(unary_dbl, long double, %Lg, UNIT__IS_TRUE)
UNIT__DECLARE_ASSERT(unary_ptr, const void*, %p, UNIT__IS_TRUE)
UNIT__DECLARE_ASSERT(unary_str, const char*, %s, UNIT__IS_NOT_EMPTY_STR)
UNIT__DECLARE_ASSERT(binary_int, intmax_t, %jd, UNIT__COMPARE_PRIMITIVE)
UNIT__DECLARE_ASSERT(binary_uint, uintmax_t, %ju, UNIT__COMPARE_PRIMITIVE)
UNIT__DECLARE_ASSERT(binary_dbl, long double, %Lg, UNIT__COMPARE_PRIMITIVE)
UNIT__DECLARE_ASSERT(binary_ptr, const void*, %p, UNIT__COMPARE_PRIMITIVE)
UNIT__DECLARE_ASSERT(binary_str, const char*, %s, strcmp)

#define UNIT__SELECT_ASSERT_UNARY(x) \
    _Generic((x), \
        void*: unit__assert_unary_ptr, \
        char*: unit__assert_unary_str, \
        bool: unit__assert_unary_int, \
        int: unit__assert_unary_int, \
        long long: unit__assert_unary_int, \
        unsigned int: unit__assert_unary_uint, \
        unsigned long: unit__assert_unary_uint, \
        unsigned long long: unit__assert_unary_uint, \
        float: unit__assert_unary_dbl, \
        double: unit__assert_unary_dbl, \
        long double: unit__assert_unary_dbl \
        )

#define UNIT__SELECT_ASSERT_BINARY(x) \
    _Generic((x), \
        void*: unit__assert_binary_ptr, \
        char*: unit__assert_binary_str, \
        int: unit__assert_binary_int, \
        long long: unit__assert_binary_int, \
        unsigned int: unit__assert_binary_uint, \
        unsigned long: unit__assert_binary_uint, \
        unsigned long long: unit__assert_binary_uint, \
        float: unit__assert_binary_dbl, \
        double: unit__assert_binary_dbl, \
        long double: unit__assert_binary_dbl \
        )

#define UNIT__UNARY_ASSERT(Level, Op, x, Desc, comments...)  UNIT__ASSERT_LAZY(UNIT__SELECT_ASSERT_UNARY(x)(x, 0, Op), Level, "" #comments, Desc)
#define UNIT__BINARY_ASSERT(Level, Op, a, b, Desc, comments...)  UNIT__ASSERT_LAZY(UNIT__SELECT_ASSERT_BINARY(b)(a, b, Op), Level, "" #comments, Desc)

#define UNIT_CHECK(x, ...)       UNIT__UNARY_ASSERT(UNIT__LEVEL_CHECK,  UNIT__OP_TRUE,  x, "check " #x, __VA_ARGS__)
#define UNIT_CHECK_FALSE(x, ...) UNIT__UNARY_ASSERT(UNIT__LEVEL_CHECK,  UNIT__OP_FALSE, x, "check " #x " is not true", __VA_ARGS__)
#define UNIT_CHECK_EQ(a, b, ...) UNIT__BINARY_ASSERT(UNIT__LEVEL_CHECK, UNIT__OP_EQ, a, b, "check " #a " == " #b, __VA_ARGS__)
#define UNIT_CHECK_NE(a, b, ...) UNIT__BINARY_ASSERT(UNIT__LEVEL_CHECK, UNIT__OP_NE, a, b, "check " #a " != " #b, __VA_ARGS__)
#define UNIT_CHECK_GT(a, b, ...) UNIT__BINARY_ASSERT(UNIT__LEVEL_CHECK, UNIT__OP_GT, a, b, "check " #a " > " #b, __VA_ARGS__)
#define UNIT_CHECK_GE(a, b, ...) UNIT__BINARY_ASSERT(UNIT__LEVEL_CHECK, UNIT__OP_GE, a, b, "check " #a " >= " #b, __VA_ARGS__)
#define UNIT_CHECK_LT(a, b, ...) UNIT__BINARY_ASSERT(UNIT__LEVEL_CHECK, UNIT__OP_LT, a, b, "check " #a " < " #b, __VA_ARGS__)
#define UNIT_CHECK_LE(a, b, ...) UNIT__BINARY_ASSERT(UNIT__LEVEL_CHECK, UNIT__OP_LE, a, b, "check " #a " <= " #b, __VA_ARGS__)

#define UNIT_REQUIRE(x, ...)       UNIT__UNARY_ASSERT(UNIT__LEVEL_REQUIRE,  UNIT__OP_TRUE,  x, "require " #x, __VA_ARGS__)
#define UNIT_REQUIRE_FALSE(x, ...) UNIT__UNARY_ASSERT(UNIT__LEVEL_REQUIRE,  UNIT__OP_FALSE, x, "require " #x " is not true", __VA_ARGS__)
#define UNIT_REQUIRE_EQ(a, b, ...) UNIT__BINARY_ASSERT(UNIT__LEVEL_REQUIRE, UNIT__OP_EQ, a, b, "require " #a " == " #b, __VA_ARGS__)
#define UNIT_REQUIRE_NE(a, b, ...) UNIT__BINARY_ASSERT(UNIT__LEVEL_REQUIRE, UNIT__OP_NE, a, b, "require " #a " != " #b, __VA_ARGS__)
#define UNIT_REQUIRE_GT(a, b, ...) UNIT__BINARY_ASSERT(UNIT__LEVEL_REQUIRE, UNIT__OP_GT, a, b, "require " #a " > " #b, __VA_ARGS__)
#define UNIT_REQUIRE_GE(a, b, ...) UNIT__BINARY_ASSERT(UNIT__LEVEL_REQUIRE, UNIT__OP_GE, a, b, "require " #a " >= " #b, __VA_ARGS__)
#define UNIT_REQUIRE_LT(a, b, ...) UNIT__BINARY_ASSERT(UNIT__LEVEL_REQUIRE, UNIT__OP_LT, a, b, "require " #a " < " #b, __VA_ARGS__)
#define UNIT_REQUIRE_LE(a, b, ...) UNIT__BINARY_ASSERT(UNIT__LEVEL_REQUIRE, UNIT__OP_LE, a, b, "require " #a " <= " #b, __VA_ARGS__)

#define UNIT_SKIP() unit_test_cur.state |= UNIT__LEVEL_REQUIRE

/** Main **/

#ifndef UNIT_NO_MAIN

int main(int argc, char** argv) {
    srand(time(NULL));
    return unit_main(argc, argv);
}

#endif // UNIT_NO_MAIN

#ifdef __cplusplus
}
#endif

// region Короткие формы записи

#define suite(...) UNIT_SUITE(__VA_ARGS__)
#define describe(...) UNIT_DESCRIBE(__VA_ARGS__)
#define it(...) UNIT_IT(__VA_ARGS__)
#define test(...) UNIT_IT(__VA_ARGS__)
#define echo(...) UNIT_ECHO(__VA_ARGS__)

#define require(...) UNIT_REQUIRE(__VA_ARGS__)
#define require_false(...) UNIT_REQUIRE_FALSE(__VA_ARGS__)
#define require_eq(...) UNIT_REQUIRE_EQ(__VA_ARGS__)
#define require_ne(...) UNIT_REQUIRE_NE(__VA_ARGS__)
#define require_gt(...) UNIT_REQUIRE_GT(__VA_ARGS__)
#define require_ge(...) UNIT_REQUIRE_GE(__VA_ARGS__)
#define require_lt(...) UNIT_REQUIRE_LT(__VA_ARGS__)
#define require_le(...) UNIT_REQUIRE_LE(__VA_ARGS__)

#define check(...) UNIT_CHECK(__VA_ARGS__)
#define check_false(...) UNIT_CHECK_FALSE(__VA_ARGS__)
#define check_eq(...) UNIT_CHECK_EQ(__VA_ARGS__)
#define check_ne(...) UNIT_CHECK_NE(__VA_ARGS__)
#define check_gt(...) UNIT_CHECK_GT(__VA_ARGS__)
#define check_ge(...) UNIT_CHECK_GE(__VA_ARGS__)
#define check_lt(...) UNIT_CHECK_LT(__VA_ARGS__)
#define check_le(...) UNIT_CHECK_LE(__VA_ARGS__)

#define skip(...) UNIT_SKIP(__VA_ARGS__)

// endregion

#endif // UNIT_H
