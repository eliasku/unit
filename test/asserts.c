// Проверяем, что компиляция заголовка возможна в более чем одной единице сборки (Translation Unit)
#include <unit.h>
#include <stdlib.h>

suite(unit) {
    static int a = 5;

    it("evaluated only once!") {
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
        int b = 10;

        warn(b == 10);
        warn_false(b != 10);
        warn_ne(b, 99);
        warn_eq(b, 10);
        warn_ge(b, 10);
        warn_gt(b, 9);
        warn_le(b, 10);
        warn_lt(b, 11);

        check(b == 10);
        check_false(b != 10);
        check_ne(b, 99);
        check_eq(b, 10);
        check_ge(b, 10);
        check_gt(b, 9);
        check_le(b, 10);
        check_lt(b, 11);

        require(b == 10);
        require_false(b != 10);
        require_ne(b, 99);
        require_eq(b, 10);
        require_ge(b, 10);
        require_gt(b, 9);
        require_le(b, 10);
        require_lt(b, 11);
    }

    it("has float support") {
        double b = 10.01;

        warn(b == 10.01);
        warn_false(b != 10.01);
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
        warn_false("");
        warn_ne("dog", "cat");
        warn_eq("bird", "bird");
        warn_ge("bird", "alpha");
        warn_gt("bird", "alpha");
        warn_le("alpha", "bird");
        warn_lt("alpha", "bird");

        check("is not empty");
        check_false("");
        check_ne("dog", "cat");
        check_eq("bird", "bird");
        check_ge("bird", "alpha");
        check_gt("bird", "alpha");
        check_le("alpha", "bird");
        check_lt("alpha", "bird");

        require("is not empty");
        require_false("");
        require_ne("dog", "cat");
        require_eq("bird", "bird");
        require_ge("bird", "alpha");
        require_gt("bird", "alpha");
        require_le("alpha", "bird");
        require_lt("alpha", "bird");
    }
}
