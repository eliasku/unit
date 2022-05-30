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

struct unit_printer* unit__printers;

#define UNIT__EACH_PRINTER(Func, ...) \
for(struct unit_printer* p = unit__printers; p; p = p->next) { p->callback(UNIT__PRINTER_ ## Func, __VA_ARGS__); }

bool unit__prepare_assert(int level, const char* loc, const char* comment, const char* desc) {
    unit_cur->assert_comment = comment;
    unit_cur->assert_desc = desc;
    unit_cur->assert_level = level;
    unit_cur->assert_loc = loc;
    if (unit_cur->state & UNIT__LEVEL_REQUIRE) {
        // пропустить проверку
        unit_cur->assert_status = UNIT_STATUS_SKIPPED;
        UNIT__EACH_PRINTER(ASSERTION, unit_cur, 0);
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
        unit_cur->state |= unit_cur->assert_level;
        for (struct unit_test* n = unit_cur; n; n = n->parent) {
            if (n->options.failing) {
                break;
            }
            n->status = UNIT_STATUS_FAILED;
        }
    }
    UNIT__EACH_PRINTER(FAIL, unit_cur, msg);
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
    unit_cur->assert_status = pass ? UNIT_STATUS_SUCCESS : UNIT_STATUS_FAILED; \
    UNIT__EACH_PRINTER(ASSERTION, unit_cur, 0); \
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
    //assert (!child->parent);
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
    if (unit->options.skip) {
        unit->status = UNIT_STATUS_SKIPPED;
    } else if (unit->type == UNIT__TYPE_TEST) {
        for (struct unit_test* u = unit_cur; u; u = u->parent) {
            u->total++;
        }
    }
    UNIT__EACH_PRINTER(BEGIN, unit, 0);
    return !unit->options.skip;
}

void unit__end(struct unit_test* unit) {
    if (unit->type == UNIT__TYPE_TEST && unit->status == UNIT_STATUS_SUCCESS) {
        for (struct unit_test* u = unit_cur; u; u = u->parent) {
            u->passed++;
        }
    }
    unit->elapsed = unit__time(unit->t0);
    UNIT__EACH_PRINTER(END, unit, 0);
    unit_cur = unit->parent;
}

void unit__echo(const char* msg) {
    UNIT__EACH_PRINTER(ECHO, unit_cur, msg);
}

int unit_main(int argc, char** argv) {
    (void) (argc);
    (void) (argv);

    bool verbose = false;
#ifdef UNIT_VERBOSE
    verbose = true;
#endif
//    bool colors = true;
//#ifdef UNIT_NO_COLOR
//    colors = false;
//#endif
    if (argv) {
        for (int i = 0; i < argc; ++i) {
            const char* arg = argv[i];
            if (arg) {
                if (strstr(arg, "--verbose") == arg) {
                    verbose = true;
                }
                // else if (strstr(arg, "--no-color") == arg) {
                //     colors = false;
                // }
            }
        }
    }
    static struct unit_printer printer;
    printer.callback = verbose ? printer_debug : printer_def;
    unit__printers = &printer;

    UNIT__EACH_PRINTER(SETUP, 0, 0);

    int failed = 0;
    for (struct unit_test* suite = unit_tests; suite; suite = suite->next) {
        UNIT_TRY_SCOPE(unit__begin(suite), unit__end(suite)) suite->fn();
        if (suite->status == UNIT_STATUS_FAILED) {
            ++failed;
        }
    }

    UNIT__EACH_PRINTER(SHUTDOWN, 0, 0);

    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}

// endregion

#ifdef __cplusplus
}
#endif

#endif // !UNIT__IMPLEMENTED
