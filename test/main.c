#include <unit.h>

describe(unit) {
    debug("start");

    it("main block contains test") {
        debug("inside first main test");
    }

    it("main block contains multiple tests") {
        debug("inside second main test");
    }

    subdesc(sub_description) {
        it("block #1 in sub_description") {

        }
        it("block #2 in sub_description") {

        }
    }
}

#include <stdio.h>

describe(files) {
    it("opens files") {
        FILE* f = fopen("/dev/zero", "r");
        assertneq(f, NULL);
        fclose(f);
    }

    it("writes to files") {
        FILE* f = fopen("testfile", "w");
        assertneq(f, NULL);

        char str[] = "hello there";
        asserteq(fwrite(str, 1, sizeof(str), f), sizeof(str));

        fclose(f);
        remove("testfile");
    }

    subdesc(fread) {
        it("reads 10 bytes") {
            FILE* f = fopen("/dev/zero", "r");
            assertneq(f, NULL);

            char buf[10];
            asserteq(fread(buf, 1, 10, f), 10);

            fclose(f);
        }

        it("reads 20 bytes") {
            FILE* f = fopen("/dev/zero", "r");
            assertneq(f, NULL);

            char buf[20];
            asserteq(fread(buf, 1, 20, f), 20);

            fclose(f);
        }
    }
}
