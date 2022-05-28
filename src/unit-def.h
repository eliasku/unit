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
    struct unit_test* children;
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
    void (* setup)(void);

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

#ifdef UNIT_NO_FILEPOS
#define UNIT__FILEPOS ""
#else
#define UNIT__FILEPOS __FILE__ ":" UNIT__X_STR(__LINE__)
#endif

#define UNIT__SCOPE_BODY(begin, end, Var) for (int Var = (begin, 0); !Var; ++Var, end)
#define UNIT_SCOPE(begin, end) UNIT__SCOPE_BODY(begin, end, UNIT__X_CONCAT(s__, __COUNTER__))

#define UNIT__TRY_BODY(begin, end, Var) for (int Var = (begin) ? 0 : (end, 1); !Var; ++Var, end)
#define UNIT_TRY_SCOPE(begin, end) UNIT__TRY_BODY(begin, end, UNIT__X_CONCAT(s__, __COUNTER__))

// find or create root unit for .c file
struct unit_test* unit__file(struct unit_test* ss, const char* filepath);

#define UNIT__SUITE(Var, Name, ...) \
    static void Var(void); \
    __attribute__((constructor)) static void UNIT__CONCAT(Var, _ctor)(void) { \
        static struct unit_test tmp = (struct unit_test) {.src = __FILE__}; \
        struct unit_test* file = unit__file(&tmp, __FILE__); \
        static struct unit_test u = (struct unit_test) {.name = Name, .src = UNIT__FILEPOS, .fn = &Var, __VA_ARGS__ }; \
        u.parent = file; \
        u.next = file->children; \
        file->children = &u; \
    } \
    static void Var(void)

#define UNIT_SUITE(Name, ...) UNIT__SUITE(UNIT__X_CONCAT(unit__, __COUNTER__), #Name, __VA_ARGS__)

#define UNIT__DECL(IsTest, Var, Name, ...) \
    static struct unit_test Var = (struct unit_test){ \
    .name = Name, .kind = IsTest, .src = UNIT__FILEPOS, __VA_ARGS__}; \
    UNIT_TRY_SCOPE(unit__begin(&Var), unit__end(&Var))

#define UNIT_DESCRIBE(Name, ...) UNIT__DECL(0, UNIT__X_CONCAT(u__, __COUNTER__), #Name, __VA_ARGS__)
#define UNIT_TEST(Name, ...) UNIT__DECL(1, UNIT__X_CONCAT(u__, __COUNTER__), "" Name, __VA_ARGS__)

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

#include "unit-alias.h"

#endif // UNIT_H
