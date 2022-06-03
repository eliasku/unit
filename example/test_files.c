#include <unit.h>
#include <stdio.h>

#ifdef _WIN32
#define IS_WINDOWS 1
#else
#define IS_WINDOWS 0
#endif

// TODO: fix for Windows
SUITE(files, .skip=IS_WINDOWS) {

    ECHO("hello io");

    IT("opens files") {
        FILE* f = fopen("/dev/zero", "r");
        REQUIRE_NE(f, NULL);
        fclose(f);
    }

    IT("writes to files") {
        FILE* f = fopen("testfile", "w");
        REQUIRE_NE(f, NULL);

        char str[] = "hello there";
        REQUIRE_EQ(fwrite(str, 1, sizeof(str), f), sizeof(str));

        fclose(f);
        remove("testfile");
    }

    DESCRIBE(fread) {
        IT("reads 10 bytes") {
            FILE* f = fopen("/dev/zero", "r");
            REQUIRE_NE(f, NULL);

            char buf[10];
            REQUIRE_EQ(fread(buf, 1, 10, f), 10);

            fclose(f);
        }

        IT("reads 20 bytes") {
            FILE* f = fopen("/dev/zero", "r");
            REQUIRE_NE(f, NULL);

            char buf[20];
            REQUIRE_EQ(fread(buf, 1, 20, f), 20);

            fclose(f);
        }

        IT("handle scope in for-loop") {
            size_t executed_times = 0;
            char buf[10];
            for (FILE* f = fopen("/dev/zero", "r"); f; fclose(f), f = NULL) {
                size_t bytes_read = fread(buf, 1, 10, f);
                ++executed_times;
                REQUIRE_EQ(bytes_read, 10);
            }
            REQUIRE_EQ(executed_times, 1);
        }
    }
}
