// Проверяем, что компиляция заголовка возможна в более чем одной единице сборки (Translation Unit)
#include <unit.h>
#include <stdlib.h>

suite(asserts) {
    it("evaluated only once!") {
        int a = 5;

        require_eq(++a, 6);
        require(++a == 7);
        check_eq(++a, 8);
        check(++a == 9);
        warn_eq(++a, 10);
        warn(++a == 11);

        // дальше мы пропускаем проверки, они будут помечены в отчёте как пропущенные,
        // но они не должны выполнять проверяемые выражения внутри, значит переменная не должна больше изменяться
        skip();
        warn_eq(++a, 12);
        warn(++a == 13);
        check_eq(++a, 14);
        check(++a == 15);
        require_eq(++a, 16);
        require(++a == 17);

        // этот код выполняется в любом случае,
        if (a != 11) exit(EXIT_FAILURE);
    }

    it("has int support") {
        int b = -10;
        int zero = 0;

        warn(b);
        warn_false(zero);
        warn_ne(b, 99);
        warn_eq(b, -10);
        warn_ge(b, -10);
        warn_gt(b, -11);
        warn_le(b, -10);
        warn_lt(b, -9);

        check(b);
        check_false(zero);
        check_ne(b, 99);
        check_eq(b, -10);
        check_ge(b, -10);
        check_gt(b, -11);
        check_le(b, -10);
        check_lt(b, -9);

        require(b);
        require_false(zero);
        require_ne(b, 99);
        require_eq(b, -10);
        require_ge(b, -10);
        require_gt(b, -11);
        require_le(b, -10);
        require_lt(b, -9);
    }

    it("has uint support") {
        unsigned int b = 10;
        unsigned int zero = 0;

        warn(b);
        warn_false(zero);
        warn_ne(b, 99u);
        warn_eq(b, 10u);
        warn_ge(b, 10u);
        warn_gt(b, 9u);
        warn_le(b, 10u);
        warn_lt(b, 11u);

        check(b);
        check_false(zero);
        check_ne(b, 99u);
        check_eq(b, 10u);
        check_ge(b, 10u);
        check_gt(b, 9u);
        check_le(b, 10u);
        check_lt(b, 11u);

        require(b);
        require_false(zero);
        require_ne(b, 99u);
        require_eq(b, 10u);
        require_ge(b, 10u);
        require_gt(b, 9u);
        require_le(b, 10u);
        require_lt(b, 11u);
    }

    it("has float support") {
        double b = 10.01;
        double zero = 0.0;

        warn(b);
        warn_false(zero);
        warn_ne(b, 99.0);
        warn_eq(b, 10.01);
        warn_ge(b, 10.01);
        warn_gt(b, 9.0);
        warn_le(b, 10.01);
        warn_lt(b, 11.0);

        check(b == 10.01);
        check_false(b != 10.01);
        check_ne(b, 99.0);
        check_eq(b, 10.01);
        check_ge(b, 10.01);
        check_gt(b, 9.0);
        check_le(b, 10.01);
        check_lt(b, 11.0);

        require(b == 10.01);
        require_false(b != 10.01);
        require_ne(b, 99.0);
        require_eq(b, 10.01);
        require_ge(b, 10.01);
        require_gt(b, 9.0);
        require_le(b, 10.01);
        require_lt(b, 11.0);
    }

    it("has c-string support") {
        warn("is not empty");
        warn_false((const char*) NULL);
        warn_false("");
        warn_ne("dog", "cat");
        warn_eq("bird", "bird");
        warn_ge("bird", "alpha");
        warn_gt("bird", "alpha");
        warn_le("alpha", "bird");
        warn_lt("alpha", "bird");

        check("is not empty");
        check_false((const char*) NULL);
        check_false("");
        check_ne("dog", "cat");
        check_eq("bird", "bird");
        check_ge("bird", "alpha");
        check_gt("bird", "alpha");
        check_le("alpha", "bird");
        check_lt("alpha", "bird");

        require("is not empty");
        require_false((const char*) NULL);
        require_false("");
        require_ne("dog", "cat");
        require_eq("bird", "bird");
        require_ge("bird", "alpha");
        require_gt("bird", "alpha");
        require_le("alpha", "bird");
        require_lt("alpha", "bird");
    }

    it("accept NULL c-string as empty string") {
        require_false((const char*) NULL);
        require_gt("alpha", (const char*) NULL);
        require_lt((const char*) NULL, "omega");
        require_eq(NULL, "");
        require_ne(NULL, "beta");
        require_le(NULL, "");
        require_ge(NULL, "");
    }

    it("compares pointer type") {
        int v1 = 1; // 0
        int v2 = 2; // 1
        require(v1);
        require(v2);
        const void* ptr1 = &v1;
        const void* ptr2 = &v2;
        require(ptr1);
        require(ptr2);
        require_lt(ptr2, ptr1);
        require_gt(ptr1, ptr2);
        require_le(NULL, ptr2);
        require_ge(ptr2, NULL);
    }

    it("has void* support") {
        const void* zero = NULL;
        const void* ptr = &zero;
        require_false(zero);
        require(ptr);
        require_eq(zero, NULL);
        require_ne(ptr, zero);
    }

    it("fail cases", .failing=1) {
        const void* zero = NULL;
        const char* str = NULL;
        unsigned u = 2;
        int s = -2;
        double f = 0.0;

        check(zero);
        check_eq(zero, (const void*) &str);
        check(str);
        check_eq(str, "string");
        check_false(u);
        check_le(u, 0u);
        check_false(s);
        check_gt(s, 0);
        check(f);
        check_gt(f, 0.0);
    }
}
