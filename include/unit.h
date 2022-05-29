/**
 * unit.h - v0.0.3 - Simple header-only testing library for C - https://github.com/eliasku/unit
 *
 * Minimal example. Compile executable with `-D UNIT_TESTING` to enable tests.
 *
 * ```c
 * #define UNIT_MAIN
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
 * ## Enable Tests
 *
 * By default all test code is stripped away, to enable test-code you should pass `-D UNIT_TESTING` and build executable
 *
 * ## Default configuration
 *
 * In any translation unit you need to `#define UNIT_MAIN` before `#include <unit.h>` to implement the library and
 * generate default main entry-point to run all tests.
 *
 * ## Custom `main()`
 *
 * If you need just to implement library, you `#define UNIT_IMPLEMENT` before `include <unit.h>` in any single translation unit
 *
 **/

/**
 * Disable colorful output
 */
// #define UNIT_NO_COLORS

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
    const char* filepos;
    // 0 - group, 1 - test
    int kind;

    double t0;
    double elapsed_time;

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
        static struct unit_test tmp = (struct unit_test) {.filepos = __FILE__}; \
        struct unit_test* file = unit__file(&tmp, __FILE__); \
        static struct unit_test u = (struct unit_test) {.name = Name, .filepos = UNIT__FILEPOS, .fn = &Var, __VA_ARGS__ }; \
        u.parent = file; \
        u.next = file->children; \
        file->children = &u; \
    } \
    static void Var(void)

#define UNIT_SUITE(Name, ...) UNIT__SUITE(UNIT__X_CONCAT(unit__, __COUNTER__), #Name, __VA_ARGS__)

#define UNIT__DECL(IsTest, Var, Name, ...) \
    static struct unit_test Var = (struct unit_test){ \
    .name = Name, .filepos = UNIT__FILEPOS, .kind = IsTest, __VA_ARGS__}; \
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


#ifdef UNIT_TESTING

#define UNIT_VERSION "0.0.3"

/**
 * @deprecated use UNIT_IMPLEMENT or UNIT_MAIN options
 */
#ifdef UNIT_IMPL
#define UNIT_MAIN
#endif // UNIT_IMPL

#ifdef UNIT_MAIN
#define UNIT_IMPLEMENT
#endif // UNIT_MAIN

#ifdef UNIT_IMPLEMENT
#ifndef UNIT__IMPLEMENTED
#define UNIT__IMPLEMENTED

#ifdef __cplusplus
extern "C" {
#endif

#ifdef UNIT_MAIN

int main(int argc, char** argv) {
    srand(time(NULL));
    return unit_main(argc, argv);
}

#endif // UNIT_MAIN
#include <stdio.h>

#ifdef _WIN32

/* https://github.com/Arryboom/fmemopen_windows  */
#include <windows.h>
#include <share.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>

FILE* fmemopen(void* buf, size_t len, const char* type) {
    int fd;
    FILE* fp;
    char tp[MAX_PATH - 13];
    char fn[MAX_PATH + 1];
    int* pfd = &fd;
    int retner = -1;
    char tfname[] = "MemTF_";
    if (!GetTempPathA(sizeof(tp), tp))
        return NULL;
    if (!GetTempFileNameA(tp, tfname, 0, fn))
        return NULL;
    retner = _sopen_s(pfd, fn, _O_CREAT | _O_SHORT_LIVED | _O_TEMPORARY | _O_RDWR | _O_BINARY | _O_NOINHERIT,
                      _SH_DENYRW, _S_IREAD | _S_IWRITE);
    if (retner != 0)
        return NULL;
    if (fd == -1)
        return NULL;
    fp = _fdopen(fd, "wb+");
    if (!fp) {
        _close(fd);
        return NULL;
    }
    /*File descriptors passed into _fdopen are owned by the returned FILE * stream.If _fdopen is successful, do not call _close on the file descriptor.Calling fclose on the returned FILE * also closes the file descriptor.*/
    fwrite(buf, len, 1, fp);
    rewind(fp);
    return fp;
}

#endif


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
#define UNIT_COLOR_INVERT_PASS "\033[30;42m"
#define UNIT_COLOR_LABEL_PASS UNIT_COLOR_BOLD UNIT_COLOR_INVERT_PASS
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
#define UNIT_COLOR_INVERT_PASS
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
    UNIT_PUTS("\n" UNIT_COLOR_LABEL_RUNS " ✓ηỉτ " UNIT_COLOR_RESET UNIT_COLOR_INVERT_PASS " v" UNIT_VERSION " " UNIT_COLOR_RESET "\n\n");
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


struct unit_test* unit_tests = NULL;
struct unit_test* unit_cur = NULL;

// find or create root unit for .c file
struct unit_test* unit__file(struct unit_test* new_unit, const char* filepath) {
    struct unit_test* t = unit_tests;
    while (t) {
        if (t->filepos == filepath) {
            return t;
        }
        t = t->next;
    }
    const char* filename = strrchr(filepath, '/');
    new_unit->name = filename ? (filename + 1) : filepath;
    new_unit->next = unit_tests;
    unit_tests = new_unit;
    return new_unit;
}

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

static struct unit_test* get_last_child(struct unit_test* children) {
    while (children && children->next) {
        children = children->next;
    }
    return children;
}

static void add_child(struct unit_test* parent, struct unit_test* child) {
    if (child->parent) {
        return;
    }
    if (parent) {
        struct unit_test* last = get_last_child(parent->children);
        if (last) {
            last->next = child;
        } else {
            child->next = parent->children;
            parent->children = child;
        }
    }
    child->parent = parent;
}

int unit__begin(struct unit_test* unit) {
    unit->t0 = unit__time(0.0);
    unit->state = 0;
    unit->status = UNIT_STATUS_SUCCESS;
    unit->assert_desc = NULL;
    add_child(unit_cur, unit);
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

    unit_printer.setup();

    int failed = 0;
    for (struct unit_test* file = unit_tests; file; file = file->next) {
        UNIT_TRY_SCOPE(unit__begin(file), unit__end(file)) {
            for (struct unit_test* suite = file->children; suite; suite = suite->next) {
                UNIT_TRY_SCOPE(unit__begin(suite), unit__end(suite)) suite->fn();
            }
        }
        failed += (file->passed < file->total) ? 1 : 0;
    }

    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}

// endregion

#ifdef __cplusplus
}
#endif

#endif // !UNIT__IMPLEMENTED


#endif // UNIT_IMPLEMENT

#endif // UNIT_TESTING
