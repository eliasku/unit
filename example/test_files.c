#include <unit.h>
#include <stdio.h>

#ifdef _WIN32
#define IS_WINDOWS 1
#else
#define IS_WINDOWS 0
#endif

// TODO: fix for Windows
suite(files, .skip=IS_WINDOWS) {

    echo("hello io");

    it("opens files") {
        FILE* f = fopen("/dev/zero", "r");
        require_ne(f, NULL);
        fclose(f);
    }

    it("writes to files") {
        FILE* f = fopen("testfile", "w");
        require_ne(f, NULL);

        char str[] = "hello there";
        require_eq(fwrite(str, 1, sizeof(str), f), sizeof(str));

        fclose(f);
        remove("testfile");
    }

    describe(fread) {
        it("reads 10 bytes") {
            FILE* f = fopen("/dev/zero", "r");
            require_ne(f, NULL);

            char buf[10];
            require_eq(fread(buf, 1, 10, f), 10);

            fclose(f);
        }

        it("reads 20 bytes") {
            FILE* f = fopen("/dev/zero", "r");
            require_ne(f, NULL);

            char buf[20];
            require_eq(fread(buf, 1, 20, f), 20);

            fclose(f);
        }

        it("handle scope in for-loop") {
            size_t executed_times = 0;
            char buf[10];
            for (FILE* f = fopen("/dev/zero", "r"); f; fclose(f), f = NULL) {
                size_t bytes_read = fread(buf, 1, 10, f);
                ++executed_times;
                require_eq(bytes_read, 10);
            }
            require_eq(executed_times, 1);
        }
    }
}
