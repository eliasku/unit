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

bool unit__prepare_assert(int level, const char* file, int line, const char* comment, const char* desc) {
    unit_cur->assert_comment = comment;
    unit_cur->assert_desc = desc;
    unit_cur->assert_level = level;
    unit_cur->assert_file = file;
    unit_cur->assert_line = line;
    if (unit_cur->state & UNIT__LEVEL_REQUIRE) {
        // пропустить проверку
        unit_cur->assert_status = UNIT_STATUS_SKIPPED;
        UNIT__EACH_PRINTER(ASSERTION, unit_cur, 0);
        return false;
    }
    unit_cur->assert_status = UNIT_STATUS_SUCCESS;
    return true;
}

static void unit__fail_impl(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    const char* msg = unit__vbprintf(fmt, args);
    va_end(args);

    if (unit_cur->assert_level > UNIT__LEVEL_WARN) {
        unit_cur->assert_status = UNIT_STATUS_FAILED;
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
    struct timespec ts = {0};
    bool success = false;
#ifdef _WIN32
    success = timespec_get(&ts, TIME_UTC) == TIME_UTC;
#else
    success = clock_gettime(CLOCK_REALTIME, &ts) == 0;
#endif
    if (!success) {
        return 0.0;
    }
    return (double) ts.tv_sec * 1000.0 + (double) ts.tv_nsec / 1000000.0 - prev;
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
    const bool run = !unit->options.skip;
    unit->t0 = unit__time(0.0);
    unit->state = 0;
    unit->status = run ? UNIT_STATUS_RUN : UNIT_STATUS_SKIPPED;
    unit->assert_desc = NULL;
    add_child(unit_cur, unit);
    unit_cur = unit;
    if (run && unit->type == UNIT__TYPE_TEST) {
        for (struct unit_test* u = unit_cur; u; u = u->parent) {
            u->total++;
        }
    }
    UNIT__EACH_PRINTER(BEGIN, unit, 0);
    return run;
}

void unit__end(struct unit_test* unit) {
    if (unit->status == UNIT_STATUS_RUN) {
        unit->status = UNIT_STATUS_SUCCESS;
        if (unit->type == UNIT__TYPE_TEST) {
            for (struct unit_test* u = unit_cur; u; u = u->parent) {
                u->passed++;
            }
        }
    }
    unit->elapsed = unit__time(unit->t0);
    UNIT__EACH_PRINTER(END, unit, 0);
    unit_cur = unit->parent;
}

void unit__echo(const char* msg) {
    UNIT__EACH_PRINTER(ECHO, unit_cur, msg);
}

struct unit_options unit__opts;

bool find_bool_arg(int argc, char** argv, const char* name, const char* alias, bool def) {
    for (int i = 0; i < argc; ++i) {
        const char* v = argv[i];
        if (v) {
            if (v[0] == '-') {
                ++v;
                if (strstr(v, alias) == v) {
                    return true;
                } else if (v[0] == '-') {
                    ++v;
                    bool invert = false;
                    if (strstr(v, "no-") == v) {
                        v += 3;
                        invert = true;
                    }
                    if (strstr(v, name) == v) {
                        return !invert;
                    }
                }
            }
        }
    }
    return def;
}

#ifdef UNIT_VERBOSE
#define UNIT__VERBOSE_V 1
#else
#define UNIT__VERBOSE_V 0
#endif

#ifdef UNIT_NO_COLOR
#define UNIT__COLOR_V 0
#else
#define UNIT__COLOR_V 1
#endif

#ifdef UNIT_ANIMATE
#define UNIT__ANIMATE_V 1
#else
#define UNIT__ANIMATE_V 0
#endif

#ifdef UNIT_QUIET
#define UNIT__QUIET_V 1
#else
#define UNIT__QUIET_V 0
#endif

int unit_main(int argc, char** argv) {
    unit__opts.color = find_bool_arg(argc, argv, "color", "c", UNIT__COLOR_V);
    unit__opts.verbose = find_bool_arg(argc, argv, "verbose", "v", UNIT__VERBOSE_V);
    unit__opts.quiet = find_bool_arg(argc, argv, "quiet", "q", UNIT__QUIET_V);
    unit__opts.animate = find_bool_arg(argc, argv, "animate", "a", UNIT__ANIMATE_V);

    static struct unit_printer printer;
    if (!unit__opts.quiet) {
        printer.callback = unit__opts.verbose ? printer_debug : printer_def;
        unit__printers = &printer;
    }

    // hack to trick CLion we are DocTest library tests
    for (int i = 0; i < argc; ++i) {
        if (argv[i] && strstr(argv[i], "-r=xml")) {
            printer.callback = print_doctest_xml;
        }
    }

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
