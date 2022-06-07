#ifndef UNIT__IMPLEMENTED
#define UNIT__IMPLEMENTED

#ifdef __cplusplus
extern "C" {
#endif

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
#ifndef UNIT_NO_TIME
#ifdef _WIN32
    timespec_get(&ts, TIME_UTC);
#else // _WIN32
    clock_gettime(CLOCK_REALTIME, &ts);
#endif // !_WIN32
#endif // !UNIT_NO_TIME
    return (double) ts.tv_sec + (double) ts.tv_nsec / 1000000000.0 - prev;
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
    unit->state = 0;
    unit->status = run ? UNIT_STATUS_RUN : UNIT_STATUS_SKIPPED;
    unit->assert_desc = NULL;
    unit->passed = 0;
    unit->total = 0;
    if (!unit->parent) {
        add_child(unit_cur, unit);
    }
    unit_cur = unit;
    if (run && unit->type == UNIT__TYPE_TEST) {
        for (struct unit_test* u = unit_cur; u; u = u->parent) {
            u->total++;
        }
    }
    UNIT__EACH_PRINTER(BEGIN, unit, 0);
    unit->t0 = unit__time(0.0);
    return run;
}

void unit__end(struct unit_test* unit) {
    unit->elapsed = unit__time(unit->t0);
    if (unit->status == UNIT_STATUS_RUN) {
        unit->status = UNIT_STATUS_SUCCESS;
        if (unit->type == UNIT__TYPE_TEST) {
            for (struct unit_test* u = unit_cur; u; u = u->parent) {
                u->passed++;
            }
        }
    }
    UNIT__EACH_PRINTER(END, unit, 0);
    unit_cur = unit->parent;
}

void unit__echo(const char* msg) {
    UNIT__EACH_PRINTER(ECHO, unit_cur, msg);
}

struct unit_run_options unit__opts;

static void unit__init_printers(void) {
    static struct unit_printer printer;
    printer.callback = unit__opts.trace ? printer_tracing : printer_def;
    if (unit__opts.doctest_xml) {
        printer.callback = printer_xml_doctest;
    }
    unit__printers = unit__opts.quiet ? NULL : &printer;
}

#define UNIT__MSG_VERSION "unit v" UNIT_VERSION "\n"
#define UNIT__MSG_USAGE "usage: %s OPTIONS\n" \
"options:\n" \
"  --version or -v: Prints the version of `unit` library\n" \
"  --help or -h: Prints usage help message\n" \
"  --list or -l: Prints all available tests\n" \
"  --ascii: Don't use colors and fancy unicode symbols in the output\n" \
"  --quiet or -q: Disables all output\n" \
"  --short-filenames or -S: Use only basename for displaying file-pos information\n" \
"  -r=xml: Special switch prints XML report in DocTest-friendly format (for CLion test run configuration)\n" \
"  --animate or -a: Simulate waits for printing messages, just for making fancy printing animation\n"

static int unit__cmd(struct unit_run_options options) {
    if (options.version) {
        fputs(UNIT__MSG_VERSION, stdout);
        return 0;
    }
    if (options.help) {
        fprintf(stdout, UNIT__MSG_USAGE, options.program);
        return 0;
    }
    if (options.list) {
        for (struct unit_test* suite = unit_tests; suite; suite = suite->next) {
            fputs(beautify_name(suite->name), stdout);
            fputs("\n", stdout);
        }
        return 0;
    }
    return 1;
}

int unit_main(struct unit_run_options options) {
    if (!unit__cmd(options)) {
        return 0;
    }

    unit__opts = options;
    srand(options.seed);
    unit__init_printers();

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

static void find_bool_arg(int argc, const char** argv, int* var, const char* name, const char* alias) {
    for (int i = 0; i < argc; ++i) {
        const char* v = argv[i];
        if (v && v[0] == '-') {
            ++v;
            if (alias && alias[0] && strstr(v, alias) == v) {
                *var = 1;
                return;
            } else if (name && name[0] && v[0] == '-') {
                ++v;
                if (strstr(v, name) == v) {
                    *var = 1;
                    return;
                }
            }
        }
    }
}

static void unit__parse_args(int argc, const char** argv, struct unit_run_options* out_options) {
    find_bool_arg(argc, argv, &out_options->version, "version", "v");
    find_bool_arg(argc, argv, &out_options->help, "help", "h");
    find_bool_arg(argc, argv, &out_options->list, "list", "l");
    find_bool_arg(argc, argv, &out_options->ascii, "ascii", NULL);
    find_bool_arg(argc, argv, &out_options->trace, "trace", "t");
    find_bool_arg(argc, argv, &out_options->quiet, "quiet", "q");
    find_bool_arg(argc, argv, &out_options->animate, "animate", "a");
    find_bool_arg(argc, argv, &out_options->short_filenames, "short-filenames", "S");
    // hack to trick CLion we are DocTest library tests
    find_bool_arg(argc, argv, &out_options->doctest_xml, NULL, "r=xml");
}

static void unit__setup_args(int argc, const char** argv, struct unit_run_options* out_options) {
    *out_options = (struct unit_run_options) {0};
    out_options->seed = (unsigned) time(NULL);
    out_options->program = argc > 0 ? short_filename(argv[0]) : "<unit>";
#ifdef UNIT_DEFAULT_ARGS
    static const char* cargv[] = { UNIT_DEFAULT_ARGS };
    static const int cargc = sizeof(cargv) / sizeof(cargv[0]);
    unit__parse_args(cargc, cargv, out_options);
#endif
    unit__parse_args(argc, argv, out_options);
}

// endregion

#ifdef UNIT_MAIN

#include "unit-main.c"

#endif // UNIT_MAIN

#ifdef UNIT__SELF_TEST

#include "self-test.c"

#endif // UNIT__SELF_TEST

#ifdef __cplusplus
}
#endif

#endif // !UNIT__IMPLEMENTED
