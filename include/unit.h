#ifndef UNIT_H
#define UNIT_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Time utilities **/

bool unit_timestamp(struct timespec* ts) {
#ifdef _WIN32
    if (timespec_get(ts, TIME_UTC) == TIME_UTC) {
        return true;
    }
#else
    if (clock_gettime(CLOCK_REALTIME, ts) == 0) {
        return true;
    }
#endif
    return false;
}

double unit_diff_time(struct timespec* t0, struct timespec* t1) {
    return (double) (t1->tv_sec - t0->tv_sec) + ((double) (t1->tv_nsec - t0->tv_nsec) / 1000000000L);
}

/** Colors, messaging, logging **/

// #define UNIT_NO_COLORS

#ifndef UNIT_NO_COLORS
#define UNIT_COLOR_RESET "\033[0m"
#define UNIT_COLOR_BOLD "\033[1m"
#define UNIT_COLOR_WHITE "\033[97m"
#define UNIT_COLOR_MAYBE "\033[35m"
#define UNIT_COLOR_SUCCESS "\033[92m"
#define UNIT_COLOR_FAIL "\033[91m"
#define UNIT_COLOR_DESC "\033[33m"
#else
#define UNIT_COLOR_RESET
#define UNIT_COLOR_BOLD
#define UNIT_COLOR_WHITE
#define UNIT_COLOR_MAYBE
#define UNIT_COLOR_SUCCESS
#define UNIT_COLOR_FAIL
#define UNIT_COLOR_DESC
#endif

// $prefix 0 Status: $message
#define UNIT_MSG_TESTING "\n%s" "◆ " UNIT_COLOR_BOLD UNIT_COLOR_WHITE "Testing %s" UNIT_COLOR_RESET ":\n"
#define UNIT_MSG_RUN "%s" UNIT_COLOR_BOLD UNIT_COLOR_SUCCESS "▶ " UNIT_COLOR_RESET UNIT_COLOR_BOLD UNIT_COLOR_WHITE "%s" UNIT_COLOR_RESET "\n"
#define UNIT_MSG_SUCCESS "%s" UNIT_COLOR_BOLD UNIT_COLOR_SUCCESS "✓ " UNIT_COLOR_RESET UNIT_COLOR_SUCCESS "Success: " UNIT_COLOR_RESET UNIT_COLOR_BOLD UNIT_COLOR_DESC "%s" UNIT_COLOR_RESET " (%0.2lf ms)\n"
#define UNIT_MSG_SKIPPED "%s" UNIT_COLOR_BOLD "? " UNIT_COLOR_RESET "Skipped: " UNIT_COLOR_RESET UNIT_COLOR_BOLD UNIT_COLOR_DESC "%s" UNIT_COLOR_RESET " (%0.2lf ms)\n"
#define UNIT_MSG_FAILED "%s" UNIT_COLOR_BOLD UNIT_COLOR_FAIL "✕ " UNIT_COLOR_RESET UNIT_COLOR_FAIL "Failed: " UNIT_COLOR_RESET UNIT_COLOR_BOLD UNIT_COLOR_DESC "%s" UNIT_COLOR_RESET " (%0.2lf ms)\n"

#define UNIT_MSG_TEST_PASSED "%s" UNIT_COLOR_BOLD UNIT_COLOR_WHITE "%s: Passed %d/%d tests" UNIT_COLOR_RESET ". (%0.2lf ms)\n"

#define UNIT_MSG_DEBUG "%s⚬ %s\n"

#define UNIT_LOG_DEBUG(msg) printf(UNIT_MSG_DEBUG, unit_get_spaces(), msg)

struct unit_test {
    const char* name;
    const char* src;
    struct timespec t0;
    struct timespec t1;

    void (* fn)(void);

    struct unit_test* next;
    struct unit_test* parent;

    int total;
    int passed;
};

static const char* unit_spaces[8] = {
        "",
        "  ",
        "    ",
        "      ",
        "       ",
        "        ",
        "         ",
        "          "
};
static int unit_depth = 0;

static const char* unit_get_spaces_with(int delta) {
    int i = unit_depth + delta;
    if (i < 0) i = 0;
    if (i > 7) i = 7;
    return unit_spaces[i];
}

static const char* unit_get_spaces(void) {
    return unit_get_spaces_with(0);
}

static struct unit_test* unit_tests = NULL;
static struct unit_test* unit_cur = NULL;
static struct {
    // status of current assertion
    int status;
    // state for this test scope
    // позволять ли дальше работать другим проверкам в рамках этого теста
    int state;
    struct timespec t0;
    const char* desc;
} unit_test_cur = {0};

static void unit_it_begin(const char* desc) {
    printf(UNIT_MSG_RUN, unit_get_spaces(), desc);
    unit_test_cur.state = 0;
    unit_test_cur.status = 0;
    unit_timestamp(&unit_test_cur.t0);
    unit_test_cur.desc = desc;
    for (struct unit_test* u = unit_cur; u; u = u->parent) {
        u->total++;
    }
    ++unit_depth;
}

static const char* unit_get_status_msg(int status) {
    if (status == 0) return UNIT_MSG_SUCCESS;
    else if (status == 1) return UNIT_MSG_SKIPPED;
    return UNIT_MSG_FAILED;
}

static void unit_it_end(void) {
    if (unit_test_cur.status == 0) {
        for (struct unit_test* u = unit_cur; u; u = u->parent) {
            u->passed++;
        }
    }

    struct timespec t1;
    unit_timestamp(&t1);
    double elapsed_time = unit_diff_time(&unit_test_cur.t0, &t1);
    printf(unit_test_cur.status ? UNIT_MSG_FAILED : UNIT_MSG_SUCCESS, unit_get_spaces(), unit_test_cur.desc,
           elapsed_time * 1000.0);

    --unit_depth;
}

#define _STRINGIZE(x) #x
#define STRINGIZE(x) _STRINGIZE(x)
#define JOIN2(a, b) a ## b
#define VAR_(Id, Line) JOIN2(Id, Line)
#define VAR(Id) VAR_(Id, __LINE__)
#define SOURCE_LOC __FILE__ ":" STRINGIZE(__LINE__)

#define UNIT_ID(Name, Tag) unit_test_ ## Name ## _ ## Tag
#define UNIT_DESCRIBE(Name) \
void UNIT_ID(Name, main) (void); \
static struct unit_test UNIT_ID(Name, data) = \
(struct unit_test) {        \
    .name = #Name,           \
    .src = SOURCE_LOC, \
    .fn = &UNIT_ID(Name, main), \
};                          \
__attribute__((constructor)) void UNIT_ID(Name, ctor)(void) { \
    UNIT_ID(Name, data).next = unit_tests;    \
    unit_tests = &UNIT_ID(Name, data); \
}\
void UNIT_ID(Name, main) (void)

#define UNIT_SUB_DESC(Name) \
static struct unit_test VAR(_u_) = (struct unit_test){.name = #Name, .src = SOURCE_LOC}; \
unit_begin(&VAR(_u_)); \
for(int VAR(_i_) = 0; !VAR(_i_); ++VAR(_i_), unit_end(&VAR(_u_)))

#define UNIT_IT(Description) \
unit_it_begin(Description); \
for(int VAR(_i_) = 0; !VAR(_i_); ++VAR(_i_), unit_it_end())

static bool unit_prev_print_results = false;

static void unit_begin(struct unit_test* unit) {
    unit->parent = unit_cur;
    unit_cur = unit;
    unit_timestamp(&unit->t0);

    printf(UNIT_MSG_TESTING, unit_get_spaces(), unit->name);
    unit_prev_print_results = false;
    ++unit_depth;
}

static void unit_end(struct unit_test* unit) {
    unit_timestamp(&unit->t1);
    const double elapsed_time = unit_diff_time(&unit->t0, &unit->t1);
    // keep separator between blocks
    if (unit_prev_print_results) {
        putchar('\n');
    }
    --unit_depth;
    printf(UNIT_MSG_TEST_PASSED, unit_get_spaces(), unit->name, unit->passed, unit->total, elapsed_time * 1000.0);
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

#ifdef __cplusplus
}
#endif

#ifndef UNIT_NO_MAIN

int main(int argc, char** argv) {
    srand(time(NULL));
    return unit_main(argc, argv);
}

#endif // UNIT_NO_MAIN

#endif // UNIT_H

#define describe(...) UNIT_DESCRIBE(__VA_ARGS__)
#define subdesc(...) UNIT_SUB_DESC(__VA_ARGS__)
#define it(...) UNIT_IT(__VA_ARGS__)
#define debug(...) UNIT_LOG_DEBUG(__VA_ARGS__)

/** Assert functions **/

#define UNIT_FAIL_IMPL(Flag, ...) do { \
    unit_test_cur.status = 2;   \
    unit_test_cur.state |= Flag;   \
    printf("%s", unit_get_spaces_with(1));                        \
    printf(__VA_ARGS__);    \
    printf("\n%sin %s (%s : %s)\n", unit_get_spaces_with(1), SOURCE_LOC, unit_cur->name, unit_test_cur.desc); \
} while(0)

#define _UNIT_ASSERT(Body, OnFail) do { \
    if((unit_test_cur.state & 2) == 2) {\
        printf("%s⚬ Skip: %s\n", unit_get_spaces(), #Body);                                    \
    } else if(Body) {                      \
        printf("%s" UNIT_COLOR_BOLD UNIT_COLOR_SUCCESS "✓ " UNIT_COLOR_RESET UNIT_COLOR_SUCCESS "Pass: " UNIT_COLOR_RESET "%s\n", unit_get_spaces(), #Body);     \
    } else {                          \
        printf("%s" UNIT_COLOR_BOLD UNIT_COLOR_FAIL "✕ " UNIT_COLOR_RESET UNIT_COLOR_FAIL "Failed: " UNIT_COLOR_RESET "%s\n", unit_get_spaces(), #Body);     \
        OnFail;                                \
    }                           \
} while(0)

#define check_eq(a, b) _UNIT_ASSERT((a) != (b), UNIT_FAIL_IMPL(0, "Assertion failed: %d == %d", a, b))
#define assertneq(a, b) _UNIT_ASSERT((a) != (b), UNIT_FAIL_IMPL(2, "Assertion failed: %d != %d", a, b))
#define asserteq(a, b) _UNIT_ASSERT((a) == (b), UNIT_FAIL_IMPL(2, "Assertion failed: %d == %d", a, b))
#ifdef assert
#undef assert
#endif
#define assert(x, msg...) _UNIT_ASSERT((x), UNIT_FAIL_IMPL(2, "Assertion failed: %d", x))
