#ifndef UNIT_H
#define UNIT_H

#ifndef UNIT_TESTING

#include "unit-disabled.h"

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
    UNIT_STATUS_RUN = 0,
    UNIT_STATUS_SUCCESS = 1,
    UNIT_STATUS_SKIPPED = 2,
    UNIT_STATUS_FAILED = 3
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

    // types
    UNIT__TYPE_CASE = 0,
    UNIT__TYPE_TEST = 1,

    // printer commands
    UNIT__PRINTER_SETUP = 0,
    UNIT__PRINTER_SHUTDOWN = 1,
    UNIT__PRINTER_BEGIN = 2,
    UNIT__PRINTER_END = 3,
    UNIT__PRINTER_ECHO = 4,
    UNIT__PRINTER_FAIL = 5,
    UNIT__PRINTER_ASSERTION = 6,
};

struct unit__options {
    bool dummy__;
    bool failing;
    bool skip;
};

struct unit_test {
    const char* name;
    const char* file;
    int line;

    void (* fn)(void);

    int type;
    struct unit__options options;

    double t0;
    double elapsed;

    struct unit_test* next;
    struct unit_test* children;
    struct unit_test* parent;

    int total;
    int passed;

    // test
    // status of current assertion
    int status;
    // state for this test scope
    // позволять ли дальше работать другим проверкам в рамках этого теста
    int state;
    const char* assert_comment;
    const char* assert_desc;
    const char* assert_file;
    int assert_line;
    int assert_level;
    int assert_status;
};

extern struct unit_test* unit_tests;
extern struct unit_test* unit_cur;

struct unit_run_options {
    int version;
    int help;
    int list;
    int ascii;
    int trace;
    int quiet;
    int animate;
    int doctest_xml;
    int short_filenames;
    unsigned seed;
    const char* program;
};

extern struct unit_run_options unit__opts;

struct unit_printer {
    void (* callback)(int cmd, struct unit_test* unit, const char* msg);

    struct unit_printer* next;
};

int unit__begin(struct unit_test* unit);

void unit__end(struct unit_test* unit);

void unit__echo(const char* msg);

int unit_main(struct unit_run_options options);

// https://gcc.gnu.org/onlinedocs/cpp/Stringizing.html
#define UNIT__STR(x) #x
#define UNIT__X_STR(x) UNIT__STR(x)

#define UNIT__CONCAT(a, b) a ## b
#define UNIT__X_CONCAT(a, b) UNIT__CONCAT(a, b)

#define UNIT__SCOPE_BODY(begin, end, Var) for (int Var = (begin, 0); !Var; ++Var, end)
#define UNIT_SCOPE(begin, end) UNIT__SCOPE_BODY(begin, end, UNIT__X_CONCAT(s__, __COUNTER__))

#define UNIT__TRY_BODY(begin, end, Var) for (int Var = (begin) ? 0 : (end, 1); !Var; ++Var, end)
#define UNIT_TRY_SCOPE(begin, end) UNIT__TRY_BODY(begin, end, UNIT__X_CONCAT(s__, __COUNTER__))

#define UNIT__SUITE(Var, Name, ...) \
    static void Var(void); \
    __attribute__((constructor)) static void UNIT__CONCAT(Var, _ctor)(void) { \
        static struct unit_test u = (struct unit_test){ .name=Name, .file=__FILE__, .line=__LINE__, .fn=Var, .type=UNIT__TYPE_CASE, .options=(struct unit__options){ __VA_ARGS__ } }; \
        u.next = unit_tests; unit_tests = &u; \
    } \
    static void Var(void)

#define UNIT_SUITE_(Name, ...) UNIT__SUITE(UNIT__X_CONCAT(unit__, __COUNTER__), Name, __VA_ARGS__)
#define UNIT_SUITE(Name, ...) UNIT__SUITE(UNIT__X_CONCAT(unit__, __COUNTER__), #Name, __VA_ARGS__)

#define UNIT__DECL(Type, Var, Name, ...) \
    static struct unit_test Var = (struct unit_test){ .name=Name, .file=__FILE__, .line=__LINE__, .fn=NULL, .type=Type, .options=(struct unit__options){ __VA_ARGS__ } }; \
    UNIT_TRY_SCOPE(unit__begin(&Var), unit__end(&Var))

#define UNIT_DESCRIBE(Name, ...) UNIT__DECL(UNIT__TYPE_CASE, UNIT__X_CONCAT(u__, __COUNTER__), #Name, __VA_ARGS__)
#define UNIT_TEST(Name, ...) UNIT__DECL(UNIT__TYPE_TEST, UNIT__X_CONCAT(u__, __COUNTER__), "" Name, __VA_ARGS__)

bool unit__prepare_assert(int level, const char* file, int line, const char* comment, const char* desc);

// устанавливает читаемое описание проверки, в случае нормального состояния выполняет проверку,
// если установлено состояние пропускать тесты - НЕ ВЫЧИСЛЯЕТ аргументы для проверки
#define UNIT__ASSERT_LAZY(Assertion, Level, Comment, Description) \
if(unit__prepare_assert(Level, __FILE__, __LINE__, Comment, Description)) Assertion

#define UNIT__IS_TRUE(_, x) (!!(x))
#define UNIT__IS_NOT_EMPTY_STR(_, x) ((x) && (x)[0])
#define UNIT__CMP(a, b) ((a) == (b) ? 0 : ((a) > (b) ? 1 : -1))
#define UNIT__STRCMP(a, b) ((a) == (b) ? 0 : strcmp((a) ? (a) : "", (b) ? (b) : ""))

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
#define UNIT_ECHO(msg) unit__echo(msg)

#ifdef __cplusplus
}
#endif

#endif // UNIT_TESTING

#include "unit-alias.h"

#endif // UNIT_H
