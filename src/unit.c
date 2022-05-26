#ifdef __cplusplus
extern "C" {
#endif

#ifndef UNIT_NO_MAIN

int main(int argc, char** argv) {
    srand(time(NULL));
    return unit_main(argc, argv);
}

#endif // UNIT_NO_MAIN

#include "printer.c"

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
