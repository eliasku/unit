#ifdef __cplusplus
extern "C" {
#endif

static void find_bool_arg(int argc, const char** argv, int* var, const char* name, const char* alias) {
    for (int i = 0; i < argc; ++i) {
        const char* v = argv[i];
        if (v && v[0] == '-') {
            ++v;
            if (alias && alias[0] && strstr(v, alias) == v) {
                *var = true;
                return;
            } else if (name && name[0] && v[0] == '-') {
                ++v;
                bool invert = false;
                if (strstr(v, "no-") == v) {
                    v += 3;
                    invert = true;
                }
                if (strstr(v, name) == v) {
                    *var = invert ? 0 : 1;
                    return;
                }
            }
        }
    }
}

static void unit__parse_args(int argc, const char** argv, struct unit_run_options* out_options) {
    find_bool_arg(argc, argv, &out_options->color, "colors", "c");
    find_bool_arg(argc, argv, &out_options->trace, "trace", "t");
    find_bool_arg(argc, argv, &out_options->silent, "silent", "s");
    find_bool_arg(argc, argv, &out_options->animate, "animate", "a");
    for (int i = 0; i < argc; ++i) {
        // hack to trick CLion we are DocTest library tests
        const char* v = argv[i];
        if (v && v[0] && strstr(argv[i], "-r=xml")) {
            out_options->doctest_xml = 1;
        }
    }
}

int main(int argc, const char** argv) {
    struct unit_run_options options = {1, 0, 0, 0, 0, time(NULL)};
#ifdef UNIT_DEFAULT_ARGS
    static const char* cargv[] = { UNIT_DEFAULT_ARGS };
    static const int cargc = sizeof(cargv) / sizeof(cargv[0]);
    unit__parse_args(cargc, cargv, &options);
#endif
    unit__parse_args(argc, argv, &options);
    return unit_main(options);
}

#ifdef __cplusplus
}
#endif
