int main(int argc, const char** argv) {
    struct unit_run_options options;
    unit__setup_args(argc, argv, &options);
    return unit_main(options);
}
