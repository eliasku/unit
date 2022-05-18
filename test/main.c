#include <unit.h>

suite(unit) {
    echo("start");

    it("contains test") {
        echo("inside first main test");
    }

    it("contains test") {
        echo("inside second main test");
    }

    describe(multiple tests) {
        it("block #1 in sub_description") {

        }
        it("block #2 in sub_description") {

        }
    }

    describe(linear execution) {
        it("and could contains skip directive") {
            require(!0, done);
            skip();
            require(0 != 1, todo);
        }
    }

    describe(.allow_fail flag, .allow_fail=1) {
        it("which pass the test in case of failure") {
            require_eq(2, 2);
            echo("Next require will fail");
            require_eq(1, 0);
            echo("Execution actually continues,");
            echo("but another checks will be ignored after fail.");
            require(0);
            require(NULL);
        }
    }
}

suite(asserts) {
    static int a = 5;

    it("evaluated only once!") {
        require_eq(++a, 6);
        require(++a == 7);
        check_eq(++a, 8);
        check(++a == 9);

        // дальше мы пропускаем проверки, они будут помечены в отчёте как пропущенные,
        // но они не должны выполнять проверяемые выражения внутри, значит переменная не должна больше изменяться
        skip();
        require_eq(++a, 10);
        require(++a == 11);
        check_eq(++a, 12);
        check(++a == 13);

        // этот код выполняется в любом случае,
        if(a != 9) exit(EXIT_FAILURE);
    }
}

#include <stdio.h>

suite(files) {
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
